<?php

/* pfBlockerNG_Update.php

	pfBlockerNG
	Copyright (C) 2014 BBcan177@gmail.com
	All rights reserved.

	Portions of this code are based on original work done for
	pfSense from the following contributors:

	pkg_mgr_install.php
	Part of pfSense (https://www.pfsense.org)
	Copyright (C) 2004-2010 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2005 Colin Smith
	All rights reserved.

	Hide/Show Rollup Code originally coded in Suricata_alerts.php
	Copyright (C) 2014 Bill Meeks
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

require_once("guiconfig.inc");
require_once("globals.inc");
require_once("pfsense-utils.inc");
require_once("functions.inc");
require_once("/usr/local/pkg/pfblockerng/pfblockerng.inc");

pfb_global();

$filterlogentries = FALSE;
$pfblog = "{$pfb['log']}";
$pfb_output = "";

// Collect pfBlockerNG log file and post Live output to Terminal window.
function pfbupdate_output($text) {
	$text = preg_replace("/\n/", "\\n", $text);
	echo "\n<script type=\"text/javascript\">";
	echo "\n//<![CDATA[";
	echo "\nthis.document.forms[0].pfb_output.value = \"" . $text . "\";";
	echo "\nthis.document.forms[0].pfb_output.scrollTop = this.document.forms[0].pfb_output.scrollHeight;";
	echo "\n//]]>";
	echo "\n</script>";
	/* ensure that contents are written out */
	ob_flush();
}

// Post Status Message to Terminal window.
function pfbupdate_status($status) {
	$status =  preg_replace("/\n/", "\\n", $status);
	echo "\n<script type=\"text/javascript\">";
	echo "\n//<![CDATA[";
	echo "\nthis.document.forms[0].pfb_status.value=\"" . $status . "\";";
	echo "\n//]]>";
	echo "\n</script>";
	/* ensure that contents are written out */
	ob_flush();
}


$pgtitle = gettext("pfBlockerNG: Update");
include_once("head.inc");
?>
<body link="#000000" vlink="#0000CC" alink="#000000">
<form action="/pfblockerng/pfblockerng_update.php" method="post">

