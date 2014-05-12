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
		$this->sportEvent = reset($games);
		
		$pickController = entity_get_controller('pick');
	}
	
	public function load_pick_for_user($uid) {
		// TODO convert 'pick' into an entity-based object, then..
		// $pick = entity_load('pick', FALSE, array('uid' => $uid));
		// return reset($pick);
		return FALSE;
	}
	
	public function displayTitle() {
		return $this->sportEvent->displayTitle();
	}
	
	public function displayTitleMobile() {
		return $this->sportEvent->displayTitleMobile();
	}
	
	public function eventDate($drupalDateType='medium') {
		return $this->sportEvent->eventDate($drupalDateType);
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
		return parent::load($ids, $conditions);
	}
}
	
class PickemPlayerUIController extends EntityDefaultUIController {
	
}

class PickemPlayerController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}
