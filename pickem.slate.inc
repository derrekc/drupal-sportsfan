<?php

function pickem_slate_form($form, &$form_state, $week = NULL, $pickem_id = NULL, $player_uid = NULL) {
	if (!user_is_logged_in()) {
		drupal_goto('pickem/overview');
		return;
	}	
	
	$now = time();
	
	drupal_add_library('system', 'drupal.ajax');
	global $user;

	//if ($week == NULL || $week == '') {
	//	$week = $_SESSION['current_week']->week;
	//}
	if (empty($week)) {
		$week_data = new stdClass();
		drupal_alter('week', $week_data, $pickem_id);
	}
	
	if (is_null($pickem_id)) {
		$pickem_id = variable_get('pickem.default_pickem_id');
	} else {
		if (!is_numeric($pickem_id)) {
			$pickem_ids = module_invoke_all('pickem_id', $pickem_id);
			$pickem_id = $pickem_ids[0];
		}		
	}
	
	$week_rec = db_query('SELECT * FROM {week} WHERE week = ' . $week)->fetchObject();
	$newtitle = drupal_set_title(t(variable_get('pickem.pickem_title','ACCbbs Pickem') . ' - Pick Slate, ' . $week_rec->title));
	
	// RESET the output message queue.
	drupal_get_messages(NULL, TRUE);
	
	$other_player = NULL;
	$result = NULL;
	if ($user->uid == 1) {
		$player_uid = 12;
	}
	// load the user picks
	$userpicks = pickem_load_slate_picks_by_uid($week);
	
	if ($player_uid) {
		$other_player = pickem_load_player($player_uid, $pickem_id);
		$result = pickem_fetch_slate($week, $pickem_id, $other_player->uid);
	} else {
		$result = pickem_fetch_slate($week, $pickem_id);
	}
	$season = variable_get('pickem.current_season');
	
	$form['week'] = array('#type' => 'hidden', '#value' => $week);
	$form['season'] = array('#type' => 'hidden','#value' => $season);
	
	# keep track of the number of valid picks
	$validPicks = 0;
	$totalGames = 0;
	$pickCutoffTime = 0;
	$num_picks_open = 0;
	$pickemWeekIsLocked = pickem_is_locked_for_picks($week, $season);
	
	$saturday_cutoff = NULL;

	foreach ($result as $record) {
		$totalGames++;
		
		// INIT Pick Is Open logic
		$cutoff_time_for_pick = '';
		if (date('D', $record->game_date) == 'Sat' && is_null($saturday_cutoff)) {
			$saturday_cutoff = $record->game_date - (60 * variable_get('pickem.cutoff_minutes', 15));
			//$pickCutoffTime = $saturday_cutoff;
		}
		
		$record->pickIsOpen = pickem_pick_is_open($record, $now, $saturday_cutoff, $cutoff_time_for_pick);
		$num_picks_open += ($record->pickIsOpen) ? 1 : 0;
		$form_state['cutoff_time'][$record->gid] = $cutoff_time_for_pick;
		
		if ($record->pickIsOpen && $pickCutoffTime == 0) {
			// because the games results are sorted by game date, this is a safe
			// block of coding logic
			$pickCutoffTime = $cutoff_time_for_pick;
		}
		
		# process the pick school name (when available)
		$form_state['picks'][$record->gid] = NULL;
		if ($record->pkid) {
			if (isset($userpicks[$record->pkid]['uid'][$player_uid])) {
				$validPicks++;
				$form_state['picks'][$record->gid] = TRUE;
			}
		}
		
		$record->vs_at_symbol = $record->neutral == 'Y' ? ' vs ' : ' at ';
		$game_title = '';
		
		if ($record->completed == 'Y') {
			$game_title = t('<h3>@visiting_school - @visiting_score, @host_school - @host_score</h3>',
				array('@visiting_school' => $record->visiting_school, 
							'@visiting_score' => $record->visiting_score, 
							'@host_school' => $record->host_school, 
							'@host_score' => $record->host_score
							)
						);
		} else {
			$game_title = t('<h3>@visiting_school @vs_at @host_school</h3>',
				array(
					'@visiting_school' => $record->visiting_school, 
					'@vs_at' => $record->vs_at_symbol, 
					'@host_school' => $record->host_school, 
				)
			);
		}

		$form['games'][$record->gid]['pkid'] = array('#type' => 'hidden', '#default_value' => $record->pkid ? $record->pkid : '');
		$form['games'][$record->gid]['game_title'] = array(
			'#type' => 'markup',
			'#markup' => $game_title,
		);
		
		$pick_options = array();
		$pickUISettings = array('disabled' => FALSE, 'default_value' => '');
		_pickem_slate_init_pick_options($pick_options, $record, $week, $pickUISettings);
		$form['games'][$record->gid]['pick_school_name'] = array(
			'#type' => 'select',
			'#title' => t('Your Pick:'),
			'#title_display' => 'invisible',
			'#options' => $pick_options,
			'#disabled' => $pickUISettings['disabled'],
			'#default_value' => $pickUISettings['default_value'],
		);
		unset($pickUISettings);
	}
	$form['actions'] = array(
		'#type' => 'actions',
		'#attributes' => array('class' => array('span12', 'clear-row')),
	);
	if (user_access('pickem player') && ($num_picks_open > 0)) {
		$form['actions']['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Save Picks'),
			'#attributes' => array(
				'class' => array('btn')
			),
			'#disabled' => ($_SESSION['current_week']->week != $week),
		);
	} else {
		if (!user_access('pickem player')) {
			$form['actions']['register'] = array(
				'#type' => 'markup',
				'#markup' => t('<p><a href="/user/register"><strong>Please register</strong></a> in order to save these picks and participate in the World\'s Greatest ACC Pickem Competition.</p>'),
			);
		} else if ($num_picks_open == 0) {
			$markup = '<p>' . t("This week's picks are now locked.  Good luck and score well.");
			if ($user->name != 'ecuacc4ever') {
				$markup .= '  ' . t('Just not as well as <strong><em>the champ</em></strong> (ecuacc4ever)');
			}
			$markup .= '</p>';
			
			$form['actions']['register'] = array(
				'#type' => 'markup',
				'#markup' => $markup,
			);
			
		}
	}
	$form['#tree'] = TRUE;
	return $form;
}

