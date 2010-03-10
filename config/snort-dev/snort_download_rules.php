<?php
/* $Id$ */
/*
	snort_rulesets.php
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2009 Robert Zelaya
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

/* Setup enviroment */

require("guiconfig.inc");
require_once("functions.inc");
require_once("service-utils.inc");
require("/usr/local/pkg/snort/snort.inc");

$tmpfname = "/tmp/snort_rules_up";
$snortdir = "/usr/local/etc/snort";
$snortdir_wan = "/usr/local/etc/snort";
$snort_filename_md5 = "snortrules-snapshot-2.8.tar.gz.md5";
$snort_filename = "snortrules-snapshot-2.8.tar.gz";
$emergingthreats_filename_md5 = "version.txt";
$emergingthreats_filename = "emerging.rules.tar.gz";
$pfsense_rules_filename_md5 = "pfsense_rules.tar.gz.md5";
$pfsense_rules_filename = "pfsense_rules.tar.gz";

$id_d = $_GET['id_d'];
if (isset($_POST['id_d']))
	$id_d = $_POST['id_d'];

/*  Time stamps define */
$last_md5_download = $config['installedpackages']['snortglobal']['last_md5_download'];
$last_rules_install = $config['installedpackages']['snortglobal']['last_rules_install'];

/* define checks */
$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingthreats = $config['installedpackages']['snortglobal']['emergingthreats'];


	if ($snortdownload == "off" && $emergingthreats != "on")
	{
		$snort_emrging_info = "stop";
	}

	if ($oinkid == "" && $snortdownload != "off")
	{
		$snort_oinkid_info = "stop";
	}

	
	/* check if main rule directory is empty */
	$if_mrule_dir = "/usr/local/etc/snort/rules";
	$mfolder_chk = (count(glob("$if_mrule_dir/*")) === 0) ? 'empty' : 'full';
	
/* If no id show the user a button */	
if ($id_d == "" || $snort_emrging_info == "stop" || $snort_oinkid_info == "stop") {

$pgtitle = "Services: Snort: Update Rules";

include("head.inc");
include("fbegin.inc");

/* make sure user has javascript on */
echo "<style type=\"text/css\">
.alert {
 position:absolute;
 top:10px;
 left:0px;
 width:94%;
background:#FCE9C0;
background-position: 15px; 
border-top:2px solid #DBAC48;
border-bottom:2px solid #DBAC48;
padding: 15px 10px 85% 50px;
}
</style> 
<noscript><div class=\"alert\" ALIGN=CENTER><img src=\"/themes/nervecenter/images/icons/icon_alert.gif\"/><strong>Please enable JavaScript to view this content</CENTER></div></noscript>\n";
echo "<body link=\"#000000\" vlink=\"#000000\" alink=\"#000000\">\n";

echo "<script src=\"/row_toggle.js\" type=\"text/javascript\"></script>\n
<script src=\"/javascript/sorttable.js\" type=\"text/javascript\"></script>\n
<table width=\"99%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
   <tr>\n
   		<td>\n";

	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", true, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);

if ($snort_emrging_info == "stop" && $snort_oinkid_info == "stop") {
$disable_enable_button = 'onclick="this.disabled=true"';
}else{
$disable_enable_button = "onClick=\"parent.location='/snort/snort_download_rules.php?id_d=up'\"";
}
echo  		"</td>\n
  </tr>\n
  <tr>\n
    <td>\n
		<div id=\"mainarea\">\n
			<table id=\"maintable\" class=\"tabcont\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
				<tr>\n
					<td>\n
<input name=\"Submit\" type=\"submit\" class=\"formbtn\" $disable_enable_button value=\"Update Rules\" $disable_button> <br><br> \n";

if ($mfolder_chk == "empty")
{
echo "<span class=\"red\"><strong>WARNING:</strong></span> &nbsp;&nbsp;The main rules <strong>directory</strong> is <strong>empty</strong>. /usr/local/etc/snort/rules <br><br>\n";
}

if ($snort_emrging_info == "stop") {
echo "<span class=\"red\"><strong>WARNING:</strong></span> &nbsp;&nbsp;Click on the <strong>\"Global Settings\"</strong> tab and select ether snort.org or enmergingthreats.net rules to download. <br><br> \n";
}

if ($snort_oinkid_info == "stop") {
echo "<span class=\"red\"><strong>WARNING:</strong></span> &nbsp;&nbsp;Click on the <strong>\"Global Settings\"</strong> tab and enter a <strong>oinkmaster</strong> code. <br><br> \n";
}


echo "		    		</td>\n
		  		</tr>\n
			</table>\n
		</div>\n
	</td>\n
  </tr>\n
</table>\n
\n
</form>\n
\n
<p>\n\n";

if ($id_d == "")
echo "Click on the <strong>\"Update Rules\"</strong> button to start the updates. <br><br> \n";

if ($config['installedpackages']['snortglobal']['last_md5_download'] != "")
echo "The last time the updates were started <strong>$last_md5_download</strong>. <br><br> \n";

if ($config['installedpackages']['snortglobal']['last_rules_install'] != "")
echo "The last time the updates were installed <strong>$last_rules_install</strong>. <br><br> \n";

include("fend.inc");

echo "</body>";
echo "</html>";

exit(0);

}

