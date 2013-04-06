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
require_once "/usr/local/pkg/snort/snort.inc";

global $snort_gui_include, $vrt_enabled, $et_enabled, $rebuild_rules;
global $protect_preproc_rules, $is_postinstall, $snort_community_rules_filename;
global $snort_community_rules_url, $snort_rules_file, $emergingthreats_filename;

$snortdir = SNORTDIR;
$snortlibdir = SNORTLIBDIR;
$snortlogdir = SNORTLOGDIR;

if (!isset($snort_gui_include))
	$pkg_interface = "console";

/* define checks */
$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingthreats = $config['installedpackages']['snortglobal']['emergingthreats'];
$snortcommunityrules = $config['installedpackages']['snortglobal']['snortcommunityrules'];
$vrt_enabled = $config['installedpackages']['snortglobal']['snortdownload'];
$et_enabled = $config['installedpackages']['snortglobal']['emergingthreats'];

/* Directory where we download rule tarballs */
$tmpfname = "{$snortdir}/tmp/snort_rules_up";

/* Snort VRT rules files and URL */
$snort_filename_md5 = "{$snort_rules_file}.md5";
$snort_filename = "{$snort_rules_file}";
$snort_rule_url = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/";

/* Emerging Threats rules MD5 file */
$emergingthreats_filename_md5 = "{$emergingthreats_filename}.md5";

/* Snort GPLv2 Community Rules MD5 file */
$snort_community_rules_filename_md5 = "{$snort_community_rules_filename}.md5";

/* Start of code */
conf_mount_rw();

/*  remove old $tmpfname files */
if (is_dir("{$tmpfname}"))
	exec("/bin/rm -r {$tmpfname}");

/*  Make sure required snortdirs exsist */
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p {$snortdir}/preproc_rules");
exec("/bin/mkdir -p {$tmpfname}");
exec("/bin/mkdir -p {$snortlibdir}/dynamicrules");
exec("/bin/mkdir -p {$snortlogdir}");

/* See if we need to automatically clear the Update Log based on 1024K size limit */
if (file_exists($update_log)) {
	if (1048576 < filesize($update_log))
		exec("/bin/rm -r {$update_log}");
}

/* Log start time for this rules update */
error_log(gettext("Starting rules update...  Time: " . date("Y-m-d H:i:s") . "\n"), 3, $update_log);

/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","150M");

