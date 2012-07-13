<?php
/*
 * snort_alerts.php
 * part of pfSense
 *
 * Copyright (C) 2005 Bill Marquette <bill.marquette@gmail.com>.
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2012 Ermal Luci
 * All rights reserved.
 *
 * Modified for the Pfsense snort package v. 1.8+
 * Copyright (C) 2009 Robert Zelaya Sr. Developer
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
require_once("/usr/local/pkg/snort/snort.inc");

/* load only javascript that is needed */
$snort_load_sortabletable = 'yes';
$snort_load_mootools = 'yes';

$snortalertlogt = $config['installedpackages']['snortglobal']['snortalertlogtype'];

if ($_GET['instance'])
	$instanceid = $_GET['instance'];
if ($_POST['instance'])
	$instanceid = $_POST['instance'];
if (empty($instanceid))
	$instanceid = 0;

if (!is_array($config['installedpackages']['snortglobal']['rule']))
        $config['installedpackages']['snortglobal']['rule'] = array();
$a_instance = &$config['installedpackages']['snortglobal']['rule'];
$snort_uuid = $a_instance[$instanceid]['uuid'];
$if_real = snort_get_real_interface($a_instance[$instanceid]['interface']);

if (is_array($config['installedpackages']['snortglobal']['alertsblocks'])) {
	$pconfig['arefresh'] = $config['installedpackages']['snortglobal']['alertsblocks']['arefresh'];
	$pconfig['alertnumber'] = $config['installedpackages']['snortglobal']['alertsblocks']['alertnumber'];
	$anentries = $pconfig['alertnumber'];
} else {
	$anentries = '250';
	$pconfig['alertnumber'] = '250';
	$pconfig['arefresh'] = 'off';
}

if ($_POST['save'])
{
	if (!is_array($config['installedpackages']['snortglobal']['alertsblocks']))
		$config['installedpackages']['snortglobal']['alertsblocks'] = array();
	$config['installedpackages']['snortglobal']['alertsblocks']['arefresh'] = $_POST['arefresh'] ? 'on' : 'off';
	$config['installedpackages']['snortglobal']['alertsblocks']['alertnumber'] = $_POST['alertnumber'];

	write_config();

	header("Location: /snort/snort_alerts.php?instance={$instanceid}");
	exit;
}

if ($_GET['action'] == "clear" || $_POST['delete']) {
	if (file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")) {
		conf_mount_rw();
		snort_post_delete_logs($snort_uuid);
		$fd = fopen("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert", "w");
		if ($fd) {
			@ftruncate($fd, 0);
			fclose($fd);
		}
		conf_mount_ro();
		/* XXX: This is needed is snort is run as snort user */
		//mwexec('/usr/sbin/chown snort:snort /var/log/snort/*', true);
		mwexec('/bin/chmod 660 /var/log/snort/*', true);
		if (file_exists("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid"))
			mwexec("/bin/pkill -HUP -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid -a");
	}
	header("Location: /snort/snort_alerts.php?instance={$instanceid}");
	exit;
}

if ($_POST['download']) {
	$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
	$file_name = "snort_logs_{$save_date}_{$if_real}.tar.gz";
	exec("/usr/bin/tar cfz /tmp/{$file_name} /var/log/snort/snort_{$if_real}{$snort_uuid}");

	if (file_exists("/tmp/{$file_name}")) {
		$file = "/tmp/snort_logs_{$save_date}.tar.gz";
		header("Expires: Mon, 26 Jul 1997 05:00:00 GMT\n");
		header("Pragma: private"); // needed for IE
		header("Cache-Control: private, must-revalidate"); // needed for IE
		header('Content-type: application/force-download');
		header('Content-Transfer-Encoding: Binary');
		header("Content-length: ".filesize($file));
		header("Content-disposition: attachment; filename = {$file_name}");
		readfile("$file");
		exec("/bin/rm /tmp/{$file_name}");
	}

	header("Location: /snort/snort_alerts.php?instance={$instanceid}");
	exit;
}

