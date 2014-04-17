<?php

?>
<div class="container-fluid">
	<?php foreach($entries as $row): ?>
	<div class="row">
		<div class="col-md-1"><?php print $row['gsx$week']['$t']; ?></div>
		<div class="col-md-3"><?php print $row['gsx$visitor']['$t']; ?></div>
		<div class="col-md-3"><?php print $row['gsx$host']['$t']; ?></div>
	</div>
	<?php endforeach; ?>
</div>