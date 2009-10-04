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
$snortdir = "/usr/local/etc/snort_bkup";
$snortdir_wan = "/usr/local/etc/snort";
$snort_filename_md5 = "snortrules-snapshot-2.8.tar.gz.md5";
$snort_filename = "snortrules-snapshot-2.8.tar.gz";
$emergingthreats_filename_md5 = "version.txt";
$emergingthreats_filename = "emerging.rules.tar.gz";
$pfsense_rules_filename_md5 = "pfsense_rules.tar.gz.md5";
$pfsense_rules_filename = "pfsense_rules.tar.gz";

require("/usr/local/pkg/snort.inc");
require_once("config.inc");

?>


<?php

$up_date_time = date('l jS \of F Y h:i:s A');
echo "";
echo "#########################";
echo "$up_date_time";
echo "#########################";
echo "";

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
        echo "Please add you oink code\n";
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

/* send current buffer */
ob_flush();

/*  remove old $tmpfname files */
if (file_exists("{$tmpfname}")) {
    exec("/bin/rm -r {$tmpfname}");
	apc_clear_cache();
}

/* send current buffer */
ob_flush();

/* If tmp dir does not exist create it */
if (file_exists($tmpfname)) {
    echo "The directory tmp exists...\n";
} else {
    mkdir("{$tmpfname}", 700);
}

/*  download md5 sig from snort.org */
if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
    echo "md5 temp file exists...\n";
} else {
    echo "Downloading md5 file...\n";
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://dl.snort.org/{$premium_url}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz.md5?oink_code={$oinkid}");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz.md5");
    $f = fopen("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5", 'w');
    fwrite($f, $image);
    fclose($f);
    echo "Done. downloading md5\n";
}

/*  download md5 sig from emergingthreats.net */
$emergingthreats_url_chk = $config['installedpackages']['snort']['config'][0]['emergingthreats'];
if ($emergingthreats_url_chk == on) {
    echo "Downloading md5 file...\n";
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://www.emergingthreats.net/version.txt");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/version.txt");
    $f = fopen("{$tmpfname}/version.txt", 'w');
    fwrite($f, $image);
    fclose($f);
    echo "Done. downloading md5\n";
}

/*  download md5 sig from pfsense.org */
if (file_exists("{$tmpfname}/{$pfsense_rules_filename_md5}")) {
    echo "md5 temp file exists...\n";
} else {
    echo "Downloading pfsense md5 file...\n";
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://www.pfsense.com/packages/config/snort/pfsense_rules/pfsense_rules.tar.gz.md5");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/pfsense_rules.tar.gz.md5");
    $f = fopen("{$tmpfname}/pfsense_rules.tar.gz.md5", 'w');
    fwrite($f, $image);
    fclose($f);
    echo "Done. downloading md5\n";
}

/*  Time stamps define */
$last_md5_download = $config['installedpackages']['snort']['last_md5_download'];
$last_rules_install = $config['installedpackages']['snort']['last_rules_install'];

/* If md5 file is empty wait 15min exit */
if (0 == filesize("{$tmpfname}/snortrules-snapshot-2.8.tar.gz.md5")){
    echo "Please wait... You may only check for New Rules every 15 minutes...\n";
    echo "Rules are released every month from snort.org. You may download the Rules at any time.\n";
    exit(0);
}

/* If emergingthreats md5 file is empty wait 15min exit not needed */

/* If pfsense md5 file is empty wait 15min exit */
if (0 == filesize("{$tmpfname}/$pfsense_rules_filename_md5")){
    echo "Please wait... You may only check for New Pfsense Rules every 15 minutes...\n";
    echo "Rules are released to support Pfsense packages.\n";
    exit(0);
}

/* Check if were up to date snort.org */
if (file_exists("{$snortdir}/snortrules-snapshot-2.8.tar.gz.md5")){
$md5_check_new_parse = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
$md5_check_new = `/bin/echo "{$md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
$md5_check_old_parse = file_get_contents("{$snortdir}/{$snort_filename_md5}");
$md5_check_old = `/bin/echo "{$md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
/* Write out time of last sucsessful md5 to cache */
$config['installedpackages']['snort']['last_md5_download'] = date("Y-M-jS-h:i-A");
write_config();
if ($md5_check_new == $md5_check_old) {
        echo "Your rules are up to date...\n";
        echo "You may start Snort now, check update.\n";
        $snort_md5_check_ok = on;
    }
}

