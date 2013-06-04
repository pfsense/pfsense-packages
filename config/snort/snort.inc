<?php
/*
 * snort.inc
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009-2010 Robert Zelaya
 * Copyright (C) 2011-2012 Ermal Luci
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

require_once("pfsense-utils.inc");
require_once("config.inc");
require_once("functions.inc");
require_once("service-utils.inc");
require_once("pkg-utils.inc");

// Needed on 2.0 because of filter_get_vpns_list()
require_once("filter.inc");

// Snort GUI needs some extra PHP memory space to manipulate large rules arrays
ini_set("memory_limit", "192M");

// Explicitly declare these as global so they work through function call includes 
global $snort_rules_file, $snort_version, $emerging_threats_version, $snort_rules_upd_log;
global $all_rules, $flowbit_rules_file, $snort_enforcing_rules_file, $rebuild_rules, $is_postinstall;
global $snort_community_rules_filename, $snort_community_rules_url, $emergingthreats_filename;

/* package version */
$snort_version = "2.9.4.1";
$pfSense_snort_version = "2.5.8";
$snort_package_version = "Snort {$snort_version} pkg v. {$pfSense_snort_version}";

// Define SNORTDIR and SNORTLIBDIR constants according to FreeBSD version (PBI support or no PBI)
if (floatval(php_uname("r")) >= 8.3) {
	exec("/usr/local/sbin/pbi_info | grep 'snort-{$snort_version}' | xargs /usr/local/sbin/pbi_info | awk '/Prefix/ {print $2}'",$pbidirarray);
	$snort_pbidir = "{$pbidirarray[0]}";
	/* In case this is an initial Snort install and pbi_info() above returned null, set a sane default value */
	if (empty($snort_pbidir))
		$snort_pbidir = "/usr/pbi/snort-" . php_uname("m");
	define("SNORTDIR", "{$snort_pbidir}/etc/snort");
	define("SNORTLIBDIR", "{$snort_pbidir}/lib/snort");
}
else {
	define("SNORTDIR", "/usr/local/etc/snort");
	define("SNORTLIBDIR", "/usr/local/lib/snort");
}

define("SNORTLOGDIR", "/var/log/snort");

/* Important file definitions */
$snort_rules_file = "snortrules-snapshot-2941.tar.gz";
$emerging_threats_version = "2.9.0";
$emergingthreats_filename = "emerging.rules.tar.gz";
$snort_community_rules_url = "https://s3.amazonaws.com/snort-org/www/rules/community/";
$snort_community_rules_filename = "community-rules.tar.gz";
$flowbit_rules_file = "flowbit-required.rules";
$snort_enforcing_rules_file = "snort.rules";
$snort_rules_upd_log = SNORTLOGDIR;
$snort_rules_upd_log .= "/snort_rules_update.log";

/* Rebuild Rules Flag -- if "on", rebuild enforcing rules and flowbit-rules files */
$rebuild_rules = "off";

/* Post-install Flag -- normally "false" except during post-install of package */
$is_postinstall = false;

if (!is_array($config['installedpackages']['snortglobal']))
	$config['installedpackages']['snortglobal'] = array();

function snort_get_blocked_ips() {
	$blocked_ips = "";
	exec('/sbin/pfctl -t snort2c -T show', $blocked_ips);
	$blocked_ips_array = array();
	if (!empty($blocked_ips)) {
		$blocked_ips_array = array();
		if (is_array($blocked_ips)) {
			foreach ($blocked_ips as $blocked_ip) {
				if (empty($blocked_ip))
					continue;
				$blocked_ips_array[] = trim($blocked_ip, " \n\t");
			}
		}
	}

	return $blocked_ips_array;
}

function snort_get_rule_part($source, $beginning, $ending, $start_pos) {

	$beginning_pos = strpos($source, $beginning, $start_pos);
	if (!$beginning_pos)
		return false;
	$middle_pos = $beginning_pos + strlen($beginning);
	$source = substr($source, $middle_pos);
	$ending_pos = strpos($source, $ending, 0);
	if (!$ending_pos)
		return false;
	return substr($source, 0, $ending_pos);
}

function snort_generate_id() {
	global $config;

	$snortglob = $config['installedpackages']['snortglobal']['rule'];
	while (true) {
		$snort_uuid = mt_rand(1, 65535);
		foreach ($snortglob as $value) {
			if ($value['uuid'] == $snort_uuid)
				continue 2;
		}
		break;
	}

	return $snort_uuid;
}

