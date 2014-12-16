<?php

/**
 *@file
 *
 * Batch functions for admin tasks like opening and closing a pickem week are
 * stored here.
 */

function pickem_batch_clear_picks_for_week($pickem_id, $week, &$context) {
	$pickems = entity_load('pickem', array($pickem_id));
	$pickem = reset($pickems);

	$pickem_slate = $pickem->getPickemSlateForWeek($week);
	
	$num_deleted = db_delete('pick')
		->condition('slid', array_keys($pickem_slate))
		->execute();
	
	sleep(1);
		
	$context['results'][] = t('Cleared ' . $num_deleted . ' pick(s) for week ' . $week);
	$context['message'] = t('Cleared picks for week ' . $week);
	$context['finished'] = 1;
}

function pickem_batch_setup_player_for_week($pickem_id, $player, $week, &$context) {
	$pickems = entity_load('pickem', array($pickem_id));
	$pickem = reset($pickems);

	if (empty($context['sandbox'])) {
		$context['sandbox']['progress'] = 0;
		$context['sandbox']['max'] = 1;
		$context['sandbox']['current_player'] = $player;
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
							
			// TODO move the above code to the model (Pickem or Pick)
			$values['pick_team_name'] = $pickem->determineDefaultPickTeamForPlayer($player, $slateEvent);
			$context['results'][] = 'Pick default is : ' . $values['pick_team_name'];
			//$context['results'][] = 'Pick that would have been, or is created -- ' . print_r($values, TRUE);
			$pick = entity_create('pick', $values);
			$pick->save();
		}
	}
	
	if ($picksProcessed) {			
		$context['results'][] = sprintf("%s was provisioned successfully", check_plain($player->name));
	} else {
		$context['results'][] = sprintf("%s was previously provisioned", check_plain($player->name));
	}
	
	// determine how many weeks this player missed.
	// if that player's 'bye' count is more than the pickem's max
	// create "empty" picks for the player.
	
	// otherwise, create picks based on the pickem default (home team, away team, highest rank)
  $context['sandbox']['progress']++;
  $context['message'] = 'Processed ' . $player->name;
	
	$context['finished'] = 1;
}

function pickem_batch_mark_pickem_week_open_status($pickem_id, $week, $status = 'Y', &$context) {
	$pickems = entity_load('pickem', array($pickem_id));
	$pickem = reset($pickems);

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

function pickem_batch_reset_standings($pickem, &$context) {
	$num_deleted = db_delete('standings')
		->condition('pickem_id', $pickem->pid)
		//->condition('week', '-1')
		->execute();
		
	$context['results'][] = t('Cleared the standings for: ' . $pickem->title);
	$context['message'] = t('Cleared the standings for ' . $pickem->title);
	$context['finished'] = 1;
}

function pickem_batch_create_standings_for_week($pickem_id, $week, &$context) {
	$pickems = entity_load('pickem', array($pickem_id));
	$pickem = reset($pickems);
	
	$affected_players = $pickem->playersWithSubmittedPicksForWeek($week);
	$standings = array();
	
	$query = new EntityFieldQuery();
	$query
		->entityCondition('entity_type', 'standings', '=')
		->propertyCondition('pickem_id', $pickem->pid, '=')
		->propertyCondition('week', $week - 1, '=')
		->propertyOrderBy('week', 'DESC');
		
	$result = $query->execute();
	$player_to_doublecheck = array();
	if (!empty($result['standings'])) {
		// make a copy of each standings row 
		$prev_standings = standings_load_multiple(array_keys($result['standings']));
		foreach ($prev_standings as $prev) {
			drush_print('processing ' . $prev->playername);
			$player_result = $pickem->player_results_by_uid[$week][$prev->uid];
			if ($player_result == NULL) {
				$player_to_doublecheck[] = $prev->playername;
				continue;
			}
			drush_print_r($player_result);
			$player = $pickem->playersByUID[$player_result->uid];
			
			$newrow = Standings::newStandingsRowForPickemPlayer($pickem, $player, $week);
			#$pickem_result = $player->pickemResultForWeek($pickem->pid, $week);
			$newrow->correct = $prev->correct + $player_result->correct;
			$newrow->incorrect = $prev->incorrect + $player_result->incorrect;
			$newrow->save();
			$context['results'][] = t(sprintf("++Standings Record for %s: %s-%s", $newrow->playername, $newrow->correct, $newrow->incorrect));
			
			$player_result->completed = 'Y';
			$player_result->save();
		}
		drush_print_r($player_to_doublecheck);
	}
	else {
		// create new standings
		foreach ($affected_players as $p) {
			// create standings entity instance
			$standings[$p->uid] = Standings::newStandingsRowForPickemPlayer($pickem, $p, $week);
			$pickem_result = $p->pickemResultForWeek($pickem->pid, $week);
			if ($pickem_result) {
				$standings[$p->uid]->correct = $pickem_result->correct;
				$standings[$p->uid]->incorrect = $pickem_result->incorrect;
				//$standings[$p->uid]->pct = number_format((float) ($pickem_result->correct / ($pickem_result->correct + $pickem_result->incorrect)) * 100.0, 3);
				$context['results'][] = t(sprintf("Standings Record for %s: %s-%s", $standings[$p->uid]->playername, $standings[$p->uid]->correct, $standings[$p->uid]->incorrect));
				$standings[$p->uid]->save();
				
				// mark the pickem_result as completed, too
				$pickem_result->completed = 'Y';
				$pickem_result->save();
			}
		}
	}
}

function pickem_batch_close_player_for_week($pickem_id, $player, $week, &$context) {
	$pickems = entity_load('pickem', array($pickem_id));
	$pickem = reset($pickems);
	
	// if a player doesn't have an entry for his/her week "metadata"
	// then increase his/her 'bye_count' in the metadata
	//if ($week_metadata === FALSE || empty($week_metadata->last_savepick)) {
	//$week_metadata = $player->weekMetadataForPickem($pickem->pid);
	
	if (!$player->submittedPicksForWeek($pickem->pid, $week)) {
		$bye_count = (int) $player->metadataValue('bye_count', $pickem->pid, 0) + 1;
		$controller = entity_get_controller('pickem_player');
		$context['results'][] = t($player->name . ' did not submit his/her picks and will be assessed a "Bye Week"');
		$context['results'][] = $controller->addMetaData($player, $pickem->pid, 'bye_count', $bye_count);	// TRUE = 'test only'
	}
	else {
		$context['results'][] = t($player->name . ' submitted picks this week');
	}
	return;
}
