<?php

function team_form($form, &$form_state, $team, $op = 'edit') {
	if ($op == 'clone') {
		$team->title .= ' (cloned)';
		$team->name .= '_clone';
	}

	$form['title'] = array(
		'#title' => t('Title'),
		'#type' => 'textfield',
		'#default_value' => $team->title,
	);
	
	//field_attach_form('team', $team, $form, $form_state);
	
	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Changes'),
		'#weight' => 50,
	);
	return $form;
}

function team_form_submit(&$form, &$form_state) {
	$team = entity_ui_form_submit_build_entity($form, $form_state);
	$team->save();
	$form_state['redirect'] = 'admin/content/teams';
}