function snort_load_suppress_sigs($snortcfg) {

	global $config;

	/**********************************************************/
	/* This function loads the GEN_ID and SIG_ID for all the  */
	/* suppressed alert entries from the Suppression List of  */
	/* the passed Snort interface.  The results are returned  */
	/* in an array with GEN_ID and SIG_ID as the keys.        */
	/**********************************************************/

	$suppress = array();

	if (!is_array($config['installedpackages']['snortglobal']))
		return;
	if (!is_array($config['installedpackages']['snortglobal']['suppress']))
		return;
	if (!is_array($config['installedpackages']['snortglobal']['suppress']['item']))
		return;
	$a_suppress = $config['installedpackages']['snortglobal']['suppress']['item'];

	foreach ($a_suppress as $a_id => $alist) {
		if ($alist['name'] == $snortcfg['suppresslistname']) {
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
					if (preg_match('/gen_id\b\s*(\d+),\s*sig_id\b\s*(\d+)/i', $line, $matches)) {
						$genid = $matches[1];
						$sigid = $matches[2];
						if (!empty($genid) && !empty($sigid))
							$suppress[$genid][$sigid] = "suppress";
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

/* func builds custom white lists */
function snort_find_list($find_name, $type = 'whitelist') {
	global $config;

	$snortglob = $config['installedpackages']['snortglobal'];
	if (!is_array($snortglob[$type]))
		return "";
	if (!is_array($snortglob[$type]['item']))
		return "";

	foreach ($snortglob[$type]['item'] as $value) {
		if ($value['name'] == $find_name)
			return $value;
	}

	return array();
}

/* func builds custom whitelists and the HOME_NET variable */
function snort_build_list($snortcfg, $listname = "", $whitelist = false) {
	global $config, $g;
	global $aliastable, $filterdns;
	$home_net = array();

	if ($listname == 'default' || empty($listname)) {
		$localnet = 'yes'; $wanip = 'yes'; $wangw = 'yes'; $wandns = 'yes'; $vips = 'yes'; $vpns = 'yes';
	} else {
                $list = snort_find_list($listname);
                if (empty($list))
                        return $list;
		$localnet = $list['localnets'];
		$wanip = $list['wanips'];
		$wangw = $list['wangateips'];
		$wandns = $list['wandnsips'];
		$vips = $list['vips'];
		$vpns = $list['vpnips'];
		if (!empty($list['address']) && is_alias($list['address'])) {
			$home_net = explode(" ", trim(filter_expand_alias($list['address'])));
		}
	}

	/* Always add loopback to HOME_NET and whitelist (ftphelper) */
	if (!in_array("127.0.0.1", $home_net))
		$home_net[] = "127.0.0.1";

	/********************************************************************/
	/* Always put the interface running Snort in HOME_NET and whitelist */
	/* unless it's the WAN.  WAN options are handled further down.      */
	/********************************************************************/
	$snortip = get_interface_ip($snortcfg['interface']);
	if (is_ipaddr($snortip)) {
		if ($snortcfg['interface'] <> "wan") {
			$sn = get_interface_subnet($snortcfg['interface']);
			$ip = gen_subnet($snortip, $sn) . "/{$sn}";
			if (!in_array($ip, $home_net))
				$home_net[] = $ip;
		}
	}

	if (function_exists('get_interface_ipv6')) {
		$snortip = get_interface_ipv6($snortcfg['interface']);
		if (is_ipaddrv6($snortip)) {
			if ($snortcfg['interface'] <> "wan") {
				$sn = get_interface_subnetv6($snortcfg['interface']);
				$ip = gen_subnetv6($snortip, $sn). "/{$sn}";
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
		}
	}

	if ($localnet == 'yes') {		
		/*************************************************************************/
		/*  Iterate through the interface list and write out whitelist items and */
		/*  also compile a HOME_NET list of all the local interfaces for snort.  */
		/*  Skip the WAN interface as we do not typically want that whole subnet */
		/*  whitelisted (just the interface IP itself).                          */
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
			if (function_exists("get_interface_ipv6")) {
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
	}

	if ($wanip == 'yes') {
		$ip = get_interface_ip("wan");
		if (is_ipaddr($ip)) {
			if (!in_array($ip, $home_net))
				$home_net[] = $ip;
		}
		if (function_exists("get_interface_ipv6")) {
			$ip = get_interface_ipv6("wan");
			if (is_ipaddrv6($ip)) {
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
		}
	}

	if ($wangw == 'yes') {
		/* Grab the default gateway if set */
		$default_gw = exec("/sbin/route -n get default |grep 'gateway:' | /usr/bin/awk '{ print $2 }'");
		if (is_ipaddr($default_gw) && !in_array($default_gw, $home_net))
			$home_net[] = $default_gw;

		/* Get any other interface gateway and put in $HOME_NET if not there already */
		$gw = get_interface_gateway($snortcfg['interface']);
		if (is_ipaddr($gw) && !in_array($gw, $home_net))
			$home_net[] = $gw;
		if (function_exists("get_interface_gatewayv6")) {
			$gw = get_interface_gatewayv6($snortcfg['interface']);
			if (is_ipaddrv6($gw) && !in_array($gw, $home_net))
				$home_net[] = $gw;
		}
	}

	if ($wandns == 'yes') {
		/* Add DNS server for WAN interface to whitelist */
		$dns_servers = get_dns_servers();
		foreach ($dns_servers as $dns) {
			if ($dns && !in_array($dns, $home_net))
				$home_net[] = $dns;
		}
	}

	if($vips == 'yes') {
		/* iterate all vips and add to whitelist */
		if (is_array($config['virtualip']) && is_array($config['virtualip']['vip'])) {
			foreach($config['virtualip']['vip'] as $vip) {
				if ($vip['subnet'] && $vip['mode'] != 'proxyarp') {
					if (!in_array("{$vip['subnet']}/{$vip['subnet_bits']}", $home_net))
						$home_net[] = "{$vip['subnet']}/{$vip['subnet_bits']}";
				}
			}
		}
	}

	/* grab a list of vpns and whitelist if user desires added by nestorfish 954 */
	if ($vpns == 'yes') {
		$vpns_list = filter_get_vpns_list();
		if (!empty($vpns_list)) {
			/* Convert the returned space-delimited string to an array */
			/* and then add each VPN address to our HOME_NET array.    */
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

	/* Release memory no longer required */
	unset($home_net);

	/* Sort the list and return it */
	natsort($valresult);
	return $valresult;
}

/* checks to see if service is running yes/no and stop/start */
function snort_is_running($snort_uuid, $if_real, $type = 'snort') {
	global $config, $g;

	if (file_exists("{$g['varrun_path']}/{$type}_{$if_real}{$snort_uuid}.pid") && isvalidpid("{$g['varrun_path']}/{$type}_{$if_real}{$snort_uuid}.pid"))
		return 'yes';

	return 'no';
}

function snort_barnyard_stop($snortcfg, $if_real) {
	global $config, $g;

	$snort_uuid = $snortcfg['uuid'];
	if (file_exists("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid") && isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Barnyard2 STOP for {$snortcfg['descr']}({$if_real})...");
		killbypid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid");
	}
}

function snort_stop($snortcfg, $if_real) {
	global $config, $g;

	$snort_uuid = $snortcfg['uuid'];
	if (file_exists("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid") && isvalidpid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Snort STOP for {$snortcfg['descr']}({$if_real})...");
		killbypid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid");
	}

	snort_barnyard_stop($snortcfg, $if_real);
}

function snort_barnyard_start($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];

	/* define snortbarnyardlog_chk */
	if ($snortcfg['barnyard_enable'] == 'on' && !empty($snortcfg['barnyard_mysql'])) {
		log_error("[Snort] Barnyard2 START for {$snortcfg['descr']}({$if_real})...");
		exec("/usr/local/bin/barnyard2 -r {$snort_uuid} -f \"snort_{$snort_uuid}_{$if_real}.u2\" --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q");
	}
}

function snort_start($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];

	if ($snortcfg['enable'] == 'on') {
		log_error("[Snort] Snort START for {$snortcfg['descr']}({$if_real})...");
		exec("/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}");
	}
	else
		return;

	snort_barnyard_start($snortcfg, $if_real);
}

/**************************************************************/
/* This function sends a SIGHUP to the Snort instance on the  */
/* passed interface to cause Snort to reload and parse the    */
/* running configuration without stopping packet processing.  */
/* It also executes the reload as a background process and    */
/* returns control immediately to the caller.                 */
/**************************************************************/
function snort_reload_config($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];

	/******************************************************/
	/* Only send the SIGHUP if Snort is running and we    */
	/* can find a valid PID for the process.              */
	/******************************************************/
	if (file_exists("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid") && isvalidpid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Snort RELOAD CONFIG for {$snortcfg['descr']}({$if_real})...");
		exec("/bin/pkill -SIGHUP -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid 2>&1 &");
	}
}

function snort_get_friendly_interface($interface) {

	if (function_exists('convert_friendly_interface_to_friendly_descr'))
		$iface = convert_friendly_interface_to_friendly_descr($interface);
	else {
		if (!$interface || ($interface == "wan"))
			$iface = "WAN";
		else if(strtolower($interface) == "lan")
			$iface = "LAN";
		else if(strtolower($interface) == "pppoe")
			$iface = "PPPoE";
		else if(strtolower($interface) == "pptp")
			$iface = "PPTP";
		else
			$iface = strtoupper($interface);
	}

	return $iface;
}

/* get the real iface name of wan */
function snort_get_real_interface($interface) {
	global $config;

	$lc_interface = strtolower($interface);
	if (function_exists('get_real_interface'))
		return get_real_interface($lc_interface);
	else {
		if ($lc_interface == "lan") {
			if ($config['inerfaces']['lan'])
				return $config['interfaces']['lan']['if'];
			return $interface;
		}
		if ($lc_interface == "wan")
			return $config['interfaces']['wan']['if'];
		$ifdescrs = array();
		for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
			$ifname = "opt{$j}";
			if(strtolower($ifname) == $lc_interface)
				return $config['interfaces'][$ifname]['if'];
			if(isset($config['interfaces'][$ifname]['descr']) && (strtolower($config['interfaces'][$ifname]['descr']) == $lc_interface))
				return $config['interfaces'][$ifname]['if'];
		}
	} 

	return $interface;
}

/*
 this code block is for deleteing logs while keeping the newest file,
 snort is linked to these files while running, do not take the easy way out
 by touch and rm, snort will lose sync and not log.
*/
function snort_post_delete_logs($snort_uuid = 0) {
	global $config, $g;

	/* do not start config build if rules is empty */
	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	foreach ($config['installedpackages']['snortglobal']['rule'] as $value) {
		if ($value['uuid'] != $snort_uuid)
			continue;
		$if_real = snort_get_real_interface($value['interface']);
		$snort_log_dir = "/var/log/snort/snort_{$if_real}{$snort_uuid}";

		if ($if_real != '') {
			$filelist = glob("{$snort_log_dir}/*{$snort_uuid}_{$if_real}.u2.*");
			unset($filelist[count($filelist) - 1]);
			foreach ($filelist as $file)
				@unlink($file);

			if ($value['perform_stat'] == 'on') {
				$fd = fopen("{$snort_log_dir}/{$if_real}.stats", "w");
				if ($fd) {
					ftruncate($fd, 0);
					fclose($fd);
				}
			}
		}
	}
}

function snort_postinstall() {
	global $config, $g, $snort_rules_file, $emerging_threats_version;
	global $snort_version, $rebuild_rules, $is_postinstall;

	$snortdir = SNORTDIR;
	$snortlibdir = SNORTLIBDIR;
	$rcdir = RCFILEPREFIX;

	/* Set flag for post-install in progress */
	$is_postinstall = true;

	/* cleanup default files */
	@rename("{$snortdir}/snort.conf-sample", "{$snortdir}/snort.conf");
	@rename("{$snortdir}/threshold.conf-sample", "{$snortdir}/threshold.conf");
	@rename("{$snortdir}/sid-msg.map-sample", "{$snortdir}/sid-msg.map");
	@rename("{$snortdir}/unicode.map-sample", "{$snortdir}/unicode.map");
	@rename("{$snortdir}/classification.config-sample", "{$snortdir}/classification.config");
	@rename("{$snortdir}/generators-sample", "{$snortdir}/generators");
	@rename("{$snortdir}/reference.config-sample", "{$snortdir}/reference.config");
	@rename("{$snortdir}/gen-msg.map-sample", "{$snortdir}/gen-msg.map");

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

	/* remove example library files */
	$files = glob("{$snortlibdir}/dynamicrules/*_example*");
	foreach ($files as $f)
		@unlink($f);
	$files = glob("{$snortlibdir}/dynamicpreprocessor/*_example*");
	foreach ($files as $f)
		@unlink($f);

	/* remake saved settings */
	if ($config['installedpackages']['snortglobal']['forcekeepsettings'] == 'on') {
		log_error(gettext("[Snort] Saved settings detected... rebuilding installation with saved settings..."));
		update_status(gettext("Saved settings detected..."));
		update_output_window(gettext("Please wait... rebuilding installation with saved settings..."));
		log_error(gettext("[Snort] Downloading and updating configured rule types..."));
		update_output_window(gettext("Please wait... downloading and updating configured rule types..."));
		@include_once("/usr/local/pkg/snort/snort_check_for_rule_updates.php");
		update_status(gettext("Generating snort.conf configuration file from saved settings..."));
		$rebuild_rules = "on";
		sync_snort_package_config();
		$rebuild_rules = "off";
		update_output_window(gettext("Finished rebuilding files..."));
		log_error(gettext("[Snort] Finished rebuilding installation from saved settings..."));

		/* Only try to start Snort if not in reboot */
		if (!$g['booting']) {
			update_status(gettext("Starting Snort using rebuilt configuration..."));
			update_output_window(gettext("Please wait... while Snort is started..."));
			log_error(gettext("[Snort] Starting Snort using rebuilt configuration..."));
			update_output_window(gettext("Snort has been started using the rebuilt configuration..."));
			start_service("snort");
		}
	}

	/* Done with post-install, so clear flag */
	$is_postinstall = false;
	log_error(gettext("[Snort] Package post-installation tasks completed..."));
}

function snort_Getdirsize($node) {
	if(!is_readable($node))
		return false;

	$blah = exec( "/usr/bin/du -kd $node" );
	return substr( $blah, 0, strpos($blah, 9) );
}

/* func for log dir size limit cron */
function snort_snortloglimit_install_cron($should_install) {
	global $config, $g;

	if (!is_array($config['cron']['item']))
		$config['cron']['item'] = array();

	$x=0;
	$is_installed = false;
	foreach($config['cron']['item'] as $item) {
		if (strstr($item['command'], 'snort_check_cron_misc.inc')) {
			$is_installed = true;
			break;
		}
		$x++;
	}

	switch($should_install) {
		case true:
			if(!$is_installed)  {

				$cron_item = array();
				$cron_item['minute'] = "*/5";
				$cron_item['hour'] = "*";
				$cron_item['mday'] = "*";
				$cron_item['month'] = "*";
				$cron_item['wday'] = "*";
				$cron_item['who'] = "root";
				$cron_item['command'] = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/snort/snort_check_cron_misc.inc";
				$config['cron']['item'][] = $cron_item;
			}
			break;
		case false:
			if($is_installed == true)
				unset($config['cron']['item'][$x]);
			break;
	}
}

/* func for updating cron */
function snort_rm_blocked_install_cron($should_install) {
	global $config, $g;

	if (!is_array($config['cron']['item']))
		$config['cron']['item'] = array();

	$x=0;
	$is_installed = false;
	foreach($config['cron']['item'] as $item) {
		if (strstr($item['command'], "snort2c")) {
			$is_installed = true;
			break;
		}
		$x++;
	}

	$snort_rm_blocked_info_ck = $config['installedpackages']['snortglobal']['rm_blocked'];
	if ($snort_rm_blocked_info_ck == "1h_b") {
		$snort_rm_blocked_min = "*/5";
		$snort_rm_blocked_hr = "*";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "3600";
	}
	if ($snort_rm_blocked_info_ck == "3h_b") {
		$snort_rm_blocked_min = "*/15";
		$snort_rm_blocked_hr = "*";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "10800";
	}
	if ($snort_rm_blocked_info_ck == "6h_b") {
		$snort_rm_blocked_min = "*/30";
		$snort_rm_blocked_hr = "*";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "21600";
	}
	if ($snort_rm_blocked_info_ck == "12h_b") {
		$snort_rm_blocked_min = "2";
		$snort_rm_blocked_hr = "*/1";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "43200";
	}
	if ($snort_rm_blocked_info_ck == "1d_b") {
		$snort_rm_blocked_min = "2";
		$snort_rm_blocked_hr = "*/2";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "86400";
	}
	if ($snort_rm_blocked_info_ck == "4d_b") {
		$snort_rm_blocked_min = "2";
		$snort_rm_blocked_hr = "*/8";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "345600";
	}
	if ($snort_rm_blocked_info_ck == "7d_b") {
		$snort_rm_blocked_min = "2";
		$snort_rm_blocked_hr = "*/14";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "604800";
	}
	if ($snort_rm_blocked_info_ck == "28d_b") {
		$snort_rm_blocked_min = "2";
		$snort_rm_blocked_hr = "0";
		$snort_rm_blocked_mday = "*/2";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "2419200";
	}
	switch($should_install) {
	case true:
		$cron_item = array();
		$cron_item['minute'] = "$snort_rm_blocked_min";
		$cron_item['hour'] = "$snort_rm_blocked_hr";
		$cron_item['mday'] = "$snort_rm_blocked_mday";
		$cron_item['month'] = "$snort_rm_blocked_month";
		$cron_item['wday'] = "$snort_rm_blocked_wday";
		$cron_item['who'] = "root";
		$cron_item['command'] = "/usr/bin/nice -n20 /usr/local/sbin/expiretable -t $snort_rm_blocked_expire snort2c";

		/* Add cron job if not already installed, else just update the existing one */
		if (!$is_installed) 
			$config['cron']['item'][] = $cron_item;
		elseif ($is_installed)
			$config['cron']['item'][$x] = $cron_item;
		break;
	case false:
		if ($is_installed == true)
			unset($config['cron']['item'][$x]);
		break;
	}
}

/* func to install snort update */
function snort_rules_up_install_cron($should_install) {
	global $config, $g;

	if(!$config['cron']['item'])
		$config['cron']['item'] = array();

	$x=0;
	$is_installed = false;
	foreach($config['cron']['item'] as $item) {
		if (strstr($item['command'], "snort_check_for_rule_updates.php")) {
			$is_installed = true;
			break;
		}
		$x++;
	}
	$snort_rules_up_info_ck = $config['installedpackages']['snortglobal']['autorulesupdate7'];
	if ($snort_rules_up_info_ck == "6h_up") {
		$snort_rules_up_min = "3";
		$snort_rules_up_hr = "*/6";
		$snort_rules_up_mday = "*";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "12h_up") {
		$snort_rules_up_min = "3";
		$snort_rules_up_hr = "*/12";
		$snort_rules_up_mday = "*";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "1d_up") {
		$snort_rules_up_min = "3";
		$snort_rules_up_hr = "0";
		$snort_rules_up_mday = "*/1";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "4d_up") {
		$snort_rules_up_min = "3";
		$snort_rules_up_hr = "0";
		$snort_rules_up_mday = "*/4";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "7d_up") {
		$snort_rules_up_min = "3";
		$snort_rules_up_hr = "0";
		$snort_rules_up_mday = "*/7";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "28d_up") {
		$snort_rules_up_min = "3";
		$snort_rules_up_hr = "0";
		$snort_rules_up_mday = "*/28";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	switch($should_install) {
	case true:
		$cron_item = array();
		$cron_item['minute'] = "$snort_rules_up_min";
		$cron_item['hour'] = "$snort_rules_up_hr";
		$cron_item['mday'] = "$snort_rules_up_mday";
		$cron_item['month'] = "$snort_rules_up_month";
		$cron_item['wday'] = "$snort_rules_up_wday";
		$cron_item['who'] = "root";
		$cron_item['command'] = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/snort/snort_check_for_rule_updates.php";

		/* Add cron job if not already installed, else just update the existing one */
		if (!$is_installed) 
			$config['cron']['item'][] = $cron_item;
		elseif ($is_installed)
			$config['cron']['item'][$x] = $cron_item;
		break;
	case false:
		if($is_installed == true)
			unset($config['cron']['item'][$x]);
		break;
	}
}

/* Only run when all ifaces needed to sync. Expects filesystem rw */
function sync_snort_package_config() {
	global $config, $g, $flowbit_rules_file, $snort_enforcing_rules_file;
	global $snort_version, $rebuild_rules, $is_postinstall;

	$snortdir = SNORTDIR;
	$rcdir = RCFILEPREFIX;

	conf_mount_rw();

	/* do not start config build if rules is empty or there are no Snort settings */
	if (!is_array($config['installedpackages']['snortglobal']) || !is_array($config['installedpackages']['snortglobal']['rule'])) {
		@unlink("{$rcdir}/snort.sh");
		conf_mount_ro();
		return;
	}

	$snortconf = $config['installedpackages']['snortglobal']['rule'];
	foreach ($snortconf as $value) {
		$if_real = snort_get_real_interface($value['interface']);

		/* create a snort.conf file for interface           */
		snort_generate_conf($value);

		/* create barnyard2.conf file for interface         */
		if ($value['barnyard_enable'] == 'on')
			snort_create_barnyard2_conf($value, $if_real);
	}

	/* create snort bootup file snort.sh only create once */
	snort_create_rc();

	$snortglob = $config['installedpackages']['snortglobal'];

	snort_snortloglimit_install_cron($snortglob['snortloglimit'] == 'on' ? true : false);

	/* set the snort block hosts time IMPORTANT */
	snort_rm_blocked_install_cron($snortglob['rm_blocked'] != "never_b" ? true : false);

	/* set the snort rules update time */
	snort_rules_up_install_cron($snortglob['autorulesupdate7'] != "never_up" ? true : false);

	configure_cron();
	
	/* Do not attempt package sync if reinstalling package or booting */
	if (!$is_postinstall && !$g['booting'])
		snort_sync_on_changes();
	
	conf_mount_ro();
}

function snort_build_sid_msg_map($rules_path, $sid_file) {

	/*************************************************************/
	/* This function reads all the rules file in the passed      */
	/* $rules_path variable and produces a properly formatted    */
	/* sid-msg.map file for use by Snort and/or barnyard2.       */
	/*************************************************************/

        $sidMap = array();
	$rule_files = array();

	/* First check if we were passed a directory, a single file   */
	/* or an array of filenames to read. Set our $rule_files      */
	/* variable accordingly. If we can't figure it out, return    */
	/* and don't write a sid_msg_map file.                        */
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

        /* Read the rule files into an array, then iterate the list  */
        foreach ($rule_files as $file) {

             /* Don't process files with "deleted" in the filename   */
             if (stristr($file, "deleted"))
                  continue;

             /* Read the file into an array, skipping missing files. */
	     if (!file_exists($file))
			continue;

             $rules_array = file($file, FILE_SKIP_EMPTY_LINES);
             $record = "";
             $b_Multiline = false;

             /* Read and process each line from the rules in the     */
             /* current file.                                        */
             foreach ($rules_array as $rule) {

                  /* Skip any non-rule lines unless we're in         */
                  /* multiline mode.				     */
                  if (!preg_match('/^\s*#*\s*(alert|drop|pass)/i', $rule) && !$b_Multiline)
                       continue;

                  /* Test for a multi-line rule, and reassemble the  */
                  /* pieces back into a single line.                 */
                  if (preg_match('/\\\\s*[\n]$/m', $rule)) {
                       $rule = substr($rule, 0, strrpos($rule, '\\'));
                       $record .= $rule;
                       $b_Multiline = true;
                       continue;
                  }
                  /* If the last segment of a multiline rule, then   */
                  /* append it onto the previous parts to form a     */
                  /* single-line rule for further processing below.  */
                  elseif (!preg_match('/\\\\s*[\n]$/m', $rule) && $b_Multiline) {
                       $record .= $rule;
                       $rule = $record;
                  }
                  $b_Multiline = false;
                  $record = "";

                  /* Parse the rule to find sid and any references.  */
                  $sid = '';
                  $msg = '';
                  $matches = '';
                  $sidEntry = '';
                  if (preg_match('/\bmsg\s*:\s*"(.+?)"\s*;/i', $rule, $matches))
                       $msg = trim($matches[1]);
                  if (preg_match('/\bsid\s*:\s*(\d+)\s*;/i', $rule, $matches))
                       $sid = trim($matches[1]);
                  if (!empty($sid) && !empty($msg)) {
                       $sidEntry = $sid . ' || ' . $msg;
                       preg_match_all('/\breference\s*:\s*([^\;]+)/i', $rule, $matches);
                       foreach ($matches[1] as $ref)
                            $sidEntry .= " || " . trim($ref);
                       $sidEntry .= "\n";
                       $sidMap[$sid] = $sidEntry;
                  }
             }
        }
        /* Sort the generated sid-msg map by sid */
        ksort($sidMap);

        /* Now print the result to the supplied file */
	@file_put_contents($sid_file, array_values($sidMap));
}

function snort_merge_reference_configs($cfg_in, $cfg_out) {

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
                                        if (!array_key_exists($matches[2], $outMap))
                                                $outMap[$matches[2]] = trim($matches[3]);
                                }
                        }
                }             
        }
        /* Sort the new reference map.  */
        uksort($outMap,'strnatcasecmp');

	/**********************************************************/
	/* Do NOT write an empty references.config file, just     */
	/* exit instead.                                          */
	/**********************************************************/
	if (empty($outMap))
		return false;

        /* Format and write it to the supplied output file.  */
        $format = "config reference: %-12s %s\n";
        foreach ($outMap as $key=>$value)
                $outMap[$key] = sprintf($format, $key, $value);
        @file_put_contents($cfg_out, array_values($outMap));
	return true;
}

function snort_merge_classification_configs($cfg_in, $cfg_out) {

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
                                        if (!array_key_exists($matches[2], $outMap))
                                                $outMap[$matches[2]] = trim($matches[3]) . "," . trim($matches[4]);
                                }
                        }
                }             
        }
        /* Sort the new classification map.  */
        uksort($outMap,'strnatcasecmp');

	/**********************************************************/
	/* Do NOT write an empty classification.config file, just */
	/* exit instead.                                          */
	/**********************************************************/
	if (empty($outMap))
		return false;

        /* Format and write it to the supplied output file.  */
        $format = "config classification: %s,%s\n";
        foreach ($outMap as $key=>$value)
                $outMap[$key] = sprintf($format, $key, $value);
        @file_put_contents($cfg_out, array_values($outMap));
	return true;
}

