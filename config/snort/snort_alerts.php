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

$snortalertlogt = $config['installedpackages']['snortglobal']['snortalertlogtype'];
$supplist = array();

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

if ($_POST['save']) {
	if (!is_array($config['installedpackages']['snortglobal']['alertsblocks']))
		$config['installedpackages']['snortglobal']['alertsblocks'] = array();
	$config['installedpackages']['snortglobal']['alertsblocks']['arefresh'] = $_POST['arefresh'] ? 'on' : 'off';
	$config['installedpackages']['snortglobal']['alertsblocks']['alertnumber'] = $_POST['alertnumber'];

	write_config();

	header("Location: /snort/snort_alerts.php?instance={$instanceid}");
	exit;
}

if ($_POST['todelete'] || $_GET['todelete']) {
	$ip = "";
	if($_POST['todelete'])
		$ip = $_POST['todelete'];
	else if($_GET['todelete'])
		$ip = $_GET['todelete'];
	if (is_ipaddr($ip)) {
		exec("/sbin/pfctl -t snort2c -T delete {$ip}");
		$savemsg = "Host IP address {$ip} has been removed from the Blocked Table.";
	}
}

if ($_GET['act'] == "addsuppress" && is_numeric($_GET['sidid']) && is_numeric($_GET['gen_id'])) {
	if (empty($_GET['descr']))
		$suppress = "suppress gen_id {$_GET['gen_id']}, sig_id {$_GET['sidid']}\n";
	else
		$suppress = "#{$_GET['descr']}\nsuppress gen_id {$_GET['gen_id']}, sig_id {$_GET['sidid']}";
	if (!is_array($config['installedpackages']['snortglobal']['suppress']))
		$config['installedpackages']['snortglobal']['suppress'] = array();
	if (!is_array($config['installedpackages']['snortglobal']['suppress']['item']))
		$config['installedpackages']['snortglobal']['suppress']['item'] = array();
	$a_suppress = &$config['installedpackages']['snortglobal']['suppress']['item'];

	if (empty($a_instance[$instanceid]['suppresslistname']) || $a_instance[$instanceid]['suppresslistname'] == 'default') {
		$s_list = array();
		$s_list['name'] = $a_instance[$instanceid]['interface'] . "suppress";
		$s_list['uuid'] = uniqid();
		$s_list['descr']  =  "Auto-generated list for suppress";
		$s_list['suppresspassthru'] = base64_encode($suppress);
		$a_suppress[] = $s_list;
		$a_instance[$instanceid]['suppresslistname'] = $s_list['name'];
	} else {
		foreach ($a_suppress as $a_id => $alist) {
			if ($alist['name'] == $a_instance[$instanceid]['suppresslistname']) {
				if (!empty($alist['suppresspassthru'])) {
					$tmplist = base64_decode($alist['suppresspassthru']);
					$tmplist .= "\n{$suppress}";
					$alist['suppresspassthru'] = base64_encode($tmplist);
					$a_suppress[$a_id] = $alist;
				}
			}
		}
	}
	$savemsg = "An entry for 'suppress gen_id {$_GET['gen_id']}, sig_id {$_GET['sidid']}' has been added to the Suppress List.";
	write_config();
	sync_snort_package_config();
}

if ($_GET['action'] == "clear" || $_POST['delete']) {
	conf_mount_rw();
	snort_post_delete_logs($snort_uuid);
	$fd = @fopen("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert", "w+");
	if ($fd)
		fclose($fd);
	conf_mount_ro();
	/* XXX: This is needed is snort is run as snort user */
	//mwexec('/usr/sbin/chown snort:snort /var/log/snort/*', true);
	mwexec('/bin/chmod 660 /var/log/snort/*', true);
	if (file_exists("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid"))
		mwexec("/bin/pkill -HUP -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid -a");
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
		@unlink("/tmp/{$file_name}");
	}

	header("Location: /snort/snort_alerts.php?instance={$instanceid}");
	exit;
}

