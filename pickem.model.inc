<?php


function pickem_load_game($gid) {
	if ($gid == NULL) {
		return NULL;
	}
	$result = db_query('SELECT g.* FROM game g WHERE gid = :gid', array(':gid' => $gid));
	return $result->fetchObject();
}

function pickem_load_games($week, $season, $conf = NULL) {
	$q = db_select('game', 'g');
	$q->join('school', 'host', 'host.name = g.host_school_name');
	$q->join('school', 'visiting', 'visiting.name = g.visiting_school_name');
	$q->fields('g');
	$q->addField('host', 'displaytitle', 'host_school');
	$q->addField('visiting', 'displaytitle', 'visiting_school');
	$q->condition('g.season', $season);
	$q->condition('g.week', $week);
	
	if (isset($conf) && $conf != NULL) {
		$or = db_or();
		$or->condition('host.conf_name', $conf);
		$or->condition('visiting.conf_name', $conf);
		$q->condition($or);
	}
	$q->orderBy('game_date');
	
	$result = $q->execute();
	$games = array();
	while ($game = $result->fetchObject()) {
		pickem_create_gamedate_attributes($game);
		$games[] = $game;
	}
	
	return $games;
}

function pickem_game_load_by_school($school, $week = NULL, $include_slate = FALSE) {
	if (is_object($school)) {
		$school_name = $school->name;
	} else {
		$school_name = $school;
	}
	$query = db_select('game', 'g');
	$query->join('school', 'host', 'g.host_school_name = host.name');
	$query->join('school', 'visiting', 'g.visiting_school_name = visiting.name');
	$query->fields('g');
	$query->addField('host', 'displaytitle', 'host_school');
	$query->addField('visiting', 'displaytitle', 'visiting_school');
	
	if ($include_slate) {
		$query->leftJoin('slate', 'sl', 'sl.gid = g.gid');
		$query->addField('sl', 'slate_id', 'slate_id');
	}
	
	$or = db_or();
	$or->condition('host.name', $school_name);
	$or->condition('visiting.name', $school_name);
	
	$query->condition($or);
	if ($week) {
		$query->condition('week', $week);
	}
	
	$result = $query->execute();
	$game = $result->fetchObject();
	if ($game) {
		# OPPONENT string properties
		if ($game->neutral == 'Y') {
			$game->vs_at = 'vs';
			$game->opponent_title = 'vs ' . 
				($game->host_school_name == $school_name ? $game->visiting_school : $game->host_school);
		} else {
			$game->vs_at = $game->host_school_name == $school_name ? '' : 'at';
			if ($game->host_school_name == $school_name) {
				$game->opponent_title = strtoupper($game->visiting_school);
			} else {
				$game->opponent_title = 'at ' . $game->host_school;
			}
		}
		
		# keep track of the opponent's conference affiliation
		# for use when determing whether or not to display a link 
		# to that school's schedule
		$game->opponent_conf_name = ($game->host_school_name == $school_name) 
			? $game->visiting_conf_name 
			: $game->host_conf_name;
			
		$game->opponent_school_name = ($game->host_school_name == $school_name) 
			? $game->visiting_school_name 
			: $game->host_school_name;
			
		$game->opponent_school = ($game->host_school_name == $school_name) 
			? $game->visiting_school 
			: $game->host_school;
			
		$game->game_date_str = date('M j, Y', $game->game_date);
		$game->kickoff_time = date('g:i A', $game->game_date);
		if ($game->kickoff_time == '12:00 AM') {
			$game->kickoff_time = 'TBA';
		}
		$game->game_day = date('D', $game->game_date);

		$game->game_title = $game->visiting_school;
		if ($game->neutral == 'Y') {
			$game->game_title .= ' vs ';
		} else {
			$game->game_title .= ' at ';
		}
		$game->game_title .= $game->host_school;
		
		$game->game_status = $game->completed == 'Y' ? 'F' : $game->kickoff_time;
	}
	return $game;
}

function pickem_load_championship_game($school, $week, $season, $sport = 'ncaaf') {

}

function pickem_load_conferences($p5 = TRUE) {
	$query = db_select('conference', 's')->extend('PagerDefault')->extend('TableSort');
	$query->fields('s');
	if ($p5) {
		$query->condition('name', array('acc','big10','big12','pac12','sec','ind'), 'IN');
	}
	$result = $query->execute();
	$rows = array();
	while ($c = $result->fetchObject()) {
		$rows[] = $c;
	}
	return $rows;
}		

