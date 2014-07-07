<?php
/*
 * snort_post_install.php
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009-2010 Robert Zelaya
 * Copyright (C) 2011-2012 Ermal Luci
 * Copyright (C) 2013 Bill Meeks
 * part of pfSense
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

/****************************************************************************/
/* This module is called once during the Snort package installation to      */
/* perform required post-installation setup.  It should only be executed    */
/* from the Package Manager process via the custom-post-install hook in     */
/* the snort.xml package configuration file.                                */
/****************************************************************************/

require_once("config.inc");
require_once("functions.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $config, $g, $rebuild_rules, $pkg_interface, $snort_gui_include;

$snortdir = SNORTDIR;
$snortlibdir = SNORTLIBDIR;
$rcdir = RCFILEPREFIX;

/* Hard kill any running Snort processes that may have been started by any   */
/* of the pfSense scripts such as check_reload_status() or rc.start_packages */
if(is_process_running("snort")) {
	exec("/usr/bin/killall -z snort");
	sleep(2);
	// Delete any leftover snort PID files in /var/run
	unlink_if_exists("/var/run/snort_*.pid");
}
// Hard kill any running Barnyard2 processes
if(is_process_running("barnyard")) {
	exec("/usr/bin/killall -z barnyard2");
	sleep(2);
	// Delete any leftover barnyard2 PID files in /var/run
	unlink_if_exists("/var/run/barnyard2_*.pid");
}

/* Set flag for post-install in progress */
$g['snort_postinstall'] = true;

/* cleanup default files */
@rename("{$snortdir}/snort.conf-sample", "{$snortdir}/snort.conf");
@rename("{$snortdir}/threshold.conf-sample", "{$snortdir}/threshold.conf");
@rename("{$snortdir}/sid-msg.map-sample", "{$snortdir}/sid-msg.map");
@rename("{$snortdir}/unicode.map-sample", "{$snortdir}/unicode.map");
@rename("{$snortdir}/classification.config-sample", "{$snortdir}/classification.config");
@rename("{$snortdir}/generators-sample", "{$snortdir}/generators");
@rename("{$snortdir}/reference.config-sample", "{$snortdir}/reference.config");
@rename("{$snortdir}/gen-msg.map-sample", "{$snortdir}/gen-msg.map");
@rename("{$snortdir}/attribute_table.dtd-sample", "{$snortdir}/attribute_table.dtd");

/* fix up the preprocessor rules filenames from a PBI package install */
$preproc_rules = array("decoder.rules", "preprocessor.rules", "sensitive-data.rules");
foreach ($preproc_rules as $file) {
	if (file_exists("{$snortdir}/preproc_rules/{$file}-sample"))
		@rename("{$snortdir}/preproc_rules/{$file}-sample", "{$snortdir}/preproc_rules/{$file}");
}

/* Remove any previously installed scripts since we rebuild them */
@unlink("{$snortdir}/sid");
@unlink("{$rcdir}/snort.sh");
@unlink("{$rcdir}/barnyard2");

/* Create required log and db directories in /var */
safe_mkdir(SNORTLOGDIR);
safe_mkdir(IPREP_PATH);

/* If installed, absorb the Snort Dashboard Widget into this package */
/* by removing it as a separately installed package.                 */
$pkgid = get_pkg_id("Dashboard Widget: Snort");
if ($pkgid >= 0) {
	log_error(gettext("[Snort] Removing legacy 'Dashboard Widget: Snort' package because the widget is now part of the Snort package."));
	unset($config['installedpackages']['package'][$pkgid]);
	unlink_if_exists("/usr/local/pkg/widget-snort.xml");
	write_config("Snort pkg: removed legacy Snort Dashboard Widget.");
}

/* Define a default Dashboard Widget Container for Snort */
$snort_widget_container = "snort_alerts-container:col2:close";

/* remake saved settings */
if ($config['installedpackages']['snortglobal']['forcekeepsettings'] == 'on') {
	log_error(gettext("[Snort] Saved settings detected... rebuilding installation with saved settings..."));
	update_status(gettext("Saved settings detected..."));
	/* Do one-time settings migration for new multi-engine configurations */
	update_output_window(gettext("Please wait... migrating settings to new configuration..."));
	include('/usr/local/pkg/snort/snort_migrate_config.php');
	update_output_window(gettext("Please wait... rebuilding installation with saved settings..."));
	log_error(gettext("[Snort] Downloading and updating configured rule types..."));
	update_output_window(gettext("Please wait... downloading and updating configured rule types..."));
	if ($pkg_interface <> "console")
		$snort_gui_include = true;
	include('/usr/local/pkg/snort/snort_check_for_rule_updates.php');
	update_status(gettext("Generating snort.conf configuration file from saved settings..."));
	$rebuild_rules = true;

	/* Create the snort.conf files for each enabled interface */
	$snortconf = $config['installedpackages']['snortglobal']['rule'];
	foreach ($snortconf as $value) {
		$if_real = get_real_interface($value['interface']);

		/* create a snort.conf file for interface */
		snort_generate_conf($value);

		/* create barnyard2.conf file for interface */
		if ($value['barnyard_enable'] == 'on')
			snort_generate_barnyard2_conf($value, $if_real);
	}

	/* create snort bootup file snort.sh */
	snort_create_rc();

	/* Set Log Limit, Block Hosts Time and Rules Update Time */
	snort_snortloglimit_install_cron(true);
	snort_rm_blocked_install_cron($config['installedpackages']['snortglobal']['rm_blocked'] != "never_b" ? true : false);
	snort_rules_up_install_cron($config['installedpackages']['snortglobal']['autorulesupdate7'] != "never_up" ? true : false);

	/* Add the recurring jobs created above to crontab */
	configure_cron();

	/* Restore the last Snort Dashboard Widget setting if none is set */
	if (!empty($config['installedpackages']['snortglobal']['dashboard_widget']) && 
	    stristr($config['widgets']['sequence'], "snort_alerts-container") === FALSE)
		$config['widgets']['sequence'] .= "," . $config['installedpackages']['snortglobal']['dashboard_widget'];

	$rebuild_rules = false;
	update_output_window(gettext("Finished rebuilding Snort configuration files..."));
	log_error(gettext("[Snort] Finished rebuilding installation from saved settings..."));

	/* Only try to start Snort if not in reboot */
	if (!$g['booting']) {
		update_status(gettext("Starting Snort using rebuilt configuration..."));
		update_output_window(gettext("Please wait... while Snort is started..."));
		log_error(gettext("[Snort] Starting Snort using rebuilt configuration..."));
		start_service("snort");
		update_output_window(gettext("Snort has been started using the rebuilt configuration..."));
	}
}

/* If an existing Snort Dashboard Widget container is not found, */
/* then insert our default Widget Dashboard container.           */
if (stristr($config['widgets']['sequence'], "snort_alerts-container") === FALSE)
	$config['widgets']['sequence'] .= ",{$snort_widget_container}";

/* Update Snort package version in configuration */
$config['installedpackages']['snortglobal']['snort_config_ver'] = "3.0.14";
write_config("Snort pkg: post-install configuration saved.");

/* Done with post-install, so clear flag */
unset($g['snort_postinstall']);
log_error(gettext("[Snort] Package post-installation tasks completed..."));
return true;

?>