/* Check if were up to date emergingthreats.net */
$emergingthreats_url_chk = $config['installedpackages']['snort']['config'][0]['emergingthreats'];
if ($emergingthreats_url_chk == on) {
if (file_exists("{$snortdir}/version.txt")){
$emerg_md5_check_new_parse = file_get_contents("{$tmpfname}/version.txt");
$emerg_md5_check_new = `/bin/echo "{$emerg_md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
$emerg_md5_check_old_parse = file_get_contents("{$snortdir}/version.txt");
$emerg_md5_check_old = `/bin/echo "{$emerg_md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
/* Write out time of last sucsessful md5 to cache */
$config['installedpackages']['snort']['last_md5_download'] = date("Y-M-jS-h:i-A");
write_config();
if ($emerg_md5_check_new == $emerg_md5_check_old) {
        echo "Your emergingthreats rules are up to date...\n";
        echo "You may start Snort now, check update.\n";
        $emerg_md5_check_chk_ok = on;
    }
  }
}

/* Check if were up to date pfsense.org */
if (file_exists("{$snortdir}/$pfsense_rules_filename_md5")){
$pfsense_md5_check_new_parse = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
$pfsense_md5_check_new = `/bin/echo "{$pfsense_md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
$pfsense_md5_check_old_parse = file_get_contents("{$snortdir}/{$snort_filename_md5}");
$pfsense_md5_check_old = `/bin/echo "{$md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
if ($pfsense_md5_check_new == $pfsense_md5_check_old) {
        $pfsense_md5_check_ok = on;
    }
}

/*  Make Clean Snort Directory emergingthreats not checked */
if ($snort_md5_check_ok == on && $emergingthreats_url_chk != on) {
	echo "Cleaning the snort Directory...\n";
    echo "removing...\n";
	exec("/bin/rm {$snortdir}/rules/emerging*\n");
	exec("/bin/rm {$snortdir}/version.txt");
	echo "Done making cleaning emrg direcory.\n";
}

/* Check if were up to date exits */
if ($snort_md5_check_ok == on && $emerg_md5_check_chk_ok == on && $pfsense_md5_check_ok == on) {
		echo "Your rules are up to date...\n";
		echo "You may start Snort now...\n";
		exit(0);
}

if ($snort_md5_check_ok == on && $pfsense_md5_check_ok == on && $emergingthreats_url_chk != on) {
		echo "Your rules are up to date...\n";
		echo "You may start Snort now...\n";
		exit(0);
}
		
/* You are Not Up to date, always stop snort when updating rules for low end machines */;
echo "You are NOT up to date...\n";
echo "Stopping Snort service...\n";
$chk_if_snort_up = exec("pgrep -x snort");
if ($chk_if_snort_up != "") {
	exec("/usr/bin/touch /tmp/snort_download_halt.pid");
	stop_service("snort");
	sleep(2);
}

/* download snortrules file */
if ($snort_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$snort_filename}")) {
    echo "Snortrule tar file exists...\n";
} else {

    echo "There is a new set of Snort rules posted. Downloading...\n";
    echo "May take 4 to 10 min...\n";
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://dl.snort.org/{$premium_url}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz?oink_code={$oinkid}");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-2.8{$premium_subscriber}.tar.gz");
    $f = fopen("{$tmpfname}/snortrules-snapshot-2.8.tar.gz", 'w');
    fwrite($f, $image);
    fclose($f);
    echo "Done downloading rules file.\n";
    if (150000 > filesize("{$tmpfname}/$snort_filename")){
          echo "Error with the snort rules download...\n";
          echo "Snort rules file downloaded failed...\n";
          exit(0);
  }		  
 }
}

/* download emergingthreats rules file */
if ($emergingthreats_url_chk == on) {
if ($emerg_md5_check_chk_ok != on) {
if (file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
    echo "Emergingthreats tar file exists...\n";
} else {
    echo "There is a new set of Emergingthreats rules posted. Downloading...\n";
    echo "May take 4 to 10 min...\n";	
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://www.emergingthreats.net/rules/emerging.rules.tar.gz");
//    $image = @file_get_contents("http://www.emergingthreats.net/rules/emerging.rules.tar.gz");
    $f = fopen("{$tmpfname}/emerging.rules.tar.gz", 'w');
    fwrite($f, $image);
    fclose($f);	
    echo "Done downloading Emergingthreats rules file.\n";
   }
  }
 }

