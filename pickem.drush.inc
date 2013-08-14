<?php

function pickem_drush_command() {
	$items = array();

	$items['pickem-load-lines'] = array(
		'description' => 'Load Betting Lines from XML RSS feed',
		'aliases' => array('pklines'),
		'bootstrap' => DRUSH_BOOTSTRAP_FULL,
	);

	return $items;
}

function drush_pickem_load_lines() {
	$xmlurl = drupal_get_path('module', 'pickem') . '/data/ncaalines-2013-1.xml';
	$content = file_get_contents($xmlurl);

	$items = array();
	try {
		$xml = new SimpleXMLElement($content);
	} catch(Exception $e) {
		return drush_set_error('PICKEM_LOAD_LINE', dt($e->getMessage()));
	}

	$items = $xml->xpath('/rss/channel/item');
	foreach ($items as $item) {
		$game_title = (string) $item->title;
		$item_desc = (string)$item->description;
		$opponents = array();

		# get the schools involved
		$matches = array();
		$bool = preg_match('/^\[.+\]\s*(?P<team1>\w+)(\([Nn]\))*\s@\s(?P<team2>\w+)(\([nN]\))*$/', $game_title, $matches);
		drush_print($game_title);
		drush_print_r($matches);

		$result = db_query('SELECT name, sportsbook_name FROM {school} where sportsbook_name = :sbname', array(':sbname' => $matches['team1']));
		$team1 = $result->fetchObject();
		$opponents[$team1->sportsbook_name] = $team1;
		$result = db_query('SELECT name, sportsbook_name FROM {school} WHERE sportsbook_name = :sbname', array(':sbname' => $matches['team2']));
		$team2 = $result->fetchObject();
		$opponents[$team2->sportsbook_name] = $team2;

		$bool = preg_match('/^(?P<favorite>\w+)\s(?P<bline>.+)\s.+$/', $item_desc, $bline_matches);
		drush_print_r($bline_matches);
		drush_print_r($team1);
		drush_print_r($team2);

		$query = db_select('game', 'g');
		$query->fields('g');
		$query->condition('host_school_name', array($team1->name, $team2->name), 'IN');
		$query->condition('visiting_school_name', array($team1->name, $team2->name), 'IN');
		$result = $query->execute();
		$o = $result->fetchObject();
		if ($o) {
			$favorite = $bline_matches['favorite'];
			$bline = $bline_matches['bline'];
			$favoriteSchool = $opponents[$favorite]->name;
			$o->betting_line = ($o->host_school_name == $favoriteSchool) ? 'h:' . $bline : 'v:' . $bline;
			drupal_write_record('game', $o, 'gid');
			drush_print_r($o);
		} else {
			drush_print('No fetched data');
		}	
	}	
}	
