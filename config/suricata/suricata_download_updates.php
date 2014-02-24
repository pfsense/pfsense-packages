<?php
/*
 * suricata_download_updates.php
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

/* Define some locally required variables from Suricata constants */
$suricatadir = SURICATADIR;
$suricata_rules_upd_log = RULES_UPD_LOGFILE;

/* load only javascript that is needed */
$suricata_load_jquery = 'yes';
$suricata_load_jquery_colorbox = 'yes';
$snortdownload = $config['installedpackages']['suricata']['config'][0]['enable_vrt_rules'];
$emergingthreats = $config['installedpackages']['suricata']['config'][0]['enable_etopen_rules'];
$etpro = $config['installedpackages']['suricata']['config'][0]['enable_etpro_rules'];
$snortcommunityrules = $config['installedpackages']['suricata']['config'][0]['snortcommunityrules'];

$snort_rules_file = VRT_DNLD_FILENAME;
$snort_community_rules_filename = GPLV2_DNLD_FILENAME;

if ($etpro == "on") {
	$emergingthreats_filename = ETPRO_DNLD_FILENAME;
	$et_name = "EMERGING THREATS PRO RULES";
}
else {
	$emergingthreats_filename = ET_DNLD_FILENAME;
	$et_name = "EMERGING THREATS RULES";
}

/* quick md5 chk of downloaded rules */
$snort_org_sig_chk_local = 'N/A';
if (file_exists("{$suricatadir}{$snort_rules_file}.md5"))
	$snort_org_sig_chk_local = file_get_contents("{$suricatadir}{$snort_rules_file}.md5");

$emergingt_net_sig_chk_local = 'N/A';
if (file_exists("{$suricatadir}{$emergingthreats_filename}.md5"))
	$emergingt_net_sig_chk_local = file_get_contents("{$suricatadir}{$emergingthreats_filename}.md5");

$snort_community_sig_chk_local = 'N/A';
if (file_exists("{$suricatadir}{$snort_community_rules_filename}.md5"))
	$snort_community_sig_chk_local = file_get_contents("{$suricatadir}{$snort_community_rules_filename}.md5");

/* Check for postback to see if we should clear the update log file. */
if ($_POST['clear']) {
	if (file_exists("{$suricata_rules_upd_log}"))
		mwexec("/bin/rm -f {$suricata_rules_upd_log}");
}

if ($_POST['update']) {
	header("Location: /suricata/suricata_download_rules.php");
	exit;
}

/* check for logfile */
if (file_exists("{$suricata_rules_upd_log}"))
	$suricata_rules_upd_log_chk = 'yes';
else
	$suricata_rules_upd_log_chk = 'no';

if ($_POST['view']&& $suricata_rules_upd_log_chk == 'yes') {
	$contents = @file_get_contents($suricata_rules_upd_log);
	if (empty($contents))
		$input_errors[] = gettext("Unable to read log file: {$suricata_rules_upd_log}");
}

if ($_POST['hide'])
	$contents = "";

$pgtitle = gettext("Suricata: Update Rules Set Files");
include_once("head.inc");
?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php include("fbegin.inc"); ?>
<?php
	/* Display Alert message */
	if ($input_errors) {
		print_input_errors($input_errors);
	}

	if ($savemsg) {
		print_info_box($savemsg);
	}