/* download pfsense rules file */
if ($pfsense_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
    echo "Snortrule tar file exists...\n";
} else {

    echo "There is a new set of Pfsense rules posted. Downloading...\n";
    echo "May take 4 to 10 min...\n";	
    ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
    $image = @file_get_contents("http://www.pfsense.com/packages/config/snort/pfsense_rules/pfsense_rules.tar.gz");
//    $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/pfsense_rules.tar.gz");
    $f = fopen("{$tmpfname}/pfsense_rules.tar.gz", 'w');
    fwrite($f, $image);
    fclose($f);			
    echo "Done downloading rules file.\n";
 }
}

/* Untar snort rules file individually to help people with low system specs */
if ($snort_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$snort_filename}")) {
    echo "Extracting rules...\n";
    echo "May take a while...\n";
    exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} etc/");
	exec("`/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/precompiled/FreeBSD-7.0/i386/2.8.4/*`");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/bad-traffic.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/chat.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/dos.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/exploit.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/imap.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/misc.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/multimedia.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/netbios.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/nntp.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/p2p.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/smtp.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/sql.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/web-client.rules/");
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/web-misc.rules/");
    echo "Done extracting Rules.\n";
} else {
    echo "The Download rules file missing...\n";
    echo "Error rules extracting failed...\n";
    exit(0);
 }
}

/* Untar emergingthreats rules to tmp */
if ($emergingthreats_url_chk == on) {
if ($emerg_md5_check_chk_ok != on) {
if (file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
    echo "Extracting rules...\n";
    echo "May take a while...\n";
    exec("/usr/bin/tar xzf {$tmpfname}/{$emergingthreats_filename} -C {$snortdir} rules/");
  }
 }
}

/* Untar Pfsense rules to tmp */
if ($pfsense_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
    echo "Extracting Pfsense rules...\n";
    echo "May take a while...\n";
    exec("/usr/bin/tar xzf {$tmpfname}/{$pfsense_rules_filename} -C {$snortdir} rules/");
 }
}

/* Untar snort signatures */
if ($snort_md5_check_ok != on) {
if (file_exists("{$tmpfname}/{$snort_filename}")) {
$signature_info_chk = $config['installedpackages']['snortadvanced']['config'][0]['signatureinfo'];
if ($premium_url_chk == on) {
	echo "Extracting Signatures...\n";
	echo "May take a while...\n";
	exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
	echo "Done extracting Signatures.\n";
  }
 }
}

/*  Make Clean Snort Directory */
//if ($snort_md5_check_ok != on && $emerg_md5_check_chk_ok != on && $pfsense_md5_check_ok != on) {
//if (file_exists("{$snortdir}/rules")) {
//    echo "Cleaning the snort Directory...\n";
//    echo "removing...\n";
//    exec("/bin/mkdir -p {$snortdir}");
//	exec("/bin/mkdir -p {$snortdir}/rules");
//	exec("/bin/mkdir -p {$snortdir}/signatures");
//	exec("/bin/rm {$snortdir}/*");
//	exec("/bin/rm {$snortdir}/rules/*");
//	exec("/bin/rm {$snortdir_wan}/*");
//	exec("/bin/rm {$snortdir_wan}/rules/*");
//    exec("/bin/rm /usr/local/lib/snort/dynamicrules/*");	
//} else {
//    echo "Making Snort Directory...\n";
//    echo "should be fast...\n";
//    exec("/bin/mkdir {$snortdir}");
//	exec("/bin/mkdir {$snortdir}/rules");
//	exec("/bin/rm {$snortdir_wan}/\*");
//	exec("/bin/rm {$snortdir_wan}/rules/*");
//	exec("/bin/rm /usr/local/lib/snort/dynamicrules/\*");
//    echo "Done making snort direcory.\n";
//  }
//}

