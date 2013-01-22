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
$vrt_enabled = $config['installedpackages']['snortglobal']['snortdownload'];
$et_enabled = $config['installedpackages']['snortglobal']['emergingthreats'];

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

/*  Make sure snortdir exits */
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p {$tmpfname}");
exec("/bin/mkdir -p /usr/local/lib/snort/dynamicrules");

/*  download md5 sig from snort.org */
if ($snortdownload == 'on') {
	update_status(gettext("Downloading snort.org md5 file..."));
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
	update_status(gettext("Done downloading rules file."));
        log_error("Snort Rules Attempts: " . (4 - $max_tries + 1));
	if (300000 > filesize("{$tmpfname}/$snort_filename")){
		update_output_window(gettext("Snort rules file download failed..."));
		log_error(gettext("Snort rules file download failed..."));
                log_error("Failed Rules Filesize: " . filesize("{$tmpfname}/$snort_filename"));
		$snortdownload = 'off';
	}
}

/*  download md5 sig from emergingthreats.net */
if ($emergingthreats == 'on') {
	update_status(gettext("Downloading emergingthreats md5 file..."));

	/* If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules.  */
	if ($vrt_enabled == "on")
	        $image = @file_get_contents("http://rules.emergingthreats.net/open-nogpl/snort-{$emerging_threats_version}/emerging.rules.tar.gz.md5");
	else
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

	/* If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules.  */
	if ($vrt_enabled == "on")
		download_file_with_progress_bar("http://rules.emergingthreats.net/open-nogpl/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");
	else
		download_file_with_progress_bar("http://rules.emergingthreats.net/open/snort-{$emerging_threats_version}/emerging.rules.tar.gz", "{$tmpfname}/{$emergingthreats_filename}");

	update_status(gettext('Done downloading Emergingthreats rules file.'));
	log_error("Emergingthreats rules file update downloaded succsesfully");
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

//		/* make sure default rules are in the right format */
//		exec("/usr/bin/sed -I '' -f {$snortdir}/tmp/sedcmd {$snortdir}/rules/emerging*.rules");

		/*  Copy emergingthreats md5 sig to snort dir */
		if (file_exists("{$tmpfname}/$emergingthreats_filename_md5")) {
			update_status(gettext("Copying md5 sig to snort directory..."));
			@copy("{$tmpfname}/$emergingthreats_filename_md5", "{$snortdir}/$emergingthreats_filename_md5");
		}
                update_status(gettext("Extraction of EmergingThreats.org rules completed..."));
	}
}

/* Untar snort rules file individually to help people with low system specs */
if ($snortdownload == 'on') {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		if ($pfsense_stable == 'yes')
			$freebsd_version_so = 'FreeBSD-7-2';
		else
			$freebsd_version_so = 'FreeBSD-8-1';

		update_status(gettext("Extracting Snort VRT rules..."));
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
			/* extract so rules none bin and rename */
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

//			/* make sure default rules are in the right format */
//			exec("/usr/bin/sed -I '' -f {$snortdir}/tmp/sedcmd {$snortdir}/rules/snort_*.rules");

			if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
				update_status(gettext("Copying md5 sig to snort directory..."));
				@copy("{$tmpfname}/$snort_filename_md5", "{$snortdir}/$snort_filename_md5");
			}
		}
                update_status(gettext("Extraction of Snort VRT rules completed..."));
	}
}

/*  remove old $tmpfname files */
if (is_dir("{$snortdir}/tmp")) {
	update_status(gettext("Cleaning up after rules extraction..."));
	exec("/bin/rm -r {$snortdir}/tmp");
}

