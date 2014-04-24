<?php foreach($pickems as $pickem): ?>
	<div class="col-md-12">
		<h3><?php print $pickem->title; ?></h3>
		<?php if (!empty($pickem->description)): ?>
			<p class=" bg-info"><?php print $pickem->description; ?></p>
		<?php endif; ?>
		<?php if ($pickem->user_can_play): ?>
			<a class="btn btn-primary" href="/pickem/<?php print $pickem->name; ?>">Overview</a>&nbsp;
			<a class="btn btn-default" href="/pickem/<?php print $pickem->name; ?>/slate">Play / Make Picks</a>&nbsp;
		<?php else: ?>
			<a class="btn btn-primary" href="/user">Join Today!</a>
		<?php endif; ?>
	</div>
<?php endforeach; ?>
