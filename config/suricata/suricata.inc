<?php
/*
	suricata.inc

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
require_once("pfsense-utils.inc");
require_once("config.inc");
require_once("functions.inc");
require_once("services.inc");
require_once("service-utils.inc");
require_once("pkg-utils.inc");
require_once("filter.inc");

global $g, $config;

if (!is_array($config['installedpackages']['suricata']))
	$config['installedpackages']['suricata'] = array();

// Define the binary and package build versions
define('SURICATA_VER', '1.4.6');
define('SURICATA_PKG_VER', 'v0.3-BETA');

// Create some other useful defines
define('SURICATADIR', '/usr/pbi/suricata-' . php_uname("m") . '/etc/suricata/');
define('SURICATALOGDIR', '/var/log/suricata/');
define('RULES_UPD_LOGFILE', SURICATALOGDIR . 'suricata_rules_update.log');
define('ENFORCING_RULES_FILENAME', 'suricata.rules');
define('FLOWBITS_FILENAME', 'flowbit-required.rules');

// Rule set download filenames and prefixes
define('ET_DNLD_FILENAME', 'emerging.rules.tar.gz');
define('ETPRO_DNLD_FILENAME', 'etpro.rules.tar.gz');
define('VRT_DNLD_FILENAME', 'snortrules-snapshot-edge.tar.gz');
define('GPLV2_DNLD_FILENAME', 'community-rules.tar.gz');
define('VRT_FILE_PREFIX', 'snort_');
define('GPL_FILE_PREFIX', 'GPLv2_');
define('ET_OPEN_FILE_PREFIX', 'emerging-');
define('ET_PRO_FILE_PREFIX', 'etpro-');

function suricata_generate_id() {
	global $config;

	$suricatacfg = $config['installedpackages']['suricata']['rule'];
	while (true) {
		$suricata_uuid = mt_rand(1, 65535);
		foreach ($suricatacfg as $value) {
			if ($value['uuid'] == $suricata_uuid)
				continue 2;
		}
		break;
	}

	return $suricata_uuid;
}

function suricata_is_running($suricata_uuid, $if_real, $type = 'suricata') {
	global $g;
	return isvalidpid("{$g['varrun_path']}/{$type}_{$if_real}{$suricata_uuid}.pid");
}

function suricata_barnyard_stop($suricatacfg, $if_real) {
	global $g;

	$suricata_uuid = $suricatacfg['uuid'];
	if (isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid")) {
		log_error("[Suricata] Barnyard2 STOP for {$suricatacfg['descr']}({$if_real})...");
		killbypid("{$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid");
	}
}

function suricata_stop($suricatacfg, $if_real) {
	global $g;

	$suricata_uuid = $suricatacfg['uuid'];
	if (isvalidpid("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid")) {
		log_error("[Suricata] Suricata STOP for {$suricatacfg['descr']}({$if_real})...");
		killbypid("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid");
		sleep(1);

		// For some reason Suricata seems to need a double TERM signal to actually shutdown
		if (isvalidpid("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid"))
			killbypid("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid");
	}
	// Stop Barnyard2 on the interface if running
	suricata_barnyard_stop($suricatacfg, $if_real);
}

function suricata_barnyard_start($suricatacfg, $if_real) {
	global $g;

	$suricata_uuid = $suricatacfg['uuid'];
	$suricatadir = SURICATADIR . "suricata_{$suricata_uuid}_{$if_real}";
	$suricatalogdir = SURICATALOGDIR . "suricata_{$if_real}{$suricata_uuid}";

	if ($suricatacfg['barnyard_enable'] == 'on') {
		log_error("[Suricata] Barnyard2 START for {$suricatacfg['descr']}({$if_real})...");
		mwexec_bg("/usr/local/bin/barnyard2 -r {$suricata_uuid} -f unified2.alert --pid-path {$g['varrun_path']} --nolock-pidfile -c {$suricatadir}/barnyard2.conf -d {$suricatalogdir} -D -q");
	}
}

function suricata_start($suricatacfg, $if_real) {
	global $g;

	$suricatadir = SURICATADIR;
	$suricata_uuid = $suricatacfg['uuid'];

	if ($suricatacfg['enable'] == 'on') {
		log_error("[Suricata] Suricata START for {$suricatacfg['descr']}({$if_real})...");
		mwexec_bg("/usr/local/bin/suricata -i {$if_real} -D -c {$suricatadir}suricata_{$suricata_uuid}_{$if_real}/suricata.yaml --pidfile {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid");
	}
	else
		return;

	// Start Barnyard2 if enabled on the interface
	suricata_barnyard_start($suricatacfg, $if_real);
}

function suricata_reload_config($suricatacfg, $signal="USR2") {

	/**************************************************************/
	/* This function sends the passed SIGNAL to the Suricata      */
	/* instance on the passed interface to cause Suricata to      */
	/* reload and parse the running configuration without         */
	/* impacting packet processing.  It also executes the reload  */
	/* as a background process and returns control immediately    */
	/* to the caller.                                             */
	/*                                                            */
	/*  $signal = USR2 (default) parses and reloads config.       */
	/**************************************************************/
	global $g;

	$suricatadir = SURICATADIR;
	$suricata_uuid = $suricatacfg['uuid'];
	$if_real = get_real_interface($suricatacfg['interface']);

	/******************************************************/
	/* Only send the SIGUSR2 if Suricata is running and   */
	/* we can find a valid PID for the process.           */
	/******************************************************/
	if (isvalidpid("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid")) {
		log_error("[Suricata] Suricata LIVE RULE RELOAD initiated for {$suricatacfg['descr']} ({$if_real})...");
//		sigkillbypid("{$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid", $signal);
		mwexec_bg("/bin/pkill -{$signal} -F {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid");
	}
}

function suricata_barnyard_reload_config($suricatacfg, $signal="HUP") {

	/**************************************************************/
	/* This function sends the passed SIGNAL to the Barnyard2     */
	/* instance on the passed interface to cause Barnyard to      */
	/* reload and parse the running configuration without         */
	/* impacting packet processing.  It also executes the reload  */
	/* as a background process and returns control immediately    */
	/* to the caller.                                             */
	/*                                                            */
	/*  $signal = HUP (default) parses and reloads config.        */
	/**************************************************************/
	global $g;

	$suricatadir = SURICATADIR;
	$suricata_uuid = $suricatacfg['uuid'];
	$if_real = get_real_interface($suricatacfg['interface']);

	/******************************************************/
	/* Only send the SIGHUP if Barnyard2 is running and   */
	/* we can find a valid PID for the process.           */
	/******************************************************/
	if (isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid")) {
		log_error("[Suricata] Barnyard2 CONFIG RELOAD initiated for {$suricatacfg['descr']} ({$if_real})...");
//		sigkillbypid("{$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid", $signal);
		mwexec_bg("/bin/pkill -{$signal} -F {$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid");
	}
}

function suricata_get_blocked_ips() {

	// This is a placeholder function for later use.
	// Blocking is not currently enabled in Suricata.
	return array();
}

/* func builds custom white lists */
function suricata_find_list($find_name, $type = 'whitelist') {
	global $config;

	$suricataglob = $config['installedpackages']['suricata'];
	if (!is_array($suricataglob[$type]))
		return "";
	if (!is_array($suricataglob[$type]['item']))
		return "";

	foreach ($suricataglob[$type]['item'] as $value) {
		if ($value['name'] == $find_name)
			return $value;
	}

	return array();
}

