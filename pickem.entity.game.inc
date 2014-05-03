<?php

class Game extends Entity {
	
	public function __construct($values = array()) {
		watchdog('pickem', 'in constructor');
		parent::__construct($values, 'pickem');
	}

	public function currentWeek($ts=NULL) {
		return 1;
	}
	
	public function active() {
		return in_array($this->active, array(1, 'Y', 'y'));
	}
}

class GameUIController extends EntityDefaultUIController {
	
}

class GameAPIController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
}
