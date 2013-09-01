<?php

function pickem_pick_breakdown($week = NULL) {
	drupal_add_library('system', 'drupal.ajax');
	if ($week == NULL) {
		$week = $_SESSION['current_week']->week;
	}
	$season = variable_get('pickem.current_season');
	
	$data = pickem_load_pick_data($week, $season);
	#dpm($data);
	$build = array();
	$build['title'] = array(
		'#type' => 'markup',
		'#markup' => t('<h3>Pick Breakdown for Week ' . $week . '</h3>'),
	);
	foreach($data['game'] as $gid => $gamedata) {
		$header = array(
			t(''),
			$gamedata['visiting_school'],
			$gamedata['host_school'],
		);
		$rows = array();

		$game_title_cell = '<strong>' . t($gamedata['game_title']) . '</strong>'; 
			#. '&nbsp;<i class="icon-arrow-down" data-toggle="collapse" data-target="#game-' . $gid . '-data"></i>';
		$game_vis_school_cell = $gamedata[$gamedata['visiting_school_name']]['count'];
		$game_host_school_cell = $gamedata[$gamedata['host_school_name']]['count'];
		
		$rows[] = array(			
			'data' => array(
				array('data' => $game_title_cell, 'width' => '50%', 'title' => $gid),
				array('data' => $game_vis_school_cell, 'width' => '25%'),
				array('data' => $game_host_school_cell, 'width' => '25%'),
			),
		);
		$rows[] = array(
			'data' => array(
				'',
				implode('<br />', $gamedata[$gamedata['visiting_school_name']]['who']),
				implode('<br />', $gamedata[$gamedata['host_school_name']]['who']),
			),
			'class' => array('collapse in'),
			'id' => 'game-' . $gid . '-data',
		);
		$build['game-'.$gid] = array(
			'#theme' => 'table',
			'#header' => $header,
			'#rows' => $rows,
			'#attributes' => array(
				'class' => array('table', 'table-hover', 'table-border'),
			),
		);
	}
	
	return $build;
}
