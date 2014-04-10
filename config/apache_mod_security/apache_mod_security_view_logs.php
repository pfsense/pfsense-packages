<?php
/* $Id$ */
/*
	apache_mod_security_view_logs.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2009, 2010 Scott Ullrich <sullrich@gmail.com>
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
	if($_REQUEST['logtype'] == "error")
		$apachelogs = `cat /var/log/httpd-error.log`;
	else
		$apachelogs = `cat /var/log/httpd-access.log`;
	echo "</pre><h1>Apache+Mod_Security_Proxy Server logs as of " . date("D M j G:i:s T Y")  . "</h1><pre>\n\n";
	echo $apachelogs;
	exit;
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "Services: Mod_Security+Apache+Proxy: Logs";
include("head.inc");

/* XXX */
if ($_POST['clear']) { }

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<style type='text/css'>
pre {
 overflow-x: auto; /* Use horizontal scroller if needed; for Firefox 2, not needed in Firefox 3 */
 white-space: pre-wrap; /* css-3 */
 white-space: -moz-pre-wrap !important; /* Mozilla, since 1999 */
 white-space: -pre-wrap; /* Opera 4-6 */
 white-space: -o-pre-wrap; /* Opera 7 */
 /* width: 99%; */
 word-wrap: break-word; /* Internet Explorer 5.5+ */
}
</style>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
	<script type="text/javascript">
		function getlogactivity() {
			var url = "/apache_mod_security_view_logs.php";
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
			$('apachelogs').innerHTML = '<font face="Courier"><pre>' + transport.responseText  + '</pre></font>';
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
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Proxy Server Settings"), false, "/pkg_edit.php?xml=apache_mod_security_settings.xml&amp;id=0");
	$tab_array[] = array(gettext("Site Proxies"), false, "/pkg.php?xml=apache_mod_security.xml");	
	$tab_array[] = array(gettext("Logs"), true, "/apache_mod_security_view_logs.php");
	display_top_tabs($tab_array);
?>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
   <tr>
     <td class="tabcont" >
      <form action="apache_mod_security_view_logs.php" method="post">
		<br>
			<div id="apachelogs">
				<pre>One moment please, loading Apache logs...</pre>
			</div>
     </td>
    </tr>
</table>
<td align="left" valign="top">
	<form id="filterform" name="filterform" action="apache_mod_security_view_logs.php" method="post" style="margin-top: 14px;">
	<p/>
	<input id="submit" name="clear" type="submit" class="formbtn" value="<?=gettext("Clear log");?>" />
	</form>
</td>
</div>
<?php
	if($_REQUEST['logtype'] = "error") {
		echo "<br/>View <a href='apache_mod_security_view_logs.php?logtype=error'>error</a> logs";
	} else {
		echo "<br/>View <a href='apache_mod_security_view_logs.php'>access</a> logs";
	}
?>
<?php include("fend.inc"); ?>
</body>
</html>
