<?php

function sscharts_apfb_motion() {

	$page['visualization'] = array(
		'#type' => 'container',
		'#title' => 'Container',
		'#attributes' => array(
			'id' => 'visualization',
			'style' => 'width: 800px; height: 400px;',
		),
		'#attached' => array(
			'js' => array(
				drupal_get_path('module', 'sscharts') . '/apfb_motion.js',
			),
		),	
	);
	$page['markup'] = array(
		'#type' => 'markup',
		'#markup' => '<h3>This is a Test</h3>',
	);

	return $page;
}

function sscharts_apfb_motion_data() {

}
