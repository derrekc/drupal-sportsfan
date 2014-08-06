<?php

/**
 *@file
 *
 * Batch functions for admin tasks like opening and closing a pickem week are
 * stored here.
 */

function pickem_batch_clear_picks_for_week($pickem, $week, &$context) {
	$pickem_slate = $pickem->getPickemSlateForWeek($week);
	
	$num_deleted = db_delete('pick')
		->condition('slid', array_keys($pickem_slate))
		->execute();
	
	sleep(1);
		
	$context['results'][] = t('Cleared ' . $num_deleted . ' pick(s) for week ' . $week);
	$context['message'] = t('Cleared picks for week ' . $week);
	$context['finished'] = 1;
}

function pickem_batch_setup_player_for_week($pickem, $player, $week, &$context) {
	if (empty($context['sandbox'])) {
		$context['sandbox']['progress'] = 0;
		$context['sandbox']['max'] = 1;
		$context['sandbox']['current_player'] = $player;
	}
	
	$pickem_wrapper = entity_metadata_wrapper('pickem', $pickem);
	$max_bye_weeks_allowed = $pickem_wrapper->max_bye_weeks_allowed->value();
	$pick_default_pickem = $pickem_wrapper->pick_default->value();
	
	$player = pickem_player_load($player->uid);

	$player_user = user_load($player->uid);
	$user_wrapper = entity_metadata_wrapper('user', $player_user);
	$pick_default_user = $user_wrapper->pick_default->value();
	
	if (!empty($pick_default_user)) {
		$pick_default = $pick_default_user;
	}
	else {
		$pick_default = $pick_default_pickem;
	}
	
	#if ($player->name == 'ecuacc4ever') {
	#	$player->player_data['bye_count'] = $max_bye_weeks_allowed + 3;
	#}

	$slate = $pickem->getPickemSlateForWeek($week);
	$picksProcessed = 0;
	
	foreach ($slate as $slateEvent) {
		if ($slateEvent->load_pick_for_user($player->uid) === FALSE) {
			$picksProcessed++;
			
			$values = array(
				'slid' => $slateEvent->slid, 
				'playername' => $player->name,
				'pick_team_name' => '',
				'uid' => $player->uid,
			);
			
			if ($player->player_data['bye_count'] < $max_bye_weeks_allowed) {
				if ($pick_default == 'favorite') {
					if ($slateEvent->sportsEvent->hasFavorite()) {
						$values['pick_team_name'] = $slateEvent->sportsEvent->favorite;
					} else {
						// determine team with highest ranking
					}
				}
				
				// if 'pick_team_name' is still empty...?
				if (empty($values['pick_team_name'])) {
					$values['pick_team_name'] = ($pick_default == 'hometeam') 
						? $slateEvent->sportsEvent->host_team_name 
						: $slateEvent->sportsEvent->visiting_team_name;
				}
			}
	
			$pick = entity_create('pick', $values);
			$pick->save();
		}
	}
	
	if ($picksProcessed) {			
			$context['results'][] = sprintf("%s was provisioned successfully", check_plain($player->name));
		} else {
			$context['results'][] = sprintf("%s was previously provisioned", check_plain($player->name));
		}
	}
	
	// determine how many weeks this player missed.
	// if that player's 'bye' count is more than the pickem's max
	// create "empty" picks for the player.
	
	// otherwise, create picks based on the pickem default (home team, away team, highest rank)
  $context['sandbox']['progress']++;
  $context['message'] = 'Processed ' . $player->name;
	
	$context['finished'] = 1;
}

function pickem_batch_mark_pickem_week_open_status($pickem, $week, $status = 'Y', &$context) {
	db_update('week')
		->fields(array(
			'open' => $status,
		))
		->condition('week', $week)
		->condition('pickem_id', $pickem->pid)
		->execute();
		
	sleep(1);
		
	$context['results'][] = t(sprintf("Changed 'open' status to '%s' for week %s.", $status, $week));
	$context['message'] = t(sprintf("Changed 'open' status to '%s' for week %s.", $status, $week));
	$context['finished'] = 1;
}

function pickem_batch_setup_week_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One task processed.', '@count tasks processed.');
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
  // Providing data for the redirected page is done through $_SESSION.
  foreach ($results as $result) {
  	drupal_set_message($result);
    $items[] = t('Loaded players %title.', array('%title' => $result));
  }
  $_SESSION['my_batch_results'] = $items;

}