/*  Copy so_rules dir to snort lib dir */
if ($snort_md5_check_ok != on) {
if (file_exists("{$snortdir}/so_rules/precompiled/FreeBSD-7.0/i386/2.8.4/")) {
    echo "Copying so_rules...\n";
    echo "May take a while...\n";
	sleep(2);
    exec("`/bin/cp -f {$snortdir}/so_rules/precompiled/FreeBSD-7.0/i386/2.8.4/* /usr/local/lib/snort/dynamicrules/`");
	exec("/bin/cp {$snortdir}/so_rules/bad-traffic.rules {$snortdir}/rules/bad-traffic.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/chat.rules {$snortdir}/rules/chat.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/dos.rules {$snortdir}/rules/dos.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/exploit.rules {$snortdir}/rules/exploit.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/imap.rules {$snortdir}/rules/imap.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/misc.rules {$snortdir}/rules/misc.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/multimedia.rules {$snortdir}/rules/multimedia.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/netbios.rules {$snortdir}/rules/netbios.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/nntp.rules {$snortdir}/rules/nntp.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/p2p.rules {$snortdir}/rules/p2p.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/smtp.rules {$snortdir}/rules/smtp.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/sql.rules {$snortdir}/rules/sql.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/web-client.rules {$snortdir}/rules/web-client.so.rules");
	exec("/bin/cp {$snortdir}/so_rules/web-misc.rules {$snortdir}/rules/web-misc.so.rules");
	exec("/bin/rm -r {$snortdir}/so_rules");
    echo "Done copying so_rules.\n";
} else {
    echo "Directory so_rules does not exist...\n";
    echo "Error copping so_rules...\n";
    exit(0);
 }
}