/*  download md5 sig from snort.org */
if ($snortdownload == 'on') {
	update_status(gettext("Downloading Snort VRT md5 file..."));
	error_log(gettext("\tDownloading Snort VRT md5 file...\n"), 3, $update_log);
        $max_tries = 4;
        while ($max_tries > 0) {
	       $image = @file_get_contents("{$snort_rule_url}{$snort_filename_md5}");
               if (false === $image) {
                       $max_tries--;
                       if ($max_tries > 0)
                               sleep(30);
                       continue;
               } else 
                       break;
        }
        log_error("Snort MD5 Attempts: " . (4 - $max_tries + 1));
        error_log("\tChecking Snort VRT md5 file...\n", 3, $update_log);
	@file_put_contents("{$tmpfname}/{$snort_filename_md5}", $image);
	if (0 == filesize("{$tmpfname}/{$snort_filename_md5}")) {
		update_status(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		log_error(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		update_output_window(gettext("Rules are released every month from snort.org. You may download the Rules at any time."));
		$snortdownload = 'off';
		error_log(gettext("\tSnort VRT md5 download failed.  Site may be offline or Oinkcode is not authorized for this level or version.\n"), 3, $update_log);
	} else
		update_status(gettext("Done downloading snort.org md5."));
}

/* Check if were up to date snort.org */
if ($snortdownload == 'on') {
	if (file_exists("{$snortdir}/{$snort_filename_md5}")) {
		$md5_check_new = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
		$md5_check_old = file_get_contents("{$snortdir}/{$snort_filename_md5}");
		if ($md5_check_new == $md5_check_old) {
			update_status(gettext("Snort VRT rules are up to date..."));
			log_error(gettext("Snort VRT rules are up to date..."));
			error_log(gettext("\tSnort VRT rules are up to date.\n"), 3, $update_log);
			$snortdownload = 'off';
		}
	}
}

/* download snortrules file */
if ($snortdownload == 'on') {
	update_status(gettext("There is a new set of Snort VRT rules posted. Downloading..."));
	log_error(gettext("There is a new set of Snort VRT rules posted. Downloading..."));
	error_log(gettext("\tThere is a new set of Snort VRT rules posted. Downloading...\n"), 3, $update_log);
        $max_tries = 4;
        while ($max_tries > 0) {
        	download_file_with_progress_bar("{$snort_rule_url}{$snort_filename}", "{$tmpfname}/{$snort_filename}");
        	if (300000 > filesize("{$tmpfname}/$snort_filename")){
                        $max_tries--;
                        if ($max_tries > 0)
                                sleep(30);
                        continue;
                } else
                        break;
        }  
	update_status(gettext("Done downloading Snort VRT rules file."));
        log_error("Snort Rules Attempts: " . (4 - $max_tries + 1));
	error_log(gettext("\tDone downloading rules file.\n"),3, $update_log);
	if (300000 > filesize("{$tmpfname}/$snort_filename")){
		update_output_window(gettext("Snort VRT rules file download failed..."));
		log_error(gettext("Snort VRT rules file download failed..."));
                log_error("Failed Rules Filesize: " . filesize("{$tmpfname}/$snort_filename"));
		error_log(gettext("\tSnort VRT rules file download failed.  Snort VRT rules will not be updated.\n"), 3, $update_log);
		$snortdownload = 'off';
	}
}

/*  download md5 sig from Snort GPLv2 Community Rules */
if ($snortcommunityrules == 'on') {
	update_status(gettext("Downloading Snort GPLv2 Community Rules md5 file..."));
	error_log(gettext("\tDownloading Snort GPLv2 Community Rules md5 file...\n"), 3, $update_log);
        $image = file_get_contents("{$snort_community_rules_url}{$snort_community_rules_filename_md5}");
	update_status(gettext("Done downloading Snort GPLv2 Community Rules md5"));
	error_log(gettext("\tChecking Snort GPLv2 Community Rules md5.\n"), 3, $update_log);
	@file_put_contents("{$tmpfname}/{$snort_community_rules_filename_md5}", $image);

	/* See if the file download was successful, and turn off Snort GPLv2 update if it failed. */
	if (0 == filesize("{$tmpfname}/{$snort_community_rules_filename_md5}")){
		update_output_window(gettext("Snort GPLv2 Community Rules md5 file download failed.  Community Rules will not be updated."));
		log_error(gettext("Snort GPLv2 Community Rules md5 file download failed.  Community Rules will not be updated."));
		error_log(gettext("\tSnort GPLv2 Community Rules md5 file download failed.  Community Rules will not be updated.\n"), 3, $update_log);
		$snortcommunityrules = 'off';
	}

	if (file_exists("{$snortdir}/{$snort_community_rules_filename_md5}") && $snortcommunityrules == "on") {
		/* Check if were up to date Snort GPLv2 Community Rules */
		$snort_comm_md5_check_new = file_get_contents("{$tmpfname}/{$snort_community_rules_filename_md5}");
		$snort_comm_md5_check_old = file_get_contents("{$snortdir}/{$snort_community_rules_filename_md5}");
		if ($snort_comm_md5_check_new == $snort_comm_md5_check_old) {
			update_status(gettext("Snort GPLv2 Community Rules are up to date..."));
			log_error(gettext("Snort GPLv2 Community Rules are up to date..."));
			error_log(gettext("\tSnort GPLv2 Community Rules are up to date.\n"), 3, $update_log);
			$snortcommunityrules = 'off';
		}
	}
}

/* download Snort GPLv2 Community rules file */
if ($snortcommunityrules == "on") {
	update_status(gettext("There is a new set of Snort GPLv2 Community Rules posted. Downloading..."));
	log_error(gettext("There is a new set of Snort GPLv2 Community Rules posted. Downloading..."));
	error_log(gettext("\tThere is a new set of Snort GPLv2 Community Rules posted. Downloading...\n"), 3, $update_log);
	download_file_with_progress_bar("{$snort_community_rules_url}{$snort_community_rules_filename}", "{$tmpfname}/{$snort_community_rules_filename}");

	/* Test for a valid rules file download.  Turn off Snort Community update if download failed. */
	if (150000 > filesize("{$tmpfname}/{$snort_community_rules_filename}")){
		update_output_window(gettext("Snort GPLv2 Community Rules file download failed..."));
		log_error(gettext("Snort GPLv2 Community Rules file download failed..."));
                log_error("Failed Rules Filesize: " . filesize("{$tmpfname}/{$snort_community_rules_filename}"));
		error_log(gettext("\tSnort GPLv2 Community Rules file download failed.  Community Rules will not be updated.\n"), 3, $update_log);
		$snortcommunityrules = 'off';
	}
	else {
		update_status(gettext('Done downloading Snort GPLv2 Community Rules file.'));
		log_error("Snort GPLv2 Community Rules file update downloaded succsesfully");
		error_log(gettext("\tDone downloading Snort GPLv2 Community Rules file.\n"), 3, $update_log);
	}
}

/* Untar Snort GPLv2 Community rules to tmp */
if ($snortcommunityrules == 'on') {
	safe_mkdir("{$snortdir}/tmp/community");
	if (file_exists("{$tmpfname}/{$snort_community_rules_filename}")) {
		update_status(gettext("Extracting Snort GPLv2 Community Rules..."));
		error_log(gettext("\tExtracting and installing Snort GPLv2 Community Rules...\n"), 3, $update_log);
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_community_rules_filename} -C {$snortdir}/tmp/community/");

		$files = glob("{$snortdir}/tmp/community/community-rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/GPLv2_{$newfile}");
		}
                /* base etc files for Snort GPLv2 Community rules */
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/community/community-rules/{$file}"))
				@copy("{$snortdir}/tmp/community/community-rules/{$file}", "{$snortdir}/tmp/GPLv2_{$file}");
		}

		/*  Copy snort community md5 sig to snort dir */
		if (file_exists("{$tmpfname}/{$snort_community_rules_filename_md5}")) {
			update_status(gettext("Copying md5 signature to snort directory..."));
			@copy("{$tmpfname}/$snort_community_rules_filename_md5", "{$snortdir}/{$snort_community_rules_filename_md5}");
		}
                update_status(gettext("Extraction of Snort GPLv2 Community Rules completed..."));
		error_log(gettext("\tInstallation of Snort GPLv2 Community Rules completed.\n"), 3, $update_log);
		exec("rm -r {$snortdir}/tmp/community");
	}
}

