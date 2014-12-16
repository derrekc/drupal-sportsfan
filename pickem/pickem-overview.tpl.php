<?php
	$currentWeek = $pickem->currentWeek();
	#$currentStandingsByPlayer = $pickem->standingsByPlayerName[$currentWeek - 1];
?>

<div class="row">
	<div class="col-sm-8">
		<h2>Welcome Back, <?php print $pickemPlayer->name; ?></h2>
		<span><strong>Current Standings: </strong></span>
			<?php $standingsByPlayerName = $pickem->standingsByPlayerName($pickemPlayer->name); print $standingsByPlayerName['overview-label']; ?>

		<h2>Week <?php print $currentWeek; ?> Is Open...</h2>
		<a class="btn btn-primary" href="/pickem/<?php print $pickem->pid; ?>/slate/<?php print $pickem->currentWeek(); ?>">Make/Review Your Picks</a>

		<?php if (!empty($playerStandings)): ?>
		<h2>Current Week Results</h2>
		<table class="table">
			<thead>
				<tr>
					<th width="50%" rowspan="2">&nbsp;</th>
					<th width="25%" colspan="2" class="text-right">Overall<sup>*</sup></th>
					<th width="25%" colspan="2" class="text-right">This Week</th>
				</tr>
				<tr>
					<th width="12.5%" class="text-right"><span title="Correct" class="glyphicon glyphicon-thumbs-up"></span></th>
					<th width="12.5%" class="text-right"><span title="Incorrect" class="glyphicon glyphicon-thumbs-down"></span></th>
					<th width="12.5%" class="text-right"><span title="Correct" class="glyphicon glyphicon-thumbs-up"></span></th>
					<th width="12.5%" class="text-right"><span title="Incorrect" class="glyphicon glyphicon-thumbs-down"></span></th>
				</tr>
			</thead>
		<?php foreach($currentWeekResults as $s): ?>
			<tr class="<?php print ($s['playername'] == $pickemPlayer->name) ? 'bg-info' : ''; ?>">
				<td><?php print $s['playername']; ?></td>
				<td class="text-right">
					<?php print $s['overall-correct']; ?>
				</td>
				<td class="text-right">
					<?php print $s['overall-incorrect']; ?>
				</td>
				<td class="text-right"><?php print $s['correct']; ?></td>
				<td class="text-right"><?php print $s['incorrect']; ?></td>
			</tr>
		<?php endforeach; ?>
		</table>		
		<?php endif; ?>
		
		<h2>Weekly Slate</h2>
		<?php print theme('pickem_slate_list', array('pickem_player' => $pickemPlayer, 'pickem' => $pickem)); ?>
		<?php if(!empty($standingsTopTen)): ?>
			
		<h2>Leaderboard</h2>
		<table class="table">
		<?php foreach($standingsTopTen as $s): ?>
			<tr class="<?php print ($s->playername == $pickemPlayer->name) ? 'bg-info' : ''; ?>">
				<td><?php print $s->playername; ?></td>
				<td class="text-right"><?php print $s->correct; ?></td>
				<td class="text-right"><?php print $s->incorrect; ?></td>
			</tr>
		<?php endforeach; ?>
		</table>
		<div id="disqus_thread"></div>
		<script type="text/javascript">
		    /* * * CONFIGURATION VARIABLES: EDIT BEFORE PASTING INTO YOUR WEBPAGE * * */
		    var disqus_shortname = 'sportsfandecronet'; // Required - Replace example with your forum shortname
		    var disqus_identifier = '<?php echo 'pickem-' . $pickem->name; ?>';
		    var disqus_url = '<?php echo url(NULL, array('absolute' => TRUE)) . 'pickem/' . $pickem->pid; ?>';
				
		    /* * * DON'T EDIT BELOW THIS LINE * * */
		    (function() {
		        var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;
		        dsq.src = '//' + disqus_shortname + '.disqus.com/embed.js';
		        (document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);
		    })();
		</script>
		<noscript>Please enable JavaScript to view the <a href="http://disqus.com/?ref_noscript">comments powered by Disqus.</a></noscript>
		<a href="http://disqus.com" class="dsq-brlink">blog comments powered by <span class="logo-disqus">Disqus</span></a>
		<?php endif; ?>
	</div>
	<div class="col-sm-4 hidden-sm hidden-xs">
		<div><?php print adsense_display(array('format' => '336x280', 'slot' => '3127734967')); ?></div>
		<div><?php print adsense_display(array('format' => '336x280', 'slot' => '8871387365')); ?></div>
	</div>
</div>

<div class="row">
	<div class="col-sm-6">
	</div>
	<div class="col-sm-6">		
	</div>
</div>

