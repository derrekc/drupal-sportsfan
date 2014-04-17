<?php

class GoogleSpreadsheetManager {

	const GAMES_SPREADSHEET_KEY = '1M58ubV1mzlTaHWgs0YUIn_X3KRpx-M8zqAehG80lNVg';
	const SCHOOLS_SPREADSHEET_KEY = '1mp_TDlGqvrYZssn_1mU6hLfoekvnWeH-LsYHMRfax6o';
	const CONF_SPREADSHEET_KEY = '1zwJv7FCywucb2KidUzHtaXMfxzxtgpz15nT4xtssrQE';
	const WORKSHEET_URL_PIECE = '%s/feeds/worksheets/%s/public/basic%s';
	const LIST_URL_PIECE = '%s/feeds/list/%s/%s/public/values';

	const FEEDURL_PREFIX = 'https://spreadsheets.google.com';
	const QUERY_STRING = '?alt=json&amp;callback=displayContent';
	
	public static function listFeedFilter($var) {
		$r = preg_match('/listfeed/', $var['rel']);
		return $r == 1;
	}
	
	public static function worksheetUrls() {
		return array(
			'schools' => sprintf(self::WORKSHEET_URL_PIECE, 
				self::FEEDURL_PREFIX,
				self::SCHOOLS_SPREADSHEET_KEY,
				self::QUERY_STRING
			),
			'conferences' => sprintf(self::WORKSHEET_URL_PIECE, 
				self::FEEDURL_PREFIX,
				self::CONF_SPREADSHEET_KEY,
				self::QUERY_STRING
			),
			'games' => sprintf(self::WORKSHEET_URL_PIECE, 
				self::FEEDURL_PREFIX,
				self::GAMES_SPREADSHEET_KEY,
				self::QUERY_STRING
			),
		);
	}
	
	public static function sampleData() {
	
		// get data from google in this testing period
		$feed_url = sprintf(self::WORKSHEET_URL_PIECE, 
			self::FEEDURL_PREFIX,
			self::GAMES_SPREADSHEET_KEY,
			self::QUERY_STRING
			);

		$worksheet_content = json_decode(file_get_contents($feed_url, 0, self::googleFeedURLContext()), TRUE);
		$worksheetEntries = $worksheet_content['feed']['entry'];

		$schedEntries = self::gs_scheduleForTitleKey($worksheetEntries, 'acc', 1);
		return $schedEntries;
	}

	public static function fetchConferenceSpreadsheetEntries() {
		$listfeed_filter = function($var) {
			$r = preg_match('/listfeed/', $var['rel']);
			return $r == 1;
		};
		
		$feed_url = sprintf(self::WORKSHEET_URL_PIECE,
			self::FEEDURL_PREFIX,
			self::CONF_SPREADSHEET_KEY,
			self::QUERY_STRING
			);

		$worksheet_content = json_decode(file_get_contents($feed_url, 0, self::googleFeedURLContext()), TRUE);
		$worksheetEntry = $worksheet_content['feed']['entry'][0];
		$conferenceEntries = array();
		
		try {
			$listfeed_url = array_pop(array_filter($worksheetEntry['link'], $listfeed_filter));
			$list_query_url = str_replace('basic', 'values', sprintf("%s%s", $listfeed_url['href'], self::QUERY_STRING));

			$list_content = json_decode(file_get_contents($list_query_url, 0, self::googleFeedURLContext()), TRUE);
			$conferenceEntries = $list_content['feed']['entry'];
		} catch(Exception $e) {
			//
		}
		return $conferenceEntries;
	}
	
	public static function fetchSchoolsSpreadsheetEntries() {
		$listfeed_filter = function($var) {
			$r = preg_match('/listfeed/', $var['rel']);
			return $r == 1;
		};
		
		$feed_url = sprintf(self::WORKSHEET_URL_PIECE,
			self::FEEDURL_PREFIX,
			self::SCHOOLS_SPREADSHEET_KEY,
			self::QUERY_STRING
			);

		$worksheet_content = json_decode(file_get_contents($feed_url, 0, self::googleFeedURLContext()), TRUE);
		$worksheetEntry = $worksheet_content['feed']['entry'][0];
		$schoolEntries = array();
		
		try {
			$listfeed_url = array_pop(array_filter($worksheetEntry['link'], $listfeed_filter));
			$list_query_url = str_replace('basic', 'values', sprintf("%s%s", $listfeed_url['href'], self::QUERY_STRING));

			$list_content = json_decode(file_get_contents($list_query_url, 0, self::googleFeedURLContext()), TRUE);
			$schoolEntries = $list_content['feed']['entry'];
		} catch(Exception $e) {
			//
		}
		return $schoolEntries;
	}
	
	/**
	 * @return array (associative array, with 'conference' abbr names as keys)
	 */
	public static function fetchGamesSpreadsheetEntries() {
		$listfeed_filter = function($var) {
			$r = preg_match('/listfeed/', $var['rel']);
			return $r == 1;
		};

		// get data from google in this testing period
		$feed_url = sprintf(self::WORKSHEET_URL_PIECE, 
			self::FEEDURL_PREFIX,
			self::GAMES_SPREADSHEET_KEY,
			self::QUERY_STRING
			);

		$worksheet_content = json_decode(file_get_contents($feed_url, 0, self::googleFeedURLContext()), TRUE);
		$worksheetEntries = $worksheet_content['feed']['entry'];
		
		$gameEntries = array();
		
		foreach ($worksheetEntries as $ws) {
			$listfeed_urls = array_filter($ws['link'], $listfeed_filter);
			$list_query_url = str_replace('basic', 'values', sprintf("%s%s", $listfeed_urls[0]['href'], self::QUERY_STRING));

			$list_content = json_decode(file_get_contents($list_query_url, 0, self::googleFeedURLContext()), TRUE);
			$gameEntries[$ws['title']['$t']] = $list_content['feed']['entry'];
		}
		
		return $gameEntries;
	}
	
	public static function gs_scheduleForTitleKey($entries, $key, $week = NULL) {
		$GLOBALS['week'] = $week;
		$listfeed_filter = function($var) {
			$r = preg_match('/listfeed/', $var['rel']);
			return $r == 1;
		};
		$week_filter = function($var) {
			if (!is_null($GLOBALS['week']))
			return $var['gsx$week']['$t'] == $GLOBALS['week'];
		};
		$listfeed_urls = array();
		
		foreach ($entries as $ws) {
			$listfeed_urls = array_filter($ws['link'], $listfeed_filter);
			$list_query_url = str_replace('basic', 'values', sprintf("%s%s", $listfeed_urls[0]['href'], self::QUERY_STRING));

			$list_content = json_decode(file_get_contents($list_query_url, 0, self::googleFeedURLContext()), TRUE);
			if ($ws['title']['$t'] == $key) {
				dpm($list_content['feed']['entry']);
				$entries = $list_content['feed']['entry'];
				if (!is_null($week)) {
					$entries = array_filter($list_content['feed']['entry'], $week_filter);
				}
				return $entries;
			}
		}	
		return NULL;
	}

	public static function googleFeedURLContext() {
		// set timeout length
		$context = stream_context_create(
			array( 
		    'http' => array( 
					'timeout' => 5 
				) 
			) 
		); 

		return $context;
	}

}

