<?php

class Pickem extends Entity {
	
	public static function sportOptions() {
		return array(
			'ncaaf' => t('College Football'),
			'ncaam' => t('Men\'s College Basketball'),
			'ncaaw' => t('Women\'s College Basketball'),
			'ncaab' => t('College Basketball'),
			'nfa' => t('NFL'),
			'nba' => t('NBA'),
		);
	}
	
	public function __construct($values = array()) {
		parent::__construct($values, 'pickem');
		
		// load the slate for the current week
		// @TODO determine 'current week'
		// $pickem_slate = entity_load('pickem_slate', FALSE, array('pid' => $this->pid, 'week' => $this->currentWeek()));
		$query = new EntityFieldQuery();
	  $query
	    ->entityCondition('entity_type', 'pickem_slate', '=')
	    ->propertyCondition('pid', $this->pid, '=')
			->propertyOrderBy('week');
	
	  $result = $query->execute();
	  
		$this->pickem_slate = array();
		if (!empty($result)) {
		  $slids = array();
		  foreach($result['pickem_slate'] as $record) {
		    $slids[] = $record->slid;
		  }
		  $pickem_slate = entity_load('pickem_slate', $slids);
		
			//$pickem_slate = entity_load('pickem_slate', FALSE, array('pid' => $this->pid));
			$this->pickem_slate = $pickem_slate;
		}
		
		// Load pickem players
		$pickem_players = entity_load('pickem_player', FALSE, array('pickem_id' => $this->pid));
		$this->players = $pickem_players;
		
		// NOTE: WEEKS is the top logical divide, 
		// with precedent over Pickem Slate
		
		// Load Weeks
		$weeks = db_select('week', 'w')
			->fields('w')
			->condition('pickem_id', $this->pid)
			->orderBy('week')
			->execute();
			
		$this->slate_weeks = array();

		foreach ($weeks as $week) {
			$query = new EntityFieldQuery();
			$query
				->entityCondition('entity_type', 'pickem_slate', '=')
				->propertyCondition('pid', $this->pid, '=')
				->propertyCondition('week', $week->week, '=')
				->propertyOrderBy('slate_date', 'ASC');
				
			$result = $query->execute();
			if (!empty($result)) {
			  $slids = array();
			  foreach($result['pickem_slate'] as $record) {
			    $slids[] = $record->slid;
			  }
			  $pickem_slate = entity_load('pickem_slate', $slids);
				$week->pickem_slate = $pickem_slate;
			}
			
			$this->slate_weeks[$week->week] = $week;
		}
	}

	public function currentWeek($ts=NULL) {
		if ($ts == NULL) {
			$ts = time();
		}
		foreach ($this->slate_weeks as $week) {
			if ($week->start > $ts) {
				return $week->week;
			}
		}
		return FALSE;
	}
	
	public function active() {
		return in_array($this->active, array(1, 'Y', 'y'));
	}
	
	public function getPickemSlate() {
		return $this->pickem_slate;
	}
	
	public function slateWeekIsOpen($week) {
		return strtoupper($this->slate_weeks[$week]->open) == 'Y';
	}
	
	public function getPlayer($uid) {
		if (empty($this->players)) {
			return FALSE;
		}
		
		$GLOBALS['search-uid'] = $uid;
		$inline = function($var) {
			return $var->uid == $GLOBALS['search-uid']; 
		};
		
		$players = array_filter($this->players, $inline);
		unset($GLOBALS['search-uid']);
		return reset($players);
	}
	
	public function getPickemSlateForWeek($week) {
		return $this->slate_weeks[$week]->pickem_slate;
	}
	
	public function userNeedsProvisioning($uid, $week) {
		$pickem_week = $this->slate_weeks[$week];
		foreach ($pickem_week->pickem_slate as $slid => $slate_event) {
			if (isset($slate_event->picks[$uid])) {
				return FALSE;
			}
		}
		 
		return TRUE;		
	}
	
	public function getWeeks() {
		return $this->weeks;
	}
	
	public function getSlateWeeks() {
		return $this->slate_weeks;
	}
}

class PickemUIController extends EntityDefaultUIController {
	
}

