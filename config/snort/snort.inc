<?php
/*
 * snort.inc
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009-2010 Robert Zelaya
 * Copyright (C) 2011-2012 Ermal Luci
 * Copyright (C) 2013,2014 Bill Meeks
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

// Explicitly declare this as global so it works through function call includes 
global $rebuild_rules, $pfSense_snort_version;

// Grab the Snort binary version programmatically, but if that fails use a safe default
$snortver = array();
exec("/usr/local/bin/snort -V 2>&1 |/usr/bin/grep Version | /usr/bin/cut -c20-26", $snortver);
$snort_version = $snortver[0];
if (empty($snort_version))
	$snort_version = "2.9.6.0";

/* package version */
$pfSense_snort_version = "3.0.5";
$snort_package_version = "Snort {$snort_version} pkg v{$pfSense_snort_version}";

// Define SNORTDIR and SNORTLIBDIR constants according to pfSense version
$pfs_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pfs_version > 2.0) {
	define("SNORTDIR", "/usr/pbi/snort-" . php_uname("m") . "/etc/snort");
	define("SNORTLIBDIR", "/usr/pbi/snort-" . php_uname("m") . "/lib/snort");
}
else {
	define("SNORTDIR", "/usr/local/etc/snort");
	define("SNORTLIBDIR", "/usr/local/lib/snort");
}

/* Define some useful constants for Snort */
/* Be sure to include trailing slash on the URL defines */
define("SNORTLOGDIR", "/var/log/snort");
define("ET_DNLD_FILENAME", "emerging.rules.tar.gz");
define("ETPRO_DNLD_FILENAME", "etpro.rules.tar.gz");
define("GPLV2_DNLD_FILENAME", "community-rules.tar.gz");
define("FLOWBITS_FILENAME", "flowbit-required.rules");
define("ENFORCING_RULES_FILENAME", "snort.rules");
define("RULES_UPD_LOGFILE", SNORTLOGDIR . "/snort_rules_update.log");
define("VRT_FILE_PREFIX", "snort_");
define("GPL_FILE_PREFIX", "GPLv2_");
define("ET_OPEN_FILE_PREFIX", "emerging-");
define("ET_PRO_FILE_PREFIX", "etpro-");
define("IPREP_PATH", "/var/db/snort/iprep/");

/* Rebuild Rules Flag -- if "true", rebuild enforcing rules and flowbit-rules files */
$rebuild_rules = false;

if (!is_array($config['installedpackages']['snortglobal']))
	$config['installedpackages']['snortglobal'] = array();

function snort_is_single_addr_alias($alias) {
	/***************************************************/
	/* This function evaluates the passed Alias to     */
	/* determine if it represents a single IP address, */
	/* or a network in CIDR form, and returns TRUE if  */
	/* the condition is met, and FALSE if not.         */
	/*                                                 */
	/* On Entry: $alias ==> Alias to be evaluated      */
	/*  Returns: TRUE if Alias represents a single     */
	/*           IP address or network, and FALSE      */
	/*           if not.                               */
	/***************************************************/

	/* If spaces in expanded Alias, it's not a single entity */
	if (strpos(trim(filter_expand_alias($alias)), " ") !== false)
		return false;
	else
		return true;
}

function snort_expand_port_range($ports, $delim = ',') {
	/**************************************************/
	/* This function examines the passed ports string */
	/* and expands any embedded port ranges into the  */
	/* individual ports separated by the specified    */
	/* delimiter.  A port range is indicated by a     */
	/* colon in the string.                           */
	/*                                                */
	/* On Entry: $ports ==> string to be evaluated    */
	/*                      with {$delim} separating  */
	/*                      the port values.          */
	/*  Returns: string with any encountered port     */
	/*           ranges expanded and the values       */
	/*           delimited by {$delim}.               */
	/**************************************************/

	$value = "";

	// Split the incoming string on the specified delimiter
	$tmp = explode($delim, $ports);

	// Look for any included port range and expand it
	foreach ($tmp as $val) {
		if (is_portrange($val)) {
			$start = strtok($val, ":");
			$end = strtok(":");
			if ($end !== false) {
				$val = $start . $delim;
				for ($i = intval($start) + 1; $i < intval($end); $i++)
					$val .= strval($i) . $delim;
				$val .= $end;
			}
		}
		$value .= $val . $delim;
	}

	// Remove any trailing delimiter in return value
	return trim($value, $delim);
}

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

function snort_load_suppress_sigs($snortcfg, $track_by=false) {

	global $config;

	/**********************************************************/
	/* This function loads the GEN_ID and SIG_ID for all the  */
	/* suppressed alert entries from the Suppression List of  */
	/* the passed Snort interface.  The results are returned  */
	/* in an array with GEN_ID and SIG_ID as the primary      */
	/* keys.  Any "track by_src" or "track by_dst" entries    */
	/* in the Suppression List are tacked on as additional    */
	/* keys in the array along with the IP address in either  */
	/* IPv4 or IPv6 format when $track_by is passed as true.  */
	/*                                                        */
	/* Sample returned array:                                 */
	/*  $suppress[1][2069] = "suppress"                       */
	/*  $suppress[1][2070]['by_src']['10.1.1.5'] = "suppress" */
	/*  $suppress[1][2070]['by_dst']['10.1.1.6'] = "suppress" */
	/*                                                        */
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

	/***********************************************************/
	/* The default is to build a HOME_NET variable unless      */
	/* '$whitelist' is set to 'true' when calling.             */
	/***********************************************************/

	global $config, $g, $aliastable, $filterdns;
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
		if (!empty($list['address']) && is_alias($list['address']))
			$home_net = explode(" ", trim(filter_expand_alias($list['address'])));
	}

	/* Always add loopback to HOME_NET and whitelist (ftphelper) */
	if (!in_array("127.0.0.1", $home_net))
		$home_net[] = "127.0.0.1";

	/********************************************************************/
	/* Always put the interface running Snort in HOME_NET and whitelist */
	/* unless it's the WAN.  WAN options are handled further down.      */
	/* If the user specifically chose not to include LOCAL_NETS in the  */
	/* WHITELIST, then do not include the Snort interface subnet in the */
	/* WHITELIST. We do include the actual LAN interface IP for Snort,  */
	/* though, to prevent locking out the firewall itself.              */
	/********************************************************************/
	$snortip = get_interface_ip($snortcfg['interface']);
	if (!$whitelist || $localnet == 'yes' || empty($localnet)) {
		if (is_ipaddr($snortip)) {
			if ($snortcfg['interface'] <> "wan") {
				$sn = get_interface_subnet($snortcfg['interface']);
				$ip = gen_subnet($snortip, $sn) . "/{$sn}";
				if (!in_array($ip, $home_net))
					$home_net[] = $ip;
			}
		}
	}
	else {
		if (is_ipaddr($snortip)) {
			if (!in_array($snortip, $home_net))
				$home_net[] = $snortip;
		}
	}

	/* Handle IPv6 if available (2.1 and higher) */
	if (function_exists('get_interface_ipv6')) {
		$snortip = get_interface_ipv6($snortcfg['interface']);
		if (!$whitelist || $localnet == 'yes' || empty($localnet)) {
			if (is_ipaddrv6($snortip)) {
				if ($snortcfg['interface'] <> "wan") {
					$sn = get_interface_subnetv6($snortcfg['interface']);
					$ip = gen_subnetv6($snortip, $sn). "/{$sn}";
					if (!in_array($ip, $home_net))
						$home_net[] = $ip;
				}
			}
		}
		else {
			if (is_ipaddrv6($snortip)) {
				if (!in_array($snortip, $home_net))
					$home_net[] = $snortip;
			}
		}
	}

	if (!$whitelist || $localnet == 'yes' || empty($localnet)) {		
		/*************************************************************************/
		/*  Iterate through the interface list and write out whitelist items and */
		/*  also compile a HOME_NET list of all the local interfaces for snort.  */
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
		if (function_exists("get_interface_gateway_v6")) {
			$gw = get_interface_gateway_v6($snortcfg['interface']);
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

/* checks to see if service is running */
function snort_is_running($snort_uuid, $if_real, $type = 'snort') {
	global $config, $g;

	return isvalidpid("{$g['varrun_path']}/{$type}_{$if_real}{$snort_uuid}.pid");
}

function snort_barnyard_stop($snortcfg, $if_real) {
	global $config, $g;

	$snort_uuid = $snortcfg['uuid'];
	if (isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Barnyard2 STOP for {$snortcfg['descr']}({$if_real})...");
		killbypid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid");
	}
}

function snort_stop($snortcfg, $if_real) {
	global $config, $g;

	$snort_uuid = $snortcfg['uuid'];
	if (isvalidpid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Snort STOP for {$snortcfg['descr']}({$if_real})...");
		killbypid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid");
	}

	snort_barnyard_stop($snortcfg, $if_real);
}

function snort_barnyard_start($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snortlogdir = SNORTLOGDIR;
	$snort_uuid = $snortcfg['uuid'];

	/* define snortbarnyardlog_chk */
	if ($snortcfg['barnyard_enable'] == 'on') {
		log_error("[Snort] Barnyard2 START for {$snortcfg['descr']}({$if_real})...");
		mwexec("/usr/local/bin/barnyard2 -r {$snort_uuid} -f \"snort_{$snort_uuid}_{$if_real}.u2\" --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d {$snortlogdir}/snort_{$if_real}{$snort_uuid} -D -q");
	}
}

function snort_start($snortcfg, $if_real) {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snortlogdir = SNORTLOGDIR;
	$snort_uuid = $snortcfg['uuid'];

	if ($snortcfg['enable'] == 'on') {
		log_error("[Snort] Snort START for {$snortcfg['descr']}({$if_real})...");
		mwexec("/usr/local/bin/snort -R {$snort_uuid} -D -q -l {$snortlogdir}/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}");
	}
	else
		return;

	snort_barnyard_start($snortcfg, $if_real);
}

/**************************************************************/
/* This function sends the passed SIGNAL to the Snort         */
/* instance on the passed interface to cause Snort to reload  */
/* and parse the running configuration without stopping       */
/* packet processing.  It also executes the reload as a       */
/* background process and returns control immediately to the  */
/* caller.                                                    */
/*                                                            */
/*  $signal = SIGHUP (default) parses and reloads config.     */
/*            SIGURG updates Host Attribute Table.            */
/**************************************************************/
function snort_reload_config($snortcfg, $signal="SIGHUP") {
	global $config, $g;

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];
	$if_real = get_real_interface($snortcfg['interface']);

	/******************************************************/
	/* Only send the SIGHUP if Snort is running and we    */
	/* can find a valid PID for the process.              */
	/******************************************************/
	if (isvalidpid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Snort RELOAD CONFIG for {$snortcfg['descr']} ({$if_real})...");
		mwexec_bg("/bin/pkill -{$signal} -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid");
	}
}

function snort_barnyard_reload_config($snortcfg, $signal="HUP") {

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

	$snortdir = SNORTDIR;
	$snort_uuid = $snortcfg['uuid'];
	$if_real = get_real_interface($snortcfg['interface']);

	/******************************************************/
	/* Only send the SIGHUP if Barnyard2 is running and   */
	/* we can find a valid PID for the process.           */
	/******************************************************/
	if (isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid")) {
		log_error("[Snort] Barnyard2 CONFIG RELOAD initiated for {$snortcfg['descr']} ({$if_real})...");
		mwexec_bg("/bin/pkill -{$signal} -F {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid");
	}
}

/*
 this code block is for deleting logs while keeping the newest file,
 snort is linked to these files while running, do not take the easy way out
 by touch and rm, snort will lose sync and not log.
*/
function snort_post_delete_logs($snort_uuid = 0) {
	global $config, $g;

	/* do nothing if no Snort interfaces active */
	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	foreach ($config['installedpackages']['snortglobal']['rule'] as $value) {
		if ($value['uuid'] != $snort_uuid)
			continue;
		$if_real = get_real_interface($value['interface']);
		$snort_log_dir = SNORTLOGDIR . "/snort_{$if_real}{$snort_uuid}";

		if ($if_real != '') {
			/* Clean-up Barnyard2 files if any exist */
			$filelist = glob("{$snort_log_dir}/*{$snort_uuid}_{$if_real}.u2.*");
			unset($filelist[count($filelist) - 1]);
			foreach ($filelist as $file)
				@unlink($file);

			/* Clean-up packet capture files if any exist */
			unlink_if_exists("{$snort_log_dir}/snort.log.*");

			/* Clean-up Barnyard2 archived files if any exist */
			unlink_if_exists("{$snort_log_dir}/barnyard2/archive/*");

			/* Clean-up stats file if enabled */
			if ($value['perform_stat'] == 'on')
				file_put_contents("{$snort_log_dir}/{$if_real}.stats", "");
		}
	}
}

/* This returns size of passed directory or file in 1024-byte blocks */
function snort_Getdirsize($node) {
	if(!is_readable($node))
		return false;

	$blah = exec( "/usr/bin/du -kdc $node" );
	return substr( $blah, 0, strpos($blah, 9) );
}

function snort_snortloglimit_install_cron($should_install=TRUE) {

	install_cron_job("/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/snort/snort_check_cron_misc.inc", $should_install, "*/5");
}

function snort_rm_blocked_install_cron($should_install) {
	global $config, $g;

	$snort_rm_blocked_info_ck = $config['installedpackages']['snortglobal']['rm_blocked'];

	if ($snort_rm_blocked_info_ck == "15m_b") {
		$snort_rm_blocked_min = "*/2";
		$snort_rm_blocked_hr = "*";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "900";
	}
	if ($snort_rm_blocked_info_ck == "30m_b") {
		$snort_rm_blocked_min = "*/5";
		$snort_rm_blocked_hr = "*";
		$snort_rm_blocked_mday = "*";
		$snort_rm_blocked_month = "*";
		$snort_rm_blocked_wday = "*";
		$snort_rm_blocked_expire = "1800";
	}
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

	$command = "/usr/bin/nice -n20 /usr/local/sbin/expiretable -t $snort_rm_blocked_expire snort2c";
	install_cron_job($command, $should_install, $snort_rm_blocked_min, $snort_rm_blocked_hr, $snort_rm_blocked_mday, $snort_rm_blocked_month, $snort_rm_blocked_wday, "root");
}

/* func to install snort update */
function snort_rules_up_install_cron($should_install) {
	global $config, $g;

	$snort_rules_up_info_ck = $config['installedpackages']['snortglobal']['autorulesupdate7'];

	/* See if a customized start time has been set for rule file updates */
	if (!empty($config['installedpackages']['snortglobal']['rule_update_starttime']))
		$snort_rules_upd_time = $config['installedpackages']['snortglobal']['rule_update_starttime'];
	else
		$snort_rules_upd_time = "00:03";

	if ($snort_rules_up_info_ck == "6h_up") {
		$snort_rules_up_min = intval(substr($snort_rules_upd_time, -2));
		$hour = intval(substr($snort_rules_upd_time, 0, 2));
		$snort_rules_up_hr = strval($hour);
		for ($i=0; $i<3; $i++) {
			$hour += 6;
			if ($hour > 24)
				$hour -= 24;
			$snort_rules_up_hr .= "," . strval($hour);
		}
		$snort_rules_up_mday = "*";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "12h_up") {
		$snort_rules_up_min = intval(substr($snort_rules_upd_time, -2));
		$hour = intval(substr($snort_rules_upd_time, 0, 2));
		$snort_rules_up_hr = strval($hour) . ",";
		$hour += 12;
		if ($hour > 24)
			$hour -= 24;
		$snort_rules_up_hr .= strval($hour);
		$snort_rules_up_mday = "*";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "1d_up") {
		$snort_rules_up_min = intval(substr($snort_rules_upd_time, -2));
		$snort_rules_up_hr = intval(substr($snort_rules_upd_time, 0, 2));
		$snort_rules_up_mday = "*/1";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "4d_up") {
		$snort_rules_up_min = intval(substr($snort_rules_upd_time, -2));
		$snort_rules_up_hr = intval(substr($snort_rules_upd_time, 0, 2));
		$snort_rules_up_mday = "*/4";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "7d_up") {
		$snort_rules_up_min = intval(substr($snort_rules_upd_time, -2));
		$snort_rules_up_hr = intval(substr($snort_rules_upd_time, 0, 2));
		$snort_rules_up_mday = "*/7";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}
	if ($snort_rules_up_info_ck == "28d_up") {
		$snort_rules_up_min = intval(substr($snort_rules_upd_time, -2));
		$snort_rules_up_hr = intval(substr($snort_rules_upd_time, 0, 2));
		$snort_rules_up_mday = "*/28";
		$snort_rules_up_month = "*";
		$snort_rules_up_wday = "*";
	}

	$command = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/www/snort/snort_check_for_rule_updates.php";
	install_cron_job($command, $should_install, $snort_rules_up_min, $snort_rules_up_hr, $snort_rules_up_mday, $snort_rules_up_month, $snort_rules_up_wday, "root");
}

/* Only run when all ifaces needed to sync. Expects filesystem rw */
function sync_snort_package_config() {
	global $config, $g;
	global $rebuild_rules;

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
		$if_real = get_real_interface($value['interface']);

		/* create a snort.conf file for interface           */
		snort_generate_conf($value);

		/* create barnyard2.conf file for interface         */
		if ($value['barnyard_enable'] == 'on')
			snort_generate_barnyard2_conf($value, $if_real);
	}

	/* create snort bootup file snort.sh only create once */
	snort_create_rc();

	$snortglob = $config['installedpackages']['snortglobal'];

	snort_snortloglimit_install_cron(true);

	/* set the snort block hosts time IMPORTANT */
	snort_rm_blocked_install_cron($snortglob['rm_blocked'] != "never_b" ? true : false);

	/* set the snort rules update time */
	snort_rules_up_install_cron($snortglob['autorulesupdate7'] != "never_up" ? true : false);

	configure_cron();
	
	/* Do not attempt package sync if reinstalling package or booting */
	if (!$g['snort_postinstall'] && !$g['booting'])
		snort_sync_on_changes();
	
	conf_mount_ro();
}

