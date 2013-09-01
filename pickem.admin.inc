<?php

function pickem_admin_settings($form, &$form_state) {
	$form['default_season'] = array(
		'#type' => 'textfield',
		'#size' => 10,
		'#default_value' => variable_get('pickem.current_season'),
		'#title' => t('Current Season'),
		'#description' => t('Enter the default season'),
	);
	
	$season_start = unserialize(variable_get('pickem.season_start'));
	$form['season_start'] = array(
		'#type' => 'date',
		'#title' => t('Start of the <em>current</em> season'),
		'#default_value' => $season_start,
	);
	$form['pickem_title'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('pickem.pickem_title'),
		'#description' => t('Enter the name of the Pickem for this site'),
		'#title' => t('Pickem Title'),
	);
	$form['standings'] = array(
		'#type' => 'container',
	);
	$form['standings']['create'] = array(
		'#type' => 'submit',
		'#value' => t('Create Initial Standings'),
		'#submit' => array('pickem_admin_create_standings_submit'),
	);
	$form['#submit'][] = 'pickem_admin_settings_submit';
	
	return system_settings_form($form);
}

function pickem_admin_settings_submit($form, $form_state) {
	variable_set('pickem.current_season', $form_state['values']['default_season']);
	variable_set('pickem.pickem_title', check_plain($form_state['values']['pickem_title']));
	
	$return_val = $form_state['values']['season_start'];
	
	variable_set('pickem.season_start', serialize($return_val));
}

function pickem_admin_games($form, &$form_state) {
	return $form;
}

function theme_admin_scoreboard($variables) {
	$form = $variables['form'];
	$header = array(
		'',
		t('Visitor'),
		t('Score'),
		t('Host'),
		t('Score'),
		t('Completed'),
		t('Overtimes'),
	);
	$rows = array();
	
	foreach ($form['games'] as $gid => $elements) {
		if (substr($gid, 0, 1) != '#') {

			$element = $form['games'][$gid]['game_date'];
			$game_date = drupal_render($element);

			$element = $form['games'][$gid]['visiting_school'];
			$visiting_school = drupal_render($element);
	
			$element = $form['games'][$gid]['visiting_school_name'];
			$visiting_school_name = drupal_render($element);
		
			$element = $form['games'][$gid]['visiting_score'];
			$visiting_score = drupal_render($element);
			
			$element = $form['games'][$gid]['host_school'];
			$host_school = drupal_render($element);
				
			$element = $form['games'][$gid]['host_school_name'];
			$host_school_name = drupal_render($element);
		
			$element = $form['games'][$gid]['host_score'];
			$host_score = drupal_render($element);
			
			$element = $form['games'][$gid]['completed'];
			$completed = drupal_render($element);
			
			$element = $form['games'][$gid]['overtimes'];
			$overtimes = drupal_render($element);
			
			$element = $form['games'][$gid]['dirty'];
			$dirty = drupal_render($element);
			
			$row = array(
				'data' => array(
					$game_date,
					$visiting_school . $visiting_school_name,
					$visiting_score,
					$host_school . $host_school_name,
					$host_score,
					$completed,
					$overtimes . $dirty,
				),
				'class' => array('game-' . $gid),
			);
			$rows[] = $row;
			unset($form['games'][$gid]);
		}
	}
	$output = theme('table', array('header' => $header, 'rows' => $rows));
	unset($form['games']);
	$output .= drupal_render_children($form);
	return $output;
}

function pickem_admin_week_games($form, &$form_state, $week = NULL, $season = NULL) {
	//db_set_active('sandbox');
	drupal_add_library('system', 'drupal.ajax');

	if (!isset($season)) {
		$season = variable_get('pickem.current_season');
	}
	if ($week == NULL) {
		$week = $_SESSION['current_week']->week;
	}
	
	$form_state['picks'] = array();
	
	$games = pickem_load_games($week, $season);
	foreach ($games as $game) {
		$form_state['picks'][$game->gid] = pickem_load_game_picks($game->gid);
		$form_state['games'][$game->gid] = $game;
		
		$form['games'][$game->gid]['game_date'] = array(
			'#type' => 'markup',
			'#markup' => $game->game_date_str,
		);
		$form['games'][$game->gid]['host_school'] = array(
			'#type' => 'item',
			'#title' => $game->host_school,
		);
		$form['games'][$game->gid]['host_school_name'] = array(
			'#type' => 'hidden',
			'#value' => $game->host_school_name,
		);
		$form['games'][$game->gid]['host_score'] = array(
			'#type' => 'textfield',
			'#size' => 4,
			'#default_value' => $game->host_score,
			'#attributes' => array('class'=>array('host_score')),
		);
		$form['games'][$game->gid]['visiting_school'] = array(
			'#type' => 'item',
			'#title' => $game->visiting_school,
		);
		$form['games'][$game->gid]['visiting_school_name'] = array(
			'#type' => 'hidden',
			'#value' => $game->visiting_school_name,
		);
		$form['games'][$game->gid]['visiting_score'] = array(
			'#type' => 'textfield',
			'#size' => 4,
			'#default_value' => $game->visiting_score,
			'#attributes' => array('class'=>array('visiting_score')),
		);
		$form['games'][$game->gid]['overtimes'] = array(
			'#type' => 'textfield',
			'#size' => 3,
			'#default_value' => $game->overtimes,
		);
		$form['games'][$game->gid]['completed'] = array(
			'#type' => 'select',
			'#options' => array(
				'N' => t('No'),
				'Y' => t('Yes'),
			),
			'#default_value' => $game->completed,
		);
		$form['games'][$game->gid]['dirty'] = array(
			'#type' => 'hidden',
			'#value' => '0',
			'#attributes' => array(
				'class' => array('dirty-flag'),
			),
		);
	}
	$form_state['week'] = $week;
	$form_state['season'] = $season;
	
	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Scores'), 
	);
	$form['#attached']['js'][] = drupal_get_path('module', 'pickem') . '/js/admin_scoreboard.js';
	$form['#tree'] = TRUE;
	
	//db_set_active();
	return $form;
}