/*  enable disable setting will carry over with updates */
/*  TODO carry signature changes with the updates */
if ($snort_md5_check_ok != on || $emerg_md5_check_chk_ok != on || $pfsense_md5_check_ok != on) {

if (!empty($config['installedpackages']['snort']['rule_sid_on'])) {
$enabled_sid_on = $config['installedpackages']['snort']['rule_sid_on'];
$enabled_sid_on_array = split('\|\|', $enabled_sid_on);
foreach($enabled_sid_on_array as $enabled_item_on)
$selected_sid_on_sections .= "$enabled_item_on\n";
	}

if (!empty($config['installedpackages']['snort']['rule_sid_off'])) {
$enabled_sid_off = $config['installedpackages']['snort']['rule_sid_off'];
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

url = dir:///usr/local/etc/snort_bkup/rules

$selected_sid_on_sections

$selected_sid_off_sections

EOD;

     /* open snort's threshold.conf for writing */
     $oinkmasterlist = fopen("/usr/local/etc/snort_bkup/oinkmaster.conf", "w");

     fwrite($oinkmasterlist, "$snort_sid_text");

     /* close snort's threshold.conf file */
     fclose($oinkmasterlist);

}

/*  Copy configs to snort dir */
if ($snort_md5_check_ok != on) {
if (file_exists("{$snortdir}/etc/Makefile.am")) {
    echo "Copying configs to snort directory...\n";
    exec("/bin/cp {$snortdir}/etc/* {$snortdir}");
	exec("/bin/rm -r {$snortdir}/etc");
} else {
    echo "The snort configs does not exist...\n";
    echo "Error copping config...\n";
    exit(0);
 }
}

/*  Copy md5 sig to snort dir */
if ($snort_md5_check_ok != on) {
if (file_exists("{$tmpfname}/$snort_filename_md5")) {
    echo "Copying md5 sig to snort directory...\n";
    exec("/bin/cp {$tmpfname}/$snort_filename_md5 {$snortdir}/$snort_filename_md5");
} else {
    echo "The md5 file does not exist...\n";
    echo "Error copping config...\n";
    exit(0);
 }
}

/*  Copy emergingthreats md5 sig to snort dir */
if ($emergingthreats_url_chk == on) {
if ($emerg_md5_check_chk_ok != on) {
if (file_exists("{$tmpfname}/$emergingthreats_filename_md5")) {
    echo "Copying md5 sig to snort directory...\n";
    exec("/bin/cp {$tmpfname}/$emergingthreats_filename_md5 {$snortdir}/$emergingthreats_filename_md5");
} else {
    echo "The emergingthreats md5 file does not exist...\n";
    echo "Error copping config...\n";
    exit(0);
  }
 }
}

/*  Copy Pfsense md5 sig to snort dir */
if ($pfsense_md5_check_ok != on) {
if (file_exists("{$tmpfname}/$pfsense_rules_filename_md5")) {
    echo "Copying Pfsense md5 sig to snort directory...\n";
    exec("/bin/cp {$tmpfname}/$pfsense_rules_filename_md5 {$snortdir}/$pfsense_rules_filename_md5");
} else {
    echo "The Pfsense md5 file does not exist...\n";
    echo "Error copping config...\n";
    exit(0);
 }
}

/*  Copy signatures dir to snort dir */
if ($snort_md5_check_ok != on) {
$signature_info_chk = $config['installedpackages']['snort']['config'][0]['signatureinfo'];
if ($premium_url_chk == on) {
if (file_exists("{$snortdir}/doc/signatures")) {
    echo "Copying signatures...\n";
    echo "May take a while...\n";
    exec("/bin/mv -f {$snortdir}/doc/signatures {$snortdir}/signatures");
	exec("/bin/rm -r {$snortdir}/doc/signatures");
    echo "Done copying signatures.\n";
} else {
    echo "Directory signatures exist...\n";
    echo "Error copping signature...\n";
    exit(0);
  }
 }
}

/* double make shure clean up emerg rules that dont belong */
if (file_exists("/usr/local/etc/snort_bkup/rules/emerging-botcc-BLOCK.rules")) {
	apc_clear_cache();
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-botcc-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-botcc.rules");
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-compromised-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-drop-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-dshield-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-rbn-BLOCK.rules");
	exec("/bin/rm /usr/local/etc/snort_bkup/rules/emerging-tor-BLOCK.rules");
}

if (file_exists("/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so")) {
	exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so");
	exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example\*");
}

echo "Updating Alert Messages...\n";
echo "Please Wait...\n";
sleep(2);
exec("/usr/local/bin/perl /usr/local/bin/create-sidmap.pl /usr/local/etc/snort_bkup/rules > /usr/local/etc/snort_bkup/sid-msg.map");

/*  Run oinkmaster to snort_wan and cp configs */
/*  If oinkmaster is not needed cp rules normally */
/*  TODO add per interface settings here */
if ($snort_md5_check_ok != on || $emerg_md5_check_chk_ok != on || $pfsense_md5_check_ok != on) {

	if (empty($config['installedpackages']['snort']['rule_sid_on']) || empty($config['installedpackages']['snort']['rule_sid_off'])) {
echo "Your first set of rules are being copied...\n";
echo "May take a while...\n";

    exec("/bin/cp {$snortdir}/rules/* {$snortdir_wan}/rules/");
	exec("/bin/cp {$snortdir}/classification.config {$snortdir_wan}");
	exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir_wan}");
	exec("/bin/cp {$snortdir}/generators {$snortdir_wan}");
	exec("/bin/cp {$snortdir}/reference.config {$snortdir_wan}");
	exec("/bin/cp {$snortdir}/sid {$snortdir_wan}");
	exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir_wan}");
	exec("/bin/cp {$snortdir}/unicode.map {$snortdir_wan}");

} else {
		echo "Your enable and disable changes are being applied to your fresh set of rules...\n";
		echo "May take a while...\n";
		exec("/bin/cp {$snortdir}/rules/* {$snortdir_wan}/rules/");
		exec("/bin/cp {$snortdir}/classification.config {$snortdir_wan}");
		exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir_wan}");
		exec("/bin/cp {$snortdir}/generators {$snortdir_wan}");
		exec("/bin/cp {$snortdir}/reference.config {$snortdir_wan}");
		exec("/bin/cp {$snortdir}/sid {$snortdir_wan}");
		exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir_wan}");
		exec("/bin/cp {$snortdir}/unicode.map {$snortdir_wan}");

		/*  oinkmaster.pl will convert saved changes for the new updates then we have to change #alert to # alert for the gui */
		/*  might have to add a sleep for 3sec for flash drives or old drives */
		exec("/usr/local/bin/perl /usr/local/bin/oinkmaster.pl -C /usr/local/etc/snort_bkup/oinkmaster.conf -o /usr/local/etc/snort/rules > /usr/local/etc/snort_bkup/oinkmaster.log");
		exec("/usr/local/bin/perl -pi -e 's/#alert/# alert/g' /usr/local/etc/snort/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/##alert/# alert/g' /usr/local/etc/snort/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/## alert/# alert/g' /usr/local/etc/snort/rules/*.rules");
				
	}
}

/*  remove old $tmpfname files */
if (file_exists("{$tmpfname}")) {
    echo "Cleaning up...\n";
    exec("/bin/rm -r /tmp/snort_rules_up");
}

/* php code to flush out cache some people are reportting missing files this might help  */
sleep(5);
apc_clear_cache();
exec("/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync ;/bin/sync");

/* if snort is running hardrestart, if snort is not running do nothing */
if (file_exists("/tmp/snort_download_halt.pid")) {
	start_service("snort");
	echo "The Rules update finished...\n";
	echo "Snort has restarted with your new set of rules...\n";
	exec("/bin/rm /tmp/snort_download_halt.pid");
} else {
		echo "The Rules update finished...\n";
		echo "You may start snort now...\n";
}

?>
