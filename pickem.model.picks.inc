<?php

class PickemManager {

	const GAMES_SPREADSHEET_KEY = '1M58ubV1mzlTaHWgs0YUIn_X3KRpx-M8zqAehG80lNVg';
	const SCHOOLS_SPREADSHEET_KEY = '1mp_TDlGqvrYZssn_1mU6hLfoekvnWeH-LsYHMRfax6o';
	const WORKSHEET_URL_PIECE = '%s/feeds/worksheets/%s/public/basic%s';
	const LIST_URL_PIECE = '%s/feeds/list/%s/%s/public/values';

	const FEEDURL_PREFIX = 'https://spreadsheets.google.com';
	const QUERY_STRING = '?alt=json&amp;callback=displayContent';
	
	public static function sampleData() {

		// set timeout length
		$context = stream_context_create(
			array( 
		    'http' => array( 
					'timeout' => 5 
				) 
			) 
		); 
	
		// get data from google in this testing period
		$feed_url = sprintf(self::WORKSHEET_URL_PIECE, 
			self::FEEDURL_PREFIX,
			self::GAMES_SPREADSHEET_KEY,
			self::QUERY_STRING
			);

		$worksheet_content = json_decode(file_get_contents($feed_url, 0, $context), TRUE);
		$worksheetEntries = $worksheet_content['feed']['entry'];

		$schedKey = self::gs_scheduleForTitle($worksheetEntries, 'acc');
	}

	public static function gs_scheduleForTitle($entries, $key) {
		$filter = function($var) {
			$r = preg_match('/listfeed/', $var['rel']);
			return $r == 1;
		};

		$listfeed_urls = array();
		foreach ($entries as $ws) {
			$listfeed_urls = array_filter($ws['link'], $filter);
			$list_query_url = str_replace('basic', 'values', sprintf("%s%s", $listfeed_urls[0]['href'], self::QUERY_STRING));

			dpm($list_query_url);

			$lists_content = json_decode(file_get_contents($list_query_url, 0, self::googleFeedURLContext()), TRUE);
			if ($ws['title']['$t'] == $key) {
				dpm($lists_content);
			}
		}		
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