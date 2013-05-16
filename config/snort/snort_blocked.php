<?php
/*
 * snort_blocked.php
 *
 * Copyright (C) 2006 Scott Ullrich
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

if (!is_array($config['installedpackages']['snortglobal']['alertsblocks']))
	$config['installedpackages']['snortglobal']['alertsblocks'] = array();

$pconfig['brefresh'] = $config['installedpackages']['snortglobal']['alertsblocks']['brefresh'];
$pconfig['blertnumber'] = $config['installedpackages']['snortglobal']['alertsblocks']['blertnumber'];

if (empty($pconfig['blertnumber']))
	$bnentries = '500';
else
	$bnentries = $pconfig['blertnumber'];

if ($_POST['todelete'] || $_GET['todelete']) {
	$ip = "";
	if($_POST['todelete'])
		$ip = $_POST['todelete'];
	else if($_GET['todelete'])
		$ip = $_GET['todelete'];
	if (is_ipaddr($ip))
		exec("/sbin/pfctl -t snort2c -T delete {$ip}");
}

if ($_POST['remove']) {
	exec("/sbin/pfctl -t snort2c -T flush");
	header("Location: /snort/snort_blocked.php");
	exit;
}

/* TODO: build a file with block ip and disc */
if ($_POST['download'])
{
	$blocked_ips_array_save = "";
	exec('/sbin/pfctl -t snort2c -T show', $blocked_ips_array_save);
	/* build the list */
	if (is_array($blocked_ips_array_save) && count($blocked_ips_array_save) > 0) {
		ob_start(); //important or other posts will fail
		$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
		$file_name = "snort_blocked_{$save_date}.tar.gz";
		exec('/bin/mkdir -p /tmp/snort_blocked');
		file_put_contents("/tmp/snort_blocked/snort_block.pf", "");
		foreach($blocked_ips_array_save as $counter => $fileline) {
			if (empty($fileline))
				continue;
			$fileline = trim($fileline, " \n\t");
			file_put_contents("/tmp/snort_blocked/snort_block.pf", "{$fileline}\n", FILE_APPEND);
		}

		exec("/usr/bin/tar cf /tmp/{$file_name} /tmp/snort_blocked");

		if(file_exists("/tmp/{$file_name}")) {
			header("Expires: Mon, 26 Jul 1997 05:00:00 GMT\n");
			header("Pragma: private"); // needed for IE
			header("Cache-Control: private, must-revalidate"); // needed for IE
			header('Content-type: application/force-download');
			header('Content-Transfer-Encoding: Binary');
			header("Content-length: " . filesize("/tmp/{$file_name}"));
			header("Content-disposition: attachment; filename = {$file_name}");
			readfile("/tmp/{$file_name}");
			ob_end_clean(); //importanr or other post will fail
			@unlink("/tmp/{$file_name}");
			exec("/bin/rm -fr /tmp/snort_blocked");
		} else
			$savemsg = "An error occurred while createing archive";
	} else
		$savemsg = "No content on snort block list";
}

if ($_POST['save'])
{
	/* no errors */
	if (!$input_errors) {
		$config['installedpackages']['snortglobal']['alertsblocks']['brefresh'] = $_POST['brefresh'] ? 'on' : 'off';
		$config['installedpackages']['snortglobal']['alertsblocks']['blertnumber'] = $_POST['blertnumber'];

		write_config();

		header("Location: /snort/snort_blocked.php");
		exit;
	}

}

$pgtitle = "Services: Snort Blocked Hosts";
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php

include_once("fbegin.inc");

/* refresh every 60 secs */
if ($pconfig['brefresh'] == 'on')
	echo "<meta http-equiv=\"refresh\" content=\"60;url=/snort/snort_blocked.php\" />\n";
?>

<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php if ($savemsg) print_info_box($savemsg); ?>
<form action="/snort/snort_blocked.php" method="post">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td>
		<?php
		$tab_array = array();
		$tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
		$tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
		$tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
		$tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
		$tab_array[4] = array(gettext("Blocked"), true, "/snort/snort_blocked.php");
		$tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
		$tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
        $tab_array[7] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
		display_top_tabs($tab_array);
		?>
	</td>
