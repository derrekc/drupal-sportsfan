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
	$pickem->save();
	$form_state['redirect'] = 'admin/content/pickem';
	
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
			$row[] = l(t('Remove Player'), 'admin/content/pickem/manage/1/players/remove/' . $user->uid);
		}
		else {
			$row[] = l(t('Add User To Pickem'), 'admin/content/pickem/manage/1/players/add/' . $user->uid);
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
