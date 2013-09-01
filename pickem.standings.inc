<?php

/**
 * Pickem Standings Overview Page
 */
function pickem_standings_overview($week, $season = NULL) {
	global $user;
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}
	
	$r = db_query('SELECT COUNT(*) as slatecount FROM {slate} WHERE season = :season AND slate_id = :week',
		array(':season' => $season, ':week' => $week));
	$obj = $r->fetchObject();
	$slateOfGamesCount = $obj->slatecount;
	

	$standings = pickem_load_standings($week, $season);

	$headers = array(
		t('Rank'),
		t('Player'),
		t('Correct'),
		t('Incorrect'),
		t('Pct'),
		t('Games Picked'),
	);
	
	$rows = array();
	foreach ($standings as $player) {
		$pct = 0.0;
		if ($player->correct + $player->incorrect) {
			$pct = (float) $player->correct / ($player->correct + $player->incorrect);
		}
		$classes = array();
		if ($player->player_name == $user->name) {
			$classes[] = 'info';
		}
		$rows[] = array(
			'data' => array(
				'-',
				$player->player_name,
				$player->correct,
				$player->incorrect,
				number_format($pct, 3),
				#$data['num_picks'],
				isset($player->pick_count) ? $player->pick_count : '',
			),
			'class' => $classes,
		);
	}
	$build['standings'] = array(
		'#theme' => 'table',
		'#header' => $headers,
		'#rows' => $rows,
		'#empty' => t('Standings are not available.  Please check back later.'),
		'#attributes' => array('class' => array('table table-condensed')),
	);
	return $build;
}	
