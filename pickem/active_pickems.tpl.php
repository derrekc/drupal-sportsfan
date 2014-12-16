<?php foreach($pickems as $pickem): ?>
	<div class="col-md-12">

		<h3><?php print $pickem->title; ?></h3>
		<?php if (!empty($pickem->description)): ?>
			<p><?php print $pickem->description; ?></p>
		<?php endif; ?>
		<?php if ($pickem->pid == 1): ?>
			<!--p>A message from the Pick'em Administrator (aka 2012 champ)</p>
			<iframe width="100%" height="166" scrolling="no" frameborder="no" src="https://w.soundcloud.com/player/?url=https%3A//api.soundcloud.com/tracks/162186886&amp;color=ff5500&amp;auto_play=false&amp;hide_related=false&amp;show_comments=true&amp;show_user=true&amp;show_reposts=false"></iframe-->
		<?php endif; ?>
		<?php
		/**
		 * Print the action buttons associated with this Pickem.
		 */
		?>
		<?php foreach ($pickem->ui_actions as $b): ?>
			<?php print render($b); ?>
		<? endforeach; ?>

	</div>
<?php endforeach; ?>
