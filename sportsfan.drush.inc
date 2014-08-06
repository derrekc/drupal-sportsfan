<?php


/**
 * Implements hook_drush_command().
 */
function sportsfan_drush_command() {
  $items = array();
  $items['import-players'] = array(
    'callback' => 'sportsfan_import_players',
    'description' => dt('Import players from last season'),
		'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'options' => array(
			'pid' => 'Add users to this pickem (by pid)',
		),
		'aliases' => array('implayers'),
  );
  return $items;
}

function sportsfan_import_players() {
	$pid = drush_get_option('pid');
	$exclude = array('4x4hokies','admin','Buckminster Fuller','ChrisLords','ClairtonPanther','Collegecard','ecuacc4ever','ken d','Ole Blue','Otacon');
	db_set_active('sandbox');
	
	$users = db_select('users','u')
		->fields('u', array('name', 'mail', 'signature'))
		->condition('name', $exclude, 'NOT IN')
		->condition('uid', '0', '<>')
		->execute();
	
	$checkUsers = array();
	foreach ($users as $u) {
		drush_print($u->name . '; ' . $u->mail);
		$checkUsers[$u->name] = array(
			'name' => $u->name,
			'mail' => $u->mail,
			'signature' => $u->signature,
		);
	}
		
	db_set_active();
	drush_print_r($checkUsers);
	
	foreach ($checkUsers as $u) {
		$prompt = sprintf("Create account for %s (%s)...?", $u['name'], $u['mail']);
		if (drush_confirm($prompt)) {
			// create the drupal account
			
			$fields = array(
				'name' => $u['name'],
				'mail' => $u['mail'],
				'status' => 1,
				'pass' => 'changeme',
				'init' => $u['mail'],
				'signature' => $u['signature'],
				'roles' => array(
					DRUPAL_AUTHENTICATED_RID => 'authenticated user',
				),
			);
			$new_user = user_save(NULL, $fields);
			drush_print_r($new_user);
			if($new_user !== FALSE) {
				drush_log($new_user->name . ' has been added to the database.', 'ok');
				
				if (module_exists('pickem')) {
					$controller = entity_get_controller('pickem');
					drush_print(sprintf("adding %s | %s to pickem...", $new_user->uid, $new_user->name));
					$controller->addPlayer($new_user->uid, $new_user->name, '1');
				}
			}
		}
	}
}
