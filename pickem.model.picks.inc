<?php

class PickemManager {
	
	public static function loadPickem($pid) {
		$pickem = drupal_static(__FUNCTION__);
		if (!isset($pickem)) {
			$cache_key = sprintf('pickem.%s.pickem', $pid);
			if ($cache = cache_get($cache_key)) {
				$pickem = $cache->data;
			}
			else {
				$dbres = db_select('pickem', 'p')->fields('p')->condition('pid', $pid)->execute();
				$pickem = $dbres->fetchObject();
				cache_set($cache_key, $pickem, 'cache', CACHE_TEMPORARY);
			}
		}
		return $pickem;
	}
	
	public static function loadPickemPlayer($uid) {
		$pickem_player = drupal_static(__FUNCTION__);
		if (!isset($pickem)) {
			$cache_key = 'pickem.pickem_player';
			if ($cache = cache_get($cache_key)) {
				$pickem_player = $cache->data;
			}
			else {
				$dbres = db_select('pickem_player', 'pp')->fields('pp')->condition('uid', $uid)->execute();
				$pickem_player = $dbres->fetchObject();
				cache_set($cache_key, $pickem_player, 'cache', CACHE_TEMPORARY);
			}
		}
		return $pickem_player;
	}
}