$pgtitle = "Services: Snort: Update Rules";

include("/usr/local/www/head.inc");

?>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script type="text/javascript" src="/snort/javascript/jquery-1.3.2.js"></script>
<script type="text/javascript" src="/snort/javascript/jquery.blockUI.js?v2.28"></script>

<script type="text/javascript">
<!--

function displaymessage()
{

		$.blockUI.defaults.message = "Please be patient....";

        $.blockUI({

			css: {
            border: 'none',
            padding: '15px',
            backgroundColor: '#000',
            '-webkit-border-radius': '10px',
            '-moz-border-radius': '10px',
            opacity: .5,
            color: '#fff',
				}
				});
               
}

function displaymessagestop()
{

setTimeout($.unblockUI, 2000);

}

// -->
</script>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("/usr/local/www/fbegin.inc"); ?>



<form action="snort_download_rules.php" method="post">
<div id="inputerrors"></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", true, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>

<script type="text/javascript">
<!--
	displaymessage();	
// -->
</script>

</td>
</tr>
		<br>
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
                      <img border='0' src='../themes/<?= $g['theme']; ?>/images/misc/progress_bar.gif' width='280' height='23' name='progressbar' id='progressbar' alt='' />
                    </td>
                  </tr>
                </table>
                      <br />
                      <!-- status box -->
                      <textarea cols="60" rows="2" name="status" id="status" wrap="hard">
                      <?=gettext("Initializing...");?>
                      </textarea>
                      <!-- command output box -->
                      <textarea cols="60" rows="2" name="output" id="output" wrap="hard">
                      </textarea>
                  </td>
                </tr>
              </table>
              </div>
          </td>
        </tr>
</table>
</form>
<?php include("fend.inc");?>

<?php

conf_mount_rw();

/* Begin main code */
/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","125M");

/* mark the time update started */
$config['installedpackages']['snortglobal']['last_md5_download'] = date("Y-M-jS-h:i-A");

/* send current buffer */
ob_flush();
conf_mount_rw();

$premium_subscriber_chk = $config['installedpackages']['snortglobal']['snortdownload'];

if ($premium_subscriber_chk == "premium") {
    $premium_subscriber = "_s";
}else{
    $premium_subscriber = "";
}

$premium_url_chk = $config['installedpackages']['snortglobal']['snortdownload'];
if ($premium_url_chk == "premium") {
    $premium_url = "sub-rules";
}else{
    $premium_url = "reg-rules";
}

/* hide progress bar */
hide_progress_bar_status();

/* send current buffer */
ob_flush();
conf_mount_rw();

/*  remove old $tmpfname files */
if (file_exists("{$tmpfname}")) {
    update_status(gettext("Removing old tmp files..."));
    exec("/bin/rm -r {$tmpfname}");
	apc_clear_cache();
}

/*  Make shure snortdir exits */
exec("/bin/mkdir -p {$snortdir}");
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p /usr/local/lib/snort/dynamicrules/");

/* send current buffer */
ob_flush();
conf_mount_rw();

/* If tmp dir does not exist create it */
if (file_exists($tmpfname)) {
    update_status(gettext("The directory tmp exists..."));
} else {
    mkdir("{$tmpfname}", 700);
}

/* unhide progress bar and lets end this party */
unhide_progress_bar_status();

/*  download md5 sig from snort.org */
if ($snortdownload == "basic" || $snortdownload == "premium")
{
	if (file_exists("{$tmpfname}/{$snort_filename_md5}") &&
	    filesize("{$tmpfname}/{$snort_filename_md5}") > 0) {
		update_status(gettext("snort.org md5 temp file exists..."));
	} else {
		update_status(gettext("Downloading snort.org md5 file..."));
		ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
		$image = @file_get_contents("http://dl.snort.org/{$premium_url}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz.md5?oink_code={$oinkid}");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz.md5");
		$f = fopen("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5", 'w');
		fwrite($f, $image);
		fclose($f);
		update_status(gettext("Done downloading snort.org md5"));
	}
}

