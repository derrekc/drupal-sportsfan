<?php

/**
 * Page (Form) callback for requesting a new pickem
 */
function pickem_request_new_pickem($form, &$form_state) {
	$form['title'] = array(
		'#type' => 'textfield',
		'#title' => t('Pick\'em Title'),
		'#required' => TRUE,
	);
	$form['pickem_name'] = array(
		'#type' => 'machine_name',
		'#title' => t('Pick\'em Name'),
		'#machine_name' => array(
			'exists' => 'pickem_edit_pickem_name_exists',
			'source' => array('title'),
			'replace_pattern' => '[^a-z0-9-]+',
			'replace' => '-',
		),
	);
  $form['description'] = array(
    '#type' => 'text_format',
    '#title' => t('Description'),
  );
	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
	);
	return $form;
}

function pickem_request_new_pickem_submit($form, &$form_state) {
	
}

function pickem_edit_pickem_name_exists($name) {
	$pname_exists = db_query_range('SELECT 1 FROM {pickem} WHERE name = :pname', 0, 1, array(':pname' => $name))->fetchField();
	return $pname_exists;
}

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