/* Load up an array with the current Suppression List GID,SID values */
$supplist = snort_load_suppress_sigs($a_instance[$instanceid]);

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

<?php if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
	/* Display Alert message */
	if ($input_errors) {
		print_input_errors($input_errors); // TODO: add checks
	}
	if ($savemsg) {
		print_info_box($savemsg);
	}
?>
<form action="/snort/snort_alerts.php" method="post" id="formalert">
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
	$tab_array[7] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
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
				<td width="22%" class="vncell"><?php echo gettext('Instance to inspect'); ?></td>
				<td width="78%" class="vtable">
					<select name="instance" id="instance" class="formselect" onChange="document.getElementById('formalert').submit()">
			<?php
				foreach ($a_instance as $id => $instance) {
					$selected = "";
					if ($id == $instanceid)
						$selected = "selected";
					echo "<option value='{$id}' {$selected}> (" . snort_get_friendly_interface($instance['interface']) . "){$instance['descr']}</option>\n";
				}
			?>
					</select>&nbsp;&nbsp;<?php echo gettext('Choose which instance alerts you want to inspect.'); ?>
				</td>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Save or Remove Logs'); ?></td>
				<td width="78%" class="vtable">
					<input name="download" type="submit" class="formbtns" value="Download"> <?php echo gettext('All ' .
						'log files will be saved.'); ?>&nbsp;&nbsp;<a href="/snort/snort_alerts.php?action=clear&instance=<?=$instanceid;?>">
					<input name="delete" type="submit" class="formbtns" value="Clear"
					onclick="return confirm('Do you really want to remove all instance logs?')"></a>
					<span class="red"><strong><?php echo gettext('Warning:'); ?></strong></span> <?php echo ' ' . gettext('all log files will be deleted.'); ?>
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext('Auto Refresh and Log View'); ?></td>
				<td width="78%" class="vtable">
					<input name="save" type="submit" class="formbtns" value="Save">
					<?php echo gettext('Refresh'); ?> <input name="arefresh" type="checkbox" value="on"
					<?php if ($config['installedpackages']['snortglobal']['alertsblocks']['arefresh']=="on") echo "checked"; ?>>
						<?php printf(gettext('%sDefault%s is %sON%s.'), '<strong>', '</strong>', '<strong>', '</strong>'); ?>&nbsp;&nbsp;
					<input name="alertnumber" type="text" class="formfld" id="alertnumber" size="5" value="<?=htmlspecialchars($anentries);?>">
					<?php printf(gettext('Enter number of log entries to view. %sDefault%s is %s250%s.'), '<strong>', '</strong>', '<strong>', '</strong>'); ?>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php printf(gettext("Last %s Alert Entries"), $anentries); ?>&nbsp;&nbsp;
				<?php echo gettext("(Most recent entries are listed first)"); ?></td>
			</tr>
	<tr>
	<td width="100%" colspan="2">
	<table id="myTable" style="table-layout: fixed;" width="100%" class="sortable" border="1" cellpadding="0" cellspacing="0">
		<colgroup>
			<col width="9%" align="center" axis="date">
			<col width="45" align="center" axis="number">
			<col width="65" align="center" axis="string">
			<col width="10%" axis="string">
			<col width="13%" align="center" axis="string">
			<col width="8%" align="center" axis="string">
			<col width="13%" align="center" axis="string">
			<col width="8%" align="center" axis="string">
			<col width="9%" align="center" axis="number">
			<col axis="string">
		</colgroup>
		<thead>
			<th class="listhdrr" axis="date"><?php echo gettext("DATE"); ?></th>
			<th class="listhdrr" axis="number"><?php echo gettext("PRI"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("PROTO"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("CLASS"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("SRC"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("SPORT"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("DST"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("DPORT"); ?></th>
			<th class="listhdrr" axis="number"><?php echo gettext("SID"); ?></th>
			<th class="listhdrr" axis="string"><?php echo gettext("DESCRIPTION"); ?></th>
		</thead>
	<tbody>
	<?php

/* make sure alert file exists */
if (file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/alert")) {
	exec("tail -{$anentries} /var/log/snort/snort_{$if_real}{$snort_uuid}/alert | sort -r > /tmp/alert_{$snort_uuid}");
	if (file_exists("/tmp/alert_{$snort_uuid}")) {
		$tmpblocked = array_flip(snort_get_blocked_ips());
		$counter = 0;
		/*                 0         1           2      3      4    5    6    7      8     9    10    11             12    */
		/* File format timestamp,sig_generator,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,id,classification,priority */
		$fd = fopen("/tmp/alert_{$snort_uuid}", "r");
		while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
			if(count($fields) < 11)
				continue;

			/* Time */
			$alert_time = substr($fields[0], strpos($fields[0], '-')+1, -8);
			/* Date */
			$alert_date = substr($fields[0], 0, strpos($fields[0], '-'));
			/* Description */
			$alert_descr = $fields[4];
			$alert_descr_url = urlencode($fields[4]);
			/* Priority */
			$alert_priority = $fields[12];
			/* Protocol */
			$alert_proto = $fields[5];
			/* IP SRC */
			$alert_ip_src = $fields[6];
			if (isset($tmpblocked[$fields[6]])) {
				$alert_ip_src .= "<br/><a href='?instance={$id}&todelete=" . trim(urlencode($fields[6])) . "'>
				<img title=\"" . gettext("Remove host from Blocked Table") . "\" border=\"0\" width='10' height='10' name='todelete' id='todelete' alt=\"Remove from Blocked Hosts\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"/></a>"; 
			}
			/* IP SRC Port */
			$alert_src_p = $fields[7];
			/* IP Destination */
			$alert_ip_dst = $fields[8];
			if (isset($tmpblocked[$fields[8]])) {
				$alert_ip_dst .= "<br/><a href='?instance={$id}&todelete=" . trim(urlencode($fields[8])) . "'>
				<img title=\"" . gettext("Remove host from Blocked Table") . "\" border=\"0\" width='10' height='10' name='todelete' id='todelete' alt=\"Remove from Blocked Hosts\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"/></a>";
			}
			/* IP DST Port */
			$alert_dst_p = $fields[9];
			/* SID */
			$alert_sid_str = "{$fields[1]}:{$fields[2]}";
			if (!isset($supplist[$fields[1]][$fields[2]])) {
				$sidsupplink = "<a href='?instance={$instanceid}&act=addsuppress&sidid={$fields[2]}&gen_id={$fields[1]}&descr={$alert_descr_url}'>";
				$sidsupplink .= "<img src='../themes/{$g['theme']}/images/icons/icon_plus.gif' width='10' height='10' border='0' ";
				$sidsupplink .= "title='" . gettext("Add this gen_id:sig_id to Suppress List") . "'></a>";	
			}
			else {
				$sidsupplink = "<img src='../themes/{$g['theme']}/images/icons/icon_plus_d.gif' width='10' height='10' border='0' ";
				$sidsupplink .= "title='" . gettext("This gen_id:sig_id already in Suppress List") . "'/>";	
			}
			$alert_class = $fields[11];

			echo "<tr>
				<td class='listr' align='center'>{$alert_date}<br/>{$alert_time}</td>
				<td class='listr' align='center'>{$alert_priority}</td>
				<td class='listr' align='center'>{$alert_proto}</td>
				<td class='listr' style=\"word-wrap:break-word;\">{$alert_class}</td>
				<td class='listr' align='center'>{$alert_ip_src}</td>
				<td class='listr' align='center'>{$alert_src_p}</td>
				<td class='listr' align='center'>{$alert_ip_dst}</td>
				<td class='listr' align='center'>{$alert_dst_p}</td>
				<td class='listr' align='center'>{$alert_sid_str}<br/>{$sidsupplink}</td>
				<td class='listr' style=\"word-wrap:break-word;\">{$alert_descr}</td>
				</tr>\n";

			$counter++;
		}
		fclose($fd);
		@unlink("/tmp/alert_{$snort_uuid}");
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
</body>
</html>