/*  download md5 sig from emergingthreats.net */
if ($emergingthreats == "on")
{
		update_status(gettext("Downloading emergingthreats md5 file..."));
		ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
		$image = @file_get_contents("http://www.emergingthreats.net/version.txt");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/version.txt");
		$f = fopen("{$tmpfname}/version.txt", 'w');
		fwrite($f, $image);
		fclose($f);
		update_status(gettext("Done downloading emergingthreats md5"));
}

/*  download md5 sig from pfsense.org */
if (file_exists("{$tmpfname}/{$pfsense_rules_filename_md5}")) {
    update_status(gettext("pfsense md5 temp file exists..."));
} else {
    update_status(gettext("Downloading pfsense md5 file..."));
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://www.pfsense.com/packages/config/snort/pfsense_rules/pfsense_rules.tar.gz.md5");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/pfsense_rules.tar.gz.md5");
    $f = fopen("{$tmpfname}/pfsense_rules.tar.gz.md5", 'w');
    fwrite($f, $image);
    fclose($f);
    update_status(gettext("Done downloading pfsense md5."));
}

/* If md5 file is empty wait 15min exit */
if ($snortdownload != "off")
{
	if (0 == filesize("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5"))
	{
		update_status(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		update_output_window(gettext("Rules are released every month from snort.org. You may download the Rules at any time."));
		hide_progress_bar_status();
		/* Display last time of sucsessful md5 check from cache */
		echo "\n\n</body>\n</html>\n";
		echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
echo "</body>";
echo "</html>";
conf_mount_ro();
		exit(0);
	}
}

/* If emergingthreats md5 file is empty wait 15min exit not needed */

/* If pfsense md5 file is empty wait 15min exit */
if (0 == filesize("{$tmpfname}/$pfsense_rules_filename_md5")){
    update_status(gettext("Please wait... You may only check for New Pfsense Rules every 15 minutes..."));
    update_output_window(gettext("Rules are released to support Pfsense packages."));
    hide_progress_bar_status();
    /* Display last time of sucsessful md5 check from cache */
    echo "\n\n</body>\n</html>\n";
	echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
echo "</body>";
echo "</html>";
conf_mount_ro();
    exit(0);
}

/* Check if were up to date snort.org */
if ($snortdownload != "off")
{
	if (file_exists("{$snortdir}/snortrules-snapshot-2.8.tar.gz.md5"))
	{
		$md5_check_new_parse = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
		$md5_check_new = `/bin/echo "{$md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
		$md5_check_old_parse = file_get_contents("{$snortdir}/{$snort_filename_md5}");
		$md5_check_old = `/bin/echo "{$md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
		/* Write out time of last sucsessful md5 to cache */
		write_config(); // Will cause switch back to read-only on nanobsd
		conf_mount_rw();
		if ($md5_check_new == $md5_check_old)
		{
			update_status(gettext("Your rules are up to date..."));
			update_output_window(gettext("You may start Snort now, check update."));
			hide_progress_bar_status();
        echo "\n\n</body>\n</html>\n";
        $snort_md5_check_ok = on;
		}
	}
}

/* Check if were up to date emergingthreats.net */
if ($emergingthreats == "on")
{
	if (file_exists("{$snortdir}/version.txt"))
	{
		$emerg_md5_check_new_parse = file_get_contents("{$tmpfname}/version.txt");
		$emerg_md5_check_new = `/bin/echo "{$emerg_md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
		$emerg_md5_check_old_parse = file_get_contents("{$snortdir}/version.txt");
		$emerg_md5_check_old = `/bin/echo "{$emerg_md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
		/* Write out time of last sucsessful md5 to cache */
		// Will cause switch back to read-only on nanobsd
		write_config();
		conf_mount_rw();
		if ($emerg_md5_check_new == $emerg_md5_check_old) 
		{
        hide_progress_bar_status();
        $emerg_md5_check_ok = on;
		}
	}
}

/* Check if were up to date pfsense.org */
	if (file_exists("{$snortdir}/pfsense_rules.tar.gz.md5"))
	{
		$pfsense_check_new_parse = file_get_contents("{$tmpfname}/pfsense_rules.tar.gz.md5");
		$pfsense_md5_check_new = `/bin/echo "{$pfsense_md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
		$pfsense_md5_check_old_parse = file_get_contents("{$snortdir}/pfsense_rules.tar.gz.md5");
		$pfsense_md5_check_old = `/bin/echo "{$pfsense_md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
		/* Write out time of last sucsessful md5 to cache */
		// Will cause switch back to read-only on nanobsd
		write_config();
		conf_mount_rw();
		if ($pfsense_md5_check_new == $pfsense_md5_check_old) 
		{
        hide_progress_bar_status();
        $pfsense_md5_check_ok = on;
		}
	}

/* Check if were up to date is so, exit */
/* WARNING This code needs constant checks */
if ($snortdownload != "off" && $emergingthreats != "off")
{
	if ($snort_md5_check_ok == "on" && $emerg_md5_check_ok == "on")
	{
			update_status(gettext("All your rules are up to date..."));
			update_output_window(gettext("You may start Snort now..."));
			echo '
	<script type="text/javascript">
	<!--
		displaymessagestop();	
	// -->
	</script>';
			echo "</body>";
			echo "</html>";
			conf_mount_ro();
			exit(0);
	}
}

if ($snortdownload == "on" && $emergingthreats == "off")
{
	if ($snort_md5_check_ok == "on")
	{
			update_status(gettext("Your snort.org rules are up to date..."));
			update_output_window(gettext("You may start Snort now..."));
			echo '
	<script type="text/javascript">
	<!--
		displaymessagestop();	
	// -->
	</script>';
			echo "</body>";
			echo "</html>";
			conf_mount_ro();
			exit(0);
	}
}

if ($snortdownload == "off" && $emergingthreats == "on")
{
	if ($emerg_md5_check_ok == "on")
	{
			update_status(gettext("Your Emergingthreats rules are up to date..."));
			update_output_window(gettext("You may start Snort now..."));
			echo '
	<script type="text/javascript">
	<!--
		displaymessagestop();	
	// -->
	</script>';
			echo "</body>";
			echo "</html>";
			conf_mount_ro();
			exit(0);
	}
}
		
/* You are Not Up to date, always stop snort when updating rules for low end machines */;
update_status(gettext("You are NOT up to date..."));
update_output_window(gettext("Stopping Snort service..."));
$chk_if_snort_up = exec("pgrep -x snort");
if ($chk_if_snort_up != "") {
	exec("/usr/bin/touch /tmp/snort_download_halt.pid");
	exec("/bin/sh /usr/local/etc/rc.d/snort.sh stop");
	sleep(2);
}

/* download snortrules file */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on) {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		update_status(gettext("Snortrule tar file exists..."));
	} else {
		unhide_progress_bar_status();
		update_status(gettext("There is a new set of Snort rules posted. Downloading..."));
		update_output_window(gettext("May take 4 to 10 min..."));
//    download_file_with_progress_bar("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz", $tmpfname . "/{$snort_filename}", "read_body_firmware");
		download_file_with_progress_bar("http://dl.snort.org/{$premium_url}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz?oink_code={$oinkid}", $tmpfname . "/{$snort_filename}", "read_body_firmware");
		update_all_status($static_output);
		update_status(gettext("Done downloading rules file."));
		if (150000 > filesize("{$tmpfname}/$snort_filename")){
			update_status(gettext("Error with the snort rules download..."));
			update_output_window(gettext("Snort rules file downloaded failed..."));
			echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
echo "</body>";
echo "</html>";
conf_mount_ro();
			exit(0);
			}		  
		}
	}
}
	
