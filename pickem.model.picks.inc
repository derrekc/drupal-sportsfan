<?php

class PickemModel {
	public $objtype = '';
	public $objid = '';
	
	public function formElementId() {
		return sprintf('%s-%s', $this->objtype, $this->objid);
	}
	
	public function displayTitle() {
		return '';
	}
}

class Pickem {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}

	public function currentWeek() {
		return 1;
	}
}

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

	public static function loadPickemPlayer($uid) {
		$pickem_player = drupal_static(__FUNCTION__);
		if (!isset($pickem)) {
			$cache_key = 'pickem.pickem_player';
			if ($cache = cache_get($cache_key)) {
				$pickem_player = $cache->data;
			}
			else {
				$dbres = db_select('pickem_player', 'pp')->fields('pp')->condition('uid', $uid)->execute();
				$pickem_player = $dbres->fetchObject();
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
}

class Slate {

}

class SlateManager {
	public static function slateForWeek($pid, $week, $season) {
		$dbres = db_select('slate', 'sl')
			->fields('sl')
			->condition('sl.week', $week)
			->condition('sl.season', $season)
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
			$class = ucfirst($row->objtype);
			$rows[] = new $class($row);
		}
		return $rows;
	}

	public static function slateForDate($date) {

	}
}

class Game extends PickemModel {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}

	public function displayTitle() {
		if (!empty($this->title)) {
			return $this->title;
		}
		return sprintf('%s %s %s',
			$this->visiting_school,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_school
			);
	}

	public function opponentsForSelectElement() {
		$options = array();
		if (!empty($this->visiting_school_name)) {
			$options[$this->visiting_school_name] = $this->visiting_school;
		}
		if (!empty($this->host_school_name)) {
			$options[$this->host_school_name] = $this->host_school;
		}
		return $options;
	}
	
	public function opponents() {
		$opponents = array();
		
		$o = new stdClass();
		$o->name = $this->visiting_school_name;
		$o->displaytitle = $this->visiting_school;
		$opponents[] = $o;
		
		$o = new stdClass();
		$o->name = $this->host_school_name;
		$o->displaytitle = $this->host_school;
		$opponents[] = $o;
		
		return $opponents;
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
		$query->fields('g');
		$query->addField('host', 'displaytitle', 'host_school');
		$query->addField('visiting', 'displaytitle', 'visiting_school');
		$query->condition('gid', $slateObject->objid);
		$dbres = $query->execute();
		$o = $dbres->fetchObject();
		foreach (get_object_vars($o) as $oKey => $oData) {
			$slateObject->{$oKey} = $o->{$oKey};
		}
	}
}

class Tourney extends PickemModel {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
	}
	
	public function opponents() {
		return $this->opponents;
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

class Series extends PickemModel {
	public function __construct($dbobj) {
		foreach (get_object_vars($dbobj) as $oKey => $oVal) {
			$this->{$oKey} = $oVal;
		}
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
