<button type="button" 
				data-toggle="modal" 
				data-target="<?php print $data_target ?>" 
				class="btn <?php print $btn_display_class; ?> <?php print $btn_size_class; ?>"   
				<?php if ($disabled): ?>disabled="disabled"<?php endif; ?>
				<?php if ($title): ?>title="<?php print $title; ?>"<?php endif; ?>
				pickem-id="<?php print $pickem_id; ?>"
				<?php if ($target_entity_id): ?>target-entity-id="<?php print $target_entity_id; ?>"<?php endif; ?>>
	<span class="glyphicon glyphicon-stats"></span>
</button>
