<div class="row game-pick-row">
	<div class="col-xs-5 col-md-6"><?php print render($slate_event['event_title']); ?></div>
	<?php foreach(element_children($slate_event['pick_school_name']) as $pick): ?>
		<div class="col-xs-3 col-md-3">
			<?php print render($slate_event['pick_school_name'][$pick]); ?>
		</div>
	<?php endforeach; ?>
</div>		
