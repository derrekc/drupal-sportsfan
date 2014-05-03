<?php

class Pickem extends Entity {
	
	public function __construct($values = array()) {
		parent::__construct($values, 'pickem');
		$pickem_slate = entity_load('pickem_slate', FALSE, array('pid' => $this->pid, 'week' => $this->currentWeek()));
		$this->pickem_slate = $pickem_slate;
	}

	public function currentWeek($ts=NULL) {
		return 1;
	}
	
	public function active() {
		return in_array($this->active, array(1, 'Y', 'y'));
	}
}

class PickemUIController extends EntityDefaultUIController {
	
}

class PickemAPIController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
}

class PickemSlate extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'pickem_slate');
		$games = entity_load($this->event_entity_type, array($this->event_entity_id));
		$this->game = reset($games);
	}
	
	public function load_pick_for_user($uid) {
		// TODO convert 'pick' into an entity-based object, then..
		// $pick = entity_load('pick', FALSE, array('uid' => $uid));
		// return reset($pick);
		return FALSE;
	}
}

class PickemSlateUIController extends EntityDefaultUIController {
	
}

class PickemSlateController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
}

class Game extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'game');

		// TODO load values for home_school (string) and 
		// visiting_school (string)
		$host_school_entity = entity_load('team', FALSE, array('name' => $this->host_school_name));
		$this->host_school = reset($host_school_entity);

		$visiting_school_entity = entity_load('team', FALSE, array('name' => $this->visiting_school_name));
		$this->visiting_school = reset($visiting_school_entity);

	}

	public function getDisplayTitle() {
		return $this->displayTitle();
	}
	
	public function displayTitle() {
		if (!empty($this->title)) {
			return $this->title;
		}
		return sprintf('%s %s %s',
			$this->visiting_school_name,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_school_name
			);
	}

	public function displayTitleMobile() {
		if (!empty($this->title)) {
			return $this->title;
		}
		return sprintf('%s %s<br />%s',
			$this->visiting_school,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_school
			);
	}
		
	public function vs_at_label() {
		return $this->neutral == 'Y' ? 'vs.' : 'at';
	}
	
	public function eventDate($type='medium') {
		return $this->gameDate($type);
	}
	
	public function gameDate($type='medium') {
		return format_date($this->game_date, $type);
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
		
		/**
		 * TODO include rank of school
		 */
		$o = new stdClass();
		$o->name = $this->visiting_school_name;
		$o->displaytitle = $this->visiting_school;
		$opponents['visitor'] = $o;
		
		$o = new stdClass();
		$o->name = $this->host_school_name;
		$o->displaytitle = $this->host_school;
		$opponents['host'] = $o;
		
		return $opponents;
	}

}

class GameUIController extends EntityDefaultUIController {}

class GameAPIController extends EntityAPIController {}