/*  download md5 sig from emergingthreats.net */
if ($emergingthreats == 'on') {
	update_status(gettext("Downloading EmergingThreats md5 file..."));
	error_log(gettext("\tDownloading EmergingThreats md5 file...\n"), 3, $update_log);

	/* If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules.  */
	if ($vrt_enabled == "on")
	        $image = @file_get_contents("http://rules.emergingthreats.net/open-nogpl/snort-{$emerging_threats_version}/emerging.rules.tar.gz.md5");
	else
	        $image = @file_get_contents("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz.md5");

	update_status(gettext("Done downloading EmergingThreats md5"));
	error_log(gettext("\tChecking EmergingThreats md5.\n"), 3, $update_log);
	@file_put_contents("{$tmpfname}/{$emergingthreats_filename_md5}", $image);

	/* See if the file download was successful, and turn off ET update if it failed. */
	if (0 == filesize("{$tmpfname}/$emergingthreats_filename_md5")){
		update_output_window(gettext("EmergingThreats md5 file download failed.  EmergingThreats rules will not be updated."));
		log_error(gettext("EmergingThreats md5 file download failed.  EmergingThreats rules will not be updated."));
		error_log(gettext("\tEmergingThreats md5 file download failed.  EmergingThreats rules will not be updated.\n"), 3, $update_log);
		$emergingthreats = 'off';
	}

	if (file_exists("{$snortdir}/{$emergingthreats_filename_md5}") && $emergingthreats == "on") {
		/* Check if were up to date emergingthreats.net */
		$emerg_md5_check_new = file_get_contents("{$tmpfname}/{$emergingthreats_filename_md5}");
		$emerg_md5_check_old = file_get_contents("{$snortdir}/{$emergingthreats_filename_md5}");
		if ($emerg_md5_check_new == $emerg_md5_check_old) {
			update_status(gettext("Emerging Threats rules are up to date..."));
			log_error(gettext("Emerging Threat rules are up to date..."));
			error_log(gettext("\tEmerging Threats rules are up to date.\n"), 3, $update_log);
			$emergingthreats = 'off';
		}
	}
}

