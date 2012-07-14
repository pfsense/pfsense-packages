<?php
/*
 * snort_check_for_rule_updates.php
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009 Robert Zelaya
 * Copyright (C) 2011-2012 Ermal Luci
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

/* Normalize rulesets */
$sedcmd = "s/^#alert/# alert/g\n";
$sedcmd .= "s/^##alert/# alert/g\n";
$sedcmd .= "s/^#  alert/# alert/g\n";
$sedcmd .= "s/^#\\talert/# alert/g\n";
$sedcmd .= "s/^##i\talert/# alert/g\n";
$sedcmd .= "s/^\\talert/alert/g\n";
$sedcmd .= "s/^ alert/alert/g\n";
$sedcmd .= "s/^  alert/alert/g\n";
@file_put_contents("{$snortdir}/tmp/sedcmd", $sedcmd);

/* Untar snort rules file individually to help people with low system specs */
if ($snortdownload == 'on') {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		if ($pfsense_stable == 'yes')
			$freebsd_version_so = 'FreeBSD-7-2';
		else
			$freebsd_version_so = 'FreeBSD-8-1';

		update_status(gettext("Extracting Snort.org rules..."));
		/* extract snort.org rules and  add prefix to all snort.org files*/
		safe_mkdir("{$snortdir}/tmp/snortrules");
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp/snortrules rules/");
		$files = glob("{$snortdir}/tmp/snortrules/rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/snort_{$newfile}");
		}
		@unlink("{$snortdir}/snortrules");

		/* extract so rules */
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
		$snort_arch = php_uname("m");
		$nosorules = false;
		if ($snort_arch  == 'i386'){
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/precompiled/$freebsd_version_so/i386/{$snort_version}/");
			exec("/bin/cp {$snortdir}/tmp/so_rules/precompiled/$freebsd_version_so/i386/{$snort_version}/* /usr/local/lib/snort/dynamicrules/");
		} else if ($snort_arch == 'amd64') {
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/precompiled/$freebsd_version_so/x86-64/{$snort_version}/");
			exec("/bin/cp {$snortdir}/tmp/so_rules/precompiled/$freebsd_version_so/x86-64/{$snort_version}/* /usr/local/lib/snort/dynamicrules/");
		} else
			$nosorules = true;
		@unlink("{$snortdir}/tmp/so_rules");

		if ($nosorules == false) {
			/* extract so rules none bin and rename */
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/");
			$files = glob("{$snortdir}/tmp/so_rules/*.rules");
			foreach ($files as $file) {
				$newfile = basename($file, ".rules");
				@copy($file, "{$snortdir}/rules/snort_{$newfile}.so.rules");
			}
			@unlink("{$snortdir}/tmp/so_rules");

			/* extract base etc files */
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp etc/");
			foreach (array("classification.config", "reference.config", "gen-msg.map", "sid-msg.map", "unicode.map") as $file) {
				if (file_exists("{$snortdir}/tmp/etc/{$file}"))
					@copy("{$snortdir}/tmp/etc/{$file}", "{$snortdir}/{$file}");
			}
			@unlink("{$snortdir}/tmp/etc");

			/* Untar snort signatures */
			$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
			if ($premium_url_chk == 'on') {
				update_status(gettext("Extracting Signatures..."));
				exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
				update_status(gettext("Done extracting Signatures."));

				if (is_dir("{$snortdir}/doc/signatures")) {
					update_status(gettext("Copying signatures..."));
					exec("/bin/cp -r {$snortdir}/doc/signatures {$snortdir}/signatures");
					update_status(gettext("Done copying signatures."));
				}
			}

			foreach (glob("/usr/local/lib/snort/dynamicrules/*example*") as $file)
				@unlink($file);

			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} preproc_rules/");

			/* make shure default rules are in the right format */
			exec("/usr/bin/sed -I '' -f {$snortdir}/tmp/sedcmd {$snortdir}/rules/*.rules");

			if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
				update_status(gettext("Copying md5 sig to snort directory..."));
				@copy("{$tmpfname}/$snort_filename_md5", "{$snortdir}/$snort_filename_md5");
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

	/* make shure default rules are in the right format */
	exec("/usr/bin/sed -I '' -f {$snortdir}/tmp/sedcmd {$snortdir}/rules/emerging*.rules");

	/*  Copy emergingthreats md5 sig to snort dir */
	if (file_exists("{$tmpfname}/$emergingthreats_filename_md5")) {
		update_status(gettext("Copying md5 sig to snort directory..."));
		@copy("{$tmpfname}/$emergingthreats_filename_md5", "{$snortdir}/$emergingthreats_filename_md5");
	}

	if ($snortdownload == 'off') {
		foreach (array("classification.config", "reference.config", "sid-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/rules/{$file}"))
				@copy("{$snortdir}/rules/{$file}", "{$snortdir}/{$file}");
		}
	}
}

