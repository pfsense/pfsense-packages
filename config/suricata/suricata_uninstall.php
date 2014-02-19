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

mwexec('/usr/bin/killall -z suricata', true);
sleep(2);
mwexec('/usr/bin/killall -9 suricata', true);
sleep(2);

// Delete any leftover suricata PID files in /var/run
array_map('@unlink', glob("/var/run/suricata_*.pid"));

/* Make sure all active Barnyard2 processes are terminated */
/* Log a message only if a running process is detected     */
if (is_service_running("barnyard2"))
	log_error(gettext("[Suricata] Barnyard2 STOP for all interfaces..."));

mwexec('/usr/bin/killall -z barnyard2', true);
sleep(2);
mwexec('/usr/bin/killall -9 barnyard2', true);
sleep(2);

// Delete any leftover barnyard2 PID files in /var/run
array_map('@unlink', glob("/var/run/barnyard2_*.pid"));

/* Remove the suricata user and group */
mwexec('/usr/sbin/pw userdel suricata; /usr/sbin/pw groupdel suricata', true);

/* Remove suricata cron entries Ugly code needs smoothness */
if (!function_exists('suricata_deinstall_cron')) {
	function suricata_deinstall_cron($crontask) {
		global $config, $g;

		if(!is_array($config['cron']['item']))
			return;

		$x=0;
		$is_installed = false;
		foreach($config['cron']['item'] as $item) {
			if (strstr($item['command'], $crontask)) {
				$is_installed = true;
				break;
			}
			$x++;
		}
		if ($is_installed == true)
			unset($config['cron']['item'][$x]);
	}
}

/* Remove all the Suricata cron jobs. */
suricata_deinstall_cron("suricata_check_for_rule_updates.php");
suricata_deinstall_cron("suricata_check_cron_misc.inc");
configure_cron();

/**********************************************************/
/* Test for existence of library backup tarballs in /tmp. */
/* If these are present, then a package "delete"          */
/* operation is in progress and we need to wipe out the   */
/* configuration files.  Otherwise we leave the binary-   */
/* side configuration intact since only a GUI files       */
/* deinstall and reinstall operation is in progress.      */
/*							  */
/* XXX: hopefully a better method presents itself in      */
/*      future versions of pfSense.                       */
/**********************************************************/
if (file_exists("/tmp/pkg_libs.tgz") || file_exists("/tmp/pkg_bins.tgz")) {
	log_error(gettext("[Suricata] Package deletion requested... removing all package files..."));
	mwexec("/bin/rm -f {$rcdir}/suricata.sh");
	mwexec("/bin/rm -rf /usr/local/etc/suricata");
	mwexec("/bin/rm -rf /usr/local/pkg/suricata");
	mwexec("/bin/rm -rf /usr/local/www/suricata");
}

if ($config['installedpackages']['suricata']['config'][0]['clearlogs'] == 'on') {
	log_error(gettext("[Suricata] Clearing all Suricata-related log files..."));
	@unlink("{$suricata_rules_upd_log}");
	mwexec("/bin/rm -rf {$suricatalogdir}");
}

/* Keep this as a last step */
if ($config['installedpackages']['suricata']['config'][0]['forcekeepsettings'] != 'on') {
	log_error(gettext("Not saving settings... all Suricata configuration info and logs deleted..."));
	unset($config['installedpackages']['suricata']);
	unset($config['installedpackages']['suricatasync']);
	@unlink("{$suricata_rules_upd_log}");
	mwexec("/bin/rm -rf {$suricatalogdir}");
	@unlink(SURICATALOGDIR);
	log_error(gettext("[Suricata] The package has been removed from this system..."));
}

?>
