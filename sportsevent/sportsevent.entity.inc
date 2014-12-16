<?php

// 50 items per page.
define('TOTAL_SPORTSEVENT_PER_PAGE', 50);

class SportsEvent extends Entity {
	
	static $tvNetwork = array(
		'espn' => 'ESPN',
		'espn2' => 'ESPN2',
		'espn3' => 'ESPN3',
		'espnu' => 'ESPNU',
		'espnews' => 'ESPNews',
		'espnnew' => 'ESPNews',
		'accn' => 'ACC Network',
		'fs1' => 'Fox Sports 1',
		'fs2' => 'Fox Sports 2',
		'fox' => 'Fox',
		'fsn' => 'Fox Sports Network',
		'rsn' => 'ACC RSN',
		'abc' => 'ABC',
		'cbs' => 'CBS',
		'cbssn' => 'CBS Sports Network',
		'nbc' => 'NBC',
		'mwc' => 'Mountain West Network',
		'sec' => 'SEC Network',
		'secn' => 'SEC Network',
		'btn' => 'B1G Network',
		'pac12' => 'PAC 12 Network',
	);
	
	static $ncaaScoreboardURLBase = array(
		'ncaaf' => 'http://www.ncaa.com/scoreboard/football/fbs/2014',
		'ncaam' => 'http://www.ncaa.com/scoreboard/basketball-men/d1',
	);
	
	public static function scoreboardUrls($sport, $week, $week_start, $game_date, $game_date_start) {
		$urls = array();
		$url_base = SportsEvent::$ncaaScoreboardURLBase[$sport];

		if ($sport == 'ncaaf') {
			if ($week_start) {
				for($i = (int)$week_start; $i <= (int)$week; $i++) {
					$new_url = $url_base . '/' . sprintf("%02d", $i);
					$urls[] = $new_url;
				}
			}
			else if ($week) {
				$urls[] = $url_base . '/' . sprintf("%02d", (int)$week);
			}
		}
		if ($sport == 'ncaam') {
			if ($game_date_start) {
				$t = $game_date_start;
				while ($t <= $game_date) {
					$urls[] = $url_base . '/' . date('Y/m/d', $t);
					$t += 86400;
				}
			}
			else if ($game_date) {
				$urls[] = $url_base . '/' . date('Y/m/d', $game_date);
			}
		}
		return $urls;
	}
	
	static $sport = array(
		'ncaaf' => 'College Football',
		'ncaam' => 'Men\'s College Basketball',
		'ncaaw' => 'Women\'s College Basketball',
	);
	
	public static function tvNetworkByAbbr($abbr) {
		$match = array();
		$pieces = array();
		
		preg_match('/^(?P<network1>\w+)((\sor\s)(?P<network2>\w+))?$/', $abbr, $match);
		
		$pieces[] = self::$tvNetwork[ $match['network1'] ];
		if (isset($match['network2']) && !empty($match['network2'])) {
			$pieces[] = self::$tvNetwork[ $match['network1'] ];
		}
		return implode(' or ', $pieces);
	}
	
	public function __construct($values = array()) {
		parent::__construct($values, 'sportsevent');
	}
	
	public function completed() {
		return in_array($this->completed, array('Y', 'y'));
	}

	public function title() {
		return $this->displayTitle();
	}

	public function getDisplayTitle() {
		return $this->displayTitle();
	}
	
	public function displayTitle() {
		# build a string
		$display_title = '';
		$display_title .= !empty($this->visiting_team_name) ? $this->visiting_team->displaytitle : 'TBA';
		$display_title .= ' ' . ('Y' ? 'vs.' : 'at') . ' ';
		$display_title .= !empty($this->host_team_name) ? $this->host_team->displaytitle : 'TBA';
		$display_title .= !empty($this->title) ? ' | (' . $this->title . ')' : '';
		return $display_title;
		
		/*
		if (!empty($this->title) && (empty($this->visiting_team_name) && empty($this->host_team_name))) {
			return $this->title;
		}
		if ($this->visiting_team && $this->host_team) {
		return sprintf('%s %s %s',
			$this->visiting_team->displaytitle,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_team->displaytitle
			);
		}
		return '';
		*/
	}
	
	public function tweetScoreMessageForPopup() {
		return sprintf("@sportsched %s %s|%s, 00-00", $this->twitter_hashtag, $this->visiting_team->name, $this->host_team->name);
	}
	
