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

require_once("functions.inc");
require_once("service-utils.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $snort_gui_include;

$snortdir = SNORTDIR;

if (!isset($snort_gui_include))
	$pkg_interface = "console";

$tmpfname = "{$snortdir}/tmp/snort_rules_up";
$snort_filename_md5 = "{$snort_rules_file}.md5";
$snort_filename = "{$snort_rules_file}";
$emergingthreats_filename_md5 = "emerging.rules.tar.gz.md5";
$emergingthreats_filename = "emerging.rules.tar.gz";

/* define checks */
$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingthreats = $config['installedpackages']['snortglobal']['emergingthreats'];

/* Start of code */
conf_mount_rw();

if (!is_dir($tmpfname))
	exec("/bin/mkdir -p {$tmpfname}");

/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","150M");

/*  remove old $tmpfname files */
if (is_dir("{$tmpfname}"))
	exec("/bin/rm -r {$tmpfname}");

/*  Make shure snortdir exits */
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p {$tmpfname}");
exec("/bin/mkdir -p /usr/local/lib/snort/dynamicrules");

/*  download md5 sig from snort.org */
if ($snortdownload == 'on') {
	update_status(gettext("Downloading snort.org md5 file..."));
	$image = @file_get_contents("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename_md5}");
	@file_put_contents("{$tmpfname}/{$snort_filename_md5}", $image);
	if (0 == filesize("{$tmpfname}/{$snort_filename_md5}")) {
		update_status(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		log_error(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		update_output_window(gettext("Rules are released every month from snort.org. You may download the Rules at any time."));
		$snortdownload = 'off';
	} else
		update_status(gettext("Done downloading snort.org md5"));
}

/* Check if were up to date snort.org */
if ($snortdownload == 'on') {
	if (file_exists("{$snortdir}/{$snort_filename_md5}")) {
		$md5_check_new = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
		$md5_check_old = file_get_contents("{$snortdir}/{$snort_filename_md5}");
		if ($md5_check_new == $md5_check_old) {
			update_status(gettext("Snort rules are up to date..."));
			log_error("Snort rules are up to date...");
			$snortdownload = 'off';
		}
	}
}

/* download snortrules file */
if ($snortdownload == 'on') {
	update_status(gettext("There is a new set of Snort.org rules posted. Downloading..."));
	log_error(gettext("There is a new set of Snort.org rules posted. Downloading..."));
	download_file_with_progress_bar("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", "{$tmpfname}/{$snort_filename}");
	update_status(gettext("Done downloading rules file."));
	if (300000 > filesize("{$tmpfname}/$snort_filename")){
		update_output_window(gettext("Snort rules file downloaded failed..."));
		log_error(gettext("Snort rules file downloaded failed..."));
		$snortdownload = 'off';
	}
}

/*  download md5 sig from emergingthreats.net */
if ($emergingthreats == 'on') {
	update_status(gettext("Downloading emergingthreats md5 file..."));
	$image = @file_get_contents("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz.md5");
	/* XXX: error checking */
	@file_put_contents("{$tmpfname}/{$emergingthreats_filename_md5}", $image);
	update_status(gettext("Done downloading emergingthreats md5"));

	if (file_exists("{$snortdir}/{$emergingthreats_filename_md5}")) {
		/* Check if were up to date emergingthreats.net */
		$emerg_md5_check_new = file_get_contents("{$tmpfname}/{$emergingthreats_filename_md5}");
		$emerg_md5_check_old = file_get_contents("{$snortdir}/{$emergingthreats_filename_md5}");
		if ($emerg_md5_check_new == $emerg_md5_check_old) {
			update_status(gettext("Emerging threat rules are up to date..."));
			log_error(gettext("Emerging threat rules are up to date..."));
			$emergingthreats = 'off';
		}
	}
}

/* download emergingthreats rules file */
if ($emergingthreats == "on") {
	update_status(gettext("There is a new set of Emergingthreats rules posted. Downloading..."));
	log_error(gettext("There is a new set of Emergingthreats rules posted. Downloading..."));
	download_file_with_progress_bar("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");
	update_status(gettext('Done downloading Emergingthreats rules file.'));
	log_error("Emergingthreats rules file update downloaded succsesfully");
}

/* XXX: need to be verified */
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

/* Untar snort rules file individually to help people with low system specs */
if ($snortdownload == 'on') {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		if ($pfsense_stable == 'yes')
			$freebsd_version_so = 'FreeBSD-7-2';
		else
			$freebsd_version_so = 'FreeBSD-8-1';

		update_status(gettext("Extracting Snort.org rules..."));
		/* extract snort.org rules and  add prefix to all snort.org files*/
		exec("/bin/rm -r {$snortdir}/rules/*");
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} rules/");
		chdir ("{$snortdir}/rules");
		exec('/usr/local/bin/perl /usr/local/bin/snort_rename.pl s/^/snort_/ *.rules');

		/* extract so rules */
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
		$snort_arch = php_uname("m");
		if ($snort_arch  == 'i386'){
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/precompiled/$freebsd_version_so/i386/{$snort_version}/");
			exec("/bin/mv -f {$snortdir}/so_rules/precompiled/$freebsd_version_so/i386/{$snort_version}/* /usr/local/lib/snort/dynamicrules/");
		} else if ($snort_arch == 'amd64') {
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/precompiled/$freebsd_version_so/x86-64/{$snort_version}/");
			exec("/bin/mv -f {$snortdir}/so_rules/precompiled/$freebsd_version_so/x86-64/{$snort_version}/* /usr/local/lib/snort/dynamicrules/");
		} else
			$snortdownload = 'off';

		if ($snortdownload == 'on') {
			/* extract so rules none bin and rename */
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} so_rules/bad-traffic.rules" .
			" so_rules/chat.rules" .
			" so_rules/dos.rules" .
			" so_rules/exploit.rules" .
			" so_rules/icmp.rules" .
			" so_rules/imap.rules" .
			" so_rules/misc.rules" .
			" so_rules/multimedia.rules" .
			" so_rules/netbios.rules" .
			" so_rules/nntp.rules" .
			" so_rules/p2p.rules" .
			" so_rules/smtp.rules" .
			" so_rules/snmp.rules" .
			" so_rules/specific-threats.rules" .
			" so_rules/web-activex.rules" .
			" so_rules/web-client.rules" .
			" so_rules/web-iis.rules" .
			" so_rules/web-misc.rules");

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
			exec("/bin/mv -f {$snortdir}/so_rules/snmp.rules {$snortdir}/rules/snort_snmp.so.rules");
			exec("/bin/mv -f {$snortdir}/so_rules/specific-threats.rules {$snortdir}/rules/snort_specific-threats.so.rules");
			exec("/bin/mv -f {$snortdir}/so_rules/web-activex.rules {$snortdir}/rules/snort_web-activex.so.rules");
			exec("/bin/mv -f {$snortdir}/so_rules/web-client.rules {$snortdir}/rules/snort_web-client.so.rules");
			exec("/bin/mv -f {$snortdir}/so_rules/web-iis.rules {$snortdir}/rules/snort_web-iis.so.rules");
			exec("/bin/mv -f {$snortdir}/so_rules/web-misc.rules {$snortdir}/rules/snort_web-misc.so.rules");
			exec("/bin/rm -r {$snortdir}/so_rules");

			/* extract base etc files */
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} etc/");
			exec("/bin/mv -f {$snortdir}/etc/* {$snortdir}");
			exec("/bin/rm -r {$snortdir}/etc");

			/* Untar snort signatures */
			$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
			if ($premium_url_chk == 'on') {
				update_status(gettext("Extracting Signatures..."));
				exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
				update_status(gettext("Done extracting Signatures."));

				if (file_exists("{$snortdir}/doc/signatures")) {
					update_status(gettext("Copying signatures..."));
					exec("/bin/mv -f {$snortdir}/doc/signatures {$snortdir}/signatures");
					update_status(gettext("Done copying signatures."));
				} else {
					update_status(gettext("Directory signatures exist..."));
					update_output_window(gettext("Error copying signature..."));
					$snortdownload = 'off';
				}
			}

			if (file_exists("/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so")) {
				exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so");
				exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example\*");
			}

			/* XXX: Convert this to sed? */
			/* make shure default rules are in the right format */
			exec("/usr/local/bin/perl -pi -e 's/#alert/# alert/g' {$snortdir}/rules/*.rules");
			exec("/usr/local/bin/perl -pi -e 's/##alert/# alert/g' {$snortdir}/rules/*.rules");
			exec("/usr/local/bin/perl -pi -e 's/## alert/# alert/g' {$snortdir}/rules/*.rules");

			/* create a msg-map for snort  */
			update_status(gettext("Updating Alert Messages..."));
			exec("/usr/local/bin/perl /usr/local/bin/create-sidmap.pl {$snortdir}/rules > {$snortdir}/sid-msg.map");

			if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
				update_status(gettext("Copying md5 sig to snort directory..."));
				exec("/bin/cp {$tmpfname}/$snort_filename_md5 {$snortdir}/$snort_filename_md5");
			}
		}
	}
}

