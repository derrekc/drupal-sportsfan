<?php
/*
 * Created on Aug 12, 2013
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */


function pickem_school_listing($form, &$form_state, $conf = NULL) {
	drupal_add_library('system', 'drupal.ajax');

	$query = db_select('school', 's')->extend('PagerDefault')->extend('TableSort');
	$query->join('conference', 'conf', 'conf_name = conf.name');
	$query->fields('s');
	$query->addField('conf', 'displaytitle', 'conf_displaytitle')
	if ($conf != NULL) {
		$query->condition('conf_name', $conf);
	}
	$query->orderBy('displaytitle')
	->limit(15);
	$result = $query->execute();

	$headers = array(
		t('Title'),
		t('Full Title'),
		t('System Name')
		t('Conference'),
		t('Division'),
		t('Schedule'),
	);
	
	$rows = array();
	
	while($record = $result->fetchObject()) {
		$rows[] = array(
			'data' => array(
				$record->displaytitle,
				$record->title,
				$record->name,
				$record->conf_displaytitle,
				$record->conf_division,
				l(t('2013 Schedule'), 'schedule/' . $record->name),
			),
		);
	}
	
	$form['slate'] = array(
		'#theme' => 'table',
		'#header' => $headers,
		'#rows' => $rows,
	);
	$form['pager'] = array(
		'#theme' => 'pager',
	);
	return $form;
}
