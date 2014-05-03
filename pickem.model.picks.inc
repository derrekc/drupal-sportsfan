<?php

class PickemEventModel {
	public $objtype = '';
	public $objid = '';
	
	public function formElementId() {
		return sprintf('%s-%s', $this->objtype, $this->objid);
	}
	
	public function displayTitle() {
		return '';
	}
	
	public function displayTitleMobile() {
		return $this->displayTitle();
	}
	
	public function gameDate() {
		return '';
	}
	
	public function eventDate($type='medium') {
		return '';
	}
	
}

/*
class Pickem {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}

	public function currentWeek($ts=NULL) {
		return 1;
	}
	
	public function active() {
		return in_array($this->active, array(1, 'Y', 'y'));
	}
}
*/

class PickemManager {
	
	public static function loadPickem($pname) {
		$pickem = drupal_static(__FUNCTION__);
		if (!isset($pickem)) {
			$cache_key = sprintf('pickem.pickem.%s', $pname);
			if ($cache = cache_get($cache_key)) {
				$pickem = $cache->data;
			}
			else {
				$dbres = db_select('pickem', 'p')->fields('p')->condition('name', $pname)->execute();
				$pickem = new Pickem($dbres->fetchObject());
				cache_set($cache_key, $pickem, 'cache', CACHE_TEMPORARY);
			}
		}
		return $pickem;
	}

	public static function newPickem() {
		$o = new stdClass();
		$o->title = '';
		$o->name = '';
		$o->description = '';
		$o->desc_format = NULL;
		$o->autoconference = '';
		$o->autoschool = '';
		$o->season = '';
		$o->sport = '';
		$o->active = 1;
		$o->joindeadline = 0;
		$o->firstdayofweek = 0;	// sunday by default
		$o->startdate = 0;
		$o->weeks = 1;
		$o->pickformat = 'straight';
		$o->picksperweek = 0;
		
		return $o;
	}
	
	public static function activePickems() {
		$pickems = drupal_static(__FUNCTION__);
		if (!isset($pickems)) {
			$cache_key = sprintf('pickem.pickems');
			if ($cache = cache_get($cache_key)) {
				$pickems = $cache->data;
			}
			else {
				$dbres = db_select('pickem', 'p')
					->fields('p')
					->condition('p.active', 1)
					->execute();
				$pickems = $dbres->fetchAll();
				cache_set($cache_key, $pickems, 'cache', CACHE_TEMPORARY);
			}
		}	
		return $pickems;
	}

	public static function loadPickemPlayer($uid, $pid) {
		$cache_key = sprintf('pickem.pickem_player.%s.%s', $uid, $pid);
		if ($cache = cache_get($cache_key)) {
			$pickem_player = $cache->data;
		}
		else {
			$dbres = db_select('pickem_player', 'pp')->fields('pp')->condition('pickem_id', $pid)->condition('uid', $uid)->execute();
			$pickem_player = $dbres->fetchObject();
			if (!is_null($pickem_player)) {
				cache_set($cache_key, $pickem_player, 'cache', CACHE_TEMPORARY);
			}
		}
		return $pickem_player;
	}
	
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
	
	public static function addUserToPickem($user, $pickem) {
		$o = new stdClass();
		$o->uid = $user->uid;
		$o->pickem_id = $pickem->pid;
		drupal_write_record('pickem_player', $o);
	}
	
	public static function userNeedsProvisioning($pickem_id, $uid, $week) {
		$select = db_select('pick', 'pk')
			->condition('pickem_id', $pickem_id)
			->condition('uid', $uid)
			->condition('week', $week);
		$select->addExpression('COUNT(pkid)', 'pick_count');
		$dbres = $select->execute();
		$row = $dbres->fetchObject();
		return $row->pick_count == 0;
	}
	
