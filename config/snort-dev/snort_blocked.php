<?php
/* $Id$ */
/*
	snort_blocked.php
	Copyright (C) 2006 Scott Ullrich
	All rights reserved.

	Modified for the Pfsense snort package v. 1.8+
	Copyright (C) 2009 Robert Zelaya Sr. Developer
	
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

$pconfig['brefresh'] = $config['installedpackages']['snortglobal']['alertsblocks']['brefresh'];
$pconfig['blertnumber'] = $config['installedpackages']['snortglobal']['alertsblocks']['blertnumber'];

if ($pconfig['blertnumber'] == '' || $pconfig['blertnumber'] == '0')
{
	$bnentries = '500';
}else{
	$bnentries = $pconfig['blertnumber'];
}

if($_POST['todelete'] or $_GET['todelete']) {
	if($_POST['todelete'])
		$ip = $_POST['todelete'];
	if($_GET['todelete'])
		$ip = $_GET['todelete'];
	exec("/sbin/pfctl -t snort2c -T delete {$ip}");
}

if ($_POST['remove']) {

exec("/sbin/pfctl -t snort2c -T flush");
sleep(1);
header("Location: /snort/snort_blocked.php");

}

/* TODO: build a file with block ip and disc */
if ($_POST['download'])
{

	ob_start(); //important or other posts will fail
	$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
	$file_name = "snort_blocked_{$save_date}.tar.gz";
	exec('/bin/mkdir /tmp/snort_blocked');	
	exec('/sbin/pfctl -t snort2c -T show > /tmp/snort_block.pf');
	
	$blocked_ips_array_save = str_replace('   ', '', array_filter(explode("\n", file_get_contents('/tmp/snort_block.pf'))));
	
	if ($blocked_ips_array_save[0] != '')
	{

		/* build the list */
		$counter = 0;
		foreach($blocked_ips_array_save as $fileline3)
		{

			$counter++;

			exec("/bin/echo  $fileline3 >> /tmp/snort_blocked/snort_block.pf");	
	
		}
	}

	exec("/usr/bin/tar cfz /tmp/snort_blocked_{$save_date}.tar.gz /tmp/snort_blocked");

	if(file_exists("/tmp/snort_blocked_{$save_date}.tar.gz"))
	{
		$file = "/tmp/snort_blocked_{$save_date}.tar.gz";
		header("Expires: Mon, 26 Jul 1997 05:00:00 GMT\n");
		header("Pragma: private"); // needed for IE
		header("Cache-Control: private, must-revalidate"); // needed for IE
		header('Content-type: application/force-download');
		header('Content-Transfer-Encoding: Binary');
		header("Content-length: ".filesize($file));
		header("Content-disposition: attachment; filename = {$file_name}");
		readfile("$file");
		exec("/bin/rm /tmp/snort_blocked_{$save_date}.tar.gz");
		exec("/bin/rm /tmp/snort_block.pf");
		exec("/bin/rm /tmp/snort_blocked/snort_block.pf");
		od_end_clean(); //importanr or other post will fail
	}else{
    echo 'Error no saved file.';
	}

}

if ($_POST['save'])
{

	/* input validation */
	if ($_POST['save'])
	{

	
	}
		
		/* no errors */
		if (!$input_errors)
		{
		
		$config['installedpackages']['snortglobal']['alertsblocks']['brefresh'] = $_POST['brefresh'] ? on : off;
		$config['installedpackages']['snortglobal']['alertsblocks']['blertnumber'] = $_POST['blertnumber'];
		
		conf_mount_rw();
		write_config();
		//conf_mount_ro();
		sleep(2);
			
		header("Location: /snort/snort_blocked.php");
				
		}
		
}

/* build filter funcs */
function get_snort_alert_ip_src($fileline)
{
        /* SRC IP */
        $re1='.*?';   # Non-greedy match on filler
        $re2='((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?![\\d])'; # IPv4 IP Address 1

        if ($c=preg_match_all ("/".$re1.$re2."/is", $fileline, $matches4))
        {
			$alert_ip_src = $matches4[1][0];
        }

return $alert_ip_src;

}

function get_snort_alert_disc($fileline)
{
        /* disc */
        if (preg_match("/\[\*\*\] (\[.*\]) (.*) (\[\*\*\])/", $fileline, $matches))
        {
			$alert_disc =  "$matches[2]";
        }

return $alert_disc;

}