function snort_load_rules_map($rules_path) {

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
	 *   disabled = 1 if rule is disabled (commented out), 0 if 
	 *              rule is enabled
	 *   flowbits = Array of applicable flowbits if rule contains 
	 *              flowbits options
	 ***************************************************************/

	/* First check if we were passed a directory, a single file   */
	/* or an array of filenames to read. Set our $rule_files      */
	/* variable accordingly. If we can't figure it out, return    */
	/* an empty rules map array.                                  */
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

        /* Read the rule files into an array, then iterate the list   */
	/* to process the rules from the files one-by-one.            */
	foreach ($rule_files as $file) {

		/* Don't process files with "deleted" in the filename. */
		if (stristr($file, "deleted"))
			continue;

		/* Read the file contents into an array, skipping    */
		/* missing files.                                    */
		if (!file_exists($file))
			continue;

		$rules_array = file($file, FILE_SKIP_EMPTY_LINES);
		$record = "";
		$b_Multiline = false;

		/* Read and process each line from the rules in the */
		/* current file into an array.                      */
		foreach ($rules_array as $rule) {

			/* Skip any lines that may be just spaces.  */
			if (trim($rule, " \n") == "")
				continue;

			/* Skip any non-rule lines unless we're in  */
			/* multiline mode.			    */
			if (!preg_match('/^\s*#*\s*(alert|drop|pass)/i', $rule) && !$b_Multiline)
				continue;

			/* Test for a multi-line rule; loop and reassemble */
			/* the pieces back into a single line.             */
			if (preg_match('/\\\\s*[\n]$/m', $rule)) {
				$rule = substr($rule, 0, strrpos($rule, '\\'));
				$record .= $rule;
				$b_Multiline = true;
				continue;
			}
			/* If the last segment of a multiline rule, then   */
			/* append it onto the previous parts to form a     */
			/* single-line rule for further processing below.  */
			elseif (!preg_match('/\\\\s*[\n]$/m', $rule) && $b_Multiline) {
				$record .= $rule;
				$rule = $record;
			}

			/* We have an actual single-line rule, or else a   */
			/* re-assembled multiline rule that is now a       */
			/* single-line rule, so store it in our rules map. */

			/* Get and test the SID.  If we don't find one,    */
			/* ignore and skip this rule as it is invalid.     */
			$sid = snort_get_sid($rule);
			if (empty($sid)) {
				$b_Multiline = false;
				$record = "";
				continue;
			}

			$gid = snort_get_gid($rule);
			if (!is_array($map_ref[$gid]))
				$map_ref[$gid] = array();
			if (!is_array($map_ref[$gid][$sid]))
				$map_ref[$gid][$sid] = array();
			$map_ref[$gid][$sid]['rule'] = $rule;
			$map_ref[$gid][$sid]['category'] = basename($file, ".rules");

			if (preg_match('/^\s*\#+/', $rule))
				$map_ref[$gid][$sid]['disabled'] = 1;
			else
				$map_ref[$gid][$sid]['disabled'] = 0;

			/* Grab any associated flowbits from the rule.     */
			$map_ref[$gid][$sid]['flowbits'] = snort_get_flowbits($rule);
			
			/* Reset our local flag and record variables       */
			/* for the next rule in the set.                   */
			$b_Multiline = false;
			$record = "";
		}

		/* Zero out our processing array and get the next file.    */
		unset($rules_array);
	}
	return $map_ref;
}

function snort_get_gid($rule) {

	/****************************************************************/
	/* If a gid is defined, then return it, else default to "1" for */
	/* general text rules match.                                    */
	/****************************************************************/

	if (preg_match('/\bgid\s*:\s*(\d+)\s*;/i', $rule, $matches))
		return trim($matches[1]);
	else
		return "1";
}

function snort_get_sid($rule) {

	/***************************************************************/
	/* If a sid is defined, then return it, else default to an     */
	/* empty value.                                                */
	/***************************************************************/

	if (preg_match('/\bsid\s*:\s*(\d+)\s*;/i', $rule, $matches))
		return trim($matches[1]);
	else
		return "";
}

function snort_get_msg($rule) {

	/**************************************************************/
	/* Return the MSG section of the passed rule as a string.     */
	/**************************************************************/

	$msg = "";
	if (preg_match('/\bmsg\s*:\s*"(.+?)"\s*;/i', $rule, $matches))
		$msg = trim($matches[1]);
	return $msg;
}

function snort_get_flowbits($rule) {

	/*************************************************************/
	/* This will pull out "flowbits:" options from the rule text */
	/* and return them in an array (minus the "flowbits:" part). */
	/*************************************************************/

	$flowbits = array();

	/* Grab any "flowbits:set, setx, unset, isset or toggle" options first.  */
	/* Examine flowbits targets for logical operators to capture all targets */ 
	if (preg_match_all('/flowbits\b\s*:\s*(set|setx|unset|toggle|isset|isnotset)\s*,([^;]+)/i', $rule, $matches)) {
		$i = -1;
		while (++$i < count($matches[1])) {
			$action = trim($matches[1][$i]);
			$target = preg_split('/[&|]/', $matches[2][$i]);
			foreach ($target as $t)
				$flowbits[] = "{$action}," . trim($t);
		}
	}

	/* Include the "flowbits:noalert or reset" options, if present. */
	if (preg_match_all('/flowbits\b\s*:\s*(noalert|reset)\b/i', $rule, $matches)) {
		$i = -1;
		while (++$i < count($matches[1])) {
			$flowbits[] = trim($matches[1][$i]);
		}
	}

	return $flowbits;
}

function snort_get_checked_flowbits(&$rules_map) {

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
				/* If no comma in flowbits option, then  skip it. */
				$pos = strpos($flowbit, ",");
				if ($pos === false)
					continue;
				$action = substr(strtolower($flowbit), 0, $pos);
				if ($action == "isset" || $action == "isnotset") {
					$target = preg_split('/[&|]/', substr($flowbit, $pos + 1));
					foreach ($target as $t)
						if (!empty($t) && !isset($checked_flowbits[$t]))
							$checked_flowbits[$t] = $action;
				}
			}
		}
	}
	unset($rulem, $rulem2);

	return $checked_flowbits;
}

function snort_get_set_flowbits(&$rules_map) {

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
						if (!empty($t) && !isset($set_flowbits[$t]))
							$set_flowbits[$t] = $action;
				}
			}
		}
	}
	unset($rulem, $rulem2);

	return $set_flowbits;
}