?>
<form action="suricata_download_updates.php" method="post" name="iform" id="iform">

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Suricata Interfaces"), false, "/suricata/suricata_interfaces.php");
        $tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
        $tab_array[] = array(gettext("Update Rules"), true, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php");
	$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php");
        display_top_tabs($tab_array);
?>
</td></tr>
<tr>
	<td>
		<div id="mainarea">
		<table id="maintable4" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td valign="top" class="listtopic" align="center"><?php echo gettext("INSTALLED RULE SET MD5 SIGNATURES");?></td>
			</tr>
			<tr>
				<td align="center"><br/>
				<table width="100%" border="0" cellpadding="2" cellspacing="2">
					<tr>
						<td align="right" class="vexpl"><b><?=$et_name;?>&nbsp;&nbsp;---></b></td>
						<td class="vexpl"><? echo $emergingt_net_sig_chk_local; ?></td>
					</tr>
					<tr>
						<td align="right" class="vexpl"><b>SNORT VRT RULES&nbsp;&nbsp;---></b></td>
						<td class="vexpl"><? echo $snort_org_sig_chk_local; ?></td>
					</tr>
						<td align="right" class="vexpl"><b>SNORT GPLv2 COMMUNITY RULES&nbsp;&nbsp;---></b></td>
						<td class="vexpl"><? echo $snort_community_sig_chk_local; ?></td>
					</tr>
				</table><br/>
				</td>
			</tr>
			<tr>
				<td valign="top" class="listtopic" align="center"><?php echo gettext("UPDATE YOUR RULE SET");?></td>
			</tr>
			<tr>
				<td align="center">
					<?php if ($snortdownload != 'on' && $emergingthreats != 'on' && $etpro != 'on'): ?>
						<br/><button disabled="disabled"><?php echo gettext("Update Rules"); ?></button><br/>
						<p style="text-align:left;">
						<font color="red" size="2px"><b><?php echo gettext("WARNING:");?></b></font><font size="1px" color="#000000">&nbsp;&nbsp;
						<?php echo gettext('No rule types have been selected for download. ') . 
						gettext('Visit the ') . '<a href="/suricata/suricata_global.php">Global Settings Tab</a>' . gettext(' to select rule types.'); ?>
						</font><br/></p>
					<?php else: ?>
						<br/>
						<input type="submit" value="<?php echo gettext(" Update "); ?>" name="update" id="submit" class="formbtn" 
						title="<?php echo gettext("Check for new updates to configured rulesets"); ?>"/><br/><br/>
					<?php endif; ?>
				</td>
			</tr>

			<tr>
				<td valign="top" class="listtopic" align="center"><?php echo gettext("MANAGE RULE SET LOG");?></td>
			</tr>
			<tr>
				<td align="center" valign="middle" class="vexpl">
					<?php if ($suricata_rules_upd_log_chk == 'yes'): ?>
						<br/>
					<?php if (!empty($contents)): ?>
						<input type="submit" value="<?php echo gettext("Hide Log"); ?>" name="hide" id="hide" class="formbtn" 
						title="<?php echo gettext("Hide rules update log"); ?>"/>
					<?php else: ?>
						<input type="submit" value="<?php echo gettext("View Log"); ?>" name="view" id="view" class="formbtn" 
						title="<?php echo gettext("View rules update log"); ?>"/>
					<?php endif; ?>
						&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
						<input type="submit" value="<?php echo gettext("Clear Log"); ?>" name="clear" id="clear" class="formbtn" 
						title="<?php echo gettext("Clear rules update log"); ?>" onClick="return confirm('Are you sure?\nOK to confirm, or CANCEL to quit');"/>
						<br/>
					<?php else: ?>
						<br/>
						<button disabled='disabled'><?php echo gettext("View Log"); ?></button><br/><?php echo gettext("Log is empty."); ?><br/>
					<?php endif; ?>
					<br/><?php echo gettext("The log file is limited to 1024K in size and automatically clears when the limit is exceeded."); ?><br/><br/>
				</td>
			</tr>
			<?php if (!empty($contents)): ?>
				<tr>
					<td valign="top" class="listtopic" align="center"><?php echo gettext("RULE SET UPDATE LOG");?></td>
				</tr>
				<tr>
					<td align="center">
						<div style="background: #eeeeee; width:100%; height:100%;" id="textareaitem"><!-- NOTE: The opening *and* the closing textarea tag must be on the same line. -->
							<textarea style="width:100%; height:100%;" readonly wrap="off" rows="24" cols="80" name="logtext"><?=$contents;?></textarea>
						</div>
					</td>
				</tr>
			<?php endif; ?>
			<tr>
				<td align="center">
					<span class="vexpl"><br/>
					<span class="red"><b><?php echo gettext("NOTE:"); ?></b></span>
					&nbsp;&nbsp;<a href="http://www.snort.org/" target="_blank"><?php echo gettext("Snort.org") . "</a>" . 
					gettext(" and ") . "<a href=\"http://www.emergingthreats.net/\" target=\"_blank\">" . gettext("EmergingThreats.net") . "</a>" . 
					gettext(" will go down from time to time. Please be patient."); ?></span><br/>
				</td>
			</tr>
		</table>
		</div>
	</td>
</tr>
</table>
<!-- end of final table -->
</form>
<?php include("fend.inc"); ?>
</body>
</html>
