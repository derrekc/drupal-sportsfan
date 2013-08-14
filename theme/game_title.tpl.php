<?php

?>
<h4>
	<?php print $game->visiting_school; ?>
	<?php if (!$game->host_is_favored): ?>
		<?php print ' (' . substr($game->betting_line, 2) . ')'; ?>
	<?php endif; ?>
	<?php if ($game->neutral): ?>
		<?php print ' vs. '; ?>
	<?php else: ?>
		<?php print ' at '; ?>
	<?php endif; ?>
	<?php print $game->host_school; ?>
	<?php if ($game->host_is_favored): ?>
		<?php print ' (' . substr($game->betting_line, 2) . ')'; ?>
	<?php endif; ?>
</h4>
<span><?php print $game->game_date_str; ?> | Kickoff: <?php print $game->kickoff_time; ?></span>