function snort_build_sid_msg_map($rules_path, $sid_file) {

	/*************************************************************/
	/* This function reads all the rules file in the passed      */
	/* $rules_path variable and produces a properly formatted    */
	/* sid-msg.map v2 file for use by Snort and/or barnyard2.    */
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

	/* First check if we were passed a directory, a single file  */
	/* or an array of filenames to read. Set our $rule_files     */
	/* variable accordingly. If we can't figure it out, return   */
	/* and don't write a sid-msg.map file.                       */
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

	/* Read the rule files into an array, then iterate the list */
	foreach ($rule_files as $file) {

		/* Don't process files with "deleted" in the filename */
		if (stristr($file, "deleted"))
			continue;

		/* Read the file into an array, skipping missing files. */
		if (!file_exists($file))
			continue;

		$rules_array = file($file, FILE_SKIP_EMPTY_LINES);
		$record = "";
		$b_Multiline = false;

		/* Read and process each line from the rules in the current file */
		foreach ($rules_array as $rule) {

			/* Skip any non-rule lines unless we're in multiline mode. */
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
	/* Sort the generated sid-msg map */
	natcasesort($sidMap);

	/* Now print the result to the supplied file */
	@file_put_contents($sid_file, "#v2\n# sid-msg.map file auto-generated by Snort.\n\n");
	@file_put_contents($sid_file, array_values($sidMap), FILE_APPEND);
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
                                        if (!array_key_exists($matches[2], $outMap)) {
						if (!is_array($outMap[$matches[2]]))
							$outMap[$matches[2]] = array();
                                                $outMap[$matches[2]] = trim($matches[3]);
					}
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
                                        if (!array_key_exists($matches[2], $outMap)) {
						if (!is_array($outMap[$matches[2]]))
							$outMap[$matches[2]] = array();
                                                $outMap[$matches[2]] = trim($matches[3]) . "," . trim($matches[4]);
					}
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
	 *  map[gid][sid]['rule']['category']['disabled']['action']['flowbits']
	 *
	 *  where:
	 *   gid      = Generator ID from rule, or 1 if general text 
	 *              rule
	 *   sid      = Signature ID from rule
         *   rule     = Complete rule text
	 *   category = File name of file containing the rule
	 *   disabled = 1 if rule is disabled (commented out), 0 if 
	 *              rule is enabled
	 *   action   = alert|log|pass|drop|reject|sdrop 
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
			if (!preg_match('/^\s*#*\s*(alert|log|pass|drop|reject|sdrop)/i', $rule) && !$b_Multiline)
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

			/* Grab the rule action (this is for a future option) */
			$matches = array();
			if (preg_match('/^\s*#*\s*(alert|log|pass|drop|reject|sdrop)/i', $rule, $matches))
				$map_ref[$gid][$sid]['action'] = $matches[1];
			else
				$map_ref[$gid][$sid]['action'] = "";

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

function snort_get_checked_flowbits($rules_map) {

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

function snort_get_set_flowbits($rules_map) {

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

function snort_find_flowbit_required_rules($rules, $unchecked_flowbits) {

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

function snort_resolve_flowbits($rules, $active_rules) {

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

	$snortdir = SNORTDIR;

	/* Check $rules array to be sure it is filled.    */
	if (empty($rules)) {
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
	$required_rules = snort_find_flowbit_required_rules($rules, $delta_flowbits);

	/* Cleanup and release memory we no longer need.      */
	unset($delta_flowbits);

	return $required_rules;
}

function snort_write_flowbit_rules_file($flowbit_rules, $rule_file) {

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

function snort_load_vrt_policy($policy, $all_rules=null) {

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

	$snortdir = SNORTDIR;
	$vrt_policy_rules = array();

	/* Load a map of all the VRT rules if we were   */
	/* not passed a pre-loaded one to use.          */
	if (is_null($all_rules)) {
		/* Since only Snort VRT rules have IPS Policy metadata, */
		/* limit our search to just those files.                */
		$snort_vrt_files = glob("{$snortdir}/rules/snort_*.rules");
		$all_rules = snort_load_rules_map($snort_vrt_files);
	}

	/* Now walk the rules list and find all those that are */
	/* defined as active for the chosen security policy.   */
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

	/* Return all the rules that match the policy. */
	return $vrt_policy_rules;
}

function snort_write_enforcing_rules_file($rule_map, $rule_path) {

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

function snort_load_sid_mods($sids) {

	/*****************************************/
	/* This function parses the string of    */
	/* GID:SID values in $sids and returns   */
	/* an array with the GID and SID as the  */
	/* keys.  The values in $sids are        */
	/* assumed to be delimited by "||".      */
	/*                                       */
	/* $sids   ==> string of GID:SID values  */
	/*             from the config file.     */
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
	$enablesid = snort_load_sid_mods($snortcfg['rule_sid_on']);
	$disablesid = snort_load_sid_mods($snortcfg['rule_sid_off']);

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

function snort_create_rc() {

/*********************************************************/
/* This function builds the /usr/local/etc/rc.d/snort.sh */
/* shell script for starting and stopping Snort.  The    */
/* script is rebuilt on each package sync operation and  */
/* after any changes to snort.conf saved in the GUI.     */
/*********************************************************/

	global $config, $g, $pfs_version;

	$snortdir = SNORTDIR;
	$snortlogdir = SNORTLOGDIR;
	$rcdir = RCFILEPREFIX;	

	// If no interfaces are configured for Snort, exit
	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;
	$snortconf = $config['installedpackages']['snortglobal']['rule'];
	if (empty($snortconf))
		return;

	// At least one interface is configured, so OK
	$start_snort_iface_start = array();
	$start_snort_iface_stop = array();

	// If not using PBI package, then make sure Barnyard2 can
	// find the latest MySQL shared libs in /usr/local/lib/mysql
	if ($pfs_version < 2.1) {
		$sql_lib_path = "\n# Ensure MySQL shared libs are in ldconfig search path\n";
		$sql_lib_path .= "/sbin/ldconfig -m /usr/local/lib/mysql";
		$start_snort_iface_start[] = $sql_lib_path;	
	}

	// Loop thru each configured interface and build
	// the shell script.
	foreach ($snortconf as $value) {
		// Skip disabled Snort interfaces
		if ($value['enable'] <> 'on')
			continue;
		$snort_uuid = $value['uuid'];
		$if_real = get_real_interface($value['interface']);

		$start_barnyard = <<<EOE

	if [ ! -f {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid ]; then
		pid=`/bin/pgrep -fn "barnyard2 -r {$snort_uuid} "`
	else
		pid=`/bin/pgrep -F {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid`
	fi
	if [ ! -z \$pid ]; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Barnyard2 SOFT RESTART for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/bin/pkill -HUP \$pid
	else
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Barnyard2 START for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/usr/local/bin/barnyard2 -r {$snort_uuid} -f snort_{$snort_uuid}_{$if_real}.u2 --pid-path {$g['varrun_path']} --nolock-pidfile -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d {$snortlogdir}/snort_{$if_real}{$snort_uuid} -D -q
	fi

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
		pid=`/bin/pgrep -fn "barnyard2 -r {$snort_uuid} "`
		if [ ! -z \$pid ]; then
			/bin/pkill -f "barnyard2 -r {$snort_uuid} "
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
		if ($value['barnyard_enable'] == 'on')
			$start_barnyard2 = $start_barnyard;
		else
			$start_barnyard2 = $stop_barnyard2;

		$start_snort_iface_start[] = <<<EOE

###### For Each Iface
	# Start snort and barnyard2
	if [ ! -f {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid ]; then
		pid=`/bin/pgrep -fn "snort -R {$snort_uuid} "`
	else
		pid=`/bin/pgrep -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid`
	fi

	if [ ! -z \$pid ]; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort SOFT RESTART for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/bin/pkill -HUP \$pid
	else
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort START for {$value['descr']}({$snort_uuid}_{$if_real})..."
		/usr/local/bin/snort -R {$snort_uuid} -D -q -l {$snortlogdir}/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c {$snortdir}/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}
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
		pid=`/bin/pgrep -fn "snort -R {$snort_uuid} "`
		if [ ! -z \$pid ]; then
			/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort STOP for {$value['descr']}({$snort_uuid}_{$if_real})..."
			/bin/pkill -fn "snort -R {$snort_uuid} "
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
######## Start of main snort.sh

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

function snort_generate_barnyard2_conf($snortcfg, $if_real) {

	/****************************************************/
	/* This function creates the barnyard2.conf config  */
	/* file for the passed interface when Barnyard2 is  */
	/* enabled.                                         */
	/****************************************************/

	global $config, $g;

	$snort_uuid = $snortcfg['uuid'];
	$snortdir = SNORTDIR;
	$snortcfgdir = SNORTDIR . "/snort_{$snort_uuid}_{$if_real}";
	$snortlogdir = SNORTLOGDIR . "/snort_{$if_real}{$snort_uuid}";

	// Create required directories for barnyard2 if missing
	if (!is_dir("{$snortlogdir}/barnyard2"))
		safe_mkdir("{$snortlogdir}/barnyard2");
	if (!is_dir("{$snortlogdir}/barnyard2/archive"))
		safe_mkdir("{$snortlogdir}/barnyard2/archive");

	// Create the barnyard2 waldo file if missing
	if (!file_exists("{$snortlogdir}/barnyard2/{$snort_uuid}_{$if_real}.waldo")) {
		@touch("{$snortlogdir}/barnyard2/{$snort_uuid}_{$if_real}.waldo");
		mwexec("/bin/chmod 770 {$snortlogdir}/barnyard2/{$snort_uuid}_{$if_real}.waldo", true);
	}

	// If there is no gen-msg.map file present, create an
	// empty one so Barnyard2 will at least start.
	if (!file_exists("{$snortcfgdir}/gen-msg.map"))
		@file_put_contents("{$snortcfgdir}/gen-msg.map", "");

	$snortbarnyard_hostname_info = php_uname("n");

	// Set general config parameters
	$gen_configs = "config quiet\nconfig daemon\nconfig decode_data_link\nconfig alert_with_interface_name\nconfig event_cache_size:    8192";
	if ($snortcfg['barnyard_show_year'] == 'on')
		$gen_configs .= "\nconfig show_year";
	if ($snortcfg['barnyard_obfuscate_ip'] == 'on')
		$gen_configs .= "\nconfig obfuscate";
	if ($snortcfg['barnyard_dump_payload'] == 'on')
		$gen_configs .= "\nconfig dump_payload";
	if ($snortcfg['barnyard_archive_enable'] == 'on')
		$gen_configs .= "\nconfig archivedir:          {$snortlogdir}/barnyard2/archive";

	// Set output plugins
	$snortbarnyardlog_output_plugins = "";
	if ($snortcfg['barnyard_mysql_enable'] == 'on') {
		$by2_dbpwd = base64_decode($snortcfg['barnyard_dbpwd']);
		$snortbarnyardlog_output_plugins .= "# database: log to a MySQL DB\noutput database: log, mysql, ";
		if (isset($snortcfg['barnyard_sensor_name']) && strlen($snortcfg['barnyard_sensor_name']) > 0)
			$snortbarnyardlog_output_plugins .= "sensor_name={$snortcfg['barnyard_sensor_name']}, ";
		$snortbarnyardlog_output_plugins .= "user={$snortcfg['barnyard_dbuser']} password={$by2_dbpwd} ";
		$snortbarnyardlog_output_plugins .= "dbname={$snortcfg['barnyard_dbname']} host={$snortcfg['barnyard_dbhost']}\n\n";
	}
	if ($snortcfg['barnyard_syslog_enable'] == 'on') {
		$snortbarnyardlog_output_plugins .= "# syslog_full: log to a syslog receiver\noutput alert_syslog_full: ";
		if (isset($snortcfg['barnyard_sensor_name']) && strlen($snortcfg['barnyard_sensor_name']) > 0)
			$snortbarnyardlog_output_plugins .= "sensor_name {$snortcfg['barnyard_sensor_name']}, ";
		else
			$snortbarnyardlog_output_plugins .= "sensor_name {$snortbarnyard_hostname_info}, ";
		if ($snortcfg['barnyard_syslog_local'] == 'on')
			$snortbarnyardlog_output_plugins .= "local, log_facility LOG_AUTH, log_priority LOG_INFO\n\n";
		else {
			$snortbarnyardlog_output_plugins .= "server {$snortcfg['barnyard_syslog_rhost']}, protocol {$snortcfg['barnyard_syslog_proto']}, ";
			$snortbarnyardlog_output_plugins .= "port {$snortcfg['barnyard_syslog_dport']}, operation_mode {$snortcfg['barnyard_syslog_opmode']}, ";
			$snortbarnyardlog_output_plugins .= "log_facility {$snortcfg['barnyard_syslog_facility']}, log_priority {$snortcfg['barnyard_syslog_priority']}\n\n";
		}
	}
	if ($snortcfg['barnyard_bro_ids_enable'] == 'on') {
		$snortbarnyardlog_output_plugins .= "# alert_bro: log to a Bro-IDS receiver\n";
		$snortbarnyardlog_output_plugins .= "output alert_bro: {$snortcfg['barnyard_bro_ids_rhost']}:{$snortcfg['barnyard_bro_ids_dport']}\n";
	}

	// Trim leading and trailing newlines and spaces
	$snortbarnyardlog_output_plugins = rtrim($snortbarnyardlog_output_plugins, "\n");	

	// User pass-through arguments
	$snortbarnyardlog_config_pass_thru = str_replace("\r", "", base64_decode($snortcfg['barnconfigpassthru']));

	// Create the conf file as a text string
	$barnyard2_conf_text = <<<EOD

#   barnyard2.conf
#   barnyard2 can be found at http://www.securixlive.com/barnyard2/index.php
#

## General Barnyard2 settings ##
{$gen_configs}
config reference_file:	    {$snortcfgdir}/reference.config
config classification_file: {$snortcfgdir}/classification.config
config sid_file:	    {$snortcfgdir}/sid-msg.map
config gen_file:            {$snortcfgdir}/gen-msg.map
config hostname:            {$snortbarnyard_hostname_info}
config interface:           {$if_real}
config waldo_file:          {$snortlogdir}/barnyard2/{$snort_uuid}_{$if_real}.waldo
config logdir:              {$snortlogdir}

## START user pass through ##
{$snortbarnyardlog_config_pass_thru}
## END user pass through ##

## Setup input plugins ##
input unified2

## Setup output plugins ##
{$snortbarnyardlog_output_plugins}

EOD;

	/* Write out barnyard2_conf text string to disk */
	@file_put_contents("{$snortcfgdir}/barnyard2.conf", $barnyard2_conf_text);
	unset($barnyard2_conf_text);
}

function snort_deinstall() {

	global $config, $g;

	$snortdir = SNORTDIR;
	$snortlibdir = SNORTLIBDIR;
	$snortlogdir = SNORTLOGDIR;
	$rcdir = RCFILEPREFIX;
	$snort_rules_upd_log = RULES_UPD_LOGFILE;
	$iprep_path = IPREP_PATH;

	log_error(gettext("[Snort] Snort package uninstall in progress..."));

	/* Make sure all active Snort processes are terminated */
	/* Log a message only if a running process is detected */
	if (is_service_running("snort"))
		log_error(gettext("[Snort] Snort STOP for all interfaces..."));
	mwexec('/usr/bin/killall -z snort', true);
	sleep(2);
	mwexec('/usr/bin/killall -9 snort', true);
	sleep(2);
	// Delete any leftover snort PID files in /var/run
	unlink_if_exists("/var/run/snort_*.pid");

	/* Make sure all active Barnyard2 processes are terminated */
	/* Log a message only if a running process is detected     */
	if (is_service_running("barnyard2"))
		log_error(gettext("[Snort] Barnyard2 STOP for all interfaces..."));
	mwexec('/usr/bin/killall -z barnyard2', true);
	sleep(2);
	mwexec('/usr/bin/killall -9 barnyard2', true);
	sleep(2);
	// Delete any leftover barnyard2 PID files in /var/run
	unlink_if_exists("/var/run/barnyard2_*.pid");

	/* Remove the snort user and group */
	mwexec('/usr/sbin/pw userdel snort; /usr/sbin/pw groupdel snort', true);

	/* Remove all the Snort cron jobs. */
	install_cron_job("snort2c", false);
	install_cron_job("snort_check_for_rule_updates.php", false);
	install_cron_job("snort_check_cron_misc.inc", false);
	configure_cron();

	/* Remove our associated Dashboard widget config.  If  */
	/* "save settings" is enabled, then save old widget    */
	/* container settings so we can restore them later.    */
	$widgets = $config['widgets']['sequence'];
	if (!empty($widgets)) {
		$widgetlist = explode(",", $widgets);
		foreach ($widgetlist as $key => $widget) {
			if (strstr($widget, "snort_alerts-container")) {
				if ($config['installedpackages']['snortglobal']['forcekeepsettings'] == 'on') {
					$config['installedpackages']['snortglobal']['dashboard_widget'] = $widget;
				}
				unset($widgetlist[$key]);
				break;
			}
		}
		$config['widgets']['sequence'] = implode(",", $widgetlist);
		write_config();
	}

	/* See if we are to clear blocked hosts on uninstall */
	if ($config['installedpackages']['snortglobal']['clearblocks'] == 'on') {
		log_error(gettext("[Snort] Removing all blocked hosts from <snort2c> table..."));
		mwexec("/sbin/pfctl -t snort2c -T flush");
	}

	/* See if we are to clear Snort log files on uninstall */
	if ($config['installedpackages']['snortglobal']['clearlogs'] == 'on') {
		log_error(gettext("[Snort] Clearing all Snort-related log files..."));
		unlink_if_exists("{$snort_rules_upd_log}");
		mwexec("/bin/rm -rf {$snortlogdir}");
	}

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
		mwexec("/bin/rm -rf /usr/local/lib/snort");
	}

	/* Keep this as a last step */
	if ($config['installedpackages']['snortglobal']['forcekeepsettings'] != 'on') {
		log_error(gettext("Not saving settings... all Snort configuration info and logs deleted..."));
		unset($config['installedpackages']['snortglobal']);
		unset($config['installedpackages']['snortsync']);
		unlink_if_exists("{$snort_rules_upd_log}");
		log_error(gettext("[Snort] Flushing <snort2c> firewall table to remove addresses blocked by Snort..."));
		mwexec("/sbin/pfctl -t snort2c -T flush");
		mwexec("/bin/rm -rf {$snortlogdir}");
		mwexec("/bin/rm -rf {$iprep_path}");
		log_error(gettext("[Snort] The package has been removed from this system..."));
	}
}

function snort_prepare_rule_files($snortcfg, $snortcfgdir) {

	/***********************************************************/
	/* This function builds a new set of enforcing rules for   */
	/* Snort and writes them to disk.                          */
	/*                                                         */
	/*    $snortcfg --> pointer to applicable section of       */
	/*                  config.xml containing settings for     */
	/*                  the interface.                         */
	/*                                                         */
	/* $snortcfgdir --> pointer to physical directory on       */
	/*                  disk where Snort configuration is      */
	/*                  to be written.                         */
	/***********************************************************/

	global $g, $rebuild_rules;

	$snortdir = SNORTDIR;
	$flowbit_rules_file = FLOWBITS_FILENAME;
	$snort_enforcing_rules_file = ENFORCING_RULES_FILENAME;
	$no_rules_defined = true;
	$enabled_rules = array();

	/* If there is no reason to rebuild the rules, exit to save time. */
	if (!$rebuild_rules)
		return;

	/* Log a message for rules rebuild in progress */
	log_error(gettext("[Snort] Updating rules configuration for: " . convert_friendly_interface_to_friendly_descr($snortcfg['interface']) . " ..."));

	/* Enable all, some or none of the SDF rules depending on setting. */
	if ($snortcfg['sensitive_data'] == 'on' && $snortcfg['protect_preproc_rules'] != 'on') {
		if (file_exists(SNORTDIR."/preproc_rules/sensitive-data.rules")) {
			$sdf_alert_pattern="(".preg_replace("/,/","|",$snortcfg['sdf_alert_data_type']).")";
			$sd_tmp_file=file(SNORTDIR."/preproc_rules/sensitive-data.rules");
			$sd_tmp_new_file="";
			foreach ($sd_tmp_file as $sd_tmp_line)
				$sd_tmp_new_file.=preg_match("/$sdf_alert_pattern/i",$sd_tmp_line) ? $sd_tmp_line : "";
			file_put_contents("{$snortcfgdir}/preproc_rules/sensitive-data.rules",$sd_tmp_new_file,LOCK_EX);
		}
	}
	elseif ($snortcfg['sensitive_data'] != 'on' && $snortcfg['protect_preproc_rules'] != 'on') {
		/* Setting is "off", so disable all SDF rules. */
		$sedcmd = '/^alert.*classtype:sdf/s/^/#/';
		@file_put_contents("{$g['tmp_path']}/sedcmd", $sedcmd);
		mwexec("/usr/bin/sed -I '' -f {$g['tmp_path']}/sedcmd {$snortcfgdir}/preproc_rules/sensitive-data.rules");
		@unlink("{$g['tmp_path']}/sedcmd");
	}

	/* Load the decoder, preprocessor and sensitive-data  */
	/* rules from the interface's preproc_rule directory  */
	/* into the $enabled_rules array.                     */
	$enabled_rules = snort_load_rules_map("{$snortcfgdir}/preproc_rules/");

	/* Only rebuild rules if some are selected or an IPS Policy is enabled  */
	if (!empty($snortcfg['rulesets']) || $snortcfg['ips_policy_enable'] == 'on') {
		$enabled_files = array();
		$all_rules = array();
		$no_rules_defined = false;

		/* Load up all the text rules into a Rules Map array. */
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
						$enabled_rules[$k1][$k2]['action'] = $v['action'];
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
			$policy_rules = snort_load_vrt_policy($snortcfg['ips_policy'], $all_rules);
			foreach ($policy_rules as $k1 => $policy) {
				foreach ($policy as $k2 => $p) {
					if (!is_array($enabled_rules[$k1]))
						$enabled_rules[$k1] = array();
					if (!is_array($enabled_rules[$k1][$k2]))
						$enabled_rules[$k1][$k2] = array();
					$enabled_rules[$k1][$k2]['rule'] = $p['rule'];
					$enabled_rules[$k1][$k2]['category'] = $p['category'];
					$enabled_rules[$k1][$k2]['disabled'] = $p['disabled'];
					$enabled_rules[$k1][$k2]['action'] = $p['action'];
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
			log_error('[Snort] Checking for rules dependent on disabled preprocessors for: ' . convert_friendly_interface_to_friendly_descr($snortcfg['interface']) . '...');
			snort_filter_preproc_rules($snortcfg, $enabled_rules);
		}

		/* Write the enforcing rules file to the Snort interface's "rules" directory. */
		snort_write_enforcing_rules_file($enabled_rules, "{$snortcfgdir}/rules/{$snort_enforcing_rules_file}");

		/* If auto-flowbit resolution is enabled, generate the dependent flowbits rules file. */
		if ($snortcfg['autoflowbitrules'] == 'on') {
			log_error('[Snort] Enabling any flowbit-required rules for: ' . convert_friendly_interface_to_friendly_descr($snortcfg['interface']) . '...');
			$fbits = snort_resolve_flowbits($all_rules, $enabled_rules);

			/* Check for and disable any flowbit-required rules the user has  */
			/* manually forced to a disabled state.                           */
			snort_modify_sids($fbits, $snortcfg);

			/* Check for and disable any flowbit-required rules dependent upon     */
			/* disabled preprocessors if this option is enabled for the interface. */
			if ($snortcfg['preproc_auto_rule_disable'] == "on") {
				log_error('[Snort] Checking flowbit rules dependent on disabled preprocessors for: ' . convert_friendly_interface_to_friendly_descr($snortcfg['interface']) . '...');
				snort_filter_preproc_rules($snortcfg, $fbits, true);
			}
			snort_write_flowbit_rules_file($fbits, "{$snortcfgdir}/rules/{$flowbit_rules_file}");
			unset($fbits);
		} else
			/* Just put an empty file to always have the file present */
			snort_write_flowbit_rules_file(array(), "{$snortcfgdir}/rules/{$flowbit_rules_file}");
	} else {
		/* No regular rules or policy were selected, so just use the decoder and preproc rules */
		snort_write_enforcing_rules_file($enabled_rules, "{$snortcfgdir}/rules/{$snort_enforcing_rules_file}");
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
		log_error(gettext("[Snort] Warning - no text rules or IPS-Policy selected for: " . convert_friendly_interface_to_friendly_descr($snortcfg['interface']) . " ..."));

	/* Build a new sid-msg.map file from the enabled */
	/* rules and copy it to the interface directory. */
	log_error(gettext("[Snort] Building new sig-msg.map file for " . convert_friendly_interface_to_friendly_descr($snortcfg['interface']) . "..."));
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
	/* $snortcfg     --> config parameters array for  */
	/*                   the interface.               */
	/*                                                */
	/* $active_rules --> rules_map array of enabled   */
	/*                   rules for the interface.     */
	/*                                                */
	/* $persist_log  --> flag indicating if new log   */
	/*                   file should be created or    */
	/*                   the existing one appended    */
	/*                   to.                          */
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
	$iface = convert_friendly_interface_to_friendly_descr($snortcfg['interface']);
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

	global $config, $g, $rebuild_rules;

	$snortdir = SNORTDIR;
	$snortlibdir = SNORTLIBDIR;
	$snortlogdir = SNORTLOGDIR;
	$flowbit_rules_file = FLOWBITS_FILENAME;
	$snort_enforcing_rules_file = ENFORCING_RULES_FILENAME;

	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	conf_mount_rw();

	/* See if we should protect and not modify the preprocessor rules files */
	if (!empty($snortcfg['protect_preproc_rules'])) 	
		$protect_preproc_rules = $snortcfg['protect_preproc_rules'];
	else
		$protect_preproc_rules = "off";

	$if_real = get_real_interface($snortcfg['interface']);
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
	// Remove the trailing newline
	$snort_config_pass_thru = rtrim($snort_config_pass_thru);

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
	$snort_files = array("gen-msg.map", "classification.config", "reference.config", "attribute_table.dtd", 
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
	if ($snortcfg['barnyard_enable'] == "on") {
		if (isset($snortcfg['unified2_log_limit']))
			$u2_log_limit = "limit {$snortcfg['unified2_log_limit']}";
		else
			$u2_log_limit = "limit 128";

		$snortunifiedlog_type = "output unified2: filename snort_{$snort_uuid}_{$if_real}.u2, {$u2_log_limit}";
		if ($snortcfg['barnyard_log_vlan_events'] == 'on')
			$snortunifiedlog_type .= ", vlan_event_types";
		if ($snortcfg['barnyard_log_mpls_events'] == 'on')
			$snortunifiedlog_type .= ", mpls_event_types";
	}

	/* define spoink */
	$spoink_type = "";
	if ($snortcfg['blockoffenders7'] == "on") {
		$pfkill = "";
		if ($snortcfg['blockoffenderskill'] == "on")
			$pfkill = "kill";
		$spoink_wlist = snort_build_list($snortcfg, $snortcfg['whitelistname'], true);
		/* write Pass List */
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

	/* Define an array of default values for the various preprocessor ports */
	$snort_ports = array(
		"dns_ports" => "53", "smtp_ports" => "25", "mail_ports" => "25,465,587,691",
		"http_ports" => "36,80,81,82,83,84,85,86,87,88,89,90,311,383,591,593,631,901,1220,1414,1533,1741,1830,2301,2381,2809,3037,3057,3128,3443,3702,4343,4848,5250,6080,6988,7000,7001,7144,7145,7510,7777,7779,8000,8008,8014,8028,8080,8081,8082,8085,8088,8090,8118,8123,8180,8181,8222,8243,8280,8300,8500,8800,8888,8899,9000,9060,9080,9090,9091,9443,9999,10000,11371,15489,29991,33300,34412,34443,34444,41080,44440,50000,50002,51423,55555,56712", 
		"oracle_ports" => "1024:", "mssql_ports" => "1433", "telnet_ports" => "23", 
		"snmp_ports" => "161", "ftp_ports" => "21,2100,3535", "ssh_ports" => $ssh_port, 
		"pop2_ports" => "109", "pop3_ports" => "110", "imap_ports" => "143", 
		"sip_ports" => "5060,5061,5600", "auth_ports" => "113", "finger_ports" => "79", 
		"irc_ports" => "6665,6666,6667,6668,6669,7000", "smb_ports" => "139,445",
		"nntp_ports" => "119", "rlogin_ports" => "513", "rsh_ports" => "514",
		"ssl_ports" => "443,465,563,636,989,992,993,994,995,7801,7802,7900,7901,7902,7903,7904,7905,7906,7907,7908,7909,7910,7911,7912,7913,7914,7915,7916,7917,7918,7919,7920",
		"file_data_ports" => "\$HTTP_PORTS,110,143", "shellcode_ports" => "!80", 
		"sun_rpc_ports" => "111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779",
		"DCERPC_NCACN_IP_TCP" => "139,445", "DCERPC_NCADG_IP_UDP" => "138,1024:",
		"DCERPC_NCACN_IP_LONG" => "135,139,445,593,1024:", "DCERPC_NCACN_UDP_LONG" => "135,1024:",
		"DCERPC_NCACN_UDP_SHORT" => "135,593,1024:", "DCERPC_NCACN_TCP" => "2103,2105,2107",
		"DCERPC_BRIGHTSTORE" => "6503,6504", "DNP3_PORTS" => "20000", "MODBUS_PORTS" => "502",
		"GTP_PORTS" => "2123,2152,3386"
	);

	/* Check for defined Aliases that may override default port settings as we build the portvars array */
	$portvardef = "";
	foreach ($snort_ports as $alias => $avalue) {
		if (!empty($snortcfg["def_{$alias}"]) && is_alias($snortcfg["def_{$alias}"]))
			$snort_ports[$alias] = trim(filter_expand_alias($snortcfg["def_{$alias}"]));
		$snort_ports[$alias] = preg_replace('/\s+/', ',', trim($snort_ports[$alias]));
		$portvardef .= "portvar " . strtoupper($alias) . " [" . $snort_ports[$alias] . "]\n";
	}

	/* Define the default ports for the Stream5 preprocessor (formatted for easier reading in the snort.conf file) */
	$stream5_ports_client = "21 22 23 25 42 53 70 79 109 110 111 113 119 135 136 137 \\\n";
	$stream5_ports_client .= "\t             139 143 161 445 513 514 587 593 691 1433 1521 1741 \\\n";
	$stream5_ports_client .= "\t             2100 3306 6070 6665 6666 6667 6668 6669 7000 8181 \\\n";
	$stream5_ports_client .= "\t             32770 32771 32772 32773 32774 32775 32776 32777 \\\n";
	$stream5_ports_client .= "\t             32778 32779";
        $stream5_ports_both = "80 81 82 83 84 85 86 87 88 89 90 110 311 383 443 465 563 \\\n";
	$stream5_ports_both .= "\t           591 593 631 636 901 989 992 993 994 995 1220 1414 1533 \\\n";
	$stream5_ports_both .= "\t           1830 2301 2381 2809 3037 3057 3128 3443 3702 4343 4848 \\\n";
	$stream5_ports_both .= "\t           5250 6080 6988 7907 7000 7001 7144 7145 7510 7802 7777 \\\n";
	$stream5_ports_both .= "\t           7779 7801 7900 7901 7902 7903 7904 7905 7906 7908 7909 \\\n";
	$stream5_ports_both .= "\t           7910 7911 7912 7913 7914 7915 7916 7917 7918 7919 7920 \\\n";
	$stream5_ports_both .= "\t           8000 8008 8014 8028 8080 8081 8082 8085 8088 8090 8118 \\\n";
	$stream5_ports_both .= "\t           8123 8180 8222 8243 8280 8300 8500 8800 8888 8899 9000 \\\n";
	$stream5_ports_both .= "\t           9060 9080 9090 9091 9443 9999 10000 11371 15489 29991 \\\n";
	$stream5_ports_both .= "\t           33300 34412 34443 34444 41080 44440 50000 50002 51423 \\\n";
	$stream5_ports_both .= "\t           55555 56712";

	/////////////////////////////
	/* preprocessor code */
	/* def perform_stat */
	$perform_stat = <<<EOD
# Performance Statistics #
preprocessor perfmonitor: time 300 file {$snortlogdir}/snort_{$if_real}{$snort_uuid}/{$if_real}.stats pktcnt 10000
	
EOD;

	/* def ftp_preprocessor */
	$telnet_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['telnet_ports']));
	$ftp_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['ftp_ports']));

	// Configure FTP_Telnet global options
	$ftp_telnet_globals = "inspection_type ";
	if ($snortcfg['ftp_telnet_inspection_type'] != "") { $ftp_telnet_globals .= $snortcfg['ftp_telnet_inspection_type']; }else{ $ftp_telnet_globals .= "stateful"; }
	if ($snortcfg['ftp_telnet_alert_encrypted'] == "on")
		$ftp_telnet_globals .= " \\\n\tencrypted_traffic yes";
	else
		$ftp_telnet_globals .= " \\\n\tencrypted_traffic no";
	if ($snortcfg['ftp_telnet_check_encrypted'] == "on")
		$ftp_telnet_globals .= " \\\n\tcheck_encrypted";

	// Configure FTP_Telnet Telnet protocol options
	$ftp_telnet_protocol = "ports { {$telnet_ports} }";
	if ($snortcfg['ftp_telnet_normalize'] == "on")
		$ftp_telnet_protocol .= " \\\n\tnormalize";
	if ($snortcfg['ftp_telnet_detect_anomalies'] == "on")
		$ftp_telnet_protocol .= " \\\n\tdetect_anomalies";
	if ($snortcfg['ftp_telnet_ayt_attack_threshold'] <> '0') {
		$ftp_telnet_protocol .= " \\\n\tayt_attack_thresh "; 
		if ($snortcfg['ftp_telnet_ayt_attack_threshold'] != "")
			$ftp_telnet_protocol .= $snortcfg['ftp_telnet_ayt_attack_threshold'];
		else
			$ftp_telnet_protocol .= "20";
	}

	// Setup the standard FTP commands used for all FTP Server engines
	$ftp_cmds = <<<EOD
	ftp_cmds { ABOR ACCT ADAT ALLO APPE AUTH CCC CDUP } \
	ftp_cmds { CEL CLNT CMD CONF CWD DELE ENC EPRT } \
	ftp_cmds { EPSV ESTA ESTP FEAT HELP LANG LIST LPRT } \
	ftp_cmds { LPSV MACB MAIL MDTM MFMT MIC MKD MLSD MLST } \
	ftp_cmds { MODE NLST NOOP OPTS PASS PASV PBSZ PORT } \
	ftp_cmds { PROT PWD QUIT REIN REST RETR RMD RNFR } \
	ftp_cmds { RNTO SDUP SITE SIZE SMNT STAT STOR STOU } \
	ftp_cmds { STRU SYST TEST TYPE USER XCUP XCRC XCWD } \
	ftp_cmds { XMAS XMD5 XMKD XPWD XRCP XRMD XRSQ XSEM } \
	ftp_cmds { XSEN XSHA1 XSHA256 } \
	alt_max_param_len 0 { ABOR CCC CDUP ESTA FEAT LPSV NOOP PASV PWD QUIT REIN STOU SYST XCUP XPWD } \
	alt_max_param_len 200 { ALLO APPE CMD HELP NLST RETR RNFR STOR STOU XMKD } \
	alt_max_param_len 256 { CWD RNTO } \
	alt_max_param_len 400 { PORT } \
	alt_max_param_len 512 { MFMT SIZE } \
	chk_str_fmt { ACCT ADAT ALLO APPE AUTH CEL CLNT CMD } \
	chk_str_fmt { CONF CWD DELE ENC EPRT EPSV ESTP HELP } \
	chk_str_fmt { LANG LIST LPRT MACB MAIL MDTM MIC MKD } \
	chk_str_fmt { MLSD MLST MODE NLST OPTS PASS PBSZ PORT } \
	chk_str_fmt { PROT REST RETR RMD RNFR RNTO SDUP SITE } \
	chk_str_fmt { SIZE SMNT STAT STOR STRU TEST TYPE USER } \
	chk_str_fmt { XCRC XCWD XMAS XMD5 XMKD XRCP XRMD XRSQ } \ 
	chk_str_fmt { XSEM XSEN XSHA1 XSHA256 } \
	cmd_validity ALLO < int [ char R int ] > \    
	cmd_validity EPSV < [ { char 12 | char A char L char L } ] > \
	cmd_validity MACB < string > \
	cmd_validity MDTM < [ date nnnnnnnnnnnnnn[.n[n[n]]] ] string > \
	cmd_validity MODE < char ASBCZ > \
	cmd_validity PORT < host_port > \
	cmd_validity PROT < char CSEP > \
	cmd_validity STRU < char FRPO [ string ] > \    
	cmd_validity TYPE < { char AE [ char NTC ] | char I | char L [ number ] } >

EOD;

	// Configure all the FTP_Telnet FTP protocol options
	// Iterate and configure the FTP Client engines
	$ftp_default_client_engine = array( "name" => "default", "bind_to" => "all", "max_resp_len" => 256, 
					    "telnet_cmds" => "no", "ignore_telnet_erase_cmds" => "yes", 
					    "bounce" => "yes", "bounce_to_net" => "", "bounce_to_port" => "" );

	if (!is_array($snortcfg['ftp_client_engine']['item']))
		$snortcfg['ftp_client_engine']['item'] = array();

	// If no FTP client engine is configured, use the default
	// to keep from breaking Snort.
	if (empty($snortcfg['ftp_client_engine']['item']))
			$snortcfg['ftp_client_engine']['item'][] = $ftp_default_client_engine;
	$ftp_client_engine = "";

	foreach ($snortcfg['ftp_client_engine']['item'] as $f => $v) {
		$buffer = "preprocessor ftp_telnet_protocol: ftp client ";
		if ($v['name'] == "default" && $v['bind_to'] == "all")
			$buffer .= "default \\\n";
		elseif (is_alias($v['bind_to'])) {
			$tmp = trim(filter_expand_alias($v['bind_to']));
			if (!empty($tmp)) {
				$tmp = preg_replace('/\s+/', ' ', $tmp);
				$buffer .= "{$tmp} \\\n";
			}
			else {
				log_error("[snort] ERROR: unable to resolve IP Address Alias '{$v['bind_to']}' for FTP client '{$v['name']}' ... skipping entry.");
				continue;
			}
		}
		else {
				log_error("[snort] ERROR: unable to resolve IP Address Alias '{$v['bind_to']}' for FTP client '{$v['name']}' ... skipping entry.");
				continue;
		}

		if ($v['max_resp_len'] == "")
			$buffer .= "\tmax_resp_len 256 \\\n";
		else
			$buffer .= "\tmax_resp_len {$v['max_resp_len']} \\\n";

		$buffer .= "\ttelnet_cmds {$v['telnet_cmds']} \\\n";
		$buffer .= "\tignore_telnet_erase_cmds {$v['ignore_telnet_erase_cmds']} \\\n";

		if ($v['bounce'] == "yes") {
			if (is_alias($v['bounce_to_net']) && is_alias($v['bounce_to_port'])) {
				$net = trim(filter_expand_alias($v['bounce_to_net']));
				$port = trim(filter_expand_alias($v['bounce_to_port']));
				if (!empty($net) && !empty($port) && 
				     snort_is_single_addr_alias($v['bounce_to_net']) && 
				    (is_port($port) || is_portrange($port))) {
					$port = preg_replace('/\s+/', ',', $port);
					// Change port range delimiter to comma for ftp_telnet client preprocessor
					if (is_portrange($port))
						$port = str_replace(":", ",", $port);
					$buffer .= "\tbounce yes \\\n";
					$buffer .= "\tbounce_to { {$net},{$port} }\n";
				}
				else {
					// One or both of the BOUNCE_TO alias values is not right,
					// so figure out which and log an appropriate error.
					if (empty($net) || !snort_is_single_addr_alias($v['bounce_to_net']))
						log_error("[snort] ERROR: illegal value for bounce_to Address Alias [{$v['bounce_to_net']}] for FTP client engine [{$v['name']}] ... omitting 'bounce_to' option for this client engine.");
					if (empty($port) || !(is_port($port) || is_portrange($port)))
						log_error("[snort] ERROR: illegal value for bounce_to Port Alias [{$v['bounce_to_port']}] for FTP client engine [{$v['name']}] ... omitting 'bounce_to' option for this client engine.");
					$buffer .= "\tbounce yes\n";
				}
			}
			else
				$buffer .= "\tbounce yes\n";
		}
		else
			$buffer .= "\tbounce no\n";

		// Add this FTP client engine to the master string
		$ftp_client_engine .= "{$buffer}\n";
	}
	// Trim final trailing newline
	rtrim($ftp_client_engine);	

	// Iterate and configure the FTP Server engines
	$ftp_default_server_engine = array( "name" => "default", "bind_to" => "all", "ports" => "default", 
					    "telnet_cmds" => "no", "ignore_telnet_erase_cmds" => "yes", 
					    "ignore_data_chan" => "no", "def_max_param_len" => 100 );

	if (!is_array($snortcfg['ftp_server_engine']['item']))
		$snortcfg['ftp_server_engine']['item'] = array();

	// If no FTP server engine is configured, use the default
	// to keep from breaking Snort.
	if (empty($snortcfg['ftp_server_engine']['item']))
			$snortcfg['ftp_server_engine']['item'][] = $ftp_default_server_engine;
	$ftp_server_engine = "";

	foreach ($snortcfg['ftp_server_engine']['item'] as $f => $v) {
		$buffer = "preprocessor ftp_telnet_protocol: ftp server ";
		if ($v['name'] == "default" && $v['bind_to'] == "all")
			$buffer .= "default \\\n";
		elseif (is_alias($v['bind_to'])) {
			$tmp = trim(filter_expand_alias($v['bind_to']));
			if (!empty($tmp)) {
				$tmp = preg_replace('/\s+/', ' ', $tmp);
				$buffer .= "{$tmp} \\\n";
			}
			else {
				log_error("[snort] ERROR: unable to resolve IP Address Alias '{$v['bind_to']}' for FTP server '{$v['name']}' ... skipping entry.");
				continue;
			}
		}
		else {
				log_error("[snort] ERROR: unable to resolve IP Address Alias '{$v['bind_to']}' for FTP server '{$v['name']}' ... skipping entry.");
				continue;
		}

		if ($v['def_max_param_len'] == "")
			$buffer .= "\tdef_max_param_len 100 \\\n";
		elseif ($v['def_max_param_len'] <> '0')
			$buffer .= "\tdef_max_param_len {$v['def_max_param_len']} \\\n";

		if ($v['ports'] == "default" || !is_alias($v['ports']) || empty($v['ports']))
			$buffer .= "\tports { {$ftp_ports} } \\\n";
		elseif (is_alias($v['ports'])) {
			$tmp = trim(filter_expand_alias($v['ports']));
			if (!empty($tmp)) {
				$tmp = preg_replace('/\s+/', ' ', $tmp);
				$tmp = snort_expand_port_range($tmp, ' ');
				$buffer .= "\tports { {$tmp} } \\\n";
			}
			else {
				log_error("[snort] ERROR: unable to resolve Port Alias '{$v['ports']}' for FTP server '{$v['name']}' ... reverting to defaults.");
				$buffer .= "\tports { {$ftp_ports} } \\\n";
			}
		}

		$buffer .= "\ttelnet_cmds {$v['telnet_cmds']} \\\n";
		$buffer .= "\tignore_telnet_erase_cmds {$v['ignore_telnet_erase_cmds']} \\\n";
		if ($v['ignore_data_chan'] == "yes")
			$buffer .= "\tignore_data_chan yes \\\n";
		$buffer .= "{$ftp_cmds}\n";

		// Add this FTP server engine to the master string
		$ftp_server_engine .= $buffer;
	}
	// Remove trailing newlines
	rtrim($ftp_server_engine);

	$ftp_preprocessor = <<<EOD
# ftp_telnet preprocessor #
preprocessor ftp_telnet: global \
	{$ftp_telnet_globals}

preprocessor ftp_telnet_protocol: telnet \
	{$ftp_telnet_protocol}

{$ftp_server_engine}
{$ftp_client_engine}
EOD;

	$pop_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['pop3_ports']));
	$pop_preproc = <<<EOD
# POP preprocessor #
preprocessor pop: \
	ports { {$pop_ports} } \
	memcap 1310700 \
	qp_decode_depth 0 \
	b64_decode_depth 0 \
	bitenc_decode_depth 0

EOD;

	$imap_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['imap_ports']));
	$imap_preproc = <<<EOD
# IMAP preprocessor #
preprocessor imap: \
	ports { {$imap_ports} } \
	memcap 1310700 \
	qp_decode_depth 0 \
	b64_decode_depth 0 \
	bitenc_decode_depth 0

EOD;

	$smtp_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['mail_ports']));
	/* def smtp_preprocessor */
	$smtp_preprocessor = <<<EOD
# SMTP preprocessor #
preprocessor SMTP: \
	ports { {$smtp_ports} } \
	inspection_type stateful \
	normalize cmds \
	ignore_tls_data \
	valid_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT \
		     NOOP RSET SEND SAML SOML AUTH TURN ETRN PIPELINING CHUNKING DATA DSN RSET QUIT ONEX QUEU \
		     STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE \
		     XQUEU XSTA XTRN XUSR } \
	normalize_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY \
			 IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN PIPELINING CHUNKING DATA DSN RSET QUIT \
			 ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 \
			 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
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
		$sf_pscan_ignore_scanners = trim(filter_expand_alias($snortcfg['pscan_ignore_scanners']));
		$sf_pscan_ignore_scanners = preg_replace('/\s+/', ',', trim($sf_pscan_ignore_scanners));
	}
	
	$sf_portscan = <<<EOD
# sf Portscan #
preprocessor sfportscan: \
	scan_type { {$sf_pscan_type} } \
	proto  { {$sf_pscan_protocol} } \
	memcap { {$sf_pscan_memcap} } \
	sense_level { {$sf_pscan_sense_level} } \
	ignore_scanners { {$sf_pscan_ignore_scanners} }
	
EOD;

	/* def ssh_preproc */
	$ssh_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['ssh_ports']));
	$ssh_preproc = <<<EOD
# SSH preprocessor #
preprocessor ssh: \
	server_ports { {$ssh_ports} } \
	autodetect \
	max_client_bytes 19600 \
	max_encrypted_packets 20 \
	max_server_version_len 100 \
	enable_respoverflow enable_ssh1crc32 \
	enable_srvoverflow enable_protomismatch

EOD;

	/* def other_preprocs */
	$sun_rpc_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['sun_rpc_ports']));
	$other_preprocs = <<<EOD
# Other preprocs #
preprocessor rpc_decode: \
	{$sun_rpc_ports} \
	no_alert_multiple_requests \
	no_alert_large_fragments \
	no_alert_incomplete

# Back Orifice preprocessor #
preprocessor bo
	
EOD;

	/* def dce_rpc_2 */
	$dce_rpc_2 = <<<EOD
# DCE/RPC 2 #
preprocessor dcerpc2: \
	memcap 102400, \
	events [co]

preprocessor dcerpc2_server: default, \
	policy WinXP, \
	detect [smb [{$snort_ports['smb_ports']}], \
	tcp 135, \
	udp 135, \
	rpc-over-http-server 593], \
	autodetect [tcp 1025:, \
	udp 1025:, \
	rpc-over-http-server 1025:], \
	smb_max_chain 3, smb_invalid_shares ["C$", "D$", "ADMIN$"]
	
EOD;

	$sip_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['sip_ports']));
	$sip_preproc = <<<EOD
