<?php

function pickem_d3_appoll() {
	$query = db_select('school', 's');
	$query->addExpression('count(t.school_name)', 'weeks_in_poll');
	$query->addExpression('sum(points)', 'total_points');
	$query->addExpression('min(rank)', 'highest_rank');
	$query->join('sportspoll_tally', 't', 't.school_name = s.name');
	$query->join('conference', 'c', 's.conf_name = c.name');
	$query->addField('s', 'displaytitle', 'school_title');
	$query->addField('s', 'conf_name');
	$query->addField('c', 'displaytitle', 'conf_title');
	$query->condition('t.rank', '25', '<=');
	$query->condition('s.conf_name', 'sec');
	$query->groupBy('school_title');
	$result = $query->execute();
	
	$rows = array(
		array('SCHOOL', 'HIGHEST RANK', 'WEEKS IN POLL', 'CONFERENCE', 'TOTAL POINTS'),
	);
	//$o = new stdClass();
	//$o->children = array();
	//$rows = array();
	foreach ($result as $row) {
		//$o->children[] = $row;
		$rows[] = array($row->school_title, (int) $row->highest_rank, (int) $row->weeks_in_poll, $row->conf_title, (int) $row->total_points);
	}
	
	return drupal_json_output(array_reverse($rows));
}

function pickem_d3_appoll_tree() {
	$query = db_select('school', 's');
	$query->addExpression('count(t.school_name)', 'weeks_in_poll');
	$query->addExpression('sum(points)', 'total_points');
	$query->addExpression('min(rank)', 'highest_rank');
	$query->join('sportspoll_tally', 't', 't.school_name = s.name');
	$query->join('conference', 'c', 's.conf_name = c.name');
	$query->addField('s', 'displaytitle', 'school_title');
	$query->addField('s', 'conf_name');
	$query->addField('c', 'displaytitle', 'conf_title');
	$query->condition('s.conf_name', array('acc','sec','big10','big12','aac','ind','pac12',));
	//$query->condition('t.rank', '25', '<=');
	//$query->condition('s.conf_name', 'sec');
	$query->groupBy('school_title');
	$result = $query->execute();
	
	$rows = array();
	//$o = new stdClass();
	//$o->children = array();
	//$rows = array();
	$conferences = array();
	foreach ($result as $row) {
		if (!in_array($row->conf_title, $conferences)) {
			$conferences[] = $row->conf_title;
		}
		//$o->children[] = $row;
		$rows[] = array(
			$row->school_title, 
			$row->conf_title, 
			(int) $row->total_points, 
			//(int) $row->weeks_in_poll,
			array_search($row->conf_title, $conferences),
		);
	}
		
	asort($conferences);
	foreach (array_reverse($conferences) as $c) {
		array_unshift($rows, array($c, 'FBS', 0, 0));
	}
	array_unshift($rows, array('FBS', null, 0, 0));
	array_unshift($rows, array('SCHOOL', 'CONFERENCE', 'TOTAL POINTS', 'WEEKS IN POLL'));	
	
	return drupal_json_output($rows);
}
