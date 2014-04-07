<?php
/*
 * suricata_alerts.php
 * part of pfSense
 *
 * Copyright (C) 2014 Bill Meeks
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

require_once("guiconfig.inc");
require_once("/usr/local/pkg/suricata/suricata.inc");

$supplist = array();

function suricata_is_alert_globally_suppressed($list, $gid, $sid) {

	/************************************************/
	/* Checks the passed $gid:$sid to see if it has */
	/* been globally suppressed.  If true, then any */
	/* "track by_src" or "track by_dst" options are */
	/* disabled since they are overridden by the    */
	/* global suppression of the $gid:$sid.         */
	/************************************************/

	/* If entry has a child array, then it's by src or dst ip. */
	/* So if there is a child array or the keys are not set,   */
	/* then this gid:sid is not globally suppressed.           */
	if (is_array($list[$gid][$sid]))
		return false;
	elseif (!isset($list[$gid][$sid]))
		return false;
	else
		return true;
}

function suricata_add_supplist_entry($suppress) {

	/************************************************/
	/* Adds the passed entry to the Suppress List   */
	/* for the active interface.  If a Suppress     */
	/* List is defined for the interface, it is     */
	/* used.  If no list is defined, a new default  */
	/* list is created using the interface name.    */
	/*                                              */
	/* On Entry:                                    */
	/*   $suppress --> suppression entry text       */
	/*                                              */
	/* Returns:                                     */
	/*   TRUE if successful or FALSE on failure     */
	/************************************************/

	global $config, $a_instance, $instanceid;

	if (!is_array($config['installedpackages']['suricata']['suppress']))
		$config['installedpackages']['suricata']['suppress'] = array();
	if (!is_array($config['installedpackages']['suricata']['suppress']['item']))
		$config['installedpackages']['suricata']['suppress']['item'] = array();
	$a_suppress = &$config['installedpackages']['suricata']['suppress']['item'];

	$found_list = false;

	/* If no Suppress List is set for the interface, then create one with the interface name */
	if (empty($a_instance[$instanceid]['suppresslistname']) || $a_instance[$instanceid]['suppresslistname'] == 'default') {
		$s_list = array();
		$s_list['uuid'] = uniqid();
		$s_list['name'] = $a_instance[$instanceid]['interface'] . "suppress" . "_" . $s_list['uuid'];
		$s_list['descr']  =  "Auto-generated list for Alert suppression";
		$s_list['suppresspassthru'] = base64_encode($suppress);
		$a_suppress[] = $s_list;
		$a_instance[$instanceid]['suppresslistname'] = $s_list['name'];
		$found_list = true;
	} else {
		/* If we get here, a Suppress List is defined for the interface so see if we can find it */
		foreach ($a_suppress as $a_id => $alist) {
			if ($alist['name'] == $a_instance[$instanceid]['suppresslistname']) {
				$found_list = true;
				if (!empty($alist['suppresspassthru'])) {
					$tmplist = base64_decode($alist['suppresspassthru']);
					$tmplist .= "\n{$suppress}";
					$alist['suppresspassthru'] = base64_encode($tmplist);
					$a_suppress[$a_id] = $alist;
				}
				else {
					$alist['suppresspassthru'] = base64_encode($suppress);
					$a_suppress[$a_id] = $alist;
				}
			}
		}
	}

	/* If we created a new list or updated an existing one, save the change, */
	/* tell Snort to load it, and return true; otherwise return false.       */
	if ($found_list) {
		write_config();
		sync_suricata_package_config();
		suricata_reload_config($a_instance[$instanceid]);
		return true;
	}
	else
		return false;
}

if (isset($_POST['instance']) && is_numericint($_POST['instance']))
	$instanceid = $_POST['instance'];
// This is for the auto-refresh so we can  stay on the same interface
elseif (isset($_GET['instance']) && is_numericint($_GET['instance']))
	$instanceid = $_GET['instance'];

if (is_null($instanceid))
	$instanceid = 0;