<?php
include_once("fbegin.inc");
?>
	<div id="mainareapkg">
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td>
					<?php
						$tab_array = array();
						$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=pfblockerng.xml&amp;id=0");
						$tab_array[] = array(gettext("Update"), true, "/pfblockerng/pfblockerng_update.php");
						$tab_array[] = array(gettext("Alerts"), false, "/pfblockerng/pfblockerng_alerts.php");
						$tab_array[] = array(gettext("Reputation"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_reputation.xml&id=0");
						$tab_array[] = array(gettext("IPv4"), false, "/pkg.php?xml=/pfblockerng/pfblockerng_v4lists.xml");
						$tab_array[] = array(gettext("IPv6"), false, "/pkg.php?xml=/pfblockerng/pfblockerng_v6lists.xml");
						$tab_array[] = array(gettext("Top 20"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_top20.xml&id=0");
						$tab_array[] = array(gettext("Africa"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Africa.xml&id=0");
						$tab_array[] = array(gettext("Asia"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Asia.xml&id=0");
						$tab_array[] = array(gettext("Europe"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Europe.xml&id=0");
						$tab_array[] = array(gettext("N.A."), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_NorthAmerica.xml&id=0");
						$tab_array[] = array(gettext("Oceania"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_Oceania.xml&id=0");
						$tab_array[] = array(gettext("S.A."), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_SouthAmerica.xml&id=0");
						$tab_array[] = array(gettext("Logs"), false, "/pfblockerng/pfblockerng_log.php");
						$tab_array[] = array(gettext("Sync"), false, "/pkg_edit.php?xml=/pfblockerng/pfblockerng_sync.xml&id=0");
						display_top_tabs($tab_array, true);
					?>
				</td>
			</tr>
		</table>
		<table id="maintable" class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tr>
				<td colspan="3" class="vncell" align="left"><?php echo gettext("LINKS :"); ?>&nbsp;
				<a href='/firewall_aliases.php' target="_blank"><?php echo gettext("Firewall Alias"); ?></a> &nbsp;
				<a href='/firewall_rules.php' target="_blank"><?php echo gettext("Firewall Rules"); ?></a> &nbsp;
				<a href='/diag_logs_filter.php' target="_blank"><?php echo gettext("Firewall Logs"); ?></a><br>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Live Log Viewer only"); ?></td>
			</tr>
				<td colspan="2" <?php echo gettext("<br>"); ?></td>
			<tr>
				<td colspan="2" class="vncell">
					<input type="submit" class="formbtns" name="pfbview" id="pfbview" value="VIEW" title="<?=gettext("VIEW pfBlockerNG LOG");?>"/>
					<input type="submit" class="formbtns" name="pfbviewcancel" id="pfbviewcancel" value="End View" title="<?=gettext("END VIEW of pfBlockerNG LOG");?>"/>
					<?php echo "&nbsp;&nbsp;" . gettext(" Select 'view' to open ") . "<strong>" . gettext(' pfBlockerNG ') . "</strong>" . gettext(" Log : &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; (Select 'End View' to terminate the viewer) <br><br>"); ?>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("CRON Status"); ?></td
			</tr>
			<tr>
				<td colspan="2" class="vncell">
				<?php
					// Collect Existing CRON settings
					if (is_array($config['cron']['item'])) {
						foreach ($config['cron']['item'] as $cron) {
							if (preg_match("/usr.local.www.pfblockerng.pfblockerng.php cron/",$cron["command"])) {
								$pfb_min = "{$cron['minute']}";
								break;
							}
						}
					}

					// Calculate Minutes Remaining till next CRON Event.
					$currentmin = date('i');
					switch ($pfb_min) {
						case "0":
							$min_remain = (60 - $currentmin);
							break;
						case "15":
							if ($currentmin < 15) {
								$min_remain = (15 - $currentmin);
							} else {
								$min_remain = (75 - $currentmin);
							}
							break;
						case "30":
							if ($currentmin < 30) {
								$min_remain = (30 - $currentmin);
							} else {
								$min_remain = (90 - $currentmin);
							}
							break;
						case "45":
							if ($currentmin < 45) {
								$min_remain = (45 - $currentmin);
							} else {
								$min_remain = (105 - $currentmin);
							}
							break;
					}

					// Default to "< 1 minute" if empty
					if (empty($min_remain))
						$min_remain = "< 1";
					if (empty($pfb['enable']))
						$min_remain = " [ Disabled ] ";

					echo "NEXT Scheduled CRON Event will run in :&nbsp; <font size=\"5\"><span class=\"red\">{$min_remain}</span></font> Minutes.";
					// Query for any Active pfBlockerNG CRON Jobs
					$result_cron = array();
					$cron_event = exec ("/bin/ps -wax", $result_cron); 
					if (preg_grep("/pfblockerng[.]php\s+cron/", $result_cron)) { 
						echo "<font size=\"2\"><span class=\"red\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Active pfBlockerNG CRON Job </span></font>&nbsp;&nbsp;";
						echo "<img src = '/themes/{$g['theme']}/images/icons/icon_pass.gif' width='15' height='15' border='0' title='pfBockerNG Cron Task is Running.'/>";
					}

					echo "<br><font size=\"3\"><span class=\"red\">Refresh</span></font> to update current Status and Minute(s) remaining<br>";
				?>
				</td>
			</tr>
			<tr>
				<td colspan="2" class="listtopic"><?php echo gettext("Run Manual Process Update"); ?></td>
			</tr>
			<tr id="filter_enable_row" style="display:<?php if (!$filterlogentries) {echo "table-row;";} else {echo "none;";} ?>">
				<td width="78%" class="vtable">
					<input name="show_filter" id="show_filter" type="button" class="formbtns" value="<?=gettext("Show Option");?>" onclick="enable_showFilter();" title="<?=gettext("Click to display advanced Manual Update options dialog");?>" />
				</td>
			</tr>

			<tr id="filter_options_row" style="display:<?php if ($filterlogentries) {echo "table-row;";} else {echo "none;";} ?>">
				<td colspan="2">
					<table style="height:100;colspacing:0" width="630" border="0" cellpadding="0" cellspacing="0" summary="images">
				<td class="listr"><?php echo "<span class='red'><strong>" . gettext("** AVOID ** ") . "&nbsp;" . "</strong></span>" . gettext("Running a -Manual Update- when CRON is expected to RUN!"); ?></td><tr>
				<td width="78%" class="tabcont" align="left">
					<input type="submit" class="formbtns" name="pfbconfirm" id="pfbconfirm" value="Manual Update" title="<?=gettext("Execute MANUAL pfBlockerNG UPDATE");?>" />
					<input type="submit" class="formbtns" name="pfbcancel" id="pfbcancel" value="Cancel" title="<?=gettext("Cancel MANUAL pfBlockerNG UPDATE");?>" />
				</td>
			</tr>
				<td width="100%" class="listr"><?php echo "<span class='red'><strong>" . gettext("NOTE: ") . "</strong></span>" . "&nbsp;" . gettext("Running this command will 'Disable' the next Scheduled pfBlockerNG CRON job until this process has completed.") ;?><tr><td></tr>
							<td width="78%" class="tabcont" align="left">
								<input name="Hide_filter" id="filterlogentries_hide" name="filterlogentries_hide" type="button" class="formbtns" value="<?=gettext("Hide Option");?>" onclick="enable_hideFilter();" title="<?=gettext("Hide Advanced options");?>" /></div>
							</td>
					</table>
				</td>
			</tr>
			<tr>
				<td class="tabcont" align="left">
					<!-- status box -->
					<textarea cols="80" rows="1" name="pfb_status" id="pfb_status" wrap="hard"><?=gettext("Manual Update Viewer Standby");?></textarea>
				</td>
			</tr>
			<tr>
				<td>
					<!-- command output box -->
					<textarea cols="80" rows="35" name="pfb_output" id="pfb_output" wrap="hard"></textarea>
				</td>
			</tr>
		</table>
	</div>
</form>

<?php include("fend.inc"); ?>
<script type="text/javascript">
//<![CDATA[
NiftyCheck();
	Rounded("div#mainareapkg","bl br","#FFF","#eeeeee","smooth");
//]]>

function enable_showFilter() {
	document.getElementById("filter_enable_row").style.display="none";
	document.getElementById("filter_options_row").style.display="table-row";
}

function enable_hideFilter() {
	document.getElementById("filter_enable_row").style.display="table-row";
	document.getElementById("filter_options_row").style.display="none";
}

</script>

<?php

if (isset($_POST['pfbview'])) {

	if (!file_exists($pfblog))
		touch($pfblog);

	// Reference: http://stackoverflow.com/questions/3218895/php-how-to-read-a-file-live-that-is-constantly-being-written-to
	pfbupdate_status(gettext("Manual Update Viewing in process.    ** Press 'END VIEW' to Exit ** "));
	$lastpos_old = "";
	$len = filesize($pfblog);

	// Start at EOF ( - 15000)
	if ($len > 15000) {
		$lastpos = ($len - 15000);
	} else {
		$lastpos = 0;
	}

	while (true) {
		usleep(300000); //0.3s
		clearstatcache(false,$pfblog);
		$len = filesize($pfblog);
		if ($len < $lastpos) {
			//file deleted or reset
			$lastpos = $len;
		} else {
			$f = fopen($pfblog, "rb");
			if ($f === false)
				die();
			fseek($f, $lastpos);

			while (!feof($f)) {

				$pfb_buffer = fread($f, 4096);
				$pfb_output .= str_replace( "\r", "", $pfb_buffer);

				// Refresh on new lines only
				if ($lastpos != $lastpos_old) {
					pfbupdate_output($pfb_output);
				}
				$lastpos_old = $lastpos;
				ob_flush();
				flush();
			}
			$lastpos = ftell($f);
			fclose($f);
		}
	}
}


if (isset($_POST['pfbviewcancel'])) {
	clearstatcache(false,$pfblog);
	ob_flush();
	flush();
	fclose($pfblog);
}


if (isset($_POST['pfbconfirm'])) {

	// Query for any Active pfBlockerNG CRON Jobs
	$result_cron = array();
	$cron_event = exec ("/bin/ps -wx", $result_cron);
	if (preg_grep("/pfblockerng[.]php\s+cron/", $result_cron)) {
		pfbupdate_status(gettext("Manual Update Standby - Previous Attempt Failed due to Active Running CRON Task"));
		exit;
	}

	if (!file_exists($pfblog))
		touch($pfblog);
	pfbupdate_status(gettext("Manual CRON Job will run within ( ** one minute ** ). Please Wait."));
	// Remove any existing pfBlockerNG CRON Jobs
	install_cron_job("pfblockerng.php cron", false);
	install_cron_job("pfblockerng.php update", false);
	write_config();

	// Define 'Manual' pfBlockerNG CRON Job
	$date_min_now = date('i');
	$pfb_cmd = "/usr/local/bin/php /usr/local/www/pfblockerng/pfblockerng.php update >> {$pfb['log']} 2>&1";
	$pfb_c_min = ($date_min_now + 1);
	if ($pfb_c_min == "60")
		$pfb_c_min = 0;
	$pfb_hour = "*";
	$pfb_mday = "*";
	$pfb_month = "*";
	$pfb_wday = "*";
	$pfb_who = "root";

	// Write CRON job (When pfBlockerNG CRON Task is complete. It will re-enable User Defined CRON Job)
	install_cron_job($pfb_cmd, true, $pfb_c_min, $pfb_hour, $pfb_mday, $pfb_month, $pfb_wday, $pfb_who);
	write_config();

	// Start at EOF
	$lastpos_old = "";
	$len = filesize($pfblog);
	$lastpos = $len;

	while (true) {
		usleep(300000); //0.3s
		clearstatcache(false,$pfblog);
		$len = filesize($pfblog);
		if ($len < $lastpos) {
			//file deleted or reset
			$lastpos = $len;
		} else {
			$f = fopen($pfblog, "rb");
			if ($f === false)
				die();
			fseek($f, $lastpos);

			while (!feof($f)) {

				$pfb_buffer = fread($f, 2048);
				$pfb_output .= str_replace( "\r", "", $pfb_buffer);

				// Refresh on new lines only
				if ($lastpos != $lastpos_old)
					pfbupdate_output($pfb_output);
				$lastpos_old = $lastpos;
				ob_flush();
				flush();
			}
			$lastpos = ftell($f);
			fclose($f);
		}
		// Capture Remaining Output before closing File
		if (preg_match("/(UPDATE PROCESS ENDED)/",$pfb_output)) {
			$f = fopen($pfblog, "rb");
			fseek($f, $lastpos);
			$pfb_buffer = fread($f, 2048);
			$pfb_output .= str_replace( "\r", "", $pfb_buffer);
			pfbupdate_output($pfb_output);
			clearstatcache(false,$pfblog);
			ob_flush();
			flush();
			fclose($f);
			# Call Log Mgmt Function
			pfb_log_mgmt();
			die();
		}
	}
}


if (isset($_POST['pfbcancel'])) {

	// Remove any pfBlockerNG CRON Jobs
	install_cron_job("pfblockerng.php update", false);
	install_cron_job("pfblockerng.php cron", false);

	// Define pfBlockerNG CRON Job
	$pfb_cmd = "/usr/local/bin/php /usr/local/www/pfblockerng/pfblockerng.php cron >> {$pfb['log']} 2>&1";
	$pfb_min = $config['installedpackages']['pfblockerng']['config'][0]['pfb_min'];
	$pfb_hour = "*";
	$pfb_mday = "*";
	$pfb_month = "*";
	$pfb_wday = "*";
	$pfb_who = "root";

	install_cron_job($pfb_cmd, true, $pfb_min, $pfb_hour, $pfb_mday, $pfb_month, $pfb_wday, $pfb_who);
	write_config();

	clearstatcache(false,$pfblog);
	ob_flush();
	flush();
	fclose($pfblog);
}

?>