# SIP preprocessor #
preprocessor sip: \
	max_sessions 40000, \
	ports { {$sip_ports} }, \
	methods { invite \
		cancel \
		ack \
		bye \
		register \
		options \
		refer \
		subscribe \
		update \
		join \
		info \
		message \
		notify \
		benotify \
		do \
		qauth \
		sprack \
		publish \
		service \
		unsubscribe \
		prack }, \
	max_call_id_len 80, \
	max_from_len 256, \
	max_to_len 256, \
	max_via_len 1024, \
	max_requestName_len 50, \
	max_uri_len 512, \
	ignore_call_channel, \
	max_content_len 2048, \
	max_contact_len 512

EOD;

	$dns_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['dns_ports']));
	/* def dns_preprocessor */
	$dns_preprocessor = <<<EOD
# DNS preprocessor #
preprocessor dns: \
	ports { {$dns_ports} } \
	enable_rdata_overflow
	
EOD;

	/* def dnp3_preprocessor */
	$dnp3_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['DNP3_PORTS']));
	$dnp3_preproc = <<<EOD
# DNP3 preprocessor #
preprocessor dnp3: \
	ports { {$dnp3_ports} } \
	memcap 262144 \
	check_crc
	
EOD;

	/* def modbus_preprocessor */
	$modbus_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['MODBUS_PORTS']));
	$modbus_preproc = <<<EOD
