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
		# who won?
		if ($game->completed == 'Y') {
			$game->result = '';
			if ($school_name == $game->winning_school_name) {
				$game->result = 'W ';
			} else {
				$game->result = 'L ';
			}
			if ($school_name == $game->host_school_name) {
				$game->result .= $game->host_score . ' - ' . $game->visiting_score;
			} else {
				$game->result .= $game->visiting_score . ' - ' . $game->host_score;
			}
		}
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

function pickem_load_players($assoc = FALSE, $player_level = FALSE) {
	$query = db_select('pickem_player', 'pp');
	$query->join('users', 'u', 'pp.uid = u.uid');
	$query->fields('pp', array('games_picked', 'player_level', 'uid'));
	$query->fields('u', array('name'));
	if ($player_level) {
		$query->condition('player_level', $player_level);
	}
	
	$res = $query->execute();
	$players = array();
	
	while ($p = $res->fetchObject()) {
		if ($assoc) {
			$players[$p->name] = $p;
		} else {
			$players[] = $p;
		}
	}
	return $players;
}

function pickem_load_conf_standings($conf, $season, $sport='ncaaf') {
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}
	$q = db_select('school_records', 'sr')
		->fields('sr');
	$q->join('school', 's', 's.name = sr.school_name');
	$q->fields('s', array('conf_division', 'displaytitle', 'conf_name'));
	$q->addField('s', 'name', 'school_name');
	$q->condition('sr.season', $season);
	$q->condition('sr.tag', array('overall', 'conf'));
	$q->condition('s.conf_name', $conf);
	$res = $q->execute();
	
	$standings = array('has_divisions'=>FALSE);
	foreach ($res as $row) {
		if ($row->conf_division != '') {
			if (!isset($standings['divisions'])) {
				$standings['divisions'] = array();
				$standings['has_divisions'] = TRUE;
			} 
			if (!isset($standings['divisions'][$row->conf_division])) {
				$standings['divisions'][$row->conf_division] = array();
			}
			if (!isset($standings['divisions'][$row->conf_division][$row->school_name])) {
				$standings['divisions'][$row->conf_division][$row->school_name] = array(
					'wins' => 0, 'losses' => 0, 'ties' => 0, 'pct' => 0.0, 'cwins' => 0, 'closses' => 0, 'cties' => 0, 'cpct' => 0.0, 
					'school' => $row->displaytitle, 'school_name' => $row->school_name, 'conf_div' => $row->conf_division,
				);
			}
			$target =& $standings['divisions'][$row->conf_division][$row->school_name];
		} else {
			if (!isset($standings[$row->school_name])) {
				$standings[$row->school_name] = array(
					'wins' => 0, 'losses' => 0, 'ties' => 0, 'pct' => 0.0, 'cwins' => 0, 'closses' => 0, 'cties' => 0, 'cpct' => 0.0,
					'school' => $row->displaytitle, 'school_name' => $row->school_name,
				);
			}
			$target =& $standings[$row->school_name];
		}
		if ($row->tag == 'overall') {
			$target['wins'] = $row->wins;
			$target['losses'] = $row->losses;
			$target['ties'] = $row->ties;
			$target['games'] = array_sum(array($row->wins, $row->ties, $row->losses));
			$target['pct'] = $target['games'] ? (float) ($row->wins / $target['games']) : 0.0;
		}
		if ($row->tag == 'conf') {
			$target['cwins'] = $row->wins;
			$target['closses'] = $row->losses;
			$target['cties'] = $row->ties;
			$target['cgames'] = array_sum(array($row->wins, $row->ties, $row->losses));
			$target['cpct'] = $target['cgames'] ? (float) ($row->wins / $target['cgames']) : 0.0;
		}
	}
	if ($standings['has_divisions']) {
		foreach (array_keys($standings['divisions']) as $div_name) {
			uasort($standings['divisions'][$div_name], 'conference_standings_sort');
		}
	} else {
		unset($standings['has_divisions']);
		uasort($standings, 'conference_standings_sort');
		$standings['has_divisions'] = FALSE;
	}
	return $standings;
}