/*  remove old $tmpfname files */
if (is_dir($tmpfname)) {
	update_status(gettext("Cleaning up..."));
	exec("/bin/rm -r {$tmpfname}");
}

function snort_apply_customizations($snortcfg, $if_real) {
	global $config, $g, $snortdir;

	if (empty($snortcfg['rulesets']))
		return;
	else {
		update_status(gettext("Your set of configured rules are being copied..."));
		log_error(gettext("Your set of configured rules are being copied..."));
		$files = explode("||", $snortcfg['rulesets']);
		foreach ($files as $file)
			@copy("{$snortdir}/rules/{$file}", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/rules/{$file}");

		@copy("{$snortdir}/classification.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/classification.config");
		@copy("{$snortdir}/gen-msg.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/gen-msg.map");
		if (is_dir("{$snortdir}/generators"))
			exec("/bin/cp -r {$snortdir}/generators {$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}");
		@copy("{$snortdir}/reference.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/reference.config");
		@copy("{$snortdir}/sid", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/sid");
		@copy("{$snortdir}/sid-msg.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/sid-msg.map");
		@copy("{$snortdir}/unicode.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/unicode.map");
	}

	if (!empty($snortcfg['rule_sid_on']) || !empty($snortcfg['rule_sid_off'])) {
		if (!empty($snortcfg['rule_sid_on'])) {
			$enabled_sid_on_array = explode("||", trim($snortcfg['rule_sid_on']));
			$enabled_sids = array_flip($enabled_sid_on_array);
		}

		if (!empty($snortcfg['rule_sid_off'])) {
			$enabled_sid_off_array = explode("||", trim($snortcfg['rule_sid_off']));
			$disabled_sids = array_flip($enabled_sid_off_array);
		}

		$files = glob("{$snortdir}/snort_{$snortcfg}_{$if_real}/rules/*.rules");
		foreach ($files as $file) {
			$splitcontents = file($file);
			$changed = false;
			foreach ( $splitcontents as $counter => $value ) {
				$sid = snort_get_rule_part($value, 'sid:', ';', 0);
				if (!is_numeric($sid))
					continue;
				if (isset($enabled_sids["enablesid {$sid}"])) {
					if (substr($value, 0, 5) == "alert")
						/* Rule is already enabled */
						continue;
					if (substr($value, 0, 7) == "# alert") {
						/* Rule is disabled, change */
						$splitcontents[$counter] = substr($value, 2);
						$changed = true;
					} else if (substr($splitcontents[$counter - 1], 0, 5) == "alert") {
						/* Rule is already enabled */
						continue;
					} else if (substr($splitcontents[$counter - 1], 0, 7) == "# alert") { 
						/* Rule is disabled, change */
						$splitcontents[$counter - 1] = substr($value, 2);
						$changed = true;
					}
				} else if (isset($disabled_sids["disablesid {$sid}"])) {
					if (substr($value, 0, 7) == "# alert")
						/* Rule is already disabled */
						continue;
					if (substr($value, 0, 5) == "alert") {
						/* Rule is enabled, change */
						$splitcontents[$counter] = "# {$value}";
						$changed = true;
					} else if (substr($splitcontents[$counter - 1], 0, 7) == "# alert") {
						/* Rule is already disabled */
						continue;
					} else if (substr($splitcontents[$counter - 1], 0, 5) == "alert") { 
						/* Rule is enabled, change */
						$splitcontents[$counter - 1] = "# {$value}";
						$changed = true;
					}

				}
			}
			if ($changed == true)
				@file_put_contents($file, implode("\n", $splitcontents));
		}
	}
}

if ($snortdownload == 'on' || $emergingthreats == 'on') {
	/* You are Not Up to date, always stop snort when updating rules for low end machines */;

	/* Start the proccess for every interface rule */
	if (is_array($config['installedpackages']['snortglobal']['rule'])) {
		foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {
			$if_real = snort_get_real_interface($value['interface']);

			/* make oinkmaster.conf for each interface rule */
			snort_apply_customizations($value, $if_real);
		}
	}

	if (is_process_running("snort")) {
		exec("/bin/sh /usr/local/etc/rc.d/snort.sh restart");
		update_output_window(gettext("Snort has restarted with your new set of rules..."));
		log_error(gettext("Snort has restarted with your new set of rules..."));
	} else
		log_error(gettext("Snort Rules update finished..."));
}

update_status(gettext("The Rules update finished..."));
conf_mount_ro();

?>