function _pickem_slate_init_pick_options(&$pick_options, $game, $week, &$pickUISettings) {
	if ($game->completed == 'Y') {
		$pick_text = sprintf("%s (%s)", $game->userpick_school, ($game->pick_is_correct == 'Y' ? 'Correct' : 'Wrong'));
		$pick_options[] = $pick_text;
		return;
	}

	if ($week > $_SESSION['current_week']->week) {
		$pick_options[] = t('Locked until Week ' . $week);
		return;
	}
	
	$pick_options[''] = t('-- Select School --');
	$pick_options[$game->visiting_school_name] = $game->visiting_school;
	$pick_options[$game->host_school_name] = $game->host_school;
	
	$pickUISettings['default_value'] = $game->pkid ? $game->pick_school_name : '';
	$pickUISettings['disabled'] = !$game->pickIsOpen;
	
}

function pickem_slate_form_submit($form, &$form_state) {
	
}
function theme_slate_form($variables) {

}

function pickem_slate_listing($form, &$form_state, $week = NULL, $pickem_id = NULL, $player_uid = NULL) {

	if (!user_is_logged_in()) {
		drupal_goto('pickem/overview');
		return;
	}
	//db_set_active('sandbox');
	$now = time();
	
	drupal_add_library('system', 'drupal.ajax');
	global $user;

	$pickem_settings = new stdClass();
	$pickem_settings->week = 1;
	$pickem_settings->pickem_id = NULL;
	$context = array('pickem_name' => $pickem_id);
	drupal_alter('pickem_settings', $pickem_settings, $pickem_id);
	$week = $pickem_settings->week->week;
	
	if ($week == NULL || $week == '') {
		$week = $_SESSION['current_week']->week;
	}
	if (is_null($pickem_id)) {
		$pickem_id = variable_get('pickem.default_pickem_id');
	}
	
	#$week_rec = db_query('SELECT * FROM {week} WHERE week = ' . $week)->fetchObject();
	try {
		$newtitle = drupal_set_title(t(variable_get('pickem.pickem_title','ACCbbs Pickem') . ' - Pick Slate, ' . $pickem_settings->week->title));
	} catch(Exception $e) {
		$newtitle = drupal_set_title(t('Pickem'));
	}
	
	// RESET the output message queue.
	drupal_get_messages(NULL, TRUE);
	
	$other_player = NULL;
	$result = NULL;
	if ($user->uid == 1) {
		$player_uid = 12;
	}
	if ($player_uid) {
		$other_player = pickem_load_player($player_uid, $pickem_settings->pickem_id);
		//$userres = db_query('SELECT * FROM {users} WHERE uid = :uid', array(':uid' => $player_uid));
		//$other_player = $userres->fetchObject();
		$result = pickem_fetch_slate($week, $pickem_settings->pickem_id, $other_player->uid);

	} else {
		$result = pickem_fetch_slate($week, $pickem_settings->pickem_id);
	}
	
	$season = variable_get('pickem.current_season');
	
	$clearrow = TRUE;
	$form['week'] = array(
		'#type' => 'hidden',
		'#value' => $week,
	);
	$form['season'] = array(
		'#type' => 'hidden',
		'#value' => $season,
	);
	# keep track of the number of valid picks
	$validPicks = 0;
	$totalGames = 0;
	$pickCutoffTime = 0;
	$num_picks_open = 0;
	$pickemWeekIsLocked = pickem_is_locked_for_picks($week, $season);
	
	$saturday_cutoff = NULL;
	
	$settings = array('uid' => $user->uid, 'games' => array(), 'pick_count' => 0);
	
	while($record = $result->fetchObject()) {
		$totalGames++;
		if ($totalGames == 1) {
			// because the games results are sorted by game date, this is a safe
			// block of coding logic
			// $pickCutoffTime = $record->game_date - (60 * variable_get('pickem.cutoff_minutes', 15));
		}
		
		$title_data = array();
		$title_data['visiting_record'] = array();
		$title_data['host_record'] = array();
		
		$rec_res = db_select('school_records', 'sr')
			->fields('sr')
			->condition('season', $season)
			->condition('sport', 'ncaaf')
			->condition('sid', $record->host_sid)
			->condition('tag', array('overall', 'conf', 'pythag'))
			->execute();
		foreach ($rec_res as $a) {
			$title_data['host_record'][$a->tag] = $a->wins . '-' . $a->losses;
			if ($a->tag == 'conf') {
				$title_data['host_record'][$a->tag] .= ' ' . $record->host_conf;
			}
		}
		
		$rec_res = db_select('school_records', 'sr')
			->fields('sr')
			->condition('season', $season)
			->condition('sport', 'ncaaf')
			->condition('sid', $record->visiting_sid)
			->condition('tag', array('overall', 'conf', 'pythag'))
			->execute();
		foreach ($rec_res as $a) {
			$title_data['visiting_record'][$a->tag] = $a->wins . '-' . $a->losses;
			if ($a->tag == 'conf') {
				$title_data['visiting_record'][$a->tag] .= ' ' . $record->visiting_conf;
			}
		}
		
		// GAME TITLE
		$game_title = $record->visiting_school;
		if ($record->neutral == 'Y') {
			$record->vs_at_symbol = ' vs ';
		} else {
			$record->vs_at_symbol = ' at ';
		}
		$game_title .= $record->host_school;
		$record->game_title = $game_title;

		// BETTING LINE
		$record->bline = array();
		if ($record->betting_line) {
			$record->bline = explode(':', $record->betting_line);
		}

		$record->game_date_str = date('l, M j, Y', $record->game_date);
		$record->kickoff_time = date('g:i A', $record->game_date);
		if ($record->kickoff_time == '12:00 AM') {
			$record->kickoff_time = 'TBA';
		}
		$record->game_day = date('D', $record->game_date);
		
		//$stats_data = pickem_load_records_and_points_data(array($record->host_school_name, $record->visiting_school_name), $season);
		//dpm($stats_data);
		
		// VARIOUS other labels, etc
		$title_data['visiting_school_rank'] = $record->visiting_school_rank != '' ? ('#' . $record->visiting_school_rank . ' ') : '';
		$title_data['visiting_school'] = $record->visiting_school;
		$title_data['visiting_conf'] = $record->visiting_conf;
		$title_data['visiting_school_name'] = $record->visiting_school_name;
		$title_data['visiting_school_bline'] = (!empty($record->bline) && ($record->visiting_school_name == $record->bline[0]))
			? $record->bline[1] : '';
		$title_data['visiting_score'] = ($record->visiting_score != '') ? $record->visiting_score : '&ndash;';
		$title_data['host_school_rank'] = $record->host_school_rank != '' ? ('#' . $record->host_school_rank . ' ') : '';
		$title_data['host_school'] = $record->host_school;
		$title_data['host_conf'] = $record->host_conf;
		$title_data['host_school_name'] = $record->host_school_name;
		$title_data['host_school_bline'] = (!empty($record->bline) && ($record->host_school_name == $record->bline[0]))
			? $record->bline[1] : '';
		$title_data['host_score'] = ($record->host_score != '') ? $record->host_score : '&ndash;';
		$title_data['vs_at_symbol'] = $record->vs_at_symbol;
		$title_data['overtimes'] = ($record->overtimes != '' && !$record->overtimes > 0) ? $record->overtimes . 'OT' : '';
		if ($record->completed == 'Y') {
			if ($record->winning_school_name == $record->userpick_school_name) {
				$title_data['label_class'] = 'label-success';
				$title_data['pick_result'] = t('Correct');
			} else {
				$title_data['label_class'] = 'label-important';
				$title_data['pick_result'] = t('Wrong');
			}
		}
		$title_data['stats_data'] = FALSE;
		$record->title_data = $title_data;		
		
		
		// TODO -- remove (deprecated)
		$pickIsOpen = user_access('administer site configuration');
		$pickIsOpen = $pickIsOpen || 
			($_SESSION['current_week']->week == $week && (time() < $pickCutoffTime)) ||
			($_SESSION['current_week']->week + 1 == $week);
			
		//$record->pickIsOpen = $pickIsOpen;
		
		if (date('D', $record->game_date) == 'Sat' && is_null($saturday_cutoff)) {
			$saturday_cutoff = $record->game_date - (60 * variable_get('pickem.cutoff_minutes', 15));
			//$pickCutoffTime = $saturday_cutoff;
		}
		
		$cutoff_time_for_pick = '';
		$record->pickIsOpen = pickem_pick_is_open($record, $now, $saturday_cutoff, $cutoff_time_for_pick);
		$form_state['cutoff_time'][$record->gid] = $cutoff_time_for_pick;
		
		if ($record->pickIsOpen && $pickCutoffTime == 0) {
			// because the games results are sorted by game date, this is a safe
			// block of coding logic
			$pickCutoffTime = $cutoff_time_for_pick;
		}
		
		$gamerow_key = 'gid_' . $record->gid;
		$form[$gamerow_key] = array(
			'#type' => 'container',
			'#title' => t($game_title),
			'#attributes' => array('class' => array('pickem-entry', 'span6', 'well')),
			'#tree' => TRUE,
		);
		//if ($now > ($record->game_date - (variable_get('pickem.hours_to_warn', 8) * 3600))) {
		//	$form[$gamerow_key]['#attributes']['class'][] = 'warning';
		//}
		$form[$gamerow_key]['game-title'] = array(
			'#type' => 'markup',
			'#markup' => theme('game_title', 
				array('game' => $record, 
							'other_player' => $other_player,
						  'logobasepath' => '/' . drupal_get_path('module', 'pickem') . '/logos/ncaaf/')),
		);
		if ($clearrow) {
			$form[$gamerow_key]['#attributes']['class'][] = 'clear-row';
		}
		$clearrow = !$clearrow;
		
		# process the pick school name (when available)
		$form_state['picks'][$record->gid] = NULL;
		if ($record->pkid) {
			$ares = db_query('SELECT p.* FROM {pick} p WHERE pkid = :pkid', 
				array(':pkid' => $record->pkid));
				
			$thepick = $ares->fetchObject();
			if (!empty($thepick->pick_school_name)) {
				$validPicks++;
			}
			$form_state['picks'][$record->gid] = $thepick;
		} else {
			$form_state['picks'][$record->gid] = NULL;
		}
		
		$form[$gamerow_key]['pkid'] = array(
			'#type' => 'hidden',
			'#default_value' => $record->pkid ? $record->pkid : '',
		);

		// TODO -- remove this code (deprecated)
		$picksAreOpen = (time() < $pickCutoffTime) || user_access('administer site configuration');
		$record->picksAreOpen = $picksAreOpen;
		
		
		$num_picks_open += ($record->pickIsOpen) ? 1 : 0;
		$pickOptions = array();
		
		if (!$record->pickIsOpen) {
			
			// PREPARE THE <SELECT> element for a locked pick...
			$pick_school_element_classes = array('label', 'pick-result');
			$right_or_wrong = '';
			
			$form[$gamerow_key]['#attributes']['class'][] = 'control-group';
			if ($record->pick_is_correct == 'Y') {
				$form[$gamerow_key]['#attributes']['class'][] = 'success';
				$pick_school_element_classes[] = 'label-success';
				$right_or_wrong = 'correct';
			} else if ($record->pick_is_correct == 'N') {
				$pick_school_element_classes[] = 'label-important';
				$form[$gamerow_key]['#attributes']['class'][] = 'error';
				$right_or_wrong = 'wrong';
			}
			
			$settings['pick_count']++;
			if ($record->completed == 'Y') {
				$pickOptions[$record->pick_school_name] = $record->userpick_school . ' (' . $right_or_wrong . ')';
			} else {
				$pickOptions[''] = t('-- Select School --');
				$pickOptions[$record->visiting_school_name] = $record->visiting_school;
				$pickOptions[$record->host_school_name] = $record->host_school;
			}	
			$settings['games']['pick-' . $record->gid] = array(
				'host_school' => $record->host_school,
				'visiting_school' => $record->visiting_school,
			);
			if ($record->pkid && ($record->pick_school_name != '')) {
				$settings['games']['pick-'.$record->gid]['userpick_school'] = $record->userpick_school;
				if ($record->pick_school_name == $record->host_school_name) {
					$settings['games']['pick-'.$record->gid]['other_school'] = $record->visiting_school;
				} else {
					$settings['games']['pick-'.$record->gid]['other_school'] = $record->host_school;
				}
			} else {
				$settings['games']['pick-'.$record->gid]['userpick_school'] = '';
			}
			
			$form[$gamerow_key]['pick_school_name'] = array(
				'#type' => 'select',
				'#options' => $pickOptions,
				'#title' => t('Your Pick:'),
				'#title_display' => 'invisible',
				'#default_value' => $record->pkid ? $record->pick_school_name : '',
				'#disabled' => TRUE,
				'#attributes' => array(
					'class' => array('pick-school-name'),
					'id' => 'pick-' . $record->gid,
				)
			);
		} else {
			if ($week == $_SESSION['current_week']->week) {
				$settings['pick_count']++;
				$pickOptions[''] = t('-- Select School --');
				$pickOptions[$record->visiting_school_name] = $record->visiting_school;
				$pickOptions[$record->host_school_name] = $record->host_school;	
				$settings['games']['pick-' . $record->gid] = array(
					'host_school' => $record->host_school,
					'visiting_school' => $record->visiting_school,
				);
				if ($record->pkid && ($record->pick_school_name != '')) {
					$settings['games']['pick-'.$record->gid]['userpick_school'] = $record->userpick_school;
					if ($record->pick_school_name == $record->host_school_name) {
						$settings['games']['pick-'.$record->gid]['other_school'] = $record->visiting_school;
					} else {
						$settings['games']['pick-'.$record->gid]['other_school'] = $record->host_school;
					}
				} else {
					$settings['games']['pick-'.$record->gid]['userpick_school'] = '';
				}
			} else {
				$pickOptions[''] = t('-- Locked until week ' . $week . ' --');
			}
			$form[$gamerow_key]['pick_school_name'] = array(
				'#type' => 'select',
				'#options' => $pickOptions,
				'#title' => t('Your Pick:'),
				'#title_display' => 'invisible',
				'#default_value' => $record->pkid ? $record->pick_school_name : '',
				'#disabled' => ($_SESSION['current_week']->week != $week),
				'#attributes' => array(
					'class' => array('pick-school-name'),
					'id' => 'pick-' . $record->gid,
				)
			);
			/*
			$form[$gamerow_key]['pick_stats'] = array(
				'#type' => 'fieldset',
				'#collapsible' => TRUE,
				'#collapsed' => TRUE,
				'#title' => t('Pick Stats'),
				'#attributes' => array(
					'id' => 'pick-stats-' . $record->gid,
				)
			);
			 * 
			 */
		}
		### PICK SCHOOL NAME element ####
	}
	
	if ($week == $_SESSION['current_week']->week) {
		drupal_set_message('<strong>' . t('Deadline') . '</strong>' . t(' for picks is ') . strftime('<strong>%l:%M %p</strong> on  <strong>%a, %b %e, %Y</strong>', $pickCutoffTime), 'warning');
	}		
	
	if (($validPicks > 0) && ($validPicks < $totalGames)) {
		// create an element that sites above the games
		drupal_set_message(t('<strong>Warning!</strong> Please be reminded that you have not completed all of your picks'), 'warning');
	}
	$form['totalgames'] = array(
		'#type' => 'hidden',
		'#value' => $totalGames,
	);
	$form['actions'] = array(
		'#type' => 'actions',
		'#attributes' => array('class' => array('span12', 'clear-row')),
	);
	if (user_access('pickem player') && ($num_picks_open > 0)) {
	//if (user_access('pickem player') && !$pickemWeekIsLocked) {
		$form['actions']['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Save Picks'),
			'#attributes' => array(
				'class' => array('btn')
			),
			'#disabled' => ($_SESSION['current_week']->week != $week),
		);
	} else {
		if (!user_access('pickem player')) {
			$form['actions']['register'] = array(
				'#type' => 'markup',
				'#markup' => t('<p><a href="/user/register"><strong>Please register</strong></a> in order to save these picks and participate in the World\'s Greatest ACC Pickem Competition.</p>'),
			);
		} else if ($num_picks_open == 0) {
			$markup = '<p>' . t("This week's picks are now locked.  Good luck and score well.");
			if ($user->name != 'ecuacc4ever') {
				$markup .= '  ' . t('Just not as well as <strong><em>the champ</em></strong> (ecuacc4ever)');
			}
			$markup .= '</p>';
			
			$form['actions']['register'] = array(
				'#type' => 'markup',
				'#markup' => $markup,
			);
			
		}
	}
	$form['line_updated'] = array(
		'#type' => 'container',
		'#attributes' => array('class' => array('span12', 'clear-row')),
		'#weight' => 1000,
	);
	$form['line_updated']['message'] = array(
		'#type' => 'markup',
		'#markup' => t('<em>Betting lines last updated on: ' . variable_get('pickem.bettinglines_updated') . '</em>'),
	);
	$form['#attributes']['class'][] = 'clearfix';
	$form['#attached']['js'][] = drupal_get_path('module', 'pickem') . '/js/slate.js';
	
	$form['#attached']['js'][] = array('data' => array('pickem' => $settings), 'type' => 'setting');
	
	db_set_active();
	return $form;
}

