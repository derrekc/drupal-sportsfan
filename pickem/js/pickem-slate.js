(function($){
	Drupal.behaviors.pickem = {
		attach : function(context, settings) {
			
			$( '#submit-the-picks' ).click(function(evt) {
				$( '#submittingPicksModal' ).modal({backdrop: 'static'});
			})
			
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
			
			/**
			 * MODAL event handles 
			 */
			$('.pick-comments').on('shown.bs.modal', function (e) {
				charRemaining = 140 - $( '.comments-text', this ).val().length;
				$commentTextArea = $( '.char-counter', this );
				$( '.char-counter', this ).text(charRemaining + ' character(s) remaining');
				
				$( '.comments-text', this).on('keyup', function (e) {
					charRemaining = 140 - $( this ).val().length;
					$commentTextArea.text(charRemaining + ' character(s) remaining');
				});
			});
			
			$('.btn-primary', '.pick-comments').on('click', function (e) {
				$topLevelParent = $( this ).parents('.pick-comments').first();
				slate_event_id = $topLevelParent.attr('slate-event-id');
				
				$textarea = $( '.comments-text', $topLevelParent );
				
				url = '/pickem/pick/comment/add/' + slate_event_id;
				
				$.ajax({
					url : url,
					type : 'POST',
					success : function(data, status, o) {
						var $d = data;
						alert(data.response);
	
					},
					error : function(o, status, err) {
						var $d = err;
						
					},
					dataType : 'json',
					data : { comment : $textarea.val() }
				})
			});
			
			// toggle the display of the point spreads
			$('.ptspread-toggle').on('click', function(evt) {
				$('.ptspread').toggle();	
				return evt.preventDefault();
			});
			
			/**
			 * Prepare the Slate Pick Stats modal 
			 */
			$('#slatePickStats')
					.on('show.bs.modal', function (e) {
						
						var slateID = $( e.relatedTarget ).attr('target-entity-id');
						var pickemID = $( e.relatedTarget ).attr('pickem-id');
						var $modal = $( this );
						
						$( '#playerPicksTab a:first').tab('show');
						$( '.progress', $modal ).show();
						$( '.pleaseWaitP', $modal ).show();
						$(' .statsView', $modal ).hide();
						
						$.ajax({
							url : '/pickem/' + pickemID + '/slate/pick-stats/' + slateID,
							success : function(data, status, o) {
								$( '.total-picks', $modal ).text(data.pick_count );
								$( '.visitor', $modal ).text( data.visitor_name );
								$( '.visitor-pick-count', $modal ).text( data.visitor_pick_count );
								$( '.visitor-percent-picked', $modal ).text( data.visitor_pick_percent_format + '%' );
								$( '.visitor-picks-label', $modal ).text( data.visitor_name );
								$( '.visitor-picks', $modal ).html( data.visitor_picks_playername );
								
								$( '.host', $modal ).text( data.host_name );
								$( '.host-pick-count', $modal ).text( data.host_pick_count );
								$( '.host-percent-picked', $modal ).text( data.host_pick_percent_format + '%' );
								$( '.host-picks-label', $modal ).text( data.host_name );
								$( '.host-picks', $modal ).html( data.host_picks_playername );
								
								$visitor_logo = $( '<img />' )
									.attr('src', data.visitor_logo_url)
									.attr('height', '32');
								$( '.visitor-logo', $modal ).append( $visitor_logo );
								
								$host_logo = $( '<img />' )
									.attr('src', data.host_logo_url)
									.attr('height', '32');
								$( '.host-logo', $modal ).append( $host_logo );
								
								$( '.progress', $modal ).hide();
								$( '.pleaseWaitP', $modal ).hide();
								$( '.statsView', $modal ).show();
							}
						})
					})
					.on('hide.bs.modal', function (e) {
						var $modal = $( this );
						$( '.total-picks' ).text('');
						$( '.visitor', $modal ).text('');
						$( '.visitor-pick-count', $modal ).text('');
						$( '.visitor-percent-picked', $modal ).text('');
						$( '.visitor-logo', $modal ).text('');
						$( '.visitor-picks', $modal ).text( '' );
						$( '.visitor-picks-label', $modal ).text( '' );
						$( '.host', $modal ).text('');
						$( '.host-pick-count', $modal ).text('');
						$( '.host-percent-picked', $modal ).text('');
						$( '.host-logo', $modal ).text('');
						$( '.host-picks-label', $modal ).text( '' );
						$( '.host-picks', $modal ).text( '' );
					})
			;
			
			$('.scoretweet-trigger')
				.popover({
					containter : 'body',
					trigger : 'click focus'
				})
				.on('click', function(e) {
					return e.preventDefault();
				});
		}
	}
})(jQuery);