/* build sec filters */
function get_snort_block_ip($fileline)
{
        /* ip */
        if (preg_match("/\[\d+\.\d+\.\d+\.\d+\]/", $fileline, $matches))
        {
			$alert_block_ip =  "$matches[0]";
        }

return $alert_block_ip;

}

function get_snort_block_disc($fileline)
{
        /* disc */
        if (preg_match("/\]\s\[.+\]$/", $fileline, $matches))
        {
			$alert_block_disc =  "$matches[0]";
        }

return $alert_block_disc;

}

/* tell the user what settings they have */
$blockedtab_msg_chk = $config['installedpackages']['snortglobal']['rm_blocked'];
	if ($blockedtab_msg_chk == "1h_b") {
		$blocked_msg = "hour";
	}
	if ($blockedtab_msg_chk == "3h_b") {
		$blocked_msg = "3 hours";
	}
	if ($blockedtab_msg_chk == "6h_b") {
		$blocked_msg = "6 hours";
	}
	if ($blockedtab_msg_chk == "12h_b") {
		$blocked_msg = "12 hours";
	}
	if ($blockedtab_msg_chk == "1d_b") {
		$blocked_msg = "day";
	}
	if ($blockedtab_msg_chk == "4d_b") {
		$blocked_msg = "4 days";
	}
	if ($blockedtab_msg_chk == "7d_b") {
		$blocked_msg = "7 days";
	}
	if ($blockedtab_msg_chk == "28d_b") {
		$blocked_msg = "28 days";
	}

if ($blockedtab_msg_chk != "never_b")
{
$blocked_msg_txt = "Hosts are removed every <strong>$blocked_msg</strong>.";
}else{
$blocked_msg_txt = "Settings are set to never <strong>remove</strong> hosts.";
}
	
