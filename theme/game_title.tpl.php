<?php

?>
<h4>
	<?php print $game->visiting_school; ?>
	<?php if (!empty($game->bline) && ($game->visiting_school_name == $game->bline[0])): ?>
		<?php print ' (' . $game->bline[1] . ')'; ?>
	<?php endif; ?>
	<?php print $game->vs_at_symbol; ?>
	<?php print $game->host_school; ?>
	<?php if (!empty($game->bline) && ($game->host_school_name == $game->bline[0])): ?>
		<?php print ' (' . $game->bline[1] . ')'; ?>
	<?php endif; ?>
</h4>
<span><?php print $game->game_date_str; ?> | Kickoff: <?php print $game->kickoff_time; ?></span>