	public function displayTitleWithLines() {
		if ($this->completed()) {
			return $this->eventResult('long');
		}

		$str = '';
		if (isset($this->visiting_team->rank[$this->week])) {
			$str .= '<span class="poll-rank">' . '#' . $this->visiting_team->rank[$this->week] . '</span> ';
		}
		$str .= $this->visiting_team->displaytitle;
		$str .= $this->neutral == 'Y' ? ' vs. ' : ' at ';
		if (isset($this->host_team->rank[$this->week])) {
			$str .= '<span class="poll-rank">' . '#' . $this->host_team->rank[$this->week] . '</span> ';
		}
		$str .= $this->host_team->displaytitle;
		if ($this->betting_line != '') {
			$str .= sprintf(" <span class=\"ptspread\">(%s)</span>", $this->betting_line == '0' ? 'PK' : $this->betting_line);
		}
		
		return $str;
		
		//return sprintf('%s %s %s (%s)',
		//	$this->visiting_team->displaytitle,
		//	$this->neutral == 'Y' ? 'vs.' : 'at',
		//	$this->host_team->displaytitle,
		//	$this->betting_line
		//	);	
	}
	
	public function displayTitleMobile() {
		if ($this->completed()) {
			return $this->eventResult('long');
		}
		if (!empty($this->title)) {
			return $this->title;
		}
		$betting_line = trim($this->betting_line);
		if (!empty($betting_line)) {
			return sprintf('%s %s<br />%s (%s)',
				$this->visiting_team->displaytitle,
				$this->neutral == 'Y' ? 'vs.' : 'at',
				$this->host_team->displaytitle,
				$this->betting_line
				);

		}
		return sprintf('%s %s<br />%s',
			$this->visiting_team->displaytitle,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_team->displaytitle
			);
	}
		
	public function vs_at_label() {
		return $this->neutral == 'Y' ? 'vs.' : 'at';
	}
	
	public function gameDate($type='medium') {
		return $this->eventDate($type);
	}
	
	public function eventDate($type='medium') {
		return format_date($this->event_date, $type);
	}

	public function eventDateAndTV($dateType='medium') {
		if (empty($this->tv)) {
			return $this->eventDate($dateType);
		}
		return sprintf('%s - %s', $this->eventDate($dateType), SportsEvent::tvNetworkByAbbr($this->tv));
	}
	
	public function opponentsForSelectElement() {
		$options = array();
		if (!empty($this->visiting_team_name)) {
			$options[$this->visiting_team_name] = $this->visiting_team;
		}
		if (!empty($this->host_team_name)) {
			$options[$this->host_team_name] = $this->host_team;
		}
		return $options;
	}
	
	public function opponents() {
		$opponents = array();
		
		/**
		 * TODO include rank of school
		 */
		$o = new stdClass();
		$o->name = $this->visiting_team_name;
		$o->displaytitle = $this->visiting_team->displaytitle;
		$opponents['visitor'] = $this->visiting_team;
		
		$o = new stdClass();
		$o->name = $this->host_team_name;
		$o->displaytitle = $this->host_team->displaytitle;
		$opponents['host'] = $this->host_team;
		
		return $opponents;
	}
	
	public function opponentsAsArray() {
			
	}
	
	public function eventResult($format='short', $target_team_name = NULL) {
		if (!$this->completed()) {
			return '';
		}
		
		$out = '';
		if ($format == 'short') {
			$out = sprintf("%s, %s F", $this->winning_team->displaytitle, $this->final_score);
			if (!is_null($this->overtimes)) {
				if ($this->overtimes == '1') {
					$out .= " (OT)";
				} else {
					$out .= " (" . $this->overtimes . "OT)";
				}
			} 
		} elseif ($format == 'short-with-win-loss') {
			if (is_null($target_team_name)) {
				return '';
			}
			$scores = array($this->visiting_score, $this->host_score);
			$out = ($target_team_name == $this->winning_team_name) ? 'W' : 'L';
			$out .= '&nbsp;';
			$out .= max($scores) . '-' . min($scores);
			if (!is_null($this->overtimes)) {
				if ($this->overtimes == '1') {
					$out .= " (OT)";
				} else {
					$out .= " (" . $this->overtimes . "OT)";
				}
			} 
		}
		else {
			$out = sprintf("%s, %s",
				$this->winning_team_name == $this->visiting_team_name
					? sprintf("%s - %s", $this->visiting_team->displaytitle, $this->visiting_score)
					: sprintf("%s - %s", $this->host_team->displaytitle, $this->host_score),
				$this->winning_team_name == $this->visiting_team_name
					? sprintf("%s - %s", $this->host_team->displaytitle, $this->host_score)
					: sprintf("%s - %s", $this->visiting_team->displaytitle, $this->visiting_score)
				);
		}
		return $out;
	}
	
