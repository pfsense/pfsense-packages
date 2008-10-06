<?php
/* $Id$ */
/*
    autoconfigbackup.php
    Copyright (C) 2005 Colin Smith
	Originally based on diag_confbak.php
    Copyright (C) 2005 Colin Smith
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

// Seperator used during client / server communications
$oper_sep = "||";

// URL to restore.php
$get_url = "https://portal.pfsense.org/pfSconfigbackups/restore.php";

// Encryption password 
$decrypt_password = $config['installedpackages']['autoconfigbackup']['config']['decrypt_password'];

if($_GET['newver'] != "") {
	// Phone home and obtain backups
	$curl_Session = curl_init($get_url);
	curl_setopt($curl_Session, CURLOPT_POST, 1);
	curl_setopt($curl_Session, CURLOPT_POSTFIELDS, "action=restore&username={$username}&password={$password}&revision={$_GET['newver']}");
	curl_setopt($curl_Session, CURLOPT_FOLLOWLOCATION, 1);
	$data = curl_exec($curl_Session);	
	if (!tagfile_deformat($data, $data, "config.xml")) 
		$input_errors[] = "The downloaded file does not appear to contain an encrypted pfSense configuration.";
	$data = decrypt_data($data, $decrypt_password);
	$fd = fopen("/tmp/config_restore.xml", "w");
	fwrite($fd, $data);
	fclose($fd);
	curl_close($curl_Session);	
	$confvers = unserialize(file_get_contents($g['cf_conf_path'] . '/backup/backup.cache'));
	unlink("/tmp/config_restore.xml");
	if(config_restore("/tmp/config_restore.xml") == 0) {
		$savemsg = "Successfully reverted to timestamp " . date("n/j/y H:i:s", $_GET['newver']) . " with description \"" . $confvers[$_GET['newver']]['description'] . "\".";
	} else {
		$savemsg = "Unable to revert to the selected configuration.";
	}
} else {
	// Grab username and password from config.xml
	$username = $config['installedpackages']['autoconfigbackup']['config']['username'];
	$password = $config['installedpackages']['autoconfigbackup']['config']['password'];
	// Phone home and obtain backups
	$curl_Session = curl_init($get_url);
	curl_setopt($curl_Session, CURLOPT_POST, 1);
	curl_setopt($curl_Session, CURLOPT_POSTFIELDS, "action=showbackups&username={$username}&password={$password}");
	curl_setopt($curl_Session, CURLOPT_FOLLOWLOCATION, 1);
	$data = curl_exec($curl_Session);
	curl_close($curl_Session);	
}

if($_GET['rmver'] != "") {
	$confvers = unserialize(file_get_contents($g['cf_conf_path'] . '/backup/backup.cache'));
	unlink_if_exists($g['conf_path'] . '/backup/config-' . $_GET['rmver'] . '.xml');
	$savemsg = "Deleted backup with timestamp " . date("n/j/y H:i:s", $_GET['rmver']) . " and description \"" . $confvers[$_GET['rmver']]['description'] . "\".";
}

// Loop through and create new confvers
$data_split = split("\n", $data);
$confvers = array();
$tmp_array = array();
foreach($data_split as $ds) {
	$ds_split = split($oper_sep, $ds);
	$tmp_array['username'] = $ds_split[0];
	$tmp_array['reason'] = $ds_split[1];
	$tmp_array['time'] = $ds_split[2];
	$confvers[] = $tmp_array();
}

$pgtitle = "Diagnostics: Auto Backup";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php if($savemsg) print_info_box($savemsg); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">  <tr><td>
<?php
	$tab_array = array();
	$tab_array[0] = array("Settings", false, "/pkg_edit.php?xml=autoconfigbackup.xml&amp;id=0");
	$tab_array[1] = array("Restore", true, "/autoconfigbackup.php");
	display_top_tabs($tab_array);
?>			
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
              <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
<?php
if(is_array($confvers)) { 
		?>
                <tr>
                  <td width="30%" class="listhdrr">Date</td>
		  		  <td width="70%" class="listhdrr">Configuration Change</td>
                </tr>
                <tr valign="top">
		  			<td class="listlr"> <?= date("n/j/y H:i:s", $config['revision']['time']) ?></td>
                  	<td class="listlr"> <?= $config['revision']['description'] ?></td>
		  			<td colspan="2" valign="middle" class="list" nowrap><b>Current</b></td>
				</tr>
		<?php
		  foreach($confvers as $version) {
			if($version['time'] != 0) {
				$date = date("n/j/y H:i:s", $version['time']);
			} else {
				$date = "Unknown";
			}
			$desc = $version['description'];
               ?>
				<tr valign="top">
					<td class="listlr"> <?= $date ?></td>
					<td class="listlr"> <?= $desc ?></td>
					<td valign="middle" class="list" nowrap>
						<a href="diag_confbak.php?newver=<?=$version['time'];?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a>
					</td>
					<td valign="middle" class="list" nowrap>
				<!-- 
					<a href="diag_confbak.php?rmver=<?=$version['time'];?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a>
				-->
			    </tr>
               <?php
                  } ?>
<?php } else { ?>
		<tr><td>
		<?php print_info_box("No backups found at http://portal.pfsense.org for username {$username}"); ?>
		</td></tr>
<?php      }
?>
	</table>
	</div>
    </td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