function pickem_load_school_stats($school_name, $season, $sport = 'ncaaf') {
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}
	$q = db_select('school_records', 'sr');
	$q->condition('season', $season);
	$q->condition('sport', $sport);
	$q->condition('tag', array('overall', 'conf'), 'NOT IN');
	$q->condition('school_name', $school_name);
	$q->fields('sr');
	$res = $q->execute();
	
	return array();
}

function conference_standings_sort($a, $b) {
	$equal_cgames = ($a['cgames'] == $b['cgames']);
	$both_played_cgames = ($a['cgames'] > 0) && ($b['cgames'] > 0);
	$equal_games = ($a['games'] == $b['games']);
	$both_played_games = ($a['games'] > 0) && ($b['games'] > 0);
	$pcts_match = ($a['pct'] == $b['pct']);
	$cpcts_match = ($a['cpct'] == $b['cpct']);
	
	$returnval = 0;
	$returnwhy = '';
	
	// conference games is the first tiebreaker
	if ($both_played_cgames) {
		if ($equal_cgames || ($a['cpct'] == $b['cpct'])) {
			$returnval = ($a['cwins'] > $b['cwins']) ? -1 : 
				(($a['cwins'] > $b['cwins']) ? -1 : 0);
			$returnwhy = 'conf-record';
		}
	} elseif (($a['cgames'] == 0) && ($b['cgames'] == 0)) {
			if ($pcts_match) {
				if ($both_played_games) {
					$returnval = $a['wins'] > $b['wins'] ? -1 : ($equal_games ? 0 : -1);
				}
			} else {
				$returnval = $a['pct'] > $b['pct'] ? -1 : 1;
			}
			$returnwhy = 'overall-record';
	} elseif (($a['cgames'] > 0) && ($b['cgames'] == 0)) {
		$returnval = ($a['wins'] > 0) ? -1 : 1;
		$returnwhy = 'conf-record';
	} else if (($a['cgames'] == 0) && ($b['cgames'] > 0)) {
		$returnval = ($b['wins'] > 0) ? 1 : -1;
		$returnwhy = 'conf-record';
	} else {
		$returnval = ($a['school'] > $b['school']) ? -1 : -1;
		$returnwhy = 'alpha';
	}
	#dpm(array($a, $b, $returnval, $returnwhy));
	return $returnval;
}