function pickem_load_players() {
	$query = db_select('pickem_player', 'pp');
	$query->join('users', 'u', 'pp.uid = u.uid');
	$query->fields('pp', array('games_picked', 'player_level', 'uid'));
	$query->fields('u', array('name'));
	
	$res = $query->execute();
	$players = array();
	
	while ($p = $res->fetchObject()) {
		$players[] = $p;
	}
	return $players;
}

function pickem_load_standings($week, $season = NULL) {
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}

	$q = db_select('standings', 's')
		->fields('s');
	$q->join('users', 'u', 's.uid = u.uid');
	$q->addField('u', 'name', 'player_name');
	$q->condition('s.week', $week)
		->condition('s.season', $season)
		->orderBy('(correct + incorrect)', 'DESC')
		->orderBy('correct', 'DESC')
		->orderBy('player_name');
		
	$res = $q->execute();
	
		
	$rows = array();
	foreach ($res as $player) {
		if ($week > 0) {
			$subq = db_select('pick', 'p');
			$subq->addExpression('COUNT(uid)', 'pick_count');
			$subq->condition('p.uid', $player->uid);
			$subq->condition('p.week', $week);
			
			$res2 = $subq->execute();
			$pick_data = $res2->fetchObject();
			$player->pick_count = $pick_data->pick_count;
		}
		$rows[] = $player;
	}
	return $rows;
}

function pickem_update_standings($week, $season = NULL) {
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}
	$num = 0;
	$pick_data = pickem_load_pick_data($week, $season);
	foreach($pick_data['player'] as $player) {
		$pct = $player['correct'] / ($player['correct'] + $player['incorrect']);
		$num += db_update('standings')->fields(array(
			'correct' => $player['correct'],
			'incorrect' => $player['incorrect'],
			'pct' => $pct,
		))
		->condition('uid', $player['uid'])
		->condition('week', $week)
		->condition('season', $season)
		->execute();
	}
	
	$pick_data = pickem_load_pick_data(-1, $season);
	foreach($pick_data['player'] as $player_name => $player) {
		if ($player['correct'] == 0 && $player['incorrect'] == 0) {
			$pct = 0.0;
			drupal_set_message('updating overal standings, player = [' . $player_name . ']; no picks', 'warning');
		} else {
			$pct = $player['correct'] / ($player['correct'] + $player['incorrect']);
		}
		$num += db_update('standings')->fields(array(
			'correct' => $player['correct'],
			'incorrect' => $player['incorrect'],
			'pct' => $pct,
		))
		->condition('uid', $player['uid'])
		->condition('week', -1)
		->condition('season', $season)
		->execute();
	}
	return $num;
}

function pickem_load_player($uid) {
	$q = db_select('pickem_player', 'p');
	$q->join('users', 'u', 'p.uid = u.uid');
	$q->fields('p')
		->fields('u', array('name'))
		->condition('p.uid', $uid);
		
	$r = $q->execute();
	return $r->fetchObject();	
}

function pickem_load_weeks() {
}

function pickem_load_game_picks($gid) {
	$result = db_query('SELECT p.* FROM {pick} p WHERE gid = :gid', array(':gid' => $gid));
	$picks = array();
	foreach ($result as $pick) {
		$picks[] = $pick;
	}
	return $picks;
}