</tr>
<tr>
	<td>
		<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Blocked Hosts Log View Settings"); ?></td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext("Save or Remove Hosts"); ?></td>
				<td width="78%" class="vtable">
					<input name="download" type="submit" class="formbtns" value="Download"> <?php echo gettext("All " .
				"blocked hosts will be saved."); ?>&nbsp;&nbsp;<input name="remove" type="submit"
					class="formbtns" value="Clear">&nbsp;<span class="red"><strong><?php echo gettext("Warning:"); ?></strong></span>
				<?php echo gettext("all hosts will be removed."); ?>
				</td>
			</tr>
			<tr>
				<td width="22%" class="vncell"><?php echo gettext("Auto Refresh and Log View"); ?></td>
				<td width="78%" class="vtable">
					<input name="save" type="submit" class="formbtns" value="Save"> <?php echo gettext("Refresh"); ?> <input
					name="brefresh" type="checkbox" value="on"
					<?php if ($config['installedpackages']['snortglobal']['alertsblocks']['brefresh']=="on" || $config['installedpackages']['snortglobal']['alertsblocks']['brefresh']=='') echo "checked"; ?>>
				<?php printf(gettext("%sDefault%s is %sON%s."), '<strong>', '</strong>', '<strong>', '</strong>'); ?>&nbsp;&nbsp;<input
					name="blertnumber" type="text" class="formfld" id="blertnumber"
					size="5" value="<?=htmlspecialchars($bnentries);?>"> <?php printf(gettext("Enter the " .
				"number of blocked entries to view. %sDefault%s is %s500%s."), '<strong>', '</strong>', '<strong>', '</strong>'); ?>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php printf(gettext("Last %s Hosts Blocked by Snort"), $bnentries); ?></td>
			</tr>
			<tr>
				<td colspan="2">
				<table id="sortabletable1" style="table-layout: fixed;" class="sortable" width="100%" border="0" cellpadding="2" cellspacing="0">
					<thead>
						<th class='listhdr' width="5%" class="listhdrr" align="center" axis="number">#</th>
						<th class='listhdr' width="15%" class="listhdrr" align="center" axis="string"><?php echo gettext("IP"); ?></th>
						<th class='listhdr' width="70%" class="listhdrr" axis="string"><?php echo gettext("Alert Description"); ?></th>
						<th class='listhdr' width="10%" class="listhdrr" align="center"><?php echo gettext("Remove"); ?></th>
					</thead>
				<tbody>
			<?php
			/* set the arrays */
			$blocked_ips_array = array();
			if (is_array($blocked_ips)) {
				foreach ($blocked_ips as $blocked_ip) {
					if (empty($blocked_ip))
						continue;
					$blocked_ips_array[] = trim($blocked_ip, " \n\t");
				}
			}
		$blocked_ips_array = snort_get_blocked_ips();
		if (!empty($blocked_ips_array)) {
			$tmpblocked = array_flip($blocked_ips_array);
			$src_ip_list = array();
			foreach (glob("/var/log/snort/*/alert") as $alertfile) {
				$fd = fopen($alertfile, "r");
				if ($fd) {
					/*                 0         1           2      3      4    5    6    7      8     9    10    11             12
					/* File format timestamp,sig_generator,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,id,classification,priority */
					while (($fields = fgetcsv($fd, 1000, ',', '"')) !== FALSE) {
						if(count($fields) < 11)
							continue;
					
						if (isset($tmpblocked[$fields[6]])) {
							if (!is_array($src_ip_list[$fields[6]]))
								$src_ip_list[$fields[6]] = array();
							$src_ip_list[$fields[6]][$fields[4]] = "{$fields[4]} - " . substr($fields[0], 0, -8);
						}
						if (isset($tmpblocked[$fields[8]])) {
							if (!is_array($src_ip_list[$fields[8]]))
								$src_ip_list[$fields[8]] = array();
							$src_ip_list[$fields[8]][$fields[4]] = "{$fields[4]} - " . substr($fields[0], 0, -8);
						}
					}
					fclose($fd);
				}
			}

			foreach($blocked_ips_array as $blocked_ip) {
				if (is_ipaddr($blocked_ip) && !isset($src_ip_list[$blocked_ip]))
					$src_ip_list[$blocked_ip] = array("N\A\n");
			}

			/* build final list, preg_match, build html */
			$counter = 0;
			foreach($src_ip_list as $blocked_ip => $blocked_msg) {
				$blocked_desc = implode("<br/>", $blocked_msg);
				if($counter > $bnentries)
					break;
				else
					$counter++;

				/* use one echo to do the magic*/
					echo "<tr>
						<td width='5%' align=\"center\" valign=\"middle\" class=\"listr\">{$counter}</td>
						<td width='15%' valign=\"middle\" class=\"listr\">{$blocked_ip}</td>
						<td width='70%' valign=\"middle\" class=\"listr\">{$blocked_desc}</td>
						<td width='10%' align=\"center\" valign=\"middle\" class=\"listr\"><a href='snort_blocked.php?todelete=" . trim(urlencode($blocked_ip)) . "'>
						<img title=\"" . gettext("Delete host from Blocked Table") . "\" border=\"0\" name='todelete' id='todelete' alt=\"Delete host from Blocked Table\" src=\"../themes/{$g['theme']}/images/icons/icon_x.gif\"></a></td>
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
					echo gettext("There are currently no hosts being blocked by Snort.");
				}
			?>
			</td>
		</tr>
	</table>
	</td>
</tr>
</table>
</form>
<?php
include("fend.inc");
?>
</body>
</html>