function snort_find_flowbit_required_rules(&$all_rules, &$unchecked_flowbits) {

	/********************************************************/
	/* This function finds all rules that must be enabled   */
	/* in order to satisfy the "checked flowbits" used by   */
	/* the currently enabled rules.  It returns the list    */
	/* of required rules in an array.                       */
	/********************************************************/

	$required_flowbits_rules = array();
	foreach ($all_rules as $k1 => $rule) {
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
							/* If not disabled, just return the rule text "as is"   */
							$required_flowbits_rules[$k1][$k2]['rule'] = ltrim($rule2['rule']);
						else {
							/* If rule is disabled, remove leading '#' to enable it */
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

function snort_resolve_flowbits(&$active_rules) {

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
	/* NOTE:  this function assumes the global variable   */
	/*        $all_rules is populated with all the rules  */
	/*        currently downloaded.                       */
	/******************************************************/

	global $all_rules;
	$snortdir = SNORTDIR;

	/* Check $all_rules array to be sure it is filled.    */
	if (empty($all_rules)) {
		log_error(gettext("[Snort] WARNING: Flowbit resolution not done - no rules in {$snortdir}/rules/ ..."));
		return array();
	}

	/* First, find all the "checked" and "set" flowbits.  */
	$checked_flowbits = snort_get_checked_flowbits($active_rules);
	$set_flowbits = snort_get_set_flowbits($active_rules);

	/* Next find any "checked" flowbits without matching  */
	/* "set" flowbit rules in the enabled rule set.       */
	$delta_flowbits = array_diff_key($checked_flowbits, $set_flowbits);

	/* Cleanup and release the memory we no longer need.  */
	unset($checked_flowbits);
	unset($set_flowbits);

	/* Now find all the needed "set flowbit" rules from   */
	/* the master list of all rules.                      */
	$required_rules = snort_find_flowbit_required_rules($all_rules, $delta_flowbits);

	/* Cleanup and release memory we no longer need.      */
	unset($delta_flowbits);

	return $required_rules;
}

function snort_write_flowbit_rules_file($flowbit_rules, $rule_file) {

	/************************************************/
	/* This function takes an array of rules in the */
	/* rules_map format and writes them to the file */
	/* given.                                       */
	/************************************************/

	global $flowbit_rules_file;

	/* See if we were passed a directory or full    */
	/* filename to write the rules to, and adjust   */
	/* the destination argument accordingly.        */
	if (is_dir($rule_file))
		$rule_file = rtrim($rule_file, '/')."/{$flowbit_rules_file}";

	if (empty($flowbit_rules)) {
		@file_put_contents($rule_file, "");
		return;
	}

	$fp = fopen($rule_file, "w");
	if ($fp) {
		@fwrite($fp, "# These rules set flowbits checked by your other enabled rules.  If the\n");
		@fwrite($fp, "# the dependent flowbits are not set, then some of your chosen rules may\n");
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

function snort_load_vrt_policy($policy, $load_rules_map=true) {

	/************************************************/
	/* This function returns an array of all rules  */
	/* marked with the passed in $policy metadata.  */
	/*                                              */
	/* $policy --> desired VRT security policy      */
	/*              1.  connectivity                */
	/*              2.  balanced                    */
	/*              3.  security                    */
	/*                                              */
	/* $load_rules --> load a local copy of all     */
	/*                 the rules if true.  If       */
	/*                 false, assume the global     */
	/*                 $all_rules array is valid.   */
	/************************************************/

	global $all_rules;
	$snortdir = SNORTDIR;
	$vrt_policy_rules = array();

	/* Refresh the map of all the rules if flag */
	/* is set.                                  */
	if ($load_rules_map) {
		/* Since only Snort VRT rules have IPS Policy metadata, */
		/* limit our search to just those files.                */
		$snort_vrt_files = glob("{$snortdir}/rules/snort_*.rules");
		$all_rules = array();
		$all_rules = snort_load_rules_map($snort_vrt_files);
	}

	/* Now walk the rules list and find all those that are defined  */
	/* defined as active for the chosen security policy.            */
	foreach ($all_rules as $k1 => $arulem) {
		foreach ($arulem as $k2 => $arulem2) {
			if (strripos($arulem2['rule'], "policy {$policy}-ips") !== false) {
				if (!preg_match('/flowbits\s*:\s*noalert/i', $arulem2['rule'])) {
					if (!is_array($vrt_policy_rules[$k1]))
						$vrt_policy_rules[$k1] = array();
					if (!is_array($vrt_policy_rules[$k1][$k2]))
						$vrt_policy_rules[$k1][$k2] = array();
					$vrt_policy_rules[$k1][$k2] = $arulem2;

					/* Enable the policy rule if disabled  */
					if ($arulem2['disabled'] == 1) {
						$vrt_policy_rules[$k1][$k2]['rule'] = ltrim(substr($arulem2['rule'], strpos($arulem2['rule'], "#") + 1));
						$vrt_policy_rules[$k1][$k2]['disabled'] = 0;
					}
				}
			}
		}
	}

	/* Release memory we no longer need. */
	unset($arulem, $arulem2);

	/* If we loaded the ALL_RULES map,   */
	/* then release the global memory.   */
	if ($load_rules_map == true)
		unset($GLOBALS['all_rules']);

	/* Return all the rules that match the policy. */
	return $vrt_policy_rules;
}

function snort_write_enforcing_rules_file($rule_map, $rule_path) {

	/************************************************/
	/* This function takes a rules map array of     */
	/* the rules chosen for the active rule set     */
	/* and writes them out to the passed path.      */
	/************************************************/

	global $snort_enforcing_rules_file;

	$rule_file = "/{$snort_enforcing_rules_file}";

	/* See if we were passed a directory or full    */
	/* filename to write the rules to, and adjust   */
	/* the destination argument accordingly.        */
	if (is_dir($rule_path))
		$rule_file = rtrim($rule_path, '/').$rule_file;
	else
		$rule_file = $rule_path;

	/* If the $rule_map array is empty, then exit.  */
	if (empty($rule_map)) {
		file_put_contents($rule_file, "");
		return;
	}

	$fp = fopen($rule_file, "w");
	if ($fp) {
		@fwrite($fp, "# These rules are your current set of enforced rules for the protected\n");
		@fwrite($fp, "# interface.  This list was compiled from the categories selected on the\n");
		@fwrite($fp, "# CATEGORIES tab of the Snort configuration for the interface and/or any\n");
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

function snort_load_sid_mods($sids, $value) {

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
	/* $value ==> type of mod (enable or     */
	/*            disable).  Not currently   */
	/*            utilized, but maintained   */
	/*            so as not to break legacy  */
	/*            code elsewhere.            */
	/*****************************************/

	$result = array();
	if (empty($sids) || empty($value))
		return $result;
	$tmp = explode("||", $sids);
	foreach ($tmp as $v) {
		if (preg_match('/\s\d+/', $v, $match))
			$result[trim($match[0])] = trim($match[0]);
	}
	unset($tmp);

	return $result;
}

function snort_modify_sids(&$rule_map, $snortcfg) {

	/*****************************************/
	/* This function modifies the rules in   */
	/* the passed rules_map array based on   */
	/* values in the enablesid/disablesid    */
	/* configuration parameters.             */
	/*                                       */
	/*  $rule_map = array of current rules   */
	/*  $snortcfg = config settings          */
	/*****************************************/

	if (!isset($snortcfg['rule_sid_on']) && !isset($snortcfg['rule_sid_off']))
		return;

	/* Load up our enablesid and disablesid  */
	/* arrays with lists of modified SIDs    */
	$enablesid = snort_load_sid_mods($snortcfg['rule_sid_on'], "enablesid");
	$disablesid = snort_load_sid_mods($snortcfg['rule_sid_off'], "disablesid");

	/* Turn on any rules that need to be     */
	/* forced "on" with enablesid mods.      */
	if (!empty($enablesid)) {
		foreach ($rule_map as $k1 => $rulem) {
			foreach ($rulem as $k2 => $v) {
				if (in_array($k2, $enablesid) && $v['disabled'] == 1) {
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
				if (in_array($k2, $disablesid) && $v['disabled'] == 0) {
					$rule_map[$k1][$k2]['rule'] = "# " . $v['rule'];
					$rule_map[$k1][$k2]['disabled'] = 1;
				}
			}
		}
	}
	unset($enablesid, $disablesid);
}

/* Start of main config files */
/* open snort.sh for writing" */
function snort_create_rc() {
	global $config, $g;

	$snortdir = SNORTDIR;
	$rcdir = RCFILEPREFIX;	

	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	$snortconf =& $config['installedpackages']['snortglobal']['rule'];
	/* do not start config build if rules is empty */
	if (empty($snortconf))
		return;

	$start_snort_iface_start = array();
	$start_snort_iface_stop = array();
	foreach ($snortconf as $value) {
		$snort_uuid = $value['uuid'];
		$if_real = snort_get_real_interface($value['interface']);

		$start_barnyard = <<<EOE

	if [ ! -f {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid ]; then
		pid=`/bin/pgrep -xf '/usr/local/bin/barnyard2 -r {$snort_uuid} -f snort_{$snort_uuid}_{$if_real}.u2 --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q' > {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid`
	else
		pid=`/bin/pgrep -F {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid`
	fi
	if [ ! -z \$pid ]; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Barnyard2 STOP for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/bin/pkill $pid -a
		time=0 timeout=30
		while kill -0 \$pid 2>/dev/null; do
			sleep 1
			time=\$((time+1))
			if [ \$time -gt \$timeout ]; then
				break
			fi
		done
		if [ -f /var/run/barnyard2_{$if_real}{$snort_uuid}.pid ]; then
			/bin/rm /var/run/barnyard2_{$if_real}{$snort_uuid}.pid
		fi
	fi
	/usr/bin/logger -p daemon.info -i -t SnortStartup "Barnyard2 START for {$value['descr']}({$snort_uuid}_{$if_real})..."
	/usr/local/bin/barnyard2 -r {$snort_uuid} -f snort_{$snort_uuid}_{$if_real}.u2 --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q

EOE;
		$stop_barnyard2 = <<<EOE

	if [ -f {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid ]; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Barnyard2 STOP for {$value['descr']}({$snort_uuid}_{$if_real})..."
		pid=`/bin/pgrep -F {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid`
                /bin/pkill -F {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid -a
		time=0 timeout=30
		while kill -0 \$pid 2>/dev/null; do
			sleep 1
			time=\$((time+1))
			if [ \$time -gt \$timeout ]; then
				break
			fi
		done
		if [ -f /var/run/barnyard2_{$if_real}{$snort_uuid}.pid ]; then
			/bin/rm /var/run/barnyard2_{$if_real}{$snort_uuid}.pid
		fi
	else
		pid=`/bin/pgrep -xf '/usr/local/bin/barnyard2 -r {$snort_uuid} -f snort_{$snort_uuid}_{$if_real}.u2 --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q'`
		if [ ! -z \$pid ]; then
			/bin/pkill -xf '/usr/local/bin/barnyard2 -r {$snort_uuid} -f snort_{$snort_uuid}_{$if_real}.u2 --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q'
			time=0 timeout=30
			while kill -0 \$pid 2>/dev/null; do
				sleep 1
				time=\$((time+1))
				if [ \$time -gt \$timeout ]; then
					break
				fi
			done
		fi
        fi

EOE;
		if ($value['barnyard_enable'] == 'on' && !empty($value['barnyard_mysql']))
			$start_barnyard2 = $start_barnyard;
		else
			$start_barnyard2 = $stop_barnyard2;

		$start_snort_iface_start[] = <<<EOE

###### For Each Iface
	# Start snort and barnyard2
	if [ ! -f {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid ]; then
		pid=`/bin/pgrep -xf '/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}'`
	else
		pid=`/bin/pgrep -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid`
	fi
	if [ ! -z \$pid ]; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort SOFT RESTART for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/bin/pkill -HUP \$pid
	else
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort START for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}
	fi

	sleep 2
	{$start_barnyard2}

EOE;

		$start_snort_iface_stop[] = <<<EOE

	if [ -f {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid ]; then
		pid=`/bin/pgrep -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid`
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort STOP for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/bin/pkill -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid -a
		time=0 timeout=30
		while kill -0 \$pid 2>/dev/null; do
			sleep 1
			time=\$((time+1))
			if [ \$time -gt \$timeout ]; then
				break
			fi
		done
		if [ -f /var/run/snort_{$if_real}{$snort_uuid}.pid ]; then
			/bin/rm /var/run/snort_{$if_real}{$snort_uuid}.pid
		fi
	else
		pid=`/bin/pgrep -xf '/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}'`
		if [ ! -z \$pid ]; then
			/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort STOP for {$value['descr']}({$snort_uuid}_{$if_real})..."
			/bin/pkill -xf '/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}'
			time=0 timeout=30
			while kill -0 \$pid 2>/dev/null; do
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

	$rc_start = implode("\n", $start_snort_iface_start);
	$rc_stop = implode("\n", $start_snort_iface_stop);

	$snort_sh_text = <<<EOD
#!/bin/sh
########
# This file was automatically generated
# by the pfSense service handler.
# Code added to protect from double starts on pfSense bootup
######## Begining of Main snort.sh

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

	/* write out snort.sh */
	if (!@file_put_contents("{$rcdir}/snort.sh", $snort_sh_text)) {
		log_error("Could not open {$rcdir}/snort.sh for writing.");
		return;
	}
	@chmod("{$rcdir}/snort.sh", 0755);
}

/* open barnyard2.conf for writing */
function snort_create_barnyard2_conf($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];

	if (!file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf"))
		exec("/usr/bin/touch {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf");

	if (!file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2/{$snort_uuid}_{$if_real}.waldo")) {
		@touch("/var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2/{$snort_uuid}_{$if_real}.waldo");
		mwexec("/bin/chmod 770 /var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo", true);
	}

	$barnyard2_conf_text = snort_generate_barnyard2_conf($snortcfg, $if_real);

	/* write out barnyard2_conf */
	@file_put_contents("{$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf", $barnyard2_conf_text);
}

/* open barnyard2.conf for writing" */
function snort_generate_barnyard2_conf($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];

	/* TODO: add support for the other 5 output plugins */
	$snortbarnyardlog_database_info_chk = $snortcfg['barnyard_mysql'];
	$snortbarnyardlog_hostname_info_chk = php_uname("n");
	/* user add arguments */
	$snortbarnyardlog_config_pass_thru = str_replace("\r", "", base64_decode($snortcfg['barnconfigpassthru']));

	$barnyard2_conf_text = <<<EOD

#	barnyard2.conf
#   barnyard2 can be found at http://www.securixlive.com/barnyard2/index.php
#
# set the appropriate paths to the file(s) your Snort process is using

config reference_file:	    {$snortdir}/snort_{$snort_uuid}_{$if_real}/reference.config
config classification_file: {$snortdir}/snort_{$snort_uuid}_{$if_real}/classification.config
config gen_file:            {$snortdir}/snort_{$snort_uuid}_{$if_real}/gen-msg.map
config sid_file:	    {$snortdir}/snort_{$snort_uuid}_{$if_real}/sid-msg.map

config hostname:       $snortbarnyardlog_hostname_info_chk
config interface:      {$if_real}
config decode_data_link
config waldo_file: /var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2/{$snort_uuid}_{$if_real}.waldo

# Show year in timestamps
config show_year

## START user pass through ##

	{$snortbarnyardlog_config_pass_thru}

## END user pass through ##

# Step 2: setup the input plugins
input unified2

config logdir: /var/log/snort/snort_{$if_real}{$snort_uuid}

# database: log to a variety of databases
# output database: log, mysql, user=xxxx password=xxxxxx dbname=xxxx host=xxx.xxx.xxx.xxxx

	$snortbarnyardlog_database_info_chk

EOD;

	return $barnyard2_conf_text;
}

function snort_deinstall() {

	global $config, $g, $snort_rules_upd_log;

	$snortdir = SNORTDIR;
	$snortlibdir = SNORTLIBDIR;
	$snortlogdir = SNORTLOGDIR;
	$rcdir = RCFILEPREFIX;

	log_error(gettext("[Snort] Snort package uninstall in progress..."));

	/* Make sure all active Snort processes are terminated */
	/* Log a message only if a running process is detected */
	if (is_service_running("snort"))
		log_error(gettext("[Snort] Snort STOP for all interfaces..."));
	mwexec('/usr/bin/killall snort', true);
	sleep(2);
	mwexec('/usr/bin/killall -9 snort', true);
	sleep(2);

	/* Make sure all active Barnyard2 processes are terminated */
	/* Log a message only if a running process is detected     */
	if (is_service_running("barnyard2"))
		log_error(gettext("[Snort] Barnyard2 STOP for all interfaces..."));
	mwexec('/usr/bin/killall barnyard2', true);
	sleep(2);
	mwexec('/usr/bin/killall -9 barnyard2', true);
	sleep(2);

	/* Remove the snort user and group */
	mwexec('/usr/sbin/pw userdel snort; /usr/sbin/pw groupdel snort', true);

	/* Remove snort cron entries Ugly code needs smoothness */
	if (!function_exists('snort_deinstall_cron')) {
		function snort_deinstall_cron($crontask) {
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

	/* Remove all the Snort cron jobs. */
	snort_deinstall_cron("snort2c");
	snort_deinstall_cron("snort_check_for_rule_updates.php");
	snort_deinstall_cron("snort_check_cron_misc.inc");
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
		log_error(gettext("[Snort] Package deletion requested... removing all files..."));
		mwexec("/bin/rm -rf {$snortdir}");
		mwexec("/bin/rm -rf {$snortlibdir}/dynamicrules");
		mwexec("/bin/rm -f {$rcdir}/snort.sh");
		mwexec("/bin/rm -rf /usr/local/pkg/snort");
		mwexec("/bin/rm -rf /usr/local/www/snort");
		mwexec("/bin/rm -rf /usr/local/etc/snort");
	}

	/* Keep this as a last step */
	if ($config['installedpackages']['snortglobal']['forcekeepsettings'] != 'on') {
		log_error(gettext("Not saving settings... all Snort configuration info and logs deleted..."));
		unset($config['installedpackages']['snortglobal']);
		unset($config['installedpackages']['snortsync']);
		@unlink("{$snort_rules_upd_log}");
		mwexec("/bin/rm -rf {$snortlogdir}");
		log_error(gettext("[Snort] The package has been removed from this system..."));
	}
}

function snort_prepare_rule_files($snortcfg, $snortcfgdir) {

	global $snort_enforcing_rules_file, $flowbit_rules_file, $rebuild_rules, $all_rules;

	$snortdir = SNORTDIR;
	$no_rules_defined = true;

	/* If there is no reason to rebuild the rules, exit to save time. */
	if ($rebuild_rules == "off")
		return;

	/* Log a message for rules rebuild in progress */
	log_error(gettext("[Snort] Updating rules configuration for: " . snort_get_friendly_interface($snortcfg['interface']) . " ..."));

	/* Only rebuild rules if some are selected or an IPS Policy is enabled  */
	if (!empty($snortcfg['rulesets']) || $snortcfg['ips_policy_enable'] == 'on') {
		$enabled_rules = array();
		$enabled_files = array();
		$no_rules_defined = false;

		/* Load up all the rules into a Rules Map array. */
		$all_rules = array();
		$all_rules = snort_load_rules_map("{$snortdir}/rules/");

		/* Create an array with the filenames of the enabled  */
		/* rule category files if we have any.                */
		if (!empty($snortcfg['rulesets'])) {
			foreach (explode("||", $snortcfg['rulesets']) as $file){
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

			/* Release memory we no longer need. */
			unset($enabled_files, $rulem, $v);
		}

		/* Check if a pre-defined Snort VRT policy is selected. If so, */
		/* add all the VRT policy rules to our enforcing rule set.     */
		if (!empty($snortcfg['ips_policy'])) {
			$policy_rules = snort_load_vrt_policy($snortcfg['ips_policy'], false);
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

		/* Process any enablesid or disablesid modifications for the selected rules. */
		snort_modify_sids($enabled_rules, $snortcfg);

		/* Check for and disable any rules dependent upon disabled preprocessors if  */
		/* this option is enabled for the interface.                                 */
		if ($snortcfg['preproc_auto_rule_disable'] == "on") {
			log_error('[Snort] Checking for rules dependent on disabled preprocessors for: ' . snort_get_friendly_interface($snortcfg['interface']) . '...');
			snort_filter_preproc_rules($snortcfg, $enabled_rules);
		}

		/* Write the enforcing rules file to the Snort interface's "rules" directory. */
		snort_write_enforcing_rules_file($enabled_rules, "{$snortcfgdir}/rules/{$snort_enforcing_rules_file}");

		/* If auto-flowbit resolution is enabled, generate the dependent flowbits rules file. */
		if ($snortcfg['autoflowbitrules'] == 'on') {
			log_error('[Snort] Enabling any flowbit-required rules for: ' . snort_get_friendly_interface($snortcfg['interface']) . '...');
			$fbits = snort_resolve_flowbits($enabled_rules);

			/* Check for and disable any flowbit-required rules dependent upon     */
			/* disabled preprocessors if this option is enabled for the interface. */
			if ($snortcfg['preproc_auto_rule_disable'] == "on") {
				log_error('[Snort] Checking flowbit rules dependent on disabled preprocessors for: ' . snort_get_friendly_interface($snortcfg['interface']) . '...');
				snort_filter_preproc_rules($snortcfg, $fbits, true);
			}
			snort_write_flowbit_rules_file($fbits, "{$snortcfgdir}/rules/{$flowbit_rules_file}");
			unset($fbits);
		} else
			/* Just put an empty file to always have the file present */
			snort_write_flowbit_rules_file(array(), "{$snortcfgdir}/rules/{$flowbit_rules_file}");

		unset($GLOBALS['all_rules']);
	} else {
		snort_write_enforcing_rules_file(array(), "{$snortcfgdir}/rules/{$snort_enforcing_rules_file}");
		snort_write_flowbit_rules_file(array(), "{$snortcfgdir}/rules/{$flowbit_rules_file}");
	}

	if (!empty($snortcfg['customrules'])) {
		@file_put_contents("{$snortcfgdir}/rules/custom.rules", base64_decode($snortcfg['customrules']));
		$no_rules_defined = false;
	}
	else
		@file_put_contents("{$snortcfgdir}/rules/custom.rules", "");

	/* Log a warning if the interface has no rules defined or enabled */
	if ($no_rules_defined)
		log_error(gettext("[Snort] Warning - no text rules selected for: " . snort_get_friendly_interface($snortcfg['interface']) . " ..."));

	/* Build a new sid-msg.map file from the enabled */
	/* rules and copy it to the interface directory. */
	log_error(gettext("[Snort] Building new sig-msg.map file for " . snort_get_friendly_interface($snortcfg['interface']) . "..."));
	snort_build_sid_msg_map("{$snortcfgdir}/rules/", "{$snortcfgdir}/sid-msg.map");
}

function snort_filter_preproc_rules($snortcfg, &$active_rules, $persist_log = false) {

	/**************************************************/
	/* This function checks the $active_rules array   */
	/* for rule options dependent upon preprocessors. */
	/* Rules with rule options dependent upon any     */
	/* non-enabled preprocessors are disabled to stop */
	/* start-up errors from unknown rule options.     */
	/*                                                */
	/* $snortcfg     -> config parameters array for   */
	/*                  the interface                 */
	/* $active_rules -> rules_map array of enabled    */
	/*                  rules for the interface       */
	/*						  */
	/* NOTE: This feature must be enabled in the GUI  */
	/*       by the user.  Use of this feature can    */
	/*       severely degrade Snort's ability to      */
	/*       detect threats by disabling potentially  */
	/*       crucial detection rules.                 */
	/**************************************************/

	global $config;

	$snortlogdir = SNORTLOGDIR;
	$disabled_count = 0;
	$log_msg = array();

	/* Check if no rules or if this option is disabled */
	if (empty($active_rules) || $snortcfg['preproc_auto_rule_disable'] <> 'on')
		return;

	/***************************************************
	 * Construct an array of rule options with their   *
	 * associated preprocessors.                       *
	 *                                                 *
	 * IMPORTANT -- Keep this part of the code current *
	 * with changes to preprocessor rule options in    *
	 * Snort VRT rules.                                *
	 *                                                 *
	 *                                                 *
	 * Format of array is:                             *
	 *    "rule_option" => "dependent_preprocessor"    *
	 *                                                 *
	 * Last Update: 04/05/2013                         *
	 *                                                 *
	 * Added: http_inspect content modifiers and       *
	 *        various "service" metadata values.       *
	 *                                                 *
	 ***************************************************/
	$rule_opts_preprocs = array("ssl_version:" => "ssl_preproc","ssl_state:" => "ssl_preproc",
				    "service ssl" => "ssl_preproc", "service ftp" => "ftp_preprocessor",
				    "service telnet" => "ftp_preprocessor", "service dns" => "dns_preprocessor",
		 		    "dce_iface:" => "dce_rpc_2", "dce_opnum:" => "dce_rpc_2",
				    "dce_stub_data;" => "dce_rpc_2", "sd_pattern:" => "sensitive_data",
				    "sip_method:" => "sip_preproc", "sip_stat_code:" => "sip_preproc",
				    "sip_header;" => "sip_preproc", "sip_body;" => "sip_preproc",
				    "gtp_type:" => "gtp_preproc", "gtp_info:" => "gtp_preproc",
				    "gtp_version:" => "gtp_preproc", "modbus_func:" => "modbus_preproc",
				    "modbus_unit:" => "modbus_preproc", "modbus_data;" => "modbus_preproc",
				    "dnp3_func:" => "dnp3_preproc", "dnp3_obj:" => "dnp3_preproc",
				    "dnp3_ind:" => "dnp3_preproc", "dnp3_data;" => "dnp3_preproc",
				    "http_client_body;" => "http_inspect", "http_cookie;" => "http_inspect",
				    "http_raw_cookie;" => "http_inspect", "http_header;" => "http_inspect",
				    "http_raw_header;" => "http_inspect", "http_method;" => "http_inspect",
				    "http_uri;" => "http_inspect", "http_raw_uri;" => "http_inspect",
				    "http_stat_code;" => "http_inspect", "http_stat_msg;" => "http_inspect",
				    "uricontent:" => "http_inspect", "urilen:" => "http_inspect",
				    "http_encode;" => "http_inspect", "service http" => "http_inspect",
				    "service imap" => "imap_preproc", "service pop2" => "pop_preproc",
				    "service pop3" => "pop_preproc", "service smtp" => "smtp_preprocessor");

	/***************************************************
	 * Iterate the enabled rules, and check for rule   *
	 * options that depend on disabled preprocessors.  *
	 * Disable any of these preprocessor-dependent     *
	 * rules we find.  Once we find at least one       *
	 * reason to disable the rule, stop further checks *
	 * and go to the next rule.                        *
	 ***************************************************/
	foreach ($active_rules as $k1 => $rulem) {
		foreach ($rulem as $k2 => $v) {

			/* If rule is already disabled, skip it. */
			if ($v['disabled'] == 1)
				continue;

			foreach ($rule_opts_preprocs as $opt => $preproc) {
				$pcre = "/\s*\b" . preg_quote($opt) . "/i";
				if (($snortcfg[$preproc] != 'on') && preg_match($pcre, $v['rule'])) {
					$active_rules[$k1][$k2]['rule'] = "# " . $v['rule'];
					$active_rules[$k1][$k2]['disabled'] = 1;
					$disabled_count++;

					/* Accumulate auto-disabled rules for logging */
					$tmp = $active_rules[$k1][$k2]['category'] . ",";
					$tmp .= "{$k1}:{$k2},{$preproc},{$opt}";
					$log_msg[] = $tmp;
					break;
				}
			}
		}
	}

	/* Release memory we no longer need. */
	unset($rulem, $v, $preproc);

	/***************************************************************/
	/* If we are persisting the log from the last pass, then open  */
	/* the log file in append mode.  Otherwise open in overwrite   */
	/* to clear the log in case we have zero disabled rules.       */
	/*							       */
	/* Typically "persist log" mode is used on the second pass     */
	/* when flowbit-required rules are being assessed after the    */
	/* primary enforcing rules have been evaluated.                */
	/***************************************************************/
	$iface = snort_get_friendly_interface($snortcfg['interface']);
	$file = "{$snortlogdir}/{$iface}_disabled_preproc_rules.log";
	if ($persist_log)
		$fp = fopen($file, 'a');
	else
		$fp = fopen($file, 'w');

	/***************************************************/
	/* Log a warning if we auto-disabled any rules     */
	/* just so the user is aware protection is less    */
	/* than optimal with the preprocessors disabled.   */
	/***************************************************/
	if ($disabled_count > 0) {
		log_error(gettext("[Snort] Warning: auto-disabled {$disabled_count} rules due to disabled preprocessor dependencies."));
		natcasesort($log_msg);
		if ($fp) {
			/* Only write the header when not persisting the log */
			if (!$persist_log) {
				@fwrite($fp, "#\n# Run Time: " . date("Y-m-d H:i:s") . "\n#\n");
				@fwrite($fp, "#\n# These rules were auto-disabled because they contain options or operators\n");
				@fwrite($fp, "# dependent on preprocessors that are currently NOT ENABLED on the Preprocessors\n");
				@fwrite($fp, "# tab.  Without these dependent preprocessors enabled, Snort would fail to start\n");
				@fwrite($fp, "# if the rules listed below were enabled.  Therefore the listed rules have been\n");
				@fwrite($fp, "# automatically disabled.  This behavior is controlled by the Auto-Rule Disable\n");
				@fwrite($fp, "# feature on the Preprocessors tab.\n#\n");
				@fwrite($fp, "# WARNING: Using the auto-disable rule feature is not recommended because it can\n");
				@fwrite($fp, "# significantly reduce the threat detection capabilities of Snort!\n#");
				@fwrite($fp, "\n# In the list below, the PREPROCESSOR column is the disabled preprocessor that\n");
				@fwrite($fp, "# triggered the auto-disable of the rule represented by GID:SID.  The RULE OPTION\n");
				@fwrite($fp, "# column shows the specific rule option or content modifier contained within\n");
				@fwrite($fp, "# the rule text that requires the preprocessor be enabled in order to execute.\n#");
				@fwrite($fp, "\n# RULE CATEGORY                 GID:SID     PREPROCESSOR          RULE OPTION\n");
			}
			foreach ($log_msg as $m) {
				$tmp = explode(",", $m);
				@fwrite($fp, sprintf("%-30s  %-10s  %-20s  %s", $tmp[0], $tmp[1], $tmp[2], $tmp[3]) . "\n");
			}
		}
		log_error(gettext("[Snort] See '{$file}' for list of auto-disabled rules."));
		unset($log_msg);
	}
	if ($fp)
		fclose($fp);
}

function snort_generate_conf($snortcfg) {

	global $config, $g, $flowbit_rules_file, $snort_enforcing_rules_file, $rebuild_rules;

	$snortdir = SNORTDIR;
	$snortlibdir = SNORTLIBDIR;
	$snortlogdir = SNORTLOGDIR;

	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	/* See if we should protect and not modify the preprocessor rules files */
	if (!empty($snortcfg['protect_preproc_rules'])) 	
		$protect_preproc_rules = $snortcfg['protect_preproc_rules'];
	else
		$protect_preproc_rules = "off";

	$if_real = snort_get_real_interface($snortcfg['interface']);
	$snort_uuid = $snortcfg['uuid'];
	$snortcfgdir = "{$snortdir}/snort_{$snort_uuid}_{$if_real}";

	/* custom home nets */
	$home_net_list = snort_build_list($snortcfg, $snortcfg['homelistname']);
	$home_net = implode(",", $home_net_list);

	$external_net = '!$HOME_NET';
	if (!empty($snortcfg['externallistname']) && $snortcfg['externallistname'] != 'default') {
		$external_net_list = snort_build_list($snortcfg, $snortcfg['externallistname']);
		$external_net = implode(",", $external_net_list);
	}

	/* user added arguments */
	$snort_config_pass_thru = str_replace("\r", "", base64_decode($snortcfg['configpassthru']));

	/* create a few directories and ensure the sample files are in place */
	$snort_dirs = array( $snortdir, $snortcfgdir, "{$snortcfgdir}/rules",
		"{$snortlogdir}/snort_{$if_real}{$snort_uuid}",
		"{$snortlogdir}/snort_{$if_real}{$snort_uuid}/barnyard2", 
		"{$snortcfgdir}/preproc_rules", 
		"dynamicrules" => "{$snortlibdir}/dynamicrules",
		"dynamicengine" => "{$snortlibdir}/dynamicengine",
		"dynamicpreprocessor" => "{$snortcfgdir}/dynamicpreprocessor"
	    );
	foreach ($snort_dirs as $dir) {
		if (!is_dir($dir))
			safe_mkdir($dir);
	}

	/********************************************************************/
	/* For fail-safe on an initial startup following installation, and  */
	/* before a rules update has occurred, copy the default config      */
	/* files to the interface directory.  If files already exist in     */
	/* the interface directory, or they are newer, that means a rule    */
	/* update has been done and we should leave the customized files    */
	/* put in place by the rules update process.                        */
	/********************************************************************/
	$snort_files = array("gen-msg.map", "classification.config", "reference.config",
			"sid-msg.map", "unicode.map", "threshold.conf", "preproc_rules/preprocessor.rules",
			"preproc_rules/decoder.rules", "preproc_rules/sensitive-data.rules"
		);
	foreach ($snort_files as $file) {
		if (file_exists("{$snortdir}/{$file}")) {
			$ftime = filemtime("{$snortdir}/{$file}");
			if (!file_exists("{$snortcfgdir}/{$file}") || ($ftime > filemtime("{$snortcfgdir}/{$file}")))
				@copy("{$snortdir}/{$file}", "{$snortcfgdir}/{$file}");
		}
	}

	/* define alertsystemlog */
	$alertsystemlog_type = "";
	if ($snortcfg['alertsystemlog'] == "on")
		$alertsystemlog_type = "output alert_syslog: log_alert";

	/* define snortunifiedlog */
	$snortunifiedlog_type = "";
	if ($snortcfg['snortunifiedlog'] == "on")
		$snortunifiedlog_type = "output unified2: filename snort_{$snort_uuid}_{$if_real}.u2, limit 128";

	/* define spoink */
	$spoink_type = "";
	if ($snortcfg['blockoffenders7'] == "on") {
		$pfkill = "";
		if ($snortcfg['blockoffenderskill'] == "on")
			$pfkill = "kill";
		$spoink_wlist = snort_build_list($snortcfg, $snortcfg['whitelistname']);
		/* write whitelist */
		@file_put_contents("{$snortcfgdir}/{$snortcfg['whitelistname']}", implode("\n", $spoink_wlist));
		$spoink_type = "output alert_pf: {$snortcfgdir}/{$snortcfg['whitelistname']},snort2c,{$snortcfg['blockoffendersip']},{$pfkill}";
	}

	/* define selected suppress file */
	$suppress_file_name = "";
	$suppress = snort_find_list($snortcfg['suppresslistname'], 'suppress');
	if (!empty($suppress)) {
		$suppress_data = str_replace("\r", "", base64_decode($suppress['suppresspassthru']));
		@file_put_contents("{$snortcfgdir}/supp{$snortcfg['suppresslistname']}", $suppress_data);
		$suppress_file_name = "include {$snortcfgdir}/supp{$snortcfg['suppresslistname']}";
	}

	/* set the snort performance model */
	$snort_performance = "ac-bnfa";
	if(!empty($snortcfg['performance']))
		$snort_performance = $snortcfg['performance'];

	/* if user has defined a custom ssh port, use it */
	if(is_array($config['system']['ssh']) && isset($config['system']['ssh']['port']))
		$ssh_port = $config['system']['ssh']['port'];
	else
		$ssh_port = "22";
	$snort_ports = array(
		"dns_ports" => "53", "smtp_ports" => "25", "mail_ports" => "25,143,465,691",
		"http_ports" => "80,901,3128,8080,9000", "oracle_ports" => "1521", "mssql_ports" => "1433",
		"telnet_ports" => "23","snmp_ports" => "161", "ftp_ports" => "21",
		"ssh_ports" => $ssh_port, "pop2_ports" => "109", "pop3_ports" => "110", 
		"imap_ports" => "143", "sip_proxy_ports" => "5060:5090,16384:32768",
		"sip_ports" => "5060,5061", "auth_ports" => "113", "finger_ports" => "79", 
		"irc_ports" => "6665,6666,6667,6668,6669,7000", "smb_ports" => "139,445",
		"nntp_ports" => "119", "rlogin_ports" => "513", "rsh_ports" => "514",
		"ssl_ports" => "443,465,563,636,989,990,992,993,994,995",
		"file_data_ports" => "\$HTTP_PORTS,110,143", "shellcode_ports" => "!80", 
		"sun_rpc_ports" => "111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779",
		"DCERPC_NCACN_IP_TCP" => "139,445", "DCERPC_NCADG_IP_UDP" => "138,1024:",
		"DCERPC_NCACN_IP_LONG" => "135,139,445,593,1024:", "DCERPC_NCACN_UDP_LONG" => "135,1024:",
		"DCERPC_NCACN_UDP_SHORT" => "135,593,1024:", "DCERPC_NCACN_TCP" => "2103,2105,2107",
		"DCERPC_BRIGHTSTORE" => "6503,6504", "DNP3_PORTS" => "20000", "MODBUS_PORTS" => "502",
		"GTP_PORTS" => "2123,2152,3386"
	);

	$portvardef = "";
	foreach ($snort_ports as $alias => $avalue) {
		if (!empty($snortcfg["def_{$alias}"]) && is_alias($snortcfg["def_{$alias}"]))
			$snort_ports[$alias] = filter_expand_alias($snortcfg["def_{$alias}"]);
		$snort_ports[$alias] = preg_replace('/\s+/', ',', trim($snort_ports[$alias]));
		$portvardef .= "portvar " . strtoupper($alias) . " [" . $snort_ports[$alias] . "]\n";
	}


	/////////////////////////////
	/* preprocessor code */
	/* def perform_stat */
	$perform_stat = <<<EOD
# Performance Statistics #
preprocessor perfmonitor: time 300 file {$snortlogdir}/snort_{$if_real}{$snort_uuid}/{$if_real}.stats pktcnt 10000
	
EOD;

	/* Pull in the user-configurable HTTP_INSPECT global preprocessor options */
	$http_inspect_memcap = "150994944";
	if (!empty($snortcfg['http_inspect_memcap']))
		$http_inspect_memcap = $snortcfg['http_inspect_memcap'];

	/* Pull in the user-configurable HTTP_INSPECT server preprocessor options */
	$server_flow_depth = '300';
	if ((!empty($snortcfg['server_flow_depth'])) || ($snortcfg['server_flow_depth'] == '0'))
		$server_flow_depth = $snortcfg['server_flow_depth'];
	$http_server_profile = "all";
	if (!empty($snortcfg['http_server_profile']))
		$http_server_profile = $snortcfg['http_server_profile'];
	$client_flow_depth = '300';
	if ((!empty($snortcfg['client_flow_depth'])) || ($snortcfg['client_flow_depth'] == '0'))
		$client_flow_depth = $snortcfg['client_flow_depth'];
	if ($snortcfg['noalert_http_inspect'] == 'on' || empty($snortcfg['noalert_http_inspect']))
		$noalert_http_inspect = "no_alerts";
	else
		$noalert_http_inspect = "";
	$http_inspect_server_opts = "enable_cookie \\\n\textended_response_inspection \\\n\tnormalize_javascript \\\n";
	$http_inspect_server_opts .= "\tinspect_gzip \\\n\tnormalize_utf \\\n\tunlimited_decompress \\\n";
	$http_inspect_server_opts .= "\tnormalize_headers \\\n\tnormalize_cookies";
	if ($snortcfg['http_inspect_enable_xff'] == "on")
		$http_inspect_server_opts .= " \\\n\tenable_xff";

	/* If Stream5 is enabled, then we can enable the "log_uri" and "log_hostname" options */
	if ($snortcfg['stream5_reassembly'] == "on") {
		if ($snortcfg['http_inspect_log_uri'] == "on")
			$http_inspect_server_opts .= " \\\n\tlog_uri";
		if ($snortcfg['http_inspect_log_hostname'] == "on")
			$http_inspect_server_opts .= " \\\n\tlog_hostname";
	}

	$http_ports = str_replace(",", " ", $snort_ports['http_ports']);

	/* def http_inspect */
	$http_inspect = <<<EOD
# HTTP Inspect #
preprocessor http_inspect: global iis_unicode_map unicode.map 1252 compress_depth 65535 decompress_depth 65535 memcap {$http_inspect_memcap}

preprocessor http_inspect_server: server default profile {$http_server_profile} {$noalert_http_inspect} \
	ports { {$http_ports} } \
	http_methods { GET POST PUT SEARCH MKCOL COPY MOVE LOCK UNLOCK NOTIFY POLL BCOPY BDELETE BMOVE LINK UNLINK OPTIONS HEAD DELETE TRACE TRACK CONNECT SOURCE SUBSCRIBE UNSUBSCRIBE PROPFIND PROPPATCH BPROPFIND BPROPPATCH RPC_CONNECT PROXY_SUCCESS BITS_POST CCM_POST SMS_POST RPC_IN_DATA RPC_OUT_DATA RPC_ECHO_DATA } \
	server_flow_depth {$server_flow_depth} \
	client_flow_depth {$client_flow_depth} \
	{$http_inspect_server_opts}

EOD;

	/* def ftp_preprocessor */
	$ftp_preprocessor = <<<EOD
# ftp preprocessor  #
preprocessor ftp_telnet: global \
inspection_type stateless

preprocessor ftp_telnet_protocol: telnet \
   normalize \
   ayt_attack_thresh 200

preprocessor ftp_telnet_protocol: \
    ftp server default \
    def_max_param_len 100 \
    ports { 21 } \
    ftp_cmds { USER PASS ACCT CWD SDUP SMNT QUIT REIN PORT PASV TYPE STRU MODE } \
    ftp_cmds { RETR STOR STOU APPE ALLO REST RNFR RNTO ABOR DELE RMD MKD PWD } \
    ftp_cmds { LIST NLST SITE SYST STAT HELP NOOP } \
    ftp_cmds { AUTH ADAT PROT PBSZ CONF ENC } \
    ftp_cmds { FEAT CEL CMD MACB } \
    ftp_cmds { MDTM REST SIZE MLST MLSD } \
    ftp_cmds { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    alt_max_param_len 0 { CDUP QUIT REIN PASV STOU ABOR PWD SYST NOOP } \
    alt_max_param_len 100 { MDTM CEL XCWD SITE USER PASS REST DELE RMD SYST TEST STAT MACB EPSV CLNT LPRT } \
    alt_max_param_len 200 { XMKD NLST ALLO STOU APPE RETR STOR CMD RNFR HELP } \
    alt_max_param_len 256 { RNTO CWD } \
    alt_max_param_len 400 { PORT } \
    alt_max_param_len 512 { SIZE } \
    chk_str_fmt { USER PASS ACCT CWD SDUP SMNT PORT TYPE STRU MODE } \
    chk_str_fmt { RETR STOR STOU APPE ALLO REST RNFR RNTO DELE RMD MKD } \
    chk_str_fmt { LIST NLST SITE SYST STAT HELP } \
    chk_str_fmt { AUTH ADAT PROT PBSZ CONF ENC } \
    chk_str_fmt { FEAT CEL CMD } \
    chk_str_fmt { MDTM REST SIZE MLST MLSD } \
    chk_str_fmt { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    cmd_validity MODE < char ASBCZ > \
    cmd_validity STRU < char FRP > \
    cmd_validity ALLO < int [ char R int ] > \
    cmd_validity TYPE < { char AE [ char NTC ] | char I | char L [ number ] } > \
    cmd_validity MDTM < [ date nnnnnnnnnnnnnn[.n[n[n]]] ] string > \
    cmd_validity PORT < host_port >

preprocessor ftp_telnet_protocol: ftp client default \
   max_resp_len 256 \
   bounce yes \
   telnet_cmds yes
	
EOD;

	$pop_ports = str_replace(",", " ", $snort_ports['pop3_ports']);
	$pop_preproc = <<<EOD
preprocessor pop: \
	ports { {$pop_ports} } \
        memcap 1310700 \
	qp_decode_depth 0 \
	b64_decode_depth 0 \
	bitenc_decode_depth 0

EOD;

	$imap_ports = str_replace(",", " ", $snort_ports['imap_ports']);
	$imap_preproc = <<<EOD
preprocessor imap: \
	ports { {$imap_ports} } \
        memcap 1310700 \
	qp_decode_depth 0 \
	b64_decode_depth 0 \
	bitenc_decode_depth 0

EOD;

	$smtp_ports = str_replace(",", " ", $snort_ports['mail_ports']);
	/* def smtp_preprocessor */
	$smtp_preprocessor = <<<EOD
# SMTP preprocessor #
preprocessor SMTP: \
    ports { {$smtp_ports} } \
    inspection_type stateful \
    normalize cmds \
    ignore_tls_data \
    valid_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN PIPELINING \
CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    normalize_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN \
PIPELINING CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    max_header_line_len 1000 \ 
    max_response_line_len 512 \
    alt_max_command_line_len 260 { MAIL } \
    alt_max_command_line_len 300 { RCPT } \
    alt_max_command_line_len 500 { HELP HELO ETRN EHLO } \
    alt_max_command_line_len 255 { EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET } \
    alt_max_command_line_len 246 { SEND SAML SOML AUTH TURN ETRN PIPELINING CHUNKING DATA DSN RSET QUIT ONEX } \
    alt_max_command_line_len 246 { QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR } \
    alt_max_command_line_len 246 { XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    xlink2state { enable } \
    log_mailfrom \
    log_rcptto \
    log_email_hdrs \
    email_hdrs_log_depth 1464 \
    log_filename \
    qp_decode_depth 0 \
    b64_decode_depth 0 \
    bitenc_decode_depth 0 \
    uu_decode_depth 0

EOD;

	/* def sf_portscan */
	$sf_pscan_protocol = "all";
	if (!empty($snortcfg['pscan_protocol']))
		$sf_pscan_protocol = $snortcfg['pscan_protocol'];
	$sf_pscan_type = "all";
	if (!empty($snortcfg['pscan_type']))
		$sf_pscan_type = $snortcfg['pscan_type'];
	$sf_pscan_memcap = "10000000";
	if (!empty($snortcfg['pscan_memcap']))
		$sf_pscan_memcap = $snortcfg['pscan_memcap'];
	$sf_pscan_sense_level = "medium";
	if (!empty($snortcfg['pscan_sense_level']))
		$sf_pscan_sense_level = $snortcfg['pscan_sense_level'];
	$sf_pscan_ignore_scanners = "\$HOME_NET";
	if (!empty($snortcfg['pscan_ignore_scanners']) && is_alias($snortcfg['pscan_ignore_scanners'])) {
		$sf_pscan_ignore_scanners = filter_expand_alias($snortcfg['pscan_ignore_scanners']);
		$sf_pscan_ignore_scanners = preg_replace('/\s+/', ',', trim($sf_pscan_ignore_scanners));
	}
	
	$sf_portscan = <<<EOD
# sf Portscan  #
preprocessor sfportscan: scan_type { {$sf_pscan_type} } \
                         proto  { {$sf_pscan_protocol} } \
                         memcap { {$sf_pscan_memcap} } \
                         sense_level { {$sf_pscan_sense_level} } \
                         ignore_scanners { {$sf_pscan_ignore_scanners} }
	
EOD;

	$sun_rpc_ports = str_replace(",", " ", $snort_ports['sun_rpc_ports']);
	/* def other_preprocs */
	$other_preprocs = <<<EOD

# Other preprocs #
preprocessor rpc_decode: {$sun_rpc_ports} no_alert_multiple_requests no_alert_large_fragments no_alert_incomplete

# Back Orifice
preprocessor bo
	
EOD;

	/* def dce_rpc_2 */
	$dce_rpc_2 = <<<EOD
# DCE/RPC 2   #
preprocessor dcerpc2: memcap 102400, events [co]
preprocessor dcerpc2_server: default, policy WinXP, \
    detect [smb [{$snort_ports['smb_ports']}], tcp 135, udp 135, rpc-over-http-server 593], \
    autodetect [tcp 1025:, udp 1025:, rpc-over-http-server 1025:], \
    smb_max_chain 3, smb_invalid_shares ["C$", "D$", "ADMIN$"]
	
EOD;

	$sip_ports = str_replace(",", " ", $snort_ports['sip_ports']);
	$sip_preproc = <<<EOD
# SIP preprocessor
preprocessor sip: ports { {$sip_ports} }, max_call_id_len 300, \
		max_from_len 100, max_to_len 200, max_via_len 1000, \
		max_requestName_len 50, max_uri_len 100, ignore_call_channel,\
		max_content_len 1000

EOD;

	$dns_ports = str_replace(",", " ", $snort_ports['dns_ports']);
	/* def dns_preprocessor */
	$dns_preprocessor = <<<EOD
# DNS preprocessor #
preprocessor dns: \
    ports { {$dns_ports} } \
    enable_rdata_overflow
	
EOD;

	/* def dnp3_preprocessor */
	$dnp3_ports = str_replace(",", " ", $snort_ports['DNP3_PORTS']);
	$dnp3_preproc = <<<EOD
# DNP3 preprocessor #
preprocessor dnp3: \
    ports { {$dnp3_ports} } \
    memcap 262144 \
    check_crc
	
EOD;

	/* def modbus_preprocessor */
	$modbus_ports = str_replace(",", " ", $snort_ports['MODBUS_PORTS']);
	$modbus_preproc = <<<EOD
# Modbus preprocessor #
preprocessor modbus: \
    ports { {$modbus_ports} }
	
EOD;

	/* def gtp_preprocessor */
	$gtp_ports = str_replace(",", " ", $snort_ports['GTP_PORTS']);
	$gtp_preproc = <<<EOD
# GTP preprocessor #
preprocessor gtp: ports { {$gtp_ports} }
	
EOD;

	$def_ssl_ports_ignore = str_replace(",", " ", $snort_ports['ssl_ports']);
	$ssl_preproc = <<<EOD
# Ignore SSL and Encryption  #
preprocessor ssl: ports { {$def_ssl_ports_ignore} }, trustservers, noinspect_encrypted

EOD;

	$sensitive_data = "preprocessor sensitive_data:\n";

	/**************************************************************/
	/* Default the HTTP_INSPECT preprocessor to "on" if not set.  */
	/* The preprocessor is required by hundreds of Snort rules,   */
	/* and without it Snort may not start and/or the number of    */
	/* rules required to be disabled reduces Snort's capability.  */
	/* Alerts from the HTTP_INSPECT preprocessor default to "off" */
	/* unless a specific value has been set by the user.          */
	/**************************************************************/
	if (empty($snortcfg['http_inspect']))
		$snortcfg['http_inspect'] = 'on';

	/* define servers and ports snortdefservers */
	$snort_servers = array (
		"dns_servers" => "\$HOME_NET", "smtp_servers" => "\$HOME_NET", "http_servers" => "\$HOME_NET",
		"www_servers" => "\$HOME_NET", "sql_servers" => "\$HOME_NET", "telnet_servers" => "\$HOME_NET",
		"snmp_servers" => "\$HOME_NET", "ftp_servers" => "\$HOME_NET", "ssh_servers" => "\$HOME_NET",
		"pop_servers" => "\$HOME_NET", "imap_servers" => "\$HOME_NET", "sip_proxy_ip" => "\$HOME_NET",
		"sip_servers" => "\$HOME_NET", "rpc_servers" => "\$HOME_NET", "dnp3_server" => "\$HOME_NET",
		"dnp3_client" => "\$HOME_NET", "modbus_server" => "\$HOME_NET", "modbus_client" => "\$HOME_NET",
		"enip_server" => "\$HOME_NET", "enip_client" => "\$HOME_NET",
		"aim_servers" => "64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24"
	    );

	$vardef = "";
	foreach ($snort_servers as $alias => $avalue) {
		if (!empty($snortcfg["def_{$alias}"]) && is_alias($snortcfg["def_{$alias}"])) {
			$avalue = filter_expand_alias($snortcfg["def_{$alias}"]);
			$avalue = str_replace(" ", ",", trim($avalue));
		}
		$vardef .= "var " . strtoupper($alias) . " [{$avalue}]\n";
	}

	$snort_preproc_libs = array(
		"dce_rpc_2" => "dce2_preproc", "dns_preprocessor" => "dns_preproc", "ftp_preprocessor" => "ftptelnet_preproc", "imap_preproc" => "imap_preproc",
		"pop_preproc" => "pop_preproc", "reputation_preproc" => "reputation_preproc", "sensitive_data" => "sdf_preproc", 
		"sip_preproc" => "sip_preproc", "gtp_preproc" => "gtp_preproc", "smtp_preprocessor" => "smtp_preproc", "ssh_preproc" => "ssh_preproc", 
		"ssl_preproc" => "ssl_preproc", "dnp3_preproc" => "dnp3_preproc", "modbus_preproc" => "modbus_preproc"
	);
	$snort_preproc = array (
		"perform_stat", "http_inspect", "other_preprocs", "ftp_preprocessor", "smtp_preprocessor", "ssl_preproc", "sip_preproc", "gtp_preproc",
		"sf_portscan", "dce_rpc_2", "dns_preprocessor", "sensitive_data", "pop_preproc", "imap_preproc", "dnp3_preproc", "modbus_preproc"
	);
	$default_disabled_preprocs = array(
		"sf_portscan", "gtp_preproc", "sensitive_data", "dnp3_preproc", "modbus_preproc"
	);
	$snort_preprocessors = "";
	foreach ($snort_preproc as $preproc) {
		if ($snortcfg[$preproc] == 'on' || empty($snortcfg[$preproc]) ) {

			/* If preprocessor is not explicitly "on" or "off", then default to "off" if in our default disabled list */
			if (empty($snortcfg[$preproc]) && in_array($preproc, $default_disabled_preprocs))
				continue;

			/* NOTE: The $$ is not a bug. It is an advanced feature of php */
			if (!empty($snort_preproc_libs[$preproc])) {
				$preproclib = "libsf_" . $snort_preproc_libs[$preproc];
				if (!file_exists($snort_dirs['dynamicpreprocessor'] . "{$preproclib}.so")) {
					if (file_exists("{$snortlibdir}/dynamicpreprocessor/{$preproclib}.so")) {
						@copy("{$snortlibdir}/dynamicpreprocessor/{$preproclib}.so", "{$snort_dirs['dynamicpreprocessor']}/{$preproclib}.so");
						$snort_preprocessors .= $$preproc;
						$snort_preprocessors .= "\n";
					} else
						log_error("Could not find the {$preproclib} file. Snort might error out!");
				} else {
					$snort_preprocessors .= $$preproc;
					$snort_preprocessors .= "\n";
				}
			} else {
				$snort_preprocessors .= $$preproc;
				$snort_preprocessors .= "\n";
			}
		}
	}

	$snort_misc_include_rules = "";
	if (file_exists("{$snortcfgdir}/reference.config"))
		$snort_misc_include_rules .= "include {$snortcfgdir}/reference.config\n";
	if (file_exists("{$snortcfgdir}/classification.config"))
		$snort_misc_include_rules .= "include {$snortcfgdir}/classification.config\n";
	if (is_dir("{$snortcfgdir}/preproc_rules")) {
		if ($snortcfg['sensitive_data'] == 'on' && $protect_preproc_rules == "off") {
			$sedcmd = '/^#alert.*classtype:sdf/s/^#//';
			if (file_exists("{$snortcfgdir}/preproc_rules/sensitive-data.rules"))
				$snort_misc_include_rules .= "include \$PREPROC_RULE_PATH/sensitive-data.rules\n";
		} else
			$sedcmd = '/^alert.*classtype:sdf/s/^/#/';
		if (file_exists("{$snortcfgdir}/preproc_rules/decoder.rules") &&
		    file_exists("{$snortcfgdir}/preproc_rules/preprocessor.rules") && $protect_preproc_rules == "off") {
			@file_put_contents("{$g['tmp_path']}/sedcmd", $sedcmd);
			mwexec("/usr/bin/sed -I '' -f {$g['tmp_path']}/sedcmd {$snortcfgdir}/preproc_rules/preprocessor.rules");
			mwexec("/usr/bin/sed -I '' -f {$g['tmp_path']}/sedcmd {$snortcfgdir}/preproc_rules/decoder.rules");
			@unlink("{$g['tmp_path']}/sedcmd");
			$snort_misc_include_rules .= "include \$PREPROC_RULE_PATH/decoder.rules\n";
			$snort_misc_include_rules .= "include \$PREPROC_RULE_PATH/preprocessor.rules\n";
		} else if (file_exists("{$snortcfgdir}/preproc_rules/decoder.rules") &&
			   file_exists("{$snortcfgdir}/preproc_rules/preprocessor.rules") && $protect_preproc_rules == "on") {
			$snort_misc_include_rules .= "include \$PREPROC_RULE_PATH/decoder.rules\n";
			$snort_misc_include_rules .= "include \$PREPROC_RULE_PATH/preprocessor.rules\n";
		}
		else {
			$snort_misc_include_rules .= "config autogenerate_preprocessor_decoder_rules\n";
			log_error("[Snort] Seems preprocessor/decoder rules are missing, enabling autogeneration of them");
		}
	} else {
		$snort_misc_include_rules .= "config autogenerate_preprocessor_decoder_rules\n";
		log_error("[Snort] Seems preprocessor/decoder rules are missing, enabling autogeneration of them");
	}

	/* generate rule sections to load */
	/* The files are always configured so the update process is easier */
	$selected_rules_sections = "include \$RULE_PATH/{$snort_enforcing_rules_file}\n";
	$selected_rules_sections .= "include \$RULE_PATH/{$flowbit_rules_file}\n";
	$selected_rules_sections  .= "include \$RULE_PATH/custom.rules\n";

	/* Create the actual rules files and save in the interface directory */
	snort_prepare_rule_files($snortcfg, $snortcfgdir);

	$cksumcheck = "all";
	if ($snortcfg['cksumcheck'] == 'on')
		$cksumcheck = "none";

	/* Pull in user-configurable options for Frag3 preprocessor settings */
	$frag3_disabled = "";
	if ($snortcfg['frag3_detection'] == "off")
		$frag3_disabled = ", disabled";
	$frag3_memcap = "memcap 4194304";
	if (!empty($snortcfg['frag3_memcap']) || $snortcfg['frag3_memcap'] == "0")
		$frag3_memcap = "memcap {$snortcfg['frag3_memcap']}";
	$frag3_max_frags = "max_frags 8192";
	if (!empty($snortcfg['frag3_max_frags']))
		$frag3_max_frags = "max_frags {$snortcfg['frag3_max_frags']}";
	$frag3_overlap_limit = "overlap_limit 0";
	if (!empty($snortcfg['frag3_overlap_limit']))
		$frag3_overlap_limit = "overlap_limit {$snortcfg['frag3_overlap_limit']}";
	$frag3_min_frag_len = "min_fragment_length 0";
	if (!empty($snortcfg['frag3_min_frag_len']))
		$frag3_min_frag_len = "min_fragment_length {$snortcfg['frag3_min_frag_len']}";
	$frag3_timeout = "timeout 60";
	if (!empty($snortcfg['frag3_timeout']))
		$frag3_timeout = "timeout {$snortcfg['frag3_timeout']}";
	$frag3_policy = "policy bsd";
	if (!empty($snortcfg['frag3_policy']))
		$frag3_policy = "policy {$snortcfg['frag3_policy']}";

	/* Pull in user-configurable options for Stream5 preprocessor settings */
	$stream5_reassembly = "";
	if ($snortcfg['stream5_reassembly'] == "off")
		$stream5_reassembly = "disabled,";
	$stream5_track_tcp = "yes";
	if ($snortcfg['stream5_track_tcp'] =="off")
		$stream5_track_tcp = "no";
	$stream5_track_udp = "yes";
	if ($snortcfg['stream5_track_udp'] =="off")
		$stream5_track_udp = "no";
	$stream5_track_icmp = "no";
	if ($snortcfg['stream5_track_icmp'] =="on")
		$stream5_track_icmp = "yes";
	$stream5_require_3whs = "";
	if ($snortcfg['stream5_require_3whs'] == "on")
		$stream5_require_3whs = ", require_3whs 0";
	$stream5_no_reassemble_async = "";
	if ($snortcfg['stream5_no_reassemble_async'] == "on")
		$stream5_no_reassemble_async = ", dont_reassemble_async";
	$stream5_dont_store_lg_pkts = "";
	if ($snortcfg['stream5_dont_store_lg_pkts'] == "on")
		$stream5_dont_store_lg_pkts = ", dont_store_large_packets";
	$stream5_max_queued_bytes_type = "";
	if ((!empty($snortcfg['max_queued_bytes'])) || ($snortcfg['max_queued_bytes'] == '0'))
		$stream5_max_queued_bytes_type = ", max_queued_bytes {$snortcfg['max_queued_bytes']}";
	$stream5_max_queued_segs_type = "";
	if ((!empty($snortcfg['max_queued_segs'])) || ($snortcfg['max_queued_segs'] == '0'))
		$stream5_max_queued_segs_type = ", max_queued_segs {$snortcfg['max_queued_segs']}";
	$stream5_mem_cap = "";
	if (!empty($snortcfg['stream5_mem_cap']))
		$stream5_mem_cap = ", memcap {$snortcfg['stream5_mem_cap']}";
	$stream5_overlap_limit = "overlap_limit 0";
	if (!empty($snortcfg['stream5_overlap_limit']))
		$stream5_overlap_limit = "overlap_limit {$snortcfg['stream5_overlap_limit']}";
	$stream5_policy = "policy bsd";
	if (!empty($snortcfg['stream5_policy']))
		$stream5_policy = "policy {$snortcfg['stream5_policy']}";
	$stream5_tcp_timeout = "timeout 30";
	if (!empty($snortcfg['stream5_tcp_timeout']))
		$stream5_tcp_timeout = "timeout {$snortcfg['stream5_tcp_timeout']}";
	$stream5_udp_timeout = "timeout 30";
	if (!empty($snortcfg['stream5_udp_timeout']))
		$stream5_udp_timeout = "timeout {$snortcfg['stream5_udp_timeout']}";
	$stream5_icmp_timeout = "timeout 30";
	if (!empty($snortcfg['stream5_icmp_timeout']))
		$stream5_icmp_timeout = "timeout {$snortcfg['stream5_icmp_timeout']}";

	/* build snort configuration file */
	$snort_conf_text = <<<EOD

# snort configuration file
# generated automatically by the pfSense subsystems do not modify manually

# Define Local Network  #
var HOME_NET [{$home_net}]
var EXTERNAL_NET [{$external_net}]

# Define Rule Paths #
var RULE_PATH {$snortcfgdir}/rules
var PREPROC_RULE_PATH {$snortcfgdir}/preproc_rules

# Define Servers  #
{$vardef}

# Define Server Ports  #
{$portvardef}

# Configure the snort decoder  #
config checksum_mode: {$cksumcheck}
config disable_decode_alerts
config disable_tcpopt_experimental_alerts
config disable_tcpopt_obsolete_alerts
config disable_ttcp_alerts
config disable_tcpopt_alerts
config disable_ipopt_alerts
config disable_decode_drops

# Enable the GTP decoder  #
config enable_gtp

# Configure PCRE match limitations
config pcre_match_limit: 3500
config pcre_match_limit_recursion: 1500

# Configure the detection engine  #
config detection: search-method {$snort_performance} search-optimize max-pattern-len 20 max_queue_events 5
config event_queue: max_queue 8 log 5 order_events content_length

# Configure to show year in timestamps
config show_year

# Configure protocol aware flushing  #
# For more information see README.stream5  #
config paf_max: 16000

#Configure dynamically loaded libraries
dynamicpreprocessor directory {$snort_dirs['dynamicpreprocessor']}
dynamicengine directory {$snort_dirs['dynamicengine']}
dynamicdetection directory {$snort_dirs['dynamicrules']}

# Inline packet normalization. For more information, see README.normalize
preprocessor normalize_ip4
preprocessor normalize_tcp: ips ecn stream
preprocessor normalize_icmp4
preprocessor normalize_ip6
preprocessor normalize_icmp6

# Flow and stream #
preprocessor frag3_global: {$frag3_memcap}, {$frag3_max_frags}{$frag3_disabled}
preprocessor frag3_engine: {$frag3_policy} detect_anomalies {$frag3_timeout} {$frag3_overlap_limit} {$frag3_min_frag_len} 

preprocessor stream5_global:{$stream5_reassembly} track_tcp {$stream5_track_tcp}, track_udp {$stream5_track_udp}, track_icmp {$stream5_track_icmp}, max_tcp 262144, max_udp 131072, max_active_responses 2, min_response_seconds 5{$stream5_mem_cap}
preprocessor stream5_tcp: {$stream5_policy}, {$stream5_overlap_limit}, {$stream5_tcp_timeout}, ports both all{$stream5_max_queued_bytes_type}{$stream5_max_queued_segs_type}{$stream5_require_3whs}{$stream5_no_reassemble_async}$stream5_dont_store_lg_pkts
preprocessor stream5_udp: {$stream5_udp_timeout}
preprocessor stream5_icmp: {$stream5_icmp_timeout}

{$snort_preprocessors}

# Snort Output Logs #
output alert_csv: alert timestamp,sig_generator,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,id,classification,priority
{$alertsystemlog_type}
{$snortunifiedlog_type}
{$spoink_type}
						
# Misc Includes #
{$snort_misc_include_rules}

{$suppress_file_name}

# Snort user pass through configuration
{$snort_config_pass_thru}

# Rules Selection #
{$selected_rules_sections}

EOD;

	/* write out snort.conf */
	$conf = fopen("{$snortcfgdir}/snort.conf", "w");
	if(!$conf) {
		log_error("Could not open {$snortcfgdir}/snort.conf for writing.");
		return -1;
	}
	fwrite($conf, $snort_conf_text);
	fclose($conf);
	unset($snort_conf_text, $selected_rules_sections, $suppress_file_name, $snort_misc_include_rules, $spoink_type, $snortunifiedlog_type, $alertsystemlog_type);
	unset($home_net, $external_net, $vardef, $portvardef);
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function snort_sync_on_changes() {
	global $config, $g, $is_postinstall;

	/* Do not attempt a package sync while booting up or installing package */
	if ($g['booting'] || $is_postinstall)
		return;

	if (is_array($config['installedpackages']['snortsync']['config'])){
		$snort_sync=$config['installedpackages']['snortsync']['config'][0];
		$synconchanges = $snort_sync['varsynconchanges'];
		$synctimeout = $snort_sync['varsynctimeout'];
		$syncdownloadrules = $snort_sync['vardownloadrules'];
		switch ($synconchanges){
			case "manual":
				if (is_array($snort_sync[row])){
					$rs=$snort_sync[row];
				}
				else{
					log_error("[snort] xmlrpc sync is enabled but there are no hosts configured as replication targets.");
					return;
				}
				break;
			case "auto":
					if (is_array($config['installedpackages']['carpsettings']) && is_array($config['installedpackages']['carpsettings']['config'])){
						$system_carp=$config['installedpackages']['carpsettings']['config'][0];
						$rs[0]['varsyncipaddress']=$system_carp['synchronizetoip'];
						$rs[0]['varsyncusername']=$system_carp['username'];
						$rs[0]['varsyncpassword']=$system_carp['password'];
						$rs[0]['varsyncsnortstart']="no";
						if ($system_carp['synchronizetoip'] ==""){
							log_error("[snort] xmlrpc sync is enabled but there are no system backup hosts configured as replication targets.");
							return;
						}
					}
					else{
						log_error("[snort] xmlrpc sync is enabled but there are no system backup hosts configured as replication targets.");
						return;
					}
				break;			
			default:
				return;
			break;
		}
		if (is_array($rs)){
			log_error("[snort] Snort pkg xmlrpc sync is starting.");
			foreach($rs as $sh){
				if ($sh['varsyncsnortstart'])
					$syncstartsnort = $sh['varsyncsnortstart'];
				else
					$syncstartsnort = "OFF";
				$sync_to_ip = $sh['varsyncipaddress'];
				$password = $sh['varsyncpassword'];
				if($sh['varsyncusername'])
					$username = $sh['varsyncusername'];
				else
					$username = 'admin';
				if($password && $sync_to_ip)
					snort_do_xmlrpc_sync($syncdownloadrules, $sync_to_ip, $username, $password, $synctimeout, $syncstartsnort);
			}
			log_error("[snort] Snort pkg xmlrpc sync completed.");
		}
 	}
}

/* Do the actual XMLRPC sync */
function snort_do_xmlrpc_sync($syncdownloadrules, $sync_to_ip, $username, $password, $synctimeout, $syncstartsnort) {
	global $config, $g, $is_postinstall;

	/* Do not attempt a package sync while booting up or installing package */
	if ($g['booting'] || $is_postinstall)
		return;

	if(!$username || !$password || !$sync_to_ip) {
		log_error("[snort] A required XMLRPC sync parameter (user, host IP or password) is empty ... aborting pkg sync");
		return;
	}

	/* Test key variables and set defaults if empty */
	if(!$synctimeout)
		$synctimeout=150;

	$xmlrpc_sync_neighbor = $sync_to_ip;
	if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
	}
	$port = $config['system']['webgui']['port'];
	/* if port is empty lets rely on the protocol selection */
	if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
	}
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['snortglobal'] = $config['installedpackages']['snortglobal'];
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("[snort] Beginning Snort pkg configuration XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);

	/* send our XMLRPC message and timeout after defined sync timeout value*/
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting snort XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "snort Settings Sync", "");
	} elseif($resp->faultCode()) {
		$error = "An error code was received while attempting snort XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "snort Settings Sync", "");
	} else {
		log_error("[snort] Snort pkg configuration XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	$downloadrulescmd = "";
	if ($syncdownloadrules == "yes") {
		$downloadrulescmd = "log_error(gettext(\"[snort] XMLRPC pkg sync: Update of downloaded rule sets requested...\"));\n";
		$downloadrulescmd .= "include_once(\"/usr/local/pkg/snort/snort_check_for_rule_updates.php\");\n";
	}
	$snortstart = "";
	if ($syncstartsnort == "ON") {
		$snortstart = "log_error(gettext(\"[snort] XMLRPC pkg sync: Checking Snort status...\"));\n";
		$snortstart .= "if (!is_process_running(\"snort\")) {\n";
		$snortstart .= "log_error(gettext(\"[snort] XMLRPC pkg sync: Snort not running. Sending a start command...\"));\n";
		$snortstart .= "exec(\"/usr/local/etc/rc.d/snort.sh start 2>&1 &\");\n}\n";
		$snortstart .= "else {log_error(gettext(\"[snort] XMLRPC pkg sync: Snort is running...\"));\n}\n";
	}

	/* Build a series of commands for the secondary host to execute that will load the new settings. */
	$execcmd = <<<EOD
	require_once("/usr/local/pkg/snort/snort.inc");
	require_once("service-utils.inc");
	global \$g, \$rebuild_rules, \$snort_gui_include, \$is_postinstall, \$pkg_interface;
	\$orig_pkg_interface = \$pkg_interface;
	\$is_postinstall = true;
	\$snort_gui_include = false;
	\$pkg_interface = "console";
	{$downloadrulescmd}
	\$is_postinstall = false;
	log_error(gettext("[snort] XMLRPC pkg sync: Generating snort.conf file using Master Host's settings..."));
	\$rebuild_rules = "on";
	sync_snort_package_config();
	\$rebuild_rules = "off";
	{$snortstart}
	log_error(gettext("[snort] XMLRPC pkg sync process on this host is complete..."));
	\$pkg_interface = \$orig_pkg_interface;
	return true;

EOD;

	/* assemble xmlrpc payload */
	$method = 'pfsense.exec_php';
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("[snort] Snort XMLRPC sending reload configuration cmd to {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting snort XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "snort Settings Sync", "");
	} elseif($resp->faultCode()) {
		$error = "An error code was received while attempting snort XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "snort Settings Sync", "");
	} else {
		log_error("[snort] Snort pkg XMLRPC reload configuration success with {$url}:{$port} (pfsense.exec_php).");
	}
}

?>
