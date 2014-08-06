<?php

/**
 * page callback for pickem overview page
 * 
 * @return render array
 */
function pickem_pickem_overview($pname) {
	global $user;
	$page = array();
	
	$pickem = pickem_load_by_name($pname);
	dpm($pickem);
	
	#$pickem = PickemManager::loadPickem($pname);
	#$pickem_player = PickemManager::loadPickemPlayer($user->uid, $pickem->pid);
	
	if (is_null($pickem)) {
		// redirect to a "Invalid Pickem" page
	}
	
	if (!pickem_player_access($pickem->pid)) {
		// redirect to a page that informs the user he/she is not currently a player
		// and invite them to join (if pickem is accepting new players)
		// or..
		// add a page element that represents the above
		$page['not_current_player'] = array(
			'#theme' => 'not-a-current-player',
			'#pickem' => $pickem,
			'#user' => $user,
		);

		return $page;
	}
	
	
	//$gameEntries = GoogleSpreadsheetManager::fetchGamesSpreadsheetEntries();
	//dpm($gameEntries);
	//dpm(GoogleSpreadsheetManager::fetchSchoolsSpreadsheetEntries());
	$standingsTopTen = array();
	$upcomingGames = array();

	#$standingsTopTen = StandingsManager::getTopTen($pickem->pid);
	#$upcomingGames = SlateManager::slateForWeek($pickem->pid, 1, $pickem->season);
	
	$page['overview'] = array(
		'#theme' => 'pickem-overview',
		'#standingsTopTen' => $standingsTopTen,
		'#upcomingGames' => $upcomingGames,
		'#pickem' => $pickem,
	);
	//$page['theme_hook_suggestions'][] = 'page__pickem';
	
	return $page;
}

/**
 * page callback for pickem/%pickem/slate
 */
function pickem_pickem_slate($pickem) {
	$pickem_slate = $pickem->getPickemSlate();
	$currentWeek = $pickem->currentWeek();

	$slate_rows = array();
	foreach ($pickem_slate as $slid => $slate_record) {
		if (!isset($slate_rows[$slate_record->week])) {
			$slate_rows[$slate_record->week] = 0;
		}
		$slate_rows[$slate_record->week]++;
	}
	
	$header = array(
		array('data' => t('Week'), 'class' => array('text-center')),
		array('data' => t('# Games'), 'class' => array('text-center')),
		array('data' => '<span title="Correct" class="glyphicon glyphicon-thumbs-up"></span>', 'class' => array('text-center')),
		array('data' => '<span title="Incorrect" class="glyphicon glyphicon-thumbs-down"></span>', 'class' => array('text-center')),
		array('data' => t('Place'), 'class' => array('text-center')),
		array('data' => t('Actions')),
	);
	
	$rows = array();
	foreach ($slate_rows as $week => $event_count) {
		$row_class = $week == $currentWeek ? 'active' : '';
		$row = array(
			'data' => array(
				array('data' => $week, 'class' => array('text-center')),
				array('data' => $event_count, 'class' => array('text-center')),
				array('data' => t('&ndash;'), 'class' => array('text-center')),
				array('data' => t('&ndash;'), 'class' => array('text-center')),
				array('data' => t('&ndash;'), 'class' => array('text-center')),
			),
			'class' => array($row_class)
		);
		
		$linkText = '';
		$btnClasses = array();
		if ($pickem->slateWeekIsOpen($week)) {	// was '$week == $currentWeek'
			$linkText = t('Make Picks');
			$btnClasses = array('btn', 'btn-primary', 'btn-xs');
			//$row['data'][] = array(
			//	'data' => l(
			//		t('Make Picks'), 
			//		sprintf('pickem/%s/slate/%s', $pickem->pid, $week),
			//		array('attributes' => array('class' => array('btn', 'btn-primary', 'btn-xs')))
			//	)
			//);
		}
		else {
			$linkText = t('See Games');
			$btnClasses = array('btn', 'btn-info', 'btn-xs');
		}
		$row['data'][] = array(
			'data' => l(
				$linkText, 
				sprintf('pickem/%s/slate/%s', $pickem->pid, $week),
				array('attributes' => array('class' => $btnClasses))
			)
		);
		$rows[] = $row;
		unset($linkText);
		unset($btnClasses);
		//$row = array(
		//	'data' => array(
		//		array('data' => 'Could use this for the pick form...?', 'colspan' => 6)
		//	),
		//	'no_striping' => TRUE,
		//);
		//$rows[] = $row;
	}
	
	$page['slate'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array(
			'class' => array('table', 'table-bordered'),
		),
	);
	
	return $page;
}

