<?php

?>
<!-- Modal -->
<div class="modal fade" id="provisioningModal" tabindex="-1" role="dialog" aria-labelledby="provisionModalLabel" aria-hidden="true">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>
        <h4 class="modal-title" id="provisionModalLabel">Create Picks...?</h4>
      </div>
      <div class="modal-body">
      <p>Shall I initialize your picks for this week...?</p>
			<div id="provisioningProgress" class="progress progress-striped active">
			  <div class="progress-bar progress-bar-info" role="progressbar" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100" style="width: 100%">
			    <span class="sr-only">Please wait..</span>
			  </div>
			</div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
        <button id="startProvisioning" type="button" class="btn btn-primary">Yes, please</button>
      </div>
    </div>
  </div>
</div>