<?php
/* $Id$ */
/*
	tinydns_view_logs.php
	part of pfSense (http://www.pfsense.com/)

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

/* Defaults to this page but if no settings are present, redirect to setup page */
if(!$config['installedpackages']['tinydns']['config'][0])
	Header("Location: /pkg_edit.php?xml=tinydns.xml&id=0");

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
	$one_two = true;

$pgtitle = "TinyDNS: View Logs";
include("head.inc");

/* NEED TO FIX there are 2 logs /etc/tinydns/log/main/current and /etc/dnscache/log/main/current */

$tinydnslogs = `cat /etc/tinydns/log/main/current | /usr/local/bin/tai64nlocal | php -f /usr/local/pkg/tinydns_parse_logs.php | grep -v ":0"`;

/* NEED TO FIX */
if ($_POST['clear']) {
//	exec("rm /etc/tinydns/log/main/current");
//	exec("touch /etc/tinydns/log/main/current");
}

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=tinydns.xml&id=0");
	$tab_array[] = array(gettext("Add/Edit Record"), false, "/tinydns_filter.php");
	$tab_array[] = array(gettext("Status"), false, "/tinydns_status.php");
	$tab_array[] = array(gettext("Logs"), true, "/tinydns_view_logs.php");
	$tab_array[] = array(gettext("Zone Sync"), false, "/pkg_edit.php?xml=tinydns_sync.xml&id=0");
	$tab_array[] = array(gettext("New domain wizard"), false, "/wizard.php?xml=new_zone_wizard.xml");		
	display_top_tabs($tab_array);
?>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
   <tr>
     <td class="tabcont" >
      <form action="tinydns_view_logs.php" method="post">
		<br>
<pre><?=$tinydnslogs?></pre>
     </td>
    </tr>
</table>
<td align="left" valign="top">
	<form id="filterform" name="filterform" action="tinydns_view_logs.php" method="post" style="margin-top: 14px;">
	<input id="submit" name="clear" type="submit" class="formbtn" value="<?=gettext("Clear log");?>" />
	</form>
</td>
</div>
<?php include("fend.inc"); ?>
<meta http-equiv="refresh" content="60;url=<?php print $_SERVER['SCRIPT_NAME']; ?>">
</body>
</html>
