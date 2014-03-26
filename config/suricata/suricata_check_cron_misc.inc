<?php
/*
 * suricata_check_cron_misc.inc
 * part of pfSense
 *
 * Copyright (C) 2014 Bill Meeks
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

require_once("/usr/local/pkg/suricata/suricata.inc");

global $g, $config;

function suricata_check_dir_size_limit($suricataloglimitsize) {

	/********************************************************
	 * This function checks the total size of the Suricata  *
	 * logging sub-directory structure and prunes the files *
	 * for all Suricata interfaces if the size exceeds the  *
	 * passed limit.                                        *
	 *                                                      *
	 * On Entry: $surictaaloglimitsize = dir size limit     *
	 *                                   in megabytes       *
	 ********************************************************/

	global $config;
	
	// Convert Log Limit Size setting from MB to KB
	$suricataloglimitsizeKB = round($suricataloglimitsize * 1024);
	$suricatalogdirsizeKB = suricata_Getdirsize(SURICATALOGDIR);

	if ($suricatalogdirsizeKB > 0 && $suricatalogdirsizeKB > $suricataloglimitsizeKB) {
		log_error(gettext("[Suricata] Log directory size exceeds configured limit of " . number_format($suricataloglimitsize) . " MB set on Global Settings tab. All Suricata log files will be truncated."));
		conf_mount_rw();

		// Truncate the Rules Update Log file if it exists
		if (file_exists(RULES_UPD_LOGFILE)) {
			log_error(gettext("[Suricata] Truncating the Rules Update Log file..."));
			@file_put_contents(RULES_UPD_LOGFILE, "");
		}

		// Initialize an array of the log files we want to prune
		$logs = array ( "alerts.log", "http.log", "files-json.log", "tls.log", "stats.log" );

		// Clean-up the logs for each configured Suricata instance
		foreach ($config['installedpackages']['suricata']['rule'] as $value) {
			$if_real = get_real_interface($value['interface']);
			$suricata_uuid = $value['uuid'];
			$suricata_log_dir = SURICATALOGDIR . "suricata_{$if_real}{$suricata_uuid}";
			log_error(gettext("[Suricata] Truncating logs for {$value['descr']} ({$if_real})..."));
			suricata_post_delete_logs($suricata_uuid);

			foreach ($logs as $file) {
				// Truncate the log file if it exists
				if (file_exists("{$suricata_log_dir}/{$file}")) {
					try {
						file_put_contents("{$suricata_log_dir}/{$file}", "");
					} catch (Exception $e) {
						log_error("[Suricata] Failed to truncate file '{$suricata_log_dir}/{$file}' -- error was {$e->getMessage()}");
					}
				}
			}

			// Check for any captured stored files and clean them up
			unlink_if_exists("{$suricata_log_dir}/files/*");

			// This is needed if suricata is run as suricata user
			mwexec('/bin/chmod 660 /var/log/suricata/*', true);
		}
		conf_mount_ro();
		log_error(gettext("[Suricata] Automatic clean-up of Suricata logs completed."));
	}
}

function suricata_check_rotate_log($log_file, $log_limit, $retention) {

	/********************************************************
	 * This function checks the passed log file against     *
	 * the passed size limit and rotates the log file if    *
	 * necessary.  It also checks the age of previously     *
	 * rotated logs and removes those older than the        *
	 * rentention  parameter.                               *
	 *                                                      *
	 * On Entry: $log_file  -> full pathname/filename of    *
	 *                         log file to check            *
	 *           $log_limit -> size of file in bytes to     *
	 *                         trigger rotation. Zero       *
	 *                         means no rotation.           *
	 *           $retention -> retention period in hours    *
	 *                         for rotated logs. Zero       *
	 *                         means never remove.          *
	 ********************************************************/

	// Check the current log to see if it needs rotating.
	// If it does, rotate it and put the current time
	// on the end of the filename as UNIX timestamp.
	if (($log_limit > 0) && (filesize($log_file) >= $log_limit)) {
		$newfile = $log_file . "." . strval(time());
		try {
			copy($log_file, $newfile);
			file_put_contents($log_file, "");
		} catch (Exception $e) {
			log_error("[Suricata] Failed to rotate file '{$log_file}' -- error was {$e->getMessage()}");
		}
	}

	// Check previously rotated logs to see if time to
	// delete any older than the retention period.
	// Rotated logs have a UNIX timestamp appended to
	// filename.
	if ($retention > 0) {
		$now = time();
		$rotated_files = glob("{$log_file}.*");
		foreach ($rotated_files as $file) {
			if (($now - filemtime($file)) > ($retention * 3600))
				unlink_if_exists($file);
		}
		unset($rotated_files);
	}
}

/*************************
 * Start of main code    *
 *************************/

// If firewall is booting, do nothing
if ($g['booting'] == true)
	return;

// If no interfaces defined, there is nothing to clean up
if (!is_array($config['installedpackages']['suricata']['rule']))
	return;

$logs = array ();

// Build an arry of files to check and limits to check them against from our saved configuration
$logs['alerts.log']['limit'] = $config['installedpackages']['suricata']['config'][0]['alert_log_limit_size'];
$logs['alerts.log']['retention'] = $config['installedpackages']['suricata']['config'][0]['alert_log_retention'];
$logs['files-json.log']['limit'] = $config['installedpackages']['suricata']['config'][0]['files_json_log_limit_size'];
$logs['files-json.log']['retention'] = $config['installedpackages']['suricata']['config'][0]['files_json_log_retention'];
$logs['http.log']['limit'] = $config['installedpackages']['suricata']['config'][0]['http_log_limit_size'];
$logs['http.log']['retention'] = $config['installedpackages']['suricata']['config'][0]['http_log_retention'];
$logs['stats.log']['limit'] = $config['installedpackages']['suricata']['config'][0]['stats_log_limit_size'];
$logs['stats.log']['retention'] = $config['installedpackages']['suricata']['config'][0]['stats_log_retention'];
$logs['tls.log']['limit'] = $config['installedpackages']['suricata']['config'][0]['tls_log_limit_size'];
$logs['tls.log']['retention'] = $config['installedpackages']['suricata']['config'][0]['tls_log_retention'];

// Check log limits and retention in the interface logging directories if enabled
if ($config['installedpackages']['suricata']['config'][0]['enable_log_mgmt'] == 'on') {
	foreach ($config['installedpackages']['suricata']['rule'] as $value) {
		$if_real = get_real_interface($value['interface']);
		$suricata_log_dir = SURICATALOGDIR . "suricata_{$if_real}{$value['uuid']}";
		foreach ($logs as $k => $p)
			suricata_check_rotate_log("{$suricata_log_dir}/{$k}", $p['limit']*1024, $p['retention']);
	}

	// Prune any aged-out Barnyard2 archived logs if any exist
	if (is_dir("{$suricata_log_dir}/barnyard2/archive") &&
	    $config['installedpackages']['suricata']['config'][0]['u2_archive_log_retention'] > 0) {
		$now = time();
		$files = glob("{$suricata_log_dir}/barnyard2/archive/unified2.alert.*");
		foreach ($files as $f) {
			if (($now - filemtime($f)) > ($config['installedpackages']['suricata']['config'][0]['u2_archive_log_retention'] * 3600))
				unlink_if_exists($f);
		}
	}
	unset($files);
}

// Check the overall log directory limit (if enabled) and prune if necessary
if ($config['installedpackages']['suricata']['config'][0]['suricataloglimit'] == 'on')
	suricata_check_dir_size_limit($config['installedpackages']['suricata']['config'][0]['suricataloglimitsize']);

?>
