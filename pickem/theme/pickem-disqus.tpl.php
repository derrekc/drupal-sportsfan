<div id="disqus_thread"></div>
<script type="text/javascript">
    /* * * CONFIGURATION VARIABLES: EDIT BEFORE PASTING INTO YOUR WEBPAGE * * */
    var disqus_shortname = 'sportsfandecronet'; // Required - Replace example with your forum shortname
    var disqus_identifier = '<?php echo $pickem_disqus_id; ?>';
    <?php if(!empty($pickem_week)): ?>
    var disqus_url = '<?php echo url(NULL, array('absolute' => TRUE)) . 'pickem/' . $pickem_id . '/slate/' . $pickem_week; ?>';
    <?php else: ?>
    var disqus_url = '<?php echo url(NULL, array('absolute' => TRUE)) . 'pickem/' . $pickem_id; ?>';
		<?php endif; ?>
		
    /* * * DON'T EDIT BELOW THIS LINE * * */
    (function() {
        var dsq = document.createElement('script'); dsq.type = 'text/javascript'; dsq.async = true;
        dsq.src = '//' + disqus_shortname + '.disqus.com/embed.js';
        (document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(dsq);
    })();
</script>
<noscript>Please enable JavaScript to view the <a href="http://disqus.com/?ref_noscript">comments powered by Disqus.</a></noscript>
<a href="http://disqus.com" class="dsq-brlink">blog comments powered by <span class="logo-disqus">Disqus</span></a>