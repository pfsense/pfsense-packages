<?php
/*
 snort_check_for_rule_updates.php
 Copyright (C) 2006 Scott Ullrich
 Copyright (C) 2009 Robert Zelaya
 Copyright (C) 2011 Ermal Luci
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

/* TODO: review if include files are needed */
require_once("functions.inc");
require_once("service-utils.inc");
require_once("/usr/local/pkg/snort/snort.inc");

$pkg_interface = "console";

$tmpfname = "/usr/local/etc/snort/tmp/snort_rules_up";
$snortdir = "/usr/local/etc/snort";
$snortdir_wan = "/usr/local/etc/snort";
$snort_filename_md5 = "{$snort_rules_file}.md5";
$snort_filename = "{$snort_rules_file}";
$emergingthreats_filename_md5 = "emerging.rules.tar.gz.md5";
$emergingthreats_filename = "emerging.rules.tar.gz";
$pfsense_rules_filename_md5 = "pfsense_rules.tar.gz.md5";
$pfsense_rules_filename = "pfsense_rules.tar.gz";

/*  Time stamps define */
$last_md5_download = $config['installedpackages']['snortglobal']['last_md5_download'];
$last_rules_install = $config['installedpackages']['snortglobal']['last_rules_install'];

$up_date_time = date('l jS \of F Y h:i:s A');
echo "\n";
echo "#########################\n";
echo "$up_date_time\n";
echo "#########################\n";
echo "\n\n";

/* define checks */
$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingthreats = $config['installedpackages']['snortglobal']['emergingthreats'];

if ($snortdownload == 'off' && $emergingthreats != 'on')
	$snort_emrging_info = 'stop';

if ($oinkid == "" && $snortdownload != 'off')
	$snort_oinkid_info = 'stop';

/* check if main rule directory is empty */
$if_mrule_dir = "/usr/local/etc/snort/rules";
$mfolder_chk = (count(glob("$if_mrule_dir/*")) === 0) ? 'empty' : 'full';

if (file_exists('/var/run/snort.conf.dirty'))
	$snort_dirty_d = 'stop';

/* Start of code */
conf_mount_rw();

if (!is_dir('/usr/local/etc/snort/tmp'))
	exec('/bin/mkdir -p /usr/local/etc/snort/tmp');

$snort_md5_check_ok = 'off';
$emerg_md5_check_ok = 'off';
$pfsense_md5_check_ok = 'off';

/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","150M");

/* mark the time update started */
$config['installedpackages']['snortglobal']['last_md5_download'] = date("Y-M-jS-h:i-A");

/* send current buffer */
ob_flush();

/* send current buffer */
ob_flush();

/*  remove old $tmpfname files */
if (is_dir("{$tmpfname}")) {
	update_status(gettext("Removing old tmp files..."));
	exec("/bin/rm -r {$tmpfname}");
	apc_clear_cache();
}

/*  Make shure snortdir exits */
exec("/bin/mkdir -p {$snortdir}");
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p {$tmpfname}");
exec("/bin/mkdir -p /usr/local/lib/snort/dynamicrules/");

/* send current buffer */
ob_flush();

$pfsensedownload = 'on';

/*  download md5 sig from snort.org */
if ($snortdownload == 'on')
{
	if (file_exists("{$tmpfname}/{$snort_filename_md5}") &&
	filesize("{$tmpfname}/{$snort_filename_md5}") > 0) {
		update_status(gettext("snort.org md5 temp file exists..."));
	} else {
		update_status(gettext("Downloading snort.org md5 file..."));
		ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');

		//$image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename_md5}");
		$image = @file_get_contents("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename_md5}");
		@file_put_contents("{$tmpfname}/{$snort_filename_md5}", $image);
		update_status(gettext("Done downloading snort.org md5"));
	}
}

/*  download md5 sig from emergingthreats.net */
if ($emergingthreats == 'on')
{
	update_status(gettext("Downloading emergingthreats md5 file..."));
	ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
	// $image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/version.txt");
	$image = @file_get_contents('http://rules.emergingthreats.net/open/snort-2.9.0/emerging.rules.tar.gz.md5');
	@file_put_contents("{$tmpfname}/{$emergingthreats_filename_md5}", $image);
	update_status(gettext("Done downloading emergingthreats md5"));
}

