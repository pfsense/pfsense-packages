<?php
/*
	snort_download_rules.php
	part of pfSense (http://www.pfsense.com)

	Copyright (C) 2005 Scott Ullrich and Colin Smith

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

	TODO:
		* modify pfSense.com XMLRPC server to return md5 hashes of firmware updates.
*/

$pgtitle = array(gettext("Services"),gettext("Snort"),gettext("Update Rules"));

require_once("guiconfig.inc");
require_once("xmlrpc.inc");

include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<form action="snort_download_rules.php" method="post">
<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Snort Settings"), false, "pkg.php?xml=snort.xml");
	$tab_array[0] = array(gettext("Snort Rules Update"), false, "/usr/local/www/snort_download_rules.php");
	display_top_tabs($tab_array);
?>
    </td>
  </tr>
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
      		      <?=gettext("Beginning system autoupdate...");?>
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
	echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
	exit;
}

$dl = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-CURRENT.tar.gz";
$dl_md5 = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-CURRENT.tar.gz.md5";
$tmpfname = tempnam("/tmp", "snortRules");
$static_output = gettext("Downloading current snort rules... ");

update_all_status($static_output);
download_file_with_progress_bar($dl, 	 $tmpfname);
$static_output = gettext("Downloading current snort rules md5... ");

update_all_status($static_output);
download_file_with_progress_bar($dl_md5, $tmpfname);

/* verify downloaded rules signature */
verify_snort_rules_md5($tmpfname);

/* extract rules */
extract_snort_rules_md5($tmpfname);

$static_output = gettext("Your snort rules are now up to date.");
update_all_status($static_output);

echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
?>
</body>
</html>

<?php
function extract_snort_rules_md5($tmpfname) {
	$static_output = gettext("Extracting snort rules...");
	update_all_status($static_output);
	exec("tar xzf {$tmpfname}/snortrules-snapshot-CURRENT.tar.gz -C /usr/local/etc/snort/");
	$static_output = gettext("Snort rules extracted.");
	update_status($static_output);
	update_output_window($static_output);
}

function verify_snort_rules_md5($tmpfname) {
	$static_output = gettext("Verifying md5 signature...");
	update_all_status($static_output);
	$md5 = file_get_contents("{$tmpfname}/snortrules-snapshot-CURRENT.tar.gz.md5");
	$file_md5_ondisk = `md5 {$tmpfname}/snortrules-snapshot-CURRENT.tar.gz | awk '{ print $4 }'`;
	if($md5 <> $file_md5_ondisk) {
		$static_output = gettext("md5 signature of rules mismatch.");
		update_all_status($static_output);
		echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
		exit;
	}
}

function update_all_status($status) {
	update_status($status);
	update_output_window($status);
}

?>