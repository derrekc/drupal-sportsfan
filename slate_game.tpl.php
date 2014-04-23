<div class="row game-pick-row">
	<div class="col-xs-2 col-md-2">
		<img height="30" src="<?php print base_path() . drupal_get_path('module', 'pickem') . '/logos/ncaaf/' . $slate_event['#event']->visiting_school_name; ?>.gif" />
		<img height="30" src="<?php print base_path() . drupal_get_path('module', 'pickem') . '/logos/ncaaf/' . $slate_event['#event']->host_school_name; ?>.gif" />
	</div>
	<div class="col-xs-3 col-md-4"><?php print render($slate_event['event_title']); ?></div>
	<?php foreach(element_children($slate_event['pick_school_name']) as $pick): ?>
		<div class="col-xs-3 col-md-3">
			<?php print render($slate_event['pick_school_name'][$pick]); ?>
		</div>
	<?php endforeach; ?>
</div>