function pickem_load_standings($week, $pickem_id, $season = NULL) {
	if ($season == NULL) {
		$season = variable_get('pickem.current_season');
	}

	$q = db_select('standings', 's')
		->fields('s');
	$q->join('users', 'u', 's.uid = u.uid');
	$q->join('pickem_player', 'pp', 'pp.uid= s.uid');
	#$q->addExpression('s', 's.correct + s.incorrect', 'games_picked');
	$q->addField('u', 'name', 'player_name');
	$q->addField('pp', 'weeks_won', 'weeks_won');
	$q->condition('s.week', $week)
		->condition('s.pickem_id', $pickem_id)
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
		} else {
			$player->pick_count = $player->correct + $player->incorrect;
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

function pickem_load_player($uid, $pickem_id) {
	$q = db_select('pickem_player', 'p');
	$q->join('users', 'u', 'p.uid = u.uid');
	$q->fields('p');
	$q->addField('u', 'name', 'player_name');
	$q->condition('p.uid', $uid);
	$q->condition('p.pickem_id', $pickem_id);
		
	$r = $q->execute();
	return $r->fetchObject();	
}

function pickem_load_weeks() {
}

function pickem_load_game_picks($gid) {
	$result = db_query('SELECT p.*, u.name AS player_name FROM {pick} p JOIN users u ON p.uid = u.uid WHERE gid = :gid', array(':gid' => $gid));
	$picks = array();
	foreach ($result as $pick) {
		$picks[] = $pick;
	}
	return $picks;
}

function pickem_load_pick_results($week, $season) {
	$q = db_select('pick', 'p')
		->fields('p')
		->condition('p.week', $week)
		->condition('p.seasson', $season);
	
	$q->join('users', 'u', 'u.uid=p.uid');
	$q->addField('u', 'name', 'playername');
	
	$q->
	$res = $q->execute();
	
	$rows = array();
	foreach ($res as $row) {
		$rows[] = $row;
	}
	return $rows;
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
	//$q->leftJoin('school', 'playerpick', 'p.pick_school_name = playerpick.name');
	$q->addField('host', 'displaytitle', 'host_school');
	$q->addField('host', 'conf_name', 'host_conf_name');
	$q->addField('visiting', 'displaytitle', 'visiting_school');
	$q->addField('visiting', 'conf_name', 'visiting_conf_name');
	//$q->addField('playerpick', 'displaytitle', 'pick_school');
	
	$q->join('game', 'g', 'p.gid = g.gid');
	$q->addField('g', 'host_school_name');
	$q->addField('g', 'visiting_school_name');
	$q->addField('g', 'winning_school_name');
	$q->addField('g', 'week', 'game_week');
	$q->addField('g', 'game_date');
	$q->addField('g', 'neutral');
	$q->addField('g', 'betting_line');
	$q->addField('g', 'location');
	$q->addField('g', 'completed');
	$q->addField('g', 'host_score');
	$q->addField('g', 'visiting_score');
	
	$q->orderBy('g.game_date');
	if (isset($options['uid'])) {
		$q->condition('p.uid', $options['uid']);
	}
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
				'school' => array(),
			);
		}

		$atemp = array(
			'num_games' => 0,	// number of games involving this school
			'pw' => 0,	// pw = 'picked_with',
			'pa' => 0,	// pa (in this context) = 'picked against'
			'pw_correct' => 0,
			'pw_correct_win' => 0,
			'pw_correct_loss' => 0,
			'pw_incorrect' => 0,
			'pw_incorrect_win' => 0,
			'pw_incorrect_loss' => 0,
			'pa_correct' => 0,
			'pa_correct_win' => 0,
			'pa_correct_loss' => 0,
			'pa_incorrect' => 0,
			'pa_incorrect_win' => 0,
			'pa_incorrect_loss' => 0,
			'correct' => 0,
			'incorrect' => 0,
		);
		$row->pick_school = '';
		if ($row->host_school_name == $row->pick_school_name) {
			$row->pick_school = $row->host_school;
		} else if ($row->visiting_school_name == $row->pick_school_name) {
			$row->pick_school = $row->visiting_school;
		}
		if ($row->pick_school_name != '') {
			if (!isset($pick_data['player'][$row->playername]['school'][$row->host_school])) {
				$pick_data['player'][$row->playername]['school'][$row->host_school] = $atemp;
				$pick_data['player'][$row->playername]['school'][$row->host_school]['conf_name'] = $row->host_conf_name;
			}
			$pick_data['player'][$row->playername]['school'][$row->host_school]['num_games']++;
			if (!isset($pick_data['player'][$row->playername]['school'][$row->visiting_school])) {
				$pick_data['player'][$row->playername]['school'][$row->visiting_school] = $atemp;
				$pick_data['player'][$row->playername]['school'][$row->visiting_school]['conf_name'] = $row->visiting_conf_name;
			}
			if ($row->correct != '') {
				$pick_data['player'][$row->playername]['school'][$row->visiting_school]['num_games']++;
			}
		}
		
		if ($row->pick_school_name != '') {
			$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw']++;
			if ($row->pick_school_name == $row->host_school_name) {
				$pick_data['player'][$row->playername]['school'][$row->visiting_school]['pa']++;
			} else {
				$pick_data['player'][$row->playername]['school'][$row->host_school]['pa']++;
			}
		}
			
		if ($row->pick_school_name != '' && ($row->correct != '')) {
			$other_school = ($row->pick_school_name == $row->host_school_name) ? $row->visiting_school : $row->host_school;
			$other_school_name = ($row->pick_school_name == $row->host_school_name) ? $row->visiting_school_name : $row->host_school_name;
			
			//dpm('game: ' . $row->host_school . ' vs. ' . $row->visiting_school . '; pick ==> [' . $row->pick_school . ']');
			
			$pick_data['player'][$row->playername]['num_picks']++;
			if ($row->correct == 'Y') {
				$pick_data['player'][$row->playername]['correct']++;
				
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['correct']++;
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw_correct']++; 
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw_correct_win'] += ($row->winning_school_name == $row->pick_school_name) ? 1 : 0; 
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw_correct_loss'] += ($row->winning_school_name != $row->pick_school_name) ? 1 : 0; 
				
				$pick_data['player'][$row->playername]['school'][$other_school]['correct']++;
				$pick_data['player'][$row->playername]['school'][$other_school]['pa_correct']++; 
				$pick_data['player'][$row->playername]['school'][$other_school]['pa_correct_win'] += ($row->winning_school_name == $other_school_name) ? 1 : 0; 
				$pick_data['player'][$row->playername]['school'][$other_school]['pa_correct_loss'] += ($row->winning_school_name != $other_school_name) ? 1 : 0; 
			} else if ($row->correct == 'N') {

				$pick_data['player'][$row->playername]['incorrect']++;
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['incorrect']++;
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw_incorrect']++; 
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw_incorrect_win'] += ($row->winning_school_name == $row->pick_school_name) ? 0 : 1; 
				$pick_data['player'][$row->playername]['school'][$row->pick_school]['pw_incorrect_loss'] += ($row->winning_school_name != $row->pick_school_name) ? 0 : 1; 
				
				$pick_data['player'][$row->playername]['school'][$other_school]['incorrect']++;
				$pick_data['player'][$row->playername]['school'][$other_school]['pa_incorrect']++; 
				$pick_data['player'][$row->playername]['school'][$other_school]['pa_incorrect_win'] += ($row->winning_school_name == $other_school_name) ? 0 : 1; 
				$pick_data['player'][$row->playername]['school'][$other_school]['pa_incorrect_loss'] += ($row->winning_school_name != $other_school_name) ? 0 : 1; 
			}
		}

		 // -1 = gather all the standings
		//if ($week == -1) {
			if (!isset($pick_data['player'][$row->playername]['week'][$row->week])) {
				$pick_data['player'][$row->playername]['week'][$row->week] = array(
					'num_picks' => 0,
					'correct' => 0,
					'incorrect' => 0,
					'game' => array(),
				);
			}
			$pick_data['player'][$row->playername]['week'][$row->week]['game'][$row->gid] = array(
				'pick_school_name' => $row->pick_school_name,
				'pick_school' => $row->pick_school,
			);
			if ($row->correct != '') {
				$pick_data['player'][$row->playername]['week'][$row->week]['num_picks']++;
				if ($row->correct == 'Y') {
					$pick_data['player'][$row->playername]['week'][$row->week]['correct']++;
				} else if ($row->correct == 'N') {
					$pick_data['player'][$row->playername]['week'][$row->week]['incorrect']++;
				}
			}
		//}
		
		// GAME data
		if (!isset($pick_data['game'][$row->gid])) {
			$pick_data['game'][$row->gid] = array(
				'num_picks' => 0,
				'num_correct_picks' => 0,
				$row->host_school_name => array('count' => 0, 'pct' => 0.0, 'who' => array(), 'pct_correct' => 0.0),
				$row->visiting_school_name => array('count' => 0, 'pct' => 0.0, 'who' => array(), 'pct_correct' => 0.0),
				'visiting_school' => $row->visiting_school,
				'visiting_school_name' => $row->visiting_school_name,
				'host_school' => $row->host_school,
				'host_school_name' => $row->host_school_name,
				'game_title' => $row->visiting_school . ' - ' . $row->host_school,
				'game_date' => $row->game_date,
				'location' => $row->location,
				'week' => $row->game_week,
				'neutral' => $row->neutral,
				'vs_at' => $row->neutral == 'Y' ? 'vs' : 'at',
				'betting_line' => $row->betting_line,
				'completed' => $row->completed,
				'visiting_score' => $row->visiting_score,
				'host_score' => $row->host_score,
			);
		}
		$pick_data['game'][$row->gid]['num_picks']++;
		if ($row->pick_school_name != '') {
			$pick_data['game'][$row->gid][$row->pick_school_name]['count']++;
			$pick_data['game'][$row->gid][$row->pick_school_name]['pct'] = 
				($pick_data['game'][$row->gid][$row->pick_school_name]['count'] / $pick_data['game'][$row->gid]['num_picks']) * 100;
				
			$pick_data['game'][$row->gid][$row->pick_school_name]['who'][] = $row->playername;
			if ($row->completed != '') {
				if ($row->completed == 'Y' && ($row->pick_school_name == $row->winning_school_name)) {
					$pick_data['game'][$row->gid]['num_correct_picks']++;
				}
			}
		}
	}
	return $pick_data;
}

