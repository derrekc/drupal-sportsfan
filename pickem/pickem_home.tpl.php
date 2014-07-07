<?php if (!$user_is_logged_in): ?>
	<div>
		<div class="alert alert-info">
			<h4>Join The Pickem's!</h4>
			<p>
				Please <a class="alert-link" href="/user/login">sign in</a> or <a class="alert-link" href="/user/register">register an account with us</a> so 
				you'll be able to join one (or more) of the Pickem contests.
			</p>
		</div>
	</div>
<?php endif; ?>
<?php print $pickem_content['active_pickems']; ?>

