<?php

// 50 items per page.
define('TOTAL_ITEMS_PER_PAGE', 50);

// Admin uri links.
define('ADMIN_CONTENT_TEAMS_MANAGE_URI', 'admin/content/teams/manage/');
define('ADMIN_CONTENT_TEAMS_URI', 'admin/content/teams');

class Team extends Entity {
	
	public $title;
	public $displaytitle;
	public $league;
	public $active;

	public function __construct($values = array()) {
		parent::__construct($values, 'team');
	}
}	

class TeamController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}
		
/**
 * Custom controller for the administrator UI
 */
class TeamUIController extends EntityDefaultUIController {
	
	/**
	 * Override the menu hook for default ui controller
	 */
	public function hook_menu() {
		$items = parent::hook_menu();
		$items[$this->path]['title'] = t('Teams');
		$items[$this->path]['description'] = t('Manage Teams, including fields');
    $items[$this->path]['access callback'] = 'team_access_callback';
    $items[$this->path]['access arguments'] = array('administer team entities');
    $items[$this->path]['type'] = MENU_LOCAL_TASK;
    return $items;
	}

  /**
   * Admin form for searching and doing bulk operations.
   */
  public function overviewForm($form, &$form_state) {
    $form['pager'] = array('#theme' => 'pager');

    $header = array(
      'id' => array('data' => t('Team ID'), 'field' => 'id'),
      'title' => array('data' => t('Title'), 'field' => 'title'),
      'operations' => array('data' => t('Operations'), 'field' => 'operations'),
    );

    $options = array();
    $search_term = !empty($_GET['search']) ? $_GET['search'] : NULL;

    $query = new EntityFieldQuery();
    $query
      ->entityCondition('entity_type', 'team');

    if (!empty($search_term)) {
      $query->propertyCondition('name', '%' . $search_term . '%', 'like');
    }
    // Check for sort order and sort key.
    if (!empty($_GET['sort']) && !empty($_GET['order'])) {
      $sort = strtoupper($_GET['sort']);
      $order = strtolower($_GET['order']);
      $order = str_replace(' ', '_', $order);
      if ($order != 'operations') {
        $query->propertyOrderBy($order, $sort);
      }
    }

    $query->pager(TOTAL_ITEMS_PER_PAGE);

    $result = $query->execute();
    $team_results = !empty($result['team']) ? $result['team'] : array();
    $team_array = !empty($team_results) ? team_load_multiple(array_keys($team_results)) : array();
    foreach ($team_array as $team_id => $team) {
      $options['team_id-' . $team_id] = array(
        'id' => l($team->id, 'team/' . $team->id),
        'title' => l($team->title, 'team/' . $team->id),
        'operations' =>
        l(t('View'), 'team/' . $team->id) . ' ' .
        l(t('Edit'), ADMIN_CONTENT_TEAMS_MANAGE_URI . $team_id, array('query' => array('destination' => ADMIN_CONTENT_TEAMS_URI))) . ' ' .
        l(t('Delete'), ADMIN_CONTENT_TEAMS_MANAGE_URI . $team_id . '/delete', array('attributes' => array('class' => array('team-delete-' . $team->id), ), 'query' => array('destination' => ADMIN_CONTENT_TEAMS_URI))),
      );
    }

    $form['search'] = array(
      '#type' => 'fieldset',
      '#title' => t('Basic Search'),
      '#collapsible' => TRUE,
      '#collapsed' => !empty($search_term) ? FALSE : TRUE,
    );

    $form['search']['search_text'] = array(
      '#type' => 'textfield',
      '#title' => t('Search Term'),
      '#default_value' => !empty($search_term) ? $search_term : '',
    );

    $form['search']['search_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
    );

    $form['bulk_operations'] = array(
      '#type' => 'fieldset',
      '#title' => t('Bulk Operations'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['bulk_operations']['operations'] = array(
      '#type' => 'select',
      '#options' => array(
        0 => t('Select a bulk operation'),
        'delete' => t('Delete selected teams'),
      ),
    );

    $form['bulk_operations']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );

    $form['entities'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#attributes' => array('class' => array('entity-sort-table')),
      '#empty' => t('There are no teams.'),
    );

    return $form;
  }

  /**
   * Form Submit method.
   */
  public function overviewFormSubmit($form, &$form_state) {
    $values = $form_state['input'];
    $team_ids = array();

    if (!empty($values['entities'])) {
      foreach ($values['entities'] as $index => $value) {
        if (!empty($value)) {
          $team_ids[] = str_replace('team_id-', '', $value);
        }
      }

      switch ($values['operations']) {
        case 'delete':
          drupal_goto('admin/content/team/bulk/delete/' . implode('|', $team_ids));
          break;
      }
    }

    if (!empty($values['search_text'])) {
      drupal_goto('admin/content/team', array('query' => array('search' => $values['search_text'])));
    }
  }
}

/*------------ CONFERENCE -------------*/
class Conference extends Entity {
	public function __construct($values = array()) {
		parent::__construct($values, 'conference');
	}	
}

class ConferenceController extends EntityAPIController {
	
	public function load($ids = array(), $conditions = array()) {
		return parent::load($ids, $conditions);
	}	
	
}

class ConferenceUIController extends EntityDefaultUIController {
	
}
