<?php

/**
 * page callback for path 'fan'
 */
function sportsfan_home() {
	return 'Coming Soon!';
}	

function sportsfan_charts() {
	return array();
}

function sportsfan_chart_appoll($team_name = 'eastcarolina') {
	drupal_add_library('highcharts', '2.0.4');
	drupal_add_library('highcharts', 'theme-sand-signika');
	
	$pickem = pickem_load(1);
	$current_week = $pickem->currentWeek();
	
	$items = array();
	
	$query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'team', '=')
    ->propertyCondition('conf_name', 'acc', '=')
		->propertyOrderBy('conf_division')
		->propertyOrderBy('displaytitle');
	$result = $query->execute();
	
	$teams = array();
	if (!empty($result['team'])) {
		$tids = array_keys($result['team']);
	  $teams = entity_load('team', $tids, FALSE, FALSE);
	}

	$division = '';
	foreach ($teams as $t) {
		if ($t->name == 'notredame') { continue; }
		if ($division != $t->conf_division) {
			if (!empty($division)) {
				$items[] = array();
			}
			$division = $t->conf_division;
		}
		$items[] = l(t($t->displaytitle), 'charts/appoll-movement/' . $t->name);
	}
	$items[] = array();
	$items[] = l(t('Other Schools'), '#', array(
		'attributes' => array('class' => array('dropdown-header'), '#header' => TRUE)
	));
	
	// assumes 'team' module is enabled
	$team = team_load_by_name($team_name);
	$team->fetchPollTallies();
	$pollresults = $team->pollResults(2014, 1);
	
	for ($i = 1; $i <= $current_week; $i++) {
		if (!isset($pollresults[$i])) {
			$pollresults[$i] = array('points' => NULL, 'rank' => NULL);
		}
	}
	ksort($pollresults);
	
	$settings = array(
		'points' => array(), 
		'rank' => array(), 
		'chart' => array('yaxis' => array(), 'xaxis' => array(),)
	);
	$settings['chart']['charttitle'] = sprintf("AP Poll Movement for %s - Thru Week %d", $team->displaytitle, $pickem->currentWeek());
	$settings['chart']['series']['colorByPoint'] = FALSE;
	
	if (($colors = $team->team_colors()) !== FALSE) {
		$settings['chart']['colors'] = array_values($colors);
	}
		
	foreach ($pollresults as $p) {
		$settings['points'][] = is_null($p['points']) ? NULL : (int) $p['points'];
		$settings['rank'][] = is_null($p['rank']) || ((int)$p['rank'] > 25)? NULL : (int) $p['rank'];
	}
	drupal_add_js(array('sportsfan' => $settings), 'setting');
	
	$page = array();
	$page['dropdown'] = array(
		'#theme' => 'bootstrap_dropdown',
		'#items' => $items,
		'#toggle' => array(
			'#type' => 'button',
			'#value' => t('Teams of Interest') . '&nbsp;<span class="caret"></span>',
			'#button_type' => 'button',
			'#attributes' => array(
				'class' => array('btn'),
			),
		),
		'#wrapper_attributes' => array(
			'style' => 'margin-bottom: 10px',
		)
	);
	
	$page['container'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'id' => 'container',
		),
	);
	$page['disclaimer'] = array(
		'#type' => 'markup',
		'#markup' => '<p>If the page is blank, please "shift-refresh" to clear your browser cache.</p>',
	);
	$page['#attached'] = array(
		'js' => array(
			drupal_get_path('module', 'sportsfan') . '/js/sportsfan-chart.js' => array(),
		),
	);
	return $page;
}

function sportsfan_chart_drop_in_rankins() {
	drupal_add_library('highcharts', '2.0.4');
	drupal_add_library('highcharts', 'theme-sand-signika');

	include_once(drupal_get_path('module', 'sportsfan') . '/data/ranked_team_losses.inc');
	$settings = array(
		'points_drop' => array(), 
		'team_names' => array(),
		'chart' => array('yaxis' => array(), 'xaxis' => array(),'colors' => array())
	);
	$settings['chart']['charttitle'] = 'Changes in Rankings for Ranked Teams Who Lost';
	$settings['chart']['colorByPoint'] = FALSE;
		
	foreach ($rank_team_losses as $t) {
		#if (($colors = $t->team_colors()) !== FALSE) {
		#	$settings['chart']['colors'][] = reset(array_values($colors));
		#}
		
		$settings['points_drop'][] = (int) $t['drop_in_points'];
		#$o = new stdClass();
		#$o->y = $team_data[$t->name]['sos'];
		#$o->color = "#000000";
		#$settings['sos'][] = $o;
		$settings['team_names'][] = $t['team_in_question'];
	}
	drupal_add_js(array('sportsfan' => $settings), 'setting');
	
	$page = array();
	
	$page['container'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'id' => 'container',
		),
	);
	$page['disclaimer'] = array(
		'#type' => 'markup',
		'#markup' => '<p>If the page is blank, please "shift-refresh" to clear your browser cache.</p>',
	);
	$page['#attached'] = array(
		'js' => array(
			drupal_get_path('module', 'sportsfan') . '/js/sportsfan-chart-ranked-team-losses.js' => array(),
		),
	);
	return $page;
		
}