/**
 * page callback for pickem slate
 * @param array $form
 * @param array $form_state
 * @param stdClass pickem
 * @param int week (optional)
 */
function pickem_pickem_slate_form($form, &$form_state, $pickem, $week = NULL) {
	global $user;
	if (!isset($user->pick_default)) {
		$user = user_load($user->uid, TRUE);
	}
	
	$user_is_logged_in = user_is_logged_in();
	
	// TODO redirect this page when a user (other than an admin)
	// is attempting to access a future week.
	// ..well maybe.
	// Maybe I can display the picks without buttons..
	
	$form['#parents'] = array();
	$form['#attached']['js'][] = drupal_get_path('module', 'pickem') . '/js/pickem-slate.js';
	$js_settings = array('week' => $week, 'pickem_id' => $pickem->pid, 'uid' => $user->uid);
	
	if (!$pickem->active()) {
		drupal_set_message('This Pickem contest is currently closed.  Please try again soon.');
		drupal_goto('<front>');
	}
	// do we show the edit controls (SELECT elements and BUTTON elements)
	$showEditControls = !(($user->pickems === FALSE || !array_key_exists($pickem->pid, $user->pickems)) && !in_array('administrator', $user->roles));
	$showEditControls = $showEditControls &&
		!(($user->pickems === FALSE || !array_key_exists($pickem->pid, $user->pickems)) && !in_array('administrator', $user->roles));
	
	#if (($user->pickems === FALSE || !array_key_exists($pickem->pid, $user->pickems)) && !in_array('administrator', $user->roles)) {
	#	drupal_set_message('You have reached an inactive or invalid Pick\'em Contest', 'error');
	#	drupal_goto('<front>');
	#}
	
	//if ($week > $pickem->currentWeek()) {
	//	drupal_set_message('That week is not ready for public viewing, yet.');
	//	drupal_goto(sprintf('pickem/%s/slate', $pickem->currentWeek()));
	//}
	
	$pickem_player = $pickem->getPlayer($user->uid);
	$form_state['storage']['pickem_player'] = $pickem_player;
	$week_slate = $pickem->getPickemSlateForWeek($week);
	
	#dpm(array('week_slate' => $week_slate, 'pickem' => $pickem, 'player' => $pickem_player));
	#dpm($week_slate);

	######### DEFAULT PICK TEAM value #########
	# This section establishes the default pick selection
	# when a player (user) has yet to make picks.
	#
	# If the user has established a default in his/her user profile
	# then use it.  Otherwise, use the pickem's default 
	$pickem_wrapper = entity_metadata_wrapper('pickem', $pickem);
	$max_bye_weeks_allowed = $pickem_wrapper->max_bye_weeks_allowed->value();
	$pick_default_pickem = $pickem_wrapper->pick_default->value();

	$user_wrapper = entity_metadata_wrapper('user', $user);
	$pick_default_user = $user_wrapper->pick_default->value();
	
	if (!empty($pick_default_user)) {
		$pick_default = $pick_default_user;
	}
	else {
		$pick_default = $pick_default_pickem;
	}
	######### END DEFAULT PICK TEAM value #########
	
	if (is_null($pickem_player)) {
		// prompt the player to enter settings.
		drupal_goto(sprintf('pickem/%s/player-setup', $pickem->name));
	}
	if ($week == NULL) {
		$week = $pickem->currentWeek();
	}
	$form_state['week'] = $week;
	
	
	// Get the event pickem slate for this $week
	// This will be an array of PickemSlate instances that will be 
	// referred to, later, as $event(s)
	$upcomingSlate = $pickem->getPickemSlateForWeek($week);

	if (count($upcomingSlate) == 0) {
		$form['no-slate-events'] = array(
			'#type' => 'markup',
			'#markup' => '<p>No events are available this week for <strong>' . $pickem->title . '</strong>',
		);
		return $form;
	}

	$target_team_name = '';

	// TODO load existing picks from the database
	
	$form['#attached']['js'][] = array(
		'data' => array('pickem' => $js_settings),
		'type' => 'setting',
	);
	
	# =========== BUILD THE FORM HERE ==============
	//$form['notes'] = array(
	//	'#theme' => 'pickem-form-notes',
	//);
	$form_state['pickem'] = $pickem;
	
	$timeInQuestion = time();
	
	#$fakeTime = mktime(8, 30, 0, 8, 30, 2014);
	#$timeInQuestion = $fakeTime;
	#$form_state['storage']['faketime'] = $fakeTime;
	#$form['fakedate'] = array(
	#	'#type' => 'container',
	#	'#attributes' => array(
	#		'class' => array('alert', 'alert-info'),
	#	),
	#);
	#$form['fakedate']['text'] = array(
	#	'#type' => 'markup',
	#	'#markup' => t('Simulating the Pickem Slate with a date of: ') . '<br /><strong>' . t(date('r', $fakeTime)) . '</strong>',
	#);
	
	$form['user-settings'] = array(
		'#type' => 'container',
	);
	$form['user-settings']['link'] = array(
		'#type' => 'markup',
		'#markup' => sprintf("<a class='btn btn-default' href=\"/user/%s/edit\"><span>Change Your Default Pick/Selection Setting</span></a><br />", $user->uid),
	);
	$form['betting-lines-disclaimer'] = array(
		'#type' => 'markup',
		'#markup' => '<p><strong><em>Note:</em></strong> - Betting lines are for display purposes only.  The picks are <strong>Straight Up</strong>.</p>',
	);
	$form['events']['#tree'] = TRUE;

	$form_state['storage']['events'] = array();
	
	foreach ($upcomingSlate as $slate_id => $event) {
		// TODO convert 'pick' into an entity-based object, then..
		$pick = $event->load_pick_for_user($user->uid);
		$pkid = isset($pick->pkid) ? $pick->pkid : 'new';
		$objid = $event->event_entity_id;
		$objtype = $event->event_entity_type;
		# $pkid = $event->pickId();
		
		$form['events'][$slate_id]['slate_event'] = array(
			'#theme' => 'slate-' . $objtype,
			'#objid' => $objid,
			'#event' => $event,
			'#prefix' => '<div id="slate-event-wrapper-' . $objid . '">',
			'#suffix' => '</div>',
		);
		$form['events'][$slate_id]['slate_event']['event_title'] = array(
			'#type' => 'item',
			'#markup' => sprintf('<div class="h4">%s<br /><small class="hidden-xs">%s</small></div>', $event->displayTitleWithLines(), $event->eventDate('sportsevent_date_medium')),
			'#attributes' => array(
				'pid' => '',
			),
		);
		$form['events'][$slate_id]['slate_event']['event_title_mobile'] = array(
			'#type' => 'item',
			'#markup' => sprintf('<div class="h5">%s<br /><small class="visible-xs">%s</small></div>', $event->displayTitleMobile(), $event->eventDate('small')),
			'#attributes' => array(
				'pid' => '',
			),
		);
		
		// buttons for each opponent
		/**
		 * TODO based on any <pickem>_pick_default settings
		 * select the appropriate school and update cache for user's pick
		 */
		$opponents = array();
		if ($event->completed()) {
			// display the sports event results (who won)
			$opponents['result'] = $event->eventResult();
		} else {
			foreach ($event->opponents() as $designation => $opponent) {
				$opponents[$opponent->name] = $opponent->displaytitle;
			}
		}
		
		# ask the PickemSlate controller about the "locked" status of the Slate Event
		$pickIsLocked = !(entity_get_controller('pickem_slate')->eventCanAcceptPicks( $event, $timeInQuestion, $week ));
		$pickCutoffTime = !(entity_get_controller('pickem_slate')->cutoffTimeForPick( $event, $week ));

		$slateWeekIsOpen = $pickem->slateWeekIsOpen($week);
		
		################ PICK TEAM SELECT ELEMENT ##################
		### Generate the element for the Pick Team (if the slate is open)
		if ($slateWeekIsOpen) {
			// SELECT element for the pick
			$form['events'][$slate_id]['slate_event']['pick_team_name'] = array(
				'#type' => 'select',
				'#options' => $opponents,
				'#description' => $pickIsLocked ? t('This pick would be locked in a real time scenario') : '',
				'#disabled' => $pickIsLocked,
				//'#disabled' => !$showEditControls || $pickIsLocked,
			);
			$form_state['storage']['events'][$slate_id] = $event;
			$form_state['storage']['pick-cutoff-time'][$slate_id] = $pickCutoffTime;
			
			if (!$event->completed()) {
				$form['events'][$slate_id]['slate_event']['pick_team_name']['#empty_value'] = '';
				$form['events'][$slate_id]['slate_event']['pick_team_name']['#empty_option'] = 'Pick Your Team';
			}
			
			if (($pick_team_name = $event->pick_team_name($user->uid)) != '') {
				$form['events'][$slate_id]['slate_event']['pick_team_name']['#default_value'] = $pick_team_name;
			} else {
				
				// DEPRECATED now that the "Open Week" batch job provisions the picks.
				
				// CURRENTLY not supporting 'topranked'
				#if ($pick_default == 'hometeam') {
				#	$target_team_name = $event->sportsEvent->host_team_name;
				#}
				#else {
				#	$target_team_name = $event->sportsEvent->visiting_team_name;
				#}
				#$form['events'][$slate_id]['slate_event']['pick_team_name']['#default_value'] = $target_team_name;
			}
			// END SELECT for the pick
		} else {
			$form['events'][$slate_id]['slate_event']['pick_team_name'] = array(
				'#type' => 'item',
			);
			
		}
		############### END PICK TEAM ELEMENT
		
		// BUTTON for "pick stats"
		if ($slateWeekIsOpen) {
			$form['events'][$slate_id]['slate_event']['show-pick-stats']['button'] = array(
				'#theme' => 'bs-modal-trigger',
				'#data_target' => '.slate-pick-stats',
				'#btn_display_class' => 'btn-info',
				'#target_entity_id' => $slate_id,
				'#pickem_id' => $pickem->pid,
			);
		}
	}

	$form['events']['slate-pick-stats']['modal'] = array(
		'#theme' => 'modal-pick-stats',
	);

	# ============= ACTION BUTTONS ==============
	
	$form['actions'] = array(
		'#type' => 'actions'
	);
	if ($user->pickems === FALSE) {
		// add a "Join" button
		$form['actions']['join-pickem'] = array(
			'#type' => 'submit',
			'#pickem' => $pickem,
			'#value' => t('Join This Pickem'),
			'#attributes' => array(
				'class' => array('btn','btn-warning'),
			),
		);
		
	}
	if ($slateWeekIsOpen) {
		$form['actions']['finalizepicks'] = array(
			'#type' => 'submit',
			'#value' => t('Submit Picks'),
			'#attributes' => array(
				'class' => array('btn','btn-primary'),
				'id' => 'submit-the-picks',
			),
			'#disabled' => !$showEditControls,
		);
		$form['actions']['clearpicks'] = array(
			'#type' => 'submit',
			'#value' => t('Clear/Reset Picks'),
			'#disabled' => !$showEditControls,
			'#attributes' => array(
				'class' => array('btn','btn-default'),
			),
		);
	}
	$form['pickem_id'] = array(
		'#type' => 'hidden',
		'#value' => $pickem->pid,
	);
	$form['submittingModal'] = array(
		'#type' => 'markup',
		'#markup' => theme('submitting-picks', array()),
	);
	
	# finally, return the form for display
	return $form;
}

