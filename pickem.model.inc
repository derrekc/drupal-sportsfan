<?php


function pickem_load_game($gid) {
	if ($gid == NULL) {
		return NULL;
	}
	$result = db_query('SELECT g.* FROM game g WHERE gid = :gid', array(':gid' => $gid));
	return $result->fetchObject();
}

function pickem_load_conferences($p5 = TRUE) {
	$query = db_select('conference', 's')->extend('PagerDefault')->extend('TableSort');
	$query->fields('s');
	if ($p5) {
		$query->condition('name', array('acc','big10','big12','pac12','sec','ind'), 'IN');
	}
	$result = $query->execute();
	$rows = array();
	while ($c = $result->fetchObject()) {
		$rows[] = $c;
	}
	return $rows;
}		

function pickem_load_weeks() {
}