/*  download md5 sig from pfsense.org */
if (file_exists("{$tmpfname}/{$pfsense_rules_filename_md5}")) {
	update_status(gettext("pfsense md5 temp file exists..."));
} else {
	update_status(gettext("Downloading pfsense md5 file..."));
	ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
	//$image = @file_get_contents("http://www.mtest.local/pub-bin/oinkmaster.cgi/{$oinkid}/pfsense_rules.tar.gz.md5");
	$image = @file_get_contents("http://www.pfsense.com/packages/config/snort/pfsense_rules/pfsense_rules.tar.gz.md5");
	@file_put_contents("{$tmpfname}/pfsense_rules.tar.gz.md5", $image);
	update_status(gettext("Done downloading pfsense md5."));
}

/* If md5 file is empty wait 15min exit */
if ($snortdownload == 'on')
{
	if (0 == filesize("{$tmpfname}/{$snort_filename_md5}"))
	{
		update_status(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		update_output_window(gettext("Rules are released every month from snort.org. You may download the Rules at any time."));
		$snortdownload = 'off';
	}
}

/* If pfsense md5 file is empty wait 15min exit */
if (0 == filesize("{$tmpfname}/$pfsense_rules_filename_md5")){
	update_status(gettext("Please wait... You may only check for New Pfsense Rules every 15 minutes..."));
	update_output_window(gettext("Rules are released to support Pfsense packages."));
	$pfsensedownload = 'off';
}

/* Check if were up to date snort.org */
if ($snortdownload == 'on')
{
	if (file_exists("{$snortdir}/{$snort_filename_md5}"))
	{
		$md5_check_new_parse = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
		$md5_check_new = `/bin/echo "{$md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
		$md5_check_old_parse = file_get_contents("{$snortdir}/{$snort_filename_md5}");
		$md5_check_old = `/bin/echo "{$md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
		if ($md5_check_new == $md5_check_old)
		{
			update_status(gettext("Your rules are up to date..."));
			update_output_window(gettext("You may start Snort now, check update."));
			$snort_md5_check_ok = 'on';
		} else {
			update_status(gettext("Your rules are not up to date..."));
			$snort_md5_check_ok = 'off';
		}
	}
}

/* Check if were up to date emergingthreats.net */
if ($emergingthreats == 'on')
{
	if (file_exists("{$snortdir}/{$emergingthreats_filename_md5}"))
	{
		$emerg_md5_check_new_parse = file_get_contents("{$tmpfname}/{$emergingthreats_filename_md5}");
		$emerg_md5_check_new = `/bin/echo "{$emerg_md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
		$emerg_md5_check_old_parse = file_get_contents("{$snortdir}/{$emergingthreats_filename_md5}");
		$emerg_md5_check_old = `/bin/echo "{$emerg_md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
		if ($emerg_md5_check_new == $emerg_md5_check_old)
		{
			$emerg_md5_check_ok = 'on';
		} else
			$emerg_md5_check_ok = 'off';
	}
}

/* Check if were up to date pfsense.org */
if ($pfsensedownload == 'on' && file_exists("{$snortdir}/pfsense_rules.tar.gz.md5"))
{
	$pfsense_check_new_parse = file_get_contents("{$tmpfname}/pfsense_rules.tar.gz.md5");
	$pfsense_md5_check_new = `/bin/echo "{$pfsense_md5_check_new_parse}" | /usr/bin/awk '{ print $1 }'`;
	$pfsense_md5_check_old_parse = file_get_contents("{$snortdir}/pfsense_rules.tar.gz.md5");
	$pfsense_md5_check_old = `/bin/echo "{$pfsense_md5_check_old_parse}" | /usr/bin/awk '{ print $1 }'`;
	if ($pfsense_md5_check_new == $pfsense_md5_check_old)
	{
		$pfsense_md5_check_ok = 'on';
	} else
		$pfsense_md5_check_ok = 'off';
}

if ($snortdownload == 'on') {
	if ($snort_md5_check_ok == 'on')
	{
		update_status(gettext("Your snort.org rules are up to date..."));
		update_output_window(gettext("You may start Snort now..."));
		$snortdownload = 'off';
	}
}
if ($emergingthreats == 'on') {
	if ($emerg_md5_check_ok == 'on')
	{
		update_status(gettext("Your Emergingthreats rules are up to date..."));
		update_output_window(gettext("You may start Snort now..."));
		$emergingthreats = 'off';
	}
}

/* download snortrules file */
if ($snortdownload == 'on')
{
	if ($snort_md5_check_ok != 'on') {
		if (file_exists("{$tmpfname}/{$snort_filename}")) {
			update_status(gettext("Snortrule tar file exists..."));
		} else {
			update_status(gettext("There is a new set of Snort.org rules posted. Downloading..."));
			update_output_window(gettext("May take 4 to 10 min..."));
			download_file_with_progress_bar("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", "{$tmpfname}/{$snort_filename}");
			update_all_status($static_output);
			update_status(gettext("Done downloading rules file."));
			if (300000 > filesize("{$tmpfname}/$snort_filename")){
				update_status(gettext("Error with the snort rules download..."));
				update_output_window(gettext("Snort rules file downloaded failed..."));
				$snortdownload = 'off';
			}
		}
	}
}

/* download emergingthreats rules file */
if ($emergingthreats == "on")
{
	if ($emerg_md5_check_ok != 'on')
	{
		if (file_exists("{$tmpfname}/{$emergingthreats_filename}"))
		{
			update_status(gettext('Emergingthreats tar file exists...'));
		}else{
			update_status(gettext("There is a new set of Emergingthreats rules posted. Downloading..."));
			update_output_window(gettext("May take 4 to 10 min..."));
			download_file_with_progress_bar('http://rules.emergingthreats.net/open/snort-2.9.0/emerging.rules.tar.gz', "{$tmpfname}/{$emergingthreats_filename}");
			update_status(gettext('Done downloading Emergingthreats rules file.'));
		}
	}
}

/* download pfsense rules file */
if ($pfsensedownload == 'on' && $pfsense_md5_check_ok != 'on') {
	if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
		update_status(gettext("Snortrule tar file exists..."));
	} else {
		update_status(gettext("There is a new set of Pfsense rules posted. Downloading..."));
		update_output_window(gettext("May take 4 to 10 min..."));
		download_file_with_progress_bar("http://www.pfsense.com/packages/config/snort/pfsense_rules/pfsense_rules.tar.gz", $tmpfname . "/{$pfsense_rules_filename}");
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
//    return;
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
//    return;
//    }
//}

/* Untar snort rules file individually to help people with low system specs */
if ($snortdownload == 'on')
{
	if ($snort_md5_check_ok != 'on') {
		if (file_exists("{$tmpfname}/{$snort_filename}")) {

			if ($pfsense_stable == 'yes')
				$freebsd_version_so = 'FreeBSD-7-2';
			else
				$freebsd_version_so = 'FreeBSD-8-1';

			update_status(gettext("Extracting Snort.org rules..."));
			update_output_window(gettext("May take a while..."));
			/* extract snort.org rules and  add prefix to all snort.org files*/
			exec("/bin/rm -r {$snortdir}/rules");
			sleep(2);
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} rules/");
			chdir ("/usr/local/etc/snort/rules");
			sleep(2);
			exec('/usr/local/bin/perl /usr/local/bin/snort_rename.pl s/^/snort_/ *.rules');

			/* extract so rules */
			exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
			if($snort_arch == 'x86'){
				exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/precompiled/$freebsd_version_so/i386/2.9.0.5/");
				exec("/bin/mv -f {$snortdir}/so_rules/precompiled/$freebsd_version_so/i386/2.9.0.5/* /usr/local/lib/snort/dynamicrules/");
			} else if ($snort_arch == 'x64') {
				exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/precompiled/$freebsd_version_so/x86-64/2.9.0.5/");
				exec("/bin/mv -f {$snortdir}/so_rules/precompiled/$freebsd_version_so/x86-64/2.9.0.5/* /usr/local/lib/snort/dynamicrules/");
                        }
			/* extract so rules none bin and rename */
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/bad-traffic.rules/" .
			" so_rules/chat.rules/" .
			" so_rules/dos.rules/" .
			" so_rules/exploit.rules/" .
			" so_rules/icmp.rules/" .
			" so_rules/imap.rules/" .
			" so_rules/misc.rules/" .
			" so_rules/multimedia.rules/" .
			" so_rules/netbios.rules/" .
			" so_rules/nntp.rules/" .
			" so_rules/p2p.rules/" .
			" so_rules/smtp.rules/" .
			" so_rules/sql.rules/" .
			" so_rules/web-activex.rules/" .
			" so_rules/web-client.rules/" .
			" so_rules/web-iis.rules/" .
			" so_rules/web-misc.rules/");

				exec("/bin/mv -f {$snortdir}/so_rules/bad-traffic.rules {$snortdir}/rules/snort_bad-traffic.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/chat.rules {$snortdir}/rules/snort_chat.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/dos.rules {$snortdir}/rules/snort_dos.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/exploit.rules {$snortdir}/rules/snort_exploit.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/icmp.rules {$snortdir}/rules/snort_icmp.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/imap.rules {$snortdir}/rules/snort_imap.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/misc.rules {$snortdir}/rules/snort_misc.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/multimedia.rules {$snortdir}/rules/snort_multimedia.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/netbios.rules {$snortdir}/rules/snort_netbios.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/nntp.rules {$snortdir}/rules/snort_nntp.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/p2p.rules {$snortdir}/rules/snort_p2p.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/smtp.rules {$snortdir}/rules/snort_smtp.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/sql.rules {$snortdir}/rules/snort_sql.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/web-activex.rules {$snortdir}/rules/snort_web-activex.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/web-client.rules {$snortdir}/rules/snort_web-client.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/web-iis.rules {$snortdir}/rules/snort_web-iis.so.rules");
				exec("/bin/mv -f {$snortdir}/so_rules/web-misc.rules {$snortdir}/rules/snort_web-misc.so.rules");
				exec("/bin/rm -r {$snortdir}/so_rules");
			}

			/* extract base etc files */
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} etc/");
			exec("/bin/mv -f {$snortdir}/etc/* {$snortdir}");
			exec("/bin/rm -r {$snortdir}/etc");

			update_status(gettext("Done extracting Snort.org Rules."));
		}else{
			update_status(gettext("Error extracting Snort.org Rules..."));
			update_output_window(gettext("Error Line 755"));
			$snortdownload = 'off';
		}
}

