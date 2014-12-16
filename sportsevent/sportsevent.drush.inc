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
	$items['seimp'] = array(
		'callback' => 'sportevent_import_events',
		'description' => dt('Import SportsEvents from CSV file'),
		'required-arguments' => 2,
		'arguments' => array(
			'file' => 'Filename to import',
			'sport' => 'Sport',
		),
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
	$items['update-tvtime-pickem'] = array(
		'callback' => 'sportsevent_update_tvtime_for_pickem',
		'description' => dt('Update the scores of sporting event'),
		'aliases' => array('upptv'),
		'required-arguments' => 1,
		'arguments' => array(
			'pid' => 'ID of the target Pickem',
			'week' => 'Week the events are played',
		),
	);
	$items['update-time-tv'] = array(
		'callback' => 'sportsevent_update_tv_time',
		'description' => dt('Update the TV and Time of a sporting event'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'aliases' => array('uptv'),
		'required-arguments' => 2,
		'arguments' => array(
			'visiting_team_name' => 'Visiting Team',
			'host_team_name' => 'Host (or Home) Team',
		),
		'options' => array(),
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
	$items['fetch-scores'] = array(
		'description' => dt('Fetch scores from reliable datasource'),
		'callback' => 'sportsevent_fetch_ncaa_scores',
		'options' => array(
			'sport' => 'The target sport (e.g. ncaaf)',
			'week' => 'Target week (or end week)',
			'week_start' => 'Week to start with',
			'date' => 'Target date (particularly for ncaam, ncaaw, etc)',
			'date_start' => 'Date to start with',
			'pickem_id' => 'Pickem ID, if supplied, update Pickem results',
			'noupdate' => 'Do not actually update the database',
			'ignore_event_status' => 'Select events regardless of completed status',
			'parse_venues' => 'Parse Venues and build a proposed list',
			'auto_add' => 'Automatically add events to the database',
			'fix' => '',
		),
		'aliases' => array('scores'),
	);
	$items['add-sportsevent'] = array(
		'description' => dt('Add Sporting Event'),
		'callback' => 'sportsevent_add_event',
		'required-arguments' => 5,
		'arguments' => array(
			'sport' => 'Affected Sport',
			'host' => 'Host Team Name (machine name)',
			'visitor' => 'Visiting Team Name (machine_name)',
			'event_date' => 'Date (and time) of the event',
			'week' => 'Week of the Event',
		),
		'options' => array(
			'venue_id' => 'Venue of the Event',
			'neutral' => 'Indicate this event is played at a neutral site',
			'pickem_id' => 'Pickem ID, if supplied, update Pickem results',
			'pickem_week' => 'Pickem Week (e.g. 1)',
			'noupdate' => 'Do not actually update the database',
			'season' => 'Affected season',
			'title' => 'Prompt for Event title',
			'tv' => 'TV Network',
			'location' => 'Location of the Event'
		),
		'aliases' => array('addsevt'),
	);
	$items['fix-scores'] = array(
		'description' => dt('Fix the scores I screwed up'),
		'callback' => 'sportsevent_fix_ncaaf_scores',
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'aliases' => array('fixit'),
		'options' => array(
			'noupdate' => 'Do not actually update the database',
		),
	);
  return $items;
}

/**
 * Drush callback
 */
function sportsevent_update_scores_for_pickem($pid, $week = NULL) {
	$pickems = entity_load('pickem', array($pid));
	$pickem = reset($pickems);

	#$pickem = array_shift(entity_load('pickem', array($pid)));
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
		$slids = array_keys($result['pickem_slate']);
		foreach ($result['pickem_slate'] as $id => $o) {
			//drush_print_r($o);
			$ids[] = $o->event_entity_id;
		}
		#$ids = array_keys($result['pickem_slate']);
		$slateEvents = entity_load('pickem_slate', $slids);
		drush_print(dt('Prepare to enter scores.  Enter \'N\' for next sporting event (game); Enter \'Q\' to exit.'));
		
		reset($slateEvents);
		#reset($sports_events);
		while ($slateEvent = current($slateEvents)) {
			$event = $slateEvent->sportsEvent;
			if ($event->completed()) {
				next($slateEvents);
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
				next($slateEvents);
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
					next($slateEvents);
				}
				continue;
			}
			$event->winning_team_name = $matches['winning_team_name'];
			$event->$key_winning_score = $matches['winning_team_score'];
			$event->$key_loser_score = $matches['losing_team_score'];
			if (isset($matches['overtime'])) {
				$event->overtimes = empty($matches['overtime_count']) ? 1 : $matches['overtime_count'];
			}
			$event->completed = 'Y';

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
			$event->save();
			next($slateEvents);
		}
	}
}

/**
 * Drush callback
 */