# Modbus preprocessor #
preprocessor modbus: \
	ports { {$modbus_ports} }
	
EOD;

	/* def gtp_preprocessor */
	$gtp_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['GTP_PORTS']));
	$gtp_preproc = <<<EOD
# GTP preprocessor #
preprocessor gtp: \
	ports { {$gtp_ports} }
	
EOD;

	/* def ssl_preprocessor */
	$ssl_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['ssl_ports']));
	$ssl_preproc = <<<EOD
# SSL preprocessor #
preprocessor ssl: \
	ports { {$ssl_ports} }, \
	trustservers, \
	noinspect_encrypted 

EOD;

	/* def sensitive_data_preprocessor */
	if ($snortcfg['sdf_mask_output'] == "on")
		$sdf_mask_output = "\\\n\tmask_output";
	else
		$sdf_mask_output = "";
	if (empty($snortcfg['sdf_alert_threshold']))
		$snortcfg['sdf_alert_threshold'] = 25;
	$sensitive_data = <<<EOD
# SDF preprocessor #
preprocessor sensitive_data: \
	alert_threshold {$snortcfg['sdf_alert_threshold']} {$sdf_mask_output}

EOD;

	/* define IP Reputation preprocessor */
	if (is_array($snortcfg['blist_files']['item'])) {
		$blist_files = "";
		$bIsFirst = TRUE;
		foreach ($snortcfg['blist_files']['item'] as $blist) {
			if ($bIsFirst) {
				$blist_files .= "blacklist " . IPREP_PATH . $blist;
				$bIsFirst = FALSE;
			}
			else
				$blist_files .= ", \\ \n\tblacklist " . IPREP_PATH . $blist;    
		}
	}
	if (is_array($snortcfg['wlist_files']['item'])) {
		$wlist_files = "";
		$bIsFirst = TRUE;
		foreach ($snortcfg['wlist_files']['item'] as $wlist) {
			if ($bIsFirst) {
				$wlist_files .= "whitelist " . IPREP_PATH . $wlist;
				$bIsFirst = FALSE;
			}
			else
				$wlist_files .= ", \\ \n\twhitelist " . IPREP_PATH . $wlist;    
		}
	}
	if (!empty($blist_files))
		$ip_lists = $blist_files;
	if (!empty($wlist_files))
		$ip_lists .= ", \\ \n" . $wlist_files;
	if ($snortcfg['iprep_scan_local'] == 'on')
		$ip_lists .= ", \\ \n\tscan_local";	

	$reputation_preproc = <<<EOD