/* Untar emergingthreats rules to tmp */
if ($emergingthreats == 'on')
{
	if ($emerg_md5_check_ok != 'on')
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
if ($pfsensedownload == 'on' && $pfsense_md5_check_ok != 'on') {
	if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
		update_status(gettext("Extracting Pfsense rules..."));
		update_output_window(gettext("May take a while..."));
		exec("/usr/bin/tar xzf {$tmpfname}/{$pfsense_rules_filename} -C {$snortdir} rules/");
	}
}

/* Untar snort signatures */
if ($snortdownload == 'on' && $snort_md5_check_ok != 'on') {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
		if ($premium_url_chk == 'on') {
			update_status(gettext("Extracting Signatures..."));
			update_output_window(gettext("May take a while..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
			update_status(gettext("Done extracting Signatures."));
		}
	}
}

/*  Copy md5 sig to snort dir */
if ($snortdownload == 'on')
{
	if ($snort_md5_check_ok != 'on') {
		if (file_exists("{$tmpfname}/$snort_filename_md5")) {
			update_status(gettext("Copying md5 sig to snort directory..."));
			exec("/bin/cp {$tmpfname}/$snort_filename_md5 {$snortdir}/$snort_filename_md5");
		}else{
			update_status(gettext("The md5 file does not exist..."));
			update_output_window(gettext("Error copying config..."));
			$snortdownload = 'off';
		}
	}
}

/*  Copy emergingthreats md5 sig to snort dir */
if ($emergingthreats == "on")
{
	if ($emerg_md5_check_ok != 'on')
	{
		if (file_exists("{$tmpfname}/$emergingthreats_filename_md5"))
		{
			update_status(gettext("Copying md5 sig to snort directory..."));
			exec("/bin/cp {$tmpfname}/$emergingthreats_filename_md5 {$snortdir}/$emergingthreats_filename_md5");
		}else{
			update_status(gettext("The emergingthreats md5 file does not exist..."));
			update_output_window(gettext("Error copying config..."));
			$emergingthreats = 'off';
		}
	}
}

/*  Copy Pfsense md5 sig to snort dir */
if ($pfsensedownload == 'on' && $pfsense_md5_check_ok != 'on') {
	if (file_exists("{$tmpfname}/$pfsense_rules_filename_md5")) {
		update_status(gettext("Copying Pfsense md5 sig to snort directory..."));
		exec("/bin/cp {$tmpfname}/$pfsense_rules_filename_md5 {$snortdir}/$pfsense_rules_filename_md5");
	} else {
		update_status(gettext("The Pfsense md5 file does not exist..."));
		update_output_window(gettext("Error copying config..."));
		$pfsensedownload = 'off';
	}
}

/*  Copy signatures dir to snort dir */
if ($snortdownload == 'on')
{
	if ($snort_md5_check_ok != 'on')
	{
		$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
		if ($premium_url_chk == 'on')
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
				$snortdownload = 'off';
			}
		}
	}
}