	public function hasFavorite() {
		return !empty($this->favorite);
	}
}

class SportsEventUIController extends EntityDefaultUIController {
	public function overviewForm($form, &$form_state) {
				
		$header = array(
			'host_team_name' => array('data' => t('Host'), 'field' => 'host_team_name'),
			'visiting_team_name' => array('data' => t('Visitor'), 'field' => 'visiting_team_name'),
			'sport' => array('data' => t('Sport'), 'field' => 'sport'),
			'event_date' => array('data' => t('Date'), 'field' => 'event_date'),
			'week' => array('data' => t('Week'), 'field' => 'week'),
			'operations' => array('data' => t('Operations'), 'field' => 'operations'),
		);
		
		$search_host_team_name = !empty($_POST['host_team_name']) ? $_POST['host_team_name'] : NULL;
		$search_visiting_team_name = !empty($_POST['visiting_team_name']) ? $_POST['visiting_team_name'] : NULL;
		$search_sport = !empty($_POST['sport']) ? $_POST['sport'] : NULL;
		
		$query = db_select('sportsevent', 'se')->extend('PagerDefault');
		$query->fields('se');
		$query->join('team', 'host_team', 'host_team.name = se.host_team_name');
		$query->join('team', 'visiting_team', 'visiting_team.name = se.visiting_team_name');
		
		if (!empty($search_host_team_name) || !empty($search_visiting_team_name) || !empty($search_sport)) {
			$master_or = db_or();
			$teams_or = db_or();
			
			if (!empty($search_host_team_name)) {
				$teams_or->condition(db_or()
					->condition('host_team.name', '%' . $search_host_team_name . '%', 'LIKE')
					->condition('host_team_name', '%' . $search_host_team_name . '%', 'LIKE')
					);
			}
			if (!empty($search_visiting_team_name)) {
				$teams_or->condition(db_or()
					->condition('visiting_team.name', '%' . $search_visiting_team_name . '%', 'LIKE')
					->condition('visiting_team_name', '%' . $search_visiting_team_name . '%', 'LIKE')
					);
			}
			$master_or->condition($teams_or);
			if ($search_sport) {
				$master_or->condition('sport', $search_sport);
			}
			$query->condition($master_or);
		}
		$query->orderBy('eid');
		$query->limit(TOTAL_SPORTSEVENT_PER_PAGE);
		
		dpm($query->__toString());
		/*
		$query = new EntityFieldQuery();
		$query
			->entityCondition('entity_type', 'sportsevent');
		if (!empty($host_team_name)) {
			$query->propertyCondition('home_team_name')
		}
		$query->extend(TOTAL_SPORTSEVENT_PER_PAGE);
		*/
			
		$result = $query->execute()->fetchAllAssoc('eid', PDO::FETCH_OBJ);

		$sportsevent_results = !empty($result) ? $result : array();
		$sportsevent_array = !empty($sportsevent_results) ? sportsevent_load_multiple(array_keys($sportsevent_results)) : array();
		$options = array();
		foreach ($sportsevent_array as $sportevent_id => $sportsevent) {
			$options[] = array(
				'host_team_name' => t($sportsevent->host_team->displaytitle),
				'visiting_team_name' => t($sportsevent->visiting_team->displaytitle),
				'sport' => t($sportsevent->sport),
				'event_date' => t(format_date($sportsevent->event_date, 'sportsevent_date')),
				'week' => t($sportsevent->week),
				'',
			);
		}
		
		$form['search'] = array(
			'#type' => 'fieldset',
			'#title' => t('Basic Search'),
			'#collapsible' => TRUE,
			'#collapsed' => !empty($visiting_team_name) || !empty($host_team_name) ? FALSE : TRUE,
		);
		$form['search']['host_team_name'] = array(
			'#type' => 'textfield',
			'#title' => t('Host Team'),
			'#default_value' => !empty($host_team_name) ? $host_team_name : '',
			'#size' => 20,
			'#attributes' => array('class' => array('sportsevent-admin-form-element')),
		);
		$form['search']['visiting_team_name'] = array(
			'#type' => 'textfield',
			'#title' => t('Visiting Team'),
			'#default_value' => !empty($visiting_team_name) ? $visiting_team_name : '',
			'#size' => 20,
			'#attributes' => array('class' => array('sportsevent-admin-form-element')),
		);
		$form['search']['actions'] = array(
			'#type' => 'actions',
		);
    $form['search']['actions']['search_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
    );
		
		$form['pager1'] = array('#theme' => 'pager');
		
		$form['entities'] = array(
			'#theme' => 'table',
			'#header' => $header,
			'#rows' => $options,
			'#attributes' => array('class' => array('entity-sort-table')),
			'#empty' => t('There are no Sporting Events'),
		);
		$form['pager2'] = array('#theme' => 'pager');
		return $form;
	}
}

