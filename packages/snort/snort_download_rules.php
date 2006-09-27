<?php
/*
	snort_download_rules.php
	part of pfSense (http://www.pfsense.com)
	Copyright (C) 2005 Scott Ullrich
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

require_once("config.inc");
require_once("functions.inc");
require_once("guiconfig.inc");
require_once("service-utils.inc");

$pgtitle = "Services: Snort: Update Rules";

include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<p class="pgtitle"><?=$pgtitle?></font></p>

<form action="snort_download_rules.php" method="post">
<div id="inputerrors"></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Snort Settings"), false, "/pkg_edit.php?xml=snort.xml&id=0");
	$tab_array[] = array(gettext("Snort Rules Update"), true, "/snort_download_rules.php");
	$tab_array[] = array(gettext("Snort Rulesets"), false, "/snort_rulesets.php");
	$tab_array[] = array(gettext("Snort Blocked"), false, "/snort_blocked.php");
	$tab_array[] = array(gettext("Snort Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
	display_top_tabs($tab_array);
?>
    </td>
  </tr>
<?php
	if(!$_GET['start'] && !$_POST['start']) {
		echo "<tr>";
		echo "<td>";
	    echo "<div id=\"mainarea\">";
	    echo "<table class=\"tabcont\" width=\"100%\" border=\"0\" cellpadding=\"6\" cellspacing=\"0\">";
      	echo "<tr>";
      	echo "<td align=\"center\" valign=\"top\">";
		$last_ruleset_download = $config['installedpackages']['snort']['last_ruleset_download'];
		$text = file_get_contents("http://www.snort.org/pub-bin/downloads.cgi");
		if (preg_match_all("/.*RELEASED\: (.*)\</", $text, $matches))
		        $last_update_date = trim($matches[1][0]);
		echo "<tr><td><table>";
		if($last_update_date)
			echo "<tr><td><b>Last snort.org rule update:</b></td><td>{$last_update_date}</td></tr>";
		if($last_ruleset_download)
			echo "<tr><td><b>You last updated the ruleset:</b></td><td>{$last_ruleset_download}</td></tr>";
		else
			echo "<tr><td><b>You last updated the ruleset:</b></td><td>NEVER</td></tr>";
		echo "</td></tr></table>";
		/* get time stamps for comparison operations */
		$date1ts = strtotime($last_ruleset_download);
		$date2ts = strtotime($last_ruleset_download);
		/* is there a newer ruleset available? */
		if($date1ts > $date2ts or !$last_ruleset_download)
			echo "<tr><td colspan='2'>Press <a href='snort_download_rules.php?start=yes'>here</a> to start download.</td></tr>";
		else
			echo "<tr><td colspan='2'>Your snort rulesets are <b>up to date</b>.</td></tr>";
        echo "</td>";
      	echo "	</tr>";
	    echo "  </table>";
	    echo "  </div>";
	  	echo "</td>";
		echo "</tr>";
		echo "</table>";
		include("fend.inc");
		exit;
	}
?>
	<tr>
	  <td>
	      <div id="mainarea">
	      <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
      		<tr>
      		  <td align="center" valign="top">
      		      <!-- progress bar -->
      		      <table id="progholder" width='420' style='border-collapse: collapse; border: 1px solid #000000;' cellpadding='2' cellspacing='2'>
                  <tr>
                    <td>
                      <img border='0' src='./themes/<?= $g['theme']; ?>/images/misc/progress_bar.gif' width='280' height='23' name='progressbar' id='progressbar' alt='' />
                    </td>
                  </tr>
                </table>
      		      <br />
      		      <!-- status box -->
      		      <textarea cols="60" rows="1" name="status" id="status" wrap="hard">
      		      <?=gettext("Initializing...");?>
      		      </textarea>
      		      <!-- command output box -->
      		      <textarea cols="60" rows="25" name="output" id="output" wrap="hard">
      		      </textarea>
      		  </td>
      		</tr>
	      </table>
	      </div>
	  </td>
	</tr>
</table>
</form>
<?php include("fend.inc"); ?>
<?php

