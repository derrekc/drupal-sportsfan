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
		array('data' => t('Number of Games'), 'class' => array('text-center')),
		array('data' => t('Correct'), 'class' => array('text-center')),
		array('data' => t('Wrong'), 'class' => array('text-center')),
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
		if ($week == $currentWeek) {
			$row['data'][] = array(
				'data' => l(
					t('Make Picks'), 
					sprintf('pickem/%s/slate/%s', $pickem->pid, $week),
					array('attributes' => array('class' => array('btn', 'btn-primary', 'btn-xs')))
				)
			);
		}
		else {
			$row['data'][] = '';
		}
		$rows[] = $row;
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
	if (($user->pickems === FALSE || !array_key_exists($pickem->pid, $user->pickems)) && !in_array('administrator', $user->roles)) {
		drupal_set_message('You have reached an inactive or invalid Pick\'em Contest', 'error');
		drupal_goto('<front>');
	}
	
	$pickem_player = $pickem->getPlayer($user->uid);
	$week_slate = $pickem->getPickemSlateForWeek($week);
	
	#dpm(array('week_slate' => $week_slate, 'pickem' => $pickem, 'player' => $pickem_player));
	#dpm($week_slate);

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
	
	if (is_null($pickem_player)) {
		// prompt the player to enter settings.
		drupal_goto(sprintf('pickem/%s/player-setup', $pickem->name));
	}
	if ($week == NULL) {
		$week = $pickem->currentWeek();
	}
	$form_state['week'] = $week;
	
	// ========= LOAD CACHE DATA ==========
	$cache_key = sprintf('picks.%s', $user->uid);
	$cache_picks = NULL;
	if ($cache = cache_get($cache_key)) {
		$cache_picks = $cache->data;
	}
	#dpm($cache_picks);
	
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

	// TODO - I think it'll be best to empty the user's pick cache here.
	// _pickem_clear_cache($user->uid);

	$target_team_name = '';
	/*
	foreach ($upcomingSlate as $slid => $event) {
		if (!isset($event->picks[$user->uid])) {
			$pkid = 'new';
			// CURRENTLY not supporting 'topranked'
			if ($pick_default == 'hometeam') {
				$target_team_name = $event->sportsEvent->host_team_name;
			}
			else {
				$target_team_name = $event->sportsEvent->visiting_team_name;
			}
		}
		else {
			$pkid = $event->picks[$user->uid]->pkid;
			$target_team_name = $event->picks[$user->uid]->pick_team_name;
		}
		_pickem_cache_pick($pickem, $slid, $week, $event->event_entity_id, $event->event_entity_type, 'new', $target_team_name);
	}
	*/
	/*
	if (empty($cache_picks) && $pickem->userNeedsProvisioning($user->uid, $week)) {

		foreach ($upcomingSlate as $slid => $event) {
			// CURRENTLY not supporting 'topranked'
			if ($pick_default == 'hometeam') {
				$target_team_name = $event->sportsEvent->host_team_name;
			}
			else {
				$target_team_name = $event->sportsEvent->visiting_team_name;
			}
			_pickem_cache_pick($pickem, $slid, $week, $event->event_entity_id, $event->event_entity_type, 'new', $target_team_name);
		}
		
		//$form['provisioning-needed'] = array(
		//	'#theme' => 'modal-provisioning-needed',
		//	'#pick_default' => $pick_default,
		//);
		//$js_settings['provisioningNeeded'] = true;
		//$js_settings['provisioning_path'] = sprintf("pickem/%s/provision-slate/%s", $pickem->pid, $week);
	}		
	*/

	// TODO load existing picks from the database
	
	$form['#attached']['js'][] = array(
		'data' => array('pickem' => $js_settings),
		'type' => 'setting',
	);
	
	# =========== BUILD THE FORM HERE ==============
	$form['notes'] = array(
		'#theme' => 'pickem-form-notes',
	);
	$form_state['pickem'] = $pickem;
	$form['events']['#tree'] = TRUE;

	$form_state['storage']['events'] = array();
	foreach ($upcomingSlate as $slate_id => $event) {
		// TODO convert 'pick' into an entity-based object, then..
		$pick = $event->load_pick_for_user($pickem_player->uid);
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
			'#markup' => sprintf('<div class="h4">%s<br /><small class="hidden-xs">%s</small></div>', $event->displayTitle(), $event->eventDate('long')),
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
		foreach ($event->opponents() as $designation => $opponent) {
			$opponents[$opponent->name] = $opponent->displaytitle;
		}
				
		$form['events'][$slate_id]['slate_event']['pick_team_name'] = array(
			'#type' => 'select',
			'#options' => $opponents,
			'#empty_value' => 'none',
		);
		if (($pick_team_name = $event->pick_team_name($user->uid)) != '') {
			$form['events'][$slate_id]['slate_event']['pick_team_name']['#default_value'] = $pick_team_name;
			$form_state['storage']['events'][$slate_id] = $event;
		} else {
			// CURRENTLY not supporting 'topranked'
			if ($pick_default == 'hometeam') {
				$target_team_name = $event->sportsEvent->host_team_name;
			}
			else {
				$target_team_name = $event->sportsEvent->visiting_team_name;
			}
			$form['events'][$slate_id]['slate_event']['pick_team_name']['#default_value'] = $target_team_name;
		}
		/*
		foreach ($event->opponents() as $designation => $opponent) {
			$form['events'][$objid]['slate_event']['pick_team_name'][$opponent->name] = array(
				'#type' => 'button',
				'#value' => t(sprintf("%s", $opponent->displaytitle)),
				'#attributes' => array(
					'class' => array('btn', 'btn-sm', 'btn-block'),
					'pkid' => $pkid,
				),
				'#name' => sprintf('slate-op-%s-%s', $objid, $opponent->name),
				'#pkid' => $pkid,
				'#objid' => $objid,
				'#objtype' => $objtype,
				'#slid' => $slate_id,
				'#team_name' => $opponent->name,
				'#ajax' => array(
					'callback' => 'pickem_pick_team_name',
					'method' => 'replace',
					'wrapper' => 'slate-event-wrapper-' . $objid,
					#'wrapper' => 'edit-debugme',
					'progress' => array('type' => 'throbber', 'message' => t('Please wait...')),
				),
			);
			$addButtonClass = 'btn-default';
			if ($opponent->name == $event->pick_team_name($user->uid)) {
				$addButtonClass = 'btn-info';
			}
			else {
				if ($cache_picks) {
					if (isset($cache_picks[$pkid])) {
						if ($pkid == 'new') {
							dpm($pkid);
							dpm($objid);
							// each key in 'new' is a game/tourney/series id
							$addButtonClass = isset($cache_picks[$pkid][$objid]) 
								&& $cache_picks[$pkid][$objid]->pick_team_name == $opponent->name 
								? 'btn-info' 
								: 'btn-default';
						}
						else {
							$addButtonClass = 'btn-info';
							$form['events'][$objid]['slate_event']['#attributes']['class'][] = 'bg-warning';
						}
					}
				}
			}
			$form['events'][$objid]['slate_event']['pick_team_name'][$opponent->name]['#attributes']['class'][] = $addButtonClass;
		}
		*/
	}
	# ============= ACTION BUTTONS ==============
	
	$form['actions'] = array(
		'#type' => 'actions'
	);
	$form['actions']['finalizepicks'] = array(
		'#type' => 'submit',
		'#value' => t('Submit Picks'),
		'#attributes' => array(
			'class' => array('btn','btn-primary'),
			'id' => 'finalize-the-picks',
		),
	);
	$form['actions']['clearpicks'] = array(
		'#type' => 'submit',
		'#value' => t('Clear/Reset Picks'),
		'#attributes' => array(
			'class' => array('btn','btn-default'),
		),
	);
	$form['pickem_id'] = array(
		'#type' => 'hidden',
		'#value' => $pickem->pid,
	);
	return $form;
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

	if ($form_state['values']['op'] == 'Clear/Reset Picks') {
		return;
	}
	
	dpm($form_state['values']['events']);
	$events = $form_state['values']['events'];
	foreach ($events as $slate_id => $event_element) {
		if (!array_key_exists($slate_id, $form_state['storage']['events'])) {
			# TODO create new entity for this pick and save it
			$pick = entity_create('pick', 
				array(
					'slid' => $slate_id, 
					'pick_team_name' => $event_element['slate_event']['pick_team_name'],
					'uid' => $user->uid,
				)
			);
			if ($pick->save() !== FALSE) {
				drupal_set_message('Pick saved');
			}
		}
	}
	dpm($form_state['storage']['events']);
	
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
