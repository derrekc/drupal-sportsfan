<?php

/**
 * Administrative form - Pickem Data
 */
function pickem_form($form, &$form_state, $pickem, $op) {
	$form['title'] = array(
		'#type' => 'textfield',
		'#title' => t('Pick\'Em Title'),
		'#description' => t('The name of the Pick\'em Contest.'),
		'#default_value' => $pickem->title,
		'#required' => TRUE,
		'#weight' => -10,
	);
	if (empty($pickem->pid)) {
		$form['name'] = array(
			'#type' => 'machine_name',
			'#default_value' => $pickem->name,
			'#weight' => -9,
		);			
	}
	$form['basic-info'] = array(
		'#type' => 'fieldset',
		'#title' => t('Basic Settings'),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE,
		'#weight' => -8,
	);
	$form['basic-info']['sport'] = array(
		'#type' => 'select',
		'#title' => t('Sport'),
		'#options' => Pickem::sportOptions(),
		'#default_value' => $pickem->sport,
	);
	$form['basic-info']['season'] = array(
		'#type' => 'textfield',
		'#title' => t('Season'),
		'#size' => '4',
		'#required' => TRUE,
		'#default_value' => $pickem->season,
	);
	$form['basic-info']['active'] = array(
		'#type' => 'checkbox',
		'#title' => t('Active'),
		'#description' => t('Mark this Pick\'em contest as active.'),
		'#default_value' => $pickem->active,
	);
  $form['basic-info']['description'] = array(
    '#type' => 'text_format',
    '#title' => t('Description'),
    '#default_value' => $pickem->description,
    '#format' => $pickem->desc_format,
  );
	$form['basic-info']['startdate'] = array(
		'#type' => 'date_popup',
  	'#date_format' => 'Y-m-d',
  	'#date_type' => DATE_UNIX,
  	'#date_timezone' => date_default_timezone(),
		'#title' => t('Start Date'),
		'#default_value' => date('Y-m-d', $pickem->startdate),
	);
	$form['basic-info']['weeks'] = array(
		'#type' => 'textfield',
		'#size' => '4',
		'#default_value' => $pickem->weeks,
		'#title' => t('Number of Weeks'),
	);
	$form['basic-info']['picksperweek'] = array(
		'#type' => 'textfield',
		'#size' => 4,
		'#default_value' => $pickem->picksperweek,
		'#title' => t('Max Picks per Week'),
	);
	$form['basic-info']['joindeadline'] = array(
		'#type' => 'date_popup',
  	'#date_format' => 'm/d/Y - H:i',
  	'#date_type' => DATE_UNIX,
  	'#date_timezone' => date_default_timezone(),
		'#title' => t('Join Deadline'),
		'#default_title' => date('Y-m-d', $pickem->joindeadline),
	);
	
	field_attach_form('pickem', $pickem, $form, $form_state);
	
	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Changes'),
	);
	if (isset($pickem->pid)) {
		$form_state['storage']['pid'] = $pickem->pid;
	}
	
	return $form;
}

/**
 * Submit Callback for Pickem Admin Form
 */
function pickem_form_submit($form, &$form_state) {
	$pickem = entity_ui_form_submit_build_entity($form, $form_state);
	$pickem->description = $form_state['values']['description']['value'];
	$pickem->desc_format = $form_state['values']['description']['format'];
	if (!empty($form_state['values']['joindeadline'])) {
		$dt = date_create_from_format('m/d/Y - H:i', $form_state['values']['joindeadline']);
		$pickem->joindeadline = $dt->getTimestamp();
	}
	if (!empty($form_state['values']['startdate'])) {
		$dt2 = date_create_from_format('Y-m-d', $form_state['values']['startdate']);
		$pickem->startdate = $dt2->getTimestamp();
	}
	if ($pickem->save()) {
		drupal_set_message('The changes were saved.');
	}
	#$form_state['redirect'] = 'admin/content/pickem';
	
}

function pickem_admin_players_form($form, &$form_state, $pickem) {
	// create table for players..
	$header = array(
		t('Name'),
		t('Actions'),
	);
	$rows = array();
	$users = db_select('users','u')
		->fields('u')
		->condition('status', 1)
		->condition('name', 'admin', '<>')
		->execute();
		
	foreach ($users as $user) {
		$row = array();
		$row[] = $user->name;
		if (array_key_exists($user->uid, $pickem->players)) {
			$row[] = l(t('Remove Player'), 'admin/content/pickem/manage/' . $pickem->pid . '/players/remove/' . $user->uid);
		}
		else {
			$row[] = l(t('Add User To Pickem'), 'admin/content/pickem/manage/' . $pickem->pid . '/players/add/' . $user->uid);
		}
		$rows[] = $row;
	}
	
	$form['users'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#empty' => t('No Players/Users'),
	);
	return $form;
}

function pickem_admin_weeks_overview($pickem) {
	$pickem_wrapper = entity_metadata_wrapper('pickem', $pickem);
	$max_bye_weeks_allowed = $pickem_wrapper->max_bye_weeks_allowed->value();
	$pick_default_pickem = $pickem_wrapper->pick_default->value();
		
	// Load Weeks
	$weeks = db_select('week', 'w')
		->fields('w')
		->condition('pickem_id', $pickem->pid)
		->orderBy('week')
		->execute();

	$build = array();

	$header = array(
		t('Week'),
		t('Start'),
		t('End'),
		t('Actions'),
	);
	
	$rows = array();
	foreach ($weeks as $week) {
		$rows[] = array(
			$week->title,
			$week->start,
			$week->end,
			l(t('Open Week'), 'admin/content/pickem/manage/' . $pickem->pid . '/weeks/' . $week->week . '/open') . ' &nbsp; ' .
			l(t('Close Week'), 'admin/content/pickem/manage/' . $pickem->pid . '/weeks/' . $week->week . '/close')
		);
	}
	
	$build['weeks'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#empty' => t('No Weeks'),
	);
	return $build;
}

