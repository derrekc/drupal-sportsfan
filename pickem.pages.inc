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
	$pickem_player = PickemManager::loadPickemPlayer($user->uid);
	
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
	$page = array();
	
	$pickem = PickemManager::loadPickem($pname);
	$pickem_player = PickemManager::loadPickemPlayer($user->uid);
	
	if (is_null($pickem)) {
		// redirect to a "Invalid Pickem" page
	}
	
	if (!pickem_user_access(sprintf('%s player', $pname))) {
		// redirect to a page that informs the user he/she is not currently a player
		// and invite them to join (if pickem is accepting new players)
		// or..
		// add a page element that represents the above
		$form['not_current_player'] = array(
			'#theme' => 'not-a-current-player',
			'#pickem' => $pickem,
			'#user' => $user,
		);

		return $form;
	}

	if (!isset($form_state['values'])) {
		// new form
		$_SESSION['picks'][$pname] = array();
	}
	$upcomingSlate = SlateManager::slateForWeek($pickem->pid, 1, $pickem->season);
	$form['notes'] = array(
		'#theme' => 'pickem-form-notes',
	);
	
	$form['events']['#tree'] = TRUE;
	$form['debugme'] = array(
		'#type' => 'container',
		'#weight' => -10,
	);
	foreach ($upcomingSlate as $event) {
		$elKey = $event->formElementId();
		$form['events'][$event->objid]['slate_event'] = array(
			'#theme' => 'slate-' . $event->objtype,
			'#objid' => $event->objid,
			'#event' => $event,
			'#prefix' => '<div id="slate-event-wrapper-' . $event->objid . '">',
			'#suffix' => '</div>',
		);
		$form['events'][$event->objid]['slate_event']['event_title'] = array(
			'#type' => 'item',
			'#markup' => '<h5>' . $event->displayTitle() . '</h5>',
			'#description' => 'This is a great game to watch',
			'#attributes' => array(
				'pid' => '',
			),
		);
		$opponent_options = array();
		foreach ($event->opponents() as $opponent) {
			$opponent_options[$opponent->name] = $opponent->displaytitle;
		}
		/*
		$form['events'][$event->objid]['slate_event']['pick_school_name']['choose'] = array(
			'#type' => 'radios',
			'#options' => $opponent_options,
			//'#default_value' => $event->pick_school_name,
		);
		*/
		foreach ($event->opponents() as $opponent) {
			$form['events'][$event->objid]['slate_event']['pick_school_name'][$opponent->name] = array(
				'#type' => 'button',
				'#value' => t(sprintf("Pick %s", $opponent->displaytitle)),
				'#attributes' => array(
					'class' => array('btn', 'btn-sm', 'btn-block', 'btn-default'),
					'pid' => '',
				),
				'#name' => sprintf('slate-op-%s-%s', $event->objid, $opponent->name),
				'#pkid' => '',
				'#objid' => $event->objid,
				'#school_name' => $opponent->name,
				'#ajax' => array(
					'callback' => 'pickem_pick_school_name',
					'method' => 'replace',
					'wrapper' => 'slate-event-wrapper-' . $event->objid,
					#'wrapper' => 'edit-debugme',
					'progress' => array('type' => 'throbber', 'message' => t('Please wait...')),
				),
			);
		}
		$form['actions'] = array(
			'#type' => 'actions'
		);
		$form['actions']['confirmpicks'] = array(
			'#type' => 'submit',
			'#value' => t('Confirm Picks'),
			'#attributes' => array(
				'class' => array('btn','btn-primary'),
			),
		);
		$form['actions']['clearpicks'] = array(
			'#type' => 'submit',
			'#value' => t('Clear Picks'),
			'#attributes' => array(
				'class' => array('btn','btn-default'),
			),
		);
	}
	return $form;
}

/**
 * page callback for pickem standings
 */
function pickem_pickem_standings($pid) {
	
	return array();
}
