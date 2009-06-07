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


/* Begin main code */
/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","125M");

/* send current buffer */
ob_flush();

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

/* premium_subscriber check  */
//unset($config['installedpackages']['snort']['config'][0]['subscriber']);
//write_config();
$premium_subscriber_chk = $config['installedpackages']['snort']['config'][0]['subscriber'];

if ($premium_subscriber_chk === on) {
    $premium_subscriber = "_s";
}else{
    $premium_subscriber = "";
}

$premium_url_chk = $config['installedpackages']['snort']['config'][0]['subscriber'];
if ($premium_url_chk === on) {
    $premium_url = "sub-rules";
}else{
    $premium_url = "reg-rules";
}

/* hide progress bar */
hide_progress_bar_status();

/* send current buffer */
ob_flush();

/*  remove old $tmpfname files */
if (file_exists("{$tmpfname}")) {
    update_status(gettext("Removing old tmp files..."));
    exec("/bin/rm -r {$tmpfname}");
}

/* send current buffer */
ob_flush();

/* If tmp dir does not exist create it */
if (file_exists($tmpfname)) {
    update_status(gettext("The directory tmp exists..."));
} else {
    mkdir("{$tmpfname}", 700);
}

/* unhide progress bar and lets end this party */
unhide_progress_bar_status();

/*  download md5 sig */
if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
    update_status(gettext("md5 temp file exists..."));
} else {
    update_status(gettext("Downloading md5 file..."));
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://dl.snort.org/{$premium_url}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz.md5?oink_code={$oinkid}");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz.md5");
    $f = fopen("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5", 'w');
    fwrite($f, $image);
    fclose($f);
    update_status(gettext("Done. downloading md5"));
}

/*  Time stamps define */
$last_md5_download = $config['installedpackages']['snort']['last_md5_download'];
$last_rules_install = $config['installedpackages']['snort']['last_rules_install'];

/* If md5 file is empty wait 15min exit */
if (0 == filesize("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5")){
    update_status(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
    update_output_window(gettext("Rules are released every month from snort.org. You may download the Rules at any time."));
    hide_progress_bar_status();
    /* Display last time of sucsessful md5 check from cache */
    echo "\n<p align=center><b>You last checked for updates: </b>{$last_md5_download}</p>\n";
    echo "\n<p align=center><b>You last installed for rules: </b>{$last_rules_install}</p>\n";
    echo "\n\n</body>\n</html>\n";
    exit(0);
}

/* Check if were up to date */
if (file_exists("{$snortdir}/snortrules-snapshot-2.8.tar.gz.md5")){
$md5_check_new_parse = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
$md5_check_new = `/bin/echo "{$md5_check_new_parse}" | /usr/bin/awk '{ print $4 }'`;
$md5_check_old_parse = file_get_contents("{$snortdir}/{$snort_filename_md5}");
$md5_check_old = `/bin/echo "{$md5_check_old_parse}" | /usr/bin/awk '{ print $4 }'`;
/* Write out time of last sucsessful md5 to cache */
$config['installedpackages']['snort']['last_md5_download'] = date("Y-M-jS-h:i-A");
write_config();
if ($md5_check_new == $md5_check_old) {
        update_status(gettext("Your rules are up to date..."));
        update_output_window(gettext("You may start Snort now, check update."));
        hide_progress_bar_status();
        /* Timestamps to html  */
        echo "\n<p align=center><b>You last checked for updates: </b>{$last_md5_download}</p>\n";
        echo "\n<p align=center><b>You last installed for rules: </b>{$last_rules_install}</p>\n";
//        echo "P is this code {$premium_subscriber}";
        echo "\n\n</body>\n</html>\n";
        exit(0);
    }
}

/* "You are Not Up to date */;
update_status(gettext("You are NOT up to date..."));

/* download snortrules file */
if (file_exists("{$tmpfname}/{$snort_filename}")) {
    update_status(gettext("Snortrule tar file exists..."));
} else {
    update_status(gettext("There is a new set of Snort rules posted. Downloading..."));
    update_output_window(gettext("May take 4 to 10 min..."));
//   download_file_with_progress_bar("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz", $tmpfname . "/{$snort_filename}", "read_body_firmware");
    download_file_with_progress_bar("http://dl.snort.org/{$premium_url}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz?oink_code={$oinkid}", $tmpfname . "/{$snort_filename}", "read_body_firmware");
    update_all_status($static_output);
    update_status(gettext("Done downloading rules file."));
}


/* Compair md5 sig to file sig */

$premium_url_chk = $config['installedpackages']['snort']['config'][0]['subscriber'];
if ($premium_url_chk == on) {
$md5 = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
$file_md5_ondisk = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
 if ($md5 == $file_md5_ondisk) {
    update_status(gettext("Valid md5 checksum pass..."));
} else {
    update_status(gettext("The downloaded file does not match the md5 file...P is ON"));
    update_output_window(gettext("Error md5 Mismatch..."));
    exit(0);
  }
}

$premium_url_chk = $config['installedpackages']['snort']['config'][0]['subscriber'];
if ($premium_url_chk != on) {
$md55 = `/bin/cat {$tmpfname}/{$snort_filename_md5} | /usr/bin/awk '{ print $4 }'`;
$file_md5_ondisk2 = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
 if ($md55 == $file_md5_ondisk2) {
    update_status(gettext("Valid md5 checksum pass..."));
} else {
    update_status(gettext("The downloaded file does not match the md5 file...Not P"));
    update_output_window(gettext("Error md5 Mismatch..."));
    exit(0);
    }
}

/* Untar snort rules file individually to help people with low system specs */
if (file_exists("{$tmpfname}/$snort_filename")) {
    update_status(gettext("Extracting rules..."));
    update_output_window(gettext("May take a while..."));
    exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} etc/");
	exec("`/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/precompiled/FreeBSD-7.0/i386/2.8.4/*`");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/bad-traffic.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/chat.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/dos.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/exploit.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/imap.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/misc.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/multimedia.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/netbios.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/nntp.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/p2p.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/smtp.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/sql.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/web-client.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} so_rules/web-misc.rules/");
    update_status(gettext("Done extracting Rules."));
} else {
    update_status(gettext("The Download rules file missing..."));
    update_output_window(gettext("Error rules extracting failed..."));
    exit(0);
}