# IP Reputation preprocessor #
preprocessor reputation: \
	memcap {$snortcfg['iprep_memcap']}, \
	priority {$snortcfg['iprep_priority']}, \
	nested_ip {$snortcfg['iprep_nested_ip']}, \
	white {$snortcfg['iprep_white']}, \
	{$ip_lists}

EOD;

	/* define servers as IP variables */
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

	// Change old name from "var" to new name of "ipvar" for IP variables because 
	// Snort is deprecating the old "var" name in newer versions.
	$ipvardef = "";
	foreach ($snort_servers as $alias => $avalue) {
		if (!empty($snortcfg["def_{$alias}"]) && is_alias($snortcfg["def_{$alias}"])) {
			$avalue = trim(filter_expand_alias($snortcfg["def_{$alias}"]));
			$avalue = preg_replace('/\s+/', ',', trim($avalue));
		}
		$ipvardef .= "ipvar " . strtoupper($alias) . " [{$avalue}]\n";
	}

	$snort_preproc_libs = array(
		"dce_rpc_2" => "dce2_preproc", "dns_preprocessor" => "dns_preproc", "ftp_preprocessor" => "ftptelnet_preproc", "imap_preproc" => "imap_preproc",
		"pop_preproc" => "pop_preproc", "reputation_preproc" => "reputation_preproc", "sensitive_data" => "sdf_preproc", 
		"sip_preproc" => "sip_preproc", "gtp_preproc" => "gtp_preproc", "smtp_preprocessor" => "smtp_preproc", "ssh_preproc" => "ssh_preproc", 
		"ssl_preproc" => "ssl_preproc", "dnp3_preproc" => "dnp3_preproc", "modbus_preproc" => "modbus_preproc"
	);
	$snort_preproc = array (
		"perform_stat", "other_preprocs", "ftp_preprocessor", "smtp_preprocessor", "ssl_preproc", "sip_preproc", "gtp_preproc", "ssh_preproc", "sf_portscan", 
		"dce_rpc_2", "dns_preprocessor", "sensitive_data", "pop_preproc", "imap_preproc", "dnp3_preproc", "modbus_preproc", "reputation_preproc"
	);
	$default_disabled_preprocs = array(
		"sf_portscan", "gtp_preproc", "sensitive_data", "dnp3_preproc", "modbus_preproc", "reputation_preproc", "perform_stat" 
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
	// Remove final trailing newline
	$snort_preprocessors = rtrim($snort_preprocessors);

	$snort_misc_include_rules = "";
	if (file_exists("{$snortcfgdir}/reference.config"))
		$snort_misc_include_rules .= "include {$snortcfgdir}/reference.config\n";
	if (file_exists("{$snortcfgdir}/classification.config"))
		$snort_misc_include_rules .= "include {$snortcfgdir}/classification.config\n";
	if (!file_exists("{$snortcfgdir}/preproc_rules/decoder.rules") || !file_exists("{$snortcfgdir}/preproc_rules/preprocessor.rules")) {
		$snort_misc_include_rules .= "config autogenerate_preprocessor_decoder_rules\n";
		log_error("[Snort] Seems preprocessor and/or decoder rules are missing, enabling autogeneration of them in conf file.");
	}

	/* generate rule sections to load */
	/* The files are always configured so the update process is easier */
	$selected_rules_sections = "include \$RULE_PATH/{$snort_enforcing_rules_file}\n";
	$selected_rules_sections .= "include \$RULE_PATH/{$flowbit_rules_file}\n";
	$selected_rules_sections  .= "include \$RULE_PATH/custom.rules\n";

	// Remove trailing newlines
	$snort_misc_include_rules = rtrim($snort_misc_include_rules);
	$selected_rules_sections = rtrim($selected_rules_sections);

	/* Create the actual rules files and save in the interface directory */
	snort_prepare_rule_files($snortcfg, $snortcfgdir);

	$cksumcheck = "all";
	if ($snortcfg['cksumcheck'] == 'on')
		$cksumcheck = "none";

	/* Pull in user-configurable detection config options */
	$cfg_detect_settings = "search-method {$snort_performance} max-pattern-len 20 max_queue_events 5";
	if ($snortcfg['fpm_split_any_any'] == "on")
		$cfg_detect_settings .= " split-any-any";
	if ($snortcfg['fpm_search_optimize'] == "on")
		$cfg_detect_settings .= " search-optimize";
	if ($snortcfg['fpm_no_stream_inserts'] == "on")
		$cfg_detect_settings .= " no_stream_inserts";

	/* Pull in user-configurable options for Frag3 preprocessor settings */
	/* Get global Frag3 options first and put into a string */
	$frag3_global = "preprocessor frag3_global: ";
	if (!empty($snortcfg['frag3_memcap']) || $snortcfg['frag3_memcap'] == "0")
		$frag3_global .= "memcap {$snortcfg['frag3_memcap']}, ";
	else
		$frag3_global .= "memcap 4194304, ";
	if (!empty($snortcfg['frag3_max_frags']))
		$frag3_global .= "max_frags {$snortcfg['frag3_max_frags']}";
	else
		$frag3_global .= "max_frags 8192";
	if ($snortcfg['frag3_detection'] == "off")
		$frag3_global .= ", disabled";

	$frag3_default_tcp_engine = array( "name" => "default", "bind_to" => "all", "policy" => "bsd", 
					   "timeout" => 60, "min_ttl" => 1, "detect_anomalies" => "on", 
					   "overlap_limit" => 0, "min_frag_len" => 0 );
	$frag3_engine = "";

	// Now iterate configured Frag3 engines and write them to a string if enabled
	if ($snortcfg['frag3_detection'] == "on") {
		if (!is_array($snortcfg['frag3_engine']['item']))
			$snortcfg['frag3_engine']['item'] = array();

		// If no frag3 tcp engine is configured, use the default
		if (empty($snortcfg['frag3_engine']['item']))
			$snortcfg['frag3_engine']['item'][] = $frag3_default_tcp_engine;

		foreach ($snortcfg['frag3_engine']['item'] as $f => $v) {
			$frag3_engine .= "preprocessor frag3_engine: ";
			$frag3_engine .= "policy {$v['policy']}";
			if ($v['bind_to'] <> "all") {
				$tmp = trim(filter_expand_alias($v['bind_to']));
				if (!empty($tmp)) {
					$tmp = preg_replace('/\s+/', ',', $tmp);
					if (strpos($tmp, ",") !== false)
						$frag3_engine .= " \\\n\tbind_to [{$tmp}]";
					else
						$frag3_engine .= " \\\n\tbind_to {$tmp}";
				}
				else
					log_error("[snort] WARNING: unable to resolve IP List Alias '{$v['bind_to']}' for Frag3 engine '{$v['name']}' ... using 0.0.0.0 failsafe.");
			}
			$frag3_engine .= " \\\n\ttimeout {$v['timeout']}";
			$frag3_engine .= " \\\n\tmin_ttl {$v['min_ttl']}";
			if ($v['detect_anomalies'] == "on") {
				$frag3_engine .= " \\\n\tdetect_anomalies";
				$frag3_engine .= " \\\n\toverlap_limit {$v['overlap_limit']}";
				$frag3_engine .= " \\\n\tmin_fragment_length {$v['min_frag_len']}";
			}
			// Add newlines to terminate this engine
			$frag3_engine .= "\n\n";
		}
		// Remove trailing newline
		$frag3_engine = rtrim($frag3_engine);
	}

	// Grab any user-customized value for Protocol Aware Flushing (PAF) max PDUs
	$paf_max_pdu_config = "config paf_max: ";
	if (empty($snortcfg['max_paf']) || $snortcfg['max_paf'] == '0')
		$paf_max_pdu_config .= "0";
	else
		$paf_max_pdu_config .= $snortcfg['max_paf'];

	// Pull in user-configurable options for Stream5 preprocessor settings
	// Get global options first and put into a string
	$stream5_global = "preprocessor stream5_global: \\\n";
	if ($snortcfg['stream5_reassembly'] == "off")
		$stream5_global .= "\tdisabled, \\\n";
	if ($snortcfg['stream5_track_tcp'] == "off")
		$stream5_global .= "\ttrack_tcp no,";
	else {
		$stream5_global .= "\ttrack_tcp yes,";
		if (!empty($snortcfg['stream5_max_tcp']))
			$stream5_global .= " \\\n\tmax_tcp {$snortcfg['stream5_max_tcp']},";
		else
			$stream5_global .= " \\\n\tmax_tcp 262144,";
	}
	if ($snortcfg['stream5_track_udp'] == "off")
		$stream5_global .= " \\\n\ttrack_udp no,";
	else {
		$stream5_global .= " \\\n\ttrack_udp yes,";
		if (!empty($snortcfg['stream5_max_udp']))
			$stream5_global .= " \\\n\tmax_udp {$snortcfg['stream5_max_udp']},";
		else
			$stream5_global .= " \\\n\tmax_udp 131072,";
	}
	if ($snortcfg['stream5_track_icmp'] == "on") {
		$stream5_global .= " \\\n\ttrack_icmp yes,";
		if (!empty($snortcfg['stream5_max_icmp']))
			$stream5_global .= " \\\n\tmax_icmp {$snortcfg['stream5_max_icmp']},";
		else
			$stream5_global .= " \\\n\tmax_icmp 65536,";
	}
	else
		$stream5_global .= " \\\n\ttrack_icmp no,";
	if (!empty($snortcfg['stream5_mem_cap']))
		$stream5_global .= " \\\n\tmemcap {$snortcfg['stream5_mem_cap']},";
	else
		$stream5_global .= " \\\n\tmemcap 8388608,";

	if (!empty($snortcfg['stream5_prune_log_max']) || $snortcfg['stream5_prune_log_max'] == '0')
		$stream5_global .= " \\\n\tprune_log_max {$snortcfg['stream5_prune_log_max']}";
	else
		$stream5_global .= " \\\n\tprune_log_max 1048576";
	if ($snortcfg['stream5_flush_on_alert'] == "on")
		$stream5_global .= ", \\\n\tflush_on_alert";

	$stream5_default_tcp_engine = array( "name" => "default", "bind_to" => "all", "policy" => "bsd", "timeout" => 30, 
					     "max_queued_bytes" => 1048576, "detect_anomalies" => "off", "overlap_limit" => 0, 
					     "max_queued_segs" => 2621, "require_3whs" => "off", "startup_3whs_timeout" => 0, 
					     "no_reassemble_async" => "off", "dont_store_lg_pkts" => "off", "max_window" => 0, 
					     "use_static_footprint_sizes" => "off", "check_session_hijacking" => "off", "ports_client" => "default", 
					     "ports_both" => "default", "ports_server" => "none" );
	$stream5_tcp_engine = "";

	// Now iterate configured Stream5 TCP engines and write them to a string if enabled
	if ($snortcfg['stream5_reassembly'] == "on") {
		if (!is_array($snortcfg['stream5_tcp_engine']['item']))
			$snortcfg['stream5_tcp_engine']['item'] = array();

		// If no stream5 tcp engine is configured, use the default
		if (empty($snortcfg['stream5_tcp_engine']['item']))
			$snortcfg['stream5_tcp_engine']['item'][] = $stream5_default_tcp_engine;

		foreach ($snortcfg['stream5_tcp_engine']['item'] as $f => $v) {
			$buffer = "preprocessor stream5_tcp: ";
			$buffer .= "policy {$v['policy']},";
			if ($v['bind_to'] <> "all") {
				$tmp = trim(filter_expand_alias($v['bind_to']));
				if (!empty($tmp)) {
					$tmp = preg_replace('/\s+/', ',', $tmp);
					if (strpos($tmp, ",") !== false)
						$buffer .= " \\\n\tbind_to [{$tmp}],";
					else
						$buffer .= " \\\n\tbind_to {$tmp},";
				}
				else {
					log_error("[snort] WARNING: unable to resolve IP Address Alias [{$v['bind_to']}] for Stream5 TCP engine '{$v['name']}' ... skipping this engine.");
					continue;
				}
			}
			$stream5_tcp_engine .= $buffer;
			$stream5_tcp_engine .= " \\\n\ttimeout {$v['timeout']},";
			$stream5_tcp_engine .= " \\\n\toverlap_limit {$v['overlap_limit']},";
			$stream5_tcp_engine .= " \\\n\tmax_window {$v['max_window']},";
			$stream5_tcp_engine .= " \\\n\tmax_queued_bytes {$v['max_queued_bytes']},";
			$stream5_tcp_engine .= " \\\n\tmax_queued_segs {$v['max_queued_segs']}";
			if ($v['use_static_footprint_sizes'] == "on")
				$stream5_tcp_engine .= ", \\\n\tuse_static_footprint_sizes";
			if ($v['check_session_hijacking'] == "on")
				$stream5_tcp_engine .= ", \\\n\tcheck_session_hijacking";
			if ($v['dont_store_lg_pkts'] == "on")
				$stream5_tcp_engine .= ", \\\n\tdont_store_large_packets";
			if ($v['no_reassemble_async'] == "on")
				$stream5_tcp_engine .= ", \\\n\tdont_reassemble_async";
			if ($v['detect_anomalies'] == "on")
				$stream5_tcp_engine .= ", \\\n\tdetect_anomalies";
			if ($v['require_3whs'] == "on")
				$stream5_tcp_engine .= ", \\\n\trequire_3whs {$v['startup_3whs_timeout']}";
			if (!empty($v['ports_client'])) {
				$stream5_tcp_engine .= ", \\\n\tports client";
				if ($v['ports_client'] == " all")
					$stream5_tcp_engine .= " all";
				elseif ($v['ports_client'] == "default")
					$stream5_tcp_engine .= " {$stream5_ports_client}";
				else {
					$tmp = trim(filter_expand_alias($v['ports_client']));
					if (!empty($tmp))
						$stream5_tcp_engine .= " " . trim(preg_replace('/\s+/', ' ', $tmp));
					else {
						$stream5_tcp_engine .= " {$stream5_ports_client}";
						log_error("[snort] WARNING: unable to resolve Ports Client Alias [{$v['ports_client']}] for Stream5 TCP engine '{$v['name']}' ... using default value.");
					}
				}
			}
			if (!empty($v['ports_both'])) {
				$stream5_tcp_engine .= ", \\\n\tports both";
				if ($v['ports_both'] == " all")
					$stream5_tcp_engine .= " all";
				elseif ($v['ports_both'] == "default")
					$stream5_tcp_engine .= " {$stream5_ports_both}";
				else {
					$tmp = trim(filter_expand_alias($v['ports_both']));
					if (!empty($tmp))
						$stream5_tcp_engine .= " " . trim(preg_replace('/\s+/', ' ', $tmp));
					else {
						$stream5_tcp_engine .= " {$stream5_ports_both}";
						log_error("[snort] WARNING: unable to resolve Ports Both Alias [{$v['ports_both']}] for Stream5 TCP engine '{$v['name']}' ... using default value.");
					}
				}
			}
			if (!empty($v['ports_server']) && $v['ports_server'] <> "none" && $v['ports_server'] <> "default") {
				if ($v['ports_server'] == " all") {
					$stream5_tcp_engine .= ", \\\n\tports server";
					$stream5_tcp_engine .= " all";
				}
				else {
					$tmp = trim(filter_expand_alias($v['ports_server']));
					if (!empty($tmp)) {
						$stream5_tcp_engine .= ", \\\n\tports server";
						$stream5_tcp_engine .= " " . trim(preg_replace('/\s+/', ' ', $tmp));
					}
					else
						log_error("[snort] WARNING: unable to resolve Ports Server Alias [{$v['ports_server']}] for Stream5 TCP engine '{$v['name']}' ... defaulting to none.");
				}
			}

			// Make sure the "ports" parameter is set, or else default to a safe value
			if (strpos($stream5_tcp_engine, "ports ") === false)
				$stream5_tcp_engine .= ", \\\n\tports both all";

			// Add a pair of newlines to terminate this engine
			$stream5_tcp_engine .= "\n\n";
		}
		// Trim off the final trailing newline
		$stream5_tcp_engine = rtrim($stream5_tcp_engine);
	}

	// Configure the Stream5 UDP engine if it and Stream5 reassembly are enabled
	if ($snortcfg['stream5_track_udp'] == "off" || $snortcfg['stream5_reassembly'] == "off")
		$stream5_udp_engine = "";
	else {
		$stream5_udp_engine = "preprocessor stream5_udp: ";
		if (!empty($snortcfg['stream5_udp_timeout'])) 
			$stream5_udp_engine .= "timeout {$snortcfg['stream5_udp_timeout']}";
		else
			$stream5_udp_engine .= "timeout 30";
	}

	// Configure the Stream5 ICMP engine if it and Stream5 reassembly are enabled
	if ($snortcfg['stream5_track_icmp'] == "on" && $snortcfg['stream5_reassembly'] == "on") {
		$stream5_icmp_engine = "preprocessor stream5_icmp: ";
		if (!empty($snortcfg['stream5_icmp_timeout'])) 
			$stream5_icmp_engine .= "timeout {$snortcfg['stream5_icmp_timeout']}";
		else
			$stream5_icmp_engine .= "timeout 30";
	}
	else
		$stream5_icmp_engine = "";

	// Check for and configure Host Attribute Table if enabled
	$host_attrib_config = "";
	if ($snortcfg['host_attribute_table'] == "on" && !empty($snortcfg['host_attribute_data'])) {
		file_put_contents("{$snortcfgdir}/host_attributes", base64_decode($snortcfg['host_attribute_data']));
		$host_attrib_config = "# Host Attribute Table #\n";
		$host_attrib_config .= "attribute_table filename {$snortcfgdir}/host_attributes\n";
		if (!empty($snortcfg['max_attribute_hosts']))
			$host_attrib_config .= "config max_attribute_hosts: {$snortcfg['max_attribute_hosts']}\n";
		if (!empty($snortcfg['max_attribute_services_per_host']))
			$host_attrib_config .= "config max_attribute_services_per_host: {$snortcfg['max_attribute_services_per_host']}";
	}

	// Configure the HTTP_INSPECT preprocessor
	// Get global options first and put into a string
	$http_inspect_global = "preprocessor http_inspect: global ";
	if ($snortcfg['http_inspect'] == "off")
		$http_inspect_global .= "disabled ";
	$http_inspect_global .= "\\\n\tiis_unicode_map unicode.map 1252 \\\n";
	$http_inspect_global .= "\tcompress_depth 65535 \\\n";
	$http_inspect_global .= "\tdecompress_depth 65535 \\\n";
	if (!empty($snortcfg['http_inspect_memcap']))
		$http_inspect_global .= "\tmemcap {$snortcfg['http_inspect_memcap']} \\\n";
	else
		$http_inspect_global .= "\tmemcap 150994944 \\\n";
	if (!empty($snortcfg['http_inspect_max_gzip_mem']))
		$http_inspect_global .= "\tmax_gzip_mem {$snortcfg['http_inspect_max_gzip_mem']}";
	else
		$http_inspect_global .= "\tmax_gzip_mem 838860";
	if ($snortcfg['http_inspect_proxy_alert'] == "on")
		$http_inspect_global .= " \\\n\tproxy_alert";

	$http_inspect_default_engine = array( "name" => "default", "bind_to" => "all", "server_profile" => "all", "enable_xff" => "off", 
					      "log_uri" => "off", "log_hostname" => "off", "server_flow_depth" => 65535, "enable_cookie" => "on", 
					      "client_flow_depth" => 1460, "extended_response_inspection" => "on", "no_alerts" => "off", 
					      "unlimited_decompress" => "on", "inspect_gzip" => "on", "normalize_cookies" =>"on", "normalize_headers" => "on", 
					      "normalize_utf" => "on", "normalize_javascript" => "on", "allow_proxy_use" => "off", "inspect_uri_only" => "off", 
					      "max_javascript_whitespaces" => 200, "post_depth" => -1, "max_headers" => 0, "max_spaces" => 0, 
					      "max_header_length" => 0, "ports" => "default" );
	$http_ports = str_replace(",", " ", snort_expand_port_range($snort_ports['http_ports']));
	$http_inspect_servers = "";

	// Iterate configured HTTP_INSPECT servers and write them to string if HTTP_INSPECT enabled
	if ($snortcfg['http_inspect'] <> "off") {
		if (!is_array($snortcfg['http_inspect_engine']['item']))
			$snortcfg['http_inspect_engine']['item'] = array();

		// If no http_inspect_engine is configured, use the default
		if (empty($snortcfg['http_inspect_engine']['item']))
			$snortcfg['http_inspect_engine']['item'][] = $http_inspect_default_engine;

		foreach ($snortcfg['http_inspect_engine']['item'] as $f => $v) {
			$buffer = "preprocessor http_inspect_server: \\\n";
			if ($v['name'] == "default")
				$buffer .= "\tserver default \\\n";
			elseif (is_alias($v['bind_to'])) {
				$tmp = trim(filter_expand_alias($v['bind_to']));
				if (!empty($tmp)) {
					$tmp = preg_replace('/\s+/', ' ', $tmp);
						$buffer .= "\tserver { {$tmp} } \\\n";
				}
				else {
					log_error("[snort] WARNING: unable to resolve IP Address Alias [{$v['bind_to']}] for HTTP_INSPECT server '{$v['name']}' ... skipping this server engine.");
					continue;
				}
			}
			else {
					log_error("[snort] WARNING: unable to resolve IP Address Alias [{$v['bind_to']}] for HTTP_INSPECT server '{$v['name']}' ... skipping this server engine.");
					continue;
			}
			$http_inspect_servers .= $buffer;
			$http_inspect_servers .= "\tprofile {$v['server_profile']} \\\n";

			if ($v['no_alerts'] == "on")
				$http_inspect_servers .= "\tno_alerts \\\n";

			if ($v['ports'] == "default" || empty($v['ports']))
				$http_inspect_servers .= "\tports { {$http_ports} } \\\n";
			elseif (is_alias($v['ports'])) {
				$tmp = trim(filter_expand_alias($v['ports']));
				if (!empty($tmp)) {
					$tmp = preg_replace('/\s+/', ' ', $tmp);
					$tmp = snort_expand_port_range($tmp, ' ');
					$http_inspect_servers .= "\tports { {$tmp} } \\\n";
				}
				else {
					log_error("[snort] WARNING: unable to resolve Ports Alias [{$v['ports']}] for HTTP_INSPECT server '{$v['name']}' ... using safe default instead.");
					$http_inspect_servers .= "\tports { {$http_ports} } \\\n";
				}
			}
			else {
					log_error("[snort] WARNING: unable to resolve Ports Alias [{$v['ports']}] for HTTP_INSPECT server '{$v['name']}' ... using safe default instead.");
					$http_inspect_servers .= "\tports { {$http_ports} } \\\n";
			}

			$http_inspect_servers .= "\tserver_flow_depth {$v['server_flow_depth']} \\\n";
			$http_inspect_servers .= "\tclient_flow_depth {$v['client_flow_depth']} \\\n";
			$http_inspect_servers .= "\thttp_methods { GET POST PUT SEARCH MKCOL COPY MOVE LOCK UNLOCK NOTIFY POLL BCOPY BDELETE BMOVE LINK UNLINK OPTIONS HEAD DELETE TRACE TRACK CONNECT SOURCE SUBSCRIBE UNSUBSCRIBE PROPFIND PROPPATCH BPROPFIND BPROPPATCH RPC_CONNECT PROXY_SUCCESS BITS_POST CCM_POST SMS_POST RPC_IN_DATA RPC_OUT_DATA RPC_ECHO_DATA } \\\n";
			$http_inspect_servers .= "\tpost_depth {$v['post_depth']} \\\n";
			$http_inspect_servers .= "\tmax_headers {$v['max_headers']} \\\n";
			$http_inspect_servers .= "\tmax_header_length {$v['max_header_length']} \\\n";
			$http_inspect_servers .= "\tmax_spaces {$v['max_spaces']}";
			if ($v['enable_xff'] == "on")
				$http_inspect_servers .= " \\\n\tenable_xff";
			if ($v['enable_cookie'] == "on")
				$http_inspect_servers .= " \\\n\tenable_cookie";
			if ($v['normalize_cookies'] == "on")
				$http_inspect_servers .= " \\\n\tnormalize_cookies";
			if ($v['normalize_headers'] == "on")
				$http_inspect_servers .= " \\\n\tnormalize_headers";
			if ($v['normalize_utf'] == "on")
				$http_inspect_servers .= " \\\n\tnormalize_utf";
			if ($v['allow_proxy_use'] == "on")
				$http_inspect_servers .= " \\\n\tallow_proxy_use";
			if ($v['inspect_uri_only'] == "on")
				$http_inspect_servers .= " \\\n\tinspect_uri_only";
			if ($v['extended_response_inspection'] == "on") {
				$http_inspect_servers .= " \\\n\textended_response_inspection";
				if ($v['inspect_gzip'] == "on") {
					$http_inspect_servers .= " \\\n\tinspect_gzip";
					if ($v['unlimited_decompress'] == "on")
						$http_inspect_servers .= " \\\n\tunlimited_decompress";
				}
				if ($v['normalize_javascript'] == "on") {
					$http_inspect_servers .= " \\\n\tnormalize_javascript";
					$http_inspect_servers .= " \\\n\tmax_javascript_whitespaces {$v['max_javascript_whitespaces']}";
				}
			}
			if ($v['log_uri'] == "on")
				$http_inspect_servers .= " \\\n\tlog_uri";
			if ($v['log_hostname'] == "on")
				$http_inspect_servers .= " \\\n\tlog_hostname";

			// Add a pair of trailing newlines to terminate this server config
			$http_inspect_servers .= "\n\n";
		}
		/* Trim off the final trailing newline */
		$http_inspect_server = rtrim($http_inspect_server);
	}

	// Finally, build the Snort configuration file
	$snort_conf_text = <<<EOD
# snort configuration file
# generated automatically by the pfSense subsystems do not modify manually

# Define Local Network #
ipvar HOME_NET [{$home_net}]
ipvar EXTERNAL_NET [{$external_net}]

# Define Rule Path #
var RULE_PATH {$snortcfgdir}/rules

# Define Servers #
{$ipvardef}

# Define Server Ports #
{$portvardef}

# Configure quiet startup mode #
config quiet

# Configure the snort decoder #
config checksum_mode: {$cksumcheck}
config disable_decode_alerts
config disable_tcpopt_experimental_alerts
config disable_tcpopt_obsolete_alerts
config disable_ttcp_alerts
config disable_tcpopt_alerts
config disable_ipopt_alerts
config disable_decode_drops

# Enable the GTP decoder #
config enable_gtp

# Configure PCRE match limitations
config pcre_match_limit: 3500
config pcre_match_limit_recursion: 1500

# Configure the detection engine #
config detection: {$cfg_detect_settings}
config event_queue: max_queue 8 log 5 order_events content_length

# Configure to show year in timestamps
config show_year

# Configure protocol aware flushing #
# For more information see README.stream5 #
{$paf_max_pdu_config}

# Configure dynamically loaded libraries
dynamicpreprocessor directory {$snort_dirs['dynamicpreprocessor']}
dynamicengine directory {$snort_dirs['dynamicengine']}
dynamicdetection directory {$snort_dirs['dynamicrules']}

# Inline packet normalization. For more information, see README.normalize
# Disabled since we do not use "inline" mode with pfSense
# preprocessor normalize_ip4
# preprocessor normalize_tcp: ips ecn stream
# preprocessor normalize_icmp4
# preprocessor normalize_ip6
# preprocessor normalize_icmp6

# Flow and stream #
{$frag3_global}

{$frag3_engine}

{$stream5_global}

{$stream5_tcp_engine}

{$stream5_udp_engine}

{$stream5_icmp_engine}

# HTTP Inspect #
{$http_inspect_global}

{$http_inspect_servers}
{$snort_preprocessors}
{$host_attrib_config}

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

	// Write out snort.conf file
	file_put_contents("{$snortcfgdir}/snort.conf", $snort_conf_text);
	conf_mount_ro();
	unset($snort_conf_text, $selected_rules_sections, $suppress_file_name, $snort_misc_include_rules, $spoink_type, $snortunifiedlog_type, $alertsystemlog_type);
	unset($home_net, $external_net, $ipvardef, $portvardef);
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function snort_sync_on_changes() {
	global $config, $g;

	/* Do not attempt a package sync while booting up or installing package */
	if ($g['booting'] || $g['snort_postinstall'])
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
	global $config, $g;

	/* Do not attempt a package sync while booting up or installing package */
	if ($g['booting'] || $g['snort_postinstall'])
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

	/* Build a series of commands as a PHP file for the secondary host to execute to load the new settings. */
	$snort_sync_cmd = <<<EOD
	<?php
	require_once("/usr/local/pkg/snort/snort.inc");
	require_once("service-utils.inc");
	global \$g, \$rebuild_rules, \$snort_gui_include, \$pkg_interface;
	\$orig_pkg_interface = \$pkg_interface;
	\$g["snort_postinstall"] = true;
	\$g["snort_sync_in_progress"] = true;
	\$snort_gui_include = false;
	\$pkg_interface = "console";
	{$downloadrulescmd}
	unset(\$g["snort_postinstall"]);
	log_error(gettext("[snort] XMLRPC pkg sync: Generating snort.conf file using Master Host settings..."));
	\$rebuild_rules = true;
	sync_snort_package_config();
	\$rebuild_rules = false;
	{$snortstart}
	log_error(gettext("[snort] XMLRPC pkg sync process on this host is complete..."));
	\$pkg_interface = \$orig_pkg_interface;
	unset(\$g["snort_sync_in_progress"]);
	return true;
	?>

EOD;

	/* First, have the target host write the commands to a PHP file in the /tmp directory */
	$execcmd = "file_put_contents('/tmp/snort_sync_cmds.php', '{$snort_sync_cmd}');";

	/* assemble xmlrpc payload */
	$method = 'pfsense.exec_php';
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("[snort] Snort XMLRPC sending reload configuration cmd set as a file to {$url}:{$port}.");
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

	/* Now assemble a command to execute the previously sent PHP file in the background */
	$execcmd = "exec(\"/usr/local/bin/php -f '/tmp/snort_sync_cmds.php' > /dev/null 2>&1 &\");";
	$params2 = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	log_error("[snort] Snort XMLRPC sending {$url}:{$port} cmd to execute configuration reload.");
	$msg2 = new XML_RPC_Message($method, $params2);
	$resp = $cli->send($msg2, $synctimeout);
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