/**
 * Page callback for opening week for play
 */
function pickem_admin_week_open_form($form, &$form_state, $pickem, $week) {
	$form_state['storage']['pickem'] = $pickem;
	$form_state['storage']['week'] = $week;
	
	//$pickem_slate = $pickem->getPickemSlateForWeek(1);
	//dpm($pickem_slate);
	
	$form['start_message'] = array(
		'#type' => 'markup',
		'#markup' => t('Press the \'Start\' button to begin the Open Week process.  This will affect @count players.', 
			array('@count' => count($pickem->players))),
	);
	
	$form['week'] = array(
		'#type' => 'select',
		'#options' => drupal_map_assoc(array_keys($pickem->slate_weeks)),
		'#description' => 'Select the Week in which to open the Pickem slate',
		'#title' => t('Week'),
		'#required' => TRUE,
	);
	$form['clearpicks'] = array(
		'#type' => 'checkbox',
		'#title' => t('Clear picks prior to provisioning'),
	);
	$form['actions'] = array('#type' => 'actions');
	$form['actions']['start'] = array(
		'#type' => 'submit',
		'#value' => t('Start'),
	);
	return $form;
}

/**
 * Submit callback
 */
function pickem_admin_week_open_form_submit($form, &$form_state) {
	$operations = array();

	$pickem = $form_state['storage']['pickem'];
	$week = $form_state['values']['week'];
	
	if ($form_state['values']['clearpicks'] == 1) {
	// operation for deleting the picks for all the players for this 'week'
		$operations[] = array('pickem_batch_clear_picks_for_week', array($pickem->pid, $week));
	}
	
	foreach ($pickem->players as $player) {
		$operations[] = array('pickem_batch_setup_player_for_week', array($pickem->pid, $player, $week));
	}
	
	$operations[] = array('pickem_batch_mark_pickem_week_open_status', array($pickem->pid, $week, 'Y'));
	
	$batch = array(
		'title' => t('Processing players for the start of the week'),
		'init_message' => t('Initializing for Week ' . $week),
		'operations' => $operations,
		'file' => drupal_get_path('module', 'pickem') . '/pickem.batch.inc',
		'finished' => 'pickem_batch_setup_week_finished',
	);
	batch_set($batch);
}

function pickem_sportsevent_getscore($pickem, $sportsevent) {
	$response = array(
		'host_team_title' => $sportsevent->host_team->displaytitle,
		'visiting_team_title' => $sportsevent->visiting_team->displaytitle,
		'host_team_name' => $sportsevent->host_team_name,
		'visiting_team_name' => $sportsevent->visiting_team_name,
		'host_team_score' => $sportsevent->host_team_score,
		'visiting_team_score' => $sportsevent->visiting_team_score,
		'overtimes' => $sportsevent->overtimes,
		'eid' => $sportsevent->eid,
		'pid' => $pickem->pid,
	);
	return drupal_json_output($response);
	
}

function pickem_admin_week_close_form($form, &$form_state, $pickem, $week) {
	$form_state['storage']['pickem'] = $pickem;
	$form_state['storage']['week'] = $week;
	
	//$pickem_slate = $pickem->getPickemSlateForWeek(1);
	//dpm($pickem_slate);
	
	$form['closeout_message'] = array(
		'#type' => 'markup',
		'#markup' => t('Press the \'Closeout Week\' button to begin the Closeout Week process.  This will affect @count players.', 
			array('@count' => count($pickem->players))),
	);
	
	$form['week'] = array(
		'#type' => 'select',
		'#options' => drupal_map_assoc(array_keys($pickem->slate_weeks)),
		'#description' => 'Select the Week in which to close out the Pickem slate',
		'#title' => t('Week'),
		'#default_value' => $week,
		'#required' => TRUE,
	);
	$form['actions'] = array('#type' => 'actions');
	$form['actions']['start'] = array(
		'#type' => 'submit',
		'#value' => t('Close Out'),
	);
	return $form;
		
}

function pickem_admin_week_close_form_submit($form, $form_state) {
	$operations = array();

	$pickem = $form_state['storage']['pickem'];
	$week = $form_state['values']['week'];
	
	foreach ($pickem->players as $player) {
		$operations[] = array('pickem_batch_close_player_for_week', array($pickem->pid, $player, $week));
	}

	$operations[] = array('pickem_batch_create_standings_for_week', array($pickem->pid, $week));
	$operations[] = array('pickem_batch_mark_pickem_week_open_status', array($pickem->pid, $week, 'N'));
	
	$batch = array(
		'title' => t('Processing players for the start of the week'),
		'init_message' => t('Initializing for Week ' . $week),
		'operations' => $operations,
		'file' => drupal_get_path('module', 'pickem') . '/pickem.batch.inc',
		'finished' => 'pickem_batch_setup_week_finished',
	);
	batch_set($batch);
	
}

function pickem_sportsevent_savescore() {
	
}