function pickem_load_pick_data($week, $season, $options = array()) {
	$pick_data = array(
		'player' => array(
		),
		'game' => array(
		),
		'dbrow' => array(),
	);
		
	$q = db_select('pick', 'p')
		->fields('p')
		->condition('p.season', $season);
		
	if ($week > 0) {
		$q->condition('p.week', $week);
	}
		
	$q->join('users', 'u', 'p.uid = u.uid');
	$q->addField('u', 'name', 'playername');
	
	$q->join('school', 'host', 'g.host_school_name = host.name');
	$q->join('school', 'visiting', 'g.visiting_school_name = visiting.name');
	$q->addField('host', 'displaytitle', 'host_school');
	$q->addField('visiting', 'displaytitle', 'visiting_school');
	
	$q->join('game', 'g', 'p.gid = g.gid');
	$q->addField('g', 'host_school_name');
	$q->addField('g', 'visiting_school_name');
	$q->addField('g', 'winning_school_name');
	
	if (isset($options['sortbyplayer'])) {
		$q->orderBy('player');
	}
	
	$res = $q->execute();
	while ($row = $res->fetchObject()) {
		
		// PLAYER data
		if (!isset($pick_data['player'][$row->playername])) {
			$pick_data['player'][$row->playername] = array(
				'num_picks' => 0,
				'correct' => 0,
				'incorrect' => 0,
				'week' => array(),
				'uid' => $row->uid,
			);
		}
		if ($row->pick_school_name != '') {
			$pick_data['player'][$row->playername]['num_picks']++;
			if ($row->correct == 'Y') {
				$pick_data['player'][$row->playername]['correct']++;
			} else if ($row->correct == 'N') {
				$pick_data['player'][$row->playername]['incorrect']++;
			}
		}
		 // -1 = gather all the standings
		if ($week == -1) {
			if (!isset($pick_data['player'][$row->playername]['week'][$row->week])) {
				$pick_data['player'][$row->playername]['week'][$row->week] = array(
					'num_picks' => 0,
					'correct' => 0,
					'incorrect' => 0,
				);
			}
			$pick_data['player'][$row->playername]['week'][$row->week]['num_picks']++;
			if ($row->correct == 'Y') {
				$pick_data['player'][$row->playername]['week'][$row->week]['correct']++;
			} else if ($row->correct == 'N') {
				$pick_data['player'][$row->playername]['week'][$row->week]['incorrect']++;
			}
		}
		
		// GAME data
		if (!isset($pick_data['game'][$row->gid])) {
			$pick_data['game'][$row->gid] = array(
				$row->host_school_name => array('count' => 0, 'who' => array()),
				$row->visiting_school_name => array('count' => 0, 'who' => array()),
				'visiting_school' => $row->visiting_school,
				'visiting_school_name' => $row->visiting_school_name,
				'host_school' => $row->host_school,
				'host_school_name' => $row->host_school_name,
				'game_title' => $row->visiting_school . ' - ' . $row->host_school,
			);
		}
		if ($row->pick_school_name != '') {
			$pick_data['game'][$row->gid][$row->pick_school_name]['count']++;
			$pick_data['game'][$row->gid][$row->pick_school_name]['who'][] = $row->playername;
		}
	}
	return $pick_data;
}

function pickem_school_load($name) {
	$query = db_select('school', 's');
	$query->leftJoin('school_colors', 'c', 's.sid = c.sid');
	$query
		->fields('s')
		->fields('c', array('color_primary', 'color_secondary', 'color_alt'));
				
	if (is_numeric($name)) {
		$query->condition('s.sid', $name);
	} else {
		$query->condition('s.name', $name);
	}
	
	$result = $query->execute();
	$school = $result->fetchObject();
	$school->has_colors = ($school->color_primary != '');
	return $school;
}

function pickem_can_show_schedule_link($school_name, $conf_name) {
	return in_array($school_name, array('notredame', 'byu', 'louisville')) 
	|| in_array($conf_name, 
			array('acc', 'big10', 'big12', 'sec', 'pac12', 'aac', 'cusa', 'mac', 'mwc', 'sunbelt'));
}

function _pickem_menu_weight_for_conf($confname) {
	# higher priority goes to P5 schools, then Independents, then G5
	$p5 = array('acc','big10','big12','pac12','sec');
	$ind = array('ind');
	$g5 = array('aac','cusa','mac','mwc','sunbelt');
	
	if (in_array($confname, $p5)) {
		return -6;
	}
	if (in_array($confname, $ind)) {
		return -5;
	}
	if (in_array($confname, $g5)) {
		return 0;
	}
}

function pickem_create_gamedate_attributes(&$game) {
	$game->game_date_str = date('M j, Y', $game->game_date);
	$game->kickoff_time = date('g:i A', $game->game_date);
	if ($game->kickoff_time == '12:00 AM') {
		$game->kickoff_time = 'TBA';
	}
	$game->game_day = date('D', $game->game_date);
}

function pickem_is_locked_for_picks($week, $season) {
	if ($week > $_SESSION['current_week']->week && $season != variable_get('pickem.current_season')) {
		return FALSE;
	}
	$q = db_select('slate', 's');
	$q->join('game', 'g', 'g.gid=s.gid');
	$q->addField('g', 'game_date');
	$q->condition('slate_id', $week);
	$q->condition('s.season', $season);
	$q->orderBy('g.game_date');
	$q->range(0, 1);
	$result = $q->execute();
	$game = $result->fetchObject();
	
	$cutoffTime = $game->game_date - (60 * variable_get('pickem.cutoff_minutes', 30));
	return time() >= $cutoffTime;
}
