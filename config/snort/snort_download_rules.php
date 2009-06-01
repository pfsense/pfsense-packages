<?php
/* $Id$ */
/*
	snort_rulesets.php
	Copyright (C) 2006 Scott Ullrich
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
$tmpfname = "/tmp/snort_rules_up";
$snortdir = "/usr/local/etc/snort";
$snort_filename_md5 = "snortrules-snapshot-2.8.tar.gz.md5";
$snort_filename = "snortrules-snapshot-2.8.tar.gz";

require_once("guiconfig.inc");
require_once("functions.inc");
require_once("service-utils.inc");
require("/usr/local/pkg/snort.inc");

$pgtitle = "Services: Snort: Update Rules";

include("/usr/local/www/head.inc");

?>

<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("/usr/local/www/fbegin.inc"); ?>

<?php
if(!$pgtitle_output)
        echo "<p class=\"pgtitle\"><?=$pgtitle?></p>";
?>

<form action="snort_download_rules.php" method="post">
<div id="inputerrors"></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=snort.xml&id=0");
        $tab_array[] = array(gettext("Update Rules"), true, "/snort_download_rules.php");
        $tab_array[] = array(gettext("Categories"), false, "/snort_rulesets.php");
        $tab_array[] = array(gettext("Rules"), false, "/snort_rules.php");
        $tab_array[] = array(gettext("Blocked"), false, "/snort_blocked.php");
        $tab_array[] = array(gettext("Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
        $tab_array[] = array(gettext("Alerts"), false, "/snort_alerts.php");
        $tab_array[] = array(gettext("Advanced"), false, "/pkg_edit.php?xml=snort_advanced.xml&id=0");
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
                      <?=gettext("Initializing...");?>
                      </textarea>
                      <!-- command output box -->
                      <textarea cols="60" rows="1" name="output" id="output" wrap="hard">
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

/* Begin main code */
/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","125M");

/* define oinkid */
if($config['installedpackages']['snort'])
        $oinkid = $config['installedpackages']['snort']['config'][0]['oinkmastercode'];

/* if missing oinkid exit */
if(!$oinkid) {
        $static_output = gettext("You must obtain an oinkid from snort.org and set its value in the Snort settings tab.");
        update_all_status($static_output);
        hide_progress_bar_status();
        exit;
}

/* hide progress bar */
hide_progress_bar_status();

/* send current buffer */
ob_flush();

/*  remove old $tmpfname files */
if (file_exists("{$tmpfname}")) {
    /* echo "removing old {$tmpfname} files\n"; */
    update_status(gettext("Removing old tmp files..."));
    exec("/bin/rm -r {$tmpfname}");
}

/* send current buffer */
ob_flush();

/* If tmp dir does not exist create it */
if (file_exists($tmpfname)) {
    /* echo "The directory $tmpfname exists\n"; */
    update_status(gettext("The directory tmp exists..."));
} else {
    mkdir("{$tmpfname}", 700);
}

/* unhide progress bar and lets end this party */
unhide_progress_bar_status();

/*  download md5 sig */
if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
    /* echo "{$snort_filename_md5} does exists\n"; */
    update_status(gettext("md5 temp file exists..."));
} else {
    /* echo "downloading md5\n"; */
    update_status(gettext("Downloading md5 file..."));
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
$image = file_get_contents("http://dl.snort.org/reg-rules/snortrules-snapshot-2.8.tar.gz.md5?oink_code={$oinkid}");
$f = fopen("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5", 'w');
fwrite($f, $image);
fclose($f);
     /* echo "done\n"; */
     update_status(gettext("Done."));
}

/* Check if were up to date */
if (file_exists("{$snortdir}/{$snort_filename_md5}")) {
$md5_check_new_parse = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
$md5_check_new = `/bin/echo "{$md5_check_new_parse}" | /usr/bin/awk '{ print $4 }'`;
$md5_check_old_parse = file_get_contents("{$snortdir}/{$snort_filename_md5}");
$md5_check_old = `/bin/echo "{$md5_check_old_parse}" | /usr/bin/awk '{ print $4 }'`; 
   if ($md5_check_new == $md5_check_old)
           echo "You are Up to date!\n\n</body>\n</html>\n", update_status(gettext("Your rules are up to date...")), update_output_window(gettext("You may start Snort now.")), hide_progress_bar_status(), exit(0);
}

