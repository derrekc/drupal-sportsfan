<?php
/*
 * Created on Aug 12, 2013
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */


function pickem_school_listing($conf = NULL) {
	drupal_add_library('system', 'drupal.ajax');
	drupal_set_title('ACCbbs Pickem: Schools');

	$query = db_select('school', 's')->extend('PagerDefault')->extend('TableSort');
	$query->join('conference', 'conf', 'conf_name = conf.name');
	$query->fields('s');
	$query->addField('conf', 'displaytitle', 'conf_displaytitle');
	if ($conf != NULL) {
		$query->condition('conf_name', $conf);
	}
	$query->orderBy('displaytitle')
	->limit(25);
	$result = $query->execute();

	$headers = array(
		t('Title'),
		t('Full Title'),
		t('System Name'),
		t('Nickname'),
		t('Conference'),
		t('Division'),
		t('Schedule'),
	);
	
	$rows = array();
	
	while($record = $result->fetchObject()) {
		if (pickem_can_show_schedule_link($record->name, $record->conf_name)) {
			$schedule = l(t('2013 Schedule'), 'schedule/ncaaf/' . $record->name);
		} else {
			$schedule = '';
		}
		$rows[] = array(
			'data' => array(
				$record->displaytitle,
				$record->title,
				$record->name,
				$record->nickname,
				$record->conf_displaytitle,
				$record->conf_division,
				$schedule,
			),
		);
	}
	
	$build['schools'] = array(
		'#theme' => 'table',
		'#header' => $headers,
		'#rows' => $rows,
		'#attributes' => array(
			'class' => array(
				'table', 
				'table-hover',
			)
		),
	);
	$build['pager'] = array(
		'#theme' => 'pager',
	);
	return $build;
}

/** 
 * THIS IS THE SCHEDULE function for a school
 */
function pickem_school_slate($school, $sport = 'ncaaf') {
	//return $school->displaytitle;
	$season = variable_get('pickem.current_season');
	$result = db_query('SELECT week FROM {week} w WHERE season = :season AND week <= 15 ORDER by week',
		array(':season' => $season));

	drupal_set_title(variable_get('pickem.current_season') . ' ' . $school->displaytitle . ' ' . $school->nickname . ' Football Schedule');
		
	if ($school->has_colors) {
		drupal_add_css('/pickem/css/' . $school->sid, array('type' => 'external', 'group' => CSS_THEME, 'weight' => 1000,));	
	}
	
	$header = array(
		t('Wk'),
		t(''),
		t('Opponent'),
		t('Date'),
		t('Kickoff/Result'),
		t('Location'),
		t('In Pickem<br />Slate?'),
	);
	$rows = array();
	while($week = $result->fetchObject()) {
		$game = pickem_game_load_by_school($school->name, $week->week, TRUE);
		//$champ_game = pickem_load_championship_game($school, $week->week, $season, 'ncaaf');
		if ($game) {
			$opp_cell = $game->vs_at;
			if ($game->vs_at != '') {
				$opp_cell .= '&nbsp;';
			}
			if (pickem_can_show_schedule_link($game->opponent_school_name, $game->opponent_conf_name)) {
				$opp_cell .= '<strong>' . l($game->opponent_school, 'schedule/' . $sport . '/' . $game->opponent_school_name) . '</strong>';
			} else {
				$opp_cell .= '<strong>' . $game->opponent_school . '</strong>';
			}
			//$opp_cell .= '<br /><small>' . $game->location . '</small>';
			$logo_cell = '<img src="/' . drupal_get_path('module', 'pickem') 
				. '/logos/ncaaf/' . $game->opponent_school_name . '.gif" width="50" />';
			$slate_cell = $game->slate_id != '' ? '<i class="icon-check" title="Included in Pickem Slate"></i>' : '';
			$rows[] = array(
				'data' => array(
					array('data' => $week->week, 'width' => 50, 'class' => array('text-center')),
					array('data' => $logo_cell, 'width' => 55),
					t($opp_cell),
					$game->game_date_str,
					($game->completed == 'Y') ? $game->result : $game->kickoff_time,
					$game->location,
					$slate_cell,
				),
			);
		} else {
			// attempt to detect a championship game entry
			
			$rows[] = array(
				'data' => array(
					$week->week,
					'',
					array('data' => 'BYE', 'colspan' => 5),
				),
			);
		}
		/*
		$build['week-' . $week->week] = array(
			'#type' => 'markup',
			'#markup' => '<strong>' . $week->week . '</strong><pre>' . print_r($game, TRUE) . '</pre>',
		);
		*/
	}
	$build['slate'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array(
			'class' => array(
				'table', 
				'table-hover',
				//'table-condensed',
				//'table-bordered',
			)
		),
	);
	$build['timezone-info'] = array(
		'#type' => 'markup',
		'#markup' => t('All times ET'),
	);
	
	$standings = pickem_load_conf_standings($school->conf_name, $season, $sport);
	$stats = pickem_load_school_stats($school->name, $season, $sport);
	#dpm($standings);

	$header = array(
		'',
		array('data' => t('Conf'), 'colspan' => 3),
		array('data' => t('Ovr'), 'colspan' => 3),
		t('Next Game'),
	);
	$subheader = array(
		array('data' => t('School'), 'header' => TRUE,),
		array('data' => t('W'), 'header' => TRUE),
		array('data' => t('L'), 'header' => TRUE),
		array('data' => t('%'), 'header' => TRUE),
		array('data' => t('W'), 'header' => TRUE),
		array('data' => t('L'), 'header' => TRUE),
		array('data' => t('%'), 'header' => TRUE),
		array('data' => '', 'header' => TRUE),
	);
	
	if ($standings['has_divisions']) {
		unset($standings['has_divisions']);
		foreach ($standings['divisions'] as $div_name => $items) {
			$rows = array();
			$rows[] = $subheader;
			foreach ($items as $item) {
				$row = array();
				$row[] = $item['school'];
				$row[] = $item['cwins'];
				$row[] = $item['closses'];
				$row[] = number_format($item['cpct'], 3);
				$row[] = $item['wins'];
				$row[] = $item['losses'];
				$row[] = number_format($item['pct'], 3);
				$row[] = '';
				$rows[] = array('data' => $row);
				if ($item['school_name'] == $school->name) {
					$rows[count($rows)-1]['class'] = array('info');
				}
			}
			$build['standings'][$div_name] = array(
				'#theme' => 'table',
				'#header' => $header,
				'#rows' => $rows,
				'#caption' => t($div_name . ' Division'),
				'#attributes' => array(
					'class' => array('table', 'table-condensed'),
				)
			);
		}
	} else {
		unset($standings['has_divisions']);
		$rows = array();
		$rows[] = $subheader;
		foreach ($standings as $item) {
			$row = array();
			$row[] = $item['school'];
			$row[] = $item['cwins'];
			$row[] = $item['closses'];
			$row[] = number_format($item['cpct'], 3);
			$row[] = $item['wins'];
			$row[] = $item['losses'];
			$row[] = number_format($item['pct'], 3);
			$row[] = '';
			$rows[] = array('data' => $row);
			if ($item['school_name'] == $school->name) {
				$rows[count($rows)-1]['class'] = array('info');
			}
			$build['standings'] = array(
				'#theme' => 'table',
				'#header' => $header,
				'#rows' => $rows,
				'#attributes' => array(
					'class' => array('table', 'table-condensed'),
				)
			);
		}
	}
	return $build;
}

function pickem_school_colors_css($school) {
	drupal_add_http_header('Content-Type', 'text/css; utf-8');
	return theme('school_colors_css', array('school' => $school));
}
