<?php
/*
	squid_antivirus.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2015 ESF, LLC
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
/* Functions for Squid C-ICAP/ClamAV integration */
require_once('globals.inc');
require_once('config.inc');
/* This file is currently only being included in squid.inc and not used separately */
// require_once('squid.inc');


/* Only needed for PBI/pfSense <2.3 */
function squid_check_clamav_user($user) {
	if (SQUID_BASE == '/usr/local') {
		return;
	}

	$_gc = exec("/usr/sbin/pw usershow {$user}", $sq_ex_output, $sq_ex_return);
	$user_arg = ($sq_ex_return == 0 ? "mod" : "add");
	$_gc = exec("/usr/sbin/pw user{$user_arg} {$user} -G wheel -u 9595 -s /sbin/nologin", $sq_ex_output, $sq_ex_return);
	if ($sq_ex_return != 0) {
		log_error("Squid - Could not change clamav user settings. " . serialize($sq_ex_output));
	}
}

/* Create /usr/local/etc/rc.d/clamd.sh rcfile */
function squid_write_clamd_rcfile() {
	$squid_base = SQUID_BASE;
	$rc = array();
	$rc['file'] = 'clamd.sh';
	$rc['start'] = <<< EOD

if [ ! -f /var/db/clamav/main.cvd -a ! -f /var/db/clamav/main.cld ];then
	echo "Missing /var/db/clamav/*.cvd or *.cld files.  You must run freshclam first"
        exit 1
fi

{$squid_base}/bin/clamd --config-file="{$squid_base}/local/etc/clamd.conf"

EOD;

	$rc['stop'] = <<< EOD

/usr/bin/killall clamd 2>/dev/null
# Just to be sure...
sleep 5
if [ -n "`/bin/ps auxw | /usr/bin/grep "[c]lamd" | /usr/bin/awk '{print $2}'`" ]; then
	/usr/bin/killall -9 clamd 2>/dev/null
fi

EOD;

	conf_mount_rw();
	write_rcfile($rc);
	conf_mount_ro();
}

/* Create /usr/local/etc/rc.d/c-icap.sh rcfile */
function squid_write_cicap_rcfile() {
	$c_icap_rcfile = "c-icap.sh";
	$cicap_libdir = SQUID_LOCALBASE . "/lib";
	$cicap_bin = SQUID_LOCALBASE . "/bin/c-icap";
	$cicap_conf = SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf";
	$cicap_start_cmd = "LD_LIBRARY_PATH={$cicap_libdir} {$cicap_bin} -f {$cicap_conf}";
	$cicap_stop_cmd = '/bin/echo -n "stop" > /var/run/c-icap/c-icap.ctl';
	conf_mount_rw();
	write_rcfile(array(
		"file" => "{$c_icap_rcfile}",
		"start" => "{$cicap_start_cmd}",
		"stop" => "{$cicap_stop_cmd}"
		)
	);
	conf_mount_ro();
}

/* ClamAV antivirus definitions updates via cron */
function squid_install_freshclam_cron($should_install) {
	global $config;

	if (platform_booting()) {
		return;
	}

	if (is_array($config['installedpackages']['squidantivirus'])) {
		$antivirus_config = $config['installedpackages']['squidantivirus']['config'][0];
	} else {
		$antivirus_config = array();
	}

	$freshclam_cmd = (SQUID_BASE . "/bin/freshclam --config-file=" . SQUID_BASE . "/etc/freshclam.conf");
	if (($should_install) && (squid_enabled())) {
		if ($antivirus_config['clamav_update'] != "0") {
			$minutes = ($antivirus_config['clamav_update'] * 60);
			install_cron_job("{$freshclam_cmd}", true, "*/{$minutes}", "*", "*", "*", "*", "clamav");
		} else {
			install_cron_job("{$freshclam_cmd}", false);
		}
	} else {
		install_cron_job("{$freshclam_cmd}", false);
	}
}

