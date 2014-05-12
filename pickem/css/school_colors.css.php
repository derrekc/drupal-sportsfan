<?php

header("Content-type: text/css");

?>
.sample-style { color: blue; }
<?php
if (function_exists('db_insert')) {
	echo '/* .db-insert {} */';
}
?>
