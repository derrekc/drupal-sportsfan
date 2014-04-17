<?php

/**
 * page callback for pickem overview page
 * 
 * @return render array
 */
function pickem_pickem_overview($pid) {
	global $user;
	$page = array();
	
	$pickem = PickemManager::loadPickem($pid);
	$pickem_player = PickemManager::loadPickemPlayer($user->uid);
	
	if (is_null($pickem)) {
		// redirect to a "Invalid Pickem" page
	}
	if (!pickem_player_access($pickem->pid)) {
		// redirect to a page that informs the user he/she is not currently a player
		// and invite them to join (if pickem is accepting new players)
		// or..
		// add a page element that represents the above
		$page['not_current_player'] = array(
			'#theme' => 'not-a-current-player',
			'#pickem' => $pickem,
			'#user' => $user,
		);
		return $page;
	}
	
	//$gameEntries = GoogleSpreadsheetManager::fetchGamesSpreadsheetEntries();
	//dpm($gameEntries);
	//dpm(GoogleSpreadsheetManager::fetchSchoolsSpreadsheetEntries());
	
	$standingsTopTen = StandingsManager::getTopTen($pid);
	$upcomingGames = array();
	
	$page['overview'] = array(
		'#theme' => 'pickem-overview',
		'#standingsTopTen' => $standingsTopTen,
		'#upcomingGames' => $upcomingGames,
	);
	
	return $page;
}

/**
 * page callback for pickem standings
 */
function pickem_pickem_standings($pid) {
	
	return array();
}
