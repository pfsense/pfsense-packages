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

//        'B' => 1,
//        'KB' => 1024,
//        'MB' => 1024 * 1024,
//        'GB' => 1024 * 1024 * 1024,
//        'TB' => 1024 * 1024 * 1024 * 1024,
//        'PB' => 1024 * 1024 * 1024 * 1024 * 1024,


/* chk if snort log dir is full if so clear it */
$suricataloglimit = $config['installedpackages']['suricata']['config'][0]['suricataloglimit'];
$suricataloglimitsize = $config['installedpackages']['suricata']['config'][0]['suricataloglimitsize'];

if ($g['booting']==true)
	return;

if ($suricataloglimit == 'off')
	return;

if (!is_array($config['installedpackages']['suricata']['rule']))
	return;

/* Convert Log Limit Size setting from MB to KB */
$suricataloglimitsizeKB = round($suricataloglimitsize * 1024);
$suricatalogdirsizeKB = suricata_Getdirsize(SURICATALOGDIR);
if ($suricatalogdirsizeKB > 0 && $suricatalogdirsizeKB > $suricataloglimitsizeKB) {
	log_error(gettext("[Suricata] Log directory size exceeds configured limit of " . number_format($suricataloglimitsize) . " MB set on Global Settings tab. All Suricata log files will be truncated."));
	conf_mount_rw();

	/* Truncate the Rules Update Log file if it exists */
	if (file_exists(RULES_UPD_LOGFILE)) {
		log_error(gettext("[Suricata] Truncating the Rules Update Log file..."));
		$fd = @fopen(RULES_UPD_LOGFILE, "w+");
		if ($fd)
			fclose($fd);
	}

	/* Clean-up the logs for each configured Suricata instance */
	foreach ($config['installedpackages']['suricata']['rule'] as $value) {
		$if_real = get_real_interface($value['interface']);
		$suricata_uuid = $value['uuid'];
		$suricata_log_dir = SURICATALOGDIR . "suricata_{$if_real}{$suricata_uuid}";
		log_error(gettext("[Suricata] Truncating logs for {$value['descr']} ({$if_real})..."));
		suricata_post_delete_logs($suricata_uuid);

		// Initialize an array of the log files we want to prune
		$logs = array ( "alerts.log", "http.log", "files-json.log", "tls.log", "stats.log" );

		foreach ($logs as $file) {
			// Truncate the log file if it exists
			if (file_exists("{$suricata_log_dir}/$file")) {
				$fd = @fopen("{$suricata_log_dir}/$file", "w+");
				if ($fd)
					fclose($fd);
			}
		}

		// Check for any captured stored files and clean them up
		$filelist = glob("{$suricata_log_dir}/files/*");
		if (!empty($filelist)) {
			foreach ($filelist as $file)
				@unlink($file);
		}

		// This is needed if suricata is run as suricata user
		mwexec('/bin/chmod 660 /var/log/suricata/*', true);

		// Soft-restart Suricata process to resync logging
		if (file_exists("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid")) {
			log_error(gettext("[Suricata] Restarting logging on {$value['descr']} ({$if_real})..."));
			mwexec("/bin/pkill -HUP -F {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid -a");
		}
	}
	conf_mount_ro();
	log_error(gettext("[Suricata] Automatic clean-up of Suricata logs completed."));
}

?>
