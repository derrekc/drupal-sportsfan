<?php

/**
 * page callback for pickem overview page
 * 
 * @return render array
 */
function pickem_pickem_overview($pname) {
	global $user;
	$page = array();
	
	$pickem = PickemManager::loadPickem($pname);
	$pickem_player = PickemManager::loadPickemPlayer($user->uid, $pickem->pid);
	
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

	$standingsTopTen = StandingsManager::getTopTen($pickem->pid);
	$upcomingGames = SlateManager::slateForWeek($pickem->pid, 1, $pickem->season);
	
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
 * page callback for pickem slate
 * @param array $form
 * @param array $form_state
 * @param stdClass pickem
 * @param int week (optional)
 */
function pickem_pickem_slate_form($form, &$form_state, $pname, $week = NULL) {
	global $user;
	$pickem = PickemManager::loadPickem($pname);
	$form['#parents'] = array();
	$form['#attached']['js'][] = drupal_get_path('module', 'pickem') . '/js/pickem-slate.js';
	$js_settings = array('week' => $week, 'pickem_id' => $pickem->pid, 'uid' => $user->uid);
	
	// Send the user back to the front page if this pickem is either invalid or inactive
	if (is_null($pickem) || (!$pickem->active() && !in_array('administrator', $user->roles))) {
		drupal_set_message('You have reached an inactive or invalid Pick\'em Contest', 'error');
		drupal_goto('<front>');
	}
	
	$pickem_player = PickemManager::loadPickemPlayer($user->uid, $pickem->pid);
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
	
	$upcomingSlate = SlateManager::slateForWeek($pickem->pid, 1, $pickem->season);
	if (count($upcomingSlate) == 0) {
		$form['no-slate-events'] = array(
			'#type' => 'markup',
			'#markup' => '<p>No events are available this week for <strong>' . $pickem->title . '</strong>',
		);
		return $form;
	}

	if (PickemManager::userNeedsProvisioning($pickem->pid, $user->uid, $week)) {
		/*
		$form['provisioning-needed'] = array(
			'#theme' => 'modal-provisioning-needed'
		);
		$js_settings['provisioningNeeded'] = true;
		$js_settings['provisioning_path'] = sprintf("%s/pickem/%s/provision-slate/%s", $GLOBALS['base_url'], $pickem->name, $week);
		*/
	}		
	
	$field_key = sprintf('%s_user_default_pick', $pname);
	$field = field_info_field($field_key);
	if ($field) {
		$instance = field_info_instance('user', $field_key, 'user');
		$items = field_get_items('user', $user, $field_key);
		dpm(array('instance' => $instance, 'items' => $items));
	}
	
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

	foreach ($upcomingSlate as $event) {
		$objid = $event->sportEventId();
		$objtype = $event->sportEventType();
		$pkid = $event->pickId();
		
		$form['events'][$objid]['slate_event'] = array(
			'#theme' => 'slate-' . $objtype,
			'#objid' => $objid,
			'#event' => $event,
			'#prefix' => '<div id="slate-event-wrapper-' . $objid . '">',
			'#suffix' => '</div>',
		);
		$form['events'][$objid]['slate_event']['event_title'] = array(
			'#type' => 'item',
			'#markup' => sprintf('<div class="h4">%s<br /><small class="hidden-xs">%s</small></div>', $event->displayTitle(), $event->eventDate('long')),
			'#attributes' => array(
				'pid' => '',
			),
		);
		$form['events'][$objid]['slate_event']['event_title_mobile'] = array(
			'#type' => 'item',
			'#markup' => sprintf('<div class="h5">%s<br /><small class="visible-xs">%s</small></div>', $event->displayTitleMobile(), $event->eventDate('small')),
			'#attributes' => array(
				'pid' => '',
			),
		);
		
		foreach ($event->opponents() as $opponent) {
			$form['events'][$objid]['slate_event']['pick_school_name'][$opponent->name] = array(
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
				'#school_name' => $opponent->name,
				'#ajax' => array(
					'callback' => 'pickem_pick_school_name',
					'method' => 'replace',
					'wrapper' => 'slate-event-wrapper-' . $objid,
					#'wrapper' => 'edit-debugme',
					'progress' => array('type' => 'throbber', 'message' => t('Please wait...')),
				),
			);
			$addButtonClass = 'btn-default';
			if ($opponent->name == $event->pick_school_name()) {
				$addButtonClass = 'btn-info';
			}
			else {
				if ($cache_picks) {
					if (isset($cache_picks[$pkid])) {
						if ($pkid == 'new') {
							// each key in 'new' is a game/tourney/series id
							$addButtonClass = isset($cache_picks[$pkid][$objid]) 
								&& $cache_picks[$pkid][$objid]->pick_school_name == $opponent->name 
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
			$form['events'][$objid]['slate_event']['pick_school_name'][$opponent->name]['#attributes']['class'][] = $addButtonClass;
		}

	}
	# ============= ACTION BUTTONS ==============
	
	$form['actions'] = array(
		'#type' => 'actions'
	);
	$form['actions']['finalizepicks'] = array(
		'#type' => 'submit',
		'#value' => t('Finalize Picks'),
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

	$cache_key = sprintf('picks.%s', $user->uid);
	sleep(5);
	dpm($form_state);
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
function pickem_pickem_provision_slate($pname, $week) {
	sleep(10);
	watchdog('pickem', 'called provisioning...');
	return drupal_json_output(array('hello' => 'world', 'pname' => $pname));
}

/**
 * page callback for pickem standings
 */
function pickem_pickem_standings($pid) {
	
	return array();
}
