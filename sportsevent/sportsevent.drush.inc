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
			
			if (strtoupper($input) == 'Q') {
				break;
			}
			
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