$pgtitle = "Services: Snort Blocked Hosts";
include("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php 

include("fbegin.inc");

/* refresh every 60 secs */
if ($pconfig['brefresh'] == 'on' || $pconfig['brefresh'] == '')
{
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/snort/snort_blocked.php\" />\n";
}
?>

<script src="/row_toggle.js" type="text/javascript"></script>
<script src="/javascript/sorttable.js" type="text/javascript"></script>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
   <tr>
   		<td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", true, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
	</td>
	</tr>
		<br>
		<tr>
		<td>
		<div id="mainarea">
		
		<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
		  <tr>
			<td width="22%" colspan="0" class="listtopic">
			Last <?=$bnentries;?> Blocked.
			</td>
			<td width="78%" class="listtopic">
			This page lists hosts that have been blocked by Snort.&nbsp;&nbsp;<?=$blocked_msg_txt;?>
			</td>
		  </tr>
    <tr>
        <td width="22%" class="vncell">Save or Remove Hosts</td>
		<td width="78%" class="vtable">
		<form action="/snort/snort_blocked.php" method="post">
        <input name="download" type="submit" class="formbtn" value="Download">
        All blocked hosts will be saved.
		<input name="remove" type="submit" class="formbtn" value="Clear">
		<span class="red"><strong>Warning:</strong></span> all hosts will be removed.
		</form>
		</td>
    </tr>
    <tr>
        <td width="22%" class="vncell">Auto Refresh and Log View</td>
		<td width="78%" class="vtable">
		<form action="/snort/snort_blocked.php" method="post">
		<input name="save" type="submit" class="formbtn" value="Save">
		Refresh
        <input name="brefresh" type="checkbox" value="on" <?php if ($config['installedpackages']['snortglobal']['alertsblocks']['brefresh']=="on" || $config['installedpackages']['snortglobal']['alertsblocks']['brefresh']=='') echo "checked"; ?>>
        <strong>Default</strong> is <strong>ON</strong>.
		<input name="blertnumber" type="text" class="formfld" id="blertnumber" size="5" value="<?=htmlspecialchars($bnentries);?>">
		Enter the number of blocked entries to view. <strong>Default</strong> is <strong>500</strong>.
		</form>
		</td>
    </tr>
		</table>
		
	</div>
	</td>
  </tr>

					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
				<tr> 
					<td>
						<table id="sortabletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
						    <tr id="frheader">
								<td width="5%" class="listhdrr">Remove</td>
								<td class="listhdrr">#</td>
								<td class="listhdrr">IP</td>
								<td class="listhdrr">Alert Description</td>
							</tr>
<?php

/* set the arrays */
exec('/sbin/pfctl -t snort2c -T show > /tmp/snort_block.cache');
$alerts_array = array_reverse(array_filter(explode("\n\n", file_get_contents('/var/log/snort/alert'))));
$blocked_ips_array = str_replace('   ', '', array_filter(explode("\n", file_get_contents('/tmp/snort_block.cache'))));

$logent = $bnentries;

if ($blocked_ips_array[0] != '' && $alerts_array[0] != '')
{

	/* build the list and compare blocks to alerts */
	$counter = 0;
	foreach($alerts_array as $fileline)
	{

	$counter++;

	$alert_ip_src =  get_snort_alert_ip_src($fileline);
	$alert_ip_disc = get_snort_alert_disc($fileline);
	$alert_ip_src_array[] = get_snort_alert_ip_src($fileline);

		if (in_array("$alert_ip_src", $blocked_ips_array))
		{
			$input[] = "[$alert_ip_src] " . "[$alert_ip_disc]\n";
		}
	}

	foreach($blocked_ips_array as $alert_block_ip)
	{

		if (!in_array($alert_block_ip, $alert_ip_src_array))
		{
			$input[] = "[$alert_block_ip] " . "[N\A]\n";
		}
	}

	/* reduce double occurrences */
	$result = array_unique($input);

	/* buil final list, preg_match, buld html */
	$counter2 = 0;

	foreach($result as $fileline2)
	{
		if($logent <= $counter2)
		continue;

		$counter2++;

		$alert_block_ip_str =  get_snort_block_ip($fileline2);

		if($alert_block_ip_str != '')
		{
			$alert_block_ip_match = array('[',']'); 
			$alert_block_ip = str_replace($alert_block_ip_match, '', "$alert_block_ip_str");
		}else{	
			$alert_block_ip = 'empty';
		}

		$alert_block_disc_str = get_snort_block_disc($fileline2);

		if($alert_block_disc_str != '')
		{
			$alert_block_disc_match = array('] [',']'); 
			$alert_block_disc = str_replace($alert_block_disc_match, '', "$alert_block_disc_str");
		}else{	
			$alert_block_disc = 'empty';
		}

		/* use one echo to do the magic*/
		echo "<tr>
			<td align=\"center\" valign=\"top\"'><a href='snort_blocked.php?todelete=" . trim(urlencode($alert_block_ip)) . "'>
			<img title=\"Delete\" border=\"0\" name='todelete' id='todelete' alt=\"Delete\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"></a></td>
			<td>&nbsp;{$counter2}</td>
			<td>&nbsp;{$alert_block_ip}</td>
			<td>&nbsp;{$alert_block_disc}</td>
			</tr>\n";
		
	}

}else{

	/* if alerts file is empty and blocked table is not empty */
	$counter2 = 0;

	foreach($blocked_ips_array as $alert_block_ip)
	{
		if($logent <= $counter2)
		continue;

		$counter2++;
		
		$alert_block_disc = 'N/A';
		
		/* use one echo to do the magic*/
		echo "<tr>
			<td align=\"center\" valign=\"top\"'><a href='snort_blocked.php?todelete=" . trim(urlencode($alert_block_ip)) . "'>
			<img title=\"Delete\" border=\"0\" name='todelete' id='todelete' alt=\"Delete\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"></a></td>
			<td>&nbsp;{$counter2}</td>
			<td>&nbsp;{$alert_block_ip}</td>
			<td>&nbsp;{$alert_block_disc}</td>
			</tr>\n";		
	}
}

if ($blocked_ips_array[0] == '')
{
		echo "\n<tr><td colspan='3' align=\"center\" valign=\"top\"><br><strong>There are currently no items being blocked by snort.</strong></td></tr>";
}else{
		echo "\n<tr><td colspan='3' align=\"center\" valign=\"top\">{$counter2} items listed.</td></tr>";
}

?>
						</table>
		    		</td>
		  		</tr>
			</table>
		</div>
	</td>
  </tr>
</table>
<?php include("fend.inc"); ?>
</body>
</html>
