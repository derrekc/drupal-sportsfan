<?php

/**
 * Administrative form - Game Data
 */
function game_form($form, &$form_state, $game, $op) {
	
	$form['title'] = array(
		'#type' => 'textfield',
		'#title' => t('Game Title'),
		'#description' => t('The name of the Game Contest.'),
		'#default_value' => $game->title,
		'#required' => TRUE,
		'#weight' => -10,
	);
	
	field_attach_form('game', $game, $form, $form_state);
	
	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save Changes'),
	);
	
	return $form;
}

/**
 * Submit Callback for Pickem Admin Form
 */
function game_form_submit($form, &$form_state) {
	$game = entity_ui_form_submit_build_entity($form, $form_state);
	$game->save();
	$form_state['redirect'] = 'admin/content/game';
	
}
