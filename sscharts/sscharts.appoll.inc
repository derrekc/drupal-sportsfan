<?php

function sscharts_appoll_tree() {
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

	$conferences = array();
	foreach ($result as $row) {
		if (!in_array($row->conf_title, $conferences)) {
			$conferences[] = $row->conf_title;
		}
		
		//$o->children[] = $row;
		
		$rows[] = array(
			'conf' => $row->school_title, 
			'parent' => $row->conf_title, 
			'totalpoints' => (int) $row->total_points, 
			'weeks_in_poll' => array_search($row->conf_title, $conferences),
		);
	}
		
	asort($conferences);
	foreach (array_reverse($conferences) as $c) {
		array_unshift($rows, array('conf' => $c, 'parent' => 'FBS', 'totalpoints' => 0, 'weeks_in_poll' => 0));
	}
	array_unshift($rows, array('conf' => 'FBS', 'parent' => null, 'totalpoints' => 0, 'weeks_in_poll' => 0));
	
	//array_unshift($rows, array('SCHOOL', 'CONFERENCE', 'TOTAL POINTS', 'WEEKS IN POLL'));	
	
	$options = array(
		'title' => t('2013 AP Poll Tree Map'),
		'fields' => array(
			'conf' => array('label' => t('Conference'),),
			'parent' => array('label' => t('Parent'),),
			'totalpoints' => array('label' => 'Total Points Received', 'enabled' => TRUE), 
			'weeks_in_poll' => array('label' => 'Weeks In Poll', 'enabled' => TRUE)
		),
		'xAxis' => array('labelField' => array('conf', 'parent')),
		'height' => 500,
	);
	$options['data'] = $rows;
	$options['type'] = 'treemap';
	
  return array(
    '#theme' => 'visualization',
    '#options' => $options,
  );
	
	//return drupal_json_output($rows);
	
}	

function sscharts_appoll_chart() {
	$query = db_select('sportspoll_tally', 't');
	$query->fields('t');
	$query->join('school', 's', 's.name = t.school_name');
	$query->addField('s', 'displaytitle', 'school_title');
	$query->condition('s.name', array('alabama','auburn'));
	$query->orderBy('t.week');
	$result = $query->execute();
	
	
	$options = array(
		'title' => t('2013 AP Poll Movement - Alabama vs Auburn'),
		'fields' => array(
			
			'week' => array(
				'label' => t('Week'),
			),
			'alabama' => array('label' => 'Alabama', 'enabled' => TRUE), 
			'auburn' => array('label' => 'Auburn', 'enabled' => TRUE)
		),
		'xAxis' => array('labelField' => 'week'),
		'height' => 500,
		'series' => array(
			array('color' => '#990030','lineWidth' => 3,),
			array('color' => '#000059','lineWidth' => 3,),
		),
	);
	
	$school_points = array();
	$data[] = array();
	
	$currentweek = 0;
	$i = -1;
	foreach ($result as $o) {
		if (!key_exists($o->week, $school_points)) {
			$school_points[$o->week] = array('alabama' => 0, 'auburn' => 0);
		}
		$school_points[$o->week][$o->school_name] = $o->points;
	}
	
	foreach ($school_points as $week => $row) {
		$line = array('week' => $week);
		foreach ($row as $skey => $points) {
			$line[$skey] = $points;
		}
		$data[] = $line;
	}
	array_shift($data);
	$options['data'] = $data;
	$options['type'] = 'line';
	
  return array(
    '#theme' => 'visualization',
    '#options' => $options,
  );
	

}
