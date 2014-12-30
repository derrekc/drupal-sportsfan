<div class="row">
	<p class="lead">Congratulations on joining the <?php print $pickem->title; ?> contest.</p>
	<p>Your initial picks for <?php print $section_title; ?> have been created based on the default settings for the contest.</p>  
	<p><strong>However</strong>, you will need to visit the pickem page for this section/slate and submit the picks.</p>
	<a class="btn btn-large btn-primary" href="/pickem/<?php print $pickem->pid; ?>/slate/<?php print $current_section; ?>">Submit Your Picks</a>
</div>