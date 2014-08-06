<?php


/**
 * @file
 * Team Drush script.
 */

/**
 * Implements hook_drush_command().
 */
function team_drush_command() {
  $items = array();
  $items['import-teams'] = array(
    'callback' => 'team_import_google_data',
    'description' => dt('Import Teams from Google Spreadsheet.'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
   $items['import-conferences'] = array(
    'callback' => 'conference_import_google_data',
    'description' => dt('Import Conferences from Google Spreadsheet.'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
	$items['add-sbname'] = array(
		'callback' => 'team_add_sportsbook_name',
		'arguments' => array(
			'teamname' => 'Team name to lookup',
			'sportsbook_name' => 'Sportsbook name to add',
		),
		'required-arguments' => 2,
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['sbname-list'] = array(
		'callback' => 'team_sportsbook_names',
		'arguments' => array(
			'teamname' => 'Team name to lookup',
		),
		'required-arguments' => 1,
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
  return $items;
}

/**
 * Drush callback
 */
function team_add_sportsbook_name($teamname, $sportsbook_name) {
	$team = team_load_by_name($teamname);
	if (!$team) {
		drush_log(sprintf("Could not load the team, '%s'!", $teamname), 'error');
		return;
	}
	$controller = entity_get_controller('team');
	try {
		$controller->addSportsbookNameToTeam($team, $sportsbook_name);
		drush_log($sportsbook_name . ' added successfully to ' . $teamname . '\'s list.', 'success');
	} catch (Exception $addException) {
		drush_log($addException->getMessage(), 'error');
	}
}

/**
 * Drush callback
 */
function team_sportsbook_names($teamname) {
	$team = team_load_by_name($teamname);
	if (!$team) {
		drush_log(sprintf("Could not load the team, '%s'!", $teamname), 'error');
		return;
	}
	$current = explode(',', $team->sportsbook_name);
	drush_print('Known Sportsbook aliases: ' . implode(', ', $current));
}

/**
 * Drush callback.
 */
function team_import_google_data() {
	$worksheets = GoogleSpreadsheetManager::worksheets(GoogleSpreadsheetManager::SCHOOLS_SPREADSHEET_KEY);
	//drush_print_r($worksheets);
	
	$prompt = array();
	foreach ($worksheets as $k => $w) {
		$prompt[] = $w['label'];
	}
	if (count($worksheets) > 1) {
		$prompt[] = dt('All');
	}
	$option = '';
	if (($option = (int) drush_choice($prompt)) === FALSE) {
		return;
	}

	if ($option < (count($worksheets) + 1)) {
		$worksheets = (array) array_slice($worksheets, $option-1, 1);
	}

	foreach ($worksheets as $ws) {
		try {
			drush_print(sprintf('Reading from "%s"', $ws['href']));
			$schools = GoogleSpreadsheetManager::spreadsheetEntries($ws['href']);
			foreach ($schools as $s) {
				$entity_type = 'team';
				$entity = entity_create($entity_type, array('type' => 'college-team'));
				$entity->title = $s['gsx$title']['$t'];
				$entity->displaytitle = $s['gsx$displaytitle']['$t'];
				$entity->name = $s['gsx$name']['$t'];
				$entity->nickname = $s['gsx$nickname']['$t'];
				$entity->league = 'NCAA';
				$entity->conf_name = $s['gsx$confname']['$t'];
				$entity->conf_division = $s['gsx$confdivision']['$t'];
				$entity->sport = $s['gsx$sport']['$t'];
				if (!empty($s['gsx$sportsbookname']['$t'])) {
					$entity->sportsbook_name = $s['gsx$sportsbookname']['$t'];
				} else {
					$entity->sportsbook_name = preg_replace('/\s+/', '', $entity->title);
				}
				if ($s['gsx$title_ineligible'] == 'TRUE') {
					$entity->title_ineligible = 1;
				}
				else {
					$entity->title_ineligible = 0;
				}
				if ($entity->save() !== FALSE) {
					drush_log(sprintf('Added "%s" to Team table', $entity->displaytitle), 'ok');
				}
			}
		} catch(Exception $e) {
			drush_log('Can\'t save teams to the database', 'error');
		}
	}
  drush_log('Import complete', 'success');
}


/**
 * Drush callback.
 */
function conference_import_google_data() {
	$worksheets = GoogleSpreadsheetManager::worksheets(GoogleSpreadsheetManager::CONF_SPREADSHEET_KEY);
	//drush_print_r($worksheets);
	
	$prompt = array();
	foreach ($worksheets as $k => $w) {
		$prompt[] = $w['label'];
	}
	if (count($worksheets) > 1) {
		$prompt[] = dt('All');
	}
	$option = '';
	if (($option = (int) drush_choice($prompt)) === FALSE) {
		return;
	}

	if ($option < (count($worksheets) + 1)) {
		$worksheets = (array) array_slice($worksheets, $option-1, 1);
	}

	foreach ($worksheets as $ws) {
		try {
			drush_print(sprintf('Reading from "%s"', $ws['href']));
			$conferences = GoogleSpreadsheetManager::spreadsheetEntries($ws['href']);
			foreach ($conferences as $c) {
				$entity_type = 'conference';
				$entity = entity_create($entity_type, array('type' => 'conference'));
				$entity->title = $c['gsx$title']['$t'];
				$entity->displaytitle = $c['gsx$displaytitle']['$t'];
				$entity->name = $c['gsx$name']['$t'];
				$entity->league = $c['gsx$league']['$t'];
				if ($entity->save() !== FALSE) {
					drush_log(sprintf('Added "%s" to Conference table', $entity->displaytitle), 'ok');
				}
			}
		} catch(Exception $e) {
			drush_log('Can\'t save conferences to the database', 'error');
		}
	}
  drush_log('Import complete', 'success');
}