function suricata_build_list($suricatacfg, $listname = "", $whitelist = false) {

	/***********************************************************/
	/* The default is to build a HOME_NET variable unless      */
	/* '$whitelist' is set to 'true' when calling.             */
	/***********************************************************/

	global $config, $g, $aliastable, $filterdns;
	$home_net = array();

	if ($listname == 'default' || empty($listname)) {
		$localnet = 'yes'; $wanip = 'yes'; $wangw = 'yes'; $wandns = 'yes'; $vips = 'yes'; $vpns = 'yes';
	} else {
                $list = suricata_find_list($listname);
                if (empty($list))
                        return $list;
		$localnet = $list['localnets'];
		$wanip = $list['wanips'];
		$wangw = $list['wangateips'];
		$wandns = $list['wandnsips'];
		$vips = $list['vips'];
		$vpns = $list['vpnips'];
		if (!empty($list['address']) && is_alias($list['address']))
			$home_net = explode(" ", trim(filter_expand_alias($list['address'])));
	}

	// Always add loopback to HOME_NET and whitelist (ftphelper)
	if (!in_array("127.0.0.1", $home_net))
		$home_net[] = "127.0.0.1";

	/********************************************************************/
	/* Always put the interface running Suricata in HOME_NET and        */
	/* whitelist unless it's the WAN.  WAN options are handled further  */
	/* down.  If the user specifically chose not to include LOCAL_NETS  */
	/* in the WHITELIST, then do not include the Suricata interface     */
	/* subnet in the WHITELIST. We do include the actual LAN interface  */
	/* IP for Suricata, though, to prevent locking out the firewall.    */
	/********************************************************************/
	$suricataip = get_interface_ip($suricatacfg['interface']);
	if (!$whitelist || $localnet == 'yes' || empty($localnet)) {
		if (is_ipaddr($suricataip)) {
			if ($suricatacfg['interface'] <> "wan") {
				$sn = get_interface_subnet($suricatacfg['interface']);
				$ip = gen_subnet($suricataip, $sn) . "/{$sn}";
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
		}
	}
	else {
		if (is_ipaddr($suricataip)) {
			if (!in_array($suricataip, $home_net))
				$home_net[] = $suricataip;
		}
	}

	$suricataip = get_interface_ipv6($suricatacfg['interface']);
	if (!$whitelist || $localnet == 'yes' || empty($localnet)) {
		if (is_ipaddrv6($suricataip)) {
			if ($suricatacfg['interface'] <> "wan") {
				$sn = get_interface_subnetv6($suricatacfg['interface']);
				$ip = gen_subnetv6($suricataip, $sn). "/{$sn}";
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
		}
	}
	else {
		if (is_ipaddrv6($suricataip)) {
			if (!in_array($suricataip, $home_net))
				$home_net[] = $suricataip;
		}
	}

	if (!$whitelist || $localnet == 'yes' || empty($localnet)) {		
		/*************************************************************************/
		/*  Iterate through the interface list and write out whitelist items and */
		/*  also compile a HOME_NET list of all the local interfaces for suricata.  */
		/*  Skip the WAN interface as we do not typically want that whole subnet */
		/*  whitelisted (just the i/f IP itself which was handled earlier).      */
		/*************************************************************************/
		$int_array = get_configured_interface_list();
		foreach ($int_array as $int) {
			if ($int == "wan")
				continue;
			$subnet = get_interface_ip($int);
			if (is_ipaddr($subnet)) {
				$sn = get_interface_subnet($int);
				$ip = gen_subnet($subnet, $sn) . "/{$sn}";
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
			if ($int == "wan")
				continue;
			$subnet = get_interface_ipv6($int);
			if (is_ipaddrv6($subnet)) {
				$sn = get_interface_subnetv6($int);
				$ip = gen_subnetv6($subnet, $sn). "/{$sn}";
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
		}
	}

	if ($wanip == 'yes') {
		$ip = get_interface_ip("wan");
		if (is_ipaddr($ip)) {
			if (!in_array($ip, $home_net))
				$home_net[] = $ip;
		}
		$ip = get_interface_ipv6("wan");
		if (is_ipaddrv6($ip)) {
			if (!in_array($ip, $home_net))
				$home_net[] = $ip;
		}
	}

	if ($wangw == 'yes') {
		// Grab the default gateway if set
		$default_gw = exec("/sbin/route -n get default |grep 'gateway:' | /usr/bin/awk '{ print $2 }'");
		if (is_ipaddr($default_gw) && !in_array($default_gw, $home_net))
			$home_net[] = $default_gw;
		if (is_ipaddrv6($default_gw) && !in_array($default_gw, $home_net))
			$home_net[] = $default_gw;

		// Get any other interface gateway and put in $HOME_NET if not there already
		$gw = get_interface_gateway($suricatacfg['interface']);
		if (is_ipaddr($gw) && !in_array($gw, $home_net))
			$home_net[] = $gw;
		$gw = get_interface_gateway_v6($suricatacfg['interface']);
		if (is_ipaddrv6($gw) && !in_array($gw, $home_net))
			$home_net[] = $gw;
	}

	if ($wandns == 'yes') {
		// Add DNS server for WAN interface to whitelist
		$dns_servers = get_dns_servers();
		foreach ($dns_servers as $dns) {
			if ($dns && !in_array($dns, $home_net))
				$home_net[] = $dns;
		}
	}

	if($vips == 'yes') {
		// iterate all vips and add to whitelist
		if (is_array($config['virtualip']) && is_array($config['virtualip']['vip'])) {
			foreach($config['virtualip']['vip'] as $vip) {
				if ($vip['subnet'] && $vip['mode'] != 'proxyarp') {
					if (!in_array("{$vip['subnet']}/{$vip['subnet_bits']}", $home_net))
						$home_net[] = "{$vip['subnet']}/{$vip['subnet_bits']}";
				}
			}
		}
	}

	// grab a list of vpns and whitelist if user desires
	if ($vpns == 'yes') {
		$vpns_list = filter_get_vpns_list();
		if (!empty($vpns_list)) {
			// Convert the returned space-delimited string to an array
			// and then add each VPN address to our HOME_NET array.
			$vpns = explode(" ", $vpns_list);
			foreach ($vpns as $vpn)
				$home_net[] = trim($vpn);
			unset($vpns, $vpns_list);
		}
	}

	$valresult = array();
	foreach ($home_net as $vald) { 
		if (empty($vald))
			continue;
		$vald = trim($vald);
		if (empty($valresult[$vald]))
			$valresult[$vald] = $vald;
	}

	// Release memory no longer required
	unset($home_net);

	// Sort the list and return it
	natsort($valresult);
	return $valresult;
}

function suricata_rules_up_install_cron($should_install=true) {
	global $config, $g;

	$command = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/www/suricata/suricata_check_for_rule_updates.php";

	// Get auto-rule update parameter from configuration
	$suricata_rules_up_info_ck = $config['installedpackages']['suricata']['config'][0]['autoruleupdate'];

	// See if a customized start time has been set for rule file updates
	if (!empty($config['installedpackages']['suricata']['config'][0]['autoruleupdatetime']))
		$suricata_rules_upd_time = $config['installedpackages']['suricata']['config'][0]['autoruleupdatetime'];
	else
		$suricata_rules_upd_time = "00:03";

	if ($suricata_rules_up_info_ck == "6h_up") {
		$suricata_rules_up_min = intval(substr($suricata_rules_upd_time, -2));
		$hour = intval(substr($suricata_rules_upd_time, 0, 2));
		$suricata_rules_up_hr = strval($hour);
		for ($i=0; $i<3; $i++) {
			$hour += 6;
			if ($hour > 24)
				$hour -= 24;
			$suricata_rules_up_hr .= "," . strval($hour);
		}
		$suricata_rules_up_mday = "*";
		$suricata_rules_up_month = "*";
		$suricata_rules_up_wday = "*";
	}
	if ($suricata_rules_up_info_ck == "12h_up") {
		$suricata_rules_up_min = intval(substr($suricata_rules_upd_time, -2));
		$hour = intval(substr($suricata_rules_upd_time, 0, 2));
		$suricata_rules_up_hr = strval($hour) . ",";
		$hour += 12;
		if ($hour > 24)
			$hour -= 24;
		$suricata_rules_up_hr .= strval($hour);
		$suricata_rules_up_mday = "*";
		$suricata_rules_up_month = "*";
		$suricata_rules_up_wday = "*";
	}
	if ($suricata_rules_up_info_ck == "1d_up") {
		$suricata_rules_up_min = intval(substr($suricata_rules_upd_time, -2));
		$suricata_rules_up_hr = intval(substr($suricata_rules_upd_time, 0, 2));
		$suricata_rules_up_mday = "*/1";
		$suricata_rules_up_month = "*";
		$suricata_rules_up_wday = "*";
	}
	if ($suricata_rules_up_info_ck == "4d_up") {
		$suricata_rules_up_min = intval(substr($suricata_rules_upd_time, -2));
		$suricata_rules_up_hr = intval(substr($suricata_rules_upd_time, 0, 2));
		$suricata_rules_up_mday = "*/4";
		$suricata_rules_up_month = "*";
		$suricata_rules_up_wday = "*";
	}
	if ($suricata_rules_up_info_ck == "7d_up") {
		$suricata_rules_up_min = intval(substr($suricata_rules_upd_time, -2));
		$suricata_rules_up_hr = intval(substr($suricata_rules_upd_time, 0, 2));
		$suricata_rules_up_mday = "*/7";
		$suricata_rules_up_month = "*";
		$suricata_rules_up_wday = "*";
	}
	if ($suricata_rules_up_info_ck == "28d_up") {
		$suricata_rules_up_min = intval(substr($suricata_rules_upd_time, -2));
		$suricata_rules_up_hr = intval(substr($suricata_rules_upd_time, 0, 2));
		$suricata_rules_up_mday = "*/28";
		$suricata_rules_up_month = "*";
		$suricata_rules_up_wday = "*";
	}

	// System call to manage the cron job.
	install_cron_job($command, $should_install, $suricata_rules_up_min, $suricata_rules_up_hr, $suricata_rules_up_mday, $suricata_rules_up_month, $suricata_rules_up_wday, "root");
}

function suricata_loglimit_install_cron($should_install=true) {

	install_cron_job("/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/suricata/suricata_check_cron_misc.inc", $should_install, "*/5");
}

function sync_suricata_package_config() {
	global $config, $g;

	$suricatadir = SURICATADIR;
	$rcdir = RCFILEPREFIX;

	conf_mount_rw();

	// Do not start config build if there are no Suricata-configured interfaces
	if (!is_array($config['installedpackages']['suricata']) || !is_array($config['installedpackages']['suricata']['rule'])) {
		@unlink("{$rcdir}/suricata.sh");
		conf_mount_ro();
		return;
	}

	$suricataconf = $config['installedpackages']['suricata']['rule'];
	foreach ($suricataconf as $value) {
		$if_real = get_real_interface($value['interface']);

		// create a suricata.yaml file for interface
		suricata_generate_yaml($value);

		// create barnyard2.conf file for interface
		if ($value['barnyard_enable'] == 'on')
			suricata_generate_barnyard2_conf($value, $if_real);
	}

	// create suricata bootup file suricata.sh
	suricata_create_rc();

	$suricataglob = $config['installedpackages']['suricata']['config'][0];
	// setup the log directory size check job if enabled
	suricata_loglimit_install_cron();
	// setup the suricata rules update job if enabled
	suricata_rules_up_install_cron($suricataglob['autoruleupdate'] != "never_up" ? true : false);

	write_config();
	configure_cron();
	
	// Do not attempt package sync if reinstalling package or booting
//	if (!$g['suricata_postinstall'] && !$g['booting'])
//		suricata_sync_on_changes();
	
	conf_mount_ro();
}

function suricata_load_suppress_sigs($suricatacfg, $track_by=false) {

	global $config;

	/**********************************************************/
	/* This function loads the GEN_ID and SIG_ID for all the  */
	/* suppressed alert entries from the Suppression List of  */
	/* the passed Suricata interface.  The results are        */
	/* returned in an array with GEN_ID and SIG_ID as the     */
	/* primary keys.  Any "track by_src" or "track by_dst"    */
	/* entries in the Suppression List are tacked on as       */
	/* additional keys in the array along with the IP address */
	/* in either IPv4 or IPv6 format when $track_by is passed */
	/* as true.                                               */
	/*                                                        */
	/* Sample returned array:                                 */
	/*  $suppress[1][2069] = "suppress"                       */
	/*  $suppress[1][2070]['by_src']['10.1.1.5'] = "suppress" */
	/*  $suppress[1][2070]['by_dst']['10.1.1.6'] = "suppress" */
	/*                                                        */
	/**********************************************************/

	$suppress = array();

	if (!is_array($config['installedpackages']['suricata']))
		return;
	if (!is_array($config['installedpackages']['suricata']['suppress']))
		return;
	if (!is_array($config['installedpackages']['suricata']['suppress']['item']))
		return;
	$a_suppress = $config['installedpackages']['suricata']['suppress']['item'];

	foreach ($a_suppress as $a_id => $alist) {
		if ($alist['name'] == $suricatacfg['suppresslistname']) {
			if (!empty($alist['suppresspassthru'])) {
				$tmplist = str_replace("\r", "", base64_decode($alist['suppresspassthru']));
				$tmp = explode("\n", $tmplist);
				foreach ($tmp as $line) {
					// Skip any blank lines
					if (trim($line, " \n") == "")
						continue;
					// Skip any comment lines
					if (preg_match('/^\s*#/', $line))
						continue;
					/* See if entry suppresses GID:SID for all hosts */
					if (preg_match('/\s*suppress\s*gen_id\b\s*(\d+),\s*sig_id\b\s*(\d+)\s*$/i', $line, $matches)) {
						$genid = $matches[1];
						$sigid = $matches[2];
						if (!empty($genid) && !empty($sigid)) {
							if (!is_array($suppress[$genid]))
								$suppress[$genid] = array();
							if (!is_array($suppress[$genid][$sigid]))
								$suppress[$genid][$sigid] = array();
							$suppress[$genid][$sigid] = "suppress";
						}
					}

					/* Get "track by IP" entries if requested */
					if ($track_by) {
						/* See if entry suppresses only by SRC or DST IPv4 address */
						if (preg_match('/\s*suppress\s*gen_id\b\s*(\d+),\s*sig_id\b\s*(\d+),\s*track\s*(by_src|by_dst),\s*ip\s*(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\s*$/i', $line, $matches)) {
							$genid = $matches[1];
							$sigid = $matches[2];
							$whichip = trim($matches[3]);
							$ip = $matches[4];
							if (!empty($genid) && !empty($sigid) && !empty($whichip) && !empty($ip)) {
								if (!is_array($suppress[$genid]))
									$suppress[$genid] = array();
								if (!is_array($suppress[$genid][$sigid]))
									$suppress[$genid][$sigid] = array();
								if (!is_array($suppress[$genid][$sigid][$whichip]))
									$suppress[$genid][$sigid][$whichip] = array();
								if (!is_array($suppress[$genid][$sigid][$whichip][$ip]))
									$suppress[$genid][$sigid][$whichip][$ip] = array();
								$suppress[$genid][$sigid][$whichip][$ip] = "suppress";
							}
						}
						/* See if entry suppresses only by SRC or DST IPv6 address */
						if (preg_match('/\s*suppress\s*gen_id\b\s*(\d+),\s*sig_id\b\s*(\d+),\s*track\s*(by_src|by_dst),\s*ip\s*([0-9a-f\.:]+)\s*$/i', $line, $matches)) {
							$genid = $matches[1];
							$sigid = $matches[2];
							$whichip = trim($matches[3]);
							$ip = trim($matches[4]);
							if (!empty($genid) && !empty($sigid) && !empty($whichip) && !empty($ip)) {
								if (!is_array($suppress[$genid]))
									$suppress[$genid] = array();
								if (!is_array($suppress[$genid][$sigid]))
									$suppress[$genid][$sigid] = array();
								if (!is_array($suppress[$genid][$sigid][$whichip]))
									$suppress[$genid][$sigid][$whichip] = array();
								if (!is_array($suppress[$genid][$sigid][$whichip][$ip]))
									$suppress[$genid][$sigid][$whichip][$ip] = array();
								$suppress[$genid][$sigid][$whichip][$ip] = "suppress";
							}
						}
					}
				}
				unset($tmp);
			}
			break;
		}
	}
	unset($alist);
	return $suppress;
}

function suricata_post_delete_logs($suricata_uuid = 0) {

	/***********************************************/
	/* This function cleans up related log files   */
	/* for the passed instance.  These include     */
	/* Barnyard2 unified2 logs and pcap logs.      */
	/***********************************************/

	global $config, $g;

	// do nothing if no Suricata interfaces active
	if (!is_array($config['installedpackages']['suricata']['rule']))
		return;

	foreach ($config['installedpackages']['suricata']['rule'] as $value) {
		if ($value['uuid'] != $suricata_uuid)
			continue;
		$if_real = get_real_interface($value['interface']);
		$suricata_log_dir = SURICATALOGDIR . "suricata_{$if_real}{$suricata_uuid}";

		if ($if_real != '') {
			/* Clean-up Barnyard2 files if any exist */
			$filelist = glob("{$suricata_log_dir}/unified2.alert.*");
			// Keep most recent file
			unset($filelist[count($filelist) - 1]);
			foreach ($filelist as $file)
				@unlink($file);

			/* Clean-up Barnyard2 archived files if any exist */
			$filelist = glob("{$suricata_log_dir}/barnyard2/archive/unified2.alert.*");
			foreach ($filelist as $file)
				@unlink($file);

			/* Clean-up packet capture files if any exist */
			$filelist = glob("{$suricata_log_dir}/log.pcap.*");
			// Keep most recent file
			unset($filelist[count($filelist) - 1]);
			foreach ($filelist as $file)
				@unlink($file);
			unset($filelist);
		}
	}
}

/* This returns size of passed directory or file in 1024-byte blocks */
function suricata_Getdirsize($node) {
	if(!is_readable($node))
		return false;

	$blah = exec( "/usr/bin/du -kdc $node" );
	return substr( $blah, 0, strpos($blah, 9) );
}

function suricata_build_sid_msg_map($rules_path, $sid_file) {

	/*************************************************************/
	/* This function reads all the rules file in the passed      */
	/* $rules_path variable and produces a properly formatted    */
	/* sid-msg.map v2 file for use by Suricata and barnyard2.    */
	/*                                                           */
	/* This function produces the new v2 format sid-msg.map      */
	/* with the field layout as follows:                         */
	/*                                                           */
	/*  GID || SID || REV || CLASSTYPE || PRI || MSG || REF ...  */
	/*                                                           */
	/*  On Entry: $rules_path --> array or directory of files    */
	/*                            or a single file containing    */
	/*                            the rules to read.             */
	/*              $sid_file --> the complete destination path  */
	/*                            and filename for the output    */
	/*                            sid-msg.map file.              */
	/*************************************************************/

	$sidMap = array();
	$rule_files = array();

	// First check if we were passed a directory, a single file
	// or an array of filenames to read. Set our $rule_files
	// variable accordingly. If we can't figure it out, return
	// and don't write a sid-msg.map file.
	if (is_string($rules_path)) {
		if (is_dir($rules_path))
			$rule_files = glob($rules_path . "*.rules");
		elseif (is_file($rules_path))
			$rule_files = (array)$rules_path;
	}
	elseif (is_array($rules_path))
		$rule_files = $rules_path;
	else
		return;

	// Read the rule files into an array, then iterate the list
	foreach ($rule_files as $file) {

		// Don't process files with "deleted" in the filename
		if (stristr($file, "deleted"))
			continue;

		// Read the file into an array, skipping missing files.
		if (!file_exists($file))
			continue;

		$rules_array = file($file, FILE_SKIP_EMPTY_LINES);
		$record = "";
		$b_Multiline = false;

		// Read and process each line from the rules in the current file
		foreach ($rules_array as $rule) {

			// Skip any non-rule lines unless we're in multiline mode.
			if (!preg_match('/^\s*#*\s*(alert|drop|pass)/i', $rule) && !$b_Multiline)
				continue;

			// Test for a multi-line rule, and reassemble the
			// pieces back into a single line.
			if (preg_match('/\\\\s*[\n]$/m', $rule)) {
				$rule = substr($rule, 0, strrpos($rule, '\\'));
				$record .= $rule;
				$b_Multiline = true;
				continue;
			}
			// If the last segment of a multiline rule, then
			// append it onto the previous parts to form a
			// single-line rule for further processing below.
			elseif (!preg_match('/\\\\s*[\n]$/m', $rule) && $b_Multiline) {
				$record .= $rule;
				$rule = $record;
			}
			$b_Multiline = false;
			$record = "";

			// Parse the rule to find sid and any references.
			$gid = '1';             // default to 1 for regular rules
			$sid = '';
			$rev = '';
			$classtype = 'NOCLASS'; // required default for v2 format
			$priority = '0';        // required default for v2 format
			$msg = '';
			$matches = '';
			$sidEntry = '';
			if (preg_match('/\bmsg\s*:\s*"(.+?)"\s*;/i', $rule, $matches))
				$msg = trim($matches[1]);
			if (preg_match('/\bsid\s*:\s*(\d+)\s*;/i', $rule, $matches))
				$sid = trim($matches[1]);
			if (preg_match('/\bgid\s*:\s*(\d+)\s*;/i', $rule, $matches))
				$gid = trim($matches[1]);
			if (preg_match('/\brev\s*:\s*([^\;]+)/i', $rule, $matches))
				$rev = trim($matches[1]);
			if (preg_match('/\bclasstype\s*:\s*([^\;]+)/i', $rule, $matches))
				$classtype = trim($matches[1]);
			if (preg_match('/\bpriority\s*:\s*([^\;]+)/i', $rule, $matches))
				$priority = trim($matches[1]);

			if (!empty($gid) && !empty($sid) && !empty($msg)) {
				$sidEntry = $gid . ' || ' . $sid . ' || ' . $rev . ' || ' . $classtype . ' || ';
				$sidEntry .= $priority . ' || ' . $msg;
				preg_match_all('/\breference\s*:\s*([^\;]+)/i', $rule, $matches);
				foreach ($matches[1] as $ref)
					$sidEntry .= " || " . trim($ref);
				$sidEntry .= "\n";
				$sidMap[] = $sidEntry;
			}
		}
        }
	// Sort the generated sid-msg map
	natcasesort($sidMap);

	// Now print the result to the supplied file
	@file_put_contents($sid_file, "#v2\n# sid-msg.map file auto-generated by Snort.\n\n");
	@file_put_contents($sid_file, array_values($sidMap), FILE_APPEND);
}

function suricata_merge_reference_configs($cfg_in, $cfg_out) {

	/***********************************************************/
	/* This function takes a list of "reference.config" files  */
	/* in the $cfg_in array and merges them into a single      */
	/* file specified by $cfg_out.  The merging is done so     */
	/* no duplication of lines occurs in the output file.      */
	/***********************************************************/

        $outMap = array();
        foreach ($cfg_in as $file) {
	     if (!file_exists($file))
			continue;
                $in = file($file, FILE_SKIP_EMPTY_LINES);
                foreach ($in as $line) {
                        /* Skip comment lines  */
                        if (preg_match('/^\s*#/', $line))
                                continue;
                        if (preg_match('/(\:)\s*(\w+)\s*(.*)/', $line, $matches)) {
                                if (!empty($matches[2]) && !empty($matches[3])) {
                                        $matches[2] = trim($matches[2]);
                                        if (!array_key_exists($matches[2], $outMap)) {
						if (!is_array($outMap[$matches[2]]))
							$outMap[$matches[2]] = array();
                                                $outMap[$matches[2]] = trim($matches[3]);
					}
                                }
                        }
                }             
        }
        // Sort the new reference map.
        uksort($outMap,'strnatcasecmp');

	// Do NOT write an empty references.config file, just
	// exit instead.
	if (empty($outMap))
		return false;

        // Format and write it to the supplied output file.
        $format = "config reference: %-12s %s\n";
        foreach ($outMap as $key=>$value)
                $outMap[$key] = sprintf($format, $key, $value);
        @file_put_contents($cfg_out, array_values($outMap));
	return true;
}

function suricata_merge_classification_configs($cfg_in, $cfg_out) {

	/************************************************************/
	/* This function takes a list of "classification.config"    */
	/* files in the $cfg_in array and merges them into a        */
	/* single file specified by $cfg_out.  The merging is done  */
	/* so no duplication of lines occurs in the output file.    */
	/************************************************************/

        $outMap = array();
        foreach ($cfg_in as $file) {
	     if (!file_exists($file))
			continue;
                $in = file($file, FILE_SKIP_EMPTY_LINES);
                foreach ($in as $line) {
                        if (preg_match('/(.*:)(\s*.*),(.*),(.*)/', $line, $matches)) {
                                /* Skip comment lines  */
                                if (preg_match('/^\s*#/', $line))
                                        continue;
                                if (!empty($matches[2]) && !empty($matches[3]) && !empty($matches[4])) {
                                        $matches[2] = trim($matches[2]);
                                        if (!array_key_exists($matches[2], $outMap)) {
						if (!is_array($outMap[$matches[2]]))
							$outMap[$matches[2]] = array();
                                                $outMap[$matches[2]] = trim($matches[3]) . "," . trim($matches[4]);
					}
                                }
                        }
                }             
        }
        // Sort the new classification map.
        uksort($outMap,'strnatcasecmp');

	// Do NOT write an empty classification.config file, just
	// exit instead.
	if (empty($outMap))
		return false;

        // Format and write it to the supplied output file.
        $format = "config classification: %s,%s\n";
        foreach ($outMap as $key=>$value)
                $outMap[$key] = sprintf($format, $key, $value);
        @file_put_contents($cfg_out, array_values($outMap));
	return true;
}

function suricata_load_rules_map($rules_path) {

	/***************************************************************/
	/* This function loads and returns an array with all the rules */
	/* found in the *.rules files in the passed rules path.        */
	/*                                                             */
	/* $rules_path can be:                                         */
	/*      a directory (assumed to contain *.rules files)         */
	/*      a filename (identifying a specific *.rules file)       */
	/*      an array of filenames (identifying *.rules files)      */
	/***************************************************************/

	$map_ref = array();
	$rule_files = array();

	if (empty($rules_path))
		return $map_ref;

	/***************************************************************
	 * Read all the rules into the map array.
	 * The structure of the map array is:
	 *
	 *  map[gid][sid]['rule']['category']['disabled']['flowbits']
	 *
	 *  where:
	 *   gid      = Generator ID from rule, or 1 if general text 
	 *              rule
	 *   sid      = Signature ID from rule
         *   rule     = Complete rule text
	 *   category = File name of file containing the rule
	 *   action   = alert, drop, reject or pass
	 *   disabled = 1 if rule is disabled (commented out), 0 if 
	 *              rule is enabled
	 *   flowbits = Array of applicable flowbits if rule contains 
	 *              flowbits options
	 ***************************************************************/

	// First check if we were passed a directory, a single file
	// or an array of filenames to read. Set our $rule_files
	// variable accordingly. If we can't figure it out, return
	// an empty rules map array.
	if (is_string($rules_path)) {
		if (is_dir($rules_path))
			$rule_files = glob($rules_path . "*.rules");
		elseif (is_file($rules_path))
			$rule_files = (array)$rules_path;
	}
	elseif (is_array($rules_path))
		$rule_files = $rules_path;
	else
		return $map_ref;

        // Read the rule files into an array, then iterate the list
	// to process the rules from the files one-by-one.
	foreach ($rule_files as $file) {

		// Don't process files with "deleted" in the filename.
		if (stristr($file, "deleted"))
			continue;

		// Read the file contents into an array, skipping
		// missing files.
		if (!file_exists($file))
			continue;

		$rules_array = file($file, FILE_SKIP_EMPTY_LINES);
		$record = "";
		$b_Multiline = false;

		// Read and process each line from the rules in the
		// current file into an array.
		foreach ($rules_array as $rule) {

			// Skip any lines that may be just spaces.
			if (trim($rule, " \n") == "")
				continue;

			// Skip any non-rule lines unless we're in
			// multiline mode.
			if (!preg_match('/^\s*#*\s*(alert|drop|pass|reject)/i', $rule) && !$b_Multiline)
				continue;

			// Test for a multi-line rule; loop and reassemble
			// the pieces back into a single line.
			if (preg_match('/\\\\s*[\n]$/m', $rule)) {
				$rule = substr($rule, 0, strrpos($rule, '\\'));
				$record .= $rule;
				$b_Multiline = true;
				continue;
			}
			// If the last segment of a multiline rule, then
			// append it onto the previous parts to form a
			// single-line rule for further processing below.
			elseif (!preg_match('/\\\\s*[\n]$/m', $rule) && $b_Multiline) {
				$record .= $rule;
				$rule = $record;
			}

			// We have an actual single-line rule, or else a
			// re-assembled multiline rule that is now a
			// single-line rule, so store it in our rules map.

			// Get and test the SID.  If we don't find one,
			// ignore and skip this rule as it is invalid.
			$sid = suricata_get_sid($rule);
			if (empty($sid)) {
				$b_Multiline = false;
				$record = "";
				continue;
			}

			$gid = suricata_get_gid($rule);
			if (!is_array($map_ref[$gid]))
				$map_ref[$gid] = array();
			if (!is_array($map_ref[$gid][$sid]))
				$map_ref[$gid][$sid] = array();
			$map_ref[$gid][$sid]['rule'] = $rule;
			$map_ref[$gid][$sid]['category'] = basename($file, ".rules");

			// Grab the rule action
			$matches = array();
			if (preg_match('/^\s*#*\s*(alert|drop|pass|reject)/i', $rule, $matches))
				$map_ref[$gid][$sid]['action'] = $matches[1];
			else
				$map_ref[$gid][$sid]['action'] = "";

			// Determine if default state is "disabled"
			if (preg_match('/^\s*\#+/', $rule))
				$map_ref[$gid][$sid]['disabled'] = 1;
			else
				$map_ref[$gid][$sid]['disabled'] = 0;

			// Grab any associated flowbits from the rule.
			$map_ref[$gid][$sid]['flowbits'] = suricata_get_flowbits($rule);
			
			// Reset our local flag and record variables
			// for the next rule in the set.
			$b_Multiline = false;
			$record = "";
		}

		// Zero out our processing array and get the next file.
		unset($rules_array);
	}
	return $map_ref;
}

function suricata_get_gid($rule) {

	/****************************************************************/
	/* If a gid is defined, then return it, else default to "1" for */
	/* general text rules match.                                    */
	/****************************************************************/

	if (preg_match('/\bgid\s*:\s*(\d+)\s*;/i', $rule, $matches))
		return trim($matches[1]);
	else
		return "1";
}

function suricata_get_sid($rule) {

	/***************************************************************/
	/* If a sid is defined, then return it, else default to an     */
	/* empty value.                                                */
	/***************************************************************/

	if (preg_match('/\bsid\s*:\s*(\d+)\s*;/i', $rule, $matches))
		return trim($matches[1]);
	else
		return "";
}

function suricata_get_msg($rule) {

	/**************************************************************/
	/* Return the MSG section of the passed rule as a string.     */
	/**************************************************************/

	$msg = "";
	if (preg_match('/\bmsg\s*:\s*"(.+?)"\s*;/i', $rule, $matches))
		$msg = trim($matches[1]);
	return $msg;
}

function suricata_get_flowbits($rule) {

	/*************************************************************/
	/* This will pull out "flowbits:" options from the rule text */
	/* and return them in an array (minus the "flowbits:" part). */
	/*************************************************************/

	$flowbits = array();

	// Grab any "flowbits:set, setx, unset, isset or toggle" options first.
	// Examine flowbits targets for logical operators to capture all targets. 
	if (preg_match_all('/flowbits\b\s*:\s*(set|setx|unset|toggle|isset|isnotset)\s*,([^;]+)/i', $rule, $matches)) {
		$i = -1;
		while (++$i < count($matches[1])) {
			$action = trim($matches[1][$i]);
			$target = preg_split('/[&|]/', $matches[2][$i]);
			foreach ($target as $t)
				$flowbits[] = "{$action}," . trim($t);
		}
	}

	// Include the "flowbits:noalert or reset" options, if present.
	if (preg_match_all('/flowbits\b\s*:\s*(noalert|reset)\b/i', $rule, $matches)) {
		$i = -1;
		while (++$i < count($matches[1])) {
			$flowbits[] = trim($matches[1][$i]);
		}
	}

	return $flowbits;
}

function suricata_get_checked_flowbits($rules_map) {

	/*************************************************************/
	/* This function checks all the currently enabled rules to   */
	/* find any checked flowbits, and returns the checked        */
	/* flowbit names in an array.                                */
	/*************************************************************/

	$checked_flowbits = array();
	foreach ($rules_map as $rulem) {
		if (!is_array($rulem))
			continue;
		foreach ($rulem as $rulem2) {
			if (!is_array($rulem2))
				continue;
			if ($rulem2['disabled'] == 1)
				continue;
			if (empty($rulem2['flowbits']))
				continue;
			if (!is_array($rulem2['flowbits']))
				continue;
			foreach ($rulem2['flowbits'] as $flowbit) {
				if (empty($flowbit))
					continue;
				// If no comma in flowbits option, then  skip it.
				$pos = strpos($flowbit, ",");
				if ($pos === false)
					continue;
				$action = substr(strtolower($flowbit), 0, $pos);
				if ($action == "isset" || $action == "isnotset") {
					$target = preg_split('/[&|]/', substr($flowbit, $pos + 1));
					foreach ($target as $t)
						if (!empty($t) && !isset($checked_flowbits[$t])) {
							if (!is_array($checked_flowbits[$t]))
								$checked_flowbits[$t] = array();
							$checked_flowbits[$t] = $action;
						}
				}
			}
		}
	}
	unset($rulem, $rulem2);
	return $checked_flowbits;
}

function suricata_get_set_flowbits($rules_map) {

	/*********************************************************/
	/* This function checks all the currently enabled rules  */
	/* to find any set flowbits, and returns the flowbit     */
	/* names in an array.                                    */
	/*********************************************************/

	$set_flowbits = array();
	foreach ($rules_map as $rulem) {
		if (!is_array($rulem))
			continue;
		foreach ($rulem as $rulem2) {
			if ($rulem2['disabled'] == 1)
				continue;
			if (empty($rulem2['flowbits']))
				continue;
			if (!is_array($rulem2['flowbits']))
				continue;
			foreach ($rulem2['flowbits'] as $flowbit) {
				if (empty($flowbit))
					continue;
				/* If no comma in flowbits option, then  skip it. */
				$pos = strpos($flowbit, ",");
				if ($pos === false)
					continue;
				$action = substr(strtolower($flowbit), 0, $pos);
				if ($action == "set" || $action == "toggle" || $action == "setx") {
					$target = preg_split('/[&|]/', substr($flowbit, $pos + 1));
					foreach ($target as $t)
						if (!empty($t) && !isset($set_flowbits[$t])) {
							if (!is_array($set_flowbits[$t]))
								$set_flowbits[$t] = array();
							$set_flowbits[$t] = $action;
						}
				}
			}
		}
	}
	unset($rulem, $rulem2);
	return $set_flowbits;
}

function suricata_find_flowbit_required_rules($rules, $unchecked_flowbits) {

	/********************************************************/
	/* This function finds all rules that must be enabled   */
	/* in order to satisfy the "checked flowbits" used by   */
	/* the currently enabled rules.  It returns the list    */
	/* of required rules in an array.                       */
	/********************************************************/

	$required_flowbits_rules = array();
	foreach ($rules as $k1 => $rule) {
		if (!is_array($rule))
			continue;
		foreach ($rule as $k2 => $rule2) {
			if (empty($rule2['flowbits']))
				continue;
			if (!is_array($rule2['flowbits']))
				continue;
			foreach ($rule2['flowbits'] as $flowbit) {
				if (empty($flowbit))
					continue;
				$action = substr($flowbit, 0, strpos($flowbit, ","));
				if (!strcasecmp(substr($action, 0, 3), "set")) {
					$tmp = substr($flowbit, strpos($flowbit, ",") +1 );
					if (!empty($tmp) && isset($unchecked_flowbits[$tmp])) {
						if (!is_array($required_flowbits_rules[$k1]))
							$required_flowbits_rules[$k1] = array();
						if (!is_array($required_flowbits_rules[$k1][$k2]))
							$required_flowbits_rules[$k1][$k2] = array();
						$required_flowbits_rules[$k1][$k2]['category'] = $rule2['category'];
						if ($rule2['disabled'] == 0)
							// If not disabled, just return the rule text "as is"
							$required_flowbits_rules[$k1][$k2]['rule'] = ltrim($rule2['rule']);
						else {
							// If rule is disabled, remove leading '#' to enable it
							$required_flowbits_rules[$k1][$k2]['rule'] = ltrim(substr($rule2['rule'], strpos($rule2['rule'], "#") + 1));
							$required_flowbits_rules[$k1][$k2]['disabled'] = 0;
						}
					}
				}
			}
		}
	}
	unset($rule, $rule2);

	return $required_flowbits_rules;
}

function suricata_resolve_flowbits($rules, $active_rules) {

	/******************************************************/
	/* This function auto-resolves flowbit requirements   */
	/* by finding all checked flowbits in the currently   */
	/* enabled rules, and then making sure all the "set"  */
	/* flowbit rules for those "checked" flowbits are     */
	/* enabled.  For any that are not enabled, they are   */
	/* copied to an array, enabled, and returned.         */
	/*                                                    */
	/* $active_rules --> Rules Map array containing       */
	/*                   the current rules for the        */
	/*                   interface to resolve flowbit     */
	/*                   dependencies for.                */
	/*                                                    */
	/*        $rules --> Rules Map array containing       */
	/*                   all the available rules.         */
	/******************************************************/

	$suricatadir = SURICATADIR;

	// Check $rules array to be sure it is filled.
	if (empty($rules)) {
		log_error(gettext("[Suricata] WARNING: Flowbit resolution not done - no rules in {$suricatadir}rules/ ..."));
		return array();
	}

	// First, find all the "checked" and "set" flowbits.
	$checked_flowbits = suricata_get_checked_flowbits($active_rules);
	$set_flowbits = suricata_get_set_flowbits($active_rules);

	// Next find any "checked" flowbits without matching
	// "set" flowbit rules in the enabled rule set.
	$delta_flowbits = array_diff_key($checked_flowbits, $set_flowbits);

	// Cleanup and release the memory we no longer need.
	unset($checked_flowbits);
	unset($set_flowbits);

	// Now find all the needed "set flowbit" rules from
	// the master list of all rules.
	$required_rules = suricata_find_flowbit_required_rules($rules, $delta_flowbits);

	// Cleanup and release memory we no longer need.
	unset($delta_flowbits);

	return $required_rules;
}

function suricata_write_flowbit_rules_file($flowbit_rules, $rule_file) {

	/************************************************/
	/* This function takes an array of rules in the */
	/* rules_map format and writes them to the file */
	/* given.                                       */
	/*                                              */
	/* $flowbit_rules --> array of flowbit-required */
	/*                    rules.                    */
	/*                                              */
	/*     $rule_file --> filename to write the     */
	/*                    flowbit-required rules    */
	/*                    to.                       */
	/************************************************/

	$flowbit_rules_file = FLOWBITS_FILENAME;

	// See if we were passed a directory or full
	// filename to write the rules to, and adjust
	// the destination argument accordingly.
	if (is_dir($rule_file))
		$rule_file = rtrim($rule_file, '/')."/{$flowbit_rules_file}";

	if (empty($flowbit_rules)) {
		@file_put_contents($rule_file, "");
		return;
	}

	$fp = fopen($rule_file, "w");
	if ($fp) {
		@fwrite($fp, "# These rules set flowbits checked by your other enabled rules.  If the\n");
		@fwrite($fp, "# dependent flowbits are not set, then some of your chosen rules may\n");
		@fwrite($fp, "# not fire.  Enabling all rules that set these dependent flowbits ensures\n");
		@fwrite($fp, "# your chosen rules fire as intended.\n#\n"); 
		@fwrite($fp, "# If you wish to prevent alerts from any of these rules, add the GID:SID\n");
		@fwrite($fp, "# of the rule to the Suppression List for the interface.\n");
		foreach ($flowbit_rules as $k1 => $rule) {
			foreach ($rule as $k2 => $rule2) {
				@fwrite($fp, "\n# Category: {$rule2['category']}");
				@fwrite($fp, "   GID:{$k1}  SID:{$k2}\n");
				@fwrite($fp, $rule2['rule']);
			}
		}
		fclose($fp);
	}
}

function suricata_load_vrt_policy($policy, $all_rules=null) {

	/************************************************/
	/* This function returns an array of all rules  */
	/* marked with the passed in $policy metadata.  */
	/*                                              */
	/*    $policy --> desired VRT security policy   */
	/*                  1.  connectivity            */
	/*                  2.  balanced                */
	/*                  3.  security                */
	/*                                              */
	/* $all_rules --> optional Rules Map array of   */
	/*                rules to scan for policy.     */
	/*                If not provided, then an      */
	/*                array will be created.        */
	/************************************************/

	$suricatadir = SURICATADIR;
	$vrt_policy_rules = array();

	// Load a map of all the VRT rules if we were
	// not passed a pre-loaded one to use.
	if (is_null($all_rules)) {
		/* Since only Snort VRT rules have IPS Policy metadata, */
		/* limit our search to just those files.                */
		$suricata_file_pattern = VRT_FILE_PREFIX . "*.rules";
		$suricata_vrt_files = glob("{$suricatadir}rules/{$suricata_file_pattern}");
		$all_rules = suricata_load_rules_map($suricata_vrt_files);
	}

	// Now walk the rules list and find all those that are
	// defined as active for the chosen security policy.
	foreach ($all_rules as $k1 => $arulem) {
		foreach ($arulem as $k2 => $arulem2) {
			if (strripos($arulem2['rule'], "policy {$policy}-ips") !== false) {
				if (!preg_match('/flowbits\s*:\s*noalert/i', $arulem2['rule'])) {
					if (!is_array($vrt_policy_rules[$k1]))
						$vrt_policy_rules[$k1] = array();
					if (!is_array($vrt_policy_rules[$k1][$k2]))
						$vrt_policy_rules[$k1][$k2] = array();
					$vrt_policy_rules[$k1][$k2] = $arulem2;

					// Enable the policy rule if disabled
					if ($arulem2['disabled'] == 1) {
						$vrt_policy_rules[$k1][$k2]['rule'] = ltrim(substr($arulem2['rule'], strpos($arulem2['rule'], "#") + 1));
						$vrt_policy_rules[$k1][$k2]['disabled'] = 0;
					}
				}
			}
		}
	}

	// Release memory we no longer need.
	unset($arulem, $arulem2);

	// Return all the rules that match the policy.
	return $vrt_policy_rules;
}

function suricata_load_sid_mods($sids) {

	/*****************************************/
	/* This function parses the string of    */
	/* SID values in $sids and returns an    */
	/* array with the SID as the key and     */
	/* value.  The SID values in $sids are   */
	/* assumed to be delimited by "||".      */
	/*                                       */
	/* $sids ==> string of SID values from   */
	/*           saved config file.          */
	/*                                       */
	/* Returns ==> a multidimensional array  */
	/*             with GID and SID as the   */
	/*             keys ($result[GID][SID])  */
	/*****************************************/

	$result = array();
	if (empty($sids))
		return $result;
	$tmp = explode("||", $sids);
	foreach ($tmp as $v) {
		if (preg_match('/(\d+)\s*:\s*(\d+)/', $v, $match)) {
			if (!is_array($result[$match[1]]))
				$result[$match[1]] = array();
			$result[$match[1]][$match[2]] = "{$match[1]}:{$match[2]}";
		}
	}
	unset($tmp);

	return $result;
}

function suricata_modify_sids(&$rule_map, $suricatacfg) {

	/*****************************************/
	/* This function modifies the rules in   */
	/* the passed rules_map array based on   */
	/* values in the enablesid/disablesid    */
	/* configuration parameters.             */
	/*                                       */
	/*  $rule_map = array of current rules   */
	/*  $suricatacfg = config settings       */
	/*****************************************/

	if (!isset($suricatacfg['rule_sid_on']) && 
	    !isset($suricatacfg['rule_sid_off']))
		return;

	// Load up our enablesid and disablesid
	// arrays with lists of modified SIDs.
	$enablesid = suricata_load_sid_mods($suricatacfg['rule_sid_on'], "enablesid");
	$disablesid = suricata_load_sid_mods($suricatacfg['rule_sid_off'], "disablesid");

	/* Turn on any rules that need to be     */
	/* forced "on" with enablesid mods.      */
	if (!empty($enablesid)) {
		foreach ($rule_map as $k1 => $rulem) {
			foreach ($rulem as $k2 => $v) {
				if (isset($enablesid[$k1][$k2]) && $v['disabled'] == 1) {
					$rule_map[$k1][$k2]['rule'] = ltrim($v['rule'], " \t#");
					$rule_map[$k1][$k2]['disabled'] = 0;
				}
			}
		}
	}

	/* Turn off any rules that need to be    */
	/* forced "off" with disablesid mods.    */
	if (!empty($disablesid)) {
		foreach ($rule_map as $k1 => $rulem) {
			foreach ($rulem as $k2 => $v) {
				if (isset($disablesid[$k1][$k2]) && $v['disabled'] == 0) {
					$rule_map[$k1][$k2]['rule'] = "# " . $v['rule'];
					$rule_map[$k1][$k2]['disabled'] = 1;
				}
			}
		}
	}
	unset($enablesid, $disablesid);
}

function suricata_prepare_rule_files($suricatacfg, $suricatacfgdir) {

	/***********************************************************/
	/* This function builds a new set of enforcing rules for   */
	/* Suricata and writes them to disk.                       */
	/*                                                         */
	/*    $suricatacfg --> pointer to applicable section of    */
	/*                  config.xml containing settings for     */
	/*                  the interface.                         */
	/*                                                         */
	/* $suricatacfgdir --> pointer to physical directory on    */
	/*                  disk where Suricata configuration is   */
	/*                  to be written.                         */
	/***********************************************************/

	global $rebuild_rules;

	$suricatadir = SURICATADIR;
	$flowbit_rules_file = FLOWBITS_FILENAME;
	$suricata_enforcing_rules_file = ENFORCING_RULES_FILENAME;
	$no_rules_defined = true;

	// If there is no reason to rebuild the rules, exit to save time.
	if (!$rebuild_rules)
		return;

	// Log a message for rules rebuild in progress
	log_error(gettext("[Suricata] Updating rules configuration for: " . convert_friendly_interface_to_friendly_descr($suricatacfg['interface']) . " ..."));

	// Only rebuild rules if some are selected or an IPS Policy is enabled
	if (!empty($suricatacfg['rulesets']) || $suricatacfg['ips_policy_enable'] == 'on') {
		$enabled_rules = array();
		$enabled_files = array();
		$all_rules = array();
		$no_rules_defined = false;

		// Load up all the rules into a Rules Map array.
		$all_rules = suricata_load_rules_map("{$suricatadir}rules/");

		// Create an array with the filenames of the enabled
		// rule category files if we have any.
		if (!empty($suricatacfg['rulesets'])) {
			foreach (explode("||", $suricatacfg['rulesets']) as $file){
					$category = basename($file, ".rules");
					if (!is_array($enabled_files[$category]))
						$enabled_files[$category] = array();
					$enabled_files[$category] = $file;
			}

			/****************************************************/
			/* Walk the ALL_RULES map array and copy the rules  */
			/* matching our selected file categories to the     */
			/* ENABLED_RULES map array.                         */
			/****************************************************/
			foreach ($all_rules as $k1 => $rulem) {
				foreach ($rulem as $k2 => $v) {
					if (isset($enabled_files[$v['category']])) {
						if (!is_array($enabled_rules[$k1]))
							$enabled_rules[$k1] = array();
						if (!is_array($enabled_rules[$k1][$k2]))
							$enabled_rules[$k1][$k2] = array();
						$enabled_rules[$k1][$k2]['rule'] = $v['rule'];
						$enabled_rules[$k1][$k2]['category'] = $v['category'];
						$enabled_rules[$k1][$k2]['disabled'] = $v['disabled'];
						$enabled_rules[$k1][$k2]['flowbits'] = $v['flowbits'];
					}
				}
			}

			// Release memory we no longer need.
			unset($enabled_files, $rulem, $v);
		}

		// Check if a pre-defined Snort VRT policy is selected. If so,
		// add all the VRT policy rules to our enforcing rule set.
		if (!empty($suricatacfg['ips_policy'])) {
			$policy_rules = suricata_load_vrt_policy($suricatacfg['ips_policy'], $all_rules);
			foreach ($policy_rules as $k1 => $policy) {
				foreach ($policy as $k2 => $p) {
					if (!is_array($enabled_rules[$k1]))
						$enabled_rules[$k1] = array();
					if (!is_array($enabled_rules[$k1][$k2]))
						$enabled_rules[$k1][$k2] = array();
					$enabled_rules[$k1][$k2]['rule'] = $p['rule'];
					$enabled_rules[$k1][$k2]['category'] = $p['category'];
					$enabled_rules[$k1][$k2]['disabled'] = $p['disabled'];
					$enabled_rules[$k1][$k2]['flowbits'] = $p['flowbits'];
				}
			}
			unset($policy_rules, $policy, $p);
		}

		// Process any enablesid or disablesid modifications for the selected rules.
		suricata_modify_sids($enabled_rules, $suricatacfg);

		// Write the enforcing rules file to the Suricata interface's "rules" directory.
		suricata_write_enforcing_rules_file($enabled_rules, "{$suricatacfgdir}/rules/{$suricata_enforcing_rules_file}");

		// If auto-flowbit resolution is enabled, generate the dependent flowbits rules file.
		if ($suricatacfg['autoflowbitrules'] == 'on') {
			log_error('[Suricata] Enabling any flowbit-required rules for: ' . convert_friendly_interface_to_friendly_descr($suricatacfg['interface']) . '...');
			$fbits = suricata_resolve_flowbits($all_rules, $enabled_rules);

			// Check for and disable any flowbit-required rules the user has
			// manually forced to a disabled state.
			suricata_modify_sids($fbits, $suricatacfg);
			suricata_write_flowbit_rules_file($fbits, "{$suricatacfgdir}/rules/{$flowbit_rules_file}");
			unset($fbits);
		} else
			// Just put an empty file to always have the file present
			suricata_write_flowbit_rules_file(array(), "{$suricatacfgdir}/rules/{$flowbit_rules_file}");
	} else {
		suricata_write_enforcing_rules_file(array(), "{$suricatacfgdir}/rules/{$suricata_enforcing_rules_file}");
		suricata_write_flowbit_rules_file(array(), "{$suricatacfgdir}/rules/{$flowbit_rules_file}");
	}

	if (!empty($suricatacfg['customrules'])) {
		@file_put_contents("{$suricatacfgdir}/rules/custom.rules", base64_decode($suricatacfg['customrules']));
		$no_rules_defined = false;
	}
	else
		@file_put_contents("{$suricatacfgdir}/rules/custom.rules", "");

	// Log a warning if the interface has no rules defined or enabled
	if ($no_rules_defined)
		log_error(gettext("[Suricata] Warning - no text rules selected for: " . convert_friendly_interface_to_friendly_descr($suricatacfg['interface']) . " ..."));

	// Build a new sid-msg.map file from the enabled
	// rules and copy it to the interface directory.
	log_error(gettext("[Suricata] Building new sig-msg.map file for " . convert_friendly_interface_to_friendly_descr($suricatacfg['interface']) . "..."));
	suricata_build_sid_msg_map("{$suricatacfgdir}/rules/", "{$suricatacfgdir}/sid-msg.map");
}


function suricata_write_enforcing_rules_file($rule_map, $rule_path) {

	/************************************************/
	/* This function takes a rules map array of     */
	/* the rules chosen for the active rule set     */
	/* and writes them out to the passed path.      */
	/*                                              */
	/*  $rule_map --> Rules Map array of rules to   */
	/*                write to disk.                */
	/*                                              */
	/* $rule_path --> filename or directory where   */
	/*                rules file will be written.   */
	/************************************************/

	$rule_file = "/" . ENFORCING_RULES_FILENAME;

	// See if we were passed a directory or full
	// filename to write the rules to, and adjust
	// the destination argument accordingly.
	if (is_dir($rule_path))
		$rule_file = rtrim($rule_path, '/').$rule_file;
	else
		$rule_file = $rule_path;

	// If the $rule_map array is empty, then exit.
	if (empty($rule_map)) {
		file_put_contents($rule_file, "");
		return;
	}

	$fp = fopen($rule_file, "w");
	if ($fp) {
		@fwrite($fp, "# These rules are your current set of enforced rules for the protected\n");
		@fwrite($fp, "# interface.  This list was compiled from the categories selected on the\n");
		@fwrite($fp, "# CATEGORIES tab of the Suricata configuration for the interface and/or any\n");
		@fwrite($fp, "# chosen Snort VRT pre-defined IPS Policy.\n#\n");
		@fwrite($fp, "# Any enablesid or disablesid customizations you made have been applied\n");
		@fwrite($fp, "# to the rules in this file.\n\n");
		foreach ($rule_map as $rulem) {
			foreach ($rulem as $rulem2) {
				/* No reason to write disabled rules to enforcing file, so skip them. */
				if ($rulem2['disabled'] == 1)
					continue;
				@fwrite($fp, $rulem2['rule']);
			}
		}
		fclose($fp);
	}
}

function suricata_create_rc() {

	/************************************************************/
	/* This function builds the /usr/local/etc/rc.d/suricata.sh */
	/* shell script for starting and stopping Suricata.  The    */
	/* script is rebuilt on each package sync operation and     */
	/* after any changes to suricata.conf saved in the GUI.     */
	/************************************************************/

	global $config, $g;

	$suricatadir = SURICATADIR;
	$suricatalogdir = SURICATALOGDIR;
	$rcdir = RCFILEPREFIX;	

	// If no interfaces are configured for Suricata, exit
	if (!is_array($config['installedpackages']['suricata']['rule']))
		return;
	$suricataconf = $config['installedpackages']['suricata']['rule'];
	if (empty($suricataconf))
		return;

	// At least one interface is configured, so OK
	$start_suricata_iface_start = array();
	$start_suricata_iface_stop = array();

	// Loop thru each configured interface and build
	// the shell script.
	foreach ($suricataconf as $value) {
		// Skip disabled Suricata interfaces
		if ($value['enable'] <> 'on')
			continue;
		$suricata_uuid = $value['uuid'];
		$if_real = get_real_interface($value['interface']);

		$start_barnyard = <<<EOE

	if [ ! -f {$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid ]; then
		pid=`/bin/pgrep -fn "barnyard2 -r {$suricata_uuid} "`
	else
		pid=`/bin/pgrep -F {$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid`
	fi

	if [ ! -z \$pid ]; then
		/usr/bin/logger -p daemon.info -i -t SuricataStartup "Barnyard2 SOFT RESTART for {$value['descr']}({$suricata_uuid}_{$if_real})..."
		/bin/pkill -HUP \$pid
	else
		/usr/bin/logger -p daemon.info -i -t SuricataStartup "Barnyard2 START for {$value['descr']}({$suricata_uuid}_{$if_real})..."
		/usr/local/bin/barnyard2 -r {$suricata_uuid} -f unified2.alert --pid-path {$g['varrun_path']} --nolock-pidfile -c {$suricatadir}suricata_{$suricata_uuid}_{$if_real}/barnyard2.conf -d {$suricatalogdir}suricata_{$if_real}{$suricata_uuid} -D -q
	fi

EOE;
		$stop_barnyard2 = <<<EOE

	if [ -f {$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid ]; then
		/usr/bin/logger -p daemon.info -i -t SuricataStartup "Barnyard2 STOP for {$value['descr']}({$suricata_uuid}_{$if_real})..."
		pid=`/bin/pgrep -F {$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid`
                /bin/pkill -TERM -F {$g['varrun_path']}/barnyard2_{$if_real}{$suricata_uuid}.pid
		time=0 timeout=30
		while /bin/kill -TERM \$pid 2>/dev/null; do
			sleep 1
			time=\$((time+1))
			if [ \$time -gt \$timeout ]; then
				break
			fi
		done
		if [ -f /var/run/barnyard2_{$if_real}{$suricata_uuid}.pid ]; then
			/bin/rm /var/run/barnyard2_{$if_real}{$suricata_uuid}.pid
		fi
	else
		pid=`/bin/pgrep -fn "barnyard2 -r {$suricata_uuid} "`
		if [ ! -z \$pid ]; then
			/bin/pkill -TERM -fn "barnyard2 -r {$suricata_uuid} "
			time=0 timeout=30
			while /bin/kill -TERM \$pid 2>/dev/null; do
				sleep 1
				time=\$((time+1))
				if [ \$time -gt \$timeout ]; then
					break
				fi
			done
		fi
        fi

EOE;
		if ($value['barnyard_enable'] == 'on')
			$start_barnyard2 = $start_barnyard;
		else
			$start_barnyard2 = $stop_barnyard2;

		$start_suricata_iface_start[] = <<<EOE

###### For Each Iface
	# Start suricata and barnyard2
	if [ ! -f {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid ]; then
		pid=`/bin/pgrep -fn "suricata -i {$if_real} "`
	else
		pid=`/bin/pgrep -F {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid`
	fi

	if [ ! -z \$pid ]; then
		/usr/bin/logger -p daemon.info -i -t SuricataStartup "Suricata SOFT RESTART for {$value['descr']}({$suricata_uuid}_{$if_real})..."
		/bin/pkill -USR2 \$pid
	else
		/usr/bin/logger -p daemon.info -i -t SuricataStartup "Suricata START for {$value['descr']}({$suricata_uuid}_{$if_real})..."
		/usr/local/bin/suricata -i {$if_real} -D -c {$suricatadir}suricata_{$suricata_uuid}_{$if_real}/suricata.yaml --pidfile {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid
	fi

	sleep 2
	{$start_barnyard2}

EOE;

		$start_suricata_iface_stop[] = <<<EOE

	if [ -f {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid ]; then
		pid=`/bin/pgrep -F {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid`
		/usr/bin/logger -p daemon.info -i -t SuricataStartup "Suricata STOP for {$value['descr']}({$suricata_uuid}_{$if_real})..."
		/bin/pkill -TERM -F {$g['varrun_path']}/suricata_{$if_real}{$suricata_uuid}.pid
		time=0 timeout=30
		while /bin/kill -TERM \$pid 2>/dev/null; do
			sleep 1
			time=\$((time+1))
			if [ \$time -gt \$timeout ]; then
				break
			fi
		done
		if [ -f /var/run/suricata_{$if_real}{$suricata_uuid}.pid ]; then
			/bin/rm /var/run/suricata_{$if_real}{$suricata_uuid}.pid
		fi
	else
		pid=`/bin/pgrep -fn "suricata -i {$if_real} "`
		if [ ! -z \$pid ]; then
			/usr/bin/logger -p daemon.info -i -t SuricataStartup "Suricata STOP for {$value['descr']}({$suricata_uuid}_{$if_real})..."
			/bin/pkill -TERM -fn "suricata -i {$if_real} "
			time=0 timeout=30
			while /bin/kill -TERM \$pid 2>/dev/null; do
				sleep 1
				time=\$((time+1))
				if [ \$time -gt \$timeout ]; then
					break
				fi
			done
		fi
	fi

	sleep 2
	{$stop_barnyard2}

EOE;
	}

	$rc_start = implode("\n", $start_suricata_iface_start);
	$rc_stop = implode("\n", $start_suricata_iface_stop);

	$suricata_sh_text = <<<EOD
#!/bin/sh
########
# This file was automatically generated
# by the pfSense service handler.
######## Start of main suricata.sh

rc_start() {
	{$rc_start}
}

rc_stop() {
	{$rc_stop}
}

case $1 in
	start)
		rc_start
		;;
	stop)
		rc_stop
		;;
	restart)
		rc_stop
		rc_start
		;;
esac

EOD;

	// Write out the suricata.sh script file
	@file_put_contents("{$rcdir}/suricata.sh", $suricata_sh_text);
	@chmod("{$rcdir}/suricata.sh", 0755);
	unset($suricata_sh_text);
}

function suricata_generate_barnyard2_conf($suricatacfg, $if_real) {
	global $config, $g;

	$suricata_uuid = $suricatacfg['uuid'];
	$suricatadir = SURICATADIR . "suricata_{$suricata_uuid}_{$if_real}";
	$suricatalogdir = SURICATALOGDIR . "suricata_{$if_real}{$suricata_uuid}";

	// Create required directories for barnyard2 if missing
	if (!is_dir("{$suricatalogdir}/barnyard2"))
		safe_mkdir("{$suricatalogdir}/barnyard2");
	if (!is_dir("{$suricatalogdir}/barnyard2/archive"))
		safe_mkdir("{$suricatalogdir}/barnyard2/archive");

	// Create the barnyard2 waldo file if missing
	if (!file_exists("{$suricatalogdir}/barnyard2/{$suricata_uuid}_{$if_real}.waldo")) {
		@touch("{$suricatalogdir}/barnyard2/{$suricata_uuid}_{$if_real}.waldo");
		mwexec("/bin/chmod 770 {$suricatalogdir}/barnyard2/{$suricata_uuid}_{$if_real}.waldo", true);
	}

	// If there is no gen-msg.map file present, create an
	// empty one so Barnyard2 will at least start.
	if (!file_exists("{$suricatadir}/gen-msg.map"))
		@file_put_contents("{$suricatadir}/gen-msg.map", "");

	if (!empty($suricatacfg['barnyard_sensor_name']))
		$suricatabarnyardlog_hostname_info_chk = $suricatacfg['barnyard_sensor_name'];
	else
		$suricatabarnyardlog_hostname_info_chk = php_uname("n");

	// Set general config parameters
	$gen_configs = "config quiet\nconfig daemon\nconfig decode_data_link\nconfig alert_with_interface_name\nconfig event_cache_size: 4096";
	if ($suricatacfg['barnyard_show_year'] == 'on')
		$gen_configs .= "\nconfig show_year";
	if ($suricatacfg['barnyard_obfuscate_ip'] == 'on')
		$gen_configs .= "\nconfig obfuscate";
	if ($suricatacfg['barnyard_dump_payload'] == 'on')
		$gen_configs .= "\nconfig dump_payload";
	if ($suricatacfg['barnyard_archive_enable'] == 'on')
		$gen_configs .= "\nconfig archivedir:          {$suricatalogdir}/barnyard2/archive";

	// Set output plugins
	$suricatabarnyardlog_output_plugins = "";
	if ($suricatacfg['barnyard_mysql_enable'] == 'on') {
		$by2_dbpwd = base64_decode($suricatacfg['barnyard_dbpwd']);
		$suricatabarnyardlog_output_plugins .= "# database: log to a MySQL DB\noutput database: alert, mysql, ";
		$suricatabarnyardlog_output_plugins .= "user={$suricatacfg['barnyard_dbuser']} password={$by2_dbpwd} ";
		$suricatabarnyardlog_output_plugins .= "dbname={$suricatacfg['barnyard_dbname']} host={$suricatacfg['barnyard_dbhost']}\n\n";
	}
	if ($suricatacfg['barnyard_syslog_enable'] == 'on') {
		$suricatabarnyardlog_output_plugins .= "# syslog_full: log to a syslog receiver\n";
		$suricatabarnyardlog_output_plugins .= "output alert_syslog_full: sensor_name {$suricatabarnyardlog_hostname_info_chk}, ";
		if ($suricatacfg['barnyard_syslog_local'] == 'on')
			$suricatabarnyardlog_output_plugins .= "local, log_facility LOG_AUTH, log_priority LOG_INFO\n\n";
		else {
			$suricatabarnyardlog_output_plugins .= "server {$suricatacfg['barnyard_syslog_rhost']}, protocol {$suricatacfg['barnyard_syslog_proto']}, ";
			$suricatabarnyardlog_output_plugins .= "port {$suricatacfg['barnyard_syslog_dport']}, operation_mode {$suricatacfg['barnyard_syslog_opmode']}, ";
			$suricatabarnyardlog_output_plugins .= "log_facility {$suricatacfg['barnyard_syslog_facility']}, log_priority {$suricatacfg['barnyard_syslog_priority']}\n\n";
		}
	}
	if ($suricatacfg['barnyard_bro_ids_enable'] == 'on') {
		$suricatabarnyardlog_output_plugins .= "# alert_bro: log to a Bro-IDS receiver\n";
		$suricatabarnyardlog_output_plugins .= "output alert_bro: {$suricatacfg['barnyard_bro_ids_rhost']}:{$suricatacfg['barnyard_bro_ids_dport']}\n";
	}

	// Trim leading and trailing newlines and spaces
	$suricatabarnyardlog_output_plugins = rtrim($suricatabarnyardlog_output_plugins, "\n");	

	// User pass-through arguments
	$suricatabarnyardlog_config_pass_thru = str_replace("\r", "", base64_decode($suricatacfg['barnconfigpassthru']));

	// Create the conf file as a text string
	$barnyard2_conf_text = <<<EOD

#   barnyard2.conf
#   barnyard2 can be found at http://www.securixlive.com/barnyard2/index.php
#

## General Barnyard2 settings ##
{$gen_configs}
config reference_file:	    {$suricatadir}/reference.config
config classification_file: {$suricatadir}/classification.config
config sid_file:	    {$suricatadir}/sid-msg.map
config gen_file:            {$suricatadir}/gen-msg.map
config hostname:            {$suricatabarnyardlog_hostname_info_chk}
config interface:           {$if_real}
config waldo_file:          {$suricatalogdir}/barnyard2/{$suricata_uuid}_{$if_real}.waldo
config logdir:              {$suricatalogdir}

## START user pass through ##
{$suricatabarnyardlog_config_pass_thru}
## END user pass through ##

## Setup input plugins ##
input unified2

## Setup output plugins ##
{$suricatabarnyardlog_output_plugins}

EOD;

	/* Write out barnyard2_conf text string to disk */
	@file_put_contents("{$suricatadir}/barnyard2.conf", $barnyard2_conf_text);
	unset($barnyard2_conf_text);
}

function suricata_generate_yaml($suricatacfg) {

	/************************************************************/
	/* This function generates the suricata.yaml configuration  */
	/* file for Suricata on the passed interface instance.  The */
	/* code uses two included files: one that contains most of  */
	/* the PHP code, and another that provides the template for */
	/* generating the configuration file.  Using two include    */
	/* files works around the "require_once()" caching issues   */
	/* that can prevent new changes in this code from being     */
	/* available during package installs on pfSense.            */
	/*                                                          */
	/* On Entry: suricatacfg --> Suricata instance info in      */
	/*                           the config.xml master config   */
	/*                           file.                          */
	/************************************************************/

	global $config, $g;

	$suricatadir = SURICATADIR;
	$suricatalogdir = SURICATALOGDIR;
	$flowbit_rules_file = FLOWBITS_FILENAME;
	$suricata_enforcing_rules_file = ENFORCING_RULES_FILENAME;
	$if_real = get_real_interface($suricatacfg['interface']);
	$suricata_uuid = $suricatacfg['uuid'];
	$suricatacfgdir = "{$suricatadir}suricata_{$suricata_uuid}_{$if_real}";

	conf_mount_rw();

	if (!is_array($config['installedpackages']['suricata']['rule']))
		return;

	// Pull in the PHP code that generates the suricata.yaml file
	// variables that will be substitued further down below.
	include("/usr/local/www/suricata/suricata_generate_yaml.php");

	// Pull in the boilerplate template for the suricata.yaml
	// configuration file.  The contents of the template along
	// with substituted variables is stored in $suricata_conf_text
	// (which is defined in the included file).
	include("/usr/local/pkg/suricata/suricata_yaml_template.inc");

	// Now write out the conf file using $suricata_conf_text contents
	@file_put_contents("{$suricatacfgdir}/suricata.yaml", $suricata_conf_text); 
	unset($suricata_conf_text);
	conf_mount_ro();
}

?>
