<?php
/* $Id$ */
/*
    autoconfigbackup_backup.php
    Copyright (C) 2008 Scott Ullrich
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

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2")) 
	require("crypt_acb.php");

// Seperator used during client / server communications
$oper_sep			= "\|\|";

// Encryption password 
$decrypt_password 	= $config['installedpackages']['autoconfigbackup']['config'][0]['crypto_password'];

// Defined username
$username			= $config['installedpackages']['autoconfigbackup']['config'][0]['username'];

// Defined password
$password			= $config['installedpackages']['autoconfigbackup']['config'][0]['password'];

// URL to restore.php
$get_url			= "https://{$username}:{$password}@portal.pfsense.org/pfSconfigbackups/restore.php";

// URL to delete.php
$del_url			= "https://{$username}:{$password}@portal.pfsense.org/pfSconfigbackups/delete.php";

// Set hostname
$hostname			= $config['system']['hostname'] . "." . $config['system']['domain'];

if(!$username) {
	Header("Location: /pkg_edit.php?xml=autoconfigbackup.xml&id=0");
	exit;
}

if($_POST) {
	touch("/tmp/acb_nooverwrite");
	if($_REQUEST['reason']) 
		write_config($_REQUEST['reason']);
	else 
		write_config("Backup invoked via Auto Config Backup.");
	$savemsg = "Backup completed successfully.";
	exec("echo > /cf/conf/lastpfSbackup.txt");
	filter_configure_sync();
	print_info_box($savemsg);	
	$donotshowheader=true;
}

$pgtitle = "Diagnostics: Auto Configuration Backup Now";

include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<div id='maincontent'>
<?php
	include("fbegin.inc"); 
	if(strstr($pfSversion, "1.2")) 
		echo "<p class=\"pgtitle\">{$pgtitle}</p>";
	if($savemsg) {
		echo "<div id='savemsg'>";
		print_info_box($savemsg);
		echo "</div>";	
	}	
	if ($input_errors)
		print_input_errors($input_errors);

?>
<form method="post" action="autoconfigbackup_backup.php">
<table width="100%" border="0" cellpadding="0" cellspacing="0">  <tr><td>
<div id='feedbackdiv'></div>
<?php
	$tab_array = array();
	$tab_array[] = array("Settings", false, "/pkg_edit.php?xml=autoconfigbackup.xml&amp;id=0");
	$tab_array[] = array("Restore", false, "/autoconfigbackup.php");
	$tab_array[] = array("Backup now", true, "/autoconfigbackup_backup.php");
	display_top_tabs($tab_array);
?>			
  </td></tr>
  <tr>
    <td>
	<table id="backuptable" class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td colspan="2" align="left">
			<table>
				<tr>
					<td align="right">
						Enter the backup reason:
					</td>
					<td>
						<input name="reason" id="reason">
					</td>
				</tr>
				<tr>
					<td align="right">
						Do not overwrite previous backups for this hostname:
					</td>
					<td>
						<input type="nooverwrite" type="checkbox">
					</td>
				</tr>
				<tr>
					<td colspan="2" align="middle">
						<input type="button" name="Backup" value="Backup">
					</td>
				</tr>
			</table>
		</td>
	</tr>
	</table>
	</div>
    </td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
