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
  return $items;
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
					drush_log(sprintf('Added "%s" to Team database', $entity->displaytitle), 'ok');
				}
			}
		} catch(Exception $e) {
			drush_log('Can\'t save teams to the database', 'error');
		}
	}
  drush_log('Import complete', 'success');
}
