<div class="row game-pick-row">
	<div class="hidden-xs col-md-2">
		<img height="32" src="<?php print base_path() . drupal_get_path('module', 'team') . '/logos/ncaaf/' . $slate_event['#event']->getSportsEvent()->visiting_team_name; ?>.gif" />
		<img height="32" src="<?php print base_path() . drupal_get_path('module', 'team') . '/logos/ncaaf/' . $slate_event['#event']->getSportsEvent()->host_team_name; ?>.gif" />
	</div>
	<div class="hidden-xs col-sm-4"><?php print render($slate_event['event_title']); ?></div>
	<div class="col-xs-4 visible-xs"><?php print render($slate_event['event_title_mobile']); ?></div>
	<div class="col-xs-6 col-md-5">
		<?php print render($slate_event['pick_team_name']); ?>
	</div>
	<div class="col-xs-1 col-md-1">
		<?php print render($slate_event['show-pick-stats']); ?>
	</div>
</div>