class PickemAPIController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
	public function addPlayer($uid, $uname, $pickem_id) {
		$bundle = 'pickem_player';
		$entity = entity_create('pickem_player', array('type' => $bundle));
		$entity->uid = $uid;
		$entity->pickem_id = $pickem_id;
		$entity->name = $uname;
		$entity->player_data = serialize(
			array(
				'madepicks' => array(),
				'savedpicks' => TRUE,
				'bye_count' => 0,
				'last_picksave' => time(),
			)
		);
		$entity->save();		
	}
	
	public function processSportsEventResults($pickem, $sportsEvent) {
		// TODO find the PickemSlate entities attached to the sportsevent->eid value
		$conditions = array('pid' => $pickem->pid, 'event_entity_id' => $sportsEvent->eid, 'event_entity_type' => 'sportsevent');

		$pickemSlate = entity_load('pickem_slate', FALSE, $conditions, TRUE);
		$pickemSlate = reset($pickemSlate);

		$response = array('correct' => array(), 'incorrect' => array(), 'processed' => 0,);
		
		if (count($pickemSlate->picks)) {
			foreach ($pickemSlate->picks as $player_uid => $pick) {
				$pick->correct = ($sportsEvent->winning_team_name) == $pick->pick_team_name ? 'Y' : 'N';
				if ($pick->correct == 'Y') {
					$response['correct'][] = sprintf("%s correctly picked winner (%s)", $pick->playername, $pick->pick_team_name);
				} else {
					$response['incorrect'][] = sprintf("%s incorrectly picked winner (%s)", $pick->playername, $pick->pick_team_name);
				}
				$response['processed']++;
			}
		}
		return $response;	// return the number of PickemSlate -> picks processed.
	}
}

class PickemSlate extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'pickem_slate');

		$query = new EntityFieldQuery();
	  $query
	    ->entityCondition('entity_type', 'pick', '=')
	    ->propertyCondition('slid', $this->slid, '=');
	
	  $result = $query->execute();
	  
		$this->picks = array();
		$this->picks_by_team = array();
		
		if (!empty($result)) {
		  $pkids = array();
		  foreach($result['pick'] as $record) {
		    $pkids[] = $record->pkid;
		  }
		  $picks = entity_load('pick', $pkids);
			
			foreach ($picks as $pkid => $p) {
				$this->picks[$p->uid] = $p;
				
				if (!array_key_exists($p->pick_team_name, $this->picks_by_team)) {
					$this->picks_by_team[$p->pick_team_name] = array();
				}
				$this->picks_by_team[$p->pick_team_name][] = $p->playername;
								
			}
		
			//$pickem_slate = entity_load('pickem_slate', FALSE, array('pid' => $this->pid));
			#$this->picks = $picks;
		}
		
		$this->sportsEvent = FALSE;
		if (!empty($this->event_entity_type)) {
			$sportsevents = entity_load($this->event_entity_type, array($this->event_entity_id));
			$this->sportsEvent = reset($sportsevents);
			
			// provide direct access to the opponents (in this case, the host and visitor)
			$host_key = $this->sportsEvent->host_team->name;
			$this->$host_key = $this->sportsEvent->host_team->displaytitle;
			$visitor_key = $this->sportsEvent->visiting_team->name;
			$this->$visitor_key = $this->sportsEvent->visiting_team->displaytitle;
		}
		
		$this->comments = entity_load('pick_comment', FALSE, array('slid' => $this->slid), TRUE);
		#$this->sportEvent = reset($games);
		
		#$pickController = entity_get_controller('pick');
	}
	
	public function load_pick_for_user($uid) {
		if (isset($this->picks[$uid])) {
			return $this->picks[$uid];
		}
		// TODO convert 'pick' into an entity-based object, then..
		// $pick = entity_load('pick', FALSE, array('uid' => $uid));
		// return reset($pick);
		return FALSE;
	}
	
	public function pickCount() {
		return count($this->picks);
	}
	
	public function completed() {
		return $this->sportsEvent->completed();
	}
	public function getSportsEvent() {
		return $this->sportsEvent;
	}
	public function eventResult() {
		return $this->sportsEvent->eventResult();
	}
	
	public function displayTitle() {
		return $this->sportsEvent->displayTitle();
	}
		
	public function displayTitleWithLines() {
		return $this->sportsEvent->displayTitleWithLines();
	}

	public function displayTitleMobile() {
		return $this->sportsEvent->displayTitleMobile();
	}
	
	public function eventDate($drupalDateType='medium') {
		return $this->sportsEvent->eventDate($drupalDateType);
	}
	
	public function opponents() {
		return $this->sportsEvent->opponents();
	}
	
	public function pick_team_name($uid) {
		if (empty($this->picks)) {
			return '';
		}
		if (!isset($this->picks[$uid])) {
			return '';
		}
		return $this->picks[$uid]->pick_team_name;
	}
	
	public function picks_by_team() {
		
	}
	
	public function comments() {
		return $this->comments;
	}
}

