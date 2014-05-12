<?php

function pickem_drush_command() {
	$items = array();

	$items['pickem-load-lines'] = array(
		'description' => 'Load Betting Lines from XML RSS feed',
		'aliases' => array('pklines'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['load-conf-games'] = array(
		'description' => 'Load games for a conference',
		'aliases' => array('lcgames'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'arguments' => array(
			'conf' => 'The (machine) name of the conference',
		),
	);
  $items['import-pickems'] = array(
    'callback' => 'pickem_import_pickem_google_data',
    'description' => dt('Import Pickems from Google Spreadsheet.'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
	$items['refresh-schools'] = array(
		'description' => 'Load unprocessed schools from "schools.tsv"',
		'aliases' => array('schools'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['update-stats'] = array(
		'description' => 'Update the standings for the schools',
		'aliases' => array('sstats'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['load-stats'] = array(
		'aliases' => array('lstats'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'description' => 'Load and update the stats for each school',
	);
	$items['import-fb-poll'] = array(
		'description' => 'Import AP football poll',
		'aliases' => array('fbpoll'),
		'arguments' => array(
			'week' => 'The week for the poll',
			'season' => 'The season.  Defaults to the current season',
		),
	);
	$items['reset-players'] = array(
		'description' => 'Reset the players in the pickem',
		'aliases' => array('prp', 'pkreset'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['create-standings'] = array(
		'description' => 'Create the standings',
		'aliases' => array('cstandings'),
		'arguments' => array(
			'target_week' => 'Week to process',
		),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['set-game-times'] = array(
		'description' => 'Update Game Times',
		'aliases' => array('gtimes'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['set-game-scores'] = array(
		'description' => 'Update Game Scores',
		'aliases' => array('gscores'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['project-acc'] = array(
		'description' => 'Project ACC 2013 Final Results',
		'aliases' => array('accproj'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	$items['export-schools'] = array(
		'description' => 'Export schools to master "team" file',
		'aliases' => array('expsch'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
	);
	return $items;
}

/**
 * Drush callback.
 */
function pickem_import_pickem_google_data() {
	$worksheets = GoogleSpreadsheetManager::worksheets(GoogleSpreadsheetManager::PICKEM_SPREADSHEET_KEY);
	
	$prompt = array();
	foreach ($worksheets as $k => $w) {
		$prompt[] = $w['label'];
	}
	if (count($worksheets) > 1) {
		$prompt[] = dt('All');
	}
	$option = drush_choice($prompt, 'Select datasource');
	if ($option === FALSE) {
		return;
	}

	if ($option < (count($worksheets) + 1)) {
		$worksheets = (array) array_slice($worksheets, $option-1, 1);
	}

	foreach ($worksheets as $ws) {
		try {
			drush_print(sprintf('Reading from "%s"', $ws['href']));
			$pickems = GoogleSpreadsheetManager::spreadsheetEntries($ws['href']);
			foreach ($pickems as $p) {
				$entity_type = 'pickem';
				$bundle = $p['gsx$bundle']['$t']; 
				$entity = entity_create($entity_type, array('type' => $bundle));
				$entity->pickem_type = $bundle;
				
				$keys = array('title','name','season','sport','active','autoconference',
					'autoschool','description','weeks','startdate','joindeadline','firstdayofweek',
					'pickformat','picksperweek',);
				foreach($keys as $key) {
					$entity->{$key} = $p['gsx$'.$key]['$t'];
				}
				if ($entity->save() !== FALSE) {
					drush_log(sprintf('Added "%s" to Pickem database', $entity->title), 'ok');
					
					if ($entity->active == 1) {
						// create an instance
						$instances = array(
							array(
								'entity_type' => 'pickem',
								'bundle' => $bundle,
								'field_name' => 'pick_default',
								'label' => t('Default Choice for Picks'),
								'description' => t('This will be the default selection for each game when making picks.'),
								'type' => 'list_text',
								'widget' => array(
									'type' => 'options_select',
									'settings' => array(
										'allowed_values' => array(
											'hometeam' => t('Home Team'),
											'visitingteam' => t('Visiting Team'),
											'topranked' => t('Team with Highest Ranking'),
										),
									),
								),
							),
							array(
								'entity_type' => 'pickem',
								'bundle' => $bundle,
								'field_name' => 'max_bye_weeks_allowed',
								'label' => t('Maximum number of bye weeks allowed'),
								'description' => t('Number of bye weeks allowed in this pickem'),
								'type' => 'number_integer',
								'settings' => array(
									'min' => 0,
								)
							)
						);
						foreach ($instances as $i) {
							try {
								field_create_instance($i);
							} catch (Exception $e) {
								drush_log($e->getMessage(), 'error');
							}
						}
					}
				}
			}
		} catch(Exception $e) {
			drush_log('Can\'t save pickems to the database', 'error');
		}
	}
  drush_log('Import complete', 'success');
}

function drush_pickem_create_standings($target_week) {
	$num_affected = db_delete('standings')->execute();
	
	# $pickdata = pickem_load_pick_data($form_state['values']['standings']['week'], 2013);
	drush_print('loading pick data...');
	drush_print(variable_get('pickem.current_week'));
	$pickdata = pickem_load_pick_data(-1, variable_get('pickem.current_season', 2013));
	$season = variable_get('pickem.current_season', 2013);
	$current_week = variable_get('pickem.current_week');
	
	foreach ($pickdata['player'] as $player => $data) {
		// handle each week
		foreach ($data['week'] as $week => $stats) {
			if ($week <= $current_week) {
				drush_print('Processing ' . $player . ' on week ' . $week . '...');
				$a = db_insert('standings')
					->fields(array(
						'season' => $season,
						'week' => $week,
						'uid' => $data['uid'],
						'correct' => $stats['correct'],
						'incorrect' => $stats['incorrect'], 
					))
					->execute();
			}
		}
		
		// then handle the overall (week = -1)
		$a = db_insert('standings')
			->fields(array(
				'season' => $season,
				'week' => -1,
				'uid' => $data['uid'],
				'correct' => $data['correct'],
				'incorrect' => $data['incorrect'], 
			))
			->execute();
	}
}

function drush_pickem_update_stats() {
	require_once(drupal_get_path('module', 'pickem') . '/pickem.stats.inc');
	
	# get the schools currently in the standings table
	db_delete('school_records')
		->condition('season', variable_get('pickem.current_season', 2013))
		->condition('sport', 'ncaaf')
		->execute();

	db_delete('school_points')
		->condition('season', variable_get('pickem.current_season', 2013))
		->condition('sport', 'ncaaf')
		->execute();
		
	$records_res = db_query('SELECT * FROM school_records WHERE season = :season AND sport = :sport',
		array(':season' => variable_get('pickem.current_season', 2013), ':sport' => 'ncaaf'));
		
	$points_res = db_query('SELECT * FROM school_points WHERE season = :season AND sport = :sport',
		array(':season' => variable_get('pickem.current_season', 2013), ':sport' => 'ncaaf'));

	$records = array();
	foreach ($records_res as $entry) {
		if (!isset($records[$entry->school_name])) {
			$records[$entry->school_name] = array();
		}
		if (!isset($records[$entry->school_name][$entry->tag])) {
			$records[$entry->school_name][$entry->tag] = $entry;
		}
	}

	$points = array();
	foreach ($points_res as $entry) {
		if (!isset($points[$entry->school_name])) {
			$points[$entry->school_name] = array();
		}
		if (!isset($points[$entry->school_name][$entry->tag])) {
			$points[$entry->school_name][$entry->tag] = $entry;
		}
	}
	
	$q = db_select('game', 'g');
	$q->fields('g');
	$q->join('school', 'host', 'g.host_school_name = host.name');
	$q->join('school', 'visiting', 'g.visiting_school_name = visiting.name');
	$q->leftJoin('sportspoll_tally', 'host_rank', 'host_rank.school_name = g.host_school_name AND g.week = host_rank.week');
	$q->leftJoin('sportspoll_tally', 'visiting_rank', 'visiting_rank.school_name = g.visiting_school_name and g.week = visiting_rank.week');
	$q->addField('host_rank', 'rank', 'host_school_rank');
	$q->addField('visiting_rank', 'rank', 'visiting_school_rank');
	$q->addField('host', 'sid', 'host_sid');
	$q->addField('host', 'conf_division', 'host_conf_division');
	$q->addField('visiting', 'sid', 'visiting_sid');
	$q->addField('visiting', 'conf_division', 'visiting_conf_division');
	$q->condition('g.sport', 'ncaaf');
	$q->condition('g.season', variable_get('pickem.current_season', 2013));
	$q->condition('g.completed', 'Y');
	//$q->condition('g.stats_processed', 0);
	//$q->condition('g.week', array(1,2));
	$res = $q->execute();
	
	foreach ($res as $game) {
			
		$host = array(
			'sid' => $game->host_sid,
			'name' => $game->host_school_name,
			'conf' => $game->host_conf_name,
			'division' => $game->host_conf_division,
			'rank' => $game->host_school_rank,
		);
		$visitor = array(
			'sid' => $game->visiting_sid, 
			'name' => $game->visiting_school_name, 
			'conf' => $game->visiting_conf_name, 
			'division' => $game->visiting_conf_division,
			'rank' => $game->visiting_school_rank,
		);
		
		$home_tags = array('overall', 'conf', 'div', 'home', 'conf_home', 'ooc', 'ooc_home', 'top25', 'top10', 'neutral');
		$away_tags = array('overall', 'conf', 'div', 'away', 'conf_away', 'ooc', 'ooc_away', 'top25', 'top10', 'neutral');
		
		pickem_process_records($home_tags, $records, (object) $host, (object) $visitor, $game->host_score, $game->visiting_score, $game->neutral == 'Y');
		pickem_process_records($away_tags, $records, (object) $visitor, (object) $host, $game->visiting_score, $game->host_score, $game->neutral == 'Y');
		
		$home_tags = array('total', 'conf', 'div', 'home', 'conf_home', 'ooc', 'ooc_home', 'top25', 'top10', 'neutral');
		$away_tags = array('total', 'conf', 'div', 'away', 'conf_away', 'ooc', 'ooc_away', 'top25', 'top10', 'neutral');
					
		pickem_process_points($home_tags, $points, (object) $host, (object) $visitor, $game->host_score, $game->visiting_score, $game->neutral == 'Y');
		pickem_process_points($away_tags, $points, (object) $visitor, (object) $host, $game->visiting_score, $game->host_score, $game->neutral == 'Y');

		/*
		db_update('game')->fields(
			array(
				'stats_processed' => 1,
			)
		)
		->condition('gid', $game->gid)
		->execute();
		 */ 
	}
	
	drush_print("processing records...");
	$handle = fopen(drupal_get_path('module', 'pickem') . '/data/records.tsv', 'w');
	foreach ($records as $school_name => $entries) {
		drush_print("--- processing " . $school_name);
		foreach ($entries as $tag => $entry) {
			$row = array($entry->sid, $entry->school_name, 'ncaaf', $entry->season, $entry->tag, $entry->wins, $entry->losses, $entry->ties);
			fputcsv($handle, $row, "\t");

			if ($entry->tag == 'overall') {
				$pf = $points[$entry->school_name]['total']->pf;
				$pa = $points[$entry->school_name]['total']->pa;
				// compute pythagorean adjustment
				$pythag = (float) ( pow($pf, 1.82) / (pow($pf, 1.82) + pow($pa, 1.82)) );
				$pythag_wins = (int) round($entry->wins * $pythag);
				$pythag_losses = ($entry->wins + $entry->losses) - $pythag_wins;
				$row = array($entry->sid, $entry->school_name, 'ncaaf', $entry->season, 'pythag', $pythag_wins, $pythag_losses, 0);
				fputcsv($handle, $row, "\t");
				
				drush_print('wins = ' . $entry->wins . '; pythag = ' . $pythag . '; adj wins = ' . $pythag_wins);
			}

			/*
			db_merge('school_records')
				->key(array('sid' => $entry->sid, 'sport' => $entry->sport, 'season' => $entry->season, 'tag' => $entry->tag))
				->fields(array(
					'sid' => $entry->sid,
					'school_name' => $entry->school_name,
					'sport' => 'ncaaf',
					'season' => $entry->season,
					'tag' => $entry->tag,
					'wins' => $entry->wins,
					'losses' => $entry->losses,
					'ties' => $entry->ties
			))->execute();
			*/
		}
	}
	fclose($handle);
	
	drush_print("processing points...");
	$handle = fopen(drupal_get_path('module', 'pickem') . '/data/points.tsv', 'w');
	foreach ($points as $school_name => $entries) {
		drush_print("--- processing " . $school_name);
		foreach ($entries as $tag => $entry) {
			$row = array($entry->sid, $entry->school_name, 'ncaaf', $entry->season, $entry->tag, $entry->pf, $entry->pa, $entry->gp, $entry->ppg, $entry->ppa);
			fputcsv($handle, $row, "\t");
		}
	}
	fclose($handle);
}

function drush_pickem_load_stats() {
	db_delete('school_points')->execute();
	db_delete('school_records')->execute();
	
	$base = drupal_get_path('module', 'pickem');
	$points_file = $base . '/data/points.tsv';
	$records_file = $base . '/data/records.tsv';
	
	$handle = fopen($records_file, 'r');
	$last_school = NULL;
	while ($row = fgetcsv($handle, 8192, "\t")) {
		if ($row[1] != $last_school) {
			drush_print('processing ' . $row[1] . ':');
			$last_school = $row[1];
		}
		drush_print('-- ' . $row[4]);
		db_insert('school_records')->fields(array(
			'sid' => $row[0],
			'school_name' => $row[1],
			'sport' => $row[2],
			'season' => $row[3],
			'tag' => $row[4],
			'wins' => $row[5],
			'losses' => $row[6],
			'ties' => $row[7],
		))->execute();
	}
	fclose($handle);
	
	$handle = fopen($points_file, 'r');
	$last_school = NULL;
	while ($row = fgetcsv($handle, 8192, "\t")) {
		if ($row[1] != $last_school) {
			drush_print('processing ' . $row[1] . ':');
			$last_school = $row[1];
		}
		drush_print('-- ' . $row[4]);
		db_insert('school_points')->fields(array(
			'sid' => $row[0],
			'school_name' => $row[1],
			'sport' => $row[2],
			'season' => $row[3],
			'tag' => $row[4],
			'pf' => $row[5],
			'pa' => $row[6],
			'gp' => $row[7],
			'ppg' => $row[8],
			'ppa' => $row[9],
		))->execute();
	}
	fclose($handle);
}

function _init_school_standings_obj(&$o, $season, $sport) {
	$o->pf = 0;
	$o->pa = 0;
	$o->wins = 0;
	$o->losses = 0;
	$o->ties = 0;
	$o->conf_wins = 0;
	$o->conf_losses = 0;
	$o->conf_ties = 0;
	$o->div_wins = 0;
	$o->div_losses = 0;
	$o->div_ties = 0;
	$o->conf_pf = 0;
	$o->conf_pa = 0;
	$o->div_pf = 0;
	$o->div_pa = 0;
	$o->wins_top25 = 0;
	$o->losses_top25 = 0;
	$o->wins_top10 = 0;
	$o->losses_top10 = 0;
	$o->is_new = TRUE;
	$o->season = $season;
	$o->sport = $sport;
}

function drush_pickem_set_game_times() {
	$file = drupal_get_path('module', 'pickem') . '/data/gtimes.tsv';
	if (($handle = fopen($file, 'r')) !== FASLE) {
		while ($row = fgetcsv($handle, 1024, "\t")) {
			if (is_numeric($row[0])) {
				$gid = $row[0];
				$date = new DateTime($row[1]);
				$query = db_update('game')
					->fields(array(
						'game_date' => $date->getTimestamp(),
					))
					->condition('gid', $gid);
				$query->execute();
			} else {
				// its actually easier to type both schools
				$visitor = $row[0];
				$host = $row[1];
				$date = new DateTime($row[2]);
				$query = db_update('game')
					->fields(array(
						'game_date' => $date->getTimestamp(),
					))
					->condition('visiting_school_name', $visitor)
					->condition('host_school_name', $host);
				$query->execute();
			}
			//$gid = $row[0];
		}
		fclose($handle);
	}
}

function drush_pickem_export_schools() {
	$result = db_select('conference', 'c')
						->fields('c')
						->execute();
	$conference = array();
	foreach ($result as $c) {
		$conference[$c->name] = $c;
	}

	$result = db_select('school', 's')
						->fields('s')
						->orderBy('s.conf_name')
						->orderBy('s.name')
						->execute();
	$football_schools = array();
	$mbb_schools = array();

	foreach($result as $s) {
		$school = array(
			$s->name,
			$s->title,
			$s->displaytitle,
			$s->nickname,
			'football',
			$s->conf_name,
			(!empty($s->conf_division)) ? strtolower($s->conf_division) : '',
			'ncaa',
		);
		if (in_array($s->conf_name, array('acc','sec','big10','big12','pac12','sunbelt','mwc','mac','ind','cusa','aac'))) {
			$school[] = 'div1-fbs';
		} else {
			$school[] = 'div1-fcs';
		}
		$school[] = $s->active;
		$football_schools[] = $school;

		if (in_array($s->conf_name, array('acc','sec','big10','big12','pac12','sunbelt','mwc','mac','cusa','aac'))) {
			$school[4] = 'mbb';
			$school[8] = 'div1';
			$mbb_schools[] = $school;
		}
	}
	
	$handle = fopen(drupal_get_path('module', 'pickem') . '/data/teams.tsv', 'w');
	foreach ($football_schools as $school) {
		drush_print("--- processing " . $school[1] . ' for ' . $school[4]);
		fputcsv($handle, $school, "\t");
	}
	foreach ($mbb_schools as $school) {
		drush_print("--- processing " . $school[1] . ' for ' . $school[4]);
		fputcsv($handle, $school, "\t");
	}
	fclose($handle);
}

function drush_pickem_set_game_scores() {
	//db_set_active('sandbox');
	
	$season = variable_get('pickem.current_season');
	$file = drupal_get_path('module', 'pickem') . '/data/gscores.tsv';
	if (($handle = fopen($file, 'r')) !== FASLE) {
		while ($row = fgetcsv($handle, 1024, "\t")) {
			$visitor = $row[0];
			$host = $row[2];
			$result = db_query("SELECT gid FROM {game} 
													 WHERE host_school_name = :host_school_name 
													 AND visiting_school_name = :visiting_school_name
													 AND season = :season
													 AND completed = 'N'", 
													 array(':host_school_name' => $host, ':visiting_school_name' => $visitor, ':season' => $season));
			$game = $result->fetchObject();
			if ($game != NULL) {
				// update the score of the game.
				$fields = array(
					'visiting_score' => $row[1],
					'host_score' => $row[3],
					'completed' => 'Y',
				);
				if (isset($row[4])) {
					$fields['overtimes'] = $row[4];
				}
				$query = db_update('game')
					->fields($fields)
					->condition('gid', $game->gid);
				$query->execute();
			} else {
				$result = db_query("SELECT gid FROM {game} 
														 WHERE host_school_name = :host_school_name 
														 AND visiting_school_name = :visiting_school_name
														 AND season = :season", 
														 array(':host_school_name' => $host, ':visiting_school_name' => $visitor, ':season' => $season));
				$existing_game = $result->fetchObject();
				if ($existing_game != NULL) {
					drush_print('Score already saved for ' . $row[0] . ' vs. ' . $row[2]);
				} else {
					drush_print('No game record for ' . $row[0] . ' vs. ' . $row[2]);
				}
			}
		}
		fclose($handle);
	}
	//db_set_active();
}

function drush_pickem_load_conf_games($conf) {
	require_once(drupal_get_path('module', 'pickem') . '/pickem.install');
	$filename = $conf.'games.tsv';
	
	$file = drupal_get_path('module', 'pickem') . '/data/' . $filename;
	if (($handle = fopen($file, 'r')) !== FALSE) {
		while (($data = fgetcsv($handle, 4096, "\t"))) {
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
			drush_print($msg);
			$tmp_school[$o->name] = $o;
		}
		fclose($handle);
	}
}

function drush_pickem_load_lines() {
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

function drush_pickem_import_fb_poll($week, $season) {
	if ($season == '') {
		$season = variable_get('pickem.current_season');
	}
	
	$file = drupal_get_path('module', 'pickem') . '/data/appoll-week-' . $week . '.tsv';
	if (($handle = fopen($file, 'r')) !== FALSE) {
		while (($data = fgetcsv($handle, 500, "\t"))) {
			if ($data[0] == 'sid') {
				continue;
			}
			drush_print('processing ' . $data[3] . '....');
			db_insert('sportspoll_tally')
				->fields(array(
					'season' => $data[0],
					'week' => $data[1],
					'pollid' => 1,
					'rank' => $data[4],
					'points' => $data[5],
					'school_name' => $data[3],
				)
			)->execute();
		}
		fclose($handle);
	}	
	
}

function drush_pickem_reset_players() {
	require_once(drupal_get_path('module', 'pickem') . '/pickem.model.inc');
	
	// select all the players who have made picks
	$subq = db_select('pickem_player', 'pp');
	$subq->addField('pp', 'uid');
	
	$q = db_select('pick', 'p');
	$q->addExpression('DISTINCT p.uid', 'player_uid');
	$q->condition('p.uid', $subq, 'NOT IN');
	$res = $q->execute();
	foreach ($res as $rec) {
		db_insert('pickem_player')
			->fields(array(
				'uid' => $rec->player_uid,
				'games_picked' => 0,
				'player_level' => 'active-full',
				'weeks_won' => 0,
			))
		->execute();
	}
	
	/*
	$pick_data = pickem_load_pick_data(-1, 2013);
	foreach ($pick_data['player'] as $player => $data) {
		db_update('pickem_player')->fields(
			array(
				'games_picked' => $data['num_picks'],
			)
		)->condition('uid', $data['uid'])
		->execute();
	}
	*/
	
	$q = db_select('pickem_player', 'pp');
	$q->join('users', 'u', 'u.uid = pp.uid');
	$q->addField('u', 'name', 'player_name');
	$q->fields('pp');
	$res = $q->execute();
	$players = array();
	foreach ($res as $r) {
		$players[$r->player_name] = $r;
	}

	$pick_data = pickem_load_pick_data(2, 2013);
	//drush_print_r($pick_data['game']);

	foreach ($pick_data['game'] as $gid => $data) {
		// combine the 'who' arrays
		$who1 = $data[$data['host_school_name']]['who'];
		$who2 = $data[$data['visiting_school_name']]['who'];
		$who = array_merge($who1, $who2);
		foreach ($players as $player_name => $p) {
			if (!in_array($player_name, $who)) {
				drush_print($player_name . '(' . $p->uid . ') does not have a pick for game #' . $gid . '; -- ' . $data['host_school_name'] . ' | ' . $data['visiting_school_name']);
			}
		}
	}
}

function permute_standings() {
	
}

$DEBUG = FALSE;
$games_of_interest = array();

function drush_pickem_project_acc() {
	global $games_of_interest;
	gc_enable();
	
	$target_conf = 'acc';
	
	db_delete('outcome_scenario')
		->condition('conf_name', 'acc')
		->condition('conf_division', 'Coastal')
		->execute();
		
	db_delete('sqlite_sequence')
		->condition('name', 'outcome_scenario')
		->execute();
	
	#gid	week	visiting_school_name	host_school_name
	$result = db_select('school', 's')
		->fields('s', array('name', 'conf_division'))
		->condition('conf_name', $target_conf)
		->execute();
	
	$conference_divisions = array();
	foreach ($result as $row) {
		$conference_divisions[$row->name] = $row->conf_division;
	}
	
	$accdivs = array('bostoncollege'=>'Atlantic','clemson'=>'Atlantic','fsu'=>'Atlantic','maryland'=>'Atlantic','ncstate'=>'Atlantic',
		'syracuse'=>'Atlantic','wakeforest'=>'Atlantic','duke'=>'Coastal','georgiatech'=>'Coastal','miami'=>'Coastal','northcarolina'=>'Coastal',
		'pitt'=>'Coastal','virginia'=>'Coastal','vatech'=>'Coastal',);
	
	$standings = array();
		
	$file = drupal_get_path('module', 'pickem') . '/data/accproj.tsv';
	if (($handle = fopen($file, 'r')) !== FASLE) {
		$index = 1;
		while ($row = fgetcsv($handle, 1024, "\t")) {
			if ($row[0] == 'gid') { continue; }
			$data[$index] = array(
				'gid' => $row[0],
				'week' => $row[1],
				'visiting_school_name' => $row[2],
				'host_school_name' => $row[3],
				'visiting_score' => 0,
				'host_score' => 0,
			);
			
			$data[$index]['same_conf'] = array_key_exists($row[2], $accdivs) && array_key_exists($row[3], $accdivs);
			$data[$index]['same_div'] = $data[$index]['same_conf'] && ($accdivs[$row[2]] == $accdivs[$row[3]]);
			 
			$index = $index * 2;
		}
		fclose($handle);
	}

	$file = drupal_get_path('module', 'pickem') . '/data/accrecords.tsv';
	if (($handle = fopen($file, 'r')) !== FASLE) {
		while ($row = fgetcsv($handle, 1024, "\t")) {
			if ($row[0] == 'sid') { continue; }
			#sid	school_name	season	sport	wins	losses	ties	tag	conf_division
			#1	bostoncollege	2013	ncaaf	2	3	0	conf	Atlantic

			if (!isset($standings[$row[1]])) {
				$standings[$row[1]] = array('cl' => array(),'cw' => array());
				$standings[$row[1]]['d'] = $accdivs[$row[1]];
			}
			$standings[$row[1]][$row[7]] = array('w' => $row[4], 'l' => $row[5]);
		}
		fclose($handle);
	}
	unset($handle);
	
	$result = db_query("SELECT visiting_school_name, visiting_score, host_school_name, host_score FROM {game} 
										  WHERE host_conf_name = 'acc' AND visiting_conf_name = 'acc' AND completed = 'Y'");
	foreach ($result as $game) {
		$game = (array) $game;
		if ($game['visiting_score'] > $game['host_score']) {
			$standings[$game['visiting_school_name']]['cw'][] = $game['host_school_name'];
			$standings[$game['host_school_name']]['cl'][] = $game['visiting_school_name'];
		} else {
			$standings[$game['visiting_school_name']]['cl'][] = $game['host_school_name'];
			$standings[$game['host_school_name']]['cw'][] = $game['visiting_school_name'];
		}
		unset($game);
	}
	unset($result);
	unset($game);
	
	$the_big_number = pow(2, count($data)-1);
	$stats = array(
		'more-info-needed' => 0, 
		'more-info-desc' => array(),
		'valid_scenes' => array(),
	);
	$valid_run = 0;
	$mmesg = '';
	
	// array arranged such that the bit mapping looks like
	// 00 0000 0000
	#$games_of_interest = array(40, 56, 72, 80, 96, 111);
	$result = db_select('game', 'g')
		->fields('g')
		->condition('gid', array(40, 72, 80, 96))
		->orderBy('game_date')
		->execute();
	
	foreach($result as $ndx => $game) {
		$game->position = $ndx;
		$games_of_interest[$game->gid] = $game;
	}
		
	
	for ($i = 0; $i <= $the_big_number; $i++) {
		
		$data_proj = $data;
		$standings_project = array_merge($standings);
		$standings_project['results_of_interest'] = 0;
		$standings_project['champ'] = '';
		$standings_project['runner_up'] = '';
		
		//$game_results = "";
		$use_this_scene = (($i | 180) >= $i) || (($i | 180) >= 180);
				
		foreach ($data_proj as $bit => $game) {
			$winning_key = '';
			$losing_key = '';
			
			if (($i & $bit) == $bit) {
				$game['visiting_score'] = 0;
				$game['host_score'] = 1;
				$winning_key = 'host_school_name';
				$losing_key = 'visiting_school_name';
				
			} else {
				$game['visiting_score'] = 1;
				$game['host_score'] = 0;
				$winning_key = 'visiting_school_name';
				$losing_key = 'host_school_name';
			}
			
			#$game_of_interest = array_search($game['gid'], $games_of_interest);
			$game_of_interest = array_key_exists($game['gid'], $games_of_interest) ? $games_of_interest[$game['gid']] : NULL;
			if ($game_of_interest && $use_this_scene) {

				#$bit = pow(2, $game_of_interest) * $game['host_score'];
				$bit = pow(2, $game_of_interest->position) * $game['host_score'];
				
				$standings_project['results_of_interest'] = $standings_project['results_of_interest'] | $bit;
			}

			if (array_key_exists($game[$losing_key], $standings_project)) {
				$standings_project[$game[$losing_key]]['overall']['l']++;
			}
			if (array_key_exists($game[$winning_key], $standings_project)) {
				$standings_project[$game[$winning_key]]['overall']['w']++;
			}
			if ($game['same_conf']) {
				// record (potential) head-to-head results
				$standings_project[$game[$winning_key]]['cw'][] = $game[$losing_key];
				$standings_project[$game[$losing_key]]['cl'][] = $game[$winning_key];

				$standings_project[$game[$losing_key]]['conf']['l']++;
				$standings_project[$game[$winning_key]]['conf']['w']++;
				if ($game['same_div']) {
					$standings_project[$game[$losing_key]]['div']['l']++;
					$standings_project[$game[$winning_key]]['div']['w']++;
				}
			}
			unset($losing_key);
			unset($winning_key);
			unset($game);
		}
		
		// only process standings for a subset of games
		// based on a bit mask (979270)
		
		$scene_key = decbin($standings_project['results_of_interest']);
		if ($use_this_scene && !key_exists($scene_key, $stats['valid_scenes'])) {
			$valid_run++;
			drush_print(sprintf("run #%s", $i));
			process_standings($standings_project, $accdivs, $stats);

			drush_print("****** " . memory_get_usage());
			drush_print("\n");
			unset($game_results);
			// drush_print_r($stats);
		}

		unset($data_proj);
		unset($standings_project);
		
	}

	gc_disable();
	drush_print_r($stats);
	drush_print(sprintf("Number of valid scenes: %d", count($stats['valid_scenes'])));
	$valid_scene_count = count($stats['valid_scenes']);
	
	$eliminated = array();
	foreach ($stats as $key => $data) {
		if (in_array($key, array('more-info-needed', 'more-info-desc', 'valid_scenes'))) { continue; }
		if ((int)$data['champ'] == 0) {
			$eliminated[] = $key;
		}
	}
	
	print_team_reports($stats, $valid_scene_count, $eliminated);
	
	$mmesg = 	"Number of Permuations: " . $the_big_number . "\n\n";
	$mmesg .= sprintf("Number of valid scenarios: %s\n\n", $valid_run);
	$mmesg .= print_r($stats, TRUE) . "\n";
	
	mail('godecro@gmail.com', 'ACC Projection Stats',$mmesg, 'From: derrek@decro.net' . "\r\n");
	unset($mmesg);
}

function print_team_reports($stats, $scene_count, $eliminated) {
	$rows = array();
	$rows[] = array('', 'Champion Scenarios', '% Champion Scenarios', '# Outright', '# Two-Way Ties', 'Two-Way Ties With...', 'Multi-Team Ties');
	foreach ($stats as $schn => $data) {
		if (in_array($schn, array('more-info-needed', 'more-info-desc', 'valid_scenes'))) { continue; }
		if (in_array($schn, $eliminated)) { continue; }
		$row = array();
		$row[] = $schn;
		$row[] = $data['champ'];
		$row[] = ($data['champ'] / $scene_count) * 100.0;
		$row[] = $data['alone-in-first'];
		$row[] = $data['head-to-head'];
		$runner_ups = array();
		foreach ($data['runner-up'] as $ru => $num) {
			$runner_ups[] = sprintf("%s - %d", $ru, $num);
		}
		$row[] = join('; ', $runner_ups);
		$row[] = $data['champ'] - ($data['head-to-head'] + $data['alone-in-first']);
		$rows[] = $row;
	}
	$header = array_shift($rows);
	drush_print(sprintf("%-35s | %10s | %10s | %10s | %10s | %-50s | %12s", 
		'school', '# champ', '% champ', 'outright', '2-way tie', 'tied with', '3+ way tie'));
	foreach ($rows as $row) {
		drush_print(sprintf("%-35s | %10s | %10s | %10s | %10s | %-50s | %12s", 
			$row[0], $row[1], $row[2], $row[3], $row[4], $row[5], $row[6]));
	}
}

function get_scenario_text($binstr, $delim="\n") {
	global $games_of_interest;
	
	$bit = 1;
	$text = array();
	$converted = bindec($binstr);

	foreach($games_of_interest as $gid => $game) {
		$text[] = ($converted & $bit) == $bit 
			? sprintf("%s over %s", $game->host_school_name, $game->visiting_school_name)
			: sprintf("%s over %s", $game->visiting_school_name, $game->host_school_name);
			
		$bit = $bit * 2;
	}
	return join($delim, $text);
	// array arranged such that the bit mapping looks like
	// 00 0000 0000

	/*
	 * 0 - georgiatech/clemson
	 * 0 - miami/duke
	 * 0 - georgia/georgiatech
	 * 0 - virginia/miami
	 * 0 - duke/northcarolina
	 * 0 - northcarolina/pitt
	 * 0 - miami/pitt
	 * 0 - pitt/syracuse
	 * 0 - vatech/virginia
	 * 0 - maryland/vatech
	 * 0 - duke/vatech
	 */
	#$text[] = ($converted & 1) == 1 ? 'georgiatech over georgia' : 'georgia over georgiatech';
	#$text[] = ($converted & 2) == 2 ? 'miami over virginia' : 'virginia over miami';
	#$text[] = ($converted & 4) == 4 ? 'northcarolina over duke' : 'duke over northcarolina';
	#$text[] = ($converted & 8) == 8 ? 'pitt over miami' : 'miami over pitt';
	#$text[] = ($converted & 16) == 16 ? 'virginia over vatech' : 'vatech over virginia';
	#$text[] = ($converted & 32) == 32 ? 'wakeforest over duke' : 'duke over wakeforest';
	#return join($delim, $text);
}

function process_standings(&$s, &$accdivs, &$stats) {
	static $mailcount = 0;
	$a = array();
	foreach ($s as $sn => $sch) {
		if ($accdivs[$sn] == 'Coastal') {
			if (!isset($stats[$sn])) {
				$stats[$sn] = array(
					'champ' => 0,
					'runner-up' => array(),
					'alone-in-first' => 0,
					'head-to-head' => 0,
					'combined-head-to-head' => 0,
					'overall-winning-pct' => 0,
					'2-team-tie' => 0,
					'3-team-tie' => 0,
					'4-team-tie' => 0,
					'5-team-tie' => 0,
					'scenes' => array(),
					'scene_count' => 0,
				);
			}
			$key = ($sch['conf']['w'] / ($sch['conf']['w'] + $sch['conf']['l'])) * 100;
			
			if (isset($a[$key])) {
				if (is_string($a[$key])) {
					$a[$key] = (array) $a[$key];
				}
				$a[$key][] = $sn . ' | ' . $sch['conf']['w'] . ' | ' . $sch['conf']['l'] . ' | ' . $sch['overall']['w'] . ' | ' . $sch['overall']['l'];
			} else {
				$a[$key] = $sn . ' | ' . $sch['conf']['w'] . ' | ' . $sch['conf']['l'] . ' | ' . $sch['overall']['w'] . ' | ' . $sch['overall']['l'];
			}
		}
	}
	
	krsort($a);	
	$top_el = array_shift($a);
		
	unset($a);
	unset($k);
	$champ = '';
	$why = array();
	
	$teams_in_first = count($top_el);
	$teams = '';
	if (count($top_el) > 1) {
		foreach ($top_el as $a) {
			list($schn) = explode(' | ', $a);
			$teams .= empty($teams) ? $schn : ', ' . $schn;
			
			$k = $teams_in_first . '-team-tie';
			$stats[$schn][$k]++;
		}
	}

	determine_champion(1, $top_el, $s, $champ, $why);
	
	if (isset($s['champ'])) {
		drush_print(sprintf("CHAMPION: %s | %s | %s | HOW?: %s | Results of Interest: %010b\n",
			$s['champ'], $teams_in_first, $teams, join(',', $why), $s['results_of_interest']
		));
		$k = decbin($s['results_of_interest']);
		if (!isset($stats['valid_scenes'][$k])) {
			//$stats['valid_scenes'][$k] = array('c' => 0, 'ch' => array());
			$stats['valid_scenes'][$k] = array(
				'c' => 0, 
				'ch' => array(), 
				'scene_text' => get_scenario_text($k), 
				'how' => join(',', $why),
				'teams' => empty($teams) ? $s['champ'] : $teams,
				'teams_in_first' => $teams_in_first,
			);
			
		}
		$stats['valid_scenes'][$k]['c']++;
		if (!in_array($s['champ'], $stats['valid_scenes'][$k]['ch'])) {
			$stats['valid_scenes'][$k]['ch'][] = $s['champ'];
		}
		db_insert('outcome_scenario')
			->fields(array(
				'scenario' => dechex($k),
				'winner' => $s['champ'],
				'how' => join(',', $why),
				'teams_in_first' => empty($teams) ? $s['champ'] : $teams,
				'scene_text' => get_scenario_text($k, ','),
				'conf_name' => 'acc',
				'conf_division' => 'Coastal',
			))
			->execute();
		
		$stats[$s['champ']]['champ']++;
		$scene_hex = dechex($s['results_of_interest']);
		if (!in_array($scene_hex, $stats[$s['champ']]['scenes'])) {
			$stats[$s['champ']]['scenes'][] = $scene_hex;
			$stats[$s['champ']]['scene_count']++;
		}
		
		$combined_why = join(', ', $why);
		if (!isset($stats[$s['champ']][$combined_why])) {
			$stats[$s['champ']][$combined_why] = 0;
		}
		$stats[$s['champ']][$combined_why]++;
		unset($combined_why);
		
		if ($s['runner_up']) {
			if (!isset($stats[$s['champ']]['runner-up'][$s['runner_up']])) {
				$stats[$s['champ']]['runner-up'][$s['runner_up']] = 0;
			}
			$stats[$s['champ']]['runner-up'][$s['runner_up']]++;
		}
		unset($k);
	} else {
		$stats['more-info-needed']++;
		$key = sprintf("%s | %s", $teams_in_first, $teams);
		if (!isset($stats['more-info-desc'][$key])) {
			$stats['more-info-desc'][$key] = 0;
		}
		$stats['more-info-desc'][$key]++;
		drush_print('- Need additional tiebreaker(s) to determine champ');
		drush_print('--------------------------------------------------');
		drush_print_r($top_el);
		drush_print('');
		
		if ($mailcount <= 5) {
			$mmesg = print_r($top_el, TRUE);
			mail('godecro@gmail.com', 'ACC Projection Stats (More Tiebreakers Needed)', $mmesg, 'From: derrek@decro.net' . "\r\n");
			$mailcount++;
		}
	}
	unset($teams);
	unset($teams_in_first);
	unset($why);
	unset($champ);
	#drush_print('*** CHAMPION is: ' . $s['champ'] . ' (' . join(',', $why) . ') ***');
}

function determine_champion($attempt, $first_place, &$s, &$champ, &$why) {
	if (!empty($champ)) {
		$s['champ'] = $champ;
		return;
	}
	if ($attempt > 1) {
		drush_print("attempt #" . $attempt . " to determine champion...");
		drush_print_r($first_place);
	}
	if (is_string($first_place)) {
		list($schname, $wins, $losses) = explode(' | ', $first_place);
		$s['champ'] = $schname;
		$champ = $schname;
		$why[] = 'alone-in-first';
		#drush_print('*** CHAMPION is: ' . $champ . ' ***');
		unset($schname);
		unset($wins);
		unset($losses);
		return;
	}

	$original_first_place = $first_place;
	if (count($first_place) == 2) {
		// head to head
		$a = explode(' | ', array_shift($first_place));
		$b = explode(' | ', array_shift($first_place));
		if (!is_array($s[$a[0]]['cw'])) {
			//drush_print_r($first_place);
			//drush_print($a[0]);
			//drush_print_r($s[$a[0]]);
			//exit;
		}
		$champ = in_array($b[0], $s[$a[0]]['cw']) ? $a[0] : $b[0];
		$runner_up = in_array($b[0], $s[$a[0]]['cw']) ? $b[0] : $a[0];
		$s['champ'] = $champ;
		
		// don't record the runner up if this is a scenario where 
		// a three (or more)-way tie was reduced to a head-to-head
		if (empty($why)) {
			$s['runner_up'] = $runner_up;
		}
		$why[] = 'head-to-head';
		#drush_print('*** CHAMPION is: ' . $champ . ' (head-to-head) ***');

		unset($a);
		unset($b);
		unset($runner_up);
		return;
	} else {
		combined_head_to_head($first_place, $s, $why) ||
		overall_winning_pct($first_place, $s, $why);
		
		if (empty($s['champ'])) {
			// continue processing 
			determine_champion($attempt++, $first_place, $s, $champ, $why);
		} else {
			list($sn) = explode(' | ', array_shift($first_place));
			$champ = $sn;
			unset($sn);
			return;			
		}
	}
}

function combined_head_to_head(&$first_place, &$s, &$why) {
	// assuming three or more entries in 'first_place'
	// drush_print("\n!! in combined_head_to_head");
	$count = count($first_place);
	$tally = '';
	$hi_tally = 0;
	for ($i = 0; $i < $count; $i++) {
		// copy the first_place array
		$cp = array_merge($first_place);
		
		list($schn, $w, $l) = explode(' | ', array_shift(array_splice($cp, $i, 1)));
		//drush_print("examining [" . $schn . "]");
		//drush_print_r($cp);
		
		// we have separated the school from the others
		$wins = 0;
		foreach($cp as $opp) {
			$opp = explode(' | ', $opp);
			//drush_print("%%%%% " . $schn . ' vs ' . $opp[0]);
			$wins += in_array($opp[0], $s[$schn]['cw']) ? 1 : 0;
			
		}
		// record the tally
		$tally .= $wins;
		$hi_tally = ($wins > $hi_tally) ? $wins : $hi_tally;
		
		unset($cp);
	}
	//drush_print("-------------- tally = [" . $tally . "]");
	//drush_print("-------------- hi_tally = [" . $hi_tally . "]");
	$tally = str_split($tally);
	for ($i = $count-1; $i >= 0; $i--) {
		if ($tally[$i] < (int)$hi_tally) {
			//drush_print("---| removed (" . $first_place[$i] . ") from consideration in combined head-to-head");
			unset($first_place[$i]);
		}
		if (empty($first_place)) {
			break;
		}
	}
	unset($hi_tally);
	unset($tally);
	unset($count);
	
	if (count($first_place) == 1) {
		list($schn, $w, $l) = explode(' | ', array_shift($first_place));
		$s['champ'] = $schn;
		$why[] = "combined-head-to-head";
		unset($schn);
		unset($w);
		unset($l);
		return TRUE; 
	} else if (count($first_place) == 2) {
		// return TRUE so we can apply the head-to-head test
		$why[] = 'combined-head-to-head';
		#drush_print("### reduced to two schools...");
		#drush_print_r($first_place);
		$first_place = array_values($first_place);
		return TRUE;
	}
	$first_place = array_values($first_place);
	return FALSE;
}

function overall_winning_pct(&$first_place, &$s, &$why) {
	//drush_print("!! in overall_winning_pct");
	//drush_print_r($first_place);
	$count = count($first_place);
	$hi_tally = 0;
	$tally = '';
	foreach ($first_place as $sch) {
		list($sch) = explode(' | ', $sch);
		$p = ($s[$sch]['overall']['w'] / ($s[$sch]['overall']['w'] + $s[$sch]['overall']['l'])) * 100;
		$tally .= empty($tally) ? dechex($p) : '|' . dechex($p);
		$hi_tally = ($p > $hi_tally) ? (int) $p : $hi_tally;
	}
	//drush_print("-------------- tally = [" . $tally . "]");
	//drush_print("-------------- hi_tally = [" . $hi_tally . "]");
	
	$tally = explode('|', $tally);
	for ($i = $count-1; $i >= 0; $i--) {
		$this_tally = (int) hexdec($tally[$i]);
		//drush_print("this tally =[" . $this_tally . "]");
		if ($this_tally < $hi_tally) {
			//drush_print("---| removed (" . $first_place[$i] . ") from consideration in overall winning pct");
			unset($first_place[$i]);
		}
		if (empty($first_place)) {
			break;
		}
	}
	unset($chars_to_capture);
	unset($count);
	unset($tally);
	unset($hi_tally);
	
	if (count($first_place) == 1) {
		list($schn) = explode(' | ', array_shift($first_place));
		$s['champ'] = $schn;
		$why[] = 'overall-winning-pct';
		unset($schn);
		return TRUE;
	}
	$first_place = array_values($first_place);
	if (count($first_place) == 2) {
		// return TRUE to apply head-to-head
		$why[] = 'overall-winning-pct';
		return TRUE;
	}
	return FALSE;
}
