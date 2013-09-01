<?php

function pickem_drush_command() {
	$items = array();

	$items['pickem-load-lines'] = array(
		'description' => 'Load Betting Lines from XML RSS feed',
		'aliases' => array('pklines'),
		'bootstrap' => DRUSH_BOOTSTRAP_FULL,
	);
	$items['load-conf-games'] = array(
		'description' => 'Load games for a conference',
		'aliases' => array('lcgames'),
		'bootstrap' => DRUSH_BOOTSTRAP_FULL,
		'arguments' => array(
			'conf' => 'The (machine) name of the conference',
		),
	);
	$items['refresh-schools'] = array(
		'description' => 'Load unprocessed schools from "schools.tsv"',
		'aliases' => array('schools'),
		'bootstrap' => DRUSH_BOOTSTRAP_FULL,
	);
	return $items;
}

function drush_pickem_load_conf_games($conf) {
	require_once(drupal_get_path('module', 'pickem') . '/pickem.install');
	$filename = $conf.'games.tsv';
	
	$file = drupal_get_path('module', 'pickem') . '/data/' . $filename;
	if (($handle = fopen($file, 'r')) !== FALSE) {
		while (($data = fgetcsv($handle, 90, "\t"))) {
			if ($data[0] == 'season') { continue; }
			$o = new stdClass();
			$o->season = $data[0];
			$o->week = $data[1];
			$o->visiting_school_name = $data[2];
			$o->host_school_name = $data[3];
			$o->sport = 'ncaaf';
			
			$query = db_select('game', 'g')
				->fields('g')
				->condition('week', $o->week)
				->condition('season', $o->season)
				->condition('host_school_name', array($data[2], $data[3]))
				->condition('visiting_school_name', array($data[2], $data[3]));

			# account for possible conference title game
			if (isset($data[7]) && $data[7] != '') {
				$query->condition('title', $data[7]);
			}
			$res = $query->execute();
			$game = $res->fetchObject();

			if (!$game) {
				$res = db_query('SELECT conf_name FROM {school} WHERE name = :name', array(':name' => $data[2]));
				$visiting_school = $res->fetchObject();
				$res = db_query('SELECT conf_name FROM {school} WHERE name = :name', array(':name' => $data[3]));
				$host_school = $res->fetchObject();
				
				$date = new DateTime($data[4]);
				$o->game_date = $date->getTimestamp();
				
				$o->location = $data[5];
				$o->host_conf_name = isset($host_school->conf_name) ? $host_school->conf_name : '';
				$o->visiting_conf_name = isset($visiting_school->conf_name) ? $visiting_school->conf_name : '';
				$o->neutral = $data[6] == 'neutral' ? 'Y' : 'N';
				$o->location = $data[5];
				$o->title = '';
				if (isset($data[7]) && $data[7] != '') {
					# game title
					$o->title = $data[7];
				}
				if ($o->title == 'PAC-12 Championship Game') {
					drush_print('got pac-12 championship game...');
					drush_print_r($o);
				}
				$gid = db_insert('game')
					->fields(array(
							'season' => $o->season,
							'week' => $o->week,
							'visiting_school_name' => $o->visiting_school_name,
							'host_school_name' => $o->host_school_name,
							'sport' => 'ncaaf',
							'game_date' => $o->game_date,
							'location' => $o->location,
							'host_conf_name' => $o->host_conf_name,
							'visiting_conf_name' => $o->visiting_conf_name,
							'neutral' => $o->neutral,
							'title' => $o->title,
						)
					)->execute();
				//drush_print_r($o);
				#drupal_write_record('game', $o);
				
				// if the game includes schools from the 'default conference'
				// automatically add them to the slate of games
				$default_conf = variable_get('pickem.default_conference','');
				$tmp_array = array($visiting_school->conf_name, $host_school->conf_name);
				$protected_schools = variable_get('pickem.protected_schools', array());
				
				$addToSlate = in_array($default_conf, $tmp_array) || 
					(in_array($data[2], $protected_schools) ||
					 in_array($data[3], $protected_schools)) ||
					$data[8] == 'slate';
				
				//if (($tmp_school[$data[2]]->conf_name == $default_conf ||
				//		$tmp_school[$data[3]]->conf_name == $default_conf) || (isset($data[8]) && $data[8] == 'slate')) {
				if ($addToSlate) {
					db_insert('slate')
						->fields(array(
							'slate_id' => $o->week,
							'gid' => $gid,
							'season' => $o->season,
							'pid' => 1
						)
					)->execute();
				}
			}	else {
				#drush_print('game exists -- skipping');
				drush_print($game->host_school_name . ' v. ' . $game->visiting_school_name . '; game exists -- skipping');
			}
		}
		fclose($handle);
	}
}

