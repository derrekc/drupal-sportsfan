<?php

function pickem_process_records($tags, &$records_data, $target, $opponent, $target_score, $opp_score, $neutral = FALSE) {
	if (!is_array($tags)) {
		$tmp = $tags;
		$tags[] = $tmp;
		unset($tmp);
	}
		
	foreach ($tags as $tag) {	
		$ignore_criteria = in_array($tag, array('overall','home','away','top25','top10'));
		
		$same_conf = $target->conf == $opponent->conf || $ignore_criteria;
		$same_div = $ignore_criteria || ($same_conf &&
			($target->division != '' && $opponent->division != '') && 
			($target->division == $opponent->division));
			
		/*
		if ($same_conf && $target->conf == 'acc') {
			drush_print_r(array(
				'target' => $target->name,
				'opponent' => $opponent->name,
				'same_conf' => (($same_conf) ? 'YES' : 'NO'),
				'same_div' => (($same_div) ? 'YES' : 'NO'),
			));
		}
		 * 
		 */
		$is_top25_opp = ($opponent->rank != '') && ($opponent->rank <= 25);
		$is_top10_opp = ($opponent->rank != '') && ($opponent->rank <= 10);
	
		$empty_record = array(
			'sport' => 'ncaaf', 
			'season' => variable_get('pickem.current_season', 2013),
			'school_name' => $target->name, 
			'sid' => $target->sid, 
			'wins' => 0, 'losses' => 0, 'ties' => 0, 
			'tag' => $tag, 
			'is_new' => TRUE,
		);
		if (!isset($records_data[$target->name])) {
			$records_data[$target->name] = array();
		}
		if (!isset($records_data[$target->name][$tag])) {
			$records_data[$target->name][$tag] = (object) $empty_record;
		}
		
		$won = ($target_score > $opp_score);
		$lost = ($target_score < $opp_score);
		$tied = ($target_score == $opp_score);
		
		$can_update = FALSE;
		switch ($tag) {
			case 'overall':
				$can_update = $ignore_criteria;
				break;
				
			case 'conf':
				$can_update = $same_conf;
				break;
				
			case 'div':
				$can_update = $same_div;
				break;

			case 'home':
			case 'away':
				$can_update = (!$neutral);
				break;
				
			case 'conf_home':
			case 'conf_away':
				$can_update = (!$neutral && $same_conf);
				break;
								
			case 'ooc':
				$can_update = !$same_conf;
				break;
				
			case 'ooc_home':
			case 'ooc_away':
				$can_update = (!$neutral && !$same_conf);
				break;
				
			case 'neutral':
				$can_update = $neutral;
				break;
				
			case 'top25':
				$can_update = $is_top25_opp;
				break;
				
			case 'top10':
				$can_update = $is_top10_opp;
				break;
		}
		
		$records_data[$target->name][$tag]->wins += ($can_update && $won) ? 1 : 0;
		$records_data[$target->name][$tag]->losses += ($can_update && $lost) ? 1 : 0;
		$records_data[$target->name][$tag]->ties += ($can_update && $tied) ? 1 : 0;
	}
}

function pickem_process_points($tags, &$points_data, $target, $opponent, $target_score, $opp_score, $neutral = FALSE) {
	if (!is_array($tags)) {
		$tmp = $tags;
		$tags = array($tmp);
		unset($tmp);
	}
		
	foreach ($tags as $tag) {	
		$ignore_criteria = in_array($tag, array('total','home','away','top25','top10'));
		
		$same_conf = $target->conf == $opponent->conf || $ignore_criteria;
		$same_div == $ignore_criteria || ($same_conf &&
			($target->division != '' && $opponent->division != '') && 
			($target->division == $opponent->division));
			
		$is_top25_opp = ($opponent->rank != '') && ($opponent->rank <= 25);
		$is_top10_opp = ($opponent->rank != '') && ($opponent->rank <= 10);
	
		$empty_record = array(
			'sport' => 'ncaaf', 
			'season' => variable_get('pickem.current_season', 2013),
			'school_name' => $target->name, 
			'sid' => $target->sid, 
			'gp' => 0, 'pf' => 0, 'pa' => 0, 'ppg' => 0.0, 'ppa' => 0.0, 
			'tag' => $tag,
			'is_new' => TRUE
		);
		if (!isset($points_data[$target->name])) {
			$points_data[$target->name] = array();
		}
		if (!isset($points_data[$target->name][$tag])) {
			$points_data[$target->name][$tag] = (object) $empty_record;
		}
				
		$can_update = FALSE;
		
		switch ($tag) {
			case 'total':
			case 'conf':
			case 'div':
				$can_update = (($ignore_criteria || $same_conf || $same_div));
				break;

			case 'home':
			case 'away':
				$can_update = (!$neutral);
				break;
				
			case 'conf_home':
			case 'conf_away':
				$can_update = (!$neutral && $same_conf);
				break;
								
			case 'ooc':
				$can_update = !$same_conf;
				break;
				
			case 'ooc_home':
			case 'ooc_away':
				$can_update = (!$neutral && !$same_conf);
				break;
				
			case 'neutral':
				$can_update = $neutral;
				break;
				
			case 'top25':
				$can_update = $is_top25_opp;
				break;
				
			case 'top10':
				$can_update = $is_top10_opp;
				break;
		}
		
		$points_data[$target->name][$tag]->pf += $can_update ? $target_score : 0;
		$points_data[$target->name][$tag]->pa += $can_update ? $opp_score : 0;
		$points_data[$target->name][$tag]->gp += $can_update ? 1 : 0;
		$points_data[$target->name][$tag]->ppg = $can_update
			? $points_data[$target->name][$tag]->pf / $points_data[$target->name][$tag]->gp
			: $points_data[$target->name][$tag]->ppg;
		$points_data[$target->name][$tag]->ppa = $can_update
			? $points_data[$target->name][$tag]->pa / $points_data[$target->name][$tag]->gp
			: $points_data[$target->name][$tag]->ppa;
	}
	
}

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
		if ($gamedata['completed'] == 'Y') { continue; }
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
