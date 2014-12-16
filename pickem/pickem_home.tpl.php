<h2>Home of the College Pickem Contests</h2>
<?php if (!$user_is_logged_in): ?>
	<div class="panel panel-info">
	  <div class="panel-heading">
	    <h3 class="panel-title">Join The Pickem's!</h3>
	  </div>
	  <div class="panel-body">
			<p>
				Please <a class="alert-link" href="/user/login"><strong>sign in</strong></a> or <a class="alert-link" href="/user/register"><strong>register an account with us</strong></a> so 
				you'll be able to join one (or more) of the Pickem contests.
			</p>
	  </div>
	</div>
<?php endif; ?>
<div class="panel panel-info">
	<div class="panel-body">
	<?php print $pickem_content['active_pickems']; ?>
	</div>
</div>
