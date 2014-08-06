<?php

// 50 items per page.
define('TOTAL_SPORTSEVENT_PER_PAGE', 50);

class SportsEvent extends Entity {
	
	public function __construct($values = array()) {
		parent::__construct($values, 'sportsevent');

		// load the host school data
		$host_team = entity_load('team', FALSE, array('name' => $this->host_team_name));
		$this->host_team = reset($host_team);

		// load the visiting school data
		$visiting_team = entity_load('team', FALSE, array('name' => $this->visiting_team_name));
		$this->visiting_team = reset($visiting_team);
		
		if ($this->completed == 'Y') {
			if ($this->winning_team_name == $this->host_team_name) {
				$this->winning_team = $this->host_team;
				$this->final_score = sprintf("%s-%s", $this->host_score, $this->visiting_score);
			} else {
				$this->winning_team = $this->visiting_team;
				$this->final_score = sprintf("%s-%s", $this->visiting_score, $this->host_score);
			}
		}
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
		if (!empty($this->title)) {
			return $this->title;
		}
		return sprintf('%s %s %s',
			$this->visiting_team->displaytitle,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_team->displaytitle
			);
	}

	public function displayTitleWithLines() {
		if (empty($this->betting_line)) {
			return $this->displayTitle();
		}
		return sprintf('%s %s %s (%s)',
			$this->visiting_team->displaytitle,
			$this->neutral == 'Y' ? 'vs.' : 'at',
			$this->host_team->displaytitle,
			$this->betting_line
			);	
	}
	
	public function displayTitleMobile() {
		if (!empty($this->title)) {
			return $this->title;
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
	
	public function eventResult() {
		if (!$this->completed()) {
			return '';
		}
		
		$out = sprintf("%s, %s F", $this->winning_team->displaytitle, $this->final_score);
		if (!is_null($this->overtimes)) {
			if ($this->overtimes == '1') {
				$out .= " (OT)";
			} else {
				$out .= " (" . $this->overtimes . "OT)";
			}
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
}

class NCAAFSportsEventUIController extends EntityDefaultUIController {
	
}