	public static function provisionSlateForUser($pickem_id, $uid, $week, $season) {
		$dbres = db_select('slate', 's')->fields('s')
			->condition('s.pickem_id', $pickem_id)
			->execute();

		foreach ($dbres as $row) {
			$o = stdClass();
			$o->objid = $row->objid;
			$o->objtype = $row->objtype;
			$o->week = $week;
			$o->season = $season;
			$o->pickem_id = $pickem_id;
			$o->uid = $uid;
			drupal_write_record('pick', $o);
		}
	}
}

class Pick {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}
	
	public function pickedSchool() {
		// If the user (player) made an initial pick for the target game, 
		// then turned around and changed his/her mind, but failed to 'finalize' (save)
		// the pick during the session, return that value
		if (in_array('pick_school_name_session', get_object_vars($this))) {
			return $this->pick_school_name_session;
		}
		return $this->pick_school_name;
	}
}

class PickManager {
	public static function loadPicks($pickem, $uid, $targetWeek=NULL) {
		
		$targetWeek = is_null($targetWeek) ? $pickem->currentWeek() : $targetWeek;
		
		$dbres = db_select('pick', 'pk')->fields('pk')->execute();
		$picks = array();
		foreach ($dbres as $row) {
			
			// detect presence of a potential change to this pick
			if (isset($_SESSION['picks'][$uid][$row->pkid])) {
				$row->pick_school_name_session = $_SESSION['picks'][$uid][$row->pkid];
			}
			
			$picks[] = new Pick($row);
		}

		return $picks;
	}
	
	public static function addDataToSlateObject(&$slateRow) {
		$select = db_select('pick', 'pk')->fields('pk', array('pick_school_name', 'pkid', 'correct'));
		$select->join('school', 'pick_school', 'pick_school.name = pk.pick_school_name');
		$select->addField('pick_school', 'displaytitle', 'pick_school');
		$select->condition('pk.objid', $slateRow->objid);
		$select->condition('pk.objtype', $slateRow->objtype);
		$dbres = $select->execute();
			
		$pick = $dbres->fetchObject();
		$slateRow->pick = $pick;
	}
}

#class Slate {
#
#}

class SlateEvent {
	private $_sportEvent = NULL;
	private $_pick = NULL;
	
	public function __construct($sportEvent) {
		$this->_sportEvent = $sportEvent;
		
		if (isset($sportEvent->pick)) {
			$this->_pick = $sportEvent->pick;
			unset($sportEvent->pick);
		}
	}
	
	public function sportEvent() {
		return $this->_sportEvent;
	}
	
	public function sportEventType() {
		return $this->_sportEvent->objtype;
	}
	
	public function sportEventId() {
		return $this->_sportEvent->objid;
	}
	
	public function displayTitle() {
		return $this->_sportEvent->displayTitle();
	}
	
	public function displayTitleMobile() {
		return $this->_sportEvent->displayTitleMobile();
	}
	
	public function eventDate($drupalDateType='medium') {
		return $this->_sportEvent->eventDate($drupalDateType);
	}
	
	public function opponents() {
		return $this->_sportEvent->opponents();
	}
	
	public function hasPick() {
		return !is_null($this->_pick) && isset($this->_pick->pkid);
	}
	
	public function pick() {
		return $this->_pick;
	}
	
	public function pickId($noPickDefault='new') {
		return $this->hasPick() ? $this->_pick->pkid : $noPickDefault;
	}
	
	public function pick_school_name() {
		return $this->hasPick() ? $this->_pick->pick_school_name : '';
	}
	
	public function pick_school() {
		return $this->hasPick() ? $this->_pick->pick_school : '';
	}
}

class SlateManager {
	public static function slateForWeek($pid, $week, $season) {
		$dbres = db_select('slate', 'sl')
			->fields('sl')
			->condition('sl.week', $week)
			->condition('sl.season', $season)
			->condition('sl.pid', $pid)
			->execute();

		$rows = array();
		foreach ($dbres as $row) {
			if ($row->objtype == 'game') {
				GameManager::addDataToSlateObject($row);
			}
			else if ($row->objtype == 'series') {
				SeriesManager::addDataToSlateObject($row);
			}
			else if ($row->objtype == 'tourney') {
				TourneyManager::addDataToSlateObject($row);
			}
			PickManager::addDataToSlateObject($row);
			$class = ucfirst($row->objtype);
			$rows[] = new SlateEvent(new $class($row));
		}
		return $rows;
	}

