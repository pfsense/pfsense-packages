<?php
/*
	varnishstat_view_logs.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2006 Scott Ullrich <sullrich@gmail.com>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

require("guiconfig.inc");

if($_REQUEST['getactivity']) {
	$varnishstatlogs = `tail -n 100  /var/log/zebedee.log`; 
	echo "<h2>Zebedee Server logs as of " . date("D M j G:i:s T Y")  . "</h2>";
	echo $varnishstatlogs;
	exit;
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "Zebedee: Logs";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
	<script type="text/javascript">
		function getlogactivity() {
			var url = "/zebedee_log.php";
			var pars = 'getactivity=yes';
			var myAjax = new Ajax.Request(
				url,
				{
					method: 'post',
					parameters: pars,
					onComplete: activitycallback
				});
		}
		function activitycallback(transport) {
			$('varnishstatlogs').innerHTML = '<font face="Courier"><pre>' + transport.responseText  + '</pre></font>';
			setTimeout('getlogactivity()', 2500);		
		}
		setTimeout('getlogactivity()', 1000);	
	</script>
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
<?php

$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=zebedee.xml&amp;id=0");
	$tab_array[] = array(gettext("Tunnels"), false, "/pkg_edit.php?xml=zebedee_tunnels.xml&amp;id=0");
	$tab_array[] = array(gettext("Keys"), false, "/zebedee_keys.php");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=zebedee_sync.xml&amp;id=0");
	$tab_array[] = array(gettext("View Configuration"), false, "/zebedee_view_config.php");
	$tab_array[] = array(gettext("View log files"), true, "/zebedee_log.php");
	display_top_tabs($tab_array);

?>
		</td></tr>
 		<tr>
    		<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
     						<td class="tabcont" >
      							<form action="zebedee_log.php" method="post">
								<div id="varnishstatlogs">
									<pre>One moment please, loading logs...</pre>
								</div>
     						</td>
						</tr>
					</table>
				</div>
			</td>
		</tr>
	</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
