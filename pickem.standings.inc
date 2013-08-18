<?php

/**
 * Pickem Standings Overview Page
 */
function pickem_standings_overview($week = -1) {
	$headers = array(
		t('Rank'),
		t('Player'),
		t('Correct'),
		t('Incorrect'),
		t('Pct'),
		t('Games Picked'),
	);
	$build['standings'] = array(
		'#type' => 'table',
		'#header' => $headers,
		'#rows' => array(),
	);

	return $build;
}	