$signature_info_chk = $config['installedpackages']['snort']['config'][0]['signatureinfo'];
if ($premium_url_chk == on) {
	update_status(gettext("Extracting Signatures..."));
	update_output_window(gettext("May take a while..."));
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$tmpfname} doc/signatures/");
	update_status(gettext("Done extracting Signatures."));
}

/*  Making Cleaning Snort Directory */
if (file_exists("{$snortdir}")) {
    update_status(gettext("Cleaning the snort Directory..."));
    update_output_window(gettext("removing..."));
	exec("/bin/rm -r {$snortdir}/*");
    exec("/bin/rm -r /usr/local/lib/snort/dynamicrules/*");
} else {
    update_status(gettext("Making Snort Directory..."));
    update_output_window(gettext("should be fast..."));
    exec("/bin/mkdir {$snortdir}");
	exec("/bin/rm -r /usr/local/lib/snort/dynamicrules/*");
    update_status(gettext("Done making snort direcory."));
}

/*  Copy rules dir to snort dir */
if (file_exists("{$tmpfname}/rules")) {
    update_status(gettext("Copying rules..."));
    update_output_window(gettext("May take a while..."));
    exec("/bin/mv -f {$tmpfname}/rules {$snortdir}/rules");
    update_status(gettext("Done copping rules."));
    /* Write out time of last sucsessful rule install catch */
    $config['installedpackages']['snort']['last_rules_install'] = date("Y-M-jS-h:i-A");
    write_config();
} else {
    update_status(gettext("Directory rules does not exists..."));
    update_output_window(gettext("Error copping rules direcory..."));
    exit(0);
}

/*  Copy md5 sig to snort dir */
if (file_exists("{$tmpfname}/$snort_filename_md5")) {
    update_status(gettext("Copying md5 sig to snort directory..."));
    exec("/bin/cp {$tmpfname}/$snort_filename_md5 {$snortdir}/$snort_filename_md5");
} else {
    update_status(gettext("The md5 file does not exist..."));
    update_output_window(gettext("Error copping config..."));
    exit(0);
}

/*  Copy configs to snort dir */
if (file_exists("{$tmpfname}/etc/Makefile.am")) {
    update_status(gettext("Copying configs to snort directory..."));
    exec("/bin/cp {$tmpfname}/etc/* {$snortdir}");
} else {
    update_status(gettext("The snort configs does not exist..."));
    update_output_window(gettext("Error copping config..."));
    exit(0);
}

/*  Copy signatures dir to snort dir */
$signature_info_chk = $config['installedpackages']['snort']['config'][0]['signatureinfo'];
if ($premium_url_chk == on) {
if (file_exists("{$tmpfname}/doc/signatures")) {
    update_status(gettext("Copying signatures..."));
    update_output_window(gettext("May take a while..."));
    exec("/bin/mv -f {$tmpfname}/doc/signatures {$snortdir}/signatures");
    update_status(gettext("Done copying signatures."));
} else {
    update_status(gettext("Directory signatures exist..."));
    update_output_window(gettext("Error copping signature..."));
    exit(0);
  }
}

/*  Copy so_rules dir to snort lib dir */
if (file_exists("{$tmpfname}/so_rules/precompiled/FreeBSD-7.0/i386/2.8.4/")) {
    update_status(gettext("Copying so_rules..."));
    update_output_window(gettext("May take a while..."));
    exec("`/bin/cp -f {$tmpfname}/so_rules/precompiled/FreeBSD-7.0/i386/2.8.4/* /usr/local/lib/snort/dynamicrules/`");
	exec("/bin/cp {$tmpfname}/so_rules/bad-traffic.rules {$snortdir}/rules/bad-traffic.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/chat.rules {$snortdir}/rules/chat.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/dos.rules {$snortdir}/rules/dos.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/exploit.rules {$snortdir}/rules/exploit.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/imap.rules {$snortdir}/rules/imap.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/misc.rules {$snortdir}/rules/misc.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/multimedia.rules {$snortdir}/rules/multimedia.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/netbios.rules {$snortdir}/rules/netbios.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/nntp.rules {$snortdir}/rules/nntp.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/p2p.rules {$snortdir}/rules/p2p.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/smtp.rules {$snortdir}/rules/smtp.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/sql.rules {$snortdir}/rules/sql.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/web-client.rules {$snortdir}/rules/web-client.so.rules");
	exec("/bin/cp {$tmpfname}/so_rules/web.misc.rules {$snortdir}/rules/web.misc.so.rules");
    update_status(gettext("Done copying so_rules."));
} else {
    update_status(gettext("Directory so_rules does not exist..."));
    update_output_window(gettext("Error copping so_rules..."));
    exit(0);
}


/* php code finish */
update_status(gettext("Rules update finished..."));
update_output_window(gettext("You may start Snort now finnal."));

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

</body>
</html>
