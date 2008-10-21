<?php
/* $Id$ */
/*
    autoconfigbackup.php
    Copyright (C) 2008 Scott Ullrich
    All rights reserved.

	Originally based on diag_confbak.php written and
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

// Set hostname
$hostname			= $config['system']['hostname'] . "." . $config['system']['domain'];

if(!$username) {
	Header("Location: /pkg_edit.php?xml=autoconfigbackup.xml&id=0");
	exit;
}

if($_POST['backup']) {
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

if($_REQUEST['savemsg']) 
	$savemsg = htmlentities($_REQUEST['savemsg']);

if($_REQUEST['download']) {
	// Phone home and obtain backups
	$curl_session = curl_init();
	curl_setopt($curl_session, CURLOPT_URL, $get_url);
	curl_setopt($curl_session, CURLOPT_POST, 3);				
	curl_setopt($curl_session, CURLOPT_SSL_VERIFYPEER, 0);	
	curl_setopt($curl_session, CURLOPT_RETURNTRANSFER, 1);	
	curl_setopt($curl_session, CURLOPT_POSTFIELDS, "action=restore" . 
				"&hostname=" . urlencode($hostname) . 
				"&revision=" . urlencode($_REQUEST['download']));
	$data = curl_exec($curl_session);
	if (!tagfile_deformat($data, $data1, "config.xml")) 
		$input_errors[] = "The downloaded file does not appear to contain an encrypted pfSense configuration.";
	if ($input_errors)
		print_input_errors($input_errors);
	else 
		echo $data;
	exit;	
}

if($_REQUEST['newver'] != "") {
	// Phone home and obtain backups
	$curl_session = curl_init();
	curl_setopt($curl_session, CURLOPT_URL, $get_url);
	curl_setopt($curl_session, CURLOPT_POST, 3);				
	curl_setopt($curl_session, CURLOPT_SSL_VERIFYPEER, 0);	
	curl_setopt($curl_session, CURLOPT_RETURNTRANSFER, 1);	
	curl_setopt($curl_session, CURLOPT_POSTFIELDS, "action=restore" . 
				"&hostname=" . urlencode($hostname) . 
				"&revision=" . urlencode($_REQUEST['newver']));
	$data = curl_exec($curl_session);
	if (!tagfile_deformat($data, $data, "config.xml")) 
		$input_errors[] = "The downloaded file does not appear to contain an encrypted pfSense configuration.";
	$data = decrypt_data($data, $decrypt_password);
	$fd = fopen("/tmp/config_restore.xml", "w");
	fwrite($fd, $data);
	fclose($fd);
	if (curl_errno($curl_session)) {
		$fd = fopen("/tmp/backupdebug.txt", "w");
		fwrite($fd, $get_url . "" . "action=restore&hostname={$hostname}&revision=" . urlencode($_REQUEST['newver']) . "\n\n");
		fwrite($fd, $data);
		fwrite($fd, curl_error($curl_session));
		fclose($fd);
	} else {
	    curl_close($curl_session);
	}
	if(!$input_errors && $data) {
		if(config_restore("/tmp/config_restore.xml") == 0) {
			$savemsg = "Successfully reverted the pfSense configuration to timestamp " . urldecode($_REQUEST['newver']) . ".";
			$savemsg .= <<<EOF
			<p/>
		  <form action="reboot.php" method="post">
			Would you like to reboot? 
		  <input name="Submit" type="submit" class="formbtn" value=" Yes ">
		  <input name="Submit" type="submit" class="formbtn" value=" No ">
		</form>
EOF;
	    	
		} else {
			$savemsg = "Unable to revert to the selected configuration.";
		}
	}
	unlink("/tmp/config_restore.xml");
} 

// Populate available backups
$curl_session = curl_init();
curl_setopt($curl_session, CURLOPT_URL, $get_url);  
curl_setopt($curl_session, CURLOPT_SSL_VERIFYPEER, 0);	
curl_setopt($curl_session, CURLOPT_POST, 1);
curl_setopt($curl_session, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($curl_session, CURLOPT_POSTFIELDS, "action=showbackups&hostname={$hostname}");
$data = curl_exec($curl_session);
if (curl_errno($curl_session)) {
	$fd = fopen("/tmp/backupdebug.txt", "w");
	fwrite($fd, $get_url . "" . "action=showbackups" . "\n\n");
	fwrite($fd, $data);
	fwrite($fd, curl_error($curl_session));
	fclose($fd);
} else {
    curl_close($curl_session);
}

if($_REQUEST['rmver'] != "") {
	// XXX: delete revision, or all backups from server.
}

// Loop through and create new confvers
$data_split = split("\n", $data);
$confvers = array();
foreach($data_split as $ds) {
	$ds_split = split($oper_sep, $ds);
	$tmp_array = array();
	$tmp_array['username'] = $ds_split[0];
	$tmp_array['reason'] = $ds_split[1];
	$tmp_array['time'] = $ds_split[2];
	if($ds_split[2] && $ds_split[0])
		$confvers[] = $tmp_array;
}

$pgtitle = "Diagnostics: Auto Configuration Backup";

include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script type="text/javascript">
	function backupnow() {
		scroll(0,0);
		var reason = prompt ("Enter the reason for the backup","");
		var url = "/autoconfigbackup.php";
		var pars = 'backup=yes&reason=' + reason;
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'post',
				parameters: pars,
				onComplete: backupcomplete
			});
			$('savemsg').innerHTML = '';
			$('feedbackdiv').innerHTML = "<img src='themes/metallic/images/misc/loader.gif'> One moment please, saving backup to portal.pfsense.org...<p/>";	
	}
	function backupcomplete(transport) {
		document.location.href = 'autoconfigbackup.php?savemsg=Backup+completed+successfully';
	}
</script>
<div id='maincontent'>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
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

<table width="100%" border="0" cellpadding="0" cellspacing="0">  <tr><td>
<div id='feedbackdiv'></div>
<?php
	$tab_array = array();
	$tab_array[0] = array("Settings", false, "/pkg_edit.php?xml=autoconfigbackup.xml&amp;id=0");
	$tab_array[1] = array("Restore", true, "/autoconfigbackup.php");
	display_top_tabs($tab_array);
?>			
  </td></tr>
  <tr>
    <td>
	<table id="backuptable" class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td width="30%" class="listhdrr">Date</td>
			<td width="70%" class="listhdrr">Configuration Change</td>
		</tr>
<?php 
	$counter = 0;
	foreach($confvers as $cv): 
?>
		<tr valign="top">
		  <td class="listlr"> <?= $cv['time']; ?></td>
			<td class="listbg"> <?= $cv['reason']; ?></td>
			<td colspan="2" valign="middle" class="list" nowrap>
			  <a href="autoconfigbackup.php?newver=<?=urlencode($cv['time']);?>">
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0">
			  </a>
			  <a href="autoconfigbackup.php?download=<?=urlencode($cv['time']);?>">
				<img src="/themes/<?= $g['theme']; ?>/images/icons/icon_down.gif" width="17" height="17" border="0">
			  </a>
		  </td>
		</tr>
<?php
	$counter++; 
	endforeach;
	if($counter == 0)
		echo "<tr><td colspan='3'><center>Sorry, we could not locate any backups at portal.pfsense.org for this hostname.</td></tr>";
?>
	</table>
	</div>
    </td>
	<tr><td>
	  <p>
	  <strong>
			&nbsp;&nbsp;<span class="red">Hint:&nbsp;
	  		</span>
	  </strong>
	  Click the + sign next to the revision you would like to restore.
	</p>	
  </td></tr>
  <tr><td>
	<form method="post" action="autoconfigbackup.php">
		<input type="button" onClick='backupnow()' name="backup" value="Backup Now">
	</form>
  </td></tr>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
