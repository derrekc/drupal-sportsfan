(function($){
	Drupal.behaviors.pickem = {
		attach : function(context, settings) {
			// do stuff!
			if (settings.pickem.provisioningNeeded == true) {
				// configure the provisioning trigger button in the modal
				
				$('#startProvisioning').on('click', function() {
					$('#provisioningProgress').show();
					$.ajax({
						dataType : 'json',
						url : settings.basePath + settings.pickem.provisioning_path,
						success : function(data, status, o) {
							$.each(data.selectors, function( index, value ) {
							  $(value)
							  	.removeClass('btn-default')
							  	.addClass('btn-info');
							});
							$('#provisioningProgress').hide();
							$('#provisioningModal').modal('hide');
						},
						error : function(o, status, error) {
							$('#provisioningProgress').hide();
							$('#provisioningModal').modal('hide');
						}
					})
				});
				
				$('#provisioningModal').on('show.bs.modal', function(e) {
					$('#provisioningProgress').hide();
				})
				$('#provisioningModal').modal('show');
			}
		}
	}
})(jQuery);

