<?php

?>
<?php global $user; ?>

<div class="pickslate-entry">
	<div class="opponent clearfix">
		<div class="logo border-hairline"><img src="<?php print $logobasepath . $game->title_data['visiting_school_name'] . '.gif'; ?>" width="35" /></div>
		<div class="school_title">
			<?php if (empty($game->title_data['visiting_school'])): ?>
				<?php print $game->title_data['visiting_conf']; ?> - TBA		
				<br /><small>&nbsp;</small>	
			<?php else: ?>
				<?php print $game->title_data['visiting_school_rank']; ?><?php print $game->title_data['visiting_school']; ?>
				<span class="badge"><?php print $game->title_data['visiting_school_bline']; ?></span>
				<?php if (!empty($game->title_data['visiting_record'])): ?>
					<br />
					<small>
						<strong><?php print $game->title_data['visiting_record']['overall']; ?></strong> Overall;&nbsp;<?php print $game->title_data['visiting_record']['conf']; ?>
						<?php if (isset($game->title_data['visiting_record']['pythag'])): ?>
							<?php print ';&nbsp;' . $game->title_data['visiting_record']['pythag'] . ' <span title="Pythageon Adjusted Record">PA</span>'; ?>
						<?php endif; ?>
					</small>
				<?php else: ?>
					<br />
					<small><?php print "N/A"; ?></small>
				<?php endif; ?>
			<?php endif; ?>
		</div>
		<div class="score text-right clearfix"><?php print $game->title_data['visiting_score']; ?></div>
	</div>
	<div class="opponent clearfix">
		<div class="logo border-hairline"><img src="<?php print $logobasepath . $game->title_data['host_school_name'] . '.gif'; ?>" width="35" /></div>
		<div class="school_title">
			<?php if (empty($game->title_data['host_school'])): ?>
				<?php print $game->title_data['host_conf']; ?> - TBA
				<br /><small>&nbsp;</small>
			<?php else: ?>
				<?php print $game->title_data['host_school_rank']; ?><?php print $game->title_data['host_school']; ?>
				<span class="badge"><?php print $game->title_data['host_school_bline']; ?></span>
				<?php if (!empty($game->title_data['host_record'])): ?>
					<br />
					<small>
						<strong><?php print $game->title_data['host_record']['overall']; ?></strong> Overall;&nbsp;<?php print $game->title_data['host_record']['conf']; ?>
						<?php if (isset($game->title_data['host_record']['pythag'])): ?>
							<?php print ';&nbsp;' . $game->title_data['host_record']['pythag'] . ' <span title="Pythageon Adjusted Record">PA</span>'; ?>
						<?php endif; ?>
					</small>
				<?php else: ?>
					<br />
					<small><?php print "N/A"; ?></small>
				<?php endif; ?>
			<?php endif; ?>
		</div>
		<div class="score text-right clearfix"><?php print $game->title_data['host_score']; ?></div>
	</div>
	<div class="result-kickoff">
		<?php if($game->completed == 'Y'): ?>
			<span>
			<?php print "FINAL"; ?>
			<?php if (!empty($game->title_data['overtimes'])): ?>
				<?php print ' - ' . $game->title_data['overtimes']; ?>
			<?php endif; ?>
			</span>
		<?php else: ?>
			<span><?php print $game->game_date_str . ', ' . $game->kickoff_time; ?> | <?php print $game->location; ?></span>
		<?php endif; ?>
	</div>
</div>

<?php if($game->title_data['stats_data']): ?>
	<div id="stats-<?php print $game->visiting_school_name; ?>">
		<dt>Total PPG</dt>
		<dd><?php print $game->title_data['stats_data'][$game->visiting_school_name]['points']['total']->ppg; ?></dd>
	</div>
	<div id="stats-<?php print $game->host_school_name; ?>">
		<dt>Total PPG</dt>
		<dd><?php print $game->title_data['stats_data'][$game->host_school_name]['points']['total']->ppg; ?></dd>
	</div>
	<div class="btn-group">
		<a data-toggle="popover" 
			 data-placement="top" 
			 data-original-title="Stats" 
			 data-html="true" 
			 data-selector="#stats-<?php print $game->visiting_school_name; ?>"
			 class="btn btn-mini school-stats"><?php print $game->title_data['visiting_school']; ?> data</a>
			 
		<a data-toggle="popover" 
			 data-placement="top" 
			 data-original-title="Stats" 
			 data-html="true" 
			 data-selector="#stats-<?php print $game->visiting_school_name; ?>"
			 class="btn btn-mini school-stats"><?php print $game->title_data['host_school']; ?> data</a>
	</div>
<?php endif; ?>

<?php if(!$game->pickIsOpen): ?>
<!--div class="pick-result text-right">
	<?php if(is_null($other_player)): ?>
		<strong>Your Pick: </strong>
	<?php else: ?>
		<strong><?php print $other_player->player_name; ?>'s Pick: </strong>
	<?php endif; ?> 
	<?php print $game->userpick_school; ?>
	<?php if ($game->completed == 'Y'): ?>
	<span class="label <?php print $game->title_data['label_class']; ?>"><?php print $game->title_data['pick_result']; ?></span>
	<?php endif; ?>
</div-->
<?php endif; ?>