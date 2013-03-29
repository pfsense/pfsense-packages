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
$vrt_enabled = $config['installedpackages']['snortglobal']['snortdownload'];
$et_enabled = $config['installedpackages']['snortglobal']['emergingthreats'];

/* Start of code */
conf_mount_rw();

/* See if we need to automatically clear the Update Log based on 512K size limit */
if (file_exists($update_log)) {
	if (524288 < filesize($update_log))
		exec("/bin/rm -r {$update_log}");
}

/*  remove old $tmpfname files */
if (is_dir("{$tmpfname}"))
	exec("/bin/rm -rf {$tmpfname}");

/* Log start time for this rules update */
error_log(gettext("Starting rules update...  Time: " . date("Y-m-d H:i:s") . "\n"), 3, $update_log);

/* Set user agent to Mozilla */
ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
ini_set("memory_limit","150M");

/*  Make sure required snortdirs exsist */
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p {$tmpfname}");
exec("/bin/mkdir -p /usr/local/lib/snort/dynamicrules");

/*  download md5 sig from snort.org */
if ($snortdownload == 'on') {
	update_status(gettext("Downloading snort.org md5 file..."));
	error_log(gettext("\tDownloading snort.org md5 file...\n"), 3, $update_log);
        $max_tries = 4;
        while ($max_tries > 0) {
	       $image = @file_get_contents("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename_md5}");
               if (false === $image) {
                       $max_tries--;
                       if ($max_tries > 0)
                               sleep(30);
                       continue;
               } else 
                       break;
        }
        log_error("Snort MD5 Attempts: " . (4 - $max_tries + 1));
        error_log("\tChecking Snort md5 file...\n", 3, $update_log);
	@file_put_contents("{$tmpfname}/{$snort_filename_md5}", $image);
	if (0 == filesize("{$tmpfname}/{$snort_filename_md5}")) {
		update_status(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		log_error(gettext("Please wait... You may only check for New Rules every 15 minutes..."));
		update_output_window(gettext("Rules are released every month from snort.org. You may download the Rules at any time."));
		$snortdownload = 'off';
		error_log(gettext("\tSnort MD5 download failed.  Site may be offline or Oinkcode is not authorized for this version.\n"), 3, $update_log);
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
	update_status(gettext("There is a new set of Snort.org VRT rules posted. Downloading..."));
	log_error(gettext("There is a new set of Snort.org VRT rules posted. Downloading..."));
	error_log(gettext("\tThere is a new set of Snort.org VRT rules posted. Downloading...\n"), 3, $update_log);
        $max_tries = 4;
        while ($max_tries > 0) {
        	download_file_with_progress_bar("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", "{$tmpfname}/{$snort_filename}");
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

/*  download md5 sig from emergingthreats.net */
if ($emergingthreats == 'on') {
	update_status(gettext("Downloading emergingthreats md5 file..."));
	error_log(gettext("\tDownloading Emergingthreats md5 file...\n"), 3, $update_log);

	/* If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules.  */
	if ($vrt_enabled == "on")
	        $image = @file_get_contents("http://rules.emergingthreats.net/open-nogpl/snort-{$emerging_threats_version}/emerging.rules.tar.gz.md5");
	else
	        $image = @file_get_contents("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz.md5");

	update_status(gettext("Done downloading Emergingthreats md5"));
	error_log(gettext("\tChecking Emergingthreats md5.\n"), 3, $update_log);
	@file_put_contents("{$tmpfname}/{$emergingthreats_filename_md5}", $image);

	/* See if the file download was successful, and turn off ET update if it failed. */
	if (0 == filesize("{$tmpfname}/$emergingthreats_filename_md5")){
		update_output_window(gettext("Emergingthreats md5 file download failed.  Emergingthreats rules will not be updated."));
		log_error(gettext("Emergingthreats md5 file download failed.  Emergingthreats rules will not be updated."));
		error_log(gettext("\tEmergingthreats md5 file download failed.  Emergingthreats rules will not be updated.\n"), 3, $update_log);
		$emergingthreats = 'off';
	}

	if (file_exists("{$snortdir}/{$emergingthreats_filename_md5}") && $emergingthreats == "on") {
		/* Check if were up to date emergingthreats.net */
		$emerg_md5_check_new = file_get_contents("{$tmpfname}/{$emergingthreats_filename_md5}");
		$emerg_md5_check_old = file_get_contents("{$snortdir}/{$emergingthreats_filename_md5}");
		if ($emerg_md5_check_new == $emerg_md5_check_old) {
			update_status(gettext("Emerging threat rules are up to date..."));
			log_error(gettext("Emerging threat rules are up to date..."));
			error_log(gettext("\tEmerging threat rules are up to date.\n"), 3, $update_log);
			$emergingthreats = 'off';
		}
	}
}

/* download emergingthreats rules file */
if ($emergingthreats == "on") {
	update_status(gettext("There is a new set of Emergingthreats rules posted. Downloading..."));
	log_error(gettext("There is a new set of Emergingthreats rules posted. Downloading..."));
	error_log(gettext("\tThere is a new set of Emergingthreats rules posted. Downloading...\n"), 3, $update_log);

	/* If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules.  */
	if ($vrt_enabled == "on")
		download_file_with_progress_bar("http://rules.emergingthreats.net/open-nogpl/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");
	else
		download_file_with_progress_bar("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");

	/* Test for a valid rules file download.  Turn off ET update if download failed. */
	if (150000 > filesize("{$tmpfname}/$emergingthreats_filename")){
		update_output_window(gettext("Emergingthreats rules file download failed..."));
		log_error(gettext("Emergingthreats rules file download failed..."));
                log_error("Failed Rules Filesize: " . filesize("{$tmpfname}/$emergingthreats_filename"));
		error_log(gettext("\tEmergingthreats rules file download failed.  EmergingThreats rules will not be updated.\n"), 3, $update_log);
		$emergingthreats = 'off';
	}
	else {
		update_status(gettext('Done downloading Emergingthreats rules file.'));
		log_error("Emergingthreats rules file update downloaded succsesfully");
		error_log(gettext("\tDone downloading Emergingthreats rules file.\n"), 3, $update_log);
	}
}

/* Normalize rulesets */
$sedcmd = "s/^#alert/# alert/g\n";
$sedcmd .= "s/^##alert/# alert/g\n";
$sedcmd .= "s/^#[ \\t#]*alert/# alert/g\n";
$sedcmd .= "s/^##\\talert/# alert/g\n";
$sedcmd .= "s/^\\talert/alert/g\n";
$sedcmd .= "s/^[ \\t]*alert/alert/g\n";
@file_put_contents("{$snortdir}/tmp/sedcmd", $sedcmd);

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
		$files = glob("{$snortdir}/tmp/emerging/rules/*.txt");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/{$newfile}");
		}
                /* base etc files for Emerging Threats rules */
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/emerging/rules/{$file}"))
				@copy("{$snortdir}/tmp/emerging/rules/{$file}", "{$snortdir}/ET_{$file}");
		}

		/*  Copy emergingthreats md5 sig to snort dir */
		if (file_exists("{$tmpfname}/$emergingthreats_filename_md5")) {
			update_status(gettext("Copying md5 sig to snort directory..."));
			@copy("{$tmpfname}/$emergingthreats_filename_md5", "{$snortdir}/$emergingthreats_filename_md5");
		}
                update_status(gettext("Extraction of EmergingThreats.org rules completed..."));
		error_log(gettext("\tInstallation of EmergingThreats.org rules completed.\n"), 3, $update_log);
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
		/* extract snort.org rules and  add prefix to all snort.org files*/
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
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
		error_log(gettext("\tUsing Snort VRT precompiled rules for {$freebsd_version_so} ...\n"), 3, $update_log);
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
		exec("rm -r {$snortdir}/tmp/so_rules");

		if ($nosorules == false) {
			/* extract so rules and rename */
		        update_status(gettext("Copying Snort VRT Shared Objects rules..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/");
			$files = glob("{$snortdir}/tmp/so_rules/*.rules");
			foreach ($files as $file) {
				$newfile = basename($file, ".rules");
				@copy($file, "{$snortdir}/rules/snort_{$newfile}.so.rules");
			}
			exec("rm -r {$snortdir}/tmp/so_rules");

			/* extract base etc files */
		        update_status(gettext("Extracting Snort VRT base config files..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp etc/");
			foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
				if (file_exists("{$snortdir}/tmp/etc/{$file}"))
					@copy("{$snortdir}/tmp/etc/{$file}", "{$snortdir}/VRT_{$file}");
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

			foreach (glob("/usr/local/lib/snort/dynamicrules/*example*") as $file)
				@unlink($file);

			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} preproc_rules/");

			if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
				update_status(gettext("Copying md5 sig to snort directory..."));
				@copy("{$tmpfname}/$snort_filename_md5", "{$snortdir}/$snort_filename_md5");
			}
		}
                update_status(gettext("Extraction of Snort VRT rules completed..."));
		error_log(gettext("\tInstallation of Snort VRT rules completed.\n"), 3, $update_log);
	}
}

