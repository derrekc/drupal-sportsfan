<?php

/**
 * page callback for pickem overview page
 * 
 * @return render array
 */
function pickem_pickem_overview($pickem) {
	global $user;
	$player = $pickem->playerByUID($user->uid);
	
	$page = array();
	
	//dpm($pickem);
	
	if (is_null($pickem)) {
		// redirect to a "Invalid Pickem" page
	}
	
	if (!$player) {
		// redirect to a page that informs the user he/she is not currently a player
		// and invite them to join (if pickem is accepting new players)
		// or..
		// add a page element that represents the above
		drupal_goto('pickem');
		return;
	}
	
	$currentWeek = $pickem->currentWeek();
	$currentStandingsByPlayer = array();
	if (($currentWeek - 1) != 0) {
		$currentStandingsByPlayer = $pickem->standingsByPlayerName[$currentWeek - 1];
	}
	else {
		$currentStandingsByPlayer = $pickem->standingsByPlayerName[0];
	}

	$standingsTopTen = array();
	$currentStandings = $pickem->currentStandings();
	$upcomingGames = array();
	$playerStandings = $pickem->playerResultsForWeek();
	
	$currentWeekResults = array();
	foreach ($playerStandings as $s) {
		$data = array();
		$data['playername'] = $s->playername;
		if (!key_exists($s->playername, $currentStandingsByPlayer)) {
			continue;
		}
		$data['overall-correct'] = $s->correct + $currentStandingsByPlayer[$s->playername]['correct'];
		$data['overall-incorrect'] = $s->incorrect + $currentStandingsByPlayer[$s->playername]['incorrect'];
		$data['correct'] = $s->correct;
		$data['incorrect'] = $s->incorrect;
		$currentWeekResults[$s->playername] = $data;
	}
	
	$sort = function($a, $b) {
		if ($a['overall-correct'] == $b['overall-correct']) {
			if ($a['correct'] == $b['correct']) {
				return $a['playername'] < $b['playername'] ? -1 : 1;
			}
			return ($a['correct'] > $b['correct']) ? -1 : 1;
		}
		return $a['overall-correct'] > $b['overall-correct'] ? -1 : 1;
	};
	uasort($currentWeekResults, $sort);
	
	$standings = $pickem->standings();
	$standingsTopTen = array();
	if (!empty($standings)) {
		$standingsTopTen = array_slice($standings, 0, 10);
	}
	$projectedStandings = array();
		
	$page['overview'] = array(
		'#theme' => 'pickem-overview',
		'#standingsTopTen' => $standingsTopTen,
		'#currentWeekResults' => $currentWeekResults,
		'#pickem' => $pickem,
		'#pickemPlayer' => $player,
		'#playerStandings' => $playerStandings,
		'#projectedStandings' => $projectedStandings,
	);
	//$page['theme_hook_suggestions'][] = 'page__pickem';
	
	return $page;
}

/**
 * page callback for pickem/%pickem/slate
 */