class PickemSlateUIController extends EntityDefaultUIController {
	
}

class PickemSlateController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}
	
	public function cutoffTimeForPick($slateEvent, $pickem_week) {
		
		// DEFAULT IMPLEMENTATION based on college football pickem culture
		$slate_date = (int) $slateEvent->slate_date;
		
		$dayOfWeek = date('D', $slate_date);
		if (in_array($dayOfWeek, array('Thu', 'Fri'))) {
			return $slate_date - (15 * 60);
		} else {
			$saturdayAtNoon = mktime(12, 0, 0, 
				date('n', $slate_date), 
				date('j', $slate_date), 
				date('Y', $slate_date)
				);
			if ($dayOfWeek == 'Sun') {
				$saturdayAtNoon -= 86400;
			}
			if ($dayOfWeek == 'Mon') {
				$saturdayAtNoon -= (86400 * 2);
			}
			return $saturdayAtNoon - (15 * 60);
		}
		
		return FALSE;
	}
	
	public function eventCanAcceptPicks($slateEvent, $time, $pickem_week) {
		if (is_null($time)) {
			$time = time();
		}
		
		if ($pickem_week < $slateEvent->week) {
			return FALSE;
		}
		
		if ($slateEvent->getSportsEvent()->completed()) {
			return FALSE;
		}
		
		// DEFAULT IMPLEMENTATION based on college football pickem culture
		$slate_date = (int) $slateEvent->slate_date;
		
		if ($time > $slate_date) {
			return FALSE;
		}
		
		$dayOfWeek = date('D', $slate_date);
		if (in_array($dayOfWeek, array('Thu', 'Fri'))) {
			$delta = $slate_date - ($time + (15 * 60));
			return $delta > 0;
		} else {
			$saturdayAtNoon = mktime(12, 0, 0, 
				date('n', $slate_date), 
				date('j', $slate_date), 
				date('Y', $slate_date)
				);
			if ($dayOfWeek == 'Sun') {
				$saturdayAtNoon -= 86400;
			}
			if ($dayOfWeek == 'Mon') {
				$saturdayAtNoon -= (86400 * 2);
			}
			$delta = $saturdayAtNoon;
			return $delta > 0;
		}
		
		return TRUE;
	}
}

class Pick 
	extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'pick');
		
		// TODO load comments
	}
	
	public function pickedTeam() {
		// If the user (player) made an initial pick for the target game, 
		// then turned around and changed his/her mind, but failed to 'finalize' (save)
		// the pick during the session, return that value
		if (in_array('pick_team_name_session', get_object_vars($this))) {
			return $this->pick_team_name_session;
		}
		return $this->pick_team_name;
	}

}

class PickUIController extends EntityDefaultUIController {
	
}

class PickController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}

/**
 * PickComment class
 */
class PickComment
	extends Entity {
	
	public function __construct($values = array()) {
		parent::__construct($values, 'pick_comment');
	}		
}

class PickCommentUIController extends EntityDefaultUIController {
	
}

class PickCommentController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
}	
	
/**
 * PickemPlayer class
 */
class PickemPlayer 
	extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'pickem_player');
	}
	
	public function setUp() {
		if ($this->player_data == NULL) {
			$this->player_data = array(
				'madepicks' => array(),
				'savedpicks' => TRUE,
				'bye_count' => 0,
				'last_picksave' => time(),
			);
			return;
			//$this->save();
		}
		if (is_array($this->player_data)) {
			return;
		}
		if ($this->player_data == 'Array') {
			$this->player_data = array(
				'madepicks' => array(),
				'savedpicks' => TRUE,
				'bye_count' => 0,
				'last_picksave' => time(),
			);
			return;
		}
		try {
			$this->player_data = unserialize($this->player_data);
		} catch (Exception $playerDataException) {
			$this->player_data = array(
				'madepicks' => array(),
				'savedpicks' => TRUE,
				'bye_count' => 0,
				'last_picksave' => time(),
			);
		}
	}
}
	
class PickemPlayerUIController extends EntityDefaultUIController {
	
}

class PickemPlayerController extends EntityAPIController {
		
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}