function pickem_slate_listing_submit($form, $form_state) {
	global $user;

	drupal_get_messages(NULL, TRUE);
	$now = time();
	
	foreach ($form_state['values'] as $key => $value) {
		if (substr($key, 0, 3) == 'gid') {
			$gid_data = explode('_', $key);
			if ($now < $form_state['cutoff_time'][$gid_data[1]]) {
				if ($form_state['picks'][$gid_data[1]]) {
					// existing pick -- update
					$o = $form_state['picks'][$gid_data[1]];
					$o->pick_school_name = $value['pick_school_name'];
					$o->season = 2013;
					$c = db_update('pick')
						->fields(array(
							'pick_school_name' => $value['pick_school_name'],
							'season' => 2013,
						)
					)
					->condition('pkid', $form_state['picks'][$gid_data[1]]->pkid)
					->execute();
					
					// drupal_write_record('pick', $o, 'pkid');
				} else {
					// new pick
					$c = db_insert('pick')
						->fields(array(
							'pick_school_name' => $value['pick_school_name'],
							'uid' => $user->uid,
							'gid' => $gid_data[1],
							'week' => $form_state['values']['week'],
							'season' => '2013',
						)
					)->execute();
				}
			} else {
				drupal_set_message(t('Unable to accept your pick for ' . $value['pick_school_name'] . ' because the deadline has passed.'), 'error');
			}
		}
	}
	drupal_set_message(t('Your pick selections have been saved.'));
}

