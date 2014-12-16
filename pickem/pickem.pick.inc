<?php

function pickem_pick_add_comment($slate_id) {
	global $user;
	$comment = $_POST['comment'];
	
	$pick_comment_entity = entity_create('pick_comment', array('type' => 'pick_comment'));
	$pick_comment_entity->slid = $slate_id;
	$pick_comment_entity->uid = $user->uid;
	$pick_comment_entity->playername = $user->name;
	$pick_comment_entity->comment = $comment;
	
	$response = t('Your comment is saved.');
	if ($pick_comment_entity->save() === FALSE) {
		$response = t('Unable to save your comment.  Please try again, later');
	}
	
	$data = array(
		'message' => t('Hello World'),
		'response' => $response,
		'data' => array(1, 2, 3),
		'slate_id' => $slate_id,
		'pick-comment' => $comment,
	);
	return drupal_json_output($data);
}

function pickem_pick_comment_listing($pickem, $slate_event) {
	global $user;
	
	$pick_comments = $slate_event->comments();
	$html = '';
	foreach ($pick_comments as $pcomm) {
		$html .= sprintf("<div>%s</div>", $pcomm->comment);
	}
	return $html;
}

function pickem_pick_stats($pickem, $slate_event) {
	global $user;
	
	//watchdog('pickem', print_r($pickem, TRUE));
	//watchdog('pickem', print_r($slate_event, TRUE));
	// TODO this assumes two opponent sporting event
	// TODO FIX at some point
	$opponents = $slate_event->opponents();
	
	$results['pick_count'] = $slate_event->pickCount();
	$results['visitor_name'] = $opponents['visitor']->displaytitle;
	$results['visitor_pick_count'] = count($slate_event->picks_by_team[$opponents['visitor']->name]);
	$results['visitor_pick_percent'] = ($results['visitor_pick_count'] / $results['pick_count']) * 100;
	$results['visitor_pick_percent_format'] = number_format($results['visitor_pick_percent'], 0);
	$results['visitor_logo_url'] = base_path() . drupal_get_path('module', 'team') . '/logos/ncaaf/' . $slate_event->getSportsEvent()->visiting_team_name . '.gif';
	if (count($slate_event->picks_by_team[$opponents['visitor']->name])) {
		$results['visitor_picks_playername'] = implode('<br />', $slate_event->picks_by_team[$opponents['visitor']->name]);
	}
	else {
		$results['visitor_picks_playername'] = '-';
	}

	$results['host_name'] = $opponents['host']->displaytitle;
	$results['host_pick_count'] = count($slate_event->picks_by_team[$opponents['host']->name]);
	$results['host_pick_percent'] = ($results['host_pick_count'] / $results['pick_count']) * 100;
	$results['host_pick_percent_format'] = number_format($results['host_pick_percent'], 0);
	$results['host_logo_url'] = base_path() . drupal_get_path('module', 'team') . '/logos/ncaaf/' . $slate_event->getSportsEvent()->host_team_name . '.gif';
	if (count($slate_event->picks_by_team[$opponents['host']->name])) {
		$results['host_picks_playername'] = implode('<br />', $slate_event->picks_by_team[$opponents['host']->name]);
	}
	else {
		$results['host_picks_playername'] = '-';
	}
	
	return drupal_json_output($results);
}
