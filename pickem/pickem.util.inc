<?php

/**
 * @file pickem.util.inc
 * Utililty functions for the pickem module
 */

/**
 * Callback for Clearing User Pickem Session data
 * path => /pickem/clear-session
 * 
 * Read week and pickem params from query string
 */
function pickem_clear_pickem_session_data($pname = NULL, $week = NULL) {
	global $user;

	$querystr = $_SERVER['QUERY_STRING'];
	// @TODO parse qstring for week

	#unset($_SESSION['picks'][$user->uid]);
	#unset($_SESSION['picks']);
	#if (!is_null($pname)) {
	#	unset($_SESSION['picks'][$pname]);
	#}
	
	$cache_key = sprintf('picks.%s', $user->uid);
	cache_clear_all('picks*', 'cache', TRUE);
	
	drupal_goto('<front>');
}	