function sportsfan_chart_ofei_sos() {
	drupal_add_library('highcharts', '2.0.4');
	drupal_add_library('highcharts', 'theme-sand-signika');
	
	$team_data = array(
		'bostoncollege' => array('name' => 'BC', 'ofei' => 0.247, 'sos' => 0.127),
		'clemson' => array('name' => 'Clem', 'ofei' => 0.247, 'sos' => 0.112),
		'duke' => array('name' => 'Duke', 'ofei' => -0.066, 'sos' => 0.285),
		'georgiatech' => array('name' => 'GT', 'ofei' => 0.763, 'sos' => 0.167),
		'fsu' => array('name' => 'FSU', 'ofei' => 0.524, 'sos' => 0.237),
		'louisville' => array('name' => 'Lou', 'ofei' => -0.086, 'sos' => 0.156),
		'miami' => array('name' => 'Mia', 'ofei' => 0.555, 'sos' => 0.094),
		'northcarolina' => array('name' => 'NC', 'ofei' => 0.245, 'sos' => 0.098),
		'ncstate' => array('name' => 'NCST', 'ofei' => -0.106, 'sos' => 0.225),
		'pitt' => array('name' => 'Pitt', 'ofei' => 0.451, 'sos' => 0.220),
		'syracuse' => array('name' => 'Syr', 'ofei' => -0.125, 'sos' => 0.160),
		'virginia' => array('name' => 'UVA', 'ofei' => 0.203, 'sos' => 0.084),
		'vatech' => array('name' => 'VT', 'ofei' => -0.086, 'sos' => 0.203),
		'wakeforest' => array('name' => 'WF', 'ofei' => -0.699, 'sos' => 0.147),
		'notredame' => array('name' => 'ND', 'ofei' => 0.228, 'sos' => 0.108),
	);
	
	$query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'team', '=')
    ->propertyCondition('conf_name', 'acc', '=')
		->propertyCondition('name', 'notredame', '!=')
		->propertyOrderBy('conf_division')
		->propertyOrderBy('displaytitle');
	$result = $query->execute();
	
	$teams = array();
	if (!empty($result['team'])) {
		$tids = array_keys($result['team']);
	  $teams = entity_load('team', $tids, FALSE, FALSE);
	}

	$settings = array(
		'ofei' => array(), 
		'sos' => array(), 
		'team_names' => array(),
		'chart' => array('yaxis' => array(), 'xaxis' => array(),'colors' => array())
	);
	$settings['chart']['charttitle'] = 'ACC - Offensive Effiency over Schedule Strength';
	$settings['chart']['colorByPoint'] = TRUE;
		
	foreach ($teams as $t) {
		if (($colors = $t->team_colors()) !== FALSE) {
			$settings['chart']['colors'][] = reset(array_values($colors));
		}
		$settings['ofei'][] = $team_data[$t->name]['ofei'];
		$o = new stdClass();
		$o->y = $team_data[$t->name]['sos'];
		$o->color = "#000000";
		$settings['sos'][] = $o;
		$settings['team_names'][] = $team_data[$t->name]['name'];
	}
	drupal_add_js(array('sportsfan' => $settings), 'setting');
	
	$page = array();
	
	$page['container'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'id' => 'container',
		),
	);
	$page['disclaimer'] = array(
		'#type' => 'markup',
		'#markup' => '<p>If the page is blank, please "shift-refresh" to clear your browser cache.</p>',
	);
	$page['#attached'] = array(
		'js' => array(
			drupal_get_path('module', 'sportsfan') . '/js/sportsfan-chart-ofei-sos.js' => array(),
		),
	);
	return $page;
}