function pickem_slate_compare($form, &$form_state, $week = NULL) {
	global $user;
	
	$uid = $user->uid;
	$player_name = $user->name;
	if ($user->uid == 1) {
		$uid = 12;
		$player_name = 'ecuacc4ever';
	}
	
	if ($week == NULL) {
		$week = $_SESSION['current_week']->week;
	}
	$season = variable_get('pickem.current_season', date('Y'));

	#$pick_data = pickem_load_pick_data(-1, $season, array('uid' => $uid));
	$pick_data = pickem_load_pick_data($week, $season, array('ignore-player-stats'));

	$slate = array();
	foreach ($pick_data['game'] as $gid => $game) {
		if ($game['week'] == $week) {
			$slate[] = $game;
		}
	}

	if (isset($form_state['compare_uid'])) {
		#$compare_pick_data = pickem_load_pick_data(-1, $season, array('uid' => $form_state['compare_uid']));
		#dpm($compare_pick_data);	
	}
	
	// GET THE LIST of players..
	$pickem_players = pickem_load_standings(-1, $season);
	
	$options_players = array('-1' => '--- Select Player ---');
	foreach ($pickem_players as $pp) {
		if ($pp->uid == $uid) { continue; }
		$options_players[$pp->player_name] = $pp->player_name;
	}

	$form['compare_player'] = array(
		'#type' => 'select',
		'#options' => $options_players,
		'#title' => t('Select a player'),
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Go'),
	);
	$form['actions'] = array(
		'#type' => 'actions',
	);

	$rows = array();
	$header = array(
		'',
		'Me',
		$form_state['compare_player'],
	);
	if (isset($form_state['compare_player']) && isset($pick_data['player'][$form_state['compare_player']])) {
		$my_picks = $pick_data['player'][$player_name];
		$their_picks = $pick_data['player'][$form_state['compare_player']];
		$rows = array();
		foreach ($pick_data['game'] as $gid => $game) {
		
			$my_pick = $my_picks['week'][$week]['game'][$gid];
			$their_pick = $their_picks['week'][$week]['game'][$gid];
			$row_class = $my_pick == $their_pick ? '' : 'warning';

			if ($game['completed'] == 'Y') {
				$left_cell = t('<h3>@visiting_school - @visiting_score, @host_school - @host_score</h3>',
					array('@visiting_school' => $game['visiting_school'], 
								'@visiting_score' => $game['visiting_score'], 
								'@host_school' => $game['host_school'], 
								'@host_score' => $game['host_score']
								)
							);
			} else {
				$left_cell = t('<h3>@visiting_school @vs_at @host_school</h3>',
					array(
						'@visiting_school' => $game['visiting_school'], 
						'@vs_at' => $game['vs_at'], 
						'@host_school' => $game['host_school'], 
					)
				);
			}
			$time_cell = date('l, M j', $game['game_date']) . ' - ';
			if (date('g:i A', $game['game_date']) == '12:00 AM') {
				$time_cell .= 'TBA';
			} else {
				$time_cell .= date('g:i A', $game['game_date']);
			}

			$rows[] = array(
				'data' => array(
					$left_cell,
					'<img class="img-polaroid" src="/sites/all/modules/pickem/logos/ncaaf/' . $my_pick['pick_school_name'] . '.gif" width="60" />',
					'<img class="img-polaroid" src="/sites/all/modules/pickem/logos/ncaaf/' . $their_pick['pick_school_name'] . '.gif" width="60" />',
				),
				'class' => array($row_class),
			);
		}
		$form['comparison'] = array(
			'#theme' => 'table',
			'#header' => $header,
			'#rows' => $rows,
			'#attributes' => array('class' => array('table')),
		);
	}
	$form['comparison'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#empty' => t('No comparisons are available -- likely because the selected player has yet to submit his/her picks for this week.'),
		'#attributes' => array('class' => array('table')),
	);
	$form['#attributes']['class'][] = 'form-inline';
	return $form;
	
	#return array(
	#	'#theme' => 'slate_compare',
	#	'#slate' => $slate,
	#);
}

