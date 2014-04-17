<?php

?>
<div class="container-fluid">
	<?php foreach($entries as $row): ?>
	<div class="row">
	<div class="col-md-1"><?php print $row['gsx$week']; ?></div>
	</div>
	<?php endforeach; ?>
</div>