/* download emergingthreats rules file */
if ($emergingthreats == "on") 
{
	if ($emerg_md5_check_ok != on)
	{
	if (file_exists("{$tmpfname}/{$emergingthreats_filename}"))
	{
		update_status(gettext("Emergingthreats tar file exists..."));
	}else{
		update_status(gettext("There is a new set of Emergingthreats rules posted. Downloading..."));
		update_output_window(gettext("May take 4 to 10 min..."));
//   download_file_with_progress_bar("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/emerging.rules.tar.gz", $tmpfname . "/{$emergingthreats_filename}", "read_body_firmware");
		download_file_with_progress_bar("http://www.emergingthreats.net/rules/emerging.rules.tar.gz", $tmpfname . "/{$emergingthreats_filename}", "read_body_firmware");
		update_all_status($static_output);
		update_status(gettext("Done downloading Emergingthreats rules file."));
		}
	}
}

/* download pfsense rules file */
if ($pfsense_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
    update_status(gettext("Snortrule tar file exists..."));
} else {
	unhide_progress_bar_status();
    update_status(gettext("There is a new set of Pfsense rules posted. Downloading..."));
    update_output_window(gettext("May take 4 to 10 min..."));
//    download_file_with_progress_bar("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/pfsense_rules.tar.gz", $tmpfname . "/{$pfsense_rules_filename}", "read_body_firmware");
    download_file_with_progress_bar("http://www.pfsense.com/packages/config/snort/pfsense_rules/pfsense_rules.tar.gz", $tmpfname . "/{$pfsense_rules_filename}", "read_body_firmware");
    update_all_status($static_output);
    update_status(gettext("Done downloading rules file."));
 }
}