function pickem_slate_compare_submit($form, &$form_state) {
	if ($form_state['values']['compare_player'] != '-1') {
		$form_state['compare_player'] = $form_state['values']['compare_player'];
	} else {
		unset($form_state['compare_player']);
	}
	$form_state['rebuild'] = TRUE;
}

function theme_slate_compare($variables) {
	$slate = $variables['slate'];
	$slate_compare = $variables['slate_compare'];
	$games = $variables['games'];
	$week = $variables['week'];

	$out = '<h3>' . t('Pick Comparison') . '</h3>';

	$out = '<table class="table table-striped">';
	$out = '<tr><th width="50%">&nbsp;</th><th>Your Pick</th><th>' . $variables['compare_player'] . '</th></tr>';

	foreach ($games as $gid => $game) {

		$row_class = '';
		$out .= '<tr class="' . $row_class . '">';
		$out .= '<td width="50%">';
		
		if ($game['completed'] == 'Y') {
			$out .= t('<h3>@visiting_school - @visiting_score @host_school - @host_score</h3>',
				array('@visiting_school' => $game['visiting_school'], 
							'@visiting_score' => $game['visiting_score'], 
							'@host_school' => $game['host_school'], 
							'@host_score' => $game['host_score']
							)
						);
		} else {
			$out .= t('<h3>@visiting_school @vs_at @host_school</h3>',
				array(
					'@visiting_school' => $game['visiting_school'], 
					'@vs_at' => $game['vs_at'], 
					'@host_school' => $game['host_school'], 
				)
			)	. date('l, M j', $game['game_date']) . ' - ';
			
			//. '<h3>' . $game['visiting_school'] . '<br />' . $game['vs_at'] . ' ' . $game['host_school'] . $point_spread_str . '</h3>'
	
			if (date('g:i A', $game['game_date']) == '12:00 AM') {
				$out .= 'TBA';
			} else {
				$out .= date('g:i A', $game['game_date']);
			}
		}
		$out .= '</td>';
		
		$my_pick = $slate['week'][$week]['game'][$gid];
		$their_pick = $slate_compare['week'][$week]['game'][$gid];
		$out .= '<td>'
			. '<img class="img-polaroid" src="/sites/all/modules/pickem/logos/ncaaf/' . $my_pick['pick_school_name'] . '.gif" width="80" />&nbsp;'
			//. '<div style="font-size: 110%">Our Experts are evenly divided on both teams.</div>'
			. '</td>';

		$out .= '<td>'
			. '<img class="img-polaroid" src="/sites/all/modules/pickem/logos/ncaaf/' . $their_pick['pick_school_name'] . '.gif" width="80" />&nbsp;'
			//. '<div style="font-size: 110%"><strong>' . $pct . '% of our experts</strong> favor <strong>' . $our_favorite . '</strong> over ' . $other . '</div>'
			. '</td>';
		
		$out .= '</tr>';
		
		
		#$out .= '<li><h3>' . $game['visiting_school'] . ' vs ' . $game['host_school'] . '</h3>';
		#$out .= t('<strong>' . $pct . '% of our experts</strong> are picking <strong><em>@favorite</em></strong> over @other.', array('@favorite' => $our_favorite, '@other' => $other)) . '<br /></li>';
	}
	$out .= '</table>';
	#$out .= '</ul>';

	return $out;
}
