<?php
/*
 * suricata_post_install.php
 *
 * Copyright (C) 2014 Bill Meeks
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
/* This module is called once during the Suricata package installation to   */
/* perform required post-installation setup.  It should only be executed    */
/* from the Package Manager process via the custom-post-install hook in     */
/* the snort.xml package configuration file.                                */
/****************************************************************************/

require_once("config.inc");
require_once("functions.inc");
require_once("/usr/local/pkg/suricata/suricata.inc");

global $config, $g, $rebuild_rules, $pkg_interface, $suricata_gui_include;

$suricatadir = SURICATADIR;
$rcdir = RCFILEPREFIX;

// Hard kill any running Suricata process that may have been started by any
// of the pfSense scripts such as check_reload_status() or rc.start_packages
if(is_process_running("suricata")) {
	killbyname("suricata");
	sleep(2);
	// Delete any leftover suricata PID files in /var/run
	unlink_if_exists("/var/run/suricata_*.pid");
}
// Hard kill any running Barnyard2 processes
if(is_process_running("barnyard")) {
	killbyname("barnyard2");
	sleep(2);
	// Delete any leftover barnyard2 PID files in /var/run
	unlink_if_exists("/var/run/barnyard2_*.pid");
}

// Set flag for post-install in progress
$g['suricata_postinstall'] = true;

// Remove any previously installed script since we rebuild it
@unlink("{$rcdir}/suricata.sh");

// Create the top-tier log directory
safe_mkdir(SURICATALOGDIR);

// remake saved settings
if ($config['installedpackages']['suricata']['config'][0]['forcekeepsettings'] == 'on') {
	log_error(gettext("[Suricata] Saved settings detected... rebuilding installation with saved settings..."));
	update_status(gettext("Saved settings detected..."));
	update_output_window(gettext("Please wait... rebuilding installation with saved settings..."));
	log_error(gettext("[Suricata] Downloading and updating configured rule types..."));
	update_output_window(gettext("Please wait... downloading and updating configured rule types..."));
	if ($pkg_interface <> "console")
		$suricata_gui_include = true;
	include('/usr/local/www/suricata/suricata_check_for_rule_updates.php');
	update_status(gettext("Generating suricata.yaml configuration file from saved settings..."));
	$rebuild_rules = true;

	// Create the suricata.yaml files for each enabled interface
	$suriconf = $config['installedpackages']['suricata']['rule'];
	foreach ($suriconf as $value) {
		$if_real = get_real_interface($value['interface']);

		// ## BETA pkg bug fix-up -- be sure default rules enabled ##
		$rules = explode("||", $value['rulesets']);
		foreach (array( "decoder-events.rules", "files.rules", "http-events.rules", "smtp-events.rules", "stream-events.rules", "tls-events.rules" ) as $r){
			if (!in_array($r, $rules))
				$rules[] = $r;
		}
		natcasesort($rules);
		$value['rulesets'] = implode("||", $rules);
		write_config();
		// ## end of BETA pkg bug fix-up ##

		// create a suricata.yaml file for interface
		suricata_generate_yaml($value);

		// create barnyard2.conf file for interface
		if ($value['barnyard_enable'] == 'on')
			suricata_generate_barnyard2_conf($value, $if_real);
	}

	// create Suricata bootup file suricata.sh
	suricata_create_rc();

	// Set Log Limit, Block Hosts Time and Rules Update Time
	suricata_loglimit_install_cron();
//	suricata_rm_blocked_install_cron($config['installedpackages']['suricata']['config'][0]['rm_blocked'] != "never_b" ? true : false);
	suricata_rules_up_install_cron($config['installedpackages']['suricata']['config'][0]['autoruleupdate'] != "never_up" ? true : false);

	// Add the recurring jobs created above to crontab
	configure_cron();

	// Restore the Dashboard Widget if it was previously enabled and saved
	if (!empty($config['installedpackages']['suricata']['config'][0]['dashboard_widget']) && !empty($config['widgets']['sequence']))
		$config['widgets']['sequence'] .= "," . $config['installedpackages']['suricata']['config'][0]['dashboard_widget'];
	if (!empty($config['installedpackages']['suricata']['config'][0]['dashboard_widget_rows']) && !empty($config['widgets']))
		$config['widgets']['widget_suricata_display_lines'] = $config['installedpackages']['suricata']['config'][0]['dashboard_widget_rows'];

	$rebuild_rules = false;
	update_output_window(gettext("Finished rebuilding Suricata configuration files..."));
	log_error(gettext("[Suricata] Finished rebuilding installation from saved settings..."));

	// Only try to start Suricata if not in reboot
	if (!$g['booting']) {
		update_status(gettext("Starting Suricata using rebuilt configuration..."));
		update_output_window(gettext("Please wait... while Suricata is started..."));
		log_error(gettext("[Suricata] Starting Suricata using rebuilt configuration..."));
		start_service("suricata");
		update_output_window(gettext("Suricata has been started using the rebuilt configuration..."));
	}
}

// Update Suricata package version in configuration
$config['installedpackages']['suricata']['config'][0]['suricata_config_ver'] = "v0.3-BETA";
write_config();

// Done with post-install, so clear flag
unset($g['suricata_postinstall']);
log_error(gettext("[Suricata] Package post-installation tasks completed..."));
return true;

?>
