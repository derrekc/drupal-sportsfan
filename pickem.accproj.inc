<?php

function pickem_scenario_details($school_name, $how, $season = 2013) {
	$query = db_select('slate', 's')->extend('PagerDefault')->extend('TableSort');

	$query = db_select('outcome_scenario', 'o')->extend('PagerDefault')->extend('TableSort');
	$query->fields('o')
		->condition('winner', $school_name)
		->condition('how', $how)
		->limit(5);
	
	$result = $query->execute();
		
	$header = array(
		t(''),
		t('Scenario'),
		t('Teams Tied'),
	);
	
	$rows = array();
	foreach ($result as $scene) {
		$row = array();
		$scene_text = join('<br />', explode(',', $scene->scene_text));
		$row[] = $scene->scenario;
		$row[] = $scene_text;
		if ($how == 'alone-in-first') {
			$row[] = 'N/A';
		} else {
			$teams = explode(', ', $scene->teams_in_first);
			$teams_in_first = array();
			foreach ($teams as $t) {
				$teams_in_first[] = '<img src="/sites/all/modules/pickem/logos/ncaaf/' . $t . '.gif" title="' . $t . '" width="48"/>';
			}
			$row[] = join('&nbsp;', $teams_in_first);
		}
		$rows[] = $row;
	}
	
	$build['title'] = array(
		'#type' => 'markup',
		'#markup' => '<h3>' . $school_name . ' | ' . $how . '</h3>',
	);
	$build['scene_details'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array('class' => array('table', 'table-condensed')),
	);
	$build['pager'] = array(
		'#theme' => 'pager',
	);
	return $build;
}
