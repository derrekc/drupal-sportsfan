<?php

/**
 * Display a listing of available venues, 
 * using the table theme (w/ pager included)
 */
function venue_admin_venue_default() {
	$venues = venue_load_venues();
	
	$limit = 15;
	// $result = pager_query($query, $limit);
		
	$header = array(
		array('data' => t('Venue'), 'field' => 'v.title', 'sort' => 'asc'),
		array('data' => t('City'), 'field' => 'v.location_city'),
		array('data' => t('State'), 'field' => 'v.location_state'),
		array('data' => t('Surface'), 'field' => 'v.surface'),
		array('data' => t('Capacity'), 'field' => 'v.capacity'),
		array('data' => t('Subtitle'), 'field' => 'v.subtitle'),
		t('Operations')
	);

	# http://www.drup-all.com/blog/table-sort-pagination-drupal-7
	$query = db_select('venues', 'v')
		->fields('v')
		->extend('TableSort')
		->orderByHeader($header)
		->extend('PagerDefault')
		->limit($limit);
	$result = $query->execute();

	$rows = array();
	#foreach ($venues as $venue) {
	while($venue = $result->fetchObject()) {
		$ops = array(l(t('edit'), 'admin/config/venues/edit/' . $venue->vid));
		if (empty($venue->locked)) {
			$ops[] = l(t('delete'), 'admin/config/venues/delete/' . $venue->vid);
		}
		$rows[] = array(
			check_plain($venue->title),
			check_plain($venue->location_city),
			check_plain($venue->location_state),
			check_plain($venue->surface),
			number_format((float)$venue->capacity, 0),
			check_plain($venue->subtitle),
			implode(' | ', $ops),
		);
	}
	
	$build = array();
	$build['venues'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#empty' => t('No venues have been defined as of now.'),
	);
	$build['venue_pager'] = array(
		'#theme' => 'pager',
		'#tags' => array('first','<prev','','next>','last'),
		'#limit' => $limit,
	);
	
	$build['header'] = array(
		'#markup' => '<h2>' . t('Add a venue') . '</h2>',
	);
	//$build['form'] = drupal_get_form('cs_conference_class_form');
	
	return $build;
	
}