/* Compair md5 sig to file sig */

//$premium_url_chk = $config['installedpackages']['snort']['config'][0]['subscriber'];
//if ($premium_url_chk == on) {
//$md5 = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
//$file_md5_ondisk = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
// if ($md5 == $file_md5_ondisk) {
//    update_status(gettext("Valid md5 checksum pass..."));
//} else {
//    update_status(gettext("The downloaded file does not match the md5 file...P is ON"));
//    update_output_window(gettext("Error md5 Mismatch..."));
//    exit(0);
//  }
//}

//$premium_url_chk = $config['installedpackages']['snort']['config'][0]['subscriber'];
//if ($premium_url_chk != on) {
//$md55 = `/bin/cat {$tmpfname}/{$snort_filename_md5} | /usr/bin/awk '{ print $4 }'`;
//$file_md5_ondisk2 = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
// if ($md55 == $file_md5_ondisk2) {
//    update_status(gettext("Valid md5 checksum pass..."));
//} else {
//    update_status(gettext("The downloaded file does not match the md5 file...Not P"));
//    update_output_window(gettext("Error md5 Mismatch..."));
//    exit(0);
//    }
//}

/* Untar snort rules file individually to help people with low system specs */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on) {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		update_status(gettext("Extracting rules..."));
		update_output_window(gettext("May take a while..."));
		exec("/bin/mkdir -p {$snortdir}/rules_bk/");
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/rules_bk rules/");
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} etc/" .
			" so_rules/precompiled/FreeBSD-7.0/i386/2.8.5.1/" .
			" so_rules/bad-traffic.rules/" .
			" so_rules/chat.rules/" .
			" so_rules/dos.rules/" .
			" so_rules/exploit.rules/" .
			" so_rules/imap.rules/" .
			" so_rules/misc.rules/" .
			" so_rules/multimedia.rules/" .
			" so_rules/netbios.rules/" .
			" so_rules/nntp.rules/" .
			" so_rules/p2p.rules/" .
			" so_rules/smtp.rules/" .
			" so_rules/sql.rules/" .
			" so_rules/web-client.rules/" .
			" so_rules/web-misc.rules/");
		/* add prefix to all snort.org files */
		/* remove this part and make it all php with the simplst code posible */
		chdir ("/usr/local/etc/snort/rules_bk/rules");
		sleep(2);
		exec('/usr/local/bin/perl /usr/local/bin/snort_rename.pl s/^/snort_/ *.rules');
		update_status(gettext("Done extracting Rules."));
	}else{
		update_status(gettext("The Download rules file missing..."));
		update_output_window(gettext("Error rules extracting failed..."));
	echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
echo "</body>";
echo "</html>";
conf_mount_ro();
		exit(0);
		}
	}
}

/* Untar emergingthreats rules to tmp */
if ($emergingthreats == "on")
{
	if ($emerg_md5_check_ok != on)
	{
		if (file_exists("{$tmpfname}/{$emergingthreats_filename}"))
		{
			update_status(gettext("Extracting rules..."));
			update_output_window(gettext("May take a while..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$emergingthreats_filename} -C {$snortdir} rules/");
		}
	}
}

/* Untar Pfsense rules to tmp */
if ($pfsense_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
    update_status(gettext("Extracting Pfsense rules..."));
    update_output_window(gettext("May take a while..."));
    exec("/usr/bin/tar xzf {$tmpfname}/{$pfsense_rules_filename} -C {$snortdir} rules/");
 }
}

/* Untar snort signatures */
if ($snort_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$snort_filename}")) {
$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
if ($premium_url_chk == on) {
	update_status(gettext("Extracting Signatures..."));
	update_output_window(gettext("May take a while..."));
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
	update_status(gettext("Done extracting Signatures."));
  }
 }
}

