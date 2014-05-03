<?php


function game_info_page($sport, $season, $host_name, $visitor_name) {
	drupal_set_title(t('Game Page'));
	
	$game = game_load_by_opponents($host_name, $visitor_name, array('season' => $season, 'sport' => $sport));
	dpm($game);
	return "This is coming soon.";
}
