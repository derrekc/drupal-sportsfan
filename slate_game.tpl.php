<div class="row game-pick-row">
	<div class="hidden-xs col-md-2">
		<img height="32" src="<?php print base_path() . drupal_get_path('module', 'pickem') . '/logos/ncaaf/' . $slate_event['#event']->sportEvent()->visiting_school_name; ?>.gif" />
		<img height="32" src="<?php print base_path() . drupal_get_path('module', 'pickem') . '/logos/ncaaf/' . $slate_event['#event']->sportEvent()->host_school_name; ?>.gif" />
	</div>
	<div class="hidden-xs col-sm-4"><?php print render($slate_event['event_title']); ?></div>
	<div class="col-xs-4 visible-xs"><?php print render($slate_event['event_title_mobile']); ?></div>
	<?php foreach(element_children($slate_event['pick_school_name']) as $pick): ?>
		<div class="col-xs-4 col-md-3">
			<?php print render($slate_event['pick_school_name'][$pick]); ?>
		</div>
	<?php endforeach; ?>
</div>