<div class="modal fade slate-pick-stats" 
	   id="slatePickStats"
		 tabindex="-1" 
		 role="dialog" 
		 aria-labelledby="myModalLabel" 
		 aria-hidden="true">
  <div class="modal-dialog">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
        <h4 class="modal-title" id="myModalLabel">Pick Breakdown</h4>
      </div>
      <div class="modal-body">
      	<p class="pleaseWaitP">Please wait...</p>
				<div class="progress progress-striped active">
				  <div class="progress-bar progress-bar-info" role="progressbar" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100" style="width: 100%">
				    <span class="sr-only">Please wait...</span>
				  </div>
				</div>
				<div class="statsView">
	      	
	      	<table class="table">
	      		<thead>
	      			<tr>
	      				<th colspan="2"><strong>Total Picks to-date: </strong><span class="total-picks"></span></th>
	      				<th class="text-right">Picks</th>
	      				<th class="text-right">%</th>
	      			</tr>
	      		</thead>
	      		<tbody>
	      			<tr><td class="visitor-logo" width="50"></td><td class="visitor"></td><td class="text-right visitor-pick-count"></td><td class="text-right visitor-percent-picked"></td></tr>
	      			<tr><td class="host-logo" width="50"></td><td class="host"></td><td class="text-right host-pick-count"></td><td class="text-right host-percent-picked"></td></tr>
	      		</tbody>
	      	</table>
	      	<h4>Who is picking:</h4>
	      	<ul class="nav nav-tabs" id="playerPicksTab" role="tablist">
	      		<li class="active"><a href="#visitorPicks" role="tab" data-toggle="tab" class="visitor-picks-label"></a></li>
	      		<li><a href="#hostPicks" role="tab" data-toggle="tab" class="host-picks-label"></a></li>
	      	</ul>
	      	<div class="tab-content">
	      		<div class="tab-pane visitor-picks active" id="visitorPicks"></div>
	      		<div class="tab-pane host-picks" id="hostPicks"></div>
	      	</div>
	      </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default btn-sm" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>
