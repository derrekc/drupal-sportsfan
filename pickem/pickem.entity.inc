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
				->propertyCondition('week', $week->week, '=');
				
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
	
	public function addPlayer($uid, $pickem_id) {
		$bundle = 'pickem_player';
		$entity = entity_create('pickem_player', array('type' => $bundle));
		$entity->uid = $uid;
		$entity->pickem_id = $pickem_id;
		$entity->save();		
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
		if (!empty($result)) {
		  $pkids = array();
		  foreach($result['pick'] as $record) {
		    $pkids[] = $record->pkid;
		  }
		  $picks = entity_load('pick', $pkids);
			foreach ($picks as $pkid => $p) {
				$this->picks[$pkid->uid] = $p;
			}
		
			//$pickem_slate = entity_load('pickem_slate', FALSE, array('pid' => $this->pid));
			#$this->picks = $picks;
		}
		
		$this->sportsEvent = FALSE;
		if (!empty($this->event_entity_type)) {
			$sportsevents = entity_load($this->event_entity_type, array($this->event_entity_id));
			$this->sportsEvent = reset($sportsevents);
		}
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
	
	public function getSportsEvent() {
		return $this->sportsEvent;
	}
	
	public function displayTitle() {
		return $this->sportsEvent->displayTitle();
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
		return $this->picks[$uid]->pick_team_name;
	}
}

class PickemSlateUIController extends EntityDefaultUIController {
	
}

class PickemSlateController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
}

class Pick 
	extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'pick');
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
 * PickemPlayer class
 */
class PickemPlayer 
	extends Entity {
	
	public function load($ids = array(), $conditions = array()) {
		$player = parent::load($ids, $conditions);
		$user = user_load(array($uid));
		if (!empty($user)) {
			$player->name = $user->name;
		}
		return $player;
	}
}
	
class PickemPlayerUIController extends EntityDefaultUIController {
	
}

class PickemPlayerController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}