/**
 * Validation callback for the slate form
 */
function pickem_pickem_slate_form_validate($form, &$form_state) {
	// get the current time
	$time = time();	
	if (isset($form_state['storage']['faketime'])) {
		$time = $form_state['storage']['faketime'];
	}
	//form_set_error(NULL, 'Make some changes, yo!');
	//dpm($form_state['values']);
}

/**
 * Page callback for submitting the slate form
 */
function pickem_pickem_slate_form_submit($form, &$form_state) {
	global $user;

	// @TODO
	// process values to find picks
	// match them up with the cached values (if needed)
	// store to the database
	// reset the cache.

	if ($form_state['values']['op'] == 'Join This Pickem') {
		$clickedButton = $form_state['clicked_button'];
		drupal_goto(sprintf('pickem/%s/join', $clickedButton['#pickem']->pid));
	}
	
	if ($form_state['values']['op'] == 'Clear/Reset Picks') {
		return;
	}
	
	//dpm($form_state['values']['events']);
	$picks_processed = 0;
	$events = $form_state['values']['events'];
	foreach ($events as $slate_id => $event_element) {
		$slateEvent = $form_state['storage']['events'][$slate_id];

		if (!isset($slateEvent->picks[$user->uid])) {
			# TODO create new entity for this pick and save it
			$pick = entity_create('pick', 
				array(
					'slid' => $slate_id, 
					'pick_team_name' => $event_element['slate_event']['pick_team_name'],
					'playername' => $user->name,
					'uid' => $user->uid,
				)
			);
			if ($pick->save() !== FALSE) {
				drupal_set_message('Pick saved');
				$picks_processed++;
			}
		} else {
			// update this user's pick
			$pick = $form_state['storage']['events'][$slate_id]->picks[$user->uid];
			if ($pick->pick_team_name != $event_element['slate_event']['pick_team_name']) {
				$pick->pick_team_name = $event_element['slate_event']['pick_team_name'];
				if ($pick->save() !== FALSE) {
					drupal_set_message(sprintf("Updated your pick for %s to '%s'", $slateEvent->sportsEvent->displayTitle(), $slateEvent->{$pick->pick_team_name}));
					$picks_processed++;
				}
			}
		}
	}

	$week = $form_state['week'];
		
	if ($picks_processed) {
		$pickem_player = $form_state['storage']['pickem_player'];
		$pickem_player->player_data['madepicks'][$week] = TRUE;
		$pickem_player->player_data['last_picksave'] = time();
		$pickem_player->weeks_won = 0;
		try {
			entity_save('pickem_player', $pickem_player);
		} catch(Exception $ex) {
			drupal_set_message($ex->getMessage(), 'error');
		}
	}
	// the picks are in the cache for this $user
	//dpm()
	#$cache_key = sprintf('picks.%s', $user->uid);
	#sleep(5);
	#dpm($form_state);
}

