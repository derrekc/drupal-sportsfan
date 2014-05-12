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
  return $items;
}

/**
 * Drush callback.
 */
function sportsevent_games_import_google_data() {
	
	$pickems = pickem_load_multiple(FALSE);
	$pickem_options = array();
	$options = array();
	foreach ($pickems as $p) {
		$options[] = $p->title;
		$pickem_options[] = $p;
	}

	$option = drush_choice($options, dt('Select a pickem'));
	if ($option === FALSE) {
		return;
	}
	$pickem = $pickem_options[$option];

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
				$game_entity = sportsevent_load_multiple($g['gsx$host']['$t'], $g['gsx$visitor']['$t'], array('season' => $g['gsx$season']['$t']));
				if (!empty($game_entity)) {
					continue;
				}
				$entity_type = 'sportsevent';
				$bundle = isset($g['gsx$bundle']['$t']) ? $g['gsx$bundle']['$t'] : 'college-game';
				$bundle = 'college-game';
				$game_entity = '';
				$game_entity = entity_create($entity_type, array('type' => $bundle));	
				$game_entity->game_type = $bundle;
				
				$game_entity->season = $g['gsx$season']['$t'];
				$date = new DateTime($g['gsx$date']['$t']);
				$game_entity->game_date = $date->getTimestamp();
				$game_entity->week = $g['gsx$week']['$t'];
				$game_entity->visiting_school_name = $g['gsx$visitor']['$t'];
				$game_entity->host_school_name = $g['gsx$host']['$t'];
				
				// TODO get conf_name for school
				try {
					$host_team = team_load_by_name($game_entity->host_school_name);
					$game_entity->host_conf_name = $host_team->conf_name;
				} catch (Exception $e) {
					drush_log($e->getMessage(), 'error');
				}
				try {
					$visiting_team = team_load_by_name($game_entity->visiting_school_name);
					$game_entity->visiting_conf_name = $visiting_team->conf_name;
				} catch (Exception $e) {
					drush_log($e->getMessage(), 'error');
				}	
				
				$game_entity->sport = $g['gsx$sport']['$t'];
				$game_entity->tv = $g['gsx$tv']['$t'];
				$game_entity->neutral = $g['gsx$neutral']['$t'] == 'TRUE' ? 'Y' : 'N';
				$game_entity->location = $g['gsx$location']['$t'];
				
				if ($game_entity->save() !== FALSE) {
					drush_log(sprintf('Added "%s" to Sports Event database', $game_entity->getDisplayTitle()), 'ok');

					if ($pickem->sport != $game_entity->sport) {
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
						$patt_b = sprintf('/%s/', $game_entity->host_conf_name);
						if ((preg_match($patt_a, $pickem->autoconference) == 1) || (preg_match($patt_b, $pickem->autoconference) == 1)) {
							// add game to slate
							$addToSlate = TRUE;
						}
						else {
							$patt_a = sprintf('/%s/', $game_entity->visiting_school_name);
							$patt_b = sprintf('/%s/', $game_entity->host_school_name);
							if ((preg_match($patt_a, $pickem->autoschool) == 1) || (preg_match($patt_b, $pickem->autoschool) == 1)) {
								// add game to slate
								$addToSlate = TRUE;
							}
						}
					}
					if ($addToSlate && module_exists('pickem')) {						
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
}