function pickem_pickem_slate($pickem) {
	global $user;
	
	$pickem_slate = $pickem->getPickemSlate();
	$currentWeek = $pickem->currentWeek();
	$player = $pickem->playerByUID($user->uid);
	
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
		$pickem_result = $player->pickemResultForWeek($pickem->pid, $week);
		
		$row_class = $week == $currentWeek ? 'active' : '';
		$row = array(
			'data' => array(
				array('data' => $week, 'class' => array('text-center')),
				array('data' => $event_count, 'class' => array('text-center')),
				array('data' => $pickem_result ? $pickem_result->correct : t('&ndash;'), 'class' => array('text-center')),
				array('data' => $pickem_result ? $pickem_result->incorrect : t('&ndash;'), 'class' => array('text-center')),
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
	
	// TODO
	// determine what type of pickem this is and 
	// perform initialization stuff
	#if ($pickem->pickem_type == 'weekly-pickem') {
	#}
	$current_title = drupal_get_title();
	
	// do we show the edit controls (SELECT elements and BUTTON elements)
	$showEditControls = TRUE;
	
	if (!user_is_logged_in()) { // anonymous user
		$showEditControls = FALSE;
	}
	else {
		$showEditControls = !(($user->pickems === FALSE || !array_key_exists($pickem->pid, $user->pickems)) && !in_array('administrator', $user->roles));
		$showEditControls = $showEditControls &&
			!(($user->pickems === FALSE || !array_key_exists($pickem->pid, $user->pickems)) && !in_array('administrator', $user->roles));
	}		
	
	$pickem_player = $pickem->getPlayer($user->uid);
	$showEditControls = $showEditControls && ($pickem_player->status == 1);
	
	$form_state['storage']['pickem_player'] = $pickem_player;
	
	if (is_null($pickem_player)) {
		// prompt the player to enter settings.
		drupal_goto(sprintf('pickem/%s/join', $pickem->name));
	}
	
	// TODO change this to accommodate different 
	// pickemk types
	// 'week' needs to be week or round
	if ($week == NULL) {
		$week = $pickem->currentWeek();
	}
	$form_state['week'] = $week;
	$form['#week'] = $week;
	$form['#pickem'] = $pickem;
	
	// Get the event pickem slate for this $week
	// This will be an array of PickemSlate instances that will be 
	// referred to, later, as $event(s)
	$upcomingSlate = $pickem->getPickemSlateForWeek($week);
	#$upcomingSlate = module_invoke_all('load_pickem_slate', $pickem->pid, $week);
	#if (empty($upcomingSlate)) {
	#	$upcomingSlate = $pickem->getPickemSlateForWeek($week);
	#}
	if (count($upcomingSlate) == 0) {
		$form['no-slate-events'] = array(
			'#type' => 'markup',
			'#markup' => '<p>No events are available this week for <strong>' . $pickem->title . '</strong>',
		);
		return $form;
	}

	// change the form theme for me (testing)
	if ($user->name == 'ecuacc4ever') {
		//$form['#theme'] = 'pickem_ecuacc4ever_slate_form';
	} // end change form
	
	$target_team_name = '';

	// TODO load existing picks from the database
	
	$form['#attached']['js'][] = array(
		'data' => array('pickem' => $js_settings),
		'type' => 'setting',
	);
	
	# =========== BUILD THE FORM HERE ==============
	$form_state['pickem'] = $pickem;
	
	$timeInQuestion = time();
	
	## extra navigation
	$related_nodes = FALSE;
	$term = reset(taxonomy_get_term_by_name(sprintf('%s-week-%s', $pickem->sport, $week), 'tags'));
	if ($term != NULL) {
		$nids = taxonomy_select_nodes($term->tid, FALSE, 10);
		$related_nodes = node_load_multiple($nids);
	}
	
	$form['navbar'] = array(
		'#theme' => 'pickemform_navbar',
		'#pickem_player' => $pickem_player,
		'#pickem' => $pickem, 
		'#week' => $week,
		'#related_nodes' => $related_nodes,
	);
	
	if ($pickem->slateWeekIsOpen($week)) {
		$form['betting-lines-disclaimer'] = array(
			'#type' => 'markup',
			'#markup' => '<p>The picks are <strong>straight up</strong>.'
				. '<br /><strong><em>Note:&nbsp;</em></strong>Betting lines are for display purposes only.  '
				. '<a href="#" class="ptspread-toggle">Toggle Point Spread Display</a>'
				. '</p>',
			'#prefix' => '<div class="well">',
			'#suffix' => '</div>',
		);
	}
	$form['events'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'class' => array('container-fluid'),
		),
		'#tree' => TRUE,
	);
	#$form['events']['#tree'] = TRUE;

	$form_state['storage']['events'] = array();
			
	// css class defaults for the event->displaytitle and the SELECT element
	$event_title_css_class = array('hidden-xs', 'col-sm-5');
	$event_title_mobile_css_class = array('col-xs-5', 'visible-xs');
	$event_selectelem_css_class = array('col-xs-5', 'col-md-4');

	$dateHeader = '';
	$weight = 1;
	foreach ($upcomingSlate as $slate_id => $event) {
		// TODO convert 'pick' into an entity-based object, then..
		$pick = $event->load_pick_for_user($user->uid);
		$pkid = isset($pick->pkid) ? $pick->pkid : 'new';
		$objid = $event->event_entity_id;
		$objtype = $event->event_entity_type;
		
		if ($dateHeader != $event->eventDate('sports_event_date_only')) {
			$dateHeader = $event->eventDate('sports_event_date_only');
			$currentDate = $event->getSportsEvent()->event_date;
		}
		$container_css_class = '';
		if ($event->picksAreEvenlySplit()) {
			$container_css_class = 'bg-warning';
		}
		$form['events'][$slate_id]['slate_event'] = array(
			'#theme' => 'slate-' . $objtype,
			'#weight' => $weight++,
			'#objid' => $objid,
			'#event' => $event,
			'#date_header' => $dateHeader,
			'#event_title_css_class' => $event_title_css_class,
			'#event_title_mobile_css_class' => $event_title_mobile_css_class,
			'#event_selectitem_css_class' => $event_selectelem_css_class,
			'#show_even_split_badge' => $event->picksAreEvenlySplit(),
			'#prefix' => '<div class="row" id="slate-event-wrapper-' . $objid . '">',
			'#suffix' => '</div>',
		);
		$se = $event->getSportsEvent();
		$form['events'][$slate_id]['slate_event']['event_title'] = array(
			'#theme' => 'slate-sportsevent-title',
			'#sportsevent' => $se,
			'#visiting_team' => $se->visiting_team,
			'#host_team' => $se->host_team,
			'#vs_at_label' => $se->vs_at_label(),
			'#betting_line' => $se->betting_line,
			'#completed' => $se->completed(),
			'#week' => $se->week,
			'#sport' => $se->sport,
			'#attributes' => array(
				'pid' => '',
			),
		);
		$form['events'][$slate_id]['slate_event']['event_title_mobile'] = array(
			'#type' => 'item',
			'#markup' => sprintf(
				'<div class="h5">%s<br /><small class="visible-xs">%s</small></div>', 
				$event->displayTitleWithLines(), 
				$event->getSportsEvent()->eventDateAndTV('sportsevent_date_medium')
				),
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
		if (!$event->completed()) {
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
		if ($slateWeekIsOpen && in_array('pickem player', $user->roles) && !$event->completed()) {
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
			}
			
			if (($pick_team_name = $event->pick_team_name($user->uid)) != '') {
				$form['events'][$slate_id]['slate_event']['pick_team_name']['#default_value'] = $pick_team_name;
			}
			// END SELECT for the pick
		} else {
			$form['events'][$slate_id]['slate_event']['pick_team_name'] = array(
				'#type' => 'markup',
				'#markup' => '',
			);
			if ($event->completed()) {
				if (!empty($pick) && $pick) {
					$btns = array('btn');
					$btns[] = $pick->correct() ? 'btn-success' : 'btn-default';
					
					$form['events'][$slate_id]['slate_event']['pick_team_name']['#prefix'] = 
						'<button type="button" class="' . implode(' ', $btns) . '" disabled="disabled">';
					$form['events'][$slate_id]['slate_event']['pick_team_name']['#suffix'] = '</button>';
					
					$form['events'][$slate_id]['slate_event']['pick_team_name']['#markup'] =  
						$pick->correct() ?
							'<span class="glyphicon glyphicon-thumbs-up"></span>&nbsp;&nbsp;Correct' :
							'<span class="glyphicon glyphicon-thumbs-down"></span>&nbsp;&nbsp;Wrong';
				}
			}
		} //END PICK TEAM ELEMENT
		
		// BUTTON for "pick stats"
		if ($slateWeekIsOpen) {
			$form['events'][$slate_id]['slate_event']['additional_buttons']['show-pick-stats'] = array(
				'#type' => 'container',
				'#attributes' => array(
					'class' => array('col-xs-1', 'col-md-1'),
				),
			);
			$form['events'][$slate_id]['slate_event']['additional_buttons']['show-pick-stats']['button'] = array(
				'#theme' => 'bs-modal-trigger',
				'#data_target' => '.slate-pick-stats',
				'#btn_display_class' => 'btn-default',
				//'#btn_size_class' => 'btn-sm',
				'#target_entity_id' => $slate_id,
				'#glyphicon' => 'glyphicon-stats',
				'#pickem_id' => $pickem->pid,
			);
		}
	}
	
	$form['events']['slate-pick-stats']['modal'] = array(
		'#theme' => 'modal-pick-stats',
	);
	if (in_array('scoreboard editor', $user->roles)) {
		$form['events']['sportsevent-scoreboard']['modal'] = array(
		);
	}
	
	# Google ads are now obligatory
	$form['adsense'] = array(
		'#type' => 'markup',
		'#markup' => adsense_display(array('format' => '320x100', 'slot' => '2463743760')),
		'#prefix' => '<div class="text-center">',
		'#suffix' => '</div>',
	);
	# ============= ACTION BUTTONS ==============
	
	$form['actions'] = array(
		'#type' => 'actions',
		'#weight' => $weight++,
		);

	if (isset($user->pickems) && ($user->pickems === FALSE || !isset($user->pickems[$pickem->pid]))) {
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
	}
	$form['pickem_id'] = array(
		'#type' => 'hidden',
		'#value' => $pickem->pid,
	);
	$form['submittingModal'] = array(
		'#type' => 'markup',
		'#markup' => theme('submitting-picks', array()),
	);
	// let the players disqus it
	$form['disqus-it'] = array(
		'#theme' => 'pickem-disqus',
		'#pickem_disqus_id' => 'pickem-' . $pickem->name . '-week-' . $week,
		'#pickem_id' => $pickem->pid,
		'#pickem_week' => $week,
		'#weight' => 1000,
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
		$controller = entity_get_controller('pickem_player');
		$pickem_player = $form_state['storage']['pickem_player'];
		
		$controller->addWeekMetaData($pickem_player, $form_state['pickem']->pid, $week, time());
	}
}

/**
 * 
 */
function pickem_pickem_slate_compare($pickem, $week, $player1, $player2) {
	global $user;
	
	$pickem_player = $pickem->getPlayer($user->uid);
	
	$header = array(
		t('Event'),
		array(
			'data'=> t('@player1', array('@player1' => $pickem->playersByUID[$player1]->name)), 
			'class' => array('text-center')
		),
		array(
			'data' => t('@player2', array('@player2' => $pickem->playersByUID[$player2]->name)),
			'class' => array('text-center'),
		),
	);
		
	$rows = array();
	
	$upcomingSlate = $pickem->getPickemSlateForWeek($week);
	foreach ($upcomingSlate as $slate_id => $event) {
		#if ($event->completed()) {
		#	continue;
		#}
		// TODO convert 'pick' into an entity-based object, then..
		$pick_player1 = $event->load_pick_for_user($player1);
		$pick_player2 = $event->load_pick_for_user($player2);
		$picks_match = ($pick_player1->pick_team_name == $pick_player2->pick_team_name);
		$row_class = $picks_match ? '' : 'warning';
		
		$row = array();
		$row[] = array(
			'data' => t('<strong>@displaytitle</strong>', array('@displaytitle' => $event->displayTitle())),
		);
		$row[] = array(
			'data' => '<img class="teamlogo" src="' . base_path() . drupal_get_path('module', 'team') . '/logos/ncaaf/' . $pick_player1->pick_team_name . '.gif" />',
			'class' => array('text-center'),
		);
		$row[] = array(
			'data' => '<img class="teamlogo" src="' . base_path() . drupal_get_path('module', 'team') . '/logos/ncaaf/' . $pick_player2->pick_team_name . '.gif" />',
			'class' => array('text-center'),
		);
		$rows[] = array(
			'data' => $row,
			'class' => array($row_class),
		);
	}
	$page = array();
	$page['navbar'] = array(
		'#theme' => 'pickemform_navbar',
		'#pickem_player' => $pickem_player,
		'#pickem' => $pickem, 
		'#week' => $week,
		'#related_nodes' => array(),
	);
	$page['compare-table'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array('class' => array('table', 'pickem-compare'))
	);
	return $page;
	#return t('Coming soon');
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
	return drupal_json_output(array());
}

/**
 * page callback for pickem standings
 */
function pickem_pickem_standings($pickem) {
 	global $user;

	$user_wrapper = entity_metadata_wrapper('user', $user);
	
	$standings = $pickem->standings();
	
	$lastrank = -1;
	foreach ($standings as $s) {
		$s->display_rank = "&nbsp;";
		
		if ($lastrank != $s->rank) {
			$lastrank = $s->rank;
			$s->display_rank = $lastrank;
		}
		
		$player = $pickem->playerByUid($s->uid);
		$s->weeks_played = $player->weeks_played;
		$s->weeks_won = ($player->weeks_won > 0) ? $player->weeks_won : '&nbsp;';
		$s->this_is_me = ($s->uid == $user->uid);
		$tmp = user_load($player->uid);
		$s->picture = theme('user_picture', array('account' => $tmp));
	}
	
	return array(
		'#theme' => 'complete-standings',
		'#standings' => $standings,
	);
}
