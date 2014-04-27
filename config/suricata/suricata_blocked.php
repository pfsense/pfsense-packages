<?php
/*
 * suricata_blocked.php
 *
 * Copyright (C) 2014 Bill Meeks
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

$suricatalogdir = SURICATALOGDIR;
$suri_pf_table = SURICATA_PF_TABLE;

if (!is_array($config['installedpackages']['suricata']['alertsblocks']))
	$config['installedpackages']['suricata']['alertsblocks'] = array();

$pconfig['brefresh'] = $config['installedpackages']['suricata']['alertsblocks']['brefresh'];
$pconfig['blertnumber'] = $config['installedpackages']['suricata']['alertsblocks']['blertnumber'];

if (empty($pconfig['blertnumber']))
	$bnentries = '500';
else
	$bnentries = $pconfig['blertnumber'];

if ($_POST['todelete']) {
	$ip = "";
	if ($_POST['ip'])
		$ip = $_POST['ip'];
	if (is_ipaddr($ip))
		exec("/sbin/pfctl -t {$suri_pf_table} -T delete {$ip}");
	else
		$input_errors[] = gettext("An invalid IP address was provided as a parameter.");
}

if ($_POST['remove']) {
	exec("/sbin/pfctl -t {$suri_pf_table} -T flush");
	header("Location: /suricata/suricata_blocked.php");
	exit;
}

/* TODO: build a file with block ip and disc */
if ($_POST['download'])
{
	$blocked_ips_array_save = "";
	exec("/sbin/pfctl -t {$suri_pf_table} -T show", $blocked_ips_array_save);
	/* build the list */
	if (is_array($blocked_ips_array_save) && count($blocked_ips_array_save) > 0) {
		$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
		$file_name = "suricata_blocked_{$save_date}.tar.gz";
		exec('/bin/mkdir -p /tmp/suricata_blocked');
		file_put_contents("/tmp/suricata_blocked/suricata_block.pf", "");
		foreach($blocked_ips_array_save as $counter => $fileline) {
			if (empty($fileline))
				continue;
			$fileline = trim($fileline, " \n\t");
			file_put_contents("/tmp/suricata_blocked/suricata_block.pf", "{$fileline}\n", FILE_APPEND);
		}

		// Create a tar gzip archive of blocked host IP addresses
		exec("/usr/bin/tar -czf /tmp/{$file_name} -C/tmp/suricata_blocked suricata_block.pf");

		// If we successfully created the archive, send it to the browser.
		if(file_exists("/tmp/{$file_name}")) {
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

			// Clean up the temp files and directory
			@unlink("/tmp/{$file_name}");
			exec("/bin/rm -fr /tmp/suricata_blocked");
		} else
			$savemsg = gettext("An error occurred while creating archive");
	} else
		$savemsg = gettext("No content on suricata block list");
}

if ($_POST['save'])
{
	/* no errors */
	if (!$input_errors) {
		$config['installedpackages']['suricata']['alertsblocks']['brefresh'] = $_POST['brefresh'] ? 'on' : 'off';
		$config['installedpackages']['suricata']['alertsblocks']['blertnumber'] = $_POST['blertnumber'];

		write_config("Suricata pkg: updated BLOCKED tab settings.");

		header("Location: /suricata/suricata_blocked.php");
		exit;
	}

}

$pgtitle = gettext("Suricata: Blocked Hosts");
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">
<script src="/javascript/filter_log.js" type="text/javascript"></script>

<?php

include_once("fbegin.inc");

/* refresh every 60 secs */
if ($pconfig['brefresh'] == 'on')
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/suricata/suricata_blocked.php\" />\n";

/* Display Alert message */
if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
}
if ($savemsg) {
	print_info_box($savemsg);
}
?>

<form action="/suricata/suricata_blocked.php" method="post">
<input type="hidden" name="ip" id="ip" value=""/>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td>
	<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Suricata Interfaces"), false, "/suricata/suricata_interfaces.php");
	$tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
	$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php");
	$tab_array[] = array(gettext("Blocked"), true, "/suricata/suricata_blocked.php");
	$tab_array[] = array(gettext("Pass Lists"), false, "/suricata/suricata_passlist.php");
	$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php?instance={$instanceid}");
	$tab_array[] = array(gettext("Logs Mgmt"), false, "/suricata/suricata_logs_mgmt.php");
	display_top_tabs($tab_array, true);
	?>
	</td>
