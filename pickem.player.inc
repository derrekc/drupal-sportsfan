<?php

function pickem_player_results($user) {
	global $user;
	if ($user->uid == 1) {
		$player = user_load(12);
	} else {
		$player = $user;
	}
	$season = variable_get('pickem.current_season');
	
	$pick_data = pickem_load_pick_data(-1, $season, array('uid' => $player->uid));
	ksort($pick_data['player'][$player->name]['school']);
	//dpm($pick_data['player'][$player->name]['school']);
	
	$header = array(
		t(''),
		t(''),
		array('data' => t('Record'), 'colspan' => 3, 'align' => 'center'),
	);
	$rows = array();
	$rows[] = array(
		'data' => array(
			array('data' => t('School'), 'header' => TRUE),
			array('data' => t('# Games'), 'header' => TRUE),
			array('data' => t('When Picking<br />Games Involving<br/ >School'), 'header' => TRUE),
			array('data' => t('When Picking<br />School'), 'header' => TRUE),
			array('data' => t('When Picking<br />Against School'), 'header' => TRUE),
		)
	);
	foreach ($pick_data['player'][$player->name]['school'] as $school => $data) {
		$row = array();
		$row[] = $school;
		$row[] = $data['num_games'];
		$row[] = $data['correct'] . '-' . $data['incorrect'];
		if (($data['pw_correct'] == 0) && ($data['pw_incorrect'] == 0)) {
			$row[] = '&ndash;';
		} else {
			$row[] = $data['pw_correct'] . '-' . $data['pw_incorrect'];			
		}
		if (($data['pa_correct'] == 0) && ($data['pa_incorrect'] == 0)) {
			$row[] = '&ndash;';
		} else {
			$row[] = $data['pa_correct'] . '-' . $data['pa_incorrect'];			
		}
		$row_data = array('data' => $row);
		if ($data['conf_name'] == 'acc') {
			$row_data['class'] = array('info');
		}
		$rows[] = $row_data;
	}
	$page['stats'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array(
			'class' => array('table', 'table-condensed', 'table-hover'),
			'id' => 'player_pick_data',
		)
	);
	return $page;
	
}

function pickem_player_records($pickem_user) {
	global $user;
	
	dpm($pickem_user);
	$current_week = variable_get('pickem.current_week');
	$season = variable_get('pickem.current_season');
	
	$header = array(
		t('Week'),
		'',
		t('W'),
		t('L'),
	);
	$rows = array();
	
	for ($i = 1; $i <= $current_week; $i++) {
		$standings = pickem_load_standings($i, $season);
		$current_rank = 0;
		$last_correct = NULL;
		$rank_pos = 0;
		
		foreach ($standings as $player) {

			if ($player->uid == $pickem_user->uid || ($pickem_user->uid == 1 && $player->uid == 12)) {
				$rows[] = array(
					$i,
					$player->player_name,
					$player->correct,
					$player->incorrect,
				);
				break;
			}
		}
	}
	$build['standings'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array('class' => array('table', 'table-condensed', 'standings')),
	);
	return $build;
	
}