/* Untar emergingthreats rules to tmp */
if ($emergingthreats == 'on') {
	if (file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
		update_status(gettext("Extracting rules..."));
		exec("/usr/bin/tar xzf {$tmpfname}/{$emergingthreats_filename} -C {$snortdir} rules/");
	}

	/*  Copy emergingthreats md5 sig to snort dir */
	if (file_exists("{$tmpfname}/$emergingthreats_filename_md5")) {
		update_status(gettext("Copying md5 sig to snort directory..."));
		exec("/bin/cp {$tmpfname}/$emergingthreats_filename_md5 {$snortdir}/$emergingthreats_filename_md5");
	}
}

/*  remove old $tmpfname files */
if (is_dir($tmpfname)) {
	update_status(gettext("Cleaning up..."));
	exec("/bin/rm -r {$tmpfname}");
}

//////////////////
/* open oinkmaster_conf for writing" function */
function oinkmaster_conf($if_real, $iface_uuid)
{
	global $config, $g, $snortdir;

	@unlink("{$snortdir}/snort_{$iface_uuid}_{$if_real}/oinkmaster_{$iface_uuid}_{$if_real}.conf");

	$selected_sid_on_section = "";
	$selected_sid_off_sections = "";

	if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on'])) {
		$enabled_sid_on = trim($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on']);
		$enabled_sid_on_array = explode("||", $enabled_sid_on);
		foreach($enabled_sid_on_array as $enabled_item_on)
			$selected_sid_on_sections .= "$enabled_item_on\n";
	}

	if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'])) {
		$enabled_sid_off = trim($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off']);
		$enabled_sid_off_array = explode("||", $enabled_sid_off);
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

url = dir://{$snortdir}/rules

{$selected_sid_on_sections}

{$selected_sid_off_sections}

EOD;

		/* open snort's oinkmaster.conf for writing */
		@file_put_contents("{$snortdir}/snort_{$iface_uuid}_{$if_real}/oinkmaster_{$iface_uuid}_{$if_real}.conf", $snort_sid_text);
	}
}