/* Copy so_rules dir to snort lib dir */
/* Disabed untill I find out why there is a segment failt coredump when using these rules on 2.8.5.3 */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on) {
	if (file_exists("{$snortdir}/so_rules/precompiled/FreeBSD-7.0/i386/2.8.5.1")) {
		update_status(gettext("Copying so_rules..."));
		update_output_window(gettext("May take a while..."));
		exec("/bin/cp -f {$snortdir}/so_rules/precompiled/FreeBSD-7.0/i386/2.8.5.1/* /usr/local/lib/snort/dynamicrules/");
		exec("/bin/cp {$snortdir}/so_rules/bad-traffic.rules {$snortdir}/rules/snort_bad-traffic.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/chat.rules {$snortdir}/rules/snort_chat.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/dos.rules {$snortdir}/rules/snort_dos.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/exploit.rules {$snortdir}/rules/snort_exploit.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/imap.rules {$snortdir}/rules/snort_imap.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/misc.rules {$snortdir}/rules/snort_misc.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/multimedia.rules {$snortdir}/rules/snort_multimedia.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/netbios.rules {$snortdir}/rules/snort_netbios.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/nntp.rules {$snortdir}/rules/snort_nntp.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/p2p.rules {$snortdir}/rules/snort_p2p.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/smtp.rules {$snortdir}/rules/snort_smtp.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/sql.rules {$snortdir}/rules/snort_sql.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/web-activex.rules {$snortdir}/rules/snort_web-activex.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/web-client.rules {$snortdir}/rules/snort_web-client.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/web-iis.rules {$snortdir}/rules/snort_web-iis.so.rules");
		exec("/bin/cp {$snortdir}/so_rules/web.misc.rules {$snortdir}/rules/snort_web.misc.so.rules");
		exec("/bin/rm -r {$snortdir}/so_rules");
		update_status(gettext("Done copying so_rules."));
	}else{
   update_status(gettext("Directory so_rules does not exist..."));
   update_output_window(gettext("Error copying so_rules..."));
	echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
	echo "</body>";
	echo "</html>";
	conf_mount_ro();
   exit(0);
		}
	}
}

/*  Copy renamed snort.org rules to snort dir */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on)
	{
		if (file_exists("{$snortdir}/rules_bk/rules/Makefile.am"))
		{
			update_status(gettext("Copying renamed snort.org rules to snort directory..."));
			exec("/bin/cp {$snortdir}/rules_bk/rules/* {$snortdir}/rules/");
		}else{
			update_status(gettext("The renamed snort.org rules do not exist..."));
			update_output_window(gettext("Error copying config..."));
			echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
			echo "</body>";
			echo "</html>";
			conf_mount_ro();
			exit(0);
			}
	}
}

/*  Copy configs to snort dir */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on)
	{
	if (file_exists("{$snortdir}/etc/Makefile.am")) {
		update_status(gettext("Copying configs to snort directory..."));
		exec("/bin/cp {$snortdir}/etc/* {$snortdir}");
		exec("/bin/rm -r {$snortdir}/etc");
	}else{
    update_status(gettext("The snort config does not exist..."));
    update_output_window(gettext("Error copying config..."));
	echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
echo "</body>";
echo "</html>";
conf_mount_ro();
    exit(0);
		}
	}
}


/*  Copy md5 sig to snort dir */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on) {
	if (file_exists("{$tmpfname}/$snort_filename_md5")) {
		update_status(gettext("Copying md5 sig to snort directory..."));
		exec("/bin/cp {$tmpfname}/$snort_filename_md5 {$snortdir}/$snort_filename_md5");
	}else{
		update_status(gettext("The md5 file does not exist..."));
		update_output_window(gettext("Error copying config..."));
		echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
		echo "</body>";
		echo "</html>";
		conf_mount_ro();
		exit(0);
		}
	}
}	
	
/*  Copy emergingthreats md5 sig to snort dir */
if ($emergingthreats == "on")
{
	if ($emerg_md5_check_ok != on) 
	{
		if (file_exists("{$tmpfname}/$emergingthreats_filename_md5"))
		{
			update_status(gettext("Copying md5 sig to snort directory..."));
			exec("/bin/cp {$tmpfname}/$emergingthreats_filename_md5 {$snortdir}/$emergingthreats_filename_md5");
		}else{
			update_status(gettext("The emergingthreats md5 file does not exist..."));
			update_output_window(gettext("Error copying config..."));
			echo "</body>";
			echo "</html>";
			conf_mount_ro();
			exit(0);
			}
	}
}

/*  Copy Pfsense md5 sig to snort dir */
if ($pfsense_md5_check_ok != on) {
if (file_exists("{$tmpfname}/$pfsense_rules_filename_md5")) {
    update_status(gettext("Copying Pfsense md5 sig to snort directory..."));
    exec("/bin/cp {$tmpfname}/$pfsense_rules_filename_md5 {$snortdir}/$pfsense_rules_filename_md5");
} else {
    update_status(gettext("The Pfsense md5 file does not exist..."));
    update_output_window(gettext("Error copying config..."));
	echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
		echo "</body>";
		echo "</html>";
		conf_mount_ro();
		exit(0);
 }
}
 
