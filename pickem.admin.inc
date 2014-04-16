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
	$form['bettinglines'] = array(
		'#type' => 'fieldset',
		'#title' => t('Point Spreads/Betting Line Setup'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		'#tree' => TRUE,
	);
	$form['bettinglines']['rss_feed_url'] = array(
		'#type' => 'textfield',
		'#title' => t('RSS Feed URL'),
		'#description' => t('Enter the URL of the RSS feed used to fetch the point spreads'),
		'#default_value' => variable_get('pickem.betting_lines_rss_feed_url', ''),
		'#size' => '128',
	);
	$form['bettinglines']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Betting Line Settings'),
		'#submit' => array('pickem_admin_betting_line_settings_submit'),
	);
	$form['standings'] = array(
		'#type' => 'fieldset',
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		'#title' => t('Standings'),
		'#tree' => TRUE,
	);
	$form['standings']['week'] = array(
		'#type' => 'select',
		'#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18)),
		'#default_value' => $_SESSION['current_week']->week,
		'#title' => t('Select Week'),
	);
	$form['standings']['create_overall'] = array(
		'#type' => 'checkbox',
		'#title' => t('(Re)Create the Overall Standings?'),
		'#description' => t('Use with extreme care'),
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

function pickem_admin_betting_line_settings_submit($form, &$form_state) {
	variable_set('pickem.betting_lines_rss_feed_url', $form_state['values']['bettinglines']['rss_feed_url']);
}

function pickem_admin_games($form, &$form_state) {
	return $form;
}

function theme_admin_scoreboard($variables) {
	$output = '';
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
	
	foreach ($form['games'] as $dateKey => $games) {
		if (substr($dateKey, 0, 1) != '#') {
			$rows = array();
			foreach ($games as $gid => $element) {
				if (substr($gid, 0, 1) != '#') {
					
					$row_classes[] = 'game-' . $gid;
					if (isset($form['games'][$dateKey][$gid]['hasPicks'])) {
						$row_classes[] = 'info';
						unset($form['games'][$dateKey][$gid]['hasPicks']);
					}
					$element = $form['games'][$dateKey][$gid]['game_date'];
					$game_date = drupal_render($element);
		
					$element = $form['games'][$dateKey][$gid]['visiting_school'];
					$visiting_school = drupal_render($element);
			
					$element = $form['games'][$dateKey][$gid]['visiting_school_name'];
					$visiting_school_name = drupal_render($element);
				
					$element = $form['games'][$dateKey][$gid]['visiting_score'];
					$visiting_score = drupal_render($element);
					
					$element = $form['games'][$dateKey][$gid]['host_school'];
					$host_school = drupal_render($element);
						
					$element = $form['games'][$dateKey][$gid]['host_school_name'];
					$host_school_name = drupal_render($element);
				
					$element = $form['games'][$dateKey][$gid]['host_score'];
					$host_score = drupal_render($element);
					
					$element = $form['games'][$dateKey][$gid]['completed'];
					$completed = drupal_render($element);
					
					$element = $form['games'][$dateKey][$gid]['overtimes'];
					$overtimes = drupal_render($element);
					
					$element = $form['games'][$dateKey][$gid]['dirty'];
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
						'class' => $row_classes,
					);
					$rows[] = $row;
					unset($form['games'][$dateKey][$gid]);
				}
			}
			$output .= theme('table', 
				array(
					'header' => $header, 
					'rows' => $rows,
					'attributes' => array('class' => array('table')),
					'caption' => $dateKey,
				)
			);	
		}
	}
	//$output = theme('table', array('header' => $header, 'rows' => $rows));
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
	$form_state['force_completed'] = FALSE;
	$form_state['update_standings'] = TRUE;
	
	$form_state['picks'] = array();
	
	$games = pickem_load_games($week, $season);
	$players = pickem_load_players(TRUE, 'active-full');
	$form_state['players'] = $players;
	
	foreach ($games as $game) {
		$dateKey = date('M j, Y', $game->game_date);
		
		$picks_for_game = pickem_load_game_picks($game->gid);
		$form_state['picks'][$game->gid] = $picks_for_game;
		if (!empty($picks_for_game)) {
			$dateKey = '@Pickem Games';		
		}
		$form_state['games'][$dateKey][$game->gid] = $game;
		
		$form['games'][$dateKey][$game->gid]['game_date'] = array(
			'#type' => 'markup',
			'#markup' => $game->game_date_str,
		);
		$form['games'][$dateKey][$game->gid]['host_school'] = array(
			'#type' => 'item',
			'#title' => $game->host_school,
		);
		$form['games'][$dateKey][$game->gid]['host_school_name'] = array(
			'#type' => 'hidden',
			'#value' => $game->host_school_name,
		);
		$form['games'][$dateKey][$game->gid]['host_score'] = array(
			'#type' => 'textfield',
			'#size' => 4,
			'#default_value' => $game->host_score,
			'#attributes' => array('class'=>array('host_score')),
		);
		$form['games'][$dateKey][$game->gid]['visiting_school'] = array(
			'#type' => 'item',
			'#title' => $game->visiting_school,
		);
		$form['games'][$dateKey][$game->gid]['visiting_school_name'] = array(
			'#type' => 'hidden',
			'#value' => $game->visiting_school_name,
		);
		$form['games'][$dateKey][$game->gid]['visiting_score'] = array(
			'#type' => 'textfield',
			'#size' => 4,
			'#default_value' => $game->visiting_score,
			'#attributes' => array('class'=>array('visiting_score')),
		);
		$form['games'][$dateKey][$game->gid]['overtimes'] = array(
			'#type' => 'textfield',
			'#size' => 3,
			'#default_value' => $game->overtimes,
		);
		$form['games'][$dateKey][$game->gid]['completed'] = array(
			'#type' => 'select',
			'#options' => array(
				'N' => t('No'),
				'Y' => t('Yes'),
			),
			'#default_value' => $game->completed,
		);
		$form['games'][$dateKey][$game->gid]['dirty'] = array(
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
	$form['actions']['submit-final'] = array(
		'#type' => 'submit',
		'#value' => t('Save Final Scores'),
		'#submit' => array('pickem_admin_week_games_submit_finals'),
	);
	$form['actions']['submit-scores'] = array(
		'#type' => 'submit',
		'#value' => t('Save Scores'),
		'#submit' => array('pickem_admin_week_games_submit_scores'),
	);
	
	$form['#attached']['js'][] = drupal_get_path('module', 'pickem') . '/js/admin_scoreboard.js';
	$form['#tree'] = TRUE;
	
	//db_set_active();
	return $form;
}

function pickem_admin_week_games_submit_finals($form, &$form_state) {
	$form_state['force_completed'] = TRUE;
	$form_state['update_standings'] = TRUE;
	pickem_admin_week_games_submit($form, $form_state);
}

function picmem_admin_week_games_submit_scores($form, &$form_state) {
	$form_state['force_completed'] = FALSE;
	$form_state['update_standings'] = FALSE;
	pickem_admin_week_games_submit($form, $form_state);	
}

function pickem_admin_week_games_submit($form, &$form_state) {
	//db_set_active('sandbox');

	$num_affected = 0;
	$num_picks_updated = 0;
	$num_picks_reset = 0;
	$num_standing_rec_affected = 0;
	
	$games = $form_state['values']['games'];
	foreach ($games as $dateKey => $slate) {
		
		foreach ($slate as $gid => $game) {
			$winning_school_name = '';
			$old_game = $form_state['games'][$dateKey][$gid];
			
			$validscores = (trim($game['host_score']) != '') && (trim($game['visiting_score']) != '');
			$dirty = $validscores && ((trim($old_game->host_score) != trim($game['host_score'])) ||
							 (trim($old_game->visiting_score) != trim($game['visiting_score'])));
							 
			$scores_existed = ($old_game->host_score != '' && $old_game->visiting_score != '')
				&& $old_game->completed = 'Y';
			$scores_changed = $scores_existed && $dirty;
			
			if ($form_state['force_completed']) {
				$game['completed'] = 'Y';
			}
			$pick_reset_needed = $scores_existed && ($game['completed'] == 'N') &&
				empty($game['host_score']) && empty($game['visiting_score']);
							 
			if (!$dirty) { continue; }		
			
			try {
				$winning_school_name = '';
				$losing_school_name = '';
				
				if ($validscores) {
					$winning_school_name = $game['host_score'] > $game['visiting_score'] 
						? $game['host_school_name'] 
						: $game['visiting_school_name'];
					$losing_school_name = $game['host_score'] < $game['visiting_score'] 
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
						$the_players = $form_state['players'];	// assoc array
						
						if (count($form_state['picks'][$gid]) > 0) {
							foreach($form_state['picks'][$gid] as $pick) {
								if (array_key_exists($pick->player_name, $the_players)) {
									unset($the_players[$pick->player_name]);
								}
								$correct = ($winning_school_name == $pick->pick_school_name) ? 'Y' : 'N';
								$num_picks_updated += db_update('pick')->fields(array(
									'correct' => $correct,
								))
								->condition('pkid', $pick->pkid)
								->execute();
							}
							
							// if any players are left, they get an "L" for this pick and we
							// have to create a record for it (assuming they are 'active-full' status)
							if (count($the_players)) {
								foreach ($the_players as $player_name => $pickem_player) {
									db_insert('pick')->fields(
										array(
											'uid' => $pickem_player->uid,
											'pick_school_name' => $losing_school_name,
											'correct' => 'N',
											'gid' => $gid,
											'week' => $form_state['week'],
											'season' => $form_state['season'],
										)
									)->execute();
								}
								drupal_set_message(t('<strong>@numplayers player(s)</strong> failed to pick the <strong>@winningschool</strong> v. <strong>@losingschool</strong> game and were assessed a loss.', 
									array('@numplayers' => count($the_players), '@winningschool' => $winning_school_name, '@losingschool' => $losing_school_name)), 'warning');
							}
						}
						// end process players who didn't pick the game...
					}
				}
			} catch (Exception $e) {
				//db_set_active();
				drupal_set_message($e->getMessage(), 'error');
			}
		}
	
	}
	if ($num_picks_updated || $num_picks_reset) {
		if ($form_state['update_standings']) {
			$num_standing_rec_affected = pickem_update_standings($form_state['week'], $form_state['season']);
		}
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
	
	###db_set_active('sandbox');
	$num_affected = db_delete('standings')->execute();
	
	# $pickdata = pickem_load_pick_data($form_state['values']['standings']['week'], 2013);
	$pickdata = pickem_load_pick_data(-1, variable_get('pickem.current_season', 2013));
	$season = variable_get('pickem.current_season', 2013);
	
	foreach ($pickdata['player'] as $player => $data) {
		// handle each week
		foreach ($data['week'] as $week => $stats) {
			$a = db_insert('standings')
				->fields(array(
					'season' => $season,
					'week' => $week,
					'uid' => $data['uid'],
					'correct' => $stats['correct'],
					'incorrect' => $stats['incorrect'], 
				))
				->execute();
		}
		
		// then handle the overall (week = -1)
		$a = db_insert('standings')
			->fields(array(
				'season' => $season,
				'week' => -1,
				'uid' => $data['uid'],
				'correct' => $data['correct'],
				'incorrect' => $data['incorrect'], 
			))
			->execute();
	}
	kpr($pickdata['player']);
	
	/*
	foreach ($pickdata['player'] as $player => $data) {
		if ($form_state['values']['standings']['create_overall'] == 1) {
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
		}

		$week = $form_state['values']['standings']['week'];
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
	 * 
	 */
	drupal_set_message('Standings have been initialized');
	###db_set_active();
}
