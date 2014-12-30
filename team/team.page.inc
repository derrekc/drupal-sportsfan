<?php

function team_sport_schedule($team, $sport, $season = FALSE) {
	global $conf;	
	global $user;
	
	$current_season = $season;
	if (!$season) {
		$current_season = $conf['current_season'][$sport];
	}
	
	$pickems = FALSE;
	if (user_is_logged_in()) {
		$result = db_query(
			"SELECT p.pid, p.title FROM {pickem} p "
			. "WHERE p.pid IN "
			. "(SELECT sl.pid FROM {pickem_slate} sl WHERE event_entity_id in "
			. "(SELECT "
			. "	  se.eid "
			. "   FROM {sportsevent} se "
			. "   WHERE "
			. "   :teamname in (host_team_name, visiting_team_name) AND "
			. "   sport = :sport AND "
			. "   season = :season AND "
			. "   completed = 'Y'"
			. ") AND p.pid IN "
			. "(SELECT pp.pickem_id FROM {pickem_player} pp WHERE name = :playername)"
			. ")",
			array(
				':playername' => $user->name, 
				':sport' => $sport,
				':season' => $season, 
				':teamname' => $team->name
			)
		);
		$objs = $result->fetchAllAssoc('pid');
		$pickems = array_keys($objs);
	}
	
	$conference = conference_load_by_name( $team->conf_name );

	$query = db_select('sportsevent', 's')
		->condition(
			db_or()->condition('visiting_team_name', $team->name)->condition('host_team_name', $team->name)
		)
		->condition('s.season', $current_season)
		->condition('s.sport', $sport)
		->fields('s', array(
			'eid', 
			'event_date', 
			'neutral', 
			'season', 
			'title', 
			'host_team_name', 
			'visiting_team_name', 
			'host_conf_name',
			'visiting_conf_name',
			'location', 
			'venue_id', 
			'completed',
			'host_score',
			'visiting_score',
			'winning_team_name',
			'summary_url',
			)
		);
	$query->addExpression("FROM_UNIXTIME(event_date, '%b %e')", 'event_date_short');
		
	$query->leftJoin('team', 'host_team', "s.host_team_name = host_team.name AND host_team.sport = '" . $sport . "'");
	$query->leftJoin('team', 'visiting_team', "s.visiting_team_name = visiting_team.name AND visiting_team.sport = '" . $sport . "'");
	$query->leftJoin('venue', 'v', 'v.vid = s.venue_id');
	$query->leftJoin('sportspoll_tally', 
		'host_poll', 
		'host_poll.team_name = s.host_team_name and host_poll.week = s.week AND host_poll.season = s.season and host_poll.rank <= 25');
	$query->leftJoin('sportspoll_tally', 
		'visiting_poll', 
		'visiting_poll.team_name = s.visiting_team_name and visiting_poll.week = s.week AND visiting_poll.season = s.season and visiting_poll.rank <= 25');
	
	$query->addField('v', 'title', 'venue_title');
	$query->addField('v', 'official_title', 'venue_official_title');
	$query->addField('v', 'nickname', 'venue_nickname');
	$query->addField('v', 'location_city', 'venue_location_city');
	$query->addField('v', 'location_state', 'venue_location_state');
	$query->addField('v', 'capacity', 'venue_capacity');
	$query->addField('v', 'surface', 'venue_surface');
	
	$query->addField('host_team', 'title', 'host_team_title');
	$query->addField('host_team', 'displaytitle', 'host_team_displaytitle');
	$query->addField('host_team', 'nickname', 'host_team_nickname');
	
	$query->addField('visiting_team', 'title', 'visiting_team_title');
	$query->addField('visiting_team', 'displaytitle', 'visiting_team_displaytitle');
	$query->addField('visiting_team', 'nickname', 'visiting_team_nickname');
	
	$query->addField('host_poll', 'rank', 'host_team_rank');
	$query->addField('visiting_poll', 'rank', 'visiting_team_rank');

	$query
		->orderBy('s.season')
		->orderBy('event_date')
		->orderBy('s.week');
		
	$sportsevents = $query->execute();

	$query = db_select('team', 't')
		->fields('t')
		->condition('conf_name', $team->conf_name)
		->condition('sport', $sport)
		->orderBy('conf_division')
		->orderBy('displaytitle');
	$conference_teams = $query->execute();
	
	
	$items = array();
	$extras = array();
	$division = '';
	foreach ($conference_teams as $t) {
		if ($t->name == 'notredame' && $sport == 'ncaaf') {
			 $extras[] = $t; 
			 continue; 
		}
		if ($division != $t->conf_division) {
			if (!empty($division)) {
				$items[] = array();
			}
			$division = $t->conf_division;
		}
		$logo_url = drupal_get_path('module', 'team') . '/logos/master/' . $t->name . '.gif';
		$items[] = l('<img height="24" src="/' . $logo_url . '"/>&nbsp;' . t($t->displaytitle), 'teams/' . $sport . '/' . $t->name,
			array('html' => TRUE));
	}
	if (!empty($extras)) {
		$items[] = array();
		$items[] = l(t('Other Schools'), '#', array(
			'attributes' => array('class' => array('dropdown-header'), '#header' => TRUE)
		));
		foreach ($extras as $t) {
			$logo_url = drupal_get_path('module', 'team') . '/logos/master/' . $t->name . '.gif';
			$items[] = l('<img height="24" src="/' . $logo_url . '"/>&nbsp;' . t($t->displaytitle), 'teams/' . $sport . '/' . $t->name,
			array('html' => TRUE));
		}
	}
	
	$master_key = 'sportsevents_' . $team->name;
	
	$team_logo_url = sprintf("/%s/logos/%s/%s.gif", drupal_get_path('module', 'team'), 'master', $team->name);
	
	$page = array();
	$page['logo'] = array(
		'#type' => 'markup',
		'#markup' => '<h3><img width="75" src="' . $team_logo_url . '"/></h3>',
	);
	#$page['subtitle'] = array(
	#	'#type' => 'markup',
	#	'#markup' => '<h3>' . $current_season . ' Schedule - ' . SportsEvent::$sport[$sport] . '</h3>',
	#);
	drupal_set_title(sprintf("%s %s Schedule - %s", $team->displaytitle, $current_season, SportsEvent::$sport[$sport]));

	$page['dropdown'] = array(
		'#theme' => 'bootstrap_dropdown',
		'#items' => $items,
		'#toggle' => array(
			'#type' => 'button',
			'#value' => ($conference ? t($conference->displaytitle . ' Teams') : t('Teams of Interest')) . '&nbsp;<span class="caret"></span>',
			'#button_type' => 'button',
			'#attributes' => array(
				'class' => array('btn'),
			),
		),
		'#wrapper_attributes' => array(
			'style' => 'margin-bottom: 10px',
		)
	);

	$last_season = FALSE;
	$last_month = FALSE;
	
	$rows = array();
	$header = array(
		t(''),
		t('Date'),
		t('Opponent'),
		t('Time/Result'),
		t('Location'),
	);
	
	$rows = array();
	foreach ($sportsevents as $se) {
		
		# MISC
		$extra_misc_str = '&nbsp;';
		if ($sport == 'ncaam') {
			$extra_misc_str = date('D', $se->event_date);
		}
		# OPPONENTS
		$opponent_str = '';
		if (($se->neutral == 'N') && ($se->visiting_team_name == $team->name)) {
			$opponent_str .= 'at ';
		}
		if ($se->neutral == 'Y') {
			$opponent_str .= 'vs ';
		}
		
		$opponent = $se->host_team_name == $team->name ? $se->visiting_team_displaytitle : $se->host_team_displaytitle;
		$opponent_url_name = $se->host_team_name == $team->name ? $se->visiting_team_name : $se->host_team_name;
		$opponent_conf_name = $se->host_team_name == $team->name ? $se->visiting_conf_name : $se->host_conf_name;
		
		if (in_array($opponent_conf_name, array('acc'))) {
			$opponent = l($opponent, 'teams/' . $sport . '/' . $opponent_url_name);
		}
		
		#$opponent_str = $se->host_team_name == $team->name 
		#	? t($opponent_str . (!empty($se->visiting_team_rank) ? '#' . $se->visiting_team_rank . ' ' : '') . $se->visiting_team_displaytitle) 
		#	: t($opponent_str . (!empty($se->host_team_rank) ? '#' . $se->host_team_rank . ' ' : '') . $se->host_team_displaytitle);
		
		$opponent_str = $se->host_team_name == $team->name 
			? t($opponent_str . (!empty($se->visiting_team_rank) ? '#' . $se->visiting_team_rank . ' ' : '') . $opponent) 
			: t($opponent_str . (!empty($se->host_team_rank) ? '#' . $se->host_team_rank . ' ' : '') . $opponent);

		if (empty($opponent_str) || in_array($opponent_str, array('at ', 'vs '))) {
			$opponent_str .= 'TBA';
		}
		
		if (!empty($se->title)) {
			$opponent_str .= '<div><small>' . $se->title . '</small></div>';
		}
		
		if ($pickems && $se->completed != 'Y') {
			#$opponent_str .= '<div><small>' . 
		}			
		# end OPPONENT

		# LOCATION
		$location_str = $se->location;
		if (!empty($se->venue_id)) {
			$location_str = sprintf("%s<br /><small>%s, %s</small>", $se->venue_title, $se->venue_location_city, $se->venue_location_state);
		}
		
		# TIME/RESULT
		$time_string = date('H:i', $se->event_date);
		$time_result = '';
		if ($se->completed == 'Y') {
			$scores = array($se->host_score, $se->visiting_score);
			arsort($scores);
			if ($team->name == $se->winning_team_name) {
				$time_result = '<strong>W</strong> ';
			} else {
				$time_result = '<strong>L</strong> ';
			}
			if (!empty($se->summary_url)) {
				$time_result .= l(implode('-', $scores), $se->summary_url, array('attributes' => array('target' => '_blank')));
			}
			else {
				$time_result .= implode('-', $scores);
			}
			
		} else {
			if ($time_string == '00:00') {
				$time_result = 'TBA';
			}
			else {
				$time_result = date('g:i A', $se->event_date);
			}
		}
		
		$month_string = date('F', $se->event_date);
		
		if ($month_string != $last_month) {
			$rows[] = array(
				'data' => array(
					array(
						'data' => '<h4>' . $month_string . '</h4>',
						'header' => TRUE,
						'colspan' => 5
					),
				),
				'class' => array('info'),
			);
			$last_month = $month_string;
		}
		# BUILD THE ROW
		$rows[] = array(
			array(
				'data' => $extra_misc_str,
				'width' => '15px'
			),
			array(
				'data' => ($se->event_date == NULL) ? t('TBA') : $se->event_date_short,
				'width' => '15%',
			),
			array(
				'data' => $opponent_str,
				'width' => '30%',
			),
			array(
				'data' => $time_result,
			),
			array(
				'data' => $location_str,
			),
		);
		
	}
	$page['table'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
	);
		
	return $page;
	/*
	$output = drupal_render($page);

	return array(
		'#type' => 'markup',
		'#markup' => $output,
		'#cache' => array(
			'cid' => $master_key,
			'bin' => 'cache_page',
			'expire' => 60,
		)
	);
	*/
}