function sportevent_import_events($filename, $sport) {
	$filename = drupal_get_path('module', 'sportsfan') . '/data/' . $filename;
	
	$columns = array(
		'eid',
		'season',
		'host_team_name',
		'visiting_team_name',
		'event_date',
		'sport',
		'location',
		'neutral',
		'title',
	);
	
	$res = db_select('team', 't')
		->fields('t', array('name','conf_name'))
		->condition('sport', $sport)
		->execute();

	$teams = array();
	foreach ($res as $r) {
		$teams[$r->name] = $r->conf_name;
	}
	drush_log('Loaded all teams in sport, ' . $sport, 'ok');
	
	$teams_not_present = array();
	
	$fh = fopen($filename, 'r');
	while (($row = fgetcsv($fh, 4096, ",")) !== FALSE) {
		$insert = array();
		foreach($columns as $col) {
			$insert[$col] = array_shift($row);
			
			if ($col == 'eid') {
				$insert['eid'] = null;
				continue;
			}
			if ($col == 'event_date') {
				$dt = new DateTime($insert[$col]);
				$insert[$col] = $dt->getTimestamp();
				continue;
			}
			if ($col == 'host_team_name') {
				if (isset($teams[$insert[$col]])) {
					$insert['host_conf_name'] = $teams[$insert[$col]];
				}
				else {
					$teams_not_present[ $insert[$col] ] = 0;
				}
				continue;
			}
			if ($col == 'visiting_team_name') {
				if (isset($teams[$insert[$col]])) {
					$insert['visiting_conf_name'] = $teams[$insert[$col]];
				}
				else {
					$teams_not_present[ $insert[$col] ] = 0;
				}
				continue;
			}
		}
		$eid = db_insert('sportsevent')->fields($insert)->execute();
		drush_log(sprintf("inserted new eid [%s]", $eid), 'ok');
	}	
	fclose( $fh );
	drush_print_r($teams_not_present);
}

/**
 * Drush callback
 */
function sportsevent_update_tvtime_for_pickem($pid, $week = NULL) {
	$pickems = entity_load('pickem', array($pid));
	$pickem = reset($pickems);

	#$pickem = array_shift(entity_load('pickem', array($pid)));
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
		$slids = array_keys($result['pickem_slate']);
		foreach ($result['pickem_slate'] as $id => $o) {
			//drush_print_r($o);
			$ids[] = $o->event_entity_id;
		}
		#$ids = array_keys($result['pickem_slate']);
		$slateEvents = entity_load('pickem_slate', $slids);
		drush_print(dt('Prepare to enter TV and/or Event Date.  Enter \'N\' for next sporting event (game); Enter \'Q\' to exit.'));
		
		reset($slateEvents);
		#reset($sports_events);
		while ($slateEvent = current($slateEvents)) {
			$event = $slateEvent->sportsEvent;
			if ($event->completed()) {
				next($slateEvents);
				continue;
			}
			$current_kickoff = date('n/j/Y H:i:s', $event->event_date);
			$prompt = sprintf("%s %s %s (%s)",
				$event->visiting_team_name,
				$event->vs_at_label(),
				$event->host_team_name,
				$current_kickoff);
			$input = drush_prompt($prompt, 'N');
			
			// quit the function when a 'Q' command is detected
			if (strtoupper($input) == 'Q') {
				break;
			}
			
			// advance to the next sporting event (game) when the 'N' key is detected
			if (strtoupper($input) == 'N') {
				next($slateEvents);
				continue;
			}
			
			$matches = array();
			preg_match('/^((?P<tvnetwork>[a-z][a-z0-9]*)\s+)?(?P<event_date>\d{1,2}\/\d{1,2}\/\d{2,4}.*)$/', $input, $matches);
			drush_print_r($matches);
			if (isset($matches['tvnetwork']) && !empty($matches['tvnetwork'])) {
				$event->tv = $matches['tvnetwork'];
			}
			if (!empty($matches['event_date'])) {
				$date = new DateTime($matches['event_date']);
				$event->event_date =  $date->getTimestamp();
				$slateEvent->slate_date = $date->getTimestamp();
			}
			if ($slateEvent->save() && $event->save()) {
				drush_log('TV/Time changes saved', 'ok');
			}
			next($slateEvents);
		}
	}
}