function snort_apply_customizations($snortcfg, $if_real) {
	global $snortdir, $snort_enforcing_rules_file, $flowbit_rules_file;

	if (!empty($snortcfg['rulesets']) || $snortcfg['ips_policy_enable'] == 'on') {
		$enabled_rules = array();
		$enabled_files = array();

		/* Remove any existing rules files (except custom rules) prior to building a new set. */
		foreach (glob("{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/rules/*.rules") as $file) {
			if (basename($file, ".rules") != "custom")
				@unlink($file);
		}

		/* Create an array with the full path filenames of the enabled  */
		/* rule category files if we have any.                          */
		if (!empty($snortcfg['rulesets'])) {
			foreach (explode("||", $snortcfg['rulesets']) as $file)
				$enabled_files[] = "{$snortdir}/rules/" . $file;

			/* Load our rules map in preparation for writing the enforcing rules file. */
			$enabled_rules = snort_load_rules_map($enabled_files);
		}

		/* Check if a pre-defined Snort VRT policy is selected. If so, */
		/* add all the VRT policy rules to our enforcing rules set.    */
		if (!empty($snortcfg['ips_policy'])) {
			$policy_rules = snort_load_vrt_policy($snortcfg['ips_policy']);
			foreach (array_keys($policy_rules) as $k1) {
				foreach (array_keys($policy_rules[$k1]) as $k2) {
					$enabled_rules[$k1][$k2]['rule'] = $policy_rules[$k1][$k2]['rule'];
					$enabled_rules[$k1][$k2]['category'] = $policy_rules[$k1][$k2]['category'];
					$enabled_rules[$k1][$k2]['disabled'] = $policy_rules[$k1][$k2]['disabled'];
					$enabled_rules[$k1][$k2]['flowbits'] = $policy_rules[$k1][$k2]['flowbits'];
				}
			}
			unset($policy_rules);
		}

		/* Process any enablesid or disablesid modifications for the selected rules. */
		snort_modify_sids($enabled_rules, $snortcfg);

		/* Write the enforcing rules file to the Snort interface's "rules" directory. */
		snort_write_enforcing_rules_file($enabled_rules, "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/rules/{$snort_enforcing_rules_file}");

		/* If auto-flowbit resolution is enabled, generate the dependent flowbits rules file. */
		if ($snortcfg['autoflowbitrules'] == "on") {
			update_status(gettext('Resolving and auto-enabling flowbit required rules for ' . snort_get_friendly_interface($snortcfg['interface']) . '...'));
			log_error('Resolving and auto-enabling flowbit required rules for ' . snort_get_friendly_interface($snortcfg['interface']) . '...');
			$enabled_files[] = "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/rules/{$snort_enforcing_rules_file}";
			snort_write_flowbit_rules_file(snort_resolve_flowbits($enabled_files), "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/rules/{$flowbit_rules_file}");
		}

		/* Build a new sid-msg.map file from the enabled rules. */
        	snort_build_sid_msg_map("{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/rules/", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/sid-msg.map");

		/* Copy the master *.config and other *.map files to the interface's directory */
		@copy("{$snortdir}/classification.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/classification.config");
		@copy("{$snortdir}/gen-msg.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/gen-msg.map");
		@copy("{$snortdir}/reference.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/reference.config");
		@copy("{$snortdir}/unicode.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/unicode.map");
	}
}

if ($snortdownload == 'on' || $emergingthreats == 'on') {

	update_status(gettext('Copying new config and map files...'));

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
                if (file_exists($file)) {
                        $cmd = "/bin/rm -r " . $file;
 	                exec($cmd);
                }
        }
        $cfgs = glob("{$snortdir}/??*_*.map");
        foreach ($cfgs as $file) {
                if (file_exists($file)) {
                        $cmd = "/bin/rm -r " . $file;
 	                exec($cmd);
                }
        }

	/* Start the proccess for each configured interface */
	if (is_array($config['installedpackages']['snortglobal']['rule'])) {
		foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {

			/* Create configuration for each active Snort interface */
			$if_real = snort_get_real_interface($value['interface']);
			$tmp = "Updating rules configuration for: " . snort_get_friendly_interface($value['interface']) . " ...";
			update_status(gettext($tmp));
			log_error($tmp);
			snort_apply_customizations($value, $if_real);
		}
	}
	update_status(gettext('Restarting Snort to activate the new set of rules...'));
        exec("/bin/sh /usr/local/etc/rc.d/snort.sh restart");
        sleep(10);
        if (!is_process_running("snort"))
               exec("/bin/sh /usr/local/etc/rc.d/snort.sh start");
        update_output_window(gettext("Snort has restarted with your new set of rules..."));
        log_error("Snort has restarted with your new set of rules...");
}

update_status(gettext("The Rules update has finished..."));
log_error("The Rules update has finished...");
conf_mount_ro();

?>