/* download emergingthreats rules file */
if ($emergingthreats == "on") {
	update_status(gettext("There is a new set of EmergingThreats rules posted. Downloading..."));
	log_error(gettext("There is a new set of EmergingThreats rules posted. Downloading..."));
	error_log(gettext("\tThere is a new set of EmergingThreats rules posted. Downloading...\n"), 3, $update_log);

	/* If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules.  */
	if ($vrt_enabled == "on")
		download_file_with_progress_bar("http://rules.emergingthreats.net/open-nogpl/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");
	else
		download_file_with_progress_bar("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");

	/* Test for a valid rules file download.  Turn off ET update if download failed. */
	if (150000 > filesize("{$tmpfname}/$emergingthreats_filename")){
		update_output_window(gettext("EmergingThreats rules file download failed..."));
		log_error(gettext("EmergingThreats rules file download failed..."));
                log_error("Failed Rules Filesize: " . filesize("{$tmpfname}/$emergingthreats_filename"));
		error_log(gettext("\tEmergingThreats rules file download failed.  EmergingThreats rules will not be updated.\n"), 3, $update_log);
		$emergingthreats = 'off';
	}
	else {
		update_status(gettext('Done downloading EmergingThreats rules file.'));
		log_error("EmergingThreats rules file update downloaded succsesfully");
		error_log(gettext("\tDone downloading EmergingThreats rules file.\n"), 3, $update_log);
	}
}

/* Untar emergingthreats rules to tmp */
if ($emergingthreats == 'on') {
	safe_mkdir("{$snortdir}/tmp/emerging");
	if (file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
		update_status(gettext("Extracting EmergingThreats.org rules..."));
		error_log(gettext("\tExtracting and installing EmergingThreats.org rules...\n"), 3, $update_log);
		exec("/usr/bin/tar xzf {$tmpfname}/{$emergingthreats_filename} -C {$snortdir}/tmp/emerging rules/");

		$files = glob("{$snortdir}/tmp/emerging/rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/{$newfile}");
		}
		/* IP lists for Emerging Threats rules */
		$files = glob("{$snortdir}/tmp/emerging/rules/*ips.txt");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/{$newfile}");
		}
                /* base etc files for Emerging Threats rules */
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/emerging/rules/{$file}"))
				@copy("{$snortdir}/tmp/emerging/rules/{$file}", "{$snortdir}/tmp/ET_{$file}");
		}

		/*  Copy emergingthreats md5 sig to snort dir */
		if (file_exists("{$tmpfname}/$emergingthreats_filename_md5")) {
			update_status(gettext("Copying md5 signature to snort directory..."));
			@copy("{$tmpfname}/$emergingthreats_filename_md5", "{$snortdir}/$emergingthreats_filename_md5");
		}
                update_status(gettext("Extraction of EmergingThreats.org rules completed..."));
		error_log(gettext("\tInstallation of EmergingThreats.org rules completed.\n"), 3, $update_log);
		exec("rm -r {$snortdir}/tmp/emerging");
	}
}

