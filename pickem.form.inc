<?php

function pickem_pickem_player_form($form, &$form_state) {
	$pickems = PickemManager::activePickems();
	$options = array();
	foreach ($pickems as $p) {
		$options[$p->pid] = $p->title;
	}

	$form['#tree'] = TRUE;
	$form['player']['player_name'] = array(
		'#type' => 'textfield',
		'#title' => t('Enter Your Handle/Screen Name'),
		'#required' => TRUE,
	);
	$form['player']['joined_pickems'] = array(
		'#type' => 'checkboxes',
		'#options' => $options,
		'#title' => t('Pickems you have joined.'),
		'#description' => t('Check or uncheck the Pickems of your choice.  Be warned that leaving a Pickem during the season will result in lost data.'),
	);	
	
	$form['actions'] = array(
		'#type' => 'actions',
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
	);
	return $form;
}

function pickem_pickem_player_form_submit($form, &$form_state) {
	
}

function theme_pickem_pickem_player_form($variables) {
	$form = $variables['form'];
	$output = drupal_render($form['player']);
	$output .= drupal_render_children($form);
	return $output;
}