/*  Run oinkmaster to snort_wan and cp configs */
/*  If oinkmaster is not needed cp rules normally */
/*  TODO add per interface settings here */
function oinkmaster_run($if_real, $iface_uuid)
{
	global $config, $g, $snortdir;

	if (empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_on']) && empty($config['installedpackages']['snortglobal']['rule'][$id]['rule_sid_off'])) { 
		update_status(gettext("Your first set of rules are being copied..."));
		exec("/bin/cp {$snortdir}/rules/* {$snortdir}/snort_{$iface_uuid}_{$if_real}/rules/");
		exec("/bin/cp {$snortdir}/classification.config {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp -r {$snortdir}/generators {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/reference.config {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/sid {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/unicode.map {$snortdir}/snort_{$iface_uuid}_{$if_real}");
	} else {
		@unlink("{$snortdir}/oinkmaster_{$iface_uuid}_{$if_real}.log");
		update_status(gettext("Your enable and disable changes are being applied to your fresh set of rules..."));
		exec("/bin/cp {$snortdir}/rules/* {$snortdir}/snort_{$iface_uuid}_{$if_real}/rules/");
		exec("/bin/cp {$snortdir}/classification.config {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/gen-msg.map {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp -r {$snortdir}/generators {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/reference.config {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/sid {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/sid-msg.map {$snortdir}/snort_{$iface_uuid}_{$if_real}");
		exec("/bin/cp {$snortdir}/unicode.map {$snortdir}/snort_{$iface_uuid}_{$if_real}");

		/*  might have to add a sleep for 3sec for flash drives or old drives */
		exec("/usr/local/bin/perl /usr/local/bin/oinkmaster.pl -C {$snortdir}/snort_{$iface_uuid}_{$if_real}/oinkmaster_{$iface_uuid}_{$if_real}.conf -o {$snortdir}/snort_{$iface_uuid}_{$if_real}/rules > {$snortdir}/oinkmaster_{$iface_uuid}_{$if_real}.log");
	}
}
//////////////

if ($snortdownload == 'on' || $emergingthreats == 'on') {
	/* You are Not Up to date, always stop snort when updating rules for low end machines */;

	/* Start the proccess for every interface rule */
	if (is_array($config['installedpackages']['snortglobal']['rule'])) {
		foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {
			$if_real = snort_get_real_interface($value['interface']);

			/* make oinkmaster.conf for each interface rule */
			oinkmaster_conf($if_real, $value['uuid']);

			/* run oinkmaster for each interface rule */
			oinkmaster_run($if_real, $value['uuid']);
		}
	}

	exec("/bin/sh /usr/local/etc/rc.d/snort.sh restart");
	update_output_window(gettext("Snort has restarted with your new set of rules..."));
	log_error(gettext("Snort has restarted with your new set of rules..."));
}

update_status(gettext("The Rules update finished..."));
conf_mount_ro();

?>