function team_sport_future_schedule($team, $sport, $season = NULL) {
	global $conf;
	global $user;
	
	$current_season = $conf['current_season'][$sport];
	$conference = conference_load_by_name( $team->conf_name );

	$query = db_select('sportsevent', 's')
		->condition(
			db_or()->condition('visiting_team_name', $team->name)->condition('host_team_name', $team->name)
		)
		->condition('season', $current_season, '>')
		->condition('s.sport', $sport)
		->fields('s', array('eid', 'event_date', 'neutral', 'season', 'title', 'host_team_name', 'visiting_team_name', 'location', 'venue_id'));
	$query->addExpression("FROM_UNIXTIME(event_date, '%b %e')", 'event_date_short');
		
	$query->join('team', 'host_team', "s.host_team_name = host_team.name AND host_team.sport = '" . $sport . "'");
	$query->join('team', 'visiting_team', "s.visiting_team_name = visiting_team.name AND visiting_team.sport = '" . $sport . "'");
	$query->leftJoin('venue', 'v', 'v.vid = s.venue_id');
	
	$query->addField('v', 'title', 'venue_title');
	$query->addField('v', 'official_title', 'venue_official_title');
	$query->addField('v', 'nickname', 'venue_nickname');
	$query->addField('v', 'location_city', 'venue_location_city');
	$query->addField('v', 'location_state', 'venue_location_state');
	$query->addField('v', 'capacity', 'venue_capacity');
	$query->addField('v', 'surface', 'venue_surface');
	
	$query->addField('host_team', 'title', 'host_team_title');
	$query->addField('host_team', 'displaytitle', 'host_team_displaytitle');
	$query->addField('host_team', 'nickname', 'host_team_nickname');
	
	$query->addField('visiting_team', 'title', 'visiting_team_title');
	$query->addField('visiting_team', 'displaytitle', 'visiting_team_displaytitle');
	$query->addField('visiting_team', 'nickname', 'visiting_team_nickname');

	$query
		->orderBy('season')
		->orderBy('event_date')
		->orderBy('week');
		
	$sportsevents = $query->execute();

	$query = db_select('team', 't')
		->fields('t')
		->condition('conf_name', $team->conf_name)
		->condition('sport', $sport)
		->orderBy('conf_division')
		->orderBy('displaytitle');
	$conference_teams = $query->execute();
	
	
	$items = array();
	$extras = array();
	$division = '';
	foreach ($conference_teams as $t) {
		if ($t->name == 'notredame') {
			 $extras[] = $t; 
			 continue; 
		}
		if ($division != $t->conf_division) {
			if (!empty($division)) {
				$items[] = array();
			}
			$division = $t->conf_division;
		}
		$logo_url = drupal_get_path('module', 'team') . '/logos/' . $sport . '/' . $t->name . '.gif';
		$items[] = l('<img height="24" src="/' . $logo_url . '"/>&nbsp;' . t($t->displaytitle), 'teams/' . $sport . '/future/' . $t->name,
			array('html' => TRUE));
	}
	if (!empty($extras)) {
		$items[] = array();
		$items[] = l(t('Other Schools'), '#', array(
			'attributes' => array('class' => array('dropdown-header'), '#header' => TRUE)
		));
		foreach ($extras as $t) {
			$logo_url = drupal_get_path('module', 'team') . '/logos/' . $sport . '/' . $t->name . '.gif';
			$items[] = l('<img height="24" src="/' . $logo_url . '"/>&nbsp;' . t($t->displaytitle), 'teams/' . $sport . '/future/' . $t->name,
			array('html' => TRUE));
		}
	}
	
	$master_key = 'sportsevents_' . $team->name;
	
	$team_logo_url = sprintf("/%s/logos/%s/%s.gif", drupal_get_path('module', 'team'), $sport, $team->name);
	
	$page = array();
	$page['logo'] = array(
		'#type' => 'markup',
		'#markup' => '<img width="75" src="' . $team_logo_url . '"/>',
	);
	#$page['subtitle'] = array(
	#	'#type' => 'markup',
	#	'#markup' => '<h3>Future Schedule(s) - ' . SportsEvent::$sport[$sport] . '</h3>',
	#);
	drupal_set_title(sprintf("%s Future Schedules - %s", $team->displaytitle, SportsEvent::$sport[$sport]));

	$page['dropdown'] = array(
		'#theme' => 'bootstrap_dropdown',
		'#items' => $items,
		'#toggle' => array(
			'#type' => 'button',
			'#value' => ($conference ? t($conference->displaytitle . ' Teams') : t('Teams of Interest')) . '&nbsp;<span class="caret"></span>',
			'#button_type' => 'button',
			'#attributes' => array(
				'class' => array('btn'),
			),
		),
		'#wrapper_attributes' => array(
			'style' => 'margin-bottom: 10px',
		)
	);

	$last_season = FALSE;
	$rows = array();
	$header = array(
		t('Date'),
		t('Opponent'),
		t('Location'),
	);
	
	$cache_expire = time() + 3600;
	$cache_expire_page = $cache_expire - 3000;
	
	$last_season = FALSE;
	foreach ($sportsevents as $se) {

		if ($se->season != $last_season) {
			$last_season = $se->season;
			$page['table_' . $last_season] = array(
				'#type' => 'fieldset',
				'#title' => $last_season,
				'#collapsible' => TRUE,
				'#collapsed' => FALSE,
			);
			$page['table_' . $last_season]['table'] = array(
				'#theme' => 'table',
				'#header' => $header,
				'#rows' => array(),
			);
		}
		
		# OPPONENTS
		$opponent_str = '';
		if (($se->neutral == 'N') && ($se->visiting_team_name == $team->name)) {
			$opponent_str .= 'at ';
		}

		if ($se->neutral == 'Y') {
			$opponent_str .= 'vs ';
		}
		$opponent_str = $se->host_team_name == $team->name 
			? t($opponent_str . $se->visiting_team_displaytitle) 
			: t($opponent_str . $se->host_team_displaytitle);

		if (empty($opponent_str) || in_array($opponent_str, array('at ', 'vs '))) {
			$opponent_str .= 'TBA';
		}
			
		# LOCATION
		$location_str = $se->location;
		if (!empty($se->venue_id)) {
			$location_str = sprintf("%s<br /><small>%s, %s</small>", $se->venue_title, $se->venue_location_city, $se->venue_location_state);
		}
		
		# BUILD THE ROW
		$page['table_' . $se->season]['table']['#rows'][] = array(
			array(
				'data' => ($se->event_date == NULL) ? t('TBA') : $se->event_date_short,
				'width' => '10%',
			),
			array(
				'data' => $opponent_str,
				'width' => '35%',
			),
			array(
				'data' => $location_str,
			),
		);
		
	}
	return $page;
	/*
	$output = drupal_render($page);

	return array(
		'#type' => 'markup',
		'#markup' => $output,
		'#cache' => array(
			'cid' => $master_key,
			'bin' => 'cache_page',
			'expire' => 60,
		)
	);
	*/
}

function team_ncaaf_future_schedules() {
	return theme('ncaaf_future_schedules');
}