/* Untar snort rules file individually to help people with low system specs */
if ($snortdownload == 'on') {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {

		/* Currently, only FreeBSD-8-1 and FreeBSD-9-0 precompiled SO rules exist from Snort.org */
		/* Default to FreeBSD 8.1, and then test for FreeBSD 9.x */
		$freebsd_version_so = 'FreeBSD-8-1';
		if (substr(php_uname("r"), 0, 1) == '9')
			$freebsd_version_so = 'FreeBSD-9-0';

		update_status(gettext("Extracting Snort VRT rules..."));
		error_log(gettext("\tExtracting and installing Snort VRT rules...\n"), 3, $update_log);
		/* extract snort.org rules and add prefix to all snort.org files */
		safe_mkdir("{$snortdir}/tmp/snortrules");
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp/snortrules rules/");
		$files = glob("{$snortdir}/tmp/snortrules/rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/snort_{$newfile}");
		}
		/* IP lists */
		$files = glob("{$snortdir}/tmp/snortrules/rules/*.txt");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/{$newfile}");
		}
		exec("rm -r {$snortdir}/tmp/snortrules");

		/* extract so rules */
		update_status(gettext("Extracting Snort VRT Shared Objects rules..."));
		exec('/bin/mkdir -p {$snortlibdir}/dynamicrules/');
		error_log(gettext("\tUsing Snort VRT precompiled SO rules for {$freebsd_version_so} ...\n"), 3, $update_log);
		$snort_arch = php_uname("m");
		$nosorules = false;
		if ($snort_arch  == 'i386'){
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/precompiled/$freebsd_version_so/i386/{$snort_version}/");
			exec("/bin/cp {$snortdir}/tmp/so_rules/precompiled/$freebsd_version_so/i386/{$snort_version}/* {$snortlibdir}/dynamicrules/");
		} elseif ($snort_arch == 'amd64') {
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/precompiled/$freebsd_version_so/x86-64/{$snort_version}/");
			exec("/bin/cp {$snortdir}/tmp/so_rules/precompiled/$freebsd_version_so/x86-64/{$snort_version}/* {$snortlibdir}/dynamicrules/");
		} else
			$nosorules = true;
		exec("rm -r {$snortdir}/tmp/so_rules");

		if ($nosorules == false) {
			/* extract so stub rules, rename and copy to the rules folder. */
		        update_status(gettext("Copying Snort VRT Shared Objects rules..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp --exclude precompiled/ --exclude src/ so_rules/");
			$files = glob("{$snortdir}/tmp/so_rules/*.rules");
			foreach ($files as $file) {
				$newfile = basename($file, ".rules");
				@copy($file, "{$snortdir}/rules/snort_{$newfile}.so.rules");
			}
			exec("rm -r {$snortdir}/tmp/so_rules");
		}
		
		/* extract base etc files */
	        update_status(gettext("Extracting Snort VRT config and map files..."));
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp etc/");
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/etc/{$file}"))
				@copy("{$snortdir}/tmp/etc/{$file}", "{$snortdir}/tmp/VRT_{$file}");
		}
		exec("rm -r {$snortdir}/tmp/etc");

		/* Untar snort signatures */
		$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
		if ($premium_url_chk == 'on') {
			update_status(gettext("Extracting Snort VRT Signatures..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
			update_status(gettext("Done extracting Signatures."));

			if (is_dir("{$snortdir}/doc/signatures")) {
				update_status(gettext("Copying Snort VRT signatures..."));
				exec("/bin/cp -r {$snortdir}/doc/signatures {$snortdir}/signatures");
				update_status(gettext("Done copying signatures."));
			}
		}

		/* Extract the Snort preprocessor rules */
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp preproc_rules/");

		if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
			update_status(gettext("Copying md5 signature to snort directory..."));
			@copy("{$tmpfname}/$snort_filename_md5", "{$snortdir}/$snort_filename_md5");
		}
		update_status(gettext("Extraction of Snort VRT rules completed..."));
		error_log(gettext("\tInstallation of Snort VRT rules completed.\n"), 3, $update_log);
	}
}

function snort_apply_customizations($snortcfg, $if_real) {

	global $vrt_enabled;
	$snortdir = SNORTDIR;

	/* Update the Preprocessor rules for the master configuration and for the interface if Snort VRT rules are in use. */
	if ($vrt_enabled == 'on') {
		exec("/bin/mkdir -p {$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/preproc_rules");
		exec("/bin/cp {$snortdir}/tmp/preproc_rules/*.rules {$snortdir}/preproc_rules/");

		/* Check if customized preprocessor rule protection is enabled before overwriting them. */
		if ($snortcfg['protect_preproc_rules'] <> 'on')
			exec("/bin/cp {$snortdir}/tmp/preproc_rules/*.rules {$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/preproc_rules/");
	}
	else {
		exec("/bin/mkdir -p {$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/preproc_rules");
	}

	snort_prepare_rule_files($snortcfg, "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}");

	/* Copy the master config and map files to the interface directory */
	@copy("{$snortdir}/tmp/classification.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/classification.config");
	@copy("{$snortdir}/tmp/gen-msg.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/gen-msg.map");
	@copy("{$snortdir}/tmp/reference.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/reference.config");
	@copy("{$snortdir}/tmp/unicode.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/unicode.map");
}