function pickem_load_slate_picks_by_uid($week, $pickem_id = NULL) {
	if (is_null($pickem_id)) {
		$pickem_id = variable_get('pickem.default_pickem_id', 1);	// default to 2013 accbbs pickem
	}
	$query = db_select('pick', 'p');
	$query->join('school', 's', 's.name = p.pick_school_name');
	$query->condition('p.week', $week)
		->condition('p.pickem_id', $pickem_id)
		->fields('p');
	$query->addField('s', 'displaytitle', 'pick_school');
	$result = $query->execute();
	
	$rows = array();
	foreach ($result as $row) {
		if (!isset($rows[$row->pkid])) {
			$rows[$row->pkid] = array(
				'uid' => array(),
			);
		}
		$rows[$row->pkid]['uid'][$row->uid] = $row;
	}
	return $rows;
}

function pickem_school_load($name) {
	$query = db_select('school', 's');
	$query->leftJoin('school_colors', 'c', 's.sid = c.sid');
	$query->join('conference', 'co', 's.conf_name = co.name');
	$query
		->fields('s')
		->fields('c', array('color_primary', 'color_secondary', 'color_alt'));
	$query->addField('co', 'displaytitle', 'conf_displaytitle');
				
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

function pickem_pick_is_open($game, $now, $saturday_cutoff, &$cutoff_date = NULL) {
	// for games on thursday or friday, 
	if ($game->week > $_SESSION['current_week']->week) {
		return TRUE;
	}
	
	if ($game->completed == 'Y') {
		return FALSE;
	}
	
	$day = date('D', $game->game_date);
	
	if (!is_null($cutoff_date)) {
		$cutoff_date = $game->game_date - (60 * variable_get('pickem.cutoff_minutes', 15));
	}

	if (in_array($day, array('Thu', 'Fri'))) {
		return ($now < ($game->game_date - (60 * variable_get('pickem.cutoff_minutes', 15))));
	}
	return $now < $saturday_cutoff;
}

function pickem_is_locked_for_picks($week, $season) {
	if ($week == ($_SESSION['current_week']->week + 1) && $season == variable_get('pickem.current_season')) {
		return FALSE;
	}
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

function pickem_load_records_and_points_data($school_name, $season, $sport='ncaaf') {
	if (!is_array($school_name)) {
		$tmp = $school_name;
		$school_name = array($tmp);
		unset($tmp);
	}
	
	$data = array();
	foreach ($school_name as $s) {
		$data[$s] = array(
			'records' => array(),
			'points' => array(),
		);
	}
	
	$q = db_select('school_records', 'r')
		->fields('r')
		->condition('school_name', $school_name)
		->condition('season', $season)
		->condition('sport', $sport);
	$r = $q->execute();
	foreach ($r as $row) {
		$data[$row->school_name]['records'][$row->tag] = $row;
	}
	
	$q = db_select('school_points', 'p')
		->fields('p')
		->condition('school_name', $school_name)
		->condition('season', $season)
		->condition('sport', $sport);
	$r = $q->execute();
	foreach ($r as $row) {
		$data[$row->school_name]['points'][$row->tag] = $row;
	}
	
	return $data;
}