/* echo "You are Not Up to date!\n"; */
update_status(gettext("You are NOT up to date..."));

/* download snortrules file */
if (file_exists("{$tmpfname}/{$snort_filename}")) {
    /* echo "{$snort_filename} does exists\n"; */
    update_status(gettext("Snortrule tar file exists..."));
} else {
    /* echo "downloading rules\n"; */
    update_status(gettext("Downloading rules..."));
    update_output_window(gettext("May take 4 to 10 min..."));

update_output_window("{$snort_filename}");
download_file_with_progress_bar("http://dl.snort.org/reg-rules/snortrules-snapshot-2.8.tar.gz?oink_code={$oinkid}", $tmpfname . "/{$snort_filename}", "read_body_firmware");
update_all_status($static_output);
    /* echo "done\n"; */
    update_status(gettext("Done."));
}


/* Compair md5 sigs */
$md555 = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
$md5 = `/bin/echo "{$md555}" | /usr/bin/awk '{ print $4 }'`;
$file_md5_ondisk = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;

   if ($md5 == $file_md5_ondisk)
           /* echo "Valid checksum pass\n"; */
           update_status(gettext("Valid checksum pass"));

/* Untar snort rules file */
if (file_exists("{$tmpfname}/rules")) {
    /* echo "The directory {$tmpfname}/rules exists\n"; */
    update_status(gettext("The directory rules exists..."));
} else {
    /* echo "extracting rules\n"; */
    update_status(gettext("Extracting rules..."));
    update_output_window(gettext("May take a while..."));
    exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname}");
    update_status(gettext("Done."));
}

/*  Copy rules dir to snort dir */
if (file_exists("{$snortdir}/rules")) {
    /* echo "The directory {$snortdir}/rules exists\n"; */
    update_status(gettext("Directory rules exists..."));
} else {
    /* echo "copying rules to {$snortdir}\n"; */
    update_status(gettext("Copying rules..."));
    update_output_window(gettext("May take a while..."));
    exec("/bin/cp -r {$tmpfname}/rules {$snortdir}/rules");
    update_status(gettext("Done."));
}

/*  Copy md5 sig to snort dir */
if (file_exists("{$snortdir}/$snort_filename_md5")) {
    /* echo "The {$snort_filename_md5} exists in the {$snortdir} exists\n"; */
    update_status(gettext("The md5 file exists..."));
} else {
    /* echo "copying sig to {$snortdir}\n"; */
    update_status(gettext("Copying md5 sig to snort directory..."));
    exec("/bin/cp {$tmpfname}/$snort_filename_md5 {$snortdir}/$snort_filename_md5");
}

/*  Copy configs to snort dir */
if (file_exists("{$snortdir}/Makefile.am")) {
    /* echo "The Snort configs exists in the {$snortdir} exists\n"; */
    update_status(gettext("The snort configs exists..."));
} else {
    /* echo "copying sig to {$snortdir}\n"; */
    update_status(gettext("Copying configs to snort directory..."));
    exec("/bin/cp {$tmpfname}/etc/* {$snortdir}");
}

/*  Copy signatures dir to snort dir */
if (file_exists("{$snortdir}/doc/signatures")) {
    /* echo "The directory {$snortdir}/signatures exists\n"; */
    update_status(gettext("Directory signatures exists..."));
} else {
    /* echo "copying signatures to {$snortdir}\n"; */
    update_status(gettext("Copying signatures..."));
    update_output_window(gettext("May take a while..."));
    exec("/bin/cp -r {$tmpfname}/doc/signatures {$snortdir}/signatures");
    update_status(gettext("Done."));
}

/* echo "done finnal\n"; */
update_status(gettext("Rules update finished..."));
update_output_window(gettext("You may start Snort now."));

/* hide progress bar and lets end this party */
hide_progress_bar_status();

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
        fwrite($fout, $string);
        return $length;
}

?>