class SportsEventAPIController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
	protected function attachLoad($sportsEvents, $revision_id = FALSE) {
		global $conf;
		#watchdog('SportsEvent attachLoad', print_r(array(array_keys($sportsEvents), debug_backtrace()), TRUE));
		
		foreach($sportsEvents as $eid => $event) {

			// load the host school data
			$sportsEvents[$eid]->host_team = FALSE;
			if (!empty($sportsEvents[$eid]->host_team_name)) {
				$host_team = entity_load('team', FALSE, array('name' => $event->host_team_name, 'sport' => $event->sport));
				$sportsEvents[$eid]->host_team = reset($host_team);
				if (isset($conf['current_pollid'][$event->sport][$event->season])) {
			
					$query = db_select('sportspoll_tally', 'st')
						->fields('st')
						->condition('team_name', $event->host_team_name)
						->condition('week', $event->week)
						->condition('season', $event->season)
						->condition('pollid', $conf['current_pollid'][$event->host_team->sport][$event->season])
						->condition('rank', 25, '<=');

					$result = $query->execute();
					if ($o = $result->fetchObject()) {
						$sportsEvents[$eid]->host_team->rank[$event->week] = $o->rank;
						#dpm(array('week' => $this->week, 'host_name' => $this->host_team->displaytitle, 'o' => $o));
					}
				}

			}

			$sportsEvents[$eid]->visiting_team = FALSE;
			if (!empty($sportsEvents[$eid]->visiting_team_name)) {
				// load the visiting school data
				$visiting_team = entity_load('team', FALSE, array('name' => $event->visiting_team_name, 'sport' => $event->sport));
				$sportsEvents[$eid]->visiting_team = reset($visiting_team);
				if (isset($conf['current_pollid'][$event->sport][$event->season])) {
					$query = db_select('sportspoll_tally', 'st')
						->fields('st')
						->condition('team_name', $event->visiting_team_name)
						->condition('week', $event->week)
						->condition('season', $event->season)
						->condition('pollid', $conf['current_pollid'][$event->sport][$event->season])
						->condition('rank', 25, '<=');
					$result = $query->execute();
					if ($o = $result->fetchObject()) {
						$sportsEvents[$eid]->visiting_team->rank[$sportsEvents[$eid]->week] = $o->rank;
					}
				}
			}
				
			// TODO incorporate SportsPoll Entity

			if ($sportsEvents[$eid]->completed == 'Y') {
				if ($sportsEvents[$eid]->winning_team_name == $sportsEvents[$eid]->host_team_name) {
					$sportsEvents[$eid]->winning_team = $sportsEvents[$eid]->host_team;
					$sportsEvents[$eid]->final_score = sprintf("%s-%s", $sportsEvents[$eid]->host_score, $sportsEvents[$eid]->visiting_score);
				} else {
					$sportsEvents[$eid]->winning_team = $sportsEvents[$eid]->visiting_team;
					$sportsEvents[$eid]->final_score = sprintf("%s-%s", $sportsEvents[$eid]->visiting_score, $sportsEvents[$eid]->host_score);
				}
			}
			
			$sportsEvents[$eid]->venue = FALSE;
			if ($sportsEvents[$eid]->venue_id != NULL && module_exists('venue')) {
				$sportsEvents[$eid]->venue = venue_load($sportsEvents[$eid]->venue_id);
			}

		}	
		
		parent::attachLoad($sportsEvents, $revision_id);	
	}
}

class NCAAFSportsEventUIController extends EntityDefaultUIController {
	
}
