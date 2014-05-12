<?php foreach($pickems as $pickem): ?>
	<div class="col-md-12">

		<h3><?php print $pickem->title; ?></h3>
		<?php if (!empty($pickem->description)): ?>
			<p><?php print $pickem->description; ?></p>
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
