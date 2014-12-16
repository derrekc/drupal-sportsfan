<?php

function sportsevent_form($form, &$form_state, $sportsevent) {
	$form['title'] = array(
		'#type' => 'textfield',
		'#title' => t('Pick\'Em Title'),
		'#description' => t('The name of the Pick\'em Contest.'),
		'#default_value' => $sportsevent->title,
		'#required' => TRUE,
		'#weight' => -10,
	);
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
		'#default_value' => $sportsevent->sport,
	);
	$form['basic-info']['season'] = array(
		'#type' => 'textfield',
		'#title' => t('Season'),
		'#size' => '4',
		'#required' => TRUE,
		'#default_value' => $sportsevent->season,
	);
	$form['basic-info']['completed'] = array(
		'#type' => 'checkbox',
		'#title' => t('Active'),
		'#description' => t('Mark this Pick\'em contest as active.'),
		'#default_value' => $sportsevent->completed,
	);
	
	field_attach_form('pickem', $sportsevent, $form, $form_state);
	
	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Changes'),
	);
	if (isset($sportsevent->eid)) {
		$form_state['storage']['eid'] = $sportsevent->eid;
	}
	
	return $form;
	
}

function sportsevent_edit_score_form($form, &$form_state, $sportsevent) {
	
}

function sportsevent_edit_score_form_submit($form, &$form_state) {
	
}