function drush_pickem_refresh_schools() {
	global $tmp_school;
	$file = drupal_get_path('module', 'pickem') . '/data/schools.tsv';
	if (($handle = fopen($file, 'r')) !== FALSE) {
		while (($data = fgetcsv($handle, 90, "\t"))) {
			$o = new stdClass();
			$o->title = $data[0];
			$o->displaytitle = $data[1];
			$o->name = $data[2];
			$o->nickname = $data[3];
			$o->conf_name = $data[4];
			$o->conf_division = $data[5];
			if (isset($data[6])) {
				$o->sportsbook_name = $data[6];
			} else {
				$o->sportsbook_name = preg_replace('/\s+/', '', $data[0]);
			}
			$msg = $o->name . ' | ' . $o->displaytitle;
			$r = db_query('SELECT s.name FROM {school} s WHERE name = :name', array(':name' => $data[2]));
			if (!$r->fetchObject()) {
				$msg .= '... adding to database';
				$o->sid = db_insert('school')
					->fields(array(
						'title' => $o->title,
						'displaytitle' => $o->displaytitle,
						'name' => $o->name,
						'nickname' => $o->nickname,
						'conf_name' => $o->conf_name,
						'conf_division' => $o->conf_division,
						'sportsbook_name' => $o->sportsbook_name,
					)
				)->execute();
				$msg .= '; sid = (' . $o->sid . ')';
			} else {
				$msg .= '... already in database';
			}
			//drush_print($msg);
			$tmp_school[$o->name] = $o;
		}
		fclose($handle);
	}
}

function drush_pickem_load_lines() {	
	$xmlurl = drupal_get_path('module', 'pickem') . '/data/ncaalines.xml';
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
		#drush_print($game_title);
		#drush_print_r($matches);

		$result = db_query('SELECT name, sportsbook_name FROM {school} where sportsbook_name = :sbname', array(':sbname' => $matches['team1']));
		$team1 = $result->fetchObject();
		$opponents[$team1->sportsbook_name] = $team1;
		$result = db_query('SELECT name, sportsbook_name FROM {school} WHERE sportsbook_name = :sbname', array(':sbname' => $matches['team2']));
		$team2 = $result->fetchObject();
		$opponents[$team2->sportsbook_name] = $team2;

		$bool = preg_match('/^(?P<favorite>\w+)(\([nN]\))*\s(?P<bline>(?P<b_operand>[\-\+]*)(?P<spread>[\d.]+))\s.+$/', $item_desc, $bline_matches);
		drush_print_r($team1);
		drush_print_r($team2);

		$query = db_select('game', 'g');
		$query->fields('g');
		$query->condition('host_school_name', array($team1->name, $team2->name), 'IN');
		$query->condition('visiting_school_name', array($team1->name, $team2->name), 'IN');
		$result = $query->execute();
		$o = $result->fetchObject();
		if ($o) {
			#$favoriteSchool = $opponents[$favorite]->name;

			$favorite = $bline_matches['favorite'];
			$bline = $bline_matches['bline'];
			if ($bline_matches['b_operand'] == '-') {
				$favoriteSchool = $opponents[$favorite]->name;		
			} else if ($bline_matches['b_operand'] == '+') {
				if ($bline_matches['favorite'] == $matches['team1']) {
					$favoriteSchool = $opponents[$matches['team2']]->name;
				} else {
					$favoriteSchool = $opponents[$matches['team1']]->name;
				}
			}
			$o->betting_line = $favoriteSchool . ':-' . trim($bline_matches['spread']);		
			
			drush_print("%%%%%%%% favorite school is [" . $favoriteSchool . "] %%%%%%%%%%%%");
			#$o->betting_line = $favoriteSchool . ':' . trim($bline);
			drupal_write_record('game', $o, 'gid');
		} else {
			#drush_print('No fetched data');
			#drush_print_r($bline_matches);
		}	
	}	
	variable_set('pickem.bettinglines_updated', date('M j, Y -- g:i A'));
}	