/*  Copy signatures dir to snort dir */
if ($snortdownload != "off")
{
	if ($snort_md5_check_ok != on)
	{
		$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
		if ($premium_url_chk == on)
		{
			if (file_exists("{$snortdir}/doc/signatures")) {
			update_status(gettext("Copying signatures..."));
			update_output_window(gettext("May take a while..."));
			exec("/bin/mv -f {$snortdir}/doc/signatures {$snortdir}/signatures");
			exec("/bin/rm -r {$snortdir}/doc/signatures");
			update_status(gettext("Done copying signatures."));
		}else{
			update_status(gettext("Directory signatures exist..."));
			update_output_window(gettext("Error copying signature..."));
			echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';
echo "</body>";
echo "</html>";
conf_mount_ro();
			exit(0);
			}
		}
	}
}

/* double make shure cleanup emerg rules that dont belong */
if (file_exists("/usr/local/etc/snort/rules/emerging-botcc-BLOCK.rules")) {
	apc_clear_cache();
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-botcc-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-botcc.rules");
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-compromised-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-drop-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-dshield-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-rbn-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort/rules/emerging-tor-BLOCK.rules");
}

if (file_exists("/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so")) {
	exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so");
	exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example\*");
}

/* make shure default rules are in the right format */
exec("/usr/local/bin/perl -pi -e 's/#alert/# alert/g' /usr/local/etc/snort/rules/*.rules");
exec("/usr/local/bin/perl -pi -e 's/##alert/# alert/g' /usr/local/etc/snort/rules/*.rules");
exec("/usr/local/bin/perl -pi -e 's/## alert/# alert/g' /usr/local/etc/snort/rules/*.rules");

/* create a msg-map for snort  */
update_status(gettext("Updating Alert Messages..."));
update_output_window(gettext("Please Wait..."));
exec("/usr/local/bin/perl /usr/local/bin/create-sidmap.pl /usr/local/etc/snort/rules > /usr/local/etc/snort/sid-msg.map");


//////////////////

/* open oinkmaster_conf for writing" function */
function oinkmaster_conf()
{

        global $config, $g, $id, $if_real, $snortdir_wan, $snortdir, $snort_md5_check_ok, $emerg_md5_check_ok, $pfsense_md5_check_ok;
        conf_mount_rw();
		
/*  enable disable setting will carry over with updates */
/*  TODO carry signature changes with the updates */
if ($snort_md5_check_ok != on || $emerg_md5_check_ok != on || $pfsense_md5_check_ok != on) {

if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on'])) {
$enabled_sid_on = $config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on'];
$enabled_sid_on_array = split('\|\|', $enabled_sid_on);
foreach($enabled_sid_on_array as $enabled_item_on)
$selected_sid_on_sections .= "$enabled_item_on\n";
	}

if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'])) {
$enabled_sid_off = $config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'];
$enabled_sid_off_array = split('\|\|', $enabled_sid_off);
foreach($enabled_sid_off_array as $enabled_item_off)
$selected_sid_off_sections .= "$enabled_item_off\n";
	}

$snort_sid_text = <<<EOD

###########################################
#                                         #
# this is auto generated on snort updates #
#                                         #
###########################################

path = /bin:/usr/bin:/usr/local/bin

update_files = \.rules$|\.config$|\.conf$|\.txt$|\.map$

url = dir:///usr/local/etc/snort/rules

$selected_sid_on_sections

$selected_sid_off_sections

EOD;

     /* open snort's oinkmaster.conf for writing */
     $oinkmasterlist = fopen("/usr/local/etc/snort/snort_$id$if_real/oinkmaster_$id$if_real.conf", "w");

     fwrite($oinkmasterlist, "$snort_sid_text");

     /* close snort's oinkmaster.conf file */
     fclose($oinkmasterlist);

	}
}

