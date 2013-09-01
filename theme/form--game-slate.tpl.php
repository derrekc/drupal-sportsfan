<?php print drupal_render($form['schools']); ?>
<?php unset($form['schools']); ?>
<hr />
<?php print drupal_render_children($form); ?>

