<?php

class Pick 
	extends Entity {

	public function __construct($values = array()) {
		parent::__construct($values, 'pick');
	}

}
	
class PickUIController extends EntityDefaultUIController {
	
}

class PickController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}
	