/* define oinkid */
if($config['installedpackages']['snort'])
	$oinkid = $config['installedpackages']['snort']['config'][0]['oinkmastercode'];

if(!$oinkid) {
	$static_output = gettext("You must obtain an oinkid from snort.com and set its value in the Snort settings tab.");
	update_all_status($static_output);
	hide_progress_bar_status();
	exit;
}

/* send current buffer */
ob_flush();

/* setup some variables */
$snort_filename = "snortrules-snapshot-CURRENT.tar.gz";
$snort_filename_md5 = "snortrules-snapshot-CURRENT.tar.gz.md5";
$dl = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}";
$dl_md5 = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename_md5}";

/* multi user system, request new filename and create directory */
$tmpfname = tempnam("/tmp", "snortRules");
exec("/bin/rm -rf {$tmpfname};/bin/mkdir -p {$tmpfname}");

/* download snort rules */
$static_output = gettext("Downloading current snort rules... ");
update_all_status($static_output);
download_file_with_progress_bar($dl, $tmpfname . "/{$snort_filename}");
verify_downloaded_file($tmpfname . "/{$snort_filename}");

/* download snort rules md5 file */
$static_output = gettext("Downloading current snort rules md5... ");
update_all_status($static_output);
download_file_with_progress_bar($dl_md5, $tmpfname . "/{$snort_filename_md5}");
verify_downloaded_file($tmpfname . "/{$snort_filename_md5}");

/* verify downloaded rules signature */
verify_snort_rules_md5($tmpfname);

/* extract rules */
extract_snort_rules_md5($tmpfname);

$static_output = gettext("Your snort rules are now up to date.");
update_all_status($static_output);

$config['installedpackages']['snort']['last_ruleset_download'] = date("Y-m-d");
write_config();

stop_service("snort");
sleep(2);
start_service("snort");

/* cleanup temporary directory */
exec("/bin/rm -rf {$tmpfname};");

/* hide progress bar and lets end this party */
hide_progress_bar_status();

?>

</body>
</html>

<script type="text/javascript">
	document.location.href='snort_download_rules.php?ran=1';
</script>

<?php

function check_for_common_errors($filename) {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	$contents = file_get_contents($filename);
	if(stristr($contents, "You don't have permission")) {
		update_all_status("An error occured.  Scroll down to inspect it's contents.");
		hide_progress_bar_status();
		echo "
      <center>
        <div id='error' style='background:white;width:90%'>
        <!-- TODO: The below paragraphs are kind of stupid. Use CSS instead -->
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
        <p>The following error occured while downloading the snort rules file from snort.org:</p>
		    {$contents}
        <p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>
		    </div>
      </center>
    ";
		scroll_down_to_bottom_of_page();
		exit;
	}
}

function scroll_down_to_bottom_of_page() {
	global $snort_filename;
	ob_flush();
	echo "\n<script type=\"text/javascript\">parent.scrollTo(0,1500);\n</script>";
}

function verify_downloaded_file($filename) {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	if(filesize($filename)<99500) {
		update_all_status("Checking {$filename}...");
		check_for_common_errors($filename);
	}
	update_all_status("Verifying {$filename}...");
	if(!file_exists($filename)) {
		update_all_status("Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.");
		hide_progress_bar_status();
		exit;
	}
	update_all_status("Verifyied {$filename}.");
}

function extract_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	$static_output = gettext("Extracting snort rules...");
	update_all_status($static_output);
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C /usr/local/etc/snort/");
	$static_output = gettext("Snort rules extracted.");
	update_all_status($static_output);
}

function verify_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	$static_output = gettext("Verifying md5 signature...");
	update_all_status($static_output);
	$md5 = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
	$file_md5_ondisk = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
	if($md5 <> $file_md5_ondisk) {
		$static_output = gettext("md5 signature of rules mismatch.");
		update_all_status($static_output);
		hide_progress_bar_status();
		exit;
	}
}

function hide_progress_bar_status() {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
}

function update_all_status($status) {
	global $snort_filename, $snort_filename_md5;
	ob_flush();
	update_status($status);
	update_output_window($status);
}

?>