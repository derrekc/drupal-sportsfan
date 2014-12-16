<?php

/**
 * Theme function that builds the render array for 
 * the primary Navigation bar that appears on the pick (slate) 
 * form
 */
function theme_pickemform_navbar($variables) {
	$pickem = $variables['pickem'];
	$pickem_player = $variables['pickem_player'];
	$week = $variables['week'];
	$related_nodes = $variables['related_nodes'];

	## extra navigation
	$nav = array();
	foreach ($pickem->slate_weeks as $_week => $w) {
		
		if ($week == $_week) {
			$nav[] = array(
				'data' => l(t($_week), 'pickem/' . $pickem->pid . '/slate/' . $_week),
				'class' => array('disabled'),
			);
		} else {
			$nav[] = l(t($_week), 'pickem/' . $pickem->pid .'/slate/' . $_week);
		}
	}

	$playerlist = array();
	$currentWeek = $pickem->currentWeek();
	$standings = $pickem->standings();
	
	if ($week == $pickem->currentWeek() && user_is_logged_in() && $pickem_player != NULL) {
		if (!empty($standings)) {
			foreach ($standings as $s_player) {
				if ($s_player->playername == $pickem_player->name) {
					$playerlist[] = array(
						'data' => l(t($s_player->playername), 
							'pickem/' . $pickem->pid .'/slate/' . $pickem->currentWeek() . '/compare/' . $pickem_player->uid . '/' . $s_player->uid),
						'class' => array('disabled'),
					);
				} else { 
					$playerlist[] = l(t($s_player->playername), 
						'pickem/' . $pickem->pid .'/slate/' . $pickem->currentWeek() . '/compare/' . $pickem_player->uid . '/' . $s_player->uid
					);
				}
			}
		}
	}
	
	$build = array();
	$build['navbar'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'class' => array('btn-group'),
			'style' => 'margin-bottom: 0.5em; margin-top: 1em;'
		),
	);
	$build['navbar']['weeks-nav'] = array(
		'#type' => 'container',
		'#attributes' => array(
			'class' => array('btn-group'),
		),
	);
	$build['navbar']['weeks-nav']['button'] = array(
		'#type' => 'markup',
		'#markup' => t('Select Week&nbsp;') . '<span class="caret"></span>',
		'#prefix' => '<button class="btn btn-default btn-primary dropdown-toggle" type="button" id="weekDropdownMenu" data-toggle="dropdown">',
		'#suffix' => '</button>',
	);
	$build['navbar']['weeks-nav']['list'] = array(
		'#theme' => 'item_list',
		'#items' => $nav,
		'#attributes' => array(
			'class' => array('dropdown-menu'),
			'role' => 'menu',
			'aria-labelledby' => 'weekDropdownMenu',
		),
	);
	if ($week == $pickem->currentWeek() && user_is_logged_in() && $pickem_player != NULL) {
		$build['navbar']['comparepicks'] = array(
			'#type' => 'container',
			'#attributes' => array(
				'class' => array('btn-group'),
			),
		);
		$build['navbar']['comparepicks']['player'] = array(
			'#type' => 'markup',
			'#markup' => t('Compare Picks With&nbsp;') . '<span class="caret"></span>',
			'#prefix' => '<button class="btn btn-info dropdown-toggle" type="button" id="comparePlayer" data-toggle="dropdown">',
			'#suffix' => '</button>',
		);
		if (!empty($playerlist)) {
			$build['navbar']['comparepicks']['list'] = array(
				'#theme' => 'item_list',
				'#items' => $playerlist,
				'#attributes' => array(
					'class' => array('dropdown-menu'),
					'role' => 'menu',
					'aria-labelledby' => 'comparePlayer',
				),
			);
		}
	}

	if (user_is_logged_in() && $pickem_player != NULL) {
		$build['navbar']['user-settings-link'] = array(
			'#type' => 'markup',
			'#markup' => sprintf(
				"<a class='btn btn-default' href=\"/user/%s/edit\">" . 
				"<span class='glyphicon glyphicon-cog'></span>&nbsp;" . 
				"<span>Change Your Default Pick Setting</span></a>", $pickem_player->uid),
		);
	}
	/*
	if ($related_nodes) {
		$node_items = array();
		foreach ($related_nodes as $n) {
			$node_items[] = l(t($n->title), '');
		}
		$build['navbar']['nodes-of-interest'] = array(
			'#theme' => 'item_list',
			'#items' => $node_items,
			'#attributes' => array(
				'class' => array('dropdown-menu')
			)
		);
	}
	*/
	return drupal_render($build);
}

function theme_pickem_slate_list($variables) {
	global $user;
	$pickem = $variables['pickem'];
	$pickem_player = $variables['pickem_player'];
	
	$pickem_slate = $pickem->getPickemSlate();
	$currentWeek = $pickem->currentWeek();
	
	$slate_rows = array();
	foreach ($pickem_slate as $slid => $slate_record) {
		if (!isset($slate_rows[$slate_record->week])) {
			$slate_rows[$slate_record->week] = 0;
		}
		$slate_rows[$slate_record->week]++;
	}
	
	$header = array(
		array('data' => t('Week'), 'class' => array('text-center')),
		array('data' => t('# Games'), 'class' => array('text-center')),
		array('data' => '<span title="Correct" class="glyphicon glyphicon-thumbs-up"></span>', 'class' => array('text-center')),
		array('data' => '<span title="Incorrect" class="glyphicon glyphicon-thumbs-down"></span>', 'class' => array('text-center')),
		array('data' => t('Rank'), 'class' => array('text-center')),
		array('data' => t('Actions'), 'class' => array('text-center')),
	);
	
	$rows = array();
	foreach ($slate_rows as $week => $event_count) {
		$pickem_result = $pickem_player->pickemResultForWeek($pickem->pid, $week);
		$player_rank = $pickem->playerRank($pickem_player->name, $week);
		
		$row_class = $week == $currentWeek ? 'bg-info' : '';
		$row = array(
			'data' => array(
				array('data' => $week, 'class' => array('text-center')),
				array('data' => $event_count, 'class' => array('text-center')),
				array('data' => $pickem_result ? $pickem_result->correct : t('&ndash;'), 'class' => array('text-center')),
				array('data' => $pickem_result ? $pickem_result->incorrect : t('&ndash;'), 'class' => array('text-center')),
				array('data' =>  $player_rank ? $player_rank : '&ndash;', 'class' => array('text-center')),
			),
			'class' => array($row_class)
		);
		
		$linkText = '';
		$btnClasses = array();
		if ($pickem->slateWeekIsOpen($week)) {	// was '$week == $currentWeek'
			$linkText = t('Make Picks');
			$btnClasses = array('btn', 'btn-primary', 'btn-xs');
		}
		else {
			$linkText = t('See Games');
			$btnClasses = array('btn', 'btn-default', 'btn-xs');
		}
		$row['data'][] = array(
			'data' => l(
				$linkText, 
				sprintf('pickem/%s/slate/%s', $pickem->pid, $week),
				array('attributes' => array('class' => $btnClasses))
			),
			'class' => array('text-center'),
		);
		$rows[] = $row;
		unset($linkText);
		unset($btnClasses);
	}
	
	$build['slate'] = array(
		'#theme' => 'table',
		'#header' => $header,
		'#rows' => $rows,
		'#attributes' => array(
			'class' => array('table', 'table-condensed', 'table-responsive'),
		),
	);
	
	return drupal_render($build);
	
}
