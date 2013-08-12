<?php

function pickem_admin_settings($form, &$form_state) {
	$form['default_season'] = array(
		'#type' => 'textfield',
		'#size' => 10,
		'#default_value' => variable_get('pickem.current_season'),
		'#title' => t('Default Season'),
		'#description' => t('Enter the default season'),
	);
	$form['pickem_title'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('pickem.pickem_title'),
		'#description' => t('Enter the name of the Pickem for this site'),
		'#title' => t('Pickem Title'),
	);
	
	return $form;
}
