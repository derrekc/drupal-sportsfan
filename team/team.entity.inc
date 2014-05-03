<?php

class Team extends Entity {
	
	public $title;
	public $displaytitle;
	public $league;
	public $active;

	public function __construct($values = array()) {
		parent::__construct($values, 'team');
	}
}	