if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();
$a_instance = &$config['installedpackages']['suricata']['rule'];
$suricata_uuid = $a_instance[$instanceid]['uuid'];
$if_real = get_real_interface($a_instance[$instanceid]['interface']);
$suricatalogdir = SURICATALOGDIR;

// Load up the arrays of force-enabled and force-disabled SIDs
$enablesid = suricata_load_sid_mods($a_instance[$instanceid]['rule_sid_on']);
$disablesid = suricata_load_sid_mods($a_instance[$instanceid]['rule_sid_off']);

$pconfig = array();
if (is_array($config['installedpackages']['suricata']['alertsblocks'])) {
	$pconfig['arefresh'] = $config['installedpackages']['suricata']['alertsblocks']['arefresh'];
	$pconfig['alertnumber'] = $config['installedpackages']['suricata']['alertsblocks']['alertnumber'];
}

if (empty($pconfig['alertnumber']))
	$pconfig['alertnumber'] = '250';
if (empty($pconfig['arefresh']))
	$pconfig['arefresh'] = 'off';
$anentries = $pconfig['alertnumber'];

if ($_POST['save']) {
	if (!is_array($config['installedpackages']['suricata']['alertsblocks']))
		$config['installedpackages']['suricata']['alertsblocks'] = array();
	$config['installedpackages']['suricata']['alertsblocks']['arefresh'] = $_POST['arefresh'] ? 'on' : 'off';
	$config['installedpackages']['suricata']['alertsblocks']['alertnumber'] = $_POST['alertnumber'];

	write_config();

	header("Location: /suricata/suricata_alerts.php?instance={$instanceid}");
	exit;
}

//if ($_POST['unblock'] && $_POST['ip']) {
//	if (is_ipaddr($_POST['ip'])) {
//		exec("/sbin/pfctl -t snort2c -T delete {$_POST['ip']}");
//		$savemsg = gettext("Host IP address {$_POST['ip']} has been removed from the Blocked Table.");
//	}
//}

if (($_POST['addsuppress_srcip'] || $_POST['addsuppress_dstip'] || $_POST['addsuppress']) && is_numeric($_POST['sidid']) && is_numeric($_POST['gen_id'])) {
	if ($_POST['addsuppress_srcip'])
		$method = "by_src";
	elseif ($_POST['addsuppress_dstip'])
		$method = "by_dst";
	else
		$method ="all";

	// See which kind of Suppress Entry to create
	switch ($method) {
		case "all":
			if (empty($_POST['descr']))
				$suppress = "suppress gen_id {$_POST['gen_id']}, sig_id {$_POST['sidid']}\n";
			else
				$suppress = "#{$_POST['descr']}\nsuppress gen_id {$_POST['gen_id']}, sig_id {$_POST['sidid']}\n";
			$success = gettext("An entry for 'suppress gen_id {$_POST['gen_id']}, sig_id {$_POST['sidid']}' has been added to the Suppress List.");
			break;
		case "by_src":
		case "by_dst":
			// Check for valid IP addresses, exit if not valid
			if (is_ipaddr($_POST['ip'])) {
				if (empty($_POST['descr']))
					$suppress = "suppress gen_id {$_POST['gen_id']}, sig_id {$_POST['sidid']}, track {$method}, ip {$_POST['ip']}\n";
				else  
					$suppress = "#{$_POST['descr']}\nsuppress gen_id {$_POST['gen_id']}, sig_id {$_POST['sidid']}, track {$method}, ip {$_POST['ip']}\n";
				$success = gettext("An entry for 'suppress gen_id {$_POST['gen_id']}, sig_id {$_POST['sidid']}, track {$method}, ip {$_POST['ip']}' has been added to the Suppress List.");
			}
			else {
				header("Location: /suricata/suricata_alerts.php");
				exit;
			}
			break;
		default:
			header("Location: /suricata/suricata_alerts.php");
			exit;
	}

	/* Add the new entry to the Suppress List and signal Suricata to reload config */
	if (suricata_add_supplist_entry($suppress)) {
		suricata_reload_config($a_instance[$instanceid]);
		$savemsg = $success;
		sleep(2);
	}
	else
		$input_errors[] = gettext("Suppress List '{$a_instance[$instanceid]['suppresslistname']}' is defined for this interface, but it could not be found!");
}

