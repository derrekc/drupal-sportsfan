<?php

function pickem_install_fields() {

}

function _pickem_installed_fields() {
	$t = get_t();
	return array(
		'pick_default' => array(
			'field_name' => 'pick_default',
			'label' => $t('Default Pick/Selection'),
			'description' => $t('This will be the default selection for each game when making picks.'),
			'type' => 'list_text',
		),
		'max_bye_weeks_allowed' => array(
			'field_name' => 'max_bye_weeks_allowed',
			'label' => $t('Maximum number of bye weeks allowed'),
			'description' => $t('Number of bye weeks allowed in this pickem'),
			'type' => 'number_integer',
			'widget' => array('type' => 'text_textfield'),
			'settings' => array(
				'min' => 0,
			)
		),
	);
}

function _pickem_installed_instances() {
	$t = get_t();

	$instances = array();
	$instances[] = array(
		'entity_type' => 'user',
		'bundle' => 'user',
		'field_name' => 'pick_default',
		'label' => $t('Default Pick/Selection'),
		'description' => $t('This will be the default selection for each game when making picks.'),
		'type' => 'list_text',
		'widget' => array(
			'type' => 'options_select',
			'settings' => array(
				'allowed_values' => array(
					'hometeam' => $t('Home Team'),
					'visitingteam' => $t('Visiting Team'),
					'topranked' => $t('Team with Highest Ranking'),
				),
			),
		),
	);
	return $instances;
}