	public static function slateForDate($date) {

	}
}

class GameManager {

	public static function gamesToBePicked($week, $season) {
		
		$query = db_select('slate', 's');
		$query->join('game', 'g', "g.gid = s.objid AND s.objtype = 'game'");
		$query->join('school', 'host', 'host.name = g.host_school_name');
		$query->join('school', 'visiting', 'visiting.name = g.visiting_school_name');

		$query->fields('g');
		$query->addField('host', 'displaytitle', 'host_school');
		$query->addField('visiting', 'displaytitle', 'visiting_school');

		$query->condition('s.week', $week)
			->condition('s.season', $season)
			->orderBy('g.game_date');

		$dbres = $query->execute();
		$games = $dbres->fetchAll('Game');

		return $games;
	}

	public static function addDataToSlateObject(&$slateObject) {
		$query = db_select('game', 'g');
		$query->join('school', 'host', 'host.name = g.host_school_name');
		$query->join('school', 'visiting', 'visiting.name = g.visiting_school_name');
		
		//$query->leftJoin('pick', 'p', "(p.objid = g.gid AND p.objtype='game')");
		//$query->leftJoin('school', 'pschool', 'p.pick_school_name = pschool.name');
		
		$query->fields('g');
		$query->addField('host', 'displaytitle', 'host_school');
		$query->addField('visiting', 'displaytitle', 'visiting_school');
		
		//$query->addField('pschool', 'displaytitle', 'pick_school');
		//$query->addField('p', 'pick_school_name');
		//$query->addField('p', 'pkid');
		
		$query->condition('gid', $slateObject->objid);
		$dbres = $query->execute();
		$o = $dbres->fetchObject();
		foreach (get_object_vars($o) as $oKey => $oData) {
			$slateObject->{$oKey} = $o->{$oKey};
		}
	}
}

class Tourney extends PickemEventModel {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}
	
	public function opponents() {
		return $this->opponents;
	}
	
	public function eventDate($type='medium') {
		return format_date($this->start, $type);
	}
}

class TourneyManager {
	public static function addDataToSlateObject(&$slateObject) {
		$query = db_select('tourney', 't');
		$query->leftJoin('school', 'winning', 'winning.name = t.winning_school_name');
		$query->fields('t');
		$query->addField('winning', 'name', 'winning_school');
		$query->condition('tid', $slateObject->objid);
		$dbres = $query->execute();
		$o = $dbres->fetchObject();
		foreach (get_object_vars($o) as $oKey => $oData) {
			$slateObject->{$oKey} = $o->{$oKey};
		}
		$slateObject->opponents = array();
		
		$query = db_select('tourney_school', 'ts');
		$query->join('school', 's', 's.name = ts.school_name');
		$query->fields('s', array('name', 'title', 'displaytitle'));
		$dbres = $query->execute();
		foreach ($dbres as $row) {
			$slateObject->opponents[] = $row;
		}
	}
}

class Series extends PickemEventModel {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}
	
	public function eventDate($type='medium') {
		return format_date($this->start, $type);
	}
}

class SeriesManager {
	public static function loadSeries($sid) {
		$series = drupal_static(__FUNCTION__);
		if (!isset($series)) {
			$cache_key = sprintf('pickem.%s.series', $sid);
			if ($cache = cache_get($cache_key)) {
				$series = $cache->data;
			}
			else {
				$dbres = db_select('series', 's')->fields('s')->condition('sid', $sid)->execute();
				$series = $dbres->fetchObject();
				cache_set($cache_key, $series, 'cache', CACHE_TEMPORARY);
			}
		}
		return $series;
	}

	public static function addDataToSlateObject(&$slateObject) {

	}

}
