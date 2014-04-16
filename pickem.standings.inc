<?php

/**
 * Pickem Standings Overview Page
 */
function pickem_standings_overview($week, $pickem_id, $season = NULL) {
	###db_set_active('sandbox');
	global $user;
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}
	
	if (!is_numeric($pickem_id)) {
		$pickem_ids = module_invoke_all('pickem_id', $pickem_id);
		$pickem_id = $pickem_ids[0];
	}
	
	$r = db_query('SELECT COUNT(*) as slatecount FROM {slate} WHERE season = :season AND pid = :pickem_id AND slate_id = :week',
		array(':season' => $season, ':pickem_id' => $pickem_id, ':week' => $week));
	$obj = $r->fetchObject();
	$slateOfGamesCount = $obj->slatecount;
	


	$games_picked_header = ($week < $_SESSION['current_week']->week) ? 'Picks Completed' : 'Games Picked';
	
	$headers = array(
		t('Rank'),
		t(''),
		t('Player'),
		t('Correct'),
		t('Incorrect'),
		t('Pct'),
		t($games_picked_header),
	);
	
	$rows = array();
	
	if ($week <= $_SESSION['current_week']->week) {
		$current_rank = 0;
		$last_correct = NULL;
		$rank_pos = 0;
		
		$standings = pickem_load_standings($week, $pickem_id, $season);
		foreach ($standings as $player) {
			$rank_pos++;
			$current_correct = $player->correct;
			
			if ($player->correct != $last_correct) {
				$current_rank = $rank_pos;
				$last_correct = $player->correct;
			}
			
			$pct = 0.0;
			if ($player->correct + $player->incorrect) {
				$pct = (float) $player->correct / ($player->correct + $player->incorrect);
			}
			$classes = array();
			$thumb = '';
			if (user_is_logged_in()) {
				$player->data = user_load($player->uid);
				//dpm($player->data);
				if ($player->data->picture != NULL) {
					$thumb = '<img src="'.image_style_url('standings', $player->data->picture->uri).'" />&nbsp;';
				}
				if ($player->player_name == $user->name) {
					$classes[] = 'info';
				}
			}
			$player_name_cell = $player->player_name;
			if ($player->weeks_won > 0 && ($week == -1)) {
				$player_name_cell .= ' (' . $player->weeks_won . ')';
			}
			if ($week == -1) {
				$player_name_cell = l($player_name_cell, 'user/'.$player->uid.'/pickemresults');
			}
			//if ($week == $_SESSION['current_week']->week) {
			//	$player_name_cell = l(t($player->player_name), 'pickem/slate/' . $player->uid);
			//}
			$rows[] = array(
				'data' => array(
					array('data' => $current_rank, 'width' => '50'),
					array('data' => $thumb, 'width' => '50'),
					$player_name_cell,
					$player->correct,
					$player->incorrect,
					number_format($pct, 3),
					#$data['num_picks'],
					isset($player->pick_count) ? $player->pick_count : '',
				),
				'class' => $classes,
			);
		}
	}
	$build['standings'] = array(
		'#theme' => 'table',
		'#header' => $headers,
		'#rows' => $rows,
		'#empty' => t('Standings are not available.  Please check back later.'),
		'#attributes' => array('class' => array('table', 'table-condensed', 'standings')),
	);
	###db_set_active();
	return $build;
}	
