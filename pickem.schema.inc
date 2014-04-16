<?php

function pickem_schema_stats() {
	return array();
}

function pickem_schema_school_standings() {
	$schema = array(
		'school_standings' => array(
			'fields' => array(
				'sid' => array('type' => 'int','not null' => TRUE,'unsigned' => TRUE,),
				'school_name' => array('type' => 'varchar','length' => 35,'not null' => TRUE,'default' => '',),
				'season' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'not null' => TRUE,),
				'sport' => array('type' => 'varchar','size' => 12,'not null' => TRUE,'default' => '',),
				'wins' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'losses' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'ties' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'conf_wins' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'conf_losses' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'conf_ties' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'div_wins' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'div_losses' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'div_ties' => array('type' => 'int', 'size' => 'tiny', 'unsigned' => TRUE, 'default' => 0,),
				'pf' => array('type' => 'int', 'size' => 'medium', 'unsigned' => TRUE, 'default' => 0,),
				'pa' => array('type' => 'int', 'size' => 'medium', 'unsigned' => TRUE, 'default' => 0,),
				'conf_pf' => array('type' => 'int', 'size' => 'medium', 'unsigned' => TRUE, 'default' => 0,),
				'conf_pa' => array('type' => 'int', 'size' => 'medium', 'unsigned' => TRUE, 'default' => 0,),
				'div_pf' => array('type' => 'int', 'size' => 'medium', 'unsigned' => TRUE, 'default' => 0,),
				'div_pa' => array('type' => 'int', 'size' => 'medium', 'unsigned' => TRUE, 'default' => 0,),
			),
			'primary key' => array('sid','season','sport'),
			'indexes' => array(
				'sport' => array(array('sport', 5)),
				'school_name' => array(array('school_name', 6)),
			),
		)
	);
}
