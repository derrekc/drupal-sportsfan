(function($){
	Drupal.behaviors.pickem = {
		attach : function(context, settings) {
			// do stuff!
			$('input[type="text"], select', '#pickem-admin-week-games').change(function(){
				$parentTR = $(this).closest('TR');
				$('input.dirty-flag', $parentTR).attr('defaultValue', '1');
				$('input.dirty-flag', $parentTR).val('1');
				console.log($('input.dirty-flag', $parentTR).val());
			});
		}
	}
})(jQuery);

