<?php
/**
 * @file
 * SportsEvent Drush script.
 */

/**
 * Implements hook_drush_command().
 */
function sportsevent_drush_command() {
  $items = array();
  $items['import-games'] = array(
    'callback' => 'sportsevent_games_import_google_data',
    'description' => dt('Import Teams from Google Spreadsheet.'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
	$items['update-scores-pickem'] = array(
		'callback' => 'sportsevent_update_scores_for_pickem',
		'description' => dt('Update the scores of sporting event'),
		'aliases' => array('upps'),
		'required-arguments' => 1,
		'arguments' => array(
			'pid' => 'ID of the target Pickem',
			'week' => 'Week the events are played',
		),
	);
	$items['update-score'] = array(
		'callback' => 'sportsevent_update_score',
		'description' => dt('Update the score of a sporting event'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'required-arguments' => 4,
		'arguments' => array(
			'visitor_name' => 'Visiting Team/Club',
			'visitor_score' => 'Visiting Team/Club\'s Score',
			'host_name' => 'Host Team/Club',
			'host_score' => 'Host Team/Club\'s Score',
			'sport' => dt('Sport for which the event exists.'),
		),
		'options' => array(
			'overtimes' => dt('Number of overtime sessions.'),
			'season' => 'Season in which this event is played.',
			'week' => 'Week in which the event is played',
			'pid' => 'ID of the pickem for which to update results',
		),
		'aliases' => array('upsc'),
	);
	$items['set-lines-rss-url'] = array(
		'description' => dt('Set the URL for the RSS feed for betting lines for a given sport (or league).'),
		'callback' => 'sportsevent_set_line_feed_url',
		'required-arguments' => 2,
		'aliases' => array('setlinesurl'),
	);
	$items['load-lines'] = array(
		'description' => dt('Load the latest lines from Vegas for the sporting events'),
		'callback' => 'sportsevent_load_lines',
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'options' => array(
			'sport' => 'The target sport (e.g. ncaaf)',
			'season' => 'Season for the event',
		),
		'aliases' => array('llines'),
	);
  return $items;
}

/**
 * Drush callback
 */
function sportsevent_update_scores_for_pickem($pid, $week = NULL) {
	$pickem = array_shift(entity_load('pickem', array($pid)));
	if (!isset($pickem)) {
		drush_log('Unable to find the Pickem record for ' . $pid . '...', 'error');
		return;
	}
	
	if (is_null($week)) {
		$week = $pickem->currentWeek();
	}
	if (empty($week)) {
		drush_log('Unable to determine the week for this pickem', 'error');
		return;
	}
		
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'pickem_slate')
		->propertyCondition('pid', $pid)
		->propertyCondition('week', $week)
		->propertyOrderBy('slate_date', 'ASC');
		
	$result = $query->execute();
	if (isset($result['pickem_slate'])) {
		$ids = array_keys($result['pickem_slate']);
		$sports_events = entity_load('sportsevent', $ids);
		drush_print(dt('Prepare to enter scores.  Enter \'N\' for next sporting event (game); Enter \'Q\' to exit.'));
		
		reset($sports_events);
		while ($event = current($sports_events)) {
			if ($event->completed()) {
				next($sports_events);
				continue;
			}
			$prompt = sprintf("%s %s %s : ",
				$event->visiting_team_name,
				$event->vs_at_label(),
				$event->host_team_name);
			$input = drush_prompt($prompt, 'N');
			
			// quit the function when a 'Q' command is detected
			if (strtoupper($input) == 'Q') {
				break;
			}
			
			// advance to the next sporting event (game) when the 'N' key is detected
			if (strtoupper($input) == 'N') {
				next($sports_events);
				continue;
			}
			
			// righteous regex to divide the input
			preg_match('/(?P<winning_team_name>[\w-]+)(\s*)?(,?)\s*(?P<winning_team_score>\d+)-(?P<losing_team_score>\d+)(\s*(?P<overtime>(?P<overtime_count>\d?)(OT|ot))?)/', $input, $matches);
			// drush_print_r($matches);
			
			$key_winner = '';
			$key_winning_score = '';
			$key_loser_score = '';
			
			if ($matches['winning_team_name'] == $event->host_team_name) {
				$key_winner = 'host_team_name';
				$key_winning_score = 'host_score';
				$key_loser_score = 'visiting_score';
			} elseif ($matches['winning_team_name'] == $event->visiting_team_name) {
				$key_winner = 'visting_team_name';
				$key_winning_score = 'visiting_score';
				$key_loser_score = 'host_score';
			} else {
				drush_log('Unable to match the winning school with an opponent from this game!', 'error');
				if (!drush_confirm('Try again...?')) {
					next($sports_events);
				}
				continue;
			}
			$event->winning_team_name = $matches['winning_team_name'];
			$event->$key_winning_score = $matches['winning_team_score'];
			$event->$key_loser_score = $matches['losing_team_score'];
			if (isset($matches['overtime'])) {
				$event->overtimes = empty($matches['overtime_count']) ? 1 : $matches['overtime_count'];
			}
			drush_print("");
			drush_print(
				sprintf("%-25s - %2s\n%-25s - %2s%s\n", 
					$event->visiting_team_name,
					$event->visiting_score,
					$event->host_team_name,
					$event->host_score,
					!empty($event->overtimes) ? ' ' . $event->overtimes . 'OT' : '')
					);
			
			if ($pickem) {
				// drush_print('Prepare to update pickem results for ' . $pickem->title . '...');
				_process_sportsevent_result_for_pickem($event, $pickem);
			}
			
			next($sports_events);
		}
	}
}
/**
 * Drush callback
 * 
 */
function sportsevent_update_score($visitor_name, $visitor_score, $host_name, $host_score, $sport = 'ncaaf') {
	drush_print(sprintf("%s - %s, %s - %s", $visitor_name, $visitor_score, $host_name, $host_score));
	
	$line = drush_prompt(dt('Enter something, please?'));
	drush_print($line);
	
	$season = drush_get_option('season', 2014);
	$pickem_module_exists = module_exists('pickem');
	
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'sportsevent')
		->entityCondition('bundle', $sport)
		->propertyCondition('visiting_team_name', $visitor_name)
		->propertyCondition('host_team_name', $host_name);
		
	$result = $query->execute();

	if (isset($result['sportsevent'])) {
		$sports_events_ids = array_keys($result['sportsevent']);
		$sports_events = entity_load('sportsevent', $sports_events_ids);

		foreach ($sports_events as $event) {
			$event->visiting_score = $visitor_score;
			$event->host_score = $host_score;
			$event->completed = 'Y';
			if ($overtimes = drush_get_option('overtimes', FALSE)) {
				$event->overtimes = $overtimes;
			}
			$event->winning_team_name = ($visitor_score > $host_score) ? $event->visting_team_name : $event->host_team_name;
			//entity_save('sportsevent', $event);
			
			// update the score
			// set the winning team
			
			drush_log('Updated the score', 'success');
			drush_log(sprintf('set %s as the winner', '<coming soon>'), 'success');
			if ($pickem_module_exists && ($pid = drush_get_option('pid'))) {
				$pickem = pickem_load($pid);
				drush_log(sprintf('pickem instance, %s, loaded and ready for processing...', $pickem->title), 'success');
				_process_sportsevent_result_for_pickem($event, $pickem);
			}
		}
	}
}

/**
 * Internal helper function
 */
function _process_sportsevent_result_for_pickem($event, $pickem) {
	$controller = entity_get_controller('pickem');
	try {
		$processed = $controller->processSportsEventResults($pickem, $event);
		drush_log(sprintf("%d pick(s) processed for this game in '%s'...", $processed['processed'], $pickem->title), 'ok');
		drush_print_r($processed);
	} catch(Exception $exception) {
		drush_log($exception->getMessage(), 'error');
	}
	// $pickem->processEventResults($event, $winner)
	
	if (module_exists('pickem_standings')) {
		// update the standings for the pickem
	}
	
}
/**
 * Drush callback.
 */
function sportsevent_games_import_google_data() {
	
	// process the desired worksheet(s)
	$worksheets = GoogleSpreadsheetManager::worksheets(GoogleSpreadsheetManager::GAMES_SPREADSHEET_KEY);	
	$options = array();
	foreach ($worksheets as $k => $w) {
		$options[] = $w['label'];
	}
	if (count($worksheets) > 1) {
		$options[] = dt('All');
	}
	$option = drush_choice($options, dt('Select a worksheet'));
	if ($option === FALSE) {
		return;
	}
	drush_print('option = ' . $option);
	if ($option < (count($worksheets))) {
		$worksheets = (array) array_slice($worksheets, $option, 1);
	}

	foreach ($worksheets as $ws) {
		try {
			drush_print(sprintf('Reading "%s" from "%s"', $ws['label'], $ws['href']));
			$games = GoogleSpreadsheetManager::spreadsheetEntries($ws['href']);
			foreach ($games as $g) {
				# $last_updated = new DateTime($g['updated']);
				# $last_updated = new DateTime($g['updated']['$t']);

				#drush_print_r($last_updated);
				#$game_entity = sportsevent_load_multiple($g['gsx$host']['$t'], $g['gsx$visitor']['$t'], array('season' => $g['gsx$season']['$t']));
				#if (!empty($game_entity)) {
				#	continue;
				#}
				
				$entity_type = 'sportsevent';
				#$bundle = isset($g['gsx$bundle']['$t']) ? $g['gsx$bundle']['$t'] : 'college-game';
				$bundle = 'ncaaf';
				$game_entity = '';
				$game_entity = entity_create($entity_type, array('type' => $bundle));	
				$game_entity->game_type = $bundle;
				
				$game_entity->season = $g['gsx$season']['$t'];
				$date = new DateTime($g['gsx$date']['$t']);
				$game_entity->event_date = $date->getTimestamp();
				$game_entity->week = $g['gsx$week']['$t'];
				$game_entity->visiting_team_name = $g['gsx$visitor']['$t'];
				$game_entity->host_team_name = $g['gsx$host']['$t'];
				
				// TODO get conf_name for school
				try {
					$host_team = team_load_by_name($game_entity->host_team_name);
					$game_entity->host_conf_name = $host_team->conf_name;
				} catch (Exception $e) {
					drush_log($e->getMessage(), 'error');
				}
				try {
					$visiting_team = team_load_by_name($game_entity->visiting_team_name);
					$game_entity->visiting_conf_name = $visiting_team->conf_name;
				} catch (Exception $e) {
					drush_log($e->getMessage(), 'error');
				}	
				
				$game_entity->sport = $g['gsx$sport']['$t'];
				$game_entity->tv = $g['gsx$tv']['$t'];
				$game_entity->neutral = $g['gsx$neutral']['$t'] == 'TRUE' ? 'Y' : 'N';
				$game_entity->location = $g['gsx$location']['$t'];
				
				if ($game_entity->save() !== FALSE) {
					drush_log(sprintf('Added %s %s %s to SportsEvent table...', 
						$game_entity->visiting_team_name,
						$game_entity->vs_at_label(),
						$game_entity->host_team_name), 'ok');
					#drush_log(sprintf('Added "%s" to Sports Event database', $game_entity->getDisplayTitle()), 'ok');

					if ($pickem->sport != $game_entity->sport) {
						continue;
					}

					if(!module_exists('pickem')) {
						continue;
					}
					
					$addToSlate = FALSE;
					$patt = sprintf('/%s/', strtolower($ws['label']));
					if (preg_match($patt, $pickem->autoconference) == 1) {
						// add game to slate
						$addToSlate = TRUE;
					}
					else {
						$patt_a = sprintf('/%s/', $game_entity->visiting_conf_name);
						$patt_b = sprintf('/%s/', $game_entity->host_team_name);
						if ((preg_match($patt_a, $pickem->autoconference) == 1) || (preg_match($patt_b, $pickem->autoconference) == 1)) {
							// add game to slate
							$addToSlate = TRUE;
						}
						else {
							$patt_a = sprintf('/%s/', $game_entity->visiting_team_name);
							$patt_b = sprintf('/%s/', $game_entity->host_team_name);
							if ((preg_match($patt_a, $pickem->autoteam) == 1) || (preg_match($patt_b, $pickem->autoteam) == 1)) {
								// add game to slate
								$addToSlate = TRUE;
							}
						}
					}
					if ($addToSlate) {						
						$pickem_slate = entity_create('pickem_slate', array('type' => 'pickem_slate'));
						$pickem_slate->pid = $pickem->pid;
						$pickem_slate->event_entity_id = $game_entity->gid;
						$pickem_slate->event_entity_type = 'game';
						$pickem_slate->season = $game_entity->season;
						$pickem_slate->week = $game_entity->week;
						$pickem_slate->slate_date = $game_entity->game_date;
						if ($pickem_slate->save()) {
							drush_log(sprintf('---- added to Pickem Slate for "%s"', $pickem->title), 'ok');
						}
					}
				}
			}
		} catch(Exception $e) {
			drush_log('Can\'t save games to the database', 'error');
		}
	}
  drush_log('Import complete', 'success');
	
	// TODO - save the timestamp of this run so 
	// I can track when updates are made to the spreadsheet
}


function drush_sportsevent_load_lines() {
	$cmd = 'wget -O ' 
		. drupal_get_path('module', 'pickem') . '/data/ncaalines.xml "' 
		. variable_get('pickem.betting_lines_rss_feed_url') . '"';
		
	$out = `$cmd`;
	
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
		$query->condition('completed', 'N');
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

function sportsevent_set_line_feed_url($rssfeedurl, $sport) {
	$key = sprintf('sportsevent.rss.bettinglines-%s.url', $sport);
	variable_set($key, $rssfeedurl);	
	drush_log('RSS Feed URL for "' . $sport . '" is set to "' . $rssfeedurl . '"', 'ok');
}

function sportsevent_load_lines() {
	
	$season = drush_get_option('season', 2014);
	$sport = drush_get_option('sport', 'ncaaf');
	
	try {
		$url = variable_get('sportsevent.rss.bettinglines-' . $sport . '.url');
		$content = file_get_contents($url);
		$xml = new SimpleXMLElement($content);
	} catch (Exception $parseException) {
		drush_log($parseException->getMessage(), 'error');
		return;
	}

	$items = $xml->xpath('/rss/channel/item');
	foreach ($items as $game) {
		$game_title = (string) $game->title;
		$description = (string) $game->description;
		drush_print('############################################');
		drush_print($game_title);
		drush_print($description);
		
		$title_match = array();
		preg_match('/^\s*(\[(?P<kickoff_time>.+)\]\s*)(?P<away_team>[\w&-\s]+)(\s+(?P<neutral_site>\([Nn]\)))?\s@\s(?P<home_team>[\w&-\s]+)(\s+(\([nN]\)))?\s*$/', $game_title, $title_match);
		
		$desc_match = array();
		preg_match('/^(?P<favorite>[\w&-\s]+)(\s*(\([nN]\)))?\s(?P<bline>(?P<b_operand>[\-\+]*)(?P<spread>[\d.]+))\s(\((?P<money_line>[-\d]+)\))\s.+$/', $description, $desc_match);
		
		//drush_print_r($title_match);
		drush_print_r($desc_match);

		$neutral_site = !empty($title_match['neutral_site']);

		$visitingTeam = FALSE;
		$query = new EntityFieldQuery();
		$teams = $query->entityCondition('entity_type', 'team')
			->propertyCondition('sportsbook_name', '%' . $title_match['away_team'] . '%', 'LIKE')
			->execute();
		
		if (isset($teams['team'])) {
			$tids = array_keys($teams['team']);
			$entities = entity_load('team', $tids);
			
			// the search based on 'sportsbook_name' is a work in progress
			if (count($entities) >= 1) {
				$options = array();
				foreach ($entities as $e) {
					$current = explode(',', $e->sportsbook_name);
					if (in_array($title_match['away_team'], $current)) {
						$visitingTeam = $e;
						$options = array();
						break;
					} else {
						$options[$e->tid] = $e->displaytitle . ' (' . $e->sportsbook_name . ')';
					}
				}
				if (count($options) && $choice = drush_choice($options, 'Select the team that best fits "' . $title_match['away_team'] . '"')) {
					// $visitingTeam = $entities[$choice - 1];
					$visitingTeam = $entities[$choice];
				}
			}
		}

		$hostTeam = FALSE;
		$query = new EntityFieldQuery();
		$teams = $query->entityCondition('entity_type', 'team')
			->propertyCondition('sportsbook_name', '%' . $title_match['home_team'] . '%', 'LIKE')
			->execute();
		if (isset($teams['team'])) {
			$tids = array_keys($teams['team']);
			$entities = entity_load('team', $tids);
			
			// the search based on 'sportsbook_name' is a work in progress
			if (count($entities) >= 1) {
				$options = array();
				foreach ($entities as $e) {
					$current = explode(',', $e->sportsbook_name);
					if (in_array($title_match['home_team'], $current)) {
						$hostTeam = $e;
						$options = array();
						break;
					} else {
						$options[$e->tid] = $e->displaytitle . ' (' . $e->sportsbook_name . ')';
					}
				}
				if (count($options) && $choice = drush_choice($options, 'Select the team that best fits "' . $title_match['home_team'] . '"')) {
					// $visitingTeam = $entities[$choice - 1];
					$hostTeam = $entities[$choice];
				}
			}
		}
		
		//drush_print($visitingTeam->displaytitle);
		//drush_print($hostTeam->displaytitle);
		
		$squery = new EntityFieldQuery();
		$events = $squery->entityCondition('entity_type', 'sportsevent')
			->propertyCondition('visiting_team_name', array($visitingTeam->name, $hostTeam->name), 'IN')
			->propertyCondition('host_team_name', array($hostTeam->name, $hostTeam->name), 'IN')
			->propertyCondition('sport', $sport)
			->propertyCondition('season', $season)
			->execute();
		if (!isset($events['sportsevent'])) {
			drush_log('Unable to locate a game with these two teams -- ' . sprintf("%s, %s", $visitingTeam->displaytitle, $hostTeam->displaytitle), 'warning');
			drush_print("");
			continue;
		}
		$eids = array_keys($events['sportsevent']);
		$sportsevent = reset(entity_load('sportsevent', $eids));
		drush_print($sportsevent->displayTitle());
		if (($sportsevent->visiting_team_name != $visitingTeam->name) && $neutral_site) {
			// my schedule and vegas' schedule are conflicting for neutral games
			$prompt = 'Vegas thinks "' . $visitingTeam->displaytitle . '" is the "Host" team.  Does you want to switch it in the database...?';
			if (drush_confirm($prompt)) {
				// make the switch and save it
				$tmp_host_team_name = $sportsevent->host_team_name;
				$tmp_host_conf_name = $sportsevent->host_conf_name;
				
				$sportsevent->host_team_name = $sportsevent->visiting_team_name;
				$sportsevent->host_conf_name = $sportsevent->visiting_conf_name;
				
				$sportsevent->visiting_team_name = $tmp_host_team_name;
				$sportsevent->visiting_conf_name = $tmp_host_conf_name;
			}
		}

		// Now process the actual line and the favorite
		if ($desc_match['favorite'] == $title_match['away_team']) {
			$sportsevent->favorite = $visitingTeam->name;
		} else {
			$sportsevent->favorite = $hostTeam->name;
		}
		$sportsevent->betting_line = $desc_match['bline'];
		$sportsevent->money_line = $desc_match['money_line'];
		try {
			$sportsevent->save();
			drush_log(sprintf("Changes to '%s' have been saved.", $sportsevent->displayTitle()), 'success');
		} catch(Exception $saveException) {
			drush_log($saveException->getMessage(), 'error');
		}
		
		drush_print("");
	}
	
}
