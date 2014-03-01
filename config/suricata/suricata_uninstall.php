<?php
/*
	suricata_uninstall.php

	Copyright (C) 2014 Bill Meeks
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

require_once("/usr/local/pkg/suricata/suricata.inc");

global $config, $g;

$suricatadir = SURICATADIR;
$suricatalogdir = SURICATALOGDIR;
$rcdir = RCFILEPREFIX;
$suricata_rules_upd_log = RULES_UPD_LOGFILE;

log_error(gettext("[Suricata] Suricata package uninstall in progress..."));

/* Make sure all active Suricata processes are terminated */
/* Log a message only if a running process is detected */
if (is_service_running("suricata"))
	log_error(gettext("[Suricata] Suricata STOP for all interfaces..."));
killbyname("suricata");
sleep(1);

// Delete any leftover suricata PID files in /var/run
array_map('@unlink', glob("/var/run/suricata_*.pid"));

/* Make sure all active Barnyard2 processes are terminated */
/* Log a message only if a running process is detected     */
if (is_service_running("barnyard2"))
	log_error(gettext("[Suricata] Barnyard2 STOP for all interfaces..."));
killbyname("barnyard2");
sleep(1);

// Delete any leftover barnyard2 PID files in /var/run
array_map('@unlink', glob("/var/run/barnyard2_*.pid"));

/* Remove the suricata user and group */
mwexec('/usr/sbin/pw userdel suricata; /usr/sbin/pw groupdel suricata', true);

/* Remove the Suricata cron jobs. */
install_cron_job("/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/www/suricata/suricata_check_for_rule_updates.php", false);
install_cron_job("/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/suricata/suricata_check_cron_misc.inc", false);

/* See if we are to keep Suricata log files on uninstall */
if ($config['installedpackages']['suricata']['config'][0]['clearlogs'] == 'on') {
	log_error(gettext("[Suricata] Clearing all Suricata-related log files..."));
	@unlink("{$suricata_rules_upd_log}");
	mwexec("/bin/rm -rf {$suricatalogdir}");
}

/* Remove the Suricata GUI app directories */
mwexec("/bin/rm -rf /usr/local/pkg/suricata");
mwexec("/bin/rm -rf /usr/local/www/suricata");

/* Remove our associated Dashboard widget config and files. */
/* If "save settings" is enabled, then save old widget      */
/* container settings so we can restore them later.         */
$widgets = $config['widgets']['sequence'];
if (!empty($widgets)) {
	$widgetlist = explode(",", $widgets);
	foreach ($widgetlist as $key => $widget) {
		if (strstr($widget, "suricata_alerts-container")) {
			if ($config['installedpackages']['suricata']['config'][0]['forcekeepsettings'] == 'on') {
				$config['installedpackages']['suricata']['config'][0]['dashboard_widget'] = $widget;
				if ($config['widgets']['widget_suricata_display_lines']) {
					$config['installedpackages']['suricata']['config'][0]['dashboard_widget_rows'] = $config['widgets']['widget_suricata_display_lines'];
					unset($config['widgets']['widget_suricata_display_lines']);
				}
			}
			unset($widgetlist[$key]);
		}
	}
	$config['widgets']['sequence'] = implode(",", $widgetlist);
	write_config();
}
@unlink("/usr/local/www/widgets/include/widget-suricata.inc");
@unlink("/usr/local/www/widgets/widgets/suricata_alerts.widget.php");
@unlink("/usr/local/www/widgets/javascript/suricata_alerts.js");

/* Keep this as a last step */
if ($config['installedpackages']['suricata']['config'][0]['forcekeepsettings'] != 'on') {
	log_error(gettext("Not saving settings... all Suricata configuration info and logs deleted..."));
	unset($config['installedpackages']['suricata']);
	unset($config['installedpackages']['suricatasync']);
	@unlink("{$suricata_rules_upd_log}");
	mwexec("/bin/rm -rf {$suricatalogdir}");
	log_error(gettext("[Suricata] The package has been removed from this system..."));
}

?>