if ($snortdownload == 'on' || $emergingthreats == 'on' || $snortcommunityrules == 'on') {

	update_status(gettext('Copying new config and map files...'));
	error_log(gettext("\tCopying new config and map files...\n"), 3, $update_log);

        /* Determine which config and map file set to use for the master copy. */
        /* If the Snort VRT rules are not enabled, then use Emerging Threats.  */
        if (($vrt_enabled == 'off') && ($et_enabled == 'on')) {
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/ET_{$file}"))
				@rename("{$snortdir}/tmp/ET_{$file}", "{$snortdir}/tmp/{$file}");
		}
        }
        elseif (($vrt_enabled == 'on') && ($et_enabled == 'off')) {
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/VRT_{$file}"))
				@rename("{$snortdir}/tmp/VRT_{$file}", "{$snortdir}/tmp/{$file}");
                }
        }
        elseif (($vrt_enabled == 'on') && ($et_enabled == 'on')) {
               /* Both VRT and ET rules are enabled, so build combined  */
               /* reference.config and classification.config files.     */
                $cfgs = glob("{$snortdir}/tmp/*reference.config");
                snort_merge_reference_configs($cfgs, "{$snortdir}/tmp/reference.config");
                $cfgs = glob("{$snortdir}/tmp/*classification.config");
                snort_merge_classification_configs($cfgs, "{$snortdir}/tmp/classification.config");

		/* Use the unicode.map and gen-msg.map files from VRT rules. */
		if (file_exists("{$snortdir}/tmp/VRT_unicode.map"))
			@rename("{$snortdir}/tmp/VRT_unicode.map", "{$snortdir}/tmp/gen-msg.map");
		if (file_exists("{$snortdir}/tmp/VRT_gen-msg.map"))
			@rename("{$snortdir}/tmp/VRT_gen-msg.map", "{$snortdir}/tmp/gen-msg.map");
        }
	else {
		/* Just Snort GPLv2 Community Rules may be enabled, so make sure required */
		/* default config files are present in the rules extraction tmp working   */
		/* directory. Only copy missing files not captured in logic above.        */

		$snort_files = array("gen-msg.map", "classification.config", "reference.config", "unicode.map");
		foreach ($snort_files as $file) {
			if (file_exists("{$snortdir}/{$file}") && !file_exists("{$snortdir}/tmp/{$file}"))
				@copy("{$snortdir}/{$file}", "{$snortdir}/tmp/{$file}");
		}
	}

	/* Start the rules rebuild proccess for each configured interface */
	if (is_array($config['installedpackages']['snortglobal']['rule'])) {

		/* Set the flag to force rule rebuilds since we downloaded new rules,    */
		/* except when in post-install mode.  Post-install does its own rebuild. */
		if ($is_postinstall)
			$rebuild_rules = 'off';
		else
			$rebuild_rules = 'on';

		/* Create configuration for each active Snort interface */
		foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {
			$if_real = snort_get_real_interface($value['interface']);
			$tmp = "Updating rules configuration for: " . snort_get_friendly_interface($value['interface']) . " ...";
			update_status(gettext($tmp));
			snort_apply_customizations($value, $if_real);

			/*  Log a message in Update Log if protecting customized preprocessor rules. */
			$tmp = "\t" . $tmp . "\n";
			if ($value['protect_preproc_rules'] == 'on') {
				$tmp .= gettext("\tPreprocessor text rules flagged as protected and not updated for ");
				$tmp .= snort_get_friendly_interface($value['interface']) . "...\n";
			}
			error_log($tmp, 3, $update_log);
		}
	}
	else {
	        update_output_window(gettext("\nWarning:  No interfaces configured for Snort were found..."));
	        update_output_window(gettext("          When Snort is added to an interface, the rules will rebuild...\n"));
		error_log(gettext("\tWarning:  No interfaces configured for Snort were found...\n"), 3, $update_log);
	}

	/* Clear the rebuild rules flag.  */
	$rebuild_rules = 'off';

	/*  remove old $tmpfname files */
	if (is_dir("{$snortdir}/tmp")) {
		update_status(gettext("Cleaning up after rules extraction..."));
		exec("/bin/rm -r {$snortdir}/tmp");
	}

	/* Restart snort if already running to pick up the new rules. */
       	if (is_process_running("snort")) {
		update_status(gettext('Restarting Snort to activate the new set of rules...'));
		error_log(gettext("\tRestarting Snort to activate the new set of rules...\n"), 3, $update_log);
       		exec("/bin/sh /usr/local/etc/rc.d/snort.sh restart");
	        update_output_window(gettext("Snort has restarted with your new set of rules..."));
       		log_error(gettext("Snort has restarted with your new set of rules..."));
		error_log(gettext("\tSnort has restarted with your new set of rules.\n"), 3, $update_log);
	}
}

update_status(gettext("The Rules update has finished..."));
log_error(gettext("The Rules update has finished."));
error_log(gettext("The Rules update has finished.  Time: " . date("Y-m-d H:i:s"). "\n\n"), 3, $update_log);
conf_mount_ro();

?>