if ($_POST['togglesid'] && is_numeric($_POST['sidid']) && is_numeric($_POST['gen_id'])) {
	// Get the GID and SID tags embedded in the clicked rule icon.
	$gid = $_POST['gen_id'];
	$sid= $_POST['sidid'];

	// See if the target SID is in our list of modified SIDs,
	// and toggle it if present.
	if (isset($enablesid[$gid][$sid]))
		unset($enablesid[$gid][$sid]);
	if (isset($disablesid[$gid][$sid]))
		unset($disablesid[$gid][$sid]);
	elseif (!isset($disablesid[$gid][$sid]))
		$disablesid[$gid][$sid] = "disablesid";

	// Write the updated enablesid and disablesid values to the config file.
	$tmp = "";
	foreach (array_keys($enablesid) as $k1) {
		foreach (array_keys($enablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_instance[$instanceid]['rule_sid_on'] = $tmp;
	else				
		unset($a_instance[$instanceid]['rule_sid_on']);

	$tmp = "";
	foreach (array_keys($disablesid) as $k1) {
		foreach (array_keys($disablesid[$k1]) as $k2)
			$tmp .= "{$k1}:{$k2}||";
	}
	$tmp = rtrim($tmp, "||");

	if (!empty($tmp))
		$a_instance[$instanceid]['rule_sid_off'] = $tmp;
	else				
		unset($a_instance[$instanceid]['rule_sid_off']);

	/* Update the config.xml file. */
	write_config();

	/*************************************************/
	/* Update the suricata.yaml file and rebuild the */
	/* rules for this interface.                     */
	/*************************************************/
	$rebuild_rules = true;
	suricata_generate_yaml($a_instance[$instanceid]);
	$rebuild_rules = false;

	/* Signal Suricata to live-load the new rules */
	suricata_reload_config($a_instance[$instanceid]);
	sleep(2);

	$savemsg = gettext("The state for rule {$gid}:{$sid} has been modified.  Suricata is 'live-reloading' the new rules list.  Please wait at least 15 secs for the process to complete before toggling additional rules.");
}

if ($_POST['delete']) {
	suricata_post_delete_logs($suricata_uuid);
	$fd = @fopen("{$suricatalogdir}suricata_{$if_real}{$suricata_uuid}/alerts.log", "w+");
	if ($fd)
		fclose($fd);
	/* XXX: This is needed if suricata is run as suricata user */
	mwexec('/bin/chmod 660 {$suricatalogdir}*', true);
	header("Location: /suricata/suricata_alerts.php?instance={$instanceid}");
	exit;
}

if ($_POST['download']) {
	$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
	$file_name = "suricata_logs_{$save_date}_{$if_real}.tar.gz";
	exec("cd {$suricatalogdir}suricata_{$if_real}{$suricata_uuid} && /usr/bin/tar -czf /tmp/{$file_name} *");

	if (file_exists("/tmp/{$file_name}")) {
		ob_start(); //important or other posts will fail
		if (isset($_SERVER['HTTPS'])) {
			header('Pragma: ');
			header('Cache-Control: ');
		} else {
			header("Pragma: private");
			header("Cache-Control: private, must-revalidate");
		}
		header("Content-Type: application/octet-stream");
		header("Content-length: " . filesize("/tmp/{$file_name}"));
		header("Content-disposition: attachment; filename = {$file_name}");
		ob_end_clean(); //important or other post will fail
		readfile("/tmp/{$file_name}");

		// Clean up the temp file
		@unlink("/tmp/{$file_name}");
	}
	else
		$savemsg = gettext("An error occurred while creating archive");
}

/* Load up an array with the current Suppression List GID,SID values */
$supplist = suricata_load_suppress_sigs($a_instance[$instanceid], true);

$pgtitle = gettext("Suricata: Alerts");
include_once("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script src="/javascript/filter_log.js" type="text/javascript"></script>
<?php
include_once("fbegin.inc");

/* refresh every 60 secs */
if ($pconfig['arefresh'] == 'on')
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/suricata/suricata_alerts.php?instance={$instanceid}\" />\n";
?>

<?php
/* Display Alert message */
if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
}
if ($savemsg) {
	print_info_box($savemsg);
}
?>
<form action="/suricata/suricata_alerts.php" method="post" id="formalert">
<input type="hidden" name="sidid" id="sidid" value=""/>
<input type="hidden" name="gen_id" id="gen_id" value=""/>
<input type="hidden" name="ip" id="ip" value=""/>
<input type="hidden" name="descr" id="descr" value=""/>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Suricata Interfaces"), false, "/suricata/suricata_interfaces.php");
	$tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
	$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), true, "/suricata/suricata_alerts.php");
	$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php?instance={$instanceid}");
	$tab_array[] = array(gettext("Logs Mgmt"), false, "/suricata/suricata_logs_mgmt.php");
	display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td><div id="mainarea">
		<table id="maintable" class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Alert Log View Settings"); ?></td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Instance to Inspect'); ?></td>
				<td width="78%" class="vtable">
					<select name="instance" id="instance" class="formselect" onChange="document.getElementById('formalert').method='post';document.getElementById('formalert').submit()">
			<?php
				foreach ($a_instance as $id => $instance) {
					$selected = "";
					if ($id == $instanceid)
						$selected = "selected";
					echo "<option value='{$id}' {$selected}> (" . convert_friendly_interface_to_friendly_descr($instance['interface']) . ") {$instance['descr']}</option>\n";
				}
			?>
					</select>&nbsp;&nbsp;<?php echo gettext('Choose which instance alerts you want to inspect.'); ?>
				</td>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Save or Remove Logs'); ?></td>
				<td width="78%" class="vtable">
					<input name="download" type="submit" class="formbtns" value="Download" 
					title="<?=gettext("Download interface log files as a gzip archive");?>"/>
					&nbsp;<?php echo gettext('All log files will be saved.');?>&nbsp;&nbsp;
					<input name="delete" type="submit" class="formbtns" value="Clear" 
					onclick="return confirm('Do you really want to remove all instance logs?')" title="<?=gettext("Clear all interface log files");?>"/>
					&nbsp;<span class="red"><strong><?php echo gettext('Warning:'); ?></strong></span>&nbsp;<?php echo gettext('all log files will be deleted.'); ?>
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Auto Refresh and Log View'); ?></td>
				<td width="78%" class="vtable">
					<input name="save" type="submit" class="formbtns" value=" Save " title="<?=gettext("Save auto-refresh and view settings");?>"/>
					&nbsp;<?php echo gettext('Refresh');?>&nbsp;&nbsp;<input name="arefresh" type="checkbox" value="on" 
					<?php if ($config['installedpackages']['snortglobal']['alertsblocks']['arefresh']=="on") echo "checked"; ?>/>
					<?php printf(gettext('%sDefault%s is %sON%s.'), '<strong>', '</strong>', '<strong>', '</strong>'); ?>&nbsp;&nbsp;
					<input name="alertnumber" type="text" class="formfld unknown" id="alertnumber" size="5" value="<?=htmlspecialchars($anentries);?>"/>
					&nbsp;<?php printf(gettext('Enter number of log entries to view. %sDefault%s is %s250%s.'), '<strong>', '</strong>', '<strong>', '</strong>'); ?>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php printf(gettext("Last %s Alert Entries"), $anentries); ?>&nbsp;&nbsp;
				<?php echo gettext("(Most recent entries are listed first)"); ?></td>
			</tr>
	<tr>
	<td width="100%" colspan="2">
	<table id="myTable" style="table-layout: fixed;" width="100%" class="sortable" border="0" cellpadding="0" cellspacing="0">
		<colgroup>
			<col width="10%" align="center" axis="date">
			<col width="40" align="center" axis="number">
			<col width="52" align="center" axis="string">
			<col width="10%" axis="string">
			<col width="13%" align="center" axis="string">
			<col width="7%" align="center" axis="string">
			<col width="13%" align="center" axis="string">
			<col width="7%" align="center" axis="string">
			<col width="10%" align="center" axis="number">
			<col axis="string">
		</colgroup>
		<thead>
		   <tr>
			<th class="listhdrr" axis="date"><?php echo gettext("Date"); ?></th>
			<th class="listhdrr" axis="number"><?php echo gettext("Pri"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Proto"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Class"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Src"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("SPort"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Dst"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("DPort"); ?></th>
			<th class="listhdrr" axis="number"><?php echo gettext("SID"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("Description"); ?></th>
		   </tr>
		</thead>
	<tbody>
	<?php

/* make sure alert file exists */
if (file_exists("/var/log/suricata/suricata_{$if_real}{$suricata_uuid}/alerts.log")) {
	exec("tail -{$anentries} -r /var/log/suricata/suricata_{$if_real}{$suricata_uuid}/alerts.log > /tmp/alerts_suricata{$suricata_uuid}");
	if (file_exists("/tmp/alerts_suricata{$suricata_uuid}")) {
		$tmpblocked = array_flip(suricata_get_blocked_ips());
		$counter = 0;
		/*             0         1      2             3      4       5   6              7        8     9   10      11  12      */
		/* File format timestamp,action,sig_generator,sig_id,sig_rev,msg,classification,priority,proto,src,srcport,dst,dstport */
		$fd = fopen("/tmp/alerts_suricata{$suricata_uuid}", "r");
		while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
			if(count($fields) < 13)
				continue;

			// Create a DateTime object from the event timestamp that
			// we can use to easily manipulate output formats.
			$event_tm = date_create_from_format("m/d/Y-H:i:s.u", $fields[0]);

			// Check the 'CATEGORY' field for the text "(null)" and
			// substitute "Not Assigned".
			if ($fields[6] == "(null)")
				$fields[6] = "Not Assigned";

			/* Time */
			$alert_time = date_format($event_tm, "H:i:s");
			/* Date */
			$alert_date = date_format($event_tm, "m/d/Y");
			/* Description */
			$alert_descr = $fields[5];
			$alert_descr_url = urlencode($fields[5]);
			/* Priority */
			$alert_priority = $fields[7];
			/* Protocol */
			$alert_proto = $fields[8];
			/* IP SRC */
			$alert_ip_src = $fields[9];
			/* Add zero-width space as soft-break opportunity after each colon if we have an IPv6 address */
			$alert_ip_src = str_replace(":", ":&#8203;", $alert_ip_src);
			/* Add Reverse DNS lookup icons */
			$alert_ip_src .= "<br/><a onclick=\"javascript:getURL('/diag_dns.php?host={$fields[9]}&dialog_output=true', outputrule);\">";
			$alert_ip_src .= "<img src='../themes/{$g['theme']}/images/icons/icon_log_d.gif' width='11' height='11' border='0' ";
			$alert_ip_src .= "title='" . gettext("Resolve host via reverse DNS lookup (quick pop-up)") . "' style=\"cursor: pointer;\"></a>&nbsp;";
			$alert_ip_src .= "<a href='/diag_dns.php?host={$fields[9]}&instance={$instanceid}'>";
			$alert_ip_src .= "<img src='../themes/{$g['theme']}/images/icons/icon_log.gif' width='11' height='11' border='0' ";
			$alert_ip_src .= "title='" . gettext("Resolve host via reverse DNS lookup") . "'></a>";
			/* Add icons for auto-adding to Suppress List if appropriate */
			if (!suricata_is_alert_globally_suppressed($supplist, $fields[2], $fields[3]) && 
			    !isset($supplist[$fields[2]][$fields[3]]['by_src'][$fields[9]])) {
				$alert_ip_src .= "&nbsp;&nbsp;<input type='image' name='addsuppress_srcip[]' onClick=\"encRuleSig('{$fields[2]}','{$fields[3]}','{$fields[9]}','{$alert_descr}');\" ";
				$alert_ip_src .= "src='../themes/{$g['theme']}/images/icons/icon_plus.gif' width='12' height='12' border='0' ";
				$alert_ip_src .= "title='" . gettext("Add this alert to the Suppress List and track by_src IP") . "'/>";	
			}
			elseif (isset($supplist[$fields[2]][$fields[3]]['by_src'][$fields[9]])) {
				$alert_ip_src .= "&nbsp;&nbsp;<img src='../themes/{$g['theme']}/images/icons/icon_plus_d.gif' width='12' height='12' border='0' ";
				$alert_ip_src .= "title='" . gettext("This alert track by_src IP is already in the Suppress List") . "'/>";	
			}
			/* Add icon for auto-removing from Blocked Table if required */
//			if (isset($tmpblocked[$fields[9]])) {
//				$alert_ip_src .= "&nbsp;<input type='image' name='unblock[]' onClick=\"document.getElementById('ip').value='{$fields[9]}';\" ";
//				$alert_ip_src .= "title='" . gettext("Remove host from Blocked Table") . "' border='0' width='12' height='12' src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"/>"; 
//			}
			/* IP SRC Port */
			$alert_src_p = $fields[10];
			/* IP Destination */
			$alert_ip_dst = $fields[11];
			/* Add zero-width space as soft-break opportunity after each colon if we have an IPv6 address */
			$alert_ip_dst = str_replace(":", ":&#8203;", $alert_ip_dst);
			/* Add Reverse DNS lookup icons */
			$alert_ip_dst .= "<br/><a onclick=\"javascript:getURL('/diag_dns.php?host={$fields[11]}&dialog_output=true', outputrule);\">";
			$alert_ip_dst .= "<img src='../themes/{$g['theme']}/images/icons/icon_log_d.gif' width='11' height='11' border='0' ";
			$alert_ip_dst .= "title='" . gettext("Resolve host via reverse DNS lookup (quick pop-up)") . "' style=\"cursor: pointer;\"></a>&nbsp;";
			$alert_ip_dst .= "<a href='/diag_dns.php?host={$fields[11]}&instance={$instanceid}'>";
			$alert_ip_dst .= "<img src='../themes/{$g['theme']}/images/icons/icon_log.gif' width='11' height='11' border='0' ";
			$alert_ip_dst .= "title='" . gettext("Resolve host via reverse DNS lookup") . "'></a>";	
			/* Add icons for auto-adding to Suppress List if appropriate */
			if (!suricata_is_alert_globally_suppressed($supplist, $fields[2], $fields[3]) && 
			    !isset($supplist[$fields[2]][$fields[3]]['by_dst'][$fields[11]])) {
				$alert_ip_dst .= "&nbsp;&nbsp;<input type='image' name='addsuppress_dstip[]' onClick=\"encRuleSig('{$fields[2]}','{$fields[3]}','{$fields[11]}','{$alert_descr}');\" ";
				$alert_ip_dst .= "src='../themes/{$g['theme']}/images/icons/icon_plus.gif' width='12' height='12' border='0' ";
				$alert_ip_dst .= "title='" . gettext("Add this alert to the Suppress List and track by_dst IP") . "'/>";	
			}
			elseif (isset($supplist[$fields[2]][$fields[3]]['by_dst'][$fields[11]])) {
				$alert_ip_dst .= "&nbsp;&nbsp;<img src='../themes/{$g['theme']}/images/icons/icon_plus_d.gif' width='12' height='12' border='0' ";
				$alert_ip_dst .= "title='" . gettext("This alert track by_dst IP is already in the Suppress List") . "'/>";	
			}
			/* Add icon for auto-removing from Blocked Table if required */
//			if (isset($tmpblocked[$fields[11]])) {
//				$alert_ip_dst .= "&nbsp;<input type='image' name='unblock[]' onClick=\"document.getElementById('ip').value='{$fields[11]}';\" ";
//				$alert_ip_dst .= "title='" . gettext("Remove host from Blocked Table") . "' border='0' width='12' height='12' src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"/>"; 
//			}
			/* IP DST Port */
			$alert_dst_p = $fields[12];
			/* SID */
			$alert_sid_str = "{$fields[2]}:{$fields[3]}";
			if (!suricata_is_alert_globally_suppressed($supplist, $fields[2], $fields[3])) {
				$sidsupplink = "<input type='image' name='addsuppress[]' onClick=\"encRuleSig('{$fields[2]}','{$fields[3]}','','{$alert_descr}');\" ";
				$sidsupplink .= "src='../themes/{$g['theme']}/images/icons/icon_plus.gif' width='12' height='12' border='0' ";
				$sidsupplink .= "title='" . gettext("Add this alert to the Suppress List") . "'/>";	
			}
			else {
				$sidsupplink = "<img src='../themes/{$g['theme']}/images/icons/icon_plus_d.gif' width='12' height='12' border='0' ";
				$sidsupplink .= "title='" . gettext("This alert is already in the Suppress List") . "'/>";	
			}
			/* Add icon for toggling rule state */
			if (isset($disablesid[$fields[2]][$fields[3]])) {
				$sid_dsbl_link = "<input type='image' name='togglesid[]' onClick=\"encRuleSig('{$fields[2]}','{$fields[3]}','','');\" ";
				$sid_dsbl_link .= "src='../themes/{$g['theme']}/images/icons/icon_reject.gif' width='11' height='11' border='0' ";
				$sid_dsbl_link .= "title='" . gettext("Rule is forced to a disabled state. Click to remove the force-disable action from this rule.") . "'/>";
			}
			else {
				$sid_dsbl_link = "<input type='image' name='togglesid[]' onClick=\"encRuleSig('{$fields[2]}','{$fields[3]}','','');\" ";
				$sid_dsbl_link .= "src='../themes/{$g['theme']}/images/icons/icon_block.gif' width='11' height='11' border='0' ";
				$sid_dsbl_link .= "title='" . gettext("Force-disable this rule and remove it from current rules set.") . "'/>";
			}
			/* DESCRIPTION */
			$alert_class = $fields[6];

			echo "<tr>
				<td class='listr' align='center'>{$alert_date}<br/>{$alert_time}</td>
				<td class='listr' align='center'>{$alert_priority}</td>
				<td class='listr' align='center'>{$alert_proto}</td>
				<td class='listr' style=\"word-wrap:break-word;\">{$alert_class}</td>
				<td class='listr' align='center' sorttable_customkey='{$fields[9]}'>{$alert_ip_src}</td>
				<td class='listr' align='center'>{$alert_src_p}</td>
				<td class='listr' align='center' sorttable_customkey='{$fields[11]}'>{$alert_ip_dst}</td>
				<td class='listr' align='center'>{$alert_dst_p}</td>
				<td class='listr' align='center' sorttable_customkey='{$fields[3]}'>{$alert_sid_str}<br/>{$sidsupplink}&nbsp;&nbsp;{$sid_dsbl_link}</td>
				<td class='listbg' style=\"word-wrap:break-word;\">{$alert_descr}</td>
				</tr>\n";

			$counter++;
		}
		fclose($fd);
		@unlink("/tmp/alerts_suricata{$suricata_uuid}");
	}
}
?>
		</tbody>
	</table>
	</td>
</tr>
</table>
</div>
</td></tr>
</table>
</form>
<?php
include("fend.inc");
?>
<script type="text/javascript">
function encRuleSig(rulegid,rulesid,srcip,ruledescr) {

	// This function stuffs the passed GID, SID
	// and other values into hidden Form Fields
	// for postback.
	if (typeof srcipip == "undefined")
		var srcipip = "";
	if (typeof ruledescr == "undefined")
		var ruledescr = "";
	document.getElementById("sidid").value = rulesid;
	document.getElementById("gen_id").value = rulegid;
	document.getElementById("ip").value = srcip;
	document.getElementById("descr").value = ruledescr;
}
</script>
</body>
</html>