/*  Run oinkmaster to snort_wan and cp configs */
/*  If oinkmaster is not needed cp rules normally */
/*  TODO add per interface settings here */
function oinkmaster_run()
{

        global $config, $g, $id, $if_real, $snortdir_wan, $snortdir, $snort_md5_check_ok, $emerg_md5_check_ok, $pfsense_md5_check_ok;
        conf_mount_rw();

	if ($snort_md5_check_ok != on || $emerg_md5_check_ok != on || $pfsense_md5_check_ok != on)
	{

		if ($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on'] == '' && $config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'] == '')
		{
		update_status(gettext("Your first set of rules are being copied..."));
		update_output_window(gettext("May take a while..."));
		exec("/bin/echo \"test {$snortdir} {$snortdir_wan} $id$if_real\" > /root/debug");
		exec("/bin/cp {$snortdir}/rules/* {$snortdir_wan}/snort_$id$if_real/rules/");
		exec("/bin/cp {$snortdir}/classification.config {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/generators {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/reference.config {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/sid {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/unicode.map {$snortdir_wan}/snort_$id$if_real");
		}else{
		update_status(gettext("Your enable and disable changes are being applied to your fresh set of rules..."));
		update_output_window(gettext("May take a while..."));
		exec("/bin/echo \"test2 {$snortdir} {$snortdir_wan} $id$if_real\" > /root/debug");
		exec("/bin/cp {$snortdir}/rules/* {$snortdir_wan}/snort_$id$if_real/rules/");
		exec("/bin/cp {$snortdir}/classification.config {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/generators {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/reference.config {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/sid {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir_wan}/snort_$id$if_real");
		exec("/bin/cp {$snortdir}/unicode.map {$snortdir_wan}/snort_$id$if_real");

		/*  might have to add a sleep for 3sec for flash drives or old drives */
		exec("/usr/local/bin/perl /usr/local/bin/oinkmaster.pl -C /usr/local/etc/snort/snort_$id$if_real/oinkmaster_$id$if_real.conf -o /usr/local/etc/snort/snort_$id$if_real/rules > /usr/local/etc/snort/oinkmaster_$id$if_real.log");
		}
	}
}

/* Start the proccess for every interface rule */
/* TODO: try to make the code smother */

if (!empty($config['installedpackages']['snortglobal']['rule']))
{

	$rule_array = $config['installedpackages']['snortglobal']['rule'];
	$id = -1;
	foreach ($rule_array as $value) {

	$id += 1;

	$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
	$if_real = convert_friendly_interface_to_real_interface_name($result_lan);

	/* make oinkmaster.conf for each interface rule */
	oinkmaster_conf();
	
	/* run oinkmaster for each interface rule */
	oinkmaster_run();

	}
}

//////////////

/* mark the time update finnished */
$config['installedpackages']['snortglobal']['last_rules_install'] = date("Y-M-jS-h:i-A");

/*  remove old $tmpfname files */
if (file_exists("{$tmpfname}"))
{
	update_status(gettext("Cleaning up..."));
	exec("/bin/rm -r /tmp/snort_rules_up");
	sleep(2);
	exec("/bin/rm -r {$snortdir}/rules_bk/rules/");
	apc_clear_cache();
}

/* php code to flush out cache some people are reportting missing files this might help  */
sleep(2);
apc_clear_cache();
exec("/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync");

/* make all dirs snorts */
exec("/usr/sbin/chown -R snort:snort /var/log/snort");
exec("/usr/sbin/chown -R snort:snort /usr/local/etc/snort");
exec("/usr/sbin/chown -R snort:snort /usr/local/lib/snort");
exec("/bin/chmod -R 755  /var/log/snort");
exec("/bin/chmod -R 755  /usr/local/etc/snort");
exec("/bin/chmod -R 755  /usr/local/lib/snort");


/* if snort is running hardrestart, if snort is not running do nothing */
if (file_exists("/tmp/snort_download_halt.pid")) {
	exec("/bin/sh /usr/local/etc/rc.d/snort.sh start");
	update_status(gettext("The Rules update finished..."));
	update_output_window(gettext("Snort has restarted with your new set of rules..."));
	exec("/bin/rm /tmp/snort_download_halt.pid");
} else {
		update_status(gettext("The Rules update finished..."));
		update_output_window(gettext("You may start snort now..."));
}

echo '
<script type="text/javascript">
<!--
	displaymessagestop();	
// -->
</script>';

/* hide progress bar and lets end this party */
hide_progress_bar_status();
conf_mount_ro();
?>

<?php

function read_body_firmware($ch, $string) {
        global $fout, $file_size, $downloaded, $counter, $version, $latest_version, $current_installed_pfsense_version;
        $length = strlen($string);
        $downloaded += intval($length);
        $downloadProgress = round(100 * (1 - $downloaded / $file_size), 0);
        $downloadProgress = 100 - $downloadProgress;
        $a = $file_size;
        $b = $downloaded;
        $c = $downloadProgress;
        $text  = "  Snort download in progress\\n";
        $text .= "----------------------------------------------------\\n";
        $text .= "  Downloaded      : {$b}\\n";
        $text .= "----------------------------------------------------\\n";
        $counter++;
        if($counter > 150) {
                update_output_window($text);
                update_progress_bar($downloadProgress);
                flush();
                $counter = 0;
        }
        conf_mount_rw();
        fwrite($fout, $string);
        conf_mount_ro();
        return $length;
}

?>

</body>
</html>
