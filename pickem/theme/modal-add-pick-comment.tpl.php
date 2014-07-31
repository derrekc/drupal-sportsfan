<div class="modal fade pick-comments <?php print $modal_class; ?>" 
	   id="<?php print $modal_dialog_id; ?>"
		 tabindex="-1" 
		 role="dialog" 
		 aria-labelledby="myModalLabel" 
		 aria-hidden="true"
		 slate-event-id="<?php print $slate_event_id; ?>">
  <div class="modal-dialog modal-sm">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
        <h4 class="modal-title" id="myModalLabel">Comment on this Pick</h4>
      </div>
      <div class="modal-body">
      	<textarea name="pick-comment" class="form-control comments-text" rows="3"></textarea>
      	<p class="help-block"><span class="char-counter"></span></p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <?php if (!$disable_buttons): ?>
        	<button type="button" class="btn btn-primary">Save comment</button>
        <?php else: ?>
        	<button type="button" class="btn btn-primary" disabled="disabled">Save comment</button>
        <?php endif; ?>
      </div>
    </div>
  </div>
</div>
