(function($){
	Drupal.behaviors.sscharts = {
		attach: function(context, settings) {
			// load json data for motion chart

			console.log('sschart js file loaded...');
			loadAjax = true;
			google.load('visualization', '1', {packages: ['motionchart']});
			
			function drawViz() {
				alert('drawVis');
			}
			
			google.setOnLoadCallback(drawViz);
			/*
			try {
				
			} catch(err) {
				console.log(err);
				loadAjax = false;
			}
			
			if (loadAjax) {
				
			}
			*/
		}
	}	
})(jQuery);