/**
 * Page callback for player setup form
 */
function pickem_pickem_player_setup_form($form, &$form_state) {
	// load pickem defaults for a player
	// (database or plugin file)
}

/**
 * page callback for provisioning a users picks for a week
 */
function pickem_pickem_provision_slate($pickem, $week) {
	global $user;
	if (!isset($user->pick_default)) {
		$user = user_load($user->uid, TRUE);
	}

	$pickem_wrapper = entity_metadata_wrapper('pickem', $pickem);
	$pick_default_pickem = $pickem_wrapper->pick_default->value();

	$user_wrapper = entity_metadata_wrapper('user', $user);
	$pick_default_user = $user_wrapper->pick_default->value();
	
	if (!empty($pick_default_user)) {
		$pick_default = $pick_default_user;
	}
	else {
		$pick_default = $pick_default_pickem;
	}
	
	$slate_week = $pickem->getPickemSlateForWeek($week);
	foreach ($slate_week as $slid => $event) {
		// CURRENTLY not supporting 'topranked'
		if ($pick_default == 'hometeam') {
			$key = sprintf('#edit-events-%s-slate-event-pick-team-name-%s', $event->event_entity_id, $event->sportsEvent->host_team_name);
			$target_team_name = $event->sportsEvent->host_team_name;
		}
		else {
			$key = sprintf('#edit-events-%s-slate-event-pick-team-name-%s', $event->event_entity_id, $event->sportsEvent->visiting_team_name);
			$target_team_name = $event->sportsEvent->visiting_team_name;
		}
		$selectors[] = $key;
		_pickem_cache_pick($pickem, $slid, $week, $event->event_entity_id, $event->event_entity_type, 'new', $target_team_name);
	}
	
	
	// TODO return an array of CSS selectors in the format/convention of 
	// 'edit-events-%slid-slate-event-pick-team-name-%team, where
	// $slid is the pickem_slate ID and %team is the team->name
	
	//watchdog('pickem', 'called provisioning...');
	$json_output = drupal_json_output(array(
		'hello' => 'world', 
		'pickem' => $pickem->name, 
		'pick_default_user' => $pick_default_user,
		'pick_default_pickem' => $pick_default_pickem,
		'selectors' => $selectors,));
	
	return $json_output;
}

/**
 * page callback for pickem standings
 */
function pickem_pickem_standings($pid) {
	
	return array();
}
