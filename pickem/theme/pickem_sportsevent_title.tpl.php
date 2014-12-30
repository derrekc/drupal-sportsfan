<?php
/*

			'#markup' => sprintf(
				'<div class="h5">%s<br /><small class="visible-xs">%s</small></div>', 
				$event->displayTitleWithLines(), 
				$event->getSportsEvent()->eventDateAndTV('sportsevent_date_medium')
				),
 * 
 */
?>
<div class="h5">
	<?php if ($sportsevent->visiting_team->conf_name == 'acc'): ?>
		<a href="/teams/<?php print $sportsevent->sport; ?>/<?php print $sportsevent->visiting_team->name; ?>">
	<?php endif; ?>
		<?php print $sportsevent->visiting_team->displaytitle;?>
	<?php if ($sportsevent->visiting_team->conf_name == 'acc'): ?>
		</a>
	<?php endif; ?>
	
	&nbps;<?php print $sportsevent->vs_at_label(); ?>
	
	<?php if ($sportsevent->host_team->conf_name == 'acc'): ?>
		<a href="/teams/<?php print $sportsevent->sport; ?>/<?php print $sportsevent->host_team->name; ?>">
	<?php endif; ?>
		<?php print $sportsevent->host_team->displaytitle;?>
	<?php if ($sportsevent->host_team->conf_name == 'acc'): ?>
		</a>
	<?php endif; ?>
	
	<?php if (!empty($sportsevent->betting_line)): ?>
		&nbsp;<?php print $sportsevent->betting_line; ?>
	<?php endif; ?>
	<br />
	<small class="visible-xs"><?php print $sportsevent->eventDateAndTV('sportsevent_date_medium'); ?></small>
</div>
