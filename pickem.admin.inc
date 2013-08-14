<?php

function pickem_admin_settings($form, &$form_state) {
	$form['default_season'] = array(
		'#type' => 'textfield',
		'#size' => 10,
		'#default_value' => variable_get('pickem.current_season'),
		'#title' => t('Current Season'),
		'#description' => t('Enter the default season'),
	);
	
	$season_start = unserialize(variable_get('pickem.season_start'));
	$form['season_start'] = array(
		'#type' => 'date',
		'#title' => t('Start of the <em>current</em> season'),
		'#default_value' => $season_start,
	);
	$form['pickem_title'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('pickem.pickem_title'),
		'#description' => t('Enter the name of the Pickem for this site'),
		'#title' => t('Pickem Title'),
	);
	$form['#submit'][] = 'pickem_admin_settings_submit';
	
	return system_settings_form($form);
}

function pickem_admin_settings_submit($form, $form_state) {
	variable_set('pickem.current_season', $form_state['values']['default_season']);
	variable_set('pickem.pickem_title', check_plain($form_state['values']['pickem_title']));
	
	$return_val = $form_state['values']['season_start'];
	
	variable_set('pickem.season_start', serialize($return_val));
}

function pickem_admin_games($forms, &$form_state) {
	drupal_add_library('system', 'drupal.ajax');

	//$result = pickem_fetch_games();
}
