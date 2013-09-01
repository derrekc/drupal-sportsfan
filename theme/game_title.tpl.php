<?php

?>
<?php global $user; ?>

<div class="pickslate-entry">
	<div class="result-kickoff">
		<?php if($game->completed == 'Y'): ?>
			<span class="label">
			<?php print "FINAL"; ?>
			<?php if (!empty($game->title_data['overtimes'])): ?>
				<?php print ' - ' . $game->title_data['overtimes']; ?>
			<?php endif; ?>
			</span>
		<?php else: ?>
			<span><?php print $game->game_date_str . ', ' . $game->kickoff_time; ?> | <?php print $game->location; ?></span>
		<?php endif; ?>
	</div>
	<div class="opponent clearfix">
		<div class="logo border-hairline"><img src="<?php print $logobasepath . $game->title_data['visiting_school_name'] . '.gif'; ?>" width="35" /></div>
		<div class="school_title">
			<?php print $game->title_data['visiting_school']; ?>
			<span class="badge"><?php print $game->title_data['visiting_school_bline']; ?></span>
		</div>
		<div class="score text-right clearfix"><?php print $game->title_data['visiting_score']; ?></div>
	</div>
	<div class="opponent clearfix">
		<div class="logo border-hairline"><img src="<?php print $logobasepath . $game->title_data['host_school_name'] . '.gif'; ?>" width="35" /></div>
		<div class="school_title">
			<?php print $game->title_data['host_school']; ?>
			<span class="badge"><?php print $game->title_data['host_school_bline']; ?></span>
		</div>
		<div class="score text-right clearfix"><?php print $game->title_data['host_score']; ?></div>
	</div>
</div>

<?php if(!$game->pickIsOpen): ?>
<div class="pick-result text-right"><strong>Your Pick: </strong> 
	<?php print $game->userpick_school; ?>
	<?php if ($game->completed == 'Y'): ?>
	<span class="label <?php print $game->title_data['label_class']; ?>"><?php print $game->title_data['pick_result']; ?></span>
	<?php endif; ?>
</div>
<?php endif; ?>