/* double make shure cleanup emerg rules that dont belong */
if (file_exists("/usr/local/etc/snort/rules/emerging-botcc-BLOCK.rules")) {
	apc_clear_cache();
	@unlink("/usr/local/etc/snort/rules/emerging-botcc-BLOCK.rules");
	@unlink("/usr/local/etc/snort/rules/emerging-botcc.rules");
	@unlink("/usr/local/etc/snort/rules/emerging-compromised-BLOCK.rules");
	@unlink("/usr/local/etc/snort/rules/emerging-drop-BLOCK.rules");
	@unlink("/usr/local/etc/snort/rules/emerging-dshield-BLOCK.rules");
	@unlink("/usr/local/etc/snort/rules/emerging-rbn-BLOCK.rules");
	@unlink("/usr/local/etc/snort/rules/emerging-tor-BLOCK.rules");
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
function oinkmaster_conf($id, $if_real, $iface_uuid)
{
	global $config, $g, $snort_md5_check_ok, $emerg_md5_check_ok, $pfsense_md5_check_ok;

	@unlink("/usr/local/etc/snort/snort_{$iface_uuid}_{$if_real}/oinkmaster_{$iface_uuid}_{$if_real}.conf");

	/*  enable disable setting will carry over with updates */
	/*  TODO carry signature changes with the updates */
	if ($snort_md5_check_ok != 'on' || $emerg_md5_check_ok != 'on' || $pfsense_md5_check_ok != 'on') {

		$selected_sid_on_section = "";
		$selected_sid_off_sections = "";

		if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on'])) {
			$enabled_sid_on = trim($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on']);
			$enabled_sid_on_array = split('\|\|', $enabled_sid_on);
			foreach($enabled_sid_on_array as $enabled_item_on)
				$selected_sid_on_sections .= "$enabled_item_on\n";
		}

		if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'])) {
			$enabled_sid_off = trim($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off']);
			$enabled_sid_off_array = split('\|\|', $enabled_sid_off);
			foreach($enabled_sid_off_array as $enabled_item_off)
				$selected_sid_off_sections .= "$enabled_item_off\n";
		}

		if (!empty($selected_sid_off_sections) || !empty($selected_sid_on_section)) {
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
			@file_put_contents("/usr/local/etc/snort/snort_{$iface_uuid}_{$if_real}/oinkmaster_{$iface_uuid}_{$if_real}.conf", $snort_sid_text);
		}
	}
}

/*  Run oinkmaster to snort_wan and cp configs */
/*  If oinkmaster is not needed cp rules normally */
/*  TODO add per interface settings here */
function oinkmaster_run($id, $if_real, $iface_uuid)
{
	global $config, $g, $snortdir_wan, $snortdir, $snort_md5_check_ok, $emerg_md5_check_ok, $pfsense_md5_check_ok;

	if ($snort_md5_check_ok != 'on' || $emerg_md5_check_ok != 'on' || $pfsense_md5_check_ok != 'on') {
		if (empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on']) && empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'])) { 
			update_status(gettext("Your first set of rules are being copied..."));
			update_output_window(gettext("May take a while..."));
			exec("/bin/cp {$snortdir}/rules/* {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}/rules/");
			exec("/bin/cp {$snortdir}/classification.config {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/generators {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/reference.config {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/sid {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/unicode.map {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
		} else {
			update_status(gettext("Your enable and disable changes are being applied to your fresh set of rules..."));
			update_output_window(gettext("May take a while..."));
			exec("/bin/cp {$snortdir}/rules/* {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}/rules/");
			exec("/bin/cp {$snortdir}/classification.config {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/generators {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/reference.config {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/sid {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");
			exec("/bin/cp {$snortdir}/unicode.map {$snortdir_wan}/snort_{$iface_uuid}_{$if_real}");

			/*  might have to add a sleep for 3sec for flash drives or old drives */
			exec("/usr/local/bin/perl /usr/local/bin/oinkmaster.pl -C /usr/local/etc/snort/snort_{$iface_uuid}_{$if_real}/oinkmaster_{$iface_uuid}_{$if_real}.conf -o /usr/local/etc/snort/snort_{$iface_uuid}_{$if_real}/rules > /usr/local/etc/snort/oinkmaster_{$iface_uuid}_{$if_real}.log");

		}
	}
}