/*  remove old $tmpfname files */
if (is_dir("{$snortdir}/tmp")) {
	update_status(gettext("Cleaning up after rules extraction..."));
	exec("/bin/rm -r {$snortdir}/tmp");
}

function snort_apply_customizations($snortcfg, $if_real) {

	$snortdir = SNORTDIR;
	snort_prepare_rule_files($snortcfg, "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}");

	/* Copy the master *.config and other *.map files to the interface's directory */
	@copy("{$snortdir}/classification.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/classification.config");
	@copy("{$snortdir}/gen-msg.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/gen-msg.map");
	@copy("{$snortdir}/reference.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/reference.config");
	@copy("{$snortdir}/unicode.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/unicode.map");
}

if ($snortdownload == 'on' || $emergingthreats == 'on') {

	update_status(gettext('Copying new config and map files...'));
	error_log(gettext("\tCopying new config and map files...\n"), 3, $update_log);

        /* Determine which base etc file set to use for the master copy.      */
        /* If the Snort VRT rules are not enabled, then use Emerging Threats. */
        if (($vrt_enabled == 'off') && ($et_enabled == 'on')) {
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/ET_{$file}"))
				@rename("{$snortdir}/ET_{$file}", "{$snortdir}/{$file}");
		}
        }
        elseif (($vrt_enabled == 'on') && ($et_enabled == 'off')) {
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/VRT_{$file}"))
				@rename("{$snortdir}/VRT_{$file}", "{$snortdir}/{$file}");
                }
        }
        else {
               /* Both VRT and ET rules are enabled, so build combined  */
               /* reference.config and classification.config files.     */
                $cfgs = glob("{$snortdir}/*reference.config");
                snort_merge_reference_configs($cfgs, "{$snortdir}/reference.config");
                $cfgs = glob("{$snortdir}/*classification.config");
                snort_merge_classification_configs($cfgs, "{$snortdir}/classification.config");
        }

        /* Clean-up our temp versions of the config and map files.      */
	update_status(gettext('Cleaning up temp files...'));
        $cfgs = glob("{$snortdir}/??*_*.config");
        foreach ($cfgs as $file) {
                if (file_exists($file))
			@unlink($file);
        }
        $cfgs = glob("{$snortdir}/??*_*.map");
        foreach ($cfgs as $file) {
                if (file_exists($file))
			@unlink($file);
        }

	/* Start the proccess for each configured interface */
	if (is_array($config['installedpackages']['snortglobal']['rule'])) {
		foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {

			/* Create configuration for each active Snort interface */
			$if_real = snort_get_real_interface($value['interface']);
			$tmp = "Updating rules configuration for: " . snort_get_friendly_interface($value['interface']) . " ...";
			update_status(gettext($tmp));
			log_error($tmp);
			error_log(gettext("\t" .$tmp . "\n"), 3, $update_log);
			snort_apply_customizations($value, $if_real);
		}
	}

	/* Restart snort if already running to pick up the new rules. */
        if (is_process_running("snort")) {
		update_status(gettext('Restarting Snort to activate the new set of rules...'));
		error_log(gettext("\tRestarting Snort to activate the new set of rules...\n"), 3, $update_log);
        	exec("/bin/sh /usr/local/etc/rc.d/snort.sh restart");

// These steps are probably no longer necessary with improvements that make Snort more stable after rule updates.
//	        sleep(20);
//        	if (!is_process_running("snort"))
//               	exec("/bin/sh /usr/local/etc/rc.d/snort.sh start");
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