function pickem_admin_week_games_submit($form, &$form_state) {
	//db_set_active('sandbox');

	$num_affected = 0;
	$num_picks_updated = 0;
	$num_picks_reset = 0;
	$num_standing_rec_affected = 0;
	
	$games = $form_state['values']['games'];
	foreach ($games as $gid => $game) {
		$winning_school_name = '';
		$old_game = $form_state['games'][$gid];
		$dirty = trim($old_game->host_score) != trim($game['host_score']) ||
						 trim($old_game->visiting_score) != trim($game['visiting_score']);
		$validscores = trim($game['host_score']) != '' && trim($game['visiting_score']) != '';
						 
		$scores_existed = ($old_game->host_score != '' && $old_game->visiting_score != '')
			&& $old_game->completed = 'Y';
		$scores_changed = $scores_existed && $dirty;
		
		$pick_reset_needed = $scores_existed && ($game['completed'] == 'N') &&
			empty($game['host_score']) && empty($game['visiting_score']);
						 
		if (!$dirty) { continue; }		
		
		try {
			if ($validscores) {
				$winning_school_name = $game['host_score'] > $game['visiting_score'] 
					? $game['host_school_name'] 
					: $game['visiting_school_name'];
			}
			$num_affected += db_update('game')
				->fields(array(
					'host_score' => $game['host_score'],
					'visiting_score' => $game['visiting_score'],
					'completed' => $game['completed'],
					'overtimes' => $game['overtimes'],
					'winning_school_name' => $winning_school_name,
				))
				->condition('gid', $gid)
				->execute();
				
			// TODO - account for the case where the game was already final
			// but we incorrectly entered the score.
			if ($pick_reset_needed) {
				$num_picks_reset += db_update('pick')->fields(array(
					'correct' => NULL,
				))
				->condition('gid', $gid)
				->execute();
			} else {
				if ($validscores && $winning_school_name != '') {
					foreach($form_state['picks'][$gid] as $pick) {
						$correct = ($winning_school_name == $pick->pick_school_name) ? 'Y' : 'N';
						$num_picks_updated += db_update('pick')->fields(array(
							'correct' => $correct,
						))
						->condition('pkid', $pick->pkid)
						->execute();
					}
				}
			}
		} catch (Exception $e) {
			//db_set_active();
			drupal_set_message($e->getMessage(), 'error');
		}
	}
	if ($num_picks_updated || $num_picks_reset) {
		$num_standing_rec_affected = pickem_update_standings($form_state['week'], $form_state['season']);
	}
	
	drupal_set_message($num_affected . ' game(s) updated');
	drupal_set_message($num_picks_updated . ' pick record(s) updated');
	drupal_set_message($num_picks_reset . ' pick record(s) reset');
	drupal_set_message($num_standing_rec_affected . ' standing record(s) updated');
	//db_set_active();
}

function pickem_admin_players($form, &$form_state) {
	$players = pickem_load_players();
	
	$header = array(
		t('Player'),
		t('Player Level'),
		t('Games Picked'),
	);
	$rows = array();
	foreach ($players as $player) {
		$rows[] = array(
			'data' => array(
				$player->name,
				$player->player_level,
				$player->games_picked,
			),
		);
	}
	$form['players'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
	);
	return $form;
}

function pickem_admin_create_standings_submit($form, $form_state) {
	# ROAD MAP
	// 1) Load player stats
	// 2) Create Standings rows
	//    a) Overall (week = -1, season = current season)
	//		b) First week 
	
	$num_affected = db_delete('standings')->execute();
	
	$pickdata = pickem_load_pick_data(1, 2013);
	kpr($pickdata['player']);
	foreach ($pickdata['player'] as $player => $data) {
		$week = -1;
		$season = variable_get('pickem.current_season', 2013);
		$q = db_insert('standings')->fields(
			array(
				'season' => $season,
				'week' => $week,
				'uid' => $data['uid'],
				'correct' => 0,
				'incorrect' => 0,
			)
		);
		try {
			$res = $q->execute();
		} catch(Exception $e) {
			drupal_set_message($e->getMessage, 'error');
		}

		$week = 1;
		$season = variable_get('pickem.current_season', 2013);
		$q = db_insert('standings')->fields(
			array(
				'season' => $season,
				'week' => $week,
				'uid' => $data['uid'],
				'correct' => 0,
				'incorrect' => 0,
			)
		);
		try {
			$res = $q->execute();
		} catch(Exception $e) {
			drupal_set_message($e->getMessage(), 'error');
		}
	}
	drupal_set_message('Standings have been initialized');
}
