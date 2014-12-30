<?php

/**
 * Available variable(s):
 * $standings - Array of standings for a given week
 */
?>

<table class="table complete-standings">
	<thead>
		<tr>
			<th>&nbsp;</th>
			<th><span>Player</span></th>
			<th class="text-right"><span class="glyphicon glyphicon-thumbs-up"></span></th>
			<th class="text-right"><span class="glyphicon glyphicon-thumbs-down"></span></th>
			<th class="text-right"><span>Weeks<br />Played</span></th>
			<th class="text-right"><span>Weeks<br />Won</span></th>
		</tr>
	</thead>
	<tbody>
		<?php foreach($standings as $s): ?>
		<?php $rowclass = $s->this_is_me ? 'bg-info' : ''; ?>
		<tr class="<?php print $rowclass; ?>">
			<td class="text-right"><span><?php print $s->display_rank; ?></span></td>
			<td><span><?php print !empty($s->picture) ? $s->picture : ''; ?><?php print $s->playername; ?></span></td>
			<td class="text-right"><span><?php print $s->correct; ?></span></td>
			<td class="text-right"><span><?php print $s->incorrect; ?></span></td>
			<td class="text-right"><span><?php print $s->weeks_played; ?></span></td>
			<td class="text-right"><span><?php print $s->weeks_won; ?></span></td>
		</tr>
		<?php endforeach; ?>
	</tbody>
	<tfoot>
		
	</tfoot>
</table>