function sportsevent_update_tv_time($visitor_name, $host_name) {
	$sport = drush_get_option('sport', 'ncaaf');
	
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'sportsevent')
		->entityCondition('bundle', $sport)
		->propertyCondition('visiting_team_name', $visitor_name)
		->propertyCondition('host_team_name', $host_name);
		
	$result = $query->execute();

	if (isset($result['sportsevent'])) {
		$sports_events_ids = array_keys($result['sportsevent']);
		$sports_events = entity_load('sportsevent', $sports_events_ids);
		if (count($sports_events) == 0) {
			drush_log(dt('No event exists for these two teams'), 'error');
			return;
		}
		drush_print_r(array_keys($sports_events));
		$selection = FALSE;
		if (count($sports_events) > 1) {
			// use drush_choice to get the game in question
			$choices = array();
			foreach ($sports_events as $eid => $event) {
				$choices[$eid] = $event->displayTitle() . ' (' . $event->season . ')';
			}
			$selection = drush_choice($choices, 'Select an event');
			
		} else {
			$selection = array_shift(array_keys($sports_events));
		}
		if ($selection) {
			drush_print($sports_events[$selection]->displayTitle());
			$tv_choices = SportsEvent::$tvNetwork;
			if (!empty($sports_events[$selection]->tv)) {
				$tv_choices[$sports_events[$selection]->tv] .= ' (*)';
			}
			$tv_selection = drush_choice($tv_choices, dt('Select TV Network'));
			drush_print("tv selection = [" . $tv_selection . "]");
			if (!empty($tv_selection)) {
				$sports_events[$selection]->tv = $tv_selection;
			}
			$current_kickoff = date('n/j/Y H:i:s', $sports_events[$selection]->event_date);
			$new_kickoff = drush_prompt(dt('Kickoff Time (m/d/yyyy hh:ii:ss -- 24-hour)'), $current_kickoff);
			if (!empty($new_kickoff)) {
				$date = new DateTime($new_kickoff);
				$sports_events[$selection]->event_date = $date->getTimestamp();
			}
			drush_print(date('n/j/Y g:i A', $sports_events[$selection]->event_date));
			if ($sports_events[$selection]->save()) {
				drush_log('TV and/or event date changes saved', 'ok');
			}
			
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
				if ($g['gsx$date']['$t'] != "") {
				$date = new DateTime($g['gsx$date']['$t']);
					$game_entity->event_date = $date->getTimestamp();
				} else {
					$game_entity->event_date = NULL;
				}
				if ($g['gsx$week']['$t'] == '') {
					$game_entity->week = 0;
				} else {
					$game_entity->week = $g['gsx$week']['$t'];
				}
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


function sportsevent_set_line_feed_url($rssfeedurl, $sport) {
	$key = sprintf('sportsevent.rss.bettinglines-%s.url', $sport);
	variable_set($key, $rssfeedurl);	
	drush_log('RSS Feed URL for "' . $sport . '" is set to "' . $rssfeedurl . '"', 'ok');
}

function _parse_event_info($summary_url) {
	$ctx = stream_context_create(array('http' => array('timeout' => 15.0)));
	$file = file_get_contents($summary_url, 0, $ctx);
	
	$doc = new DOMDocument();
	// supress DOM parsing errors
	@$doc->loadHTML( $file );
	
	# need to parse the date out of the URL
	$datematch = array();
	$date_info = array();
	if (preg_match('/^.+\/(fbs|d1)\/(?P<year>\d{4})\/(?P<month>\d{2})\/(?P<day>\d{2}\/.+$)/', $summary_url, $datematch) == 1) {
		$date_info['month'] = $datematch['month'];
		$date_info['year'] = $datematch['year'];
		$date_info['day'] = $datematch['day'];
	}
	
	$xpath = new DOMXPath( $doc );
	$locationElements = $xpath->query("//p[@class='location']");
	$index = $locationElements->length == 1 ? 0 : 1;
	
	$game_state_elements = $xpath->query("//div[contains(concat(' ', @class, ' '), ' game-state')]//span[@class='state']");
	$game_state = $game_state_elements->item(0)->nodeValue;
	
	$schoolElements = $xpath->query("//table[@id='linescore']//td[@class='school']/a");
	$host_school = $schoolElements->item(1)->nodeValue;
	$visiting_school = $schoolElements->item(0)->nodeValue;
	
	$m = array();
	$r = preg_match("/^(?P<venue_name>[\w\s\-.&']+),\s(?P<location_city>[\w\s-.]+),\s(?P<location_state>\w{2})$/", $locationElements->item($index)->nodeValue, $m);
	if ($r == 1) {		
		return array(
			'host_team' => $host_school,
			'visiting_team' => $visiting_school,
			'location_state' => $m['location_state'],
			'location_city' => $m['location_city'],
			'location' => sprintf("%s, %s", $m['location_city'], $m['location_state']),
			'venue_name' => $m['venue_name'],
		) + $date_info;
	}
	return FALSE;
}

function _build_venues_list($summary_url, &$venues) {
	if ($info = _parse_event_info($summary_url)) {
		if (!isset($venues[$host_school])) {
			$venues[$host_school] = array();
		}

		$location_key = $info['location'];
		
		if (!isset($venues[$host_school][$location_key])) {
			$venues[$host_school][$location_key] = array();
		}
		$venues[$host_school][$location_key][$info['venue_name']] = $location_key;
		
	}
}

/**
 * Temporarily fix the scores for ncaam
 * 
 */
function sportsevent_fix_ncaam_scores() {
		
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'sportsevent')
		->propertyCondition('completed', 'Y')
		->propertyCondition('winning_team_name', NULL)
		->propertyCondition('sport', 'ncaam')
		->propertyCondition('season', 2015);
		
	$result = $query->execute();
	if (isset($result['sportsevent'])) {
		$eids = array_keys($result['sportsevent']);
		drush_print(sprintf("Loading %s sportsevent(s) to fix...", count($eids)));
		$sportsevents = entity_load('sportsevent', $eids);
		foreach ($sportsevents as $eid => $sportsevent) {
			drush_print(sprintf("processing %s vs %s...", $sportsevent->visiting_team_name, $sportsevent->host_team_name));
			
			$sportsevent->winning_team_name = ($sportsevent->visiting_score > $sportsevent->host_score) 
				? $sportsevent->visiting_team_name
				: $sportsevent->host_team_name;
				
			if ($sportsevent->save()) {
				drush_log(sprintf("SportsEvent [%s] updated, setting winner to '%s'...", $eid, $sportsevent->winning_team_name), 'ok');
			}
		}
	}
}

/**
 * Add an event to the database
 * 			'season' => 'Season',
			'sport' => 'Affected Sport',
			'host' => 'Host Team Name (machine name)',
			'visitor' => 'Visiting Team Name (machine_name)',
			'event_date' => 'Date (and time) of the event',
			'week' => 'Week of the Event',
 * 
 */
function sportsevent_add_event($sport, $host, $visitor, $event_date, $week) {
	global $conf;
	
	$pickem_id = drush_get_option('pickem_id', FALSE);
	$pickem_week = drush_get_option('pickem_week', 1);
	$neutral = drush_get_option('neutral', FALSE);
	$venue_id = drush_get_option('venue_id', FALSE);	
	$noupdate = drush_get_option('noupdate', FALSE);
	$completed = drush_get_option('completed', FALSE);
	$season = drush_get_option('season', FALSE);
	$title = drush_get_option('title', FALSE);
	$tv = drush_get_option('tv', FALSE);
	$location = drush_get_option('location', FALSE);

	drush_print("Loading Teams.  Please wait...");
	
	$res = db_select('team', 't')
		->fields('t', array('name','conf_name'))
		->condition('sport', $sport)
		->execute();

	$teams = array();
	foreach ($res as $r) {
		$teams[$r->name] = $r->conf_name;
	}
	drush_log('Loaded all teams in sport, ' . $sport, 'ok');
	
	$venue = FALSE;
	if (!empty($venue_id) && module_exists('venue')) {
		drush_print("Loding Venue.  Please wait...");
		$venue = venue_load( $venue_id );
		drush_log(sprintf("Venue, %s (%s), loaded.", $venue->title, $venue->vid), 'ok');
	}

	$bundle = $sport;
	$entity = entity_create('sportsevent', array('type' => $bundle));
	$entity->game_type = $bundle;
	$entity->sport = $sport;
	
	$d = new DateTime($event_date);
	$entity->event_date = $d->getTimestamp();

	if (!empty($season)) {
		$entity->season = $season;
	} else {
		$entity->season = $conf['current_season'][$sport];
	}
	$entity->host_team_name = $host;
	$entity->visiting_team_name = $visitor;
	$entity->host_conf_name = $teams[$host];
	$entity->visiting_conf_name = $teams[$visitor];
	
	if ($venue) {
		// use the location from the venue
		$entity->location = $venue->location_city . ', ' . $venue->location_state;
		$entity->venue_id = $venue_id;
	} else {
		if (!is_bool($location) || ($location = drush_prompt('Location')) !== '') {
			$entity->location = $location;
		}
	}
	$entity->neutral = !empty($neutral) ? 'Y' : 'N';
	
	if (!empty($title)) {
		if (!is_bool($title) || ($title = drush_prompt('Enter Title')) !== '') {
			$entity->title = $title;
		}
	}
	
	if (!empty($completed)) {
		// get the score
		// from the NCAA
		//	$entity->summary_url = "http://ncaa.com" . $gameSummaryLink;
		
	} else {
		$entity->completed = 'N';
		$entity->summary_url = '';
	}
	drush_print_r($entity);
	if (empty($noupdate) && $entity->save()) {
		// saved, tell me so i can rest easy
		drush_log("Event saved", 'ok');
		if (!empty($pickem_id)) {
			drush_print('Adding to pickem...');
			$result = db_insert('pickem_slate')
				->fields(array(
					'slid' => NULL,
					'pid' => $pickem_id,
					'event_entity_id' => $entity->eid,
					'event_entity_type' => 'sportsevent',
					'season' => $entity->season,
					'week' => $pickem_week,
					'slate_date' => $entity->event_date,
					'status' => 1,
				))->execute();
		}
	}
}
/**
 * Fetch the sporting evnet scores from a reliable
 * datasource
 */
function sportsevent_fetch_ncaa_scores() {
	global $conf;
	$sport = drush_get_option('sport', 'ncaaf');
	$fix_events = drush_get_option('fix', FALSE);
	if (!empty($fix_events)) {
		sportsevent_fix_ncaam_scores();
		return;
	}
	$pickem_id = drush_get_option('pickem_id', FALSE);
	$week = drush_get_option('week', FALSE);
	$week_start = drush_get_option('week_start', FALSE);	
	$noupdate = drush_get_option('noupdate', FALSE);
	$ignore_event_status = drush_get_option('ignore_event_status', FALSE);
	$parse_venues = drush_get_option('parse_venues', FALSE);
	$auto_add_event = drush_get_option('auto_add', FALSE);

	$game_date = drush_get_option('date', FALSE);
	if ($game_date) {
		// convert to timestamp
		$dt = new DateTime($game_date);
		$game_date = $dt->getTimestamp();
	}
	$game_date_start = drush_get_option('date_start', FALSE);
	if ($game_date_start) {
		$dt = new DateTime($game_date_start);
		$game_date_start = $dt->getTimestamp();
	}
	
	$pickem = FALSE;
	if ($pickem_id) {
		$pickem = pickem_load($pickem_id);
		drush_log("loaded pickem, '" . $pickem->title . "'...", 'ok');
	}
	
	$proposed_venues = array();
	
	// determine URL list for the sport in question
	$urls = SportsEvent::scoreboardUrls($sport, $week, $week_start, $game_date, $game_date_start);
	drush_print_r($urls);
	
	// team mappings for ncaa display names to our convention
	// defines the $team variable
	include_once(drupal_get_path('module', 'sportsfan') . '/data/teams_appendix.inc');
	$unknowns = array();
	
	drush_log("Loading teams for " . SportsEvent::$sport[$sport] . "...", 'ok');
	$res = db_select('team', 't')
		->fields('t', array('displaytitle', 'name', 'conf_name'))
		->condition('sport', $sport)
		->execute();
		
	if (!isset($teams)) {
		$teams = array();
	}
	foreach ($res as $team) {
		$teams[$team->displaytitle] = $team->name;
	}

	libxml_use_internal_errors( FALSE );
	foreach ($urls as $url) {
		drush_print("URL to load: " . $url);
		$in = drush_prompt("(L)oad, (S)kip, or (Q)uit", "L");
		if (strtolower($in) == 's') {
			continue;
		}
		if (strtolower($in) == 'q') {
			break;
		}
		# open the file with file_get_contents and set a timeout context
		$ctx = stream_context_create(array('http' => array('timeout' => 15.0)));
		$file = file_get_contents($url, 0, $ctx);
		
		$doc = new DOMDocument();
		// supress DOM parsing errors
		@$doc->loadHTML( $file );
		
		$xpath = new DOMXPath( $doc );
		$elements = $xpath->query("//section[contains(concat(' ', @class, ' '), ' game final')]");
		if ($elements->length == 0) {
			// either a timeout occurred or there were no events for this date
			continue;
		}

		// PROCESS each event
		foreach ($elements as $element) {
			//drush_print($element->getAttribute('class'));
			$headerElement = $xpath->query('h3', $element);
			
			$match = array();
			preg_match('/^(?P<visitor>[\w\s\(\)\.]+) vs (?P<host>[\w\s\(\)\.]+)$/', $headerElement->item(0)->nodeValue, $match);
			
			$schoolElements = $xpath->query("div[@class='game-contents']//td[@class='school']", $element);
			$scoreElements = $xpath->query("div[@class='game-contents']//td[@class='final score']", $element);
			$linkElements = $xpath->query("div[@class='game-contents']//ul[contains(concat(' ', @class, ' '), ' linklist')]/li/a", $element);
			$gameSummaryLink = (string) $linkElements->item(0)->getAttribute('href');
			
			$overtimes = 0;
			# query for the element in the line score table just for the "final" TH
			$linescoreHeaderElement2 = $xpath->query(
				"div[@class='game-contents']//table[@class='linescore']//th[@class='final']/preceding-sibling::th[1]", $element);
			if ($linescoreHeaderElement2->length == 1) {
				$fs_match = array();
				if (preg_match('/^(?P<overtimes>\d{1})?OT$/', $linescoreHeaderElement2->item(0)->nodeValue, $fs_match) == 1) {
					$overtimes = isset($fs_match['overtimes']) ? $fs_match['overtimes'] : 1;
				}
			}
			
			$v = $schoolElements->item(0);
			$h = $schoolElements->item(1);
			
			$visitor = $xpath->query("div[@class='team']/a", $v);
			$host = $xpath->query("div[@class='team']/a", $h);
			
			$visitor_score = $scoreElements->item(0)->nodeValue;
			$host_score = $scoreElements->item(1)->nodeValue;
			
			$visitor_title = $visitor->item(0)->nodeValue;
			$host_title = $host->item(0)->nodeValue;
			
			$z = TRUE;
			if (!isset($teams[$visitor_title])) {
				$unknowns["'" . $visitor_title . "'"] = "'" . $visitor_title . "'";
				$z = FALSE;
			}
			if (!isset($teams[$host_title])) {
				$unknowns["'" . $host_title . "'"] = "'" . $host_title . "'";
				$z = FALSE;
			}
			if (!empty($parse_venues)) {
				_build_venues_list("http://ncaa.com" . $gameSummaryLink, $proposed_venues);
				#drush_print_r($proposed_venues);
				continue;
			}
			if ($z) {
				$visiting_team_name = $teams[$visitor_title];
				$host_team_name = $teams[$host_title];
				drush_print_r(array('visitor' => $visiting_team_name, 'host' => $host_team_name));
				$select = db_select('sportsevent', 's')
					->fields('s', array('eid'))
					->condition(
						db_or()->condition(db_and()
						->condition('visiting_team_name', $visiting_team_name)
						->condition('host_team_name', $host_team_name)
						)->condition(db_and()
						->condition('visiting_team_name', $host_team_name)
						->condition('host_team_name', $visiting_team_name)
						)
					)
					->condition('sport', $sport)
					->condition('season', $conf['current_season'][$sport]);
				if (empty($ignore_event_status)) {
					$select->condition('completed', 'N');
				}
				$result = $select->execute();

				$g = $result->fetchObject();
				if ($g == FALSE) {
					drush_print(sprintf("I could not find a sporting event for %s vs %s", $host_team_name, $visiting_team_name));
					if (!empty($auto_add_event) || drush_confirm("Shall I add it for you?")) {
						// add the game
						drush_print("adding sporting event...");
						$event_info = _parse_event_info("http://ncaa.com" . $gameSummaryLink);
						
						$entity_type = 'sportsevent';
						#$bundle = isset($g['gsx$bundle']['$t']) ? $g['gsx$bundle']['$t'] : 'college-game';
						$bundle = $sport;
						$entity = entity_create('sportsevent', array('type' => $bundle));
						$entity->game_type = $bundle;
						$entity->sport = $sport;
						if (isset($event_info['month'])) {
							$entity->event_date = mktime(0, 0, 0, $event_info['month'], $event_info['day'], $event_info['year']);
						} else {
							$entity->event_date = 0;
						}
						$entity->season = $conf['current_season'][$sport];
						$entity->host_team_name = $host_team_name;
						$entity->visiting_team_name = $visiting_team_name;
						$entity->host_score = $host_score;
						$entity->visiting_score = $visitor_score;
						$entity->location = $event_info['location'];
						$entity->completed = 'Y';
						$entity->summary_url = "http://ncaa.com" . $gameSummaryLink;
						if ($entity->save()) {
							// saved, tell me so i can rest easy
							drush_log("Event saved", 'ok');
						}
					}
					continue;
				}
				drush_log("loading sportsevent[".$g->eid."]...", 'ok');
				$o = sportsevent_load($g->eid);
				if ($o) {
					$o->overtimes = $overtimes;
					$o->summary_url = "http://ncaa.com" . $gameSummaryLink;
					$o->visiting_score = $visitor_score;
					$o->host_score = $host_score;
					$o->completed = 'Y';
					if ($o->visiting_team_name == $host_team_name) {
						$o->host_team_name = $host_team_name;
						$o->visiting_team_name = $visiting_team_name;
						$o->host_score = $host_score;
						$o->visiting_score = $visitor_score;

						$tmp_conf = $o->visiting_conf_name;
						$o->visiting_conf_name = $o->host_conf_name;
						$o->host_conf_name = $tmp_conf;
						/*
						$o->visiting_team_name = $visiting_team_name;
						$o->host_team_name = $host_team_name;
						$o->visiting_score = $host_score->nodeValue;
						$o->host_score = $visitor_score->nodeValue;
						*/
					}
					if (TRUE) {
						// fix the error decro made the other day
						$o->winning_team_name = ($o->visiting_score > $o->host_score) ?
							$o->visiting_team_name :
							$o->host_team_name;
	
						#drush_print_r($o);
							
						$event_string = sprintf("[%s] %s - %s, %s - %s", 
							$o->eid,
							(($o->winning_team_name == $o->visiting_team_name) ? "(*)" : "") . $o->visiting_team_name, 
							$o->visiting_score, 
							(($o->winning_team_name == $o->host_team_name) ? "(*)" : "") . $o->host_team_name, 
							$o->host_score);
							
						if (empty($noupdate)) {
							drush_print($o->summary_url);
							if ($pickem) {
								// drush_print('Prepare to update pickem results for ' . $pickem->title . '...');
								_process_sportsevent_result_for_pickem($event, $pickem);
							}
							if ($o->save()) {
								drush_log("Saved Event: " . $event_string, "ok");
							} else {
								drush_log("Unable to save event", "error");
							}
						}
					}
				}
			}
		}
		if (!empty($unknowns)) {
			drush_print_r($unknowns);
		}
	}
	if (!empty($proposed_venues)) {
		drush_print_r($proposed_venues);
		ksort($proposed_venues);
		foreach ($proposed_venues as $school => $location) {
			foreach ($location as $venue) {
				$out = array($school, $location, $venue);
				drush_print(implode("\t", $out));
			}
		}
	}
}

function sportsevent_fix_ncaaf_scores() {
	$unknowns = array();
	
	include_once(drupal_get_path('module', 'sportsfan') . '/data/teams_appendix.inc');
	$urls = array(
		'1' => 'http://www.ncaa.com/game/football/fbs/2014/08/30/elon-duke',
	);
	$noupdate = drush_get_option('noupdate', FALSE);
	
	drush_log("Loading teams for football...", 'ok');
	$res = db_select('team', 't')
		->fields('t', array('displaytitle', 'name'))
		->condition('sport', 'ncaaf')
		->execute();
		
	if (!isset($teams)) {
		$teams = array();
	}
	foreach ($res as $team) {
		$teams[$team->displaytitle] = $team->name;
	}

	libxml_use_internal_errors( FALSE );
	
	foreach ($urls as $wk => $url) {
		drush_print("URL to load: " . $url);
		$in = drush_prompt("(L)oad, (S)kip, or (Q)uit", "L");
		if (strtolower($in) == 's') {
			continue;
		}
		if (strtolower($in) == 'q') {
			break;
		}
		# open the file with file_get_contents and set a timeout context
		$ctx = stream_context_create(array('http' => array('timeout' => 15.0)));
		$file = file_get_contents($url, 0, $ctx);
		
		$doc = new DOMDocument();
		// supress DOM parsing errors
		@$doc->loadHTML( $file );
		
		$xpath = new DOMXPath( $doc );
		$elements = $xpath->query("//p[@class='location']");
		drush_print($elements->item(1)->nodeValue);
		$m = array();
		preg_match('/^(?P<venue_name>[\w\s\-]+),\s(?P<location_city>[\w\s-]+),\s(?P<location_state>\w{2})$/', $elements->item(1)->nodeValue, $m);
		drush_print_r($m);
		return;
		
		foreach ($elements as $element) {
			// get the game title
			drush_print($element->getAttribute('class'));
			$headerElement = $xpath->query('h3', $element);
			
			$match = array();
			preg_match('/^(?P<visitor>[\w\s\(\)\.]+) vs (?P<host>[\w\s\(\)\.]+)$/', $headerElement->item(0)->nodeValue, $match);
			
			$schoolElements = $xpath->query("div[@class='game-contents']//td[@class='school']", $element);
			$scoreElements = $xpath->query("div[@class='game-contents']//td[@class='final score']", $element);
			
			$v = $schoolElements->item(0);
			$h = $schoolElements->item(1);
			
			$visitor = $xpath->query("div[@class='team']/a", $v);
			$host = $xpath->query("div[@class='team']/a", $h);
			
			$visitor_score = $scoreElements->item(0);
			$host_score = $scoreElements->item(1);
			
			$visitor_title = $visitor->item(0)->nodeValue;
			$host_title = $host->item(0)->nodeValue;

			$visiting_team_name = isset($teams_app[$visitor_title]) ? $teams_app[$visitor_title] : $teams[$visitor_title];
			$host_team_name = isset($teams_app[$host_title]) ? $teams_app[$host_title] : $teams[$host_title];
			
			$z = TRUE;
			if (!isset($teams[$visitor_title]) && !isset($teams_app[$visitor_title])) {
				$unknowns["'" . $visitor_title . "'"] = "'" . $visitor_title . "'";
				$z = FALSE;
			}
			if (!isset($teams[$host_title]) && !isset($teams_app[$host_title])) {
				$unknowns["'" . $host_title . "'"] = "'" . $host_title . "'";
				$z = FALSE;
			}
			if ($z) {
				$result = db_select('sportsevent', 's')
					->fields('s')
					->condition(
						db_or()->condition(db_and()
						->condition('visiting_team_name', $visiting_team_name)
						->condition('host_team_name', $host_team_name)
						)->condition(db_and()
						->condition('visiting_team_name', $host_team_name)
						->condition('host_team_name', $visiting_team_name)
						)
					)
					->condition('sport', 'ncaaf')
					->condition('season', '2014')
					->execute();

				$o = $result->fetchObject();
				if ($o) {

					$o->visiting_score = $visitor_score->nodeValue;
					$o->host_score = $host_score->nodeValue;
					$o->completed = 'Y';
					if ($o->visiting_team_name == $host_team_name) {
						$o->visiting_team_name = $visiting_team_name;
						$o->host_team_name = $host_team_name;
						$o->visiting_score = $host_score->nodeValue;
						$o->host_score = $visitor_score->nodeValue;
					}
					if (TRUE) {
						// fix the error decro made the other day
						$o->winning_team_name = ($o->visiting_score > $o->host_score) ?
							$o->visiting_team_name :
							$o->host_team_name;
	
						drush_print_r($o);
						$update_o = (array) $o;
						$eid = $update_o['eid'];
						unset(
							$update_o['week'],
							$update_o['eid'], 
							$update_o['sport'], 
							$update_o['event_date'], 
							$update_o['neutral'],
							$update_o['season']
							);

						$update = db_update('sportsevent')
							->fields($update_o)
							->condition('eid', $eid);
							
						drush_print(sprintf("[%s] %s - %s, %s - %s", 
							$eid,
							(($o->winning_team_name == $o->visiting_team_name) ? "(*)" : "") . $o->visiting_team_name, 
							$o->visiting_score, 
							(($o->winning_team_name == $o->host_team_name) ? "(*)" : "") . $o->host_team_name, 
							$o->host_score)
							);
							
						//drush_print($update->__toString());
						if ($noupdate !== FALSE) {
							$rows_affected = $update->execute();
							drush_log($rows_affected . ' row(s) updated', 'ok');
						}
					}
					// discover if the teams are in the correct order

				}
			}
		}
		drush_print_r($unknowns);
	}
}

function sportsevent_load_lines() {
	
	$season = drush_get_option('season', 2014);
	$sport = drush_get_option('sport', 'ncaaf');

	// this comes in handy later on...
	$validateSBName = array();
	
	$doc = FALSE;
	try {
		$doc = new DOMDocument();
		@$doc->loadHTMLFile('https://app.sportsbooks.com/sbk/sportsbook4/ncaa-football-ncaaf-game-lines-sports.sbk');
	} catch (Exception $parseException) {
		drush_log($parseException->getMessage(), 'error');
	}
	
	$xpath = new DOMXPath($doc);
	$elements = $xpath->query("//div[contains(concat(' ', @class, ' '), ' eventbox ')]");
	drush_print_r($elements);
	
	if (!is_null($elements)) {
	  foreach ($elements as $element) {
			$visitingTeamElement = $xpath->query("div//span[@id='awayTeamName']", $element);
			$visiting_team = str_replace(' (N)', '', $visitingTeamElement->item(0)->nodeValue);
			$visitingPointSpreadElement = $xpath->query("div//a[@ate='PSA']/div", $element);
			$visiting_team_spread = preg_replace('/\(.+\)/', '', $visitingPointSpreadElement->item(0)->nodeValue);
			$hostTeamElement = $xpath->query("div//span[@id='homeTeamName']", $element);
			$host_team = str_replace(' (N)', '', $hostTeamElement->item(0)->nodeValue);
			$hostTeamSpreadElement = $xpath->query("div//a[@ate='PSH']/div", $element);
			$host_team_spread = preg_replace('/\(.+\)/', '', $hostTeamSpreadElement->item(0)->nodeValue);
			drush_print_r(array(
				'visiting team' => $visiting_team, 
				'visiting team point spread' => $visiting_team_spread,
				'host team' => $host_team,
				'host team point spread' => $host_team_spread)
			);
			drush_print(((int) $visiting_team_spread < (int) $host_team_spread) ? "visitor favored" : "host favored");

			#$neutral_site = !empty($title_match['neutral_site']);
			$neutral_site = FALSE;
			#continue;
	
			$visitingTeam = FALSE;
			$query = new EntityFieldQuery();
			$teams = $query->entityCondition('entity_type', 'team')
				->propertyCondition('sportsbook_name', '%' . $visiting_team . '%', 'LIKE')
				->execute();
			
			if (isset($teams['team'])) {
				$tids = array_keys($teams['team']);
				$entities = entity_load('team', $tids);
				
				// the search based on 'sportsbook_name' is a work in progress
				if (count($entities) >= 1) {
					$options = array();
					foreach ($entities as $e) {
						$current = explode(',', $e->sportsbook_name);
						if (in_array($visiting_team, $current)) {
							$visitingTeam = $e;
							$options = array();
							break;
						} else {
							$options[$e->tid] = $e->displaytitle . ' (' . $e->sportsbook_name . ')';
						}
					}
					if (count($options) && $choice = drush_choice($options, 'Select the team that best fits "' . $visiting_team . '"')) {
						// $visitingTeam = $entities[$choice - 1];
						$visitingTeam = $entities[$choice];
					}
				}
			}
			else {
				drush_log(sprintf('Unable to locate away team with SportsBook title of "%s"', $visiting_team), 'warning');
				$validateSBName[] = $visiting_team;
			}
	
			$hostTeam = FALSE;
			$query = new EntityFieldQuery();
			$teams = $query->entityCondition('entity_type', 'team')
				->propertyCondition('sportsbook_name', '%' . $host_team . '%', 'LIKE')
				->execute();
			if (isset($teams['team'])) {
				$tids = array_keys($teams['team']);
				$entities = entity_load('team', $tids);
				
				// the search based on 'sportsbook_name' is a work in progress
				if (count($entities) >= 1) {
					$options = array();
					foreach ($entities as $e) {
						$current = explode(',', $e->sportsbook_name);
						if (in_array($host_team, $current)) {
							$hostTeam = $e;
							$options = array();
							break;
						} else {
							$options[$e->tid] = $e->displaytitle . ' (' . $e->sportsbook_name . ')';
						}
					}
					if (count($options) && $choice = drush_choice($options, 'Select the team that best fits "' . $host_team . '"')) {
						// $visitingTeam = $entities[$choice - 1];
						$hostTeam = $entities[$choice];
					}
				}
			}
			else {
				drush_log(sprintf('Unable to locate home team with SportsBook title of "%s"', $host_team), 'warning');
				$validateSBName[] = $host_team;
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
				drush_log('Unable to locate a game with these two teams -- ' 
				. sprintf("%s, %s", $visiting_team, $host_team), 
				'warning');
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
			
			$last_favorite = $sportsevent->favorite;
			
			if (((int) $visiting_team_spread == 0)) {
				$sportsevent->favorite = NULL;
			} else {
				if (((int) $visiting_team_spread < (int) $host_team_spread)) {
					$sportsevent->favorite = $visitingTeam->name;
				} else {
					$sportsevent->favorite = $hostTeam->name;
				}
			}
			
			if ($host_team_spread != $sportsevent->betting_line) {
				drush_log(sprintf("line changed from %s to %s", $sportsevent->betting_line, $host_team_spread), 'ok');
				
				$log_message = t("!visitor at !host - line changed from !last_line to !new_line",
					array(
						'!visitor' => $visitingTeam->displaytitle,
						'!host' => $hostTeam->displaytitle,
						'!last_line' => $sportsevent->betting_line,
						'!new_line' => $host_team_spread
					)
				);
				// log it in the database, too
				watchdog('!sport - Week !week', $log_message, 
					array('!sport' => $sportevent->sport, '!week' => $sportsevent->week),
					WATCHDOG_INFO
				);
			}
	
			$sportsevent->betting_line = $host_team_spread;
			//$sportsevent->money_line = $desc_match['money_line'];
			
			if (empty($sportsevent->twitter_hashtag)) {
				if (!empty($visitingTeam->twitter_hashtag) && !empty($hostTeam->twitter_hashtag)) {
					$sportsevent->twitter_hashtag = sprintf("#%svs%s", $visitingTeam->twitter_hashtag, $hostTeam->twitter_hashtag);
				}
			}
			try {
				$sportsevent->save();
				drush_log(sprintf("Changes to '%s' have been saved.", $sportsevent->displayTitle()), 'success');
			} catch(Exception $saveException) {
				drush_log($saveException->getMessage(), 'error');
			}
			drush_print("");
	    
		}
	}
	
	return;
	
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
		
		drush_print_r($title_match);
		//drush_print_r($desc_match);
	}
		
	if (!empty($validateSBName)) {
		drush_print("Please verify these team names from Sportsbook: ");
		foreach ($validateSBName as $v) {
			drush_print($v);
		}
	}
	
}
