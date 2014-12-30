<div class="h4">
	<?php if ($completed): ?>
		<?php print $sportsevent->eventResult('long'); ?>
	<?php else: ?>
	
	<?php // VISITING TEAM with RANKING when applicable ?>
	<?php if (isset($visiting_team->rank[$week])): ?>
	<span class="poll-rank">#<?php print $visiting_team->rank[$week]; ?></span><?php endif; ?>
	<?php if ($sportsevent->visiting_conf_name == 'acc'): ?><?php print l(t($visiting_team->displaytitle), 'teams/' . $sport . '/' . $visiting_team->name); ?>
	<?php else: ?><?php print t($visiting_team->displaytitle); ?>
	<?php endif; ?>
	<?php print $vs_at_label; ?>
	<?php // HOST TEAM with RANKING when applicable ?>
	<?php if (isset($host_team->rank[$week])): ?><span class="poll-rank">#<?php print $host_team->rank[$week]; ?></span><?php endif; ?>
	<?php if ($sportsevent->host_conf_name == 'acc'): ?><?php print l(t($host_team->displaytitle), 'teams/' . $sport . '/' . $host_team->name); ?>
	<?php else: ?><?php print t($host_team->displaytitle); ?>
	<?php endif; ?>
	<?php if ($betting_line != ''): ?>
	<span class="ptspread">(<?php print sprintf("%s", $betting_line == '0' ? 'PK' : $betting_line); ?>)</span>
	<?php endif; ?>
	
	<?php endif; ?>
	
	<br />
	<?php if (!empty($sportsevent->title)): ?>
	<span class="small"><em><?php print $sportsevent->title; ?></em></span><br />
	<?php endif; ?>

	<?php if (!$sportsevent->completed()): ?>
		<small class="hidden-xs"><?php print $sportsevent->eventDateAndTV('sportsevent_date_medium'); ?></small>
	<?php else: ?>
		<small>&nbsp;</small>
	<?php endif; ?>
	<?php if (!empty($sportsevent->twitter_hashtag)): ?>
		<!--a href="#" tabindex="0" class="btn btn-xs btn-danger glyphicon glyphicon-bullhorn" data-toggle="popover" title="Tweet The Score" data-content="<?php print $sportsevent->tweetScoreMessageForPopup(); ?>"></a-->
	<?php endif; ?>
</div>