</tr>
<tr>
	<td><div id="mainarea">
		<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Blocked Hosts Log View Settings"); ?></td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext("Save or Remove Hosts"); ?></td>
				<td width="78%" class="vtable">
				<input name="download" type="submit" class="formbtns" value="Download" title="<?=gettext("Download list of blocked hosts as a gzip archive");?>"/>
				&nbsp;<?php echo gettext("All blocked hosts will be saved."); ?>&nbsp;&nbsp;
				<input name="remove" type="submit" class="formbtns" value="Clear" title="<?=gettext("Remove blocks for all listed hosts");?>" 
				onClick="return confirm('<?=gettext("Are you sure you want to remove all blocked hosts?  Click OK to continue or CANCLE to quit.");?>');"/>&nbsp;
				<span class="red"><strong><?php echo gettext("Warning:"); ?></strong></span>&nbsp;<?php echo gettext("all hosts will be removed."); ?>
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext("Auto Refresh and Log View"); ?></td>
				<td width="78%" class="vtable">
				<input name="save" type="submit" class="formbtns" value=" Save " title="<?=gettext("Save auto-refresh and view settings");?>"/>
				&nbsp;&nbsp;<?php echo gettext("Refresh"); ?>&nbsp;<input name="brefresh" type="checkbox" value="on" 
				<?php if ($config['installedpackages']['suricata']['alertsblocks']['brefresh']=="on" || $config['installedpackages']['suricata']['alertsblocks']['brefresh']=='') echo "checked"; ?>/>
				&nbsp;<?php printf(gettext("%sDefault%s is %sON%s."), '<strong>', '</strong>', '<strong>', '</strong>'); ?>&nbsp;&nbsp;
				<input name="blertnumber" type="text" class="formfld unknown" id="blertnumber" 
				size="5" value="<?=htmlspecialchars($bnentries);?>"/>&nbsp;<?php printf(gettext("Enter number of " .
				"blocked entries to view. %sDefault%s is %s500%s."), '<strong>', '</strong>', '<strong>', '</strong>'); ?>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php printf(gettext("Last %s Hosts Blocked by Suricata"), $bnentries); ?></td>
			</tr>
			<tr>
				<td colspan="2">
				<table id="sortabletable1" style="table-layout: fixed;" class="sortable" width="100%" border="0" cellpadding="2" cellspacing="0">
					<colgroup>
						<col width="5%" align="center" axis="number">
						<col width="15%" align="center" axis="string">
						<col width="70%" align="left" axis="string">
						<col width="10%" align="center">
					</colgroup>
					<thead>
					   <tr>
						<th class="listhdrr" axis="number">#</th>
						<th class="listhdrr" axis="string"><?php echo gettext("IP"); ?></th>
						<th class="listhdrr" axis="string"><?php echo gettext("Alert Description"); ?></th>
						<th class="listhdrr"><?php echo gettext("Remove"); ?></th>
					   </tr>
					</thead>
				<tbody>
		<?php

		/* set the arrays */
		$blocked_ips_array = suricata_get_blocked_ips();
		if (!empty($blocked_ips_array)) {
			foreach ($blocked_ips_array as &$ip)
				$ip = inet_pton($ip);
			$tmpblocked = array_flip($blocked_ips_array);
			$src_ip_list = array();
			foreach (glob("{$suricatalogdir}*/block.log*") as $alertfile) {
				$fd = fopen($alertfile, "r");
				if ($fd) {
					/*	       0         1      2             3      4       5   6              7        8     9  10   */
					/* File format timestamp,action,sig_generator,sig_id,sig_rev,msg,classification,priority,proto,ip,port */
					while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
						if(count($fields) < 11)
							continue;
						$fields[9] = inet_pton($fields[9]);
						if (isset($tmpblocked[$fields[9]])) {
							if (!is_array($src_ip_list[$fields[9]]))
								$src_ip_list[$fields[9]] = array();
							$src_ip_list[$fields[9]][$fields[5]] = "{$fields[5]} - " . substr($fields[0], 0, -7);
						}
					}
					fclose($fd);
				}
			}

			foreach($blocked_ips_array as $blocked_ip) {
				if (is_ipaddr($blocked_ip) && !isset($src_ip_list[$blocked_ip]))
					$src_ip_list[$blocked_ip] = array("N\A\n");
			}

			/* build final list, build html */
			$counter = 0;
			foreach($src_ip_list as $blocked_ip => $blocked_msg) {
				$blocked_desc = implode("<br/>", $blocked_msg);
				if($counter > $bnentries)
					break;
				else
					$counter++;

				$block_ip_str = inet_ntop($blocked_ip);
				/* Add zero-width space as soft-break opportunity after each colon if we have an IPv6 address */
				$tmp_ip = str_replace(":", ":&#8203;", $block_ip_str);
				/* Add reverse DNS lookup icons */
				$rdns_link = "";
				$rdns_link .= "<a onclick=\"javascript:getURL('/diag_dns.php?host={$block_ip_str}&dialog_output=true', outputrule);\">";
				$rdns_link .= "<img src='../themes/{$g['theme']}/images/icons/icon_log_d.gif' width='11' height='11' border='0' ";
				$rdns_link .= "title='" . gettext("Resolve host via reverse DNS lookup (quick pop-up)") . "' style=\"cursor: pointer;\"></a>&nbsp;";
				$rdns_link .= "<a href='/diag_dns.php?host={$block_ip_str}'>";
				$rdns_link .= "<img src='../themes/{$g['theme']}/images/icons/icon_log.gif' width='11' height='11' border='0' ";
				$rdns_link .= "title='" . gettext("Resolve host via reverse DNS lookup") . "'></a>";
				/* use one echo to do the magic*/
					echo "<tr>
						<td align=\"center\" valign=\"middle\" class=\"listr\">{$counter}</td>
						<td align=\"center\" valign=\"middle\" class=\"listr\">{$tmp_ip}<br/>{$rdns_link}</td>
						<td valign=\"middle\" class=\"listr\">{$blocked_desc}</td>
						<td align=\"center\" valign=\"middle\" class=\"listr\" sorttable_customkey=\"\">
						<input type=\"image\" name=\"todelete[]\" onClick=\"document.getElementById('ip').value='{$block_ip_str}';\" 
						src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\" title=\"" . gettext("Delete host from Blocked Table") . "\" border=\"0\" /></td>
					</tr>\n";
			}
		}
		?>
					</tbody>
				</table>
			</td>
		</tr>
		<tr>
			<td colspan="2" class="vexpl" align="center">
			<?php	if (!empty($blocked_ips_array)) {
					if ($counter > 1)
						echo "{$counter}" . gettext(" host IP addresses are currently being blocked.");
					else
						echo "{$counter}" . gettext(" host IP address is currently being blocked.");
				}
				else {
					echo gettext("There are currently no hosts being blocked by Suricata.");
				}
			?>
			</td>
		</tr>
	</table>
	</div>
	</td>
</tr>
</table>
</form>
<?php
include("fend.inc");
?>
</body>
</html>