/* WARNING: took me forever to figure reg expression, dont lose */
// $fileline = '12/09-18:12:02.086733  [**] [122:6:0] (portscan) TCP Filtered Decoy Portscan [**] [Priority: 3] {PROTO:255} 125.135.214.166 -> 70.61.243.50';
function get_snort_alert_date($fileline)
{
	/* date full date \d+\/\d+-\d+:\d+:\d+\.\d+\s */
	if (preg_match("/\d+\/\d+-\d+:\d+:\d\d/", $fileline, $matches))
		$alert_date =  "$matches[0]";

	return $alert_date;
}

function get_snort_alert_disc($fileline)
{
	/* disc */
	if (preg_match("/\[\*\*\] (\[.*\]) (.*) (\[\*\*\])/", $fileline, $matches))
		$alert_disc =  "$matches[2]";

	return $alert_disc;
}

function get_snort_alert_class($fileline)
{
	/* class */
	if (preg_match('/\[Classification:\s.+[^\d]\]/', $fileline, $matches))
		$alert_class = "$matches[0]";

	return $alert_class;
}

function get_snort_alert_priority($fileline)
{
	/* Priority */
	if (preg_match('/Priority:\s\d/', $fileline, $matches))
		$alert_priority = "$matches[0]";

	return $alert_priority;
}

function get_snort_alert_proto($fileline)
{
	/* Priority */
	if (preg_match('/\{.+\}/', $fileline, $matches))
		$alert_proto = "$matches[0]";

	return $alert_proto;
}

function get_snort_alert_proto_full($fileline)
{
	/* Protocal full */
	if (preg_match('/.+\sTTL/', $fileline, $matches))
		$alert_proto_full = "$matches[0]";

	return $alert_proto_full;
}

function get_snort_alert_ip_src($fileline)
{
	/* SRC IP */
	$re1='.*?';   # Non-greedy match on filler
	$re2='((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?![\\d])'; # IPv4 IP Address 1

	if (preg_match_all ("/".$re1.$re2."/is", $fileline, $matches))
		$alert_ip_src = $matches[1][0];

	return $alert_ip_src;
}

function get_snort_alert_src_p($fileline)
{
	/* source port */
	if (preg_match('/:\d+\s-/', $fileline, $matches))
		$alert_src_p = "$matches[0]";

	return $alert_src_p;
}

function get_snort_alert_flow($fileline)
{
	/* source port */
	if (preg_match('/(->|<-)/', $fileline, $matches))
		$alert_flow = "$matches[0]";

	return $alert_flow;
}

function get_snort_alert_ip_dst($fileline)
{
	/* DST IP */
	$re1dp='.*?';   # Non-greedy match on filler
	$re2dp='(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(?![\\d])';   # Uninteresting: ipaddress
	$re3dp='.*?';   # Non-greedy match on filler
	$re4dp='((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?![\\d])'; # IPv4 IP Address 1

	if (preg_match_all("/".$re1dp.$re2dp.$re3dp.$re4dp."/is", $fileline, $matches))
		$alert_ip_dst = $matches[1][0];

	return $alert_ip_dst;
}

function get_snort_alert_dst_p($fileline)
{
	/* dst port */
	if (preg_match('/:\d+$/', $fileline, $matches))
		$alert_dst_p = "$matches[0]";

	return $alert_dst_p;
}

function get_snort_alert_dst_p_full($fileline)
{
	/* dst port full */
	if (preg_match('/:\d+\n[A-Z]+\sTTL/', $fileline, $matches))
		$alert_dst_p = "$matches[0]";

	return $alert_dst_p;
}

function get_snort_alert_sid($fileline)
{
	/* SID */
	if (preg_match('/\[\d+:\d+:\d+\]/', $fileline, $matches))
		$alert_sid = "$matches[0]";

	return $alert_sid;
}