/* Manually update ClamAV virus definitions (via the GUI button) */
function squid_update_clamav() {
	log_error("Updating ClamAV definitions now... This will take a while. Check /var/log/clamav/freshclam.log for progress information.");
	mwexec_bg(SQUID_BASE . "/bin/freshclam --config-file=" . SQUID_BASE . "/etc/freshclam.conf");
}

/* Antivirus features configuration */
function squid_resync_antivirus() {
	global $config;

	if (is_array($config['installedpackages']['squidantivirus'])) {
		$antivirus_config = $config['installedpackages']['squidantivirus']['config'][0];
	} else {
		$antivirus_config = array();
	}

	if (squid_enabled() && ($antivirus_config['enable'] == "on")) {
		switch ($antivirus_config['client_info']) {
			case "both":
			default:
				$icap_send_client_ip = "on";
				$icap_send_client_username = "on";
				break;
			case "ip":
				$icap_send_client_ip = "on";
				$icap_send_client_username = "off";
				break;
			case "username":
				$icap_send_client_ip = "off";
				$icap_send_client_username = "on";
				break;
			case "none":
				$icap_send_client_ip = "off";
				$icap_send_client_username = "off";
				break;
		}

		$conf = <<< EOF
icap_enable on
icap_send_client_ip {$icap_send_client_ip}
icap_send_client_username {$icap_send_client_username}
icap_client_username_encode off
icap_client_username_header X-Authenticated-User
icap_preview_enable on
icap_preview_size 1024

icap_service service_avi_req reqmod_precache icap://[::1]:1344/squid_clamav bypass=off
adaptation_access service_avi_req allow all
icap_service service_avi_resp respmod_precache icap://[::1]:1344/squid_clamav bypass=on
adaptation_access service_avi_resp allow all

EOF;

		// check clamav user
		squid_check_clamav_user('clamav');
		// patch sample files to pfsense dirs
		// squidclamav.conf
		if (file_exists(SQUID_LOCALBASE . "/etc/c-icap/squidclamav.conf.default")) {
			$sample_file = file_get_contents(SQUID_LOCALBASE . "/etc/c-icap/squidclamav.conf.default");
			$clamav_m[0] = "@/var/run/clamav/clamd.ctl@";
			$clamav_m[1] = "@http\://proxy.domain.dom/cgi-bin/clwarn.cgi@";
			$clamav_r[0] = "/var/run/clamav/clamd.sock";
			$clamav_r[1] = "{$config['system']['webgui']['protocol']}://{$config['system']['hostname']}.{$config['system']['domain']}/squid_clwarn.php";
			if ($antivirus_config['clamav_safebrowsing'] == "on") {
				$clamav_m[2] = "@safebrowsing\s0@";
				$clamav_r[2] = "safebrowsing 1";
			} else {
				$clamav_m[2] = "@safebrowsing\s1@";
				$clamav_r[2] = "safebrowsing 0";
			}
			file_put_contents(SQUID_LOCALBASE . "/etc/c-icap/squidclamav.conf.sample", preg_replace($clamav_m, $clamav_r, $sample_file), LOCK_EX);
		}
		// c-icap.conf
		// make a backup of default c-icap.conf.sample first
		// unlike with other config files, the file distributed in package is called c-icap.conf.sample, not c-icap.conf.default
		if (!file_exists(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.default")) {
			copy(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.sample", SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.default");
		}
		if (file_exists(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.default")) {
			$sample_file = file_get_contents(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.default");
			if (!preg_match("/squid_clamav/", $sample_file)) {
				$sample_file .= "\nService squid_clamav squidclamav.so\n";
			}
			$cicap_m[0] = "@Manager:Apassword\S+@";
			$cicap_r[0] = "";
			// XXX: Bug #4615
			if (is_array($config['installedpackages']['squid'])) {
				$squidsettings = $config['installedpackages']['squid']['config'][0];
			} else {
				$squidsettings = array();
			}
			$logdir = ($squidsettings['log_dir'] ? $squidsettings['log_dir'] : '/var/squid/logs');
			$cicap_m[1] = "@DebugLevel\s1@";
			$cicap_r[1] = "DebugLevel 0";
			$cicap_m[2] = "@AccessLog /var/log/c-icap/access.log@";
			$cicap_r[2] = "AccessLog $logdir/c-icap-access.log";
			$cicap_m[3] = "@ServerLog /var/log/c-icap/server.log@";
			$cicap_r[3] = "ServerLog $logdir/c-icap-server.log";
			file_put_contents(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.sample", preg_replace($cicap_m, $cicap_r, $sample_file), LOCK_EX);
		}
		// freshclam.conf
		// make a backup of default freshclam.conf.sample first
		if (!file_exists(SQUID_LOCALBASE . "/etc/freshclam.conf.default")) {
			copy(SQUID_LOCALBASE . "/etc/freshclam.conf.sample", SQUID_LOCALBASE . "/etc/freshclam.conf.default");
		}
		if (file_exists(SQUID_LOCALBASE . "/etc/freshclam.conf.default")) {
			$sample_file = file_get_contents(SQUID_LOCALBASE . "/etc/freshclam.conf.default");
			$freshclam_m[0] = "@#Example@";
			$freshclam_r[0] = "";
			$clamav_mirrors = "";
			if ($antivirus_config['clamav_dbregion'] != "") {
				$clamav_mirrors .= "DatabaseMirror db.{$antivirus_config['clamav_dbregion']}.clamav.net\n";
			}
			if ($antivirus_config['clamav_dbservers'] != "") {
				foreach (explode(";", $antivirus_config['clamav_dbservers']) as $dbserver) {
					$clamav_mirrors .= "DatabaseMirror {$dbserver}\n";
				}
			}
			if ($clamav_mirrors != "") {
				$freshclam_m[1] = "@#DatabaseMirror db.XY.clamav.net@";
				$freshclam_r[1] = "{$clamav_mirrors}";
			}
			if ($antivirus_config['clamav_safebrowsing'] == "on") {
				$freshclam_m[2] = "@#SafeBrowsing yes@";
				$freshclam_r[2] = "SafeBrowsing yes";
			} else {
				if (!preg_match("@#SafeBrowsing yes@", file_get_contents($sample_file))) {
					$freshclam_m[2] = "@SafeBrowsing yes@";
					$freshclam_r[2] = "#SafeBrowsing yes";
				}
			}
			file_put_contents(SQUID_LOCALBASE . "/etc/freshclam.conf.sample", preg_replace($freshclam_m, $freshclam_r, $sample_file), LOCK_EX);
		}
		// freshclam cronjob
		squid_install_freshclam_cron(true);

		// check squidclamav files until PBIs are gone (https://redmine.pfsense.org/issues/4197)
		$ln_icap = array('bin/c-icap', 'bin/c-icap-client', 'c-icap-config', 'c-icap-libicapapi-config', 'c-icap-stretch', 'lib/c_icap', 'share/c_icap', 'etc/c-icap');
		foreach ($ln_icap as $ln) {
			if (SQUID_LOCALBASE != '/usr/local' && !file_exists("/usr/local/{$ln}") && file_exists(SQUID_LOCALBASE . "/{$ln}")) {
				symlink(SQUID_LOCALBASE . "/{$ln}", "/usr/local/{$ln}");
			}
		}
		if (SQUID_LOCALBASE != '/usr/local' && !file_exists("/usr/local/lib/libicapapi.so.3") && file_exists(SQUID_LOCALBASE . "/lib/libicapapi.so.3.0.5")) {
			symlink(SQUID_LOCALBASE . "/lib/libicapapi.so.3.0.5", "/usr/local/lib/libicapapi.so.3");
		}

		$loadsample = 0;
		if ($antivirus_config['squidclamav'] == "" && file_exists(SQUID_LOCALBASE . "/etc/c-icap/squidclamav.conf.sample")) {
			$config['installedpackages']['squidantivirus']['config'][0]['squidclamav'] = base64_encode(str_replace("\r", "", file_get_contents(SQUID_LOCALBASE . "/etc/c-icap/squidclamav.conf.sample")));
			$loadsample++;
		}
		if ($antivirus_config['c-icap_conf'] == "" && file_exists(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.sample")) {
			$config['installedpackages']['squidantivirus']['config'][0]['c-icap_conf'] = base64_encode(str_replace("\r", "", file_get_contents(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf.sample")));
			$loadsample++;
		}
		if ($antivirus_config['c-icap_magic'] == "" && file_exists(SQUID_LOCALBASE . "/etc/c-icap/c-icap.magic.sample")) {
			$config['installedpackages']['squidantivirus']['config'][0]['c-icap_magic'] = base64_encode(str_replace("\r", "", file_get_contents(SQUID_LOCALBASE . "/etc/c-icap/c-icap.magic.sample")));
			$loadsample++;
		}
		if ($antivirus_config['freshclam_conf'] == "" && file_exists(SQUID_LOCALBASE . "/etc/freshclam.conf.sample")) {
			$config['installedpackages']['squidantivirus']['config'][0]['freshclam_conf'] = base64_encode(str_replace("\r", "", file_get_contents(SQUID_LOCALBASE . "/etc/freshclam.conf.sample")));
			$loadsample++;
		}
		if ($loadsample > 0) {
			write_config();
			$antivirus_config = $config['installedpackages']['squidantivirus']['config'][0];
		}
		// check dirs
		$dirs = array(
			"/var/run/c-icap" => "clamav",
			"/var/log/c-icap" => "clamav",
			"/var/log/clamav" => "clamav",
			"/var/run/clamav" => "clamav",
			"/var/db/clamav" => "clamav"
		);
		foreach ($dirs as $dir_path => $dir_user) {
			safe_mkdir($dir_path, 0755);
			squid_chown_recursive($dir_path, $dir_user, "wheel");
		}

		// write advanced clamav/icap config files
		file_put_contents(SQUID_LOCALBASE . "/etc/c-icap/squidclamav.conf", base64_decode($antivirus_config['squidclamav']), LOCK_EX);
		file_put_contents(SQUID_LOCALBASE . "/etc/c-icap/c-icap.conf", base64_decode($antivirus_config['c-icap_conf']), LOCK_EX);
		file_put_contents(SQUID_LOCALBASE . "/etc/c-icap/c-icap.magic", base64_decode($antivirus_config['c-icap_magic']), LOCK_EX);
		file_put_contents(SQUID_LOCALBASE . "/etc/freshclam.conf", base64_decode($antivirus_config['freshclam_conf']), LOCK_EX);
	}
	// this will (re)start or stop/disable services as needed
	// depending on whether Squid proxy and/or antivirus features are enabled
	squid_restart_antivirus();

	return $conf;
}

/* Antivirus services handling */
function squid_restart_antivirus() {
	global $config;
	if (is_array($config['installedpackages']['squidantivirus'])) {
		$antivirus_config = $config['installedpackages']['squidantivirus']['config'][0];
	} else {
		$antivirus_config = array();
	}

	// reconfigure and (re)start service as needed if enabled, otherwise stop them
	// do not (re)start antivirus services on boot
	if (platform_booting()) {
		return;
	}

	if (squid_enabled() && ($antivirus_config['enable'] == "on")) {
		// Check clamav database
		if (count(glob("/var/db/clamav/*d")) == 0) {
			log_error("Squid - Missing /var/db/clamav/*.cvd or *.cld files. Running freshclam in background.");
			mwexec_bg(SQUID_BASE . "/bin/freshclam --config-file=" . SQUID_BASE . "/etc/freshclam.conf");
		} elseif ($antivirus_config['clamav_safebrowsing'] == "on" && !is_file("/var/db/clamav/safebrowsing.cvd")) {
			log_error("Squid - Google Safe Browsing is enabled but missing safebrowsing.cvd definitions. Running freshclam in background.");
			mwexec_bg(SQUID_BASE . "/bin/freshclam --config-file=" . SQUID_BASE . "/etc/freshclam.conf");
		} elseif ($antivirus_config['clamav_safebrowsing'] != "on" && is_file("/var/db/clamav/safebrowsing.cvd")) {
			log_error("Squid - Google Safe Browsing is disabled. Removing safebrowsing.cvd definitions.");
			mwexec("/bin/rm -f /var/db/clamav/safebrowsing.cvd");
		}

		// start/reload clamav
		$clamd_rcfile = "/usr/local/etc/rc.d/clamd.sh";
		if (!file_exists($clamd_rcfile)) {
			squid_write_clamd_rcfile();
		}
		if (is_process_running("clamd")) {
			log_error("Reloading ClamAV...");
			$reload_cmd = SQUID_BASE . "/bin/clamdscan --reload";
			mwexec_bg("{$reload_cmd}");
		} else {
			log_error("Starting ClamAV...");
			mwexec_bg("{$clamd_rcfile} start");
		}

		// check c-icap rcfile
		$c_icap_rcfile = "/usr/local/etc/rc.d/c-icap.sh";
		if (!file_exists($c_icap_rcfile)) {
			squid_write_cicap_rcfile();
		}
		if (is_process_running("c-icap")) {
			mwexec_bg('/bin/echo -n "reconfigure" > /var/run/c-icap/c-icap.ctl');
		} else {
			mwexec_bg("{$c_icap_rcfile} start");
		}
	} else {
		// stop AV services and disable all C-ICAP/AV features
		log_error("Squid antivirus features disabled.");
		if (is_process_running("clamd")) {
			log_error("Stopping and disabling ClamAV...");
			mwexec("/usr/bin/killall clamd");
		}
		unlink_if_exists("/usr/local/etc/rc.d/clamd.sh");

		// freshclam cronjob
		log_error("Removing freshclam cronjob...");
		squid_install_freshclam_cron(false);

		// check c-icap rcfile
		if (is_process_running("c-icap")) {
			log_error("Stopping and disabling C-ICAP...");
			mwexec('/bin/echo -n "stop" > /var/run/c-icap/c-icap.ctl');
		}
		unlink_if_exists("/usr/local/etc/rc.d/c-icap.sh");
	}
}

/* Input validation */
function squid_validate_antivirus($post, &$input_errors) {
	global $config;

	/* Manual ClamAV database update */
	if ($post['submit'] == 'Update AV') {
		squid_update_clamav();
		return;
	}

	if ($post['enable'] != "on") {
		return;
	}

	if ($post['squidclamav'] && preg_match("/(\S+proxy.domain\S+)/", $post['squidclamav'], $a_match)) {
		$input_errors[] = "SquidClamav warnings redirect points to sample config domain ({$a_match[1]})";
		$input_errors[] = "Change redirect info on 'squidclamav.conf' field to pfSense GUI or an external host.";
	}
	if ($post['c-icap_conf']) {
		if (!preg_match("/squid_clamav/", $post['c-icap_conf'])) {
			$input_errors[] = "c-icap Squidclamav service definition is not present.";
			$input_errors[] = "Add 'Service squid_clamav squidclamav.so'(without quotes) to 'c-icap.conf' field in order to get it working.";
		}
		if (preg_match("/(Manager:Apassword\S+)/", $post['c-icap_conf'], $c_match)) {
			$input_errors[] = "Remove ldap configuration'{$c_match[1]}' from 'c-icap.conf' field.";
		}
	}

	if ($post['clamav_dbservers']) {
		foreach (explode(";", $post['clamav_dbservers']) as $dbserver) {
			$dbserver = trim($dbserver);
			if (!empty($dbserver) && !is_ipaddr($dbserver) && !is_hostname($dbserver)) {
				$input_errors[] = "'Optional ClamAV Database Update Servers' entry '$dbserver' is not a valid IP address or hostname.";
			}
		}
	}
}

?>
