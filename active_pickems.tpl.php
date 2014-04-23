<?php foreach($pickems as $pickem): ?>
	<div class="col-md-12">
		<h3><?php print $pickem->title; ?></h3>
		<?php if (!empty($pickem->description)): ?>
			<p class=" bg-info"><?php print $pickem->description; ?></p>
		<?php endif; ?>
	</div>
<?php endforeach; ?>