$pgtitle = "Services: Snort: Snort Alerts";
include_once("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php
include_once("fbegin.inc");

/* refresh every 60 secs */
if ($pconfig['arefresh'] == 'on')
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/snort/snort_alerts.php\" />\n";
?>

<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
        $tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
        $tab_array[3] = array(gettext("Alerts"), true, "/snort/snort_alerts.php?instance={$instanceid}");
        $tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
        $tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
        $tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
		<div id="mainarea2">
		<table class="tabcont" width="100%" border="1" cellspacing="0" cellpadding="0">
			<form action="/snort/snort_alerts.php" method="post" id="formalert">
			<tr>
				<td width="22%" colspan="0" class="listtopic">Last <?=$anentries;?> Alert Entries.</td>
				<td width="78%" class="listtopic">Latest Alert Entries Are Listed First.</td>
			</tr>
			<tr>
				<td width="22%" class="vncell">Instance to inspect</td>
				<td width="78%" class="vtable">
					<br/>   <select name="instance" id="instance" class="formselect" onChange="document.getElementById('formalert').submit()">
			<?php
				foreach ($a_instance as $id => $instance) {
					$selected = "";
					if ($id == $instanceid)
						$selected = "selected";
					echo "<option value='{$id}' {$selected}> (" . snort_get_friendly_interface($instance['interface']) . "){$instance['descr']}</option>\n";
				}
			?>
					</select><br/>   Choose which instance alerts you want to inspect.
				</td>
			<tr>
				<td width="22%" class="vncell">Save or Remove Logs</td>
				<td width="78%" class="vtable">
					<input name="download" type="submit" class="formbtn" value="Download"> All
						log files will be saved. <a href="/snort/snort_alerts.php?action=clear">
					<input name="delete" type="button" class="formbtn" value="Clear"
					onclick="return confirm('Do you really want to remove all instance logs?')"></a>
					<span class="red"><strong>Warning:</strong></span> all log files will be deleted.
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell">Auto Refresh and Log View</td>
				<td width="78%" class="vtable">
					<input name="save" type="submit" class="formbtn" value="Save">
					Refresh <input name="arefresh" type="checkbox" value="on"
					<?php if ($config['installedpackages']['snortglobal']['alertsblocks']['arefresh']=="on") echo "checked"; ?>>
						<strong>Default</strong> is <strong>ON</strong>.
					<input name="alertnumber" type="text" class="formfld" id="alertnumber" size="5" value="<?=htmlspecialchars($anentries);?>">
					Enter the number of log entries to view. <strong>Default</strong> is <strong>250</strong>.
				</td>
			</tr>
			</form>
		</table>
		</div>
		</td>
	</tr>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<td width="100%"><br>
	<div class="tableFilter">
	<form id="tableFilter"
		onsubmit="myTable.filter(this.id); return false;">Filter: <select
		id="column">
		<option value="1">PRIORITY</option>
		<option value="2">PROTO</option>
		<option value="3">DESCRIPTION</option>
		<option value="4">CLASS</option>
		<option value="5">SRC</option>
		<option value="6">SRC PORT</option>
		<option value="7">FLOW</option>
		<option value="8">DST</option>
		<option value="9">DST PORT</option>
		<option value="10">SID</option>
		<option value="11">Date</option>
	</select> <input type="text" id="keyword" /> <input type="submit"
		value="Submit" /> <input type="reset" value="Clear" /></form>
	</div>
	<table class="allRow" id="myTable" width="100%" border="2"
		cellpadding="1" cellspacing="1">
		<thead>
			<th axis="number">#</th>
			<th axis="string">PRI</th>
			<th axis="string">PROTO</th>
			<th axis="string">DESCRIPTION</th>
			<th axis="string">CLASS</th>
			<th axis="string">SRC</th>
			<th axis="string">SPORT</th>
			<th axis="string">FLOW</th>
			<th axis="string">DST</th>
			<th axis="string">DPORT</th>
			<th axis="string">SID</th>
			<th axis="date">Date</th>
		</thead>
		<tbody>
		<?php

		/* make sure alert file exists */
		if (!file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert"))
			@touch("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert");

		/* detect the alert file type */
		if ($snortalertlogt == 'full')
			$alerts_array = array_reverse(explode("\n\n", file_get_contents("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")));
		else
			$alerts_array = array_reverse(explode("\n", file_get_contents("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")));

		if (is_array($alerts_array)) {
			foreach($alerts_array as $counter => $fileline) {
				if (empty($fileline))
					continue;
				if ($counter > $anentries)
					break;

				/* Date */
				$alert_date_str = get_snort_alert_date($fileline);
				if($alert_date_str != '')
					$alert_date = $alert_date_str;
				else
					$alert_date = 'empty';

				/* Discription */
				$alert_disc_str = get_snort_alert_disc($fileline);
				if(empty($alert_disc_str))
					$alert_disc = 'empty';
				else
					$alert_disc = $alert_disc_str;

				/* Classification */
				$alert_class_str = get_snort_alert_class($fileline);
				if($alert_class_str != '')
				{
					$alert_class_match = array('[Classification:',']');
					$alert_class = str_replace($alert_class_match, '', "$alert_class_str");
				}else{
					$alert_class = 'Prep';
				}
					
				/* Priority */
				$alert_priority_str = get_snort_alert_priority($fileline);
				if($alert_priority_str != '')
				{
					$alert_priority_match = array('Priority: ',']');
					$alert_priority = str_replace($alert_priority_match, '', "$alert_priority_str");
				}else{
					$alert_priority = 'empty';
				}

				/* Protocol */
				/* Detect alert file type */
				if ($snortalertlogt == 'full')
				{
					$alert_proto_str = get_snort_alert_proto_full($fileline);
				}else{
					$alert_proto_str = get_snort_alert_proto($fileline);
				}

				if($alert_proto_str != '')
				{
					$alert_proto_match = array(" TTL",'{','}');
					$alert_proto = str_replace($alert_proto_match, '', "$alert_proto_str");
				}else{
					$alert_proto = 'empty';
				}
					
				/* IP SRC */
				$alert_ip_src_str = get_snort_alert_ip_src($fileline);
				if($alert_ip_src_str != '')
				{
					$alert_ip_src = $alert_ip_src_str;
				}else{
					$alert_ip_src = 'empty';
				}
					
				/* IP SRC Port */
				$alert_src_p_str = get_snort_alert_src_p($fileline);
				if($alert_src_p_str != '')
				{
					$alert_src_p_match = array(' -',':');
					$alert_src_p = str_replace($alert_src_p_match, '', "$alert_src_p_str");
				}else{
					$alert_src_p = 'empty';
				}

				/* Flow */
				$alert_flow_str = get_snort_alert_flow($fileline);
				if($alert_flow_str != '')
				{
					$alert_flow = $alert_flow_str;
				}else{
					$alert_flow = 'empty';
				}

				/* IP Destination */
				$alert_ip_dst_str = get_snort_alert_ip_dst($fileline);
				if($alert_ip_dst_str != '')
				{
					$alert_ip_dst = $alert_ip_dst_str;
				}else{
					$alert_ip_dst = 'empty';
				}

				/* IP DST Port */
				if ($snortalertlogt == 'full')
				{
					$alert_dst_p_str = get_snort_alert_dst_p_full($fileline);
				}else{
					$alert_dst_p_str = get_snort_alert_dst_p($fileline);
				}

				if($alert_dst_p_str != '')
				{
					$alert_dst_p_match = array(':',"\n"," TTL");
					$alert_dst_p_str2 = str_replace($alert_dst_p_match, '', "$alert_dst_p_str");
					$alert_dst_p_match2 = array('/[A-Z]/');
					$alert_dst_p = preg_replace($alert_dst_p_match2, '', "$alert_dst_p_str2");
				}else{
					$alert_dst_p = 'empty';
				}

				/* SID */
				$alert_sid_str = get_snort_alert_sid($fileline);

				if($alert_sid_str != '')
				{
					$alert_sid_match = array('[',']');
					$alert_sid = str_replace($alert_sid_match, '', "$alert_sid_str");
				}else{
					$alert_sid_str = 'empty';
				}

				/* NOTE: using one echo improves performance by 2x */
					echo "<tr id=\"{$counter}\">
				<td class=\"centerAlign\">{$counter}</td>
				<td class=\"centerAlign\">{$alert_priority}</td>
				<td class=\"centerAlign\">{$alert_proto}</td>
				<td>{$alert_disc}</td>
				<td class=\"centerAlign\">{$alert_class}</td>
				<td>{$alert_ip_src}</td>
				<td class=\"centerAlign\">{$alert_src_p}</td>
				<td class=\"centerAlign\">{$alert_flow}</td>
				<td>{$alert_ip_dst}</td>
				<td class=\"centerAlign\">{$alert_dst_p}</td>
				<td class=\"centerAlign\">{$alert_sid}</td>
				<td>{$alert_date}</td>
				</tr>\n";
			}
		}
		?>
		</tbody>
	</table>
	</td>
</table>
<?php
include("fend.inc");
?>
</body>
</html>
