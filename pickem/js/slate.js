(function($){
	Drupal.behaviors.pickem = {
		attach: function(context, settings) {

			s = settings;
			if (s.colorbox != undefined) {
				return;
			}

			if (settings.pickem.pick_count > 0) {
				$.ajax({
					url : '/pickem/load/slate/stats/' + settings.pickem.uid,
					cache : false,
					success : function(data, txtstatus, jqxhr) {

						$('.pick-school-name').each(function(ndx, el) {
							elID = $(this).attr("id");
							
							pw_correct = "0";
							pw_incorrect = "0";
							pa_correct = "0";
							pa_incorrect = "0";
							
							if (s.pickem.games[elID]) {
								msg = '';
								if (s.pickem.games[elID].userpick_school != '') {
									console.log(s.pickem.games[elID].userpick_school);
									my_pick = s.pickem.games[elID].userpick_school;
									other_school = s.pickem.games[elID].other_school;
	
									pw_correct = data.player.school[my_pick].pw_correct;
									pw_incorrect = data.player.school[my_pick].pw_incorrect;
									if (pw_correct == "0" && pw_incorrect == "0") {
										msg += "<br /><small>1st time you have picked " + my_pick + "</small>";
									} else {
										msg += "<br /><small>" + pw_correct + '-' + pw_incorrect + ' when picking ' + my_pick + "</small>";
									}
	
									pa_correct = data.player.school[other_school].pa_correct;
									pa_incorrect = data.player.school[other_school].pa_incorrect;
									if (pa_correct == "0" && pa_incorrect == "0") {
										msg += "<br /><small>1st time you have picked against " + other_school + "</small>";
									} else {
										msg += "<br /><small>" + pa_correct + '-' + pa_incorrect + ' when picking against ' + other_school + "</small>";
									}
									$(this).after(msg);
								} else {
									$(this).after("<br /><small><em>&nbsp;</em></small>");
								}
							} else {
								$(this).after("<br /><small><em>&nbsp;</em></small>");
							}
						});
						debugger;
					}
				});
			}
		}
	}	
})(jQuery);