/* Start the proccess for every interface rule */
/* TODO: try to make the code smother */
if (is_array($config['installedpackages']['snortglobal']['rule']))
{
	foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {
		$result_lan = $value['interface'];
		$if_real = snort_get_real_interface($result_lan);
		$iface_uuid = $value['uuid'];

		/* make oinkmaster.conf for each interface rule */
		oinkmaster_conf($id, $if_real, $iface_uuid);

		/* run oinkmaster for each interface rule */
		oinkmaster_run($id, $if_real, $iface_uuid);
	}
}

//////////////

/* mark the time update finnished */
$config['installedpackages']['snortglobal']['last_rules_install'] = date("Y-M-jS-h:i-A");

/*  remove old $tmpfname files */
if (is_dir('/usr/local/etc/snort/tmp')) {
	update_status(gettext("Cleaning up..."));
	exec("/bin/rm -r /usr/local/etc/snort/tmp/snort_rules_up");
	sleep(2);
	exec("/bin/rm -r /usr/local/etc/snort/tmp/rules_bk");
}

/* make all dirs snorts */
mwexec("/bin/chmod -R 755  /var/log/snort", true);
mwexec("/bin/chmod -R 755  /usr/local/etc/snort", true);
mwexec("/bin/chmod -R 755  /usr/local/lib/snort", true);

if ($snortdownload == 'off' && $emergingthreats == 'off' && $pfsensedownload == 'off')
	update_output_window(gettext("Finished..."));
else if ($snort_md5_check_ok == 'on' && $emerg_md5_check_ok == 'on' && $pfsense_md5_check_ok == 'on')
	update_output_window(gettext("Finished..."));
else {
	/* You are Not Up to date, always stop snort when updating rules for low end machines */;
	update_status(gettext("You are NOT up to date..."));
	exec("/bin/sh /usr/local/etc/rc.d/snort.sh start");
	update_status(gettext("The Rules update finished..."));
	update_output_window(gettext("Snort has restarted with your new set of rules..."));
	exec("/bin/rm /tmp/snort_download_halt.pid");
}

update_status(gettext("The Rules update finished..."));
conf_mount_ro();

?>
