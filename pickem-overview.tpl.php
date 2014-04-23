<?php

?>

<div class="container">
	<div class="row">
		<div class="col-md-12">
			<h2>Welcome Back...</h2>
		</div>
	</div>
	<div class="row">
		<div class="col-md-6 col-xs-4">
			<h2>Upcoming Games To Pick</h2>
			<?php foreach($upcomingGames as $game): ?>
				<p><?php printf('%s vs %s', $game->visiting_school, $game->host_school); ?></p>
			<?php endforeach; ?>
			<a href="<?php print base_path(); ?>pickem/<?php print $pickem->name; ?>/slate" type="button" class="btn btn-primary">Make Your Picks</a>
		</div>
		<div class="col-md-6 col-xs-4">
			<?php if(isset($standingsTopTen)): ?>
			<h2>Standings</h2>
			<?php endif; ?>
		</div>
	</div>
</div>
