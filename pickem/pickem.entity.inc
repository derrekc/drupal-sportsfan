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
			$this->pickem_slate_ids = array_keys($result['pickem_slate']);
		}
		
		// Load pickem players
		$pickem_players = entity_load('pickem_player', FALSE, array('pickem_id' => $this->pid), TRUE);
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
		$this->player_results = array();
		
		foreach ($weeks as $week) {
			$this->loadWeek($week);
			
			// load player results
			foreach ($pickem_players as $player) {
				#if ($player->weekMetadataForPickem($this->pid) &&
				#	$player->submittedPicksForWeek($this->pid, $week->week)) {
				
					if (!isset($this->player_results[$week->week])) {
						$this->player_results[$week->week] = array();
						$this->player_results_by_uid[$week->week] = array();
					}
					$pickem_result_for_week = $player->pickemResultForWeek($this->pid, $week->week);
					if ($pickem_result_for_week !== FALSE) {
						$this->player_results[$week->week][] = $pickem_result_for_week;
						$this->player_results_by_uid[$week->week][$player->uid] = $pickem_result_for_week;
					}
					// $this->player_results[$week->week][] = $player->pickemResultForWeek($this->pid, $week->week);
				#}
			}
		}
		$current_week = $this->currentWeek();
		$this->playersByUID = array();
		foreach ($pickem_players as $player) {
			$this->playersByUID[$player->uid] = $player;
			
			$player->weeks_played = 0;
			
			$week_metadata = $player->weekMetaData($this->pid);
			if ($week_metadata !== FALSE) {			
				$count = count($week_metadata);
				if (in_array($current_week, array_keys($week_metadata))) {
					$count--;
				}
				$player->weeks_played = $count;
			}
		#	if ($player->weekMetadataForPickem($pickem) && $p)
		}

		$this->loadStandings();
		//dpm($this->player_results);
	}
	
	protected function loadWeek($weekObject) {
		
		$query = new EntityFieldQuery();
		$query
			->entityCondition('entity_type', 'pickem_slate', '=')
			->propertyCondition('pid', $this->pid, '=')
			->propertyCondition('week', $weekObject->week, '=')
			->propertyOrderBy('slate_date', 'ASC');
			
		$result = $query->execute();
		
		if (!empty($result['pickem_slate'])) {
			$slids = array_keys($result['pickem_slate']);
		  $pickem_slate = entity_load('pickem_slate', $slids, FALSE, TRUE);
			$weekObject->pickem_slate = $pickem_slate;
		}
		
		$this->slate_weeks[$weekObject->week] = $weekObject;
	}	

	public function playerByUid($uid) {
		if (!isset($this->playersByUID[$uid])) {
			return FALSE;
		}
		return $this->playersByUID[$uid];
	}
	
	public function playersWithSubmittedPicksForWeek($week=FALSE) {
		if ($week === FALSE) {
			$week = $this->currentWeek();
		}
		
		$players_with_submitted_picks = array();
		foreach ($this->players as $uid => $player) {
			if ($player->submittedPicksForWeek($this->pid, $week)) {
				$players_with_submitted_picks[$uid] = $player;
			}
		}	
		return $players_with_submitted_picks;
	}
	
	public function playerResultsForWeek($week = FALSE) {
		if (empty($this->player_results)) {
			return FALSE;
		}
		
		if (!$week) {
			$week = $this->currentWeek();
		}

		if (!isset($this->player_results[$week])) {
			return FALSE;
		}
		
		# sort the results by 'correct'
		$fn = function ($a, $b) {
			if ($a->correct == $b->correct) {
				 return 0;
			}
			return $a->correct > $b->correct ? -1 : 1;
		};
		uasort($this->player_results[$week], $fn);
		return $this->player_results[$week];
	}
	
	/**
	 * @param $ts Timestamp (optional)
	 * @return mixed Current week as integer, otherwise FALSE
	 */
	public function currentWeek($ts=NULL) {
		if ($ts == NULL) {
			$ts = time();
		}
		foreach ($this->slate_weeks as $week) {
			if ($week->end > $ts) {
				return $week->week;
			}
		}
		return FALSE;
	}
	
	public function howManyWeeksMissed() {
		return $this->currentWeek() - 1;
	}
	
	public function currentWeekObject($ts=NULL) {
		if ($ts == NULL) {
			$ts = time();
		}
		foreach ($this->slate_weeks as $week) {
			if ($week->start > $ts) {
				return $week;
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
		$fn = function($a, $b) {
			if ($a->sportsEvent->event_date == $b->sportsEvent->event_date) {
				return 0;
			}
			return $a->sportsEvent->event_date < $b->sportsEvent->event_date ? -1 : 1;
		};
		uasort($this->slate_weeks[$week]->pickem_slate, $fn);
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
	
	public function getSlateWeek($week_no) {
		return $this->slate_weeks[$week_no];
	}
	
	public function getSlateWeeks() {
		return $this->slate_weeks;
	}
	
	public function defaultPickIndicator($player) {
		$pick_default = '';
		
		$pickem_wrapper = entity_metadata_wrapper('pickem', $this);
		$max_bye_weeks_allowed = $pickem_wrapper->max_bye_weeks_allowed->value();
		$pick_default_pickem = $pickem_wrapper->pick_default->value();
		
		$player = pickem_player_load($player->uid);
	
		$player_user = user_load($player->uid);
		$user_wrapper = entity_metadata_wrapper('user', $player_user);
		$pick_default_user = $user_wrapper->pick_default->value();
	
		if (!empty($pick_default_user)) {
			$pick_default = $pick_default_user;
		}
		else {
			$pick_default = $pick_default_pickem;
		}
		return $pick_default;		
	}
	
	public function determineDefaultPickTeamForPlayer($player, $slateEvent) {
		$pickem_wrapper = entity_metadata_wrapper('pickem', $this);
		$max_bye_weeks_allowed = $pickem_wrapper->max_bye_weeks_allowed->value();
		$pick_default_pickem = $pickem_wrapper->pick_default->value();
		
		$player = pickem_player_load($player->uid);
	
		$player_user = user_load($player->uid);
		$user_wrapper = entity_metadata_wrapper('user', $player_user);
		$pick_default_user = $user_wrapper->pick_default->value();
	
		if (!empty($pick_default_user)) {
			$pick_default = $pick_default_user;
		}
		else {
			$pick_default = $pick_default_pickem;
		}
		
		if ((int)$player->metadataValue('bye_count', $this->pid) > $max_bye_weeks_allowed) {
			return '';
		}
		
		if ($pick_default == 'favorite') {
			if ($slateEvent->sportsEvent->hasFavorite()) {
				return $slateEvent->sportsEvent->favorite;
			} else {
				// determine team with highest ranking
				return  $slateEvent->sportsEvent->host_team_name;
			}
		} else if ($pick_default == 'visitingteam') {
			return $slateEvent->sportsEvent->visiting_team_name;
		} else {
			return $slateEvent->sportsEvent->host_team_name;
		}
		
		return '';
	}

	public function addSportsEventToSlate($sportsEvent, $week) {
		$values = array(
			'pid' => $this->pid,
			'event_entity_type' => 'sportsevent',
			'event_entity_id' => $sportsEvent->eid,
			'season' => $sportsEvent->season,
			'week' => $week,
			'slate_date' => $sportsEvent->event_date,
		);
		$slate = entity_create('pickem_slate', $values);
		if (entity_save('pickem_slate', $slate) === FALSE) {
			return FALSE;
		}

		return $slate;
	}
	
	public function currentStandings() {
		if (empty($this->standings)) {
			return FALSE;
		}
		$copy_of_standings = $this->standings;
		return array_shift($copy_of_standings);
	}
	
	public function loadStandings($week = FALSE) {
		$results = db_select('standings', 's')
			->fields('s')
			->condition('pickem_id', $this->pid)
			->orderBy('week', 'DESC')
			->orderBy('correct', 'DESC')
			->orderBy('playername', 'ASC')
			->execute();
		
		$this->standings = array();
		$this->standingsByPlayerName = array();
		$this->player_rank = array();
		
		$lastweek = 0;
		$lastcorrect = -1;
		$rank = 0;
		
		foreach ($results as $s) {
			if ($s->week != $lastweek) {
				$lastweek = $s->week;
				$rank = 0;
				$lastcorrect = -1;
				$this->standings[$lastweek] = array();
				$this->player_rank[$lastweek] = array();
			}
			if ($lastcorrect != $s->correct) {
				$rank++;
				$lastcorrect = $s->correct;
			}
			$s->rank = $rank;
			$this->standings[$lastweek][] = $s;
			$this->standingsByPlayerName[$lastweek][$s->playername] = array(
				'overview-label' => sprintf("%s-%s (%s)", $s->correct, $s->incorrect, $rank),
				'correct' => $s->correct,
				'incorrect' => $s->incorrect,
				'rank' => $s->rank,
			);
			$this->player_rank[$lastweek][$s->playername] = $rank; 
		}
	}
	
	public function standings($week=FALSE) {
		if (!isset($this->standings)) {
			return FALSE;
		}
		if (!$week) {
			$result = db_query('SELECT MAX(week) as week FROM {standings} WHERE pickem_id = :pid', array(':pid' => $this->pid));
			$o = $result->fetchObject();
			$week = $o->week;
		}
		if (!isset($this->standings[$week])) {
			return FALSE;
		}
		return $this->standings[$week];
	}
	
	public function standingsByPlayerName($playername, $week=FALSE) {
		if (!isset($this->standings)) {
			return FALSE;
		}
		if (!$week) {
			$result = db_query('SELECT MAX(week) as week FROM {standings} WHERE pickem_id = :pid', array(':pid' => $this->pid));
			$o = $result->fetchObject();
			$week = $o->week;
		}
		if (!isset($this->standingsByPlayerName[$week])) {
			return FALSE;
		}
		if (!isset($this->standingsByPlayerName[$week][$playername])) {
			return FALSE;
		}
		return $this->standingsByPlayerName[$week][$playername];
	}
	
	public function playerRank($playername, $week = FALSE) {
		if (!isset($this->player_rank)) {
			return FALSE;
		}
		if (!$week) {
			$result = db_query('SELECT MAX(week) as week FROM {standings} WHERE pickem_id = :pid', array(':pid' => $this->pid));
			$o = $result->fetchObject();
			$week = $o->week;
		}
		if (!isset($this->player_rank[$week])) {
			return FALSE;
		}
		return $this->player_rank[$week][$playername];
	}
}

class PickemUIController extends EntityDefaultUIController {
	
}

class PickemAPIController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
	public function addPlayer($uid, $uname, $pickem_id, $status=1) {
		
		$bundle = 'pickem_player';
		$entity = entity_create('pickem_player', array('type' => $bundle));
		$entity->uid = $uid;
		$entity->pickem_id = $pickem_id;
		$entity->name = $uname;
		$entity->status = $status;
		if ($entity->save()) {
			$pickemPlayerController = entity_get_controller('pickem_player');
			$pickemPlayerController->addMetaData($entity, $pickem_id, 'bye_count', 0);
			return $entity;
		}
		return FALSE;
	}
	
	public function processSportsEventResults($pickem, $sportsEvent) {
		// TODO find the PickemSlate entities attached to the sportsevent->eid value
		$pickemPlayerResultController = entity_get_controller('pickem_player_result');

		$conditions = array('pid' => $pickem->pid, 'event_entity_id' => $sportsEvent->eid, 'event_entity_type' => 'sportsevent');

		$pickemSlate = entity_load('pickem_slate', FALSE, $conditions, TRUE);
		$pickemSlate = reset($pickemSlate);

		$response = array('correct' => array(), 'incorrect' => array(), 'processed' => 0,);
		
		if (count($pickemSlate->picks)) {
			foreach ($pickemSlate->picks as $player_uid => $pick) {

				// create or locate the PickemPlayerResult for this player
				$playerResult = $pickemPlayerResultController->pickemResultForPlayer($pickem->pid, $player_uid, $pick->playername, $sportsEvent->week);

				$pick->correct = ($sportsEvent->winning_team_name) == $pick->pick_team_name ? 'Y' : 'N';
				if ($pick->correct == 'Y') {
					$response['correct'][] = sprintf("%s correctly picked winner (%s)", $pick->playername, $pick->pick_team_name);
					$playerResult->correct++;
					// TODO implement pickem_player_results here
				} else {
					$response['incorrect'][] = sprintf("%s incorrectly picked winner (%s)", $pick->playername, $pick->pick_team_name);
					$playerResult->incorrect++;
				}
				
				$pick->save();
				$playerResult->save();
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
				if ($p->pick_team_name == '') { continue; }
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
			if ($this->sportsEvent->host_team) {
				$host_key = $this->sportsEvent->host_team->name;
				$this->$host_key = $this->sportsEvent->host_team->displaytitle;
			}
			if ($this->sportsEvent->visiting_team) {
				$visitor_key = $this->sportsEvent->visiting_team->name;
				$this->$visitor_key = $this->sportsEvent->visiting_team->displaytitle;
			}
			
			if (!isset($this->sportsEvent->belongsToSlate)) {
				$this->sportsEvent->belongsToSlate = array();
			}
			if (!in_array($this->slid, $this->sportsEvent->belongsToSlate)) {
				$this->sportsEvent->belongsToSlate[] = $this->slid;
			}
		}
		
		#$this->comments = entity_load('pick_comment', FALSE, array('slid' => $this->slid), TRUE);
		$this->comments = FALSE;
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
	
	public function picksAreEvenlySplit() {
		if (count(array_keys($this->picks_by_team)) <= 1) {
			return FALSE;
		}
		$pick_count = count($this->picks);
		$picks = array_keys($this->picks_by_team);
		$pct1 = (int) number_format( (count($this->picks_by_team[$picks[0]]) / $pick_count) * 100, 0 );
		$pct2 = (int) number_format( (count($this->picks_by_team[$picks[1]]) / $pick_count) * 100, 0 );
		if (abs($pct1 - $pct2) <= 10) {
			return TRUE;
		}
		return FALSE;
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
		$slate_date = (int) $slateEvent->sportsEvent->event_date;
		
		$dayOfWeek = date('D', $slate_date);
		if (in_array($dayOfWeek, array('Tue', 'Wed', 'Thu', 'Fri'))) {
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
				if ($slateEvent->sportsEvent->sport == 'ncaaf') {
					$saturdayAtNoon -= (86400 * 2);
				}
				else {
					return $slate_date - (15 * 60);
				}
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
		$slate_date = (int) $slateEvent->sportsEvent->event_date;
		
		if ($time > $slate_date) {
			return FALSE;
		}
		
		$dayOfWeek = date('D', $slate_date);
		if (in_array($dayOfWeek, array('Tue', 'Wed', 'Thu', 'Fri'))) {
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
				if ($slateEvent->sportsEvent->sport == 'ncaaf') {
					$saturdayAtNoon -= (86400 * 2);
				}
				else {
					$delta = $slate_date - ($time + (15 * 60));
					return $delta > 0;
				}
			}
			$delta = $saturdayAtNoon - ($time + (15 * 60));
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

	public function correct() {
		return strtoupper($this->correct) == 'Y';
	}
}

class PickUIController extends EntityDefaultUIController {
	
}

class PickController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
	public function addPick($slate_id, $pick_team_name, $playername, $uid) {
		$pick = entity_create('pick', 
			array(
				'slid' => $slate_id, 
				'pick_team_name' => $pick_team_name,
				'playername' => $playername,
				'uid' => $uid,
			)
		);
		if ($pick->save() === FALSE) {
			return FALSE;
		}
		return $pick;
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

		if (empty($this->uid)) {
			return;
		}
		
    $query = new EntityFieldQuery();
    $query
      ->entityCondition('entity_type', 'pickem_player_result', '=')
      ->propertyCondition('uid', $this->uid, '=')
      ->propertyOrderBy('week');
      
    $result = $query->execute();
    $this->pickem_results = array();
    if (!empty($result['pickem_player_result'])) {
      $rids = array_keys($result['pickem_player_result']);
      $pickem_player_results = entity_load('pickem_player_result', $rids, FALSE, TRUE);

      foreach ($pickem_player_results as $pickem_player_result) {
        if (!isset($this->pickem_results[$pickem_player_result->pickem_id])) {
          $this->pickem_results[$pickem_player_result->pickem_id] = array();
        }
        $this->pickem_results[$pickem_player_result->pickem_id][$pickem_player_result->week] = $pickem_player_result;
      }
    }

		// fetch data from metadata tables
		$overall_metadata = db_select('pickem_player_metadata', 'pm')
			->fields('pm')
			->condition('playername', $this->name)
			->execute();
		$this->metadata = array();
		foreach ($overall_metadata as $om) {
			$this->metadata[$om->pickem_id][$om->metadata_key] = $om->metadata_value;
		}
		
		$week_metadata = db_select('pickem_player_week_metadata', 'pwm')
			->fields('pwm')
			->condition('playername', $this->name)
			->execute();
		$this->week_metadata = array();
		foreach ($week_metadata as $wm) {
			if (!isset($this->week_metadata[$wm->pickem_id])) {
				$this->week_metadata[$wm->pickem_id] = array('weeks' => array());
			}
			//$this->week_metadata[$wm->pickem_id] = $wm;
			$this->week_metadata[$wm->pickem_id]['weeks'][$wm->week] = $wm;
		}
	}
	
	public function pickemResultForWeek($pickem_id, $week) {
		if (!isset($this->pickem_results[$pickem_id][$week])) {
			return FALSE;
		}
		return $this->pickem_results[$pickem_id][$week];
	}
	
	public function weekMetadataForPickem($pickem_id) {
		if (!isset($this->week_metadata[$pickem_id])) {
			return FALSE;
		}
		return $this->week_metadata[$pickem_id];
	}
	
	public function submittedPicksForWeek($pickem_id, $week) {
		if (!isset($this->week_metadata[$pickem_id])) {
			return FALSE;
		}
		return isset($this->week_metadata[$pickem_id]['weeks'][$week]);
	}

	public function weekMetaData($pickem_id) {
		if (!isset($this->week_metadata[$pickem_id])) {
			return FALSE;
		}
		return $this->week_metadata[$pickem_id]['weeks'];
	}
	
	public function submittedWeeksCount($pickem_id) {
		if (!isset($this->week_metadata[$pickem_id])) {
			return 0;
		}
		if (!isset($this->week_metadata[$pickem_id]['weeks'])) {
			return 0;
		}
		return count($this->week_metadata[$pickem_id]['weeks']);
	}

	public function metadataValue($key, $pickem_id, $default = '') {
		if (empty($key) || empty($pickem_id)) {
			return $default;
		}
		
		if (!isset($this->metadata[$pickem_id])) {
			return $default;
		}
		
		if (!isset($this->metadata[$pickem_id][$key])) {
			return $default;
		}
		
		return $this->metadata[$pickem_id][$key];
	}
}
	
class PickemPlayerUIController extends EntityDefaultUIController {
	
}

class PickemPlayerController extends EntityAPIController {
		
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
	/**
	 * Add metadata to this player (typically, 'bye_count')
	 * 
	 * @param Player $player
	 * @param int $pickem_id ID of the pickem instance
	 * @param $key Metadata key
	 * @param $value Metadata value
	 * @param boolean $testOnly Don't save anything (optional)
	 * @return void
	 * @throws Exception
	 */
	public function addMetaData(PickemPlayer $player, $pickem_id, $key, $value, $testOnly=FALSE) {
		if ($testOnly) {
			return sprintf("addMetaData recevied playerName: %s; pickem_id: %s, key: %s, value: %s", 
				$player->name, $pickem_id, $key, $value);
		}
		try {
			db_merge('pickem_player_metadata')
				->key(array('playername' => $player->name, 'pickem_id' => $pickem_id, 'metadata_key' => $key))
				->fields(array(
					'playername' => $player->name,
					'pickem_id' => $pickem_id,
					'metadata_key' => $key,
					'metadata_value' => $value,
				))
				->execute();
		} catch (Exception $e) {
			throw $e;
		}
		return TRUE;
	}
	
	public function addWeekMetaData($player, $pickem_id, $week, $lastPickSave, $testOnly=FALSE) {
		if ($testOnly) {
			return sprintf("addWeekMetaData received playerName: %s; pickem_id: %s, key: %s, value: %s",
				$player->name, $pickem_id, $key, $value);
		}
		try {
			db_merge('pickem_player_week_metadata')
				->key(array('playername' => $player->name, 'pickem_id' => $pickem_id, 'week' => $week))
				->fields(array(
					'playername' => $player->name,
					'pickem_id' => $pickem_id,
					'week' => $week,
					'last_picksave' => $lastPickSave,
				))
				->execute();
		} catch (Exception $e) {
			throw $e;
		}
		return TRUE;
	}
}

class PickemPlayerResult
	extends Entity {
	
	public function __construct($values = array()) {
		parent::__construct($values, 'pickem_player_result');
	}	
}
	
class PickemPlayerResultUIController extends EntityDefaultUIController {
	
}

class PickemPlayerResultController extends EntityAPIController {
		
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
	public function pickemResultForPlayer($pickem_id, $uid, $playername, $week) {
		$pickem_player_result = FALSE;

		$query = new EntityFieldQuery();
		$query->entityCondition('entity_type', 'pickem_player_result')
			->propertyCondition('uid', $uid)
			->propertyCondition('pickem_id', $pickem_id)
			->propertyCondition('week', $week)
			->propertyCondition('playername', $playername);

		$result = $query->execute();
		if (isset($result['pickem_player_result'])) {
			$ids = array_keys($result['pickem_player_result']);
			$pickem_player_result = entity_load('pickem_player_result', $ids);
			$pickem_player_result = reset($pickem_player_result);
		}
		else {
			// create a new one
			$pickem_player_result = entity_create('pickem_player_result', array('bundle' => 'pickem_player_result'));
			$pickem_player_result->uid = $uid;
			$pickem_player_result->playername = $playername;
			$pickem_player_result->week = $week;
			$pickem_player_result->pickem_id = $pickem_id;
			$pickem_player_result->save();
		}
		return $pickem_player_result;
	}
}
	
class Standings
	extends Entity {

	public static function newStandingsRowForPickemPlayer($pickem, $player, $week) {
		$entity = entity_create('standings', array('type' => 'standings'));
		$entity->pickem_id = $pickem->pid;
		$entity->uid = $player->uid;
		$entity->playername = $player->name;
		$entity->week = $week;
		$entity->season = $pickem->season;
		return $entity;
	}
	
	public function __construct($values = array()) {
		parent::__construct($values, 'standings');
	}
}

class StandingsController extends EntityAPIController {
		
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}
