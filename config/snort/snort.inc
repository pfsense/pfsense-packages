<?php
/*
 snort.inc
 Copyright (C) 2006 Scott Ullrich
 Copyright (C) 2009-2010 Robert Zelaya
 Copyright (C) 2011 Ermal Luci
 part of pfSense
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

// Needed on 2.0 because of filter_get_vpns_list()
require_once("filter.inc");

/* package version */
$snort_package_version = 'Snort 2.9.2.3 pkg v. 2.2.2';
$snort_rules_file = "snortrules-snapshot-2922.tar.gz";

/* Allow additional execution time 0 = no limit. */
ini_set('max_execution_time', '9999');
ini_set('max_input_time', '9999');

if (!is_array($config['installedpackages']['snortglobal']))
	$config['installedpackages']['snortglobal'] = array();

/* find out if were in 1.2.3-RELEASE */
if (intval($config['version']) > 6)
	$snort_pfsense_basever = 'no';
else
	$snort_pfsense_basever = 'yes';

/* find out what arch where in x86 , x64 */
global $snort_arch;
$snort_arch  = 'x86';
$snort_arch_ck = php_uname("m");
if ($snort_arch_ck == 'i386')
	$snort_arch = 'x86';
else if ($snort_arch_ck == "amd64")
	$snort_arch = 'x64';
else
	$snort_arch = "Unknown";

/* tell me my theme */
$pfsense_theme_is = $config['theme'];

/* func builds custom white lists */
function find_whitelist_key($find_wlist_number) {
	global $config, $g;

	if (!is_array($config['installedpackages']['snortglobal']['whitelist']))
		$config['installedpackages']['snortglobal']['whitelist'] = array();
	if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
		return 0; /* XXX */

	foreach ($config['installedpackages']['snortglobal']['whitelist']['item'] as $w_key => $value) {
		if ($value['name'] == $find_wlist_number)
			return $w_key;
	}
}

/* func builds custom suppress lists */
function find_suppress_key($find_slist_number) {
	global $config, $g;

	if (!is_array($config['installedpackages']['snortglobal']['suppress']))
		$config['installedpackages']['snortglobal']['suppress'] = array();
	if (!is_array($config['installedpackages']['snortglobal']['suppress']['item']))
		return 0; /* XXX */

	foreach ($config['installedpackages']['snortglobal']['suppress']['item'] as $s_key => $value) {
		if ($value['name'] == $find_slist_number)
			return $s_key;
	}
}

/* func builds custom whitelests */
function build_base_whitelist($build_netlist, $wanip, $wangw, $wandns, $vips, $vpns, $userwips) {
	global $config, $g, $snort_pfsense_basever;

	/* build an interface array list */
	if (function_exists('get_configured_interface_list'))
		$int_array = get_configured_interface_list();
	else {
		$int_array = array('lan');
		for ($j = 1; isset ($config['interfaces']['opt' . $j]); $j++)
		if(isset($config['interfaces']['opt' . $j]['enable']))
		if(isset($config['interfaces']['opt' . $j]['gateway']))
			$int_array[] = "opt{$j}";
	}

	$home_net = "";

	/*  iterate through interface list and write out whitelist items
	 *  and also compile a home_net list for snort.
	 */
	foreach ($int_array as $int) {
		/* calculate interface subnet information */
		if (function_exists('get_interface_ip')) {
			$subnet = get_interface_ip($int);
			if (is_ipaddr($subnet)) {
				$sn = get_interface_subnet($int);
				$home_net .= "{$subnet}/{$sn} ";
			}
		} else {
			$ifcfg = $config['interfaces'][$int];
			switch ($ifcfg['ipaddr']) {
			case "pppoe":
			case "pptp":
			case "l2tp":
				if (function_exists('get_interface_ip'))
					$subnet = get_interface_ip($int);
				else
					$subnet = find_interface_ip("ng0");

				if (is_ipaddr($subnet))
					$home_net .= "{$subnet} ";
				break;
			case "dhcp":
				$subnet = find_interface_ip(snort_get_real_interface($int));
				if (is_ipaddr($subnet))
					$home_net .= "{$subnet} ";
				break;
			default:
				if (is_ipaddr($ifcfg['ipaddr'])) {
					$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
					if ($ifcfg['subnet'])
						$home_net .= "{$subnet}/{$ifcfg['subnet']} ";
				}
				break;
			}
		}
	}

	if ($snort_pfsense_basever == 'yes' && $wanip == 'yes') {
		/* add all WAN ips to the whitelist */
		$wan_if = get_real_wan_interface();
		$ip = find_interface_ip($wan_if);
		if (is_ipaddr($ip))
			$home_net .= "{$ip} ";
	}

	if ($wangw == 'yes') {
		/* Add Gateway on WAN interface to whitelist (For RRD graphs) */
		$gw = get_interface_gateway('wan');
		if($gw)
			$home_net .= "{$gw} ";
	}

	if($wandns == 'yes') {
		/* Add DNS server for WAN interface to whitelist */
		$dns_servers = get_dns_servers();
		foreach ($dns_servers as $dns) {
			if($dns)
				$home_net .= "{$dns} ";
		}
	}

	if($vips == 'yes') {
		/* iterate all vips and add to whitelist */
		if (is_array($config['virtualip']) && is_array($config['virtualip']['vip'])) {
			foreach($config['virtualip']['vip'] as $vip)
				if($vip['subnet'])
					$home_net .= "{$vip['subnet']} ";
		}
	}

	/* Add loopback to whitelist (ftphelper) */
	$home_net .= "127.0.0.1 ";

	/* grab a list of vpns and whitelist if user desires added by nestorfish 954 */
	if ($vpns == 'yes') {
		if ($snort_pfsense_basever == 'yes') // chk what pfsense version were on
			$vpns_list = get_vpns_list();
		else if ($snort_pfsense_basever == 'no') // chk what pfsense version were on
			$vpns_list = filter_get_vpns_list();

		if (!empty($vpns_list))
			$home_net .= "{$vpns_list} ";
	}

	/* never ever compair numbers to words */
	if ($userwips > -1) {
		if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
			$config['installedpackages']['snortglobal']['whitelist']['item'] = array();

		$home_net .= $config['installedpackages']['snortglobal']['whitelist']['item'][$userwips]['address'];
	}

	$home_net = trim($home_net);

	/* this foe whitelistfile, convert spaces to carriage returns */
	if ($build_netlist == 'whitelist') {
		$whitelist_home_net = str_replace("  ", "\n", $home_net);
		$whitelist_home_net = str_replace(" ", "\n", $home_net);
		return $whitelist_home_net;
	}

	/* this is for snort.conf */
	$validator = explode(" ", $home_net);
	$valresult = array();
	foreach ($validator as $vald) { 
		if (empty($vald))
			continue;
		$valresult[] = $vald;
	}
	$home_net = implode(",", $valresult);
	$home_net = "[{$home_net}]";

	return $home_net;
}


/* checks to see if snort is running yes/no and stop/start */
function Running_Ck($snort_uuid, $if_real, $id) {
	global $config, $g;

	$snort_uph = 'no';
	if (file_exists("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid") && isvalidpid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid"))
		$snort_uph = 'yes';

	return $snort_uph;
}

/* checks to see if barnyard2 is running yes/no */
function Running_Ck_b($snort_uuid, $if_real, $id) {
	global $config, $g;

	$snort_up_b = 'no';
	if (file_exists("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid") && isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid"))
		$snort_up_b = 'yes';

	return $snort_up_b;
}

function Running_Stop($snort_uuid, $if_real, $id) {
	global $config, $g;

	if (isvalidpid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid")) {
		killbypid("{$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid");
		exec("/bin/rm {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid");
	}

	if (isvalidpid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid")) {
		killbypid("{$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid");
		exec("/bin/rm {$g['varrun_path']}/barnyard2_{$snort_uuid}_{$if_real}*");
	}

	/* Log Iface stop */
	exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'Interface Rule STOP for {$snort_uuid}_{$if_real}...'");
	sleep(2); // Give time so GUI displays correctly
}

function Running_Start($snort_uuid, $if_real, $id) {
	global $config, $g;

	$snort_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['enable'];
	if ($snort_info_chk == 'on')
		exec("/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}");
	else
		return;

	/* define snortbarnyardlog_chk */
	/* top will have trouble if the uuid is to far back */
	$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
	$snortbarnyardlog_mysql_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_mysql'];
	if ($snortbarnyardlog_info_chk == 'on' && $snortbarnyardlog_mysql_info_chk != '')
		exec("/usr/local/bin/barnyard2 -r {$snort_uuid} -f \"snort_{$snort_uuid}_{$if_real}.u2\" --pid-path {$g['varrun_path']} --nolock-pidfile -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q");

	/* Log Iface stop */
	exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'Interface Rule START for {$id}_{$snort_uuid}_{$if_real}...'");
	sleep(2); // Give time so GUI displays correctly
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
function post_delete_logs()
{
	global $config, $g;

	/* do not start config build if rules is empty */
	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	foreach ($config['installedpackages']['snortglobal']['rule'] as $value) {
		$if_real = snort_get_real_interface($value['interface']);
		$snort_uuid = $value['uuid'];
		$snort_log_dir = "/var/log/snort/snort_{$if_real}{$snort_uuid}";

		if ($if_real != '' && $snort_uuid != '') {
			$filelist = glob("{$snort_log_dir}/*{$snort_uuid}_{$if_real}.u2.*");
			unset($filelist[count($filelist) - 1]);
			foreach ($filelist as $file)
				@unlink($file);
			$filelist = glob("{$snort_log_dir}/*{$snort_uuid}_{$if_real}.tcpdump.*");
			unset($filelist[count($filelist) - 1]);
			foreach ($filelist as $file)
				@unlink($file);

			/* create barnyard2 configuration file */
			//if ($value['barnyard_enable'] == 'on')
			//create_barnyard2_conf($id, $if_real, $snort_uuid);

			if ($value['perform_stat'] == 'on')
				@file_put_contents("{$snort_log_dir}/{$if_real}.stats", "");
		}
	}
}

function snort_postinstall()
{
	global $config, $g, $snort_pfsense_basever, $snort_arch;

	/* snort -> advanced features */
	if (is_array($config['installedpackages']['snortglobal'])) {
		$bpfbufsize = $config['installedpackages']['snortglobal']['bpfbufsize'];
		$bpfmaxbufsize = $config['installedpackages']['snortglobal']['bpfmaxbufsize'];
		$bpfmaxinsns = $config['installedpackages']['snortglobal']['bpfmaxinsns'];
	}

	/* cleanup default files */
	@rename('/usr/local/etc/snort/snort.conf-sample', '/usr/local/etc/snort/snort.conf');
	@rename('/usr/local/etc/snort/threshold.conf-sample', '/usr/local/etc/snort/threshold.conf');
	@rename('/usr/local/etc/snort/sid-msg.map-sample', '/usr/local/etc/snort/sid-msg.map');
	@rename('/usr/local/etc/snort/unicode.map-sample', '/usr/local/etc/snort/unicode.map');
	@rename('/usr/local/etc/snort/classification.config-sample', '/usr/local/etc/snort/classification.config');
	@rename('/usr/local/etc/snort/generators-sample', '/usr/local/etc/snort/generators');
	@rename('/usr/local/etc/snort/reference.config-sample', '/usr/local/etc/snort/reference.config');
	@rename('/usr/local/etc/snort/gen-msg.map-sample', '/usr/local/etc/snort/gen-msg.map');
	@unlink('/usr/local/etc/snort/sid');
	@unlink('/usr/local/etc/rc.d/snort');
	@unlink('/usr/local/etc/rc.d/barnyard2');

	/* remove example files */
	if (file_exists('/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so.0'))
		exec('/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example*');

	if (file_exists('/usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example.so'))
		exec('/bin/rm /usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example*');

	mwexec('/usr/sbin/chown -R snort:snort /var/log/snort', true);
	mwexec('/usr/sbin/chown -R snort:snort /usr/local/etc/snort', true);
	mwexec('/usr/sbin/chown -R snort:snort /usr/local/lib/snort', true);
	mwexec('/usr/sbin/chown snort:snort /tmp/snort*', true);
	mwexec('/usr/sbin/chown snort:snort /var/db/whitelist', true);

	/* move files around, make it look clean */
	mwexec('/bin/mkdir -p /usr/local/www/snort/css');
	mwexec('/bin/mkdir -p /usr/local/www/snort/images');

	chdir ("/usr/local/www/snort/css/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/css/style.css');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/css/sexybuttons.css');
	chdir("/usr/local/www/snort/images/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/alert.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/down.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/down2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon-table-sort.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon-table-sort-asc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon-table-sort-desc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/up.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/up2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/logo.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon_excli.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/arrow_down.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/awesome-overlay-sprite.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/logo22.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/page_white_text.png');

	/* remake saved settings */
	if ($config['installedpackages']['snortglobal']['forcekeepsettings'] == 'on') {
		update_status(gettext("Saved settings detected..."));
		update_output_window(gettext("Please wait... rebuilding files..."));
		sync_snort_package_config();
		update_output_window(gettext("Finnished Rebuilding files..."));
	}
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
		if (strstr($item['command'], '/usr/local/pkg/snort/snort_check_cron_misc.inc')) {
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
		if(!$is_installed) {
			$cron_item = array();
			$cron_item['minute'] = "$snort_rm_blocked_min";
			$cron_item['hour'] = "$snort_rm_blocked_hr";
			$cron_item['mday'] = "$snort_rm_blocked_mday";
			$cron_item['month'] = "$snort_rm_blocked_month";
			$cron_item['wday'] = "$snort_rm_blocked_wday";
			$cron_item['who'] = "root";
			$cron_item['command'] = "/usr/bin/nice -n20 /usr/local/sbin/expiretable -t $snort_rm_blocked_expire snort2c";
			$config['cron']['item'][] = $cron_item;
		}
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
		if(!$is_installed) {
			$cron_item = array();
			$cron_item['minute'] = "$snort_rules_up_min";
			$cron_item['hour'] = "$snort_rules_up_hr";
			$cron_item['mday'] = "$snort_rules_up_mday";
			$cron_item['month'] = "$snort_rules_up_month";
			$cron_item['wday'] = "$snort_rules_up_wday";
			$cron_item['who'] = "root";
			$cron_item['command'] = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/snort/snort_check_for_rule_updates.php >> /tmp/snort_update.log";
			$config['cron']['item'][] = $cron_item;
		}
		break;
	case false:
		if($is_installed == true)
			unset($config['cron']['item'][$x]);
		break;
	}
}

/* Only run when all ifaces needed to sync. Expects filesystem rw */
function sync_snort_package_config()
{
	global $config, $g;

	/* RedDevil suggested code */
	/* TODO: more testing needs to be done */
	/* may cause voip to fail */
	//exec("/sbin/sysctl net.bpf.bufsize=8388608");
	//exec("/sbin/sysctl net.bpf.maxbufsize=4194304");
	//exec("/sbin/sysctl net.bpf.maxinsns=512");
	//exec("/sbin/sysctl net.inet.tcp.rfc1323=1");

	conf_mount_rw();

	/* do not start config build if rules is empty */
	if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
		exec('/bin/rm /usr/local/etc/rc.d/snort.sh');
		conf_mount_ro();
		return;
	}

	foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {
		$if_real = snort_get_real_interface($value['interface']);
		$snort_uuid = $value['uuid'];

		if ($if_real != '' && $snort_uuid != '') {

			/* only build whitelist when needed */
			if ($value['blockoffenders7'] == 'on')
				create_snort_whitelist($id, $if_real);

			/* only build threshold when needed */
			if ($value['suppresslistname'] != 'default')
				create_snort_suppress($id, $if_real);

			/* create snort configuration file */
			create_snort_conf($id, $if_real, $snort_uuid);

			/* if rules exist cp rules to each iface */
			create_rules_iface($id, $if_real, $snort_uuid);

			/* create barnyard2 configuration file */
			if ($value['barnyard_enable'] == 'on')
				create_barnyard2_conf($id, $if_real, $snort_uuid);
		}
	}

	/* create snort bootup file snort.sh only create once */
	create_snort_sh();

	/* all new files are for the user snort nologin */
	if (!is_dir('/var/log/snort'))
		exec('/bin/mkdir -p /var/log/snort');

	conf_mount_ro();
}

/* Start of main config files */

/* create threshold file */
function create_snort_suppress($id, $if_real) {
	global $config, $g;

	/* make sure dir is there */
	if (!is_dir('/usr/local/etc/snort/suppress'))
		exec('/bin/mkdir -p /usr/local/etc/snort/suppress');

	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	if ($config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'] != 'default') {
		$whitelist_key_s = find_suppress_key($config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname']);

		/* file name */
		$suppress_file_name = $config['installedpackages']['snortglobal']['suppress']['item'][$whitelist_key_s]['name'];

		/* Message */
		$s_data = '# This file is auto generated by the snort package. Please do not edit this file by hand.' . "\n\n";

		/* user added arguments */
		$s_data .= str_replace("\r", "", base64_decode($config['installedpackages']['snortglobal']['suppress']['item'][$whitelist_key_s]['suppresspassthru']));

		/* open snort's whitelist for writing */
		@file_put_contents("/usr/local/etc/snort/suppress/$suppress_file_name", $s_data);
	}
}

function create_snort_whitelist($id, $if_real) {
	global $config, $g;

	/* make sure dir is there */
	if (!is_dir('/usr/local/etc/snort/whitelist'))
		exec('/bin/mkdir -p /usr/local/etc/snort/whitelist');

	if ($config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'] == 'default') {

		$w_data = build_base_whitelist('whitelist', 'yes', 'yes', 'yes', 'yes', 'yes', 'no');

		/* open snort's whitelist for writing */
		@file_put_contents("/usr/local/etc/snort/whitelist/defaultwlist", $w_data);

	} else if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'])) {
		$whitelist_key_w = find_whitelist_key($config['installedpackages']['snortglobal']['rule'][$id]['whitelistname']);

		if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
			return;

		$whitelist = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w];
		$w_data = build_base_whitelist($whitelist['snortlisttype'], $whitelist['wanips'], $whitelist['wangateips'],
				$whitelist['wandnsips'], $whitelist['vips'], $whitelist['vpnips'], $whitelist_key_w);

		/* open snort's whitelist for writing */
		@file_put_contents("/usr/local/etc/snort/whitelist/" . $config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'], $w_data);
	}
}

function create_snort_homenet($id, $if_real) {
	global $config, $g;

	if ($config['installedpackages']['snortglobal']['rule'][$id]['homelistname'] == 'default' || $config['installedpackages']['snortglobal']['rule'][$id]['homelistname'] == '')
		return build_base_whitelist('netlist', 'yes', 'yes', 'yes', 'yes', 'yes', 'no');
	else if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['homelistname'])) {
		$whitelist_key_h = find_whitelist_key($config['installedpackages']['snortglobal']['rule'][$id]['homelistname']);

		if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
			return;

		$build_netlist_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['snortlisttype'];
		$wanip_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['wanips'];
		$wangw_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['wangateips'];
		$wandns_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['wandnsips'];
		$vips_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['vips'];
		$vpns_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['vpnips'];

		return build_base_whitelist($build_netlist_h, $wanip_h, $wangw_h, $wandns_h, $vips_h, $vpns_h, $whitelist_key_h);
	}
}

function create_snort_externalnet($id, $if_real) {
	global $config, $g;

	if (!empty($config['installedpackages']['snortglobal']['rule'][$id]['externallistname'])) {
		$whitelist_key_ex = find_whitelist_key($config['installedpackages']['snortglobal']['rule'][$id]['externallistname']);

		if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
			return;

		$build_netlist_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['snortlisttype'];
		$wanip_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['wanips'];
		$wangw_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['wangateips'];
		$wandns_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['wandnsips'];
		$vips_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['vips'];
		$vpns_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['vpnips'];

		return build_base_whitelist($build_netlist_ex, $wanip_ex, $wangw_ex, $wandns_ex, $vips_ex, $vpns_ex, $whitelist_key_ex);
	}
}

/* open snort.sh for writing" */
function create_snort_sh() {
	global $config, $g;

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

		/* define snortbarnyardlog_chk */
		$snortbarnyardlog_info_chk = $value['barnyard_enable'];
		$snortbarnyardlog_mysql_info_chk = $value['barnyard_mysql'];

		if ($snortbarnyardlog_info_chk == 'on' && $snortbarnyardlog_mysql_info_chk != '')
			$start_barnyard2 = "sleep 4;/usr/local/bin/barnyard2 -r {$snort_uuid} -f snort_{$snort_uuid}_{$if_real}.u2 --pid-path {$g['varrun_path']} --nolock-pidfile -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort/snort_{$if_real}{$snort_uuid} -D -q";
		else
			$start_barnyard2 = "";

		$start_snort_iface_start[] = <<<EOE

###### For Each Iface

# Start snort and barnyard2
/usr/local/bin/snort -R {$snort_uuid} -D -q -l /var/log/snort/snort_{$if_real}{$snort_uuid} --pid-path {$g['varrun_path']} --nolock-pidfile -G {$snort_uuid} -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}
$start_barnyard2

/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort HARD START For {$snort_uuid}_{$if_real}..."

EOE;
		$start_snort_iface_stop[] = <<<EOF

if [ -e {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid ]; then
	if [ `/bin/pgrep -nF {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid` -gt 3 ]; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort HARD STOP For {$snort_uuid}_{$if_real}..."

		/bin/pkill -F {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid -a
		sleep 1
		if [ -f {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid ]; then
			/bin/pkill -F {$g['varrun_path']}/barnyard2_{$if_real}{$snort_uuid}.pid -a
			/bin/rm /var/run/barnyard2_{$if_real}{$snort_uuid}.pid
		fi

		# Delete pid file, just in case pkill didn't do the job.
		if [ -e {$g['varrun_path']}/snort_{$if_real}{$snort_uuid}.pid ]; then
			/bin/rm /var/run/snort_{$if_real}{$snort_uuid}.pid
		fi
	fi
fi

EOF;
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
# Kill running instances before restarting them or, if system startup,
# handle the issue with snort starting an instance twice.
rc_stop
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
		# rc_start handles both stopping and starting of snort.
		rc_start
		;;
esac

EOD;

	/* write out snort.sh */
	if (!@file_put_contents("/usr/local/etc/rc.d/snort.sh", $snort_sh_text)) {
		log_error("Could not open /usr/local/etc/rc.d/snort.sh for writing.");
		return;
	}
	@chmod("/usr/local/etc/rc.d/snort.sh", 0755);
}

/* if rules exist copy to new interfaces */
function create_rules_iface($id, $if_real, $snort_uuid)
{
	global $config, $g;

	$if_rule_dir = "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}";
	$folder_chk = (count(glob("{$if_rule_dir}/rules/*")) === 0) ? 'empty' : 'full';

	if ($folder_chk == "empty") {
		if (!is_dir("{$if_rule_dir}/rules"))
                        exec("/bin/mkdir -p {$if_rule_dir}/rules");
		exec("/bin/cp /usr/local/etc/snort/rules/* {$if_rule_dir}/rules");
		if (file_exists("/usr/local/etc/snort/custom_rules/local_{$snort_uuid}_{$if_real}.rules"))
			exec("/bin/cp /usr/local/etc/snort/custom_rules/local_{$snort_uuid}_{$if_real}.rules {$if_rule_dir}/local_{$snort_uuid}_{$if_real}.rules");
	}
}

/* open barnyard2.conf for writing */
function create_barnyard2_conf($id, $if_real, $snort_uuid) {
	global $config, $g;

	if (!file_exists("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf"))
		exec("/usr/bin/touch /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf");

	if (!file_exists("/var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2/{$snort_uuid}_{$if_real}.waldo")) {
		@touch("/var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2/{$snort_uuid}_{$if_real}.waldo");
		mwexec("/bin/chmod 770 /var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo", true);
	}

	$barnyard2_conf_text = generate_barnyard2_conf($id, $if_real, $snort_uuid);

	/* write out barnyard2_conf */
	@file_put_contents("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf", $barnyard2_conf_text);
}

/* open barnyard2.conf for writing" */
function generate_barnyard2_conf($id, $if_real, $snort_uuid) {
	global $config, $g;

	/* define snortbarnyardlog */
	/* TODO: add support for the other 5 output plugins */

	$snortbarnyardlog_database_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_mysql'];
	$snortbarnyardlog_hostname_info_chk = exec("/bin/hostname");
	/* user add arguments */
	$snortbarnyardlog_config_pass_thru = str_replace("\r", "", base64_decode($config['installedpackages']['snortglobal']['rule'][$id]['barnconfigpassthru']));

	$barnyard2_conf_text = <<<EOD

#	barnyard2.conf
#   barnyard2 can be found at http://www.securixlive.com/barnyard2/index.php
#
# set the appropriate paths to the file(s) your Snort process is using

config reference_file:	    /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/reference.config
config classification_file: /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/classification.config
config gen_file:            /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/gen-msg.map
config sid_file:	        /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/sid-msg.map

config hostname:       $snortbarnyardlog_hostname_info_chk
config interface:      {$snort_uuid}_{$if_real}
config decode_data_link
config waldo_file: /var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2/{$snort_uuid}_{$if_real}.waldo

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

function create_snort_conf($id, $if_real, $snort_uuid)
{
	global $config, $g;

	if (!empty($if_real) && !empty($snort_uuid)) {
		if (!is_dir("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}")) {
			exec("/bin/mkdir -p /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}");
			@touch("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf");
		}

		$snort_conf_text = generate_snort_conf($id, $if_real, $snort_uuid);
		if (empty($snort_conf_text))
			return;

		/* write out snort.conf */
		$conf = fopen("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf", "w");
		if(!$conf) {
			log_error("Could not open /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf for writing.");
			return -1;
		}
		fwrite($conf, $snort_conf_text);
		fclose($conf);
	}
}

function snort_deinstall() {
	global $config, $g;

	/* remove custom sysctl */
	remove_text_from_file("/etc/sysctl.conf", "sysctl net.bpf.bufsize=20480");

	/* decrease bpf buffers back to 4096, from 20480 */
	exec('/sbin/sysctl net.bpf.bufsize=4096');
	mwexec('/usr/bin/killall snort', true);
	sleep(2);
	mwexec('/usr/bin/killall -9 snort', true);
	sleep(2);
	mwexec('/usr/bin/killall barnyard2', true);
	sleep(2);
	mwexec('/usr/bin/killall -9 barnyard2', true);
	sleep(2);
	mwexec('/usr/sbin/pw userdel snort; /usr/sbin/pw groupdel snort', true);
	mwexec('/bin/rm -rf /usr/local/etc/snort*; /bin/rm -rf /usr/local/pkg/snort*', true);
	mwexec('/bin/rm -rf /usr/local/www/snort; /bin/rm -rf /var/log/snort', true);

	/* Remove snort cron entries Ugly code needs smoothness*/
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

	snort_deinstall_cron("snort2c");
	snort_deinstall_cron("snort_check_for_rule_updates.php");
	snort_deinstall_cron("/usr/local/pkg/snort/snort_check_cron_misc.inc");
	configure_cron();

	/* Unset snort registers in conf.xml IMPORTANT snort will not start with out this */
	/* Keep this as a last step */
	if ($config['installedpackages']['snortglobal']['forcekeepsettings'] != 'on')
		unset($config['installedpackages']['snortglobal']);
}

function generate_snort_conf($id, $if_real, $snort_uuid)
{
	global $config, $g, $snort_pfsense_basever;

	if (!is_array($config['installedpackages']['snortglobal']['rule']))
		return;

	$snortcfg =& $config['installedpackages']['snortglobal']['rule'][$id];

	/* custom home nets */
	$home_net = create_snort_homenet($id, $if_real);

	if ($snortcfg['externallistname'] == 'default')
		$external_net = '!$HOME_NET';
	else
		$external_net = create_snort_externalnet($id, $if_real);

	/* obtain external interface */
	$snort_ext_int = $snortcfg['interface'];

	/* user added arguments */
	$snort_config_pass_thru = str_replace("\r", "", base64_decode($snortcfg['configpassthru']));

	/* create a few directories and ensure the sample files are in place */
	if (!is_dir('/usr/local/etc/snort'))
		exec('/bin/mkdir -p /usr/local/etc/snort/custom_rules');
	if (!is_dir("/usr/local/etc/snort/snort/snort_{$snort_uuid}_{$if_real}"))
		exec("/bin/mkdir -p /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}");
	if (!is_dir("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules"))
		exec("/bin/mkdir -p /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules");
	if (!is_dir('/usr/local/etc/snort/whitelist'))
		exec('/bin/mkdir -p /usr/local/etc/snort/whitelist/');
	if (!is_dir("/var/log/snort/snort_{$if_real}{$snort_uuid}"))
		exec("/bin/mkdir -p /var/log/snort/snort_{$if_real}{$snort_uuid}");
	if (!is_dir("/var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2"))
		exec("/bin/mkdir -p /var/log/snort/snort_{$if_real}{$snort_uuid}/barnyard2");
	if (!is_dir('/usr/local/lib/snort/dynamicrules'))
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules');
	if (!is_dir('/usr/local/lib/snort/dynamicengine'))
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicengine');
	if (!is_dir('/usr/local/lib/snort/dynamicpreprocessor'))
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicpreprocessor');
	if (!file_exists('/var/db/whitelist'))
		@touch('/var/db/whitelist');

	@copy("/usr/local/etc/snort/gen-msg.map", "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/gen-msg.map");
	@copy("/usr/local/etc/snort/classification.config", "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/classification.config");
	@copy("/usr/local/etc/snort/reference.config", "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/reference.config");
	@copy("/usr/local/etc/snort/sid-msg.map", "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/sid-msg.map");
	@copy("/usr/local/etc/snort/unicode.map", "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/unicode.map");
	@copy("/usr/local/etc/snort/threshold.conf", "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/threshold.conf");
	@touch("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf");

	/* define basic log filename */
	$snortunifiedlogbasic_type = "output unified: filename snort_{$snort_uuid}_{$if_real}.log, limit 128";

	/* define snortalertlogtype */
	if ($config['installedpackages']['snortglobal']['snortalertlogtype'] == "fast")
		$snortalertlogtype_type = "output alert_fast: alert";
	else
		$snortalertlogtype_type = "output alert_full: alert";

	/* define alertsystemlog */
	$alertsystemlog_type = "";
	if ($snortcfg['alertsystemlog'] == "on")
		$alertsystemlog_type = "output alert_syslog: log_alert";

	/* define tcpdumplog */
	$tcpdumplog_type = "";
	if ($snortcfg['tcpdumplog'] == "on")
		$tcpdumplog_type = "output log_tcpdump: snort_{$snort_uuid}_{$if_real}.tcpdump";

	/* define snortunifiedlog */
	$snortunifiedlog_type = "";
	if ($snortcfg['snortunifiedlog'] == "on")
		$snortunifiedlog_type = "output unified2: filename snort_{$snort_uuid}_{$if_real}.u2, limit 128";

	/* define spoink */
	$spoink_type = "";
	if ($snortcfg['blockoffenders7'] == "on") {
		if ($snortcfg['whitelistname'] == "default")
			$spoink_whitelist_name = 'defaultwlist';
		else if (file_exists("/usr/local/etc/snort/whitelist/{$snortcfg['whitelistname']}"))
			$spoink_whitelist_name = $snortcfg['whitelistname'];

		$pfkill = "";
		if ($snortcfg['blockoffenderskill'] == "on")
			$pfkill = "kill";
		
		$spoink_type = "output alert_pf: /usr/local/etc/snort/whitelist/{$spoink_whitelist_name},snort2c,{$snortcfg['blockoffendersip']},{$pfkill}";
	}

	/* define threshold file */
	$threshold_file_name = "";
	if ($snortcfg['suppresslistname'] != 'default') {
		if (file_exists("/usr/local/etc/snort/suppress/{$snortcfg['suppresslistname']}"))
			$threshold_file_name = "include /usr/local/etc/snort/suppress/{$snortcfg['suppresslistname']}";
	}

	/* define servers and ports snortdefservers */
	/* def DNS_SERVSERS */
	$def_dns_servers_info_chk = $snortcfg['def_dns_servers'];
	if ($def_dns_servers_info_chk == "")
		$def_dns_servers_type = "\$HOME_NET";
	else
		$def_dns_servers_type = "$def_dns_servers_info_chk";

	/* def DNS_PORTS */
	$def_dns_ports_info_chk = $snortcfg['def_dns_ports'];
	if ($def_dns_ports_info_chk == "")
		$def_dns_ports_type = "53";
	else
		$def_dns_ports_type = "$def_dns_ports_info_chk";

	/* def SMTP_SERVSERS */
	$def_smtp_servers_info_chk = $snortcfg['def_smtp_servers'];
	if ($def_smtp_servers_info_chk == "")
		$def_smtp_servers_type = "\$HOME_NET";
	else
		$def_smtp_servers_type = "$def_smtp_servers_info_chk";

	/* def SMTP_PORTS */
	$def_smtp_ports_info_chk = $snortcfg['def_smtp_ports'];
	if ($def_smtp_ports_info_chk == "")
		$def_smtp_ports_type = "25";
	else
		$def_smtp_ports_type = "$def_smtp_ports_info_chk";

	/* def MAIL_PORTS */
	$def_mail_ports_info_chk = $snortcfg['def_mail_ports'];
	if ($def_mail_ports_info_chk == "")
		$def_mail_ports_type = "25,143,465,691";
	else
		$def_mail_ports_type = "$def_mail_ports_info_chk";

	/* def HTTP_SERVSERS */
	$def_http_servers_info_chk = $snortcfg['def_http_servers'];
	if ($def_http_servers_info_chk == "")
		$def_http_servers_type = "\$HOME_NET";
	else
		$def_http_servers_type = "$def_http_servers_info_chk";

	/* def WWW_SERVSERS */
	$def_www_servers_info_chk = $snortcfg['def_www_servers'];
	if ($def_www_servers_info_chk == "")
		$def_www_servers_type = "\$HOME_NET";
	else
		$def_www_servers_type = "$def_www_servers_info_chk";

	/* def HTTP_PORTS */
	$def_http_ports_info_chk = $snortcfg['def_http_ports'];
	if ($def_http_ports_info_chk == "")
		$def_http_ports_type = "80";
	else
		$def_http_ports_type = "$def_http_ports_info_chk";

	/* def SQL_SERVSERS */
	$def_sql_servers_info_chk = $snortcfg['def_sql_servers'];
	if ($def_sql_servers_info_chk == "")
		$def_sql_servers_type = "\$HOME_NET";
	else
		$def_sql_servers_type = "$def_sql_servers_info_chk";

	/* def ORACLE_PORTS */
	$def_oracle_ports_info_chk = $snortcfg['def_oracle_ports'];
	if ($def_oracle_ports_info_chk == "")
		$def_oracle_ports_type = "1521";
	else
		$def_oracle_ports_type = "$def_oracle_ports_info_chk";

	/* def MSSQL_PORTS */
	$def_mssql_ports_info_chk = $snortcfg['def_mssql_ports'];
	if ($def_mssql_ports_info_chk == "")
		$def_mssql_ports_type = "1433";
	else
		$def_mssql_ports_type = "$def_mssql_ports_info_chk";

	/* def TELNET_SERVSERS */
	$def_telnet_servers_info_chk = $snortcfg['def_telnet_servers'];
	if ($def_telnet_servers_info_chk == "")
		$def_telnet_servers_type = "\$HOME_NET";
	else
		$def_telnet_servers_type = "$def_telnet_servers_info_chk";

	/* def TELNET_PORTS */
	$def_telnet_ports_info_chk = $snortcfg['def_telnet_ports'];
	if ($def_telnet_ports_info_chk == "")
		$def_telnet_ports_type = "23";
	else
		$def_telnet_ports_type = "$def_telnet_ports_info_chk";

	/* def SNMP_SERVSERS */
	$def_snmp_servers_info_chk = $snortcfg['def_snmp_servers'];
	if ($def_snmp_servers_info_chk == "")
		$def_snmp_servers_type = "\$HOME_NET";
	else
		$def_snmp_servers_type = "$def_snmp_servers_info_chk";

	/* def SNMP_PORTS */
	$def_snmp_ports_info_chk = $snortcfg['def_snmp_ports'];
	if ($def_snmp_ports_info_chk == "")
		$def_snmp_ports_type = "161";
	else
		$def_snmp_ports_type = "$def_snmp_ports_info_chk";

	/* def FTP_SERVSERS */
	$def_ftp_servers_info_chk = $snortcfg['def_ftp_servers'];
	if ($def_ftp_servers_info_chk == "")
		$def_ftp_servers_type = "\$HOME_NET";
	else
		$def_ftp_servers_type = "$def_ftp_servers_info_chk";

	/* def FTP_PORTS */
	$def_ftp_ports_info_chk = $snortcfg['def_ftp_ports'];
	if ($def_ftp_ports_info_chk == "")
		$def_ftp_ports_type = "21";
	else
		$def_ftp_ports_type = "$def_ftp_ports_info_chk";

	/* def SSH_SERVSERS */
	$def_ssh_servers_info_chk = $snortcfg['def_ssh_servers'];
	if ($def_ssh_servers_info_chk == "")
		$def_ssh_servers_type = "\$HOME_NET";
	else
		$def_ssh_servers_type = "$def_ssh_servers_info_chk";

	/* if user has defined a custom ssh port, use it */
	if(isset($config['system']['ssh']['port']))
		$ssh_port = $config['system']['ssh']['port'];
	else
		$ssh_port = "22";

	/* def SSH_PORTS */
	$def_ssh_ports_info_chk = $snortcfg['def_ssh_ports'];
	if ($def_ssh_ports_info_chk == "")
		$def_ssh_ports_type = "{$ssh_port}";
	else
		$def_ssh_ports_type = "$def_ssh_ports_info_chk";

	/* def POP_SERVSERS */
	$def_pop_servers_info_chk = $snortcfg['def_pop_servers'];
	if ($def_pop_servers_info_chk == "")
		$def_pop_servers_type = "\$HOME_NET";
	else
		$def_pop_servers_type = "$def_pop_servers_info_chk";

	/* def POP2_PORTS */
	$def_pop2_ports_info_chk = $snortcfg['def_pop2_ports'];
	if ($def_pop2_ports_info_chk == "")
		$def_pop2_ports_type = "109";
	else
		$def_pop2_ports_type = "$def_pop2_ports_info_chk";

	/* def POP3_PORTS */
	$def_pop3_ports_info_chk = $snortcfg['def_pop3_ports'];
	if ($def_pop3_ports_info_chk == "")
		$def_pop3_ports_type = "110";
	else
		$def_pop3_ports_type = "$def_pop3_ports_info_chk";

	/* def IMAP_SERVSERS */
	$def_imap_servers_info_chk = $snortcfg['def_imap_servers'];
	if ($def_imap_servers_info_chk == "")
		$def_imap_servers_type = "\$HOME_NET";
	else
		$def_imap_servers_type = "$def_imap_servers_info_chk";

	/* def IMAP_PORTS */
	$def_imap_ports_info_chk = $snortcfg['def_imap_ports'];
	if ($def_imap_ports_info_chk == "")
		$def_imap_ports_type = "143";
	else
		$def_imap_ports_type = "$def_imap_ports_info_chk";

	/* def SIP_PROXY_IP */
	$def_sip_proxy_ip_info_chk = $snortcfg['def_sip_proxy_ip'];
	if ($def_sip_proxy_ip_info_chk == "")
		$def_sip_proxy_ip_type = "\$HOME_NET";
	else
		$def_sip_proxy_ip_type = "$def_sip_proxy_ip_info_chk";

	/* def SIP_PROXY_PORTS */
	$def_sip_proxy_ports_info_chk = $snortcfg['def_sip_proxy_ports'];
	if ($def_sip_proxy_ports_info_chk == "")
		$def_sip_proxy_ports_type = "5060:5090,16384:32768";
	else
		$def_sip_proxy_ports_type = "$def_sip_proxy_ports_info_chk";
		
	/* def SIP_SERVERS */
	$def_sip_servers_info_chk = $snortcfg['def_sip_servers'];
	if ($def_sip_servers_info_chk == "")
		$def_sip_servers_type = "\$HOME_NET";
	else
		$def_sip_servers_type = "$def_sip_servers_info_chk";

	/* def SIP_PORTS */
	$def_sip_ports_info_chk = $snortcfg['def_sip_ports'];
	if ($def_sip_ports_info_chk == "")
		$def_sip_ports_type = "5060:5090,16384:32768";
	else
		$def_sip_ports_type = "$def_sip_ports_info_chk";

	/* def AUTH_PORTS */
	$def_auth_ports_info_chk = $snortcfg['def_auth_ports'];
	if ($def_auth_ports_info_chk == "")
		$def_auth_ports_type = "113";
	else
		$def_auth_ports_type = "$def_auth_ports_info_chk";

	/* def FINGER_PORTS */
	$def_finger_ports_info_chk = $snortcfg['def_finger_ports'];
	if ($def_finger_ports_info_chk == "")
		$def_finger_ports_type = "79";
	else
		$def_finger_ports_type = "$def_finger_ports_info_chk";

	/* def IRC_PORTS */
	$def_irc_ports_info_chk = $snortcfg['def_irc_ports'];
	if ($def_irc_ports_info_chk == "")
		$def_irc_ports_type = "6665,6666,6667,6668,6669,7000";
	else
		$def_irc_ports_type = "$def_irc_ports_info_chk";

	/* def NNTP_PORTS */
	$def_nntp_ports_info_chk = $snortcfg['def_nntp_ports'];
	if ($def_nntp_ports_info_chk == "")
		$def_nntp_ports_type = "119";
	else
		$def_nntp_ports_type = "$def_nntp_ports_info_chk";

	/* def RLOGIN_PORTS */
	$def_rlogin_ports_info_chk = $snortcfg['def_rlogin_ports'];
	if ($def_rlogin_ports_info_chk == "")
		$def_rlogin_ports_type = "513";
	else
		$def_rlogin_ports_type = "$def_rlogin_ports_info_chk";

	/* def RSH_PORTS */
	$def_rsh_ports_info_chk = $snortcfg['def_rsh_ports'];
	if ($def_rsh_ports_info_chk == "")
		$def_rsh_ports_type = "514";
	else
		$def_rsh_ports_type = "$def_rsh_ports_info_chk";

	/* def SSL_PORTS */
	$def_ssl_ports_info_chk = $snortcfg['def_ssl_ports'];
	if ($def_ssl_ports_info_chk == "")
		$def_ssl_ports_type = "443,465,563,636,989,990,992,993,994,995";
	else
		$def_ssl_ports_type = "$def_ssl_ports_info_chk";

	/* if user is on pppoe, we really want to use ng0 interface */
	if ($snort_pfsense_basever == 'yes' && $snort_ext_int == "wan")
		$snort_ext_int = get_real_wan_interface();

	/* set the snort performance model */
	if($snortcfg['performance'])
		$snort_performance = $snortcfg['performance'];
	else
		$snort_performance = "ac-bnfa";


	/* generate rule sections to load */
	$enabled_rulesets = $snortcfg['rulesets'];
	$selected_rules_sections = "";
	if (!empty($enabled_rulesets)) {
		$enabled_rulesets_array = split("\|\|", $enabled_rulesets);
		foreach($enabled_rulesets_array as $enabled_item)
			$selected_rules_sections  .= "include \$RULE_PATH/{$enabled_item}\n";
	}

	/////////////////////////////

	/* preprocessor code */

	/* def perform_stat */
	$snort_perform_stat = <<<EOD
##########################
                         #
# NEW                    #
# Performance Statistics #
                         #
##########################

preprocessor perfmonitor: time 300 file /var/log/snort/snort_{$if_real}{$snort_uuid}/{$if_real}.stats pktcnt 10000
	
EOD;

	$def_perform_stat_info_chk = $snortcfg['perform_stat'];
	if ($def_perform_stat_info_chk == "on")
		$def_perform_stat_type = "$snort_perform_stat";
	else
		$def_perform_stat_type = "";

	$def_flow_depth_info_chk = $snortcfg['flow_depth'];
	if (empty($def_flow_depth_info_chk))
		$def_flow_depth_type = '0';
	else
		$def_flow_depth_type = $snortcfg['flow_depth'];

	/* def http_inspect */
	$snort_http_inspect = <<<EOD
#################
                #
# HTTP Inspect  #
                #
#################

preprocessor http_inspect: global iis_unicode_map unicode.map 1252 compress_depth 65535 decompress_depth 65535

preprocessor http_inspect_server: server default \
                        ports  { 80 8080 }  \
                        non_strict \
                        non_rfc_char  { 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 }  \
                        flow_depth {$def_flow_depth_type}  \
                        apache_whitespace no \
                        directory no \
                        iis_backslash no \
                        u_encode yes \
			extended_response_inspection \
			inspect_gzip \
			normalize_utf \
			normalize_javascript \
			unlimited_decompress \
                        ascii no \
                        chunk_length 500000 \
                        bare_byte yes \
                        double_decode yes \
                        iis_unicode no \
                        iis_delimiter no \
                        multi_slash no
	
EOD;

	$def_http_inspect_info_chk = $snortcfg['http_inspect'];
	if ($def_http_inspect_info_chk == "on")
		$def_http_inspect_type = "$snort_http_inspect";
	else
		$def_http_inspect_type = "";

	/* def other_preprocs */
	$snort_other_preprocs = <<<EOD
##################
                 #
# Other preprocs #
                 #
##################

preprocessor rpc_decode: 111 32770 32771 32772 32773 32774 32775 32776 32777 32778 32779
preprocessor bo
	
EOD;

	$def_other_preprocs_info_chk = $snortcfg['other_preprocs'];
	if ($def_other_preprocs_info_chk == "on")
		$def_other_preprocs_type = "$snort_other_preprocs";
	else
		$def_other_preprocs_type = "";

	/* def ftp_preprocessor */
	$snort_ftp_preprocessor = <<<EOD
#####################
                    #
# ftp preprocessor  #
                    #
#####################

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

	$def_ftp_preprocessor_info_chk = $snortcfg['ftp_preprocessor'];
	if ($def_ftp_preprocessor_info_chk == "on")
	$def_ftp_preprocessor_type = "$snort_ftp_preprocessor";
	else
	$def_ftp_preprocessor_type = "";

	/* def smtp_preprocessor */
	$snort_smtp_preprocessor = <<<EOD
#####################
                    #
# SMTP preprocessor #
                    #
#####################

preprocessor SMTP: \
    ports { 25 465 691 } \
    inspection_type stateful \
    normalize cmds \
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
    xlink2state { enable }
	
EOD;

	$def_smtp_preprocessor_info_chk = $snortcfg['smtp_preprocessor'];
	if ($def_smtp_preprocessor_info_chk == "on")
		$def_smtp_preprocessor_type = "$snort_smtp_preprocessor";
	else
		$def_smtp_preprocessor_type = "";

	/* def sf_portscan */
	$snort_sf_portscan = <<<EOD
################
               #
# sf Portscan  #
               #
################

preprocessor sfportscan: scan_type { all } \
                         proto  { all } \
                         memcap { 10000000 } \
                         sense_level { medium } \
                         ignore_scanners { \$HOME_NET }
	
EOD;

	$def_sf_portscan_info_chk = $snortcfg['sf_portscan'];
	if ($def_sf_portscan_info_chk == "on")
		$def_sf_portscan_type = "$snort_sf_portscan";
	else
		$def_sf_portscan_type = "";

	/* def dce_rpc_2 */
	$snort_dce_rpc_2 = <<<EOD
###############
              #
# NEW         #
# DCE/RPC 2   #
              #
###############

preprocessor dcerpc2: memcap 102400, events [smb, co, cl]
preprocessor dcerpc2_server: default, policy WinXP, \
    detect [smb [139,445], tcp 135, udp 135, rpc-over-http-server 593], \
    autodetect [tcp 1025:, udp 1025:, rpc-over-http-server 1025:], \
    smb_max_chain 3
	
EOD;

	$def_dce_rpc_2_info_chk = $snortcfg['dce_rpc_2'];
	if ($def_dce_rpc_2_info_chk == "on")
		$def_dce_rpc_2_type = "$snort_dce_rpc_2";
	else
		$def_dce_rpc_2_type = "";

	/* def dns_preprocessor */
	$snort_dns_preprocessor = <<<EOD
####################
                   #
# DNS preprocessor #
                   #
####################

preprocessor dns: \
    ports { 53 } \
    enable_rdata_overflow
	
EOD;

	$def_dns_preprocessor_info_chk = $snortcfg['dns_preprocessor'];
	if ($def_dns_preprocessor_info_chk == "on")
		$def_dns_preprocessor_type = "$snort_dns_preprocessor";
	else
		$def_dns_preprocessor_type = "";

	/* def SSL_PORTS IGNORE */
	$def_ssl_ports_ignore_info_chk = $snortcfg['def_ssl_ports_ignore'];
	if ($def_ssl_ports_ignore_info_chk == "")
		$def_ssl_ports_ignore_type = "443 465 563 636 989 990 992 993 994 995";
	else
		$def_ssl_ports_ignore_type = "$def_ssl_ports_ignore_info_chk";

	/* stream5 queued settings */


	$def_max_queued_bytes_info_chk = $snortcfg['max_queued_bytes'];
	if ($def_max_queued_bytes_info_chk == '')
		$def_max_queued_bytes_type = '';
	else
		$def_max_queued_bytes_type = ' max_queued_bytes ' . $snortcfg['max_queued_bytes'] . ',';

	$def_max_queued_segs_info_chk = $snortcfg['max_queued_segs'];
	if ($def_max_queued_segs_info_chk == '')
		$def_max_queued_segs_type = '';
	else
		$def_max_queued_segs_type = ' max_queued_segs ' . $snortcfg['max_queued_segs'] . ',';

	$snort_preprocessor_decoder_rules = "";
	if (file_exists("/usr/local/etc/snort/preproc_rules/preprocessor.rules"))
		$snort_preprocessor_decoder_rules .= "include \$PREPROC_RULE_PATH/preprocessor.rules\n";
	if (file_exists("/usr/local/etc/snort/preproc_rules/decoder.rules"))
		$snort_preprocessor_decoder_rules .= "include \$PREPROC_RULE_PATH/decoder.rules\n";

	/* build snort configuration file */
	$snort_conf_text = <<<EOD

# snort configuration file
# generated by the pfSense
# package manager system
# see /usr/local/pkg/snort.inc
# for more information
#	snort.conf
#   Snort can be found at http://www.snort.org/

#########################
                        #
# Define Local Network  #
                        #
#########################

var HOME_NET {$home_net}
var EXTERNAL_NET {$external_net}

###################
                  #
# Define Servers  #
                  #
###################

var DNS_SERVERS [{$def_dns_servers_type}]
var SMTP_SERVERS [{$def_smtp_servers_type}]
var HTTP_SERVERS [{$def_http_servers_type}]
var SQL_SERVERS [{$def_sql_servers_type}]
var TELNET_SERVERS [{$def_telnet_servers_type}]
var SNMP_SERVERS [{$def_snmp_servers_type}]
var FTP_SERVERS [{$def_ftp_servers_type}]
var SSH_SERVERS [{$def_ssh_servers_type}]
var POP_SERVERS [{$def_pop_servers_type}]
var IMAP_SERVERS [{$def_imap_servers_type}]
var RPC_SERVERS \$HOME_NET
var WWW_SERVERS [{$def_www_servers_type}]
var SIP_PROXY_IP [{$def_sip_proxy_ip_type}]
var SIP_SERVERS [{$def_sip_servers_type}]
var AIM_SERVERS \
[64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24]

########################
                       #
# Define Server Ports  #
                       #
########################

portvar HTTP_PORTS [{$def_http_ports_type}]
portvar FILE_DATA_PORTS [$HTTP_PORTS,110,143] 
portvar SHELLCODE_PORTS !80
portvar ORACLE_PORTS [{$def_oracle_ports_type}]
portvar AUTH_PORTS [{$def_auth_ports_type}]
portvar DNS_PORTS [{$def_dns_ports_type}]
portvar FINGER_PORTS [{$def_finger_ports_type}]
portvar FTP_PORTS [{$def_ftp_ports_type}]
portvar IMAP_PORTS [{$def_imap_ports_type}]
portvar IRC_PORTS [{$def_irc_ports_type}]
portvar MSSQL_PORTS [{$def_mssql_ports_type}]
portvar NNTP_PORTS [{$def_nntp_ports_type}]
portvar POP2_PORTS [{$def_pop2_ports_type}]
portvar POP3_PORTS [{$def_pop3_ports_type}]
portvar SUNRPC_PORTS [111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779]
portvar RLOGIN_PORTS [{$def_rlogin_ports_type}]
portvar RSH_PORTS [{$def_rsh_ports_type}]
portvar SMB_PORTS [139,445]
portvar SMTP_PORTS [{$def_smtp_ports_type}]
portvar SNMP_PORTS [{$def_snmp_ports_type}]
portvar SSH_PORTS [{$def_ssh_ports_type}]
portvar TELNET_PORTS [{$def_telnet_ports_type}]
portvar MAIL_PORTS [{$def_mail_ports_type}]
portvar SSL_PORTS [{$def_ssl_ports_type}]
portvar SIP_PROXY_PORTS [{$def_sip_proxy_ports_type}]
portvar SIP_PORTS [{$def_sip_ports_type}]

# DCERPC NCACN-IP-TCP
portvar DCERPC_NCACN_IP_TCP [139,445]
portvar DCERPC_NCADG_IP_UDP [138,1024:]
portvar DCERPC_NCACN_IP_LONG [135,139,445,593,1024:]
portvar DCERPC_NCACN_UDP_LONG [135,1024:]
portvar DCERPC_NCACN_UDP_SHORT [135,593,1024:]
portvar DCERPC_NCACN_TCP [2103,2105,2107]
portvar DCERPC_BRIGHTSTORE [6503,6504]

#####################
                    #
# Define Rule Paths #
                    #
#####################

var RULE_PATH /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules
var PREPROC_RULE_PATH /usr/local/etc/snort/preproc_rules

################################
                               #
# Configure the snort decoder  #
                               #
################################

config checksum_mode: all
config disable_decode_alerts
config disable_tcpopt_experimental_alerts
config disable_tcpopt_obsolete_alerts
config disable_ttcp_alerts
config disable_tcpopt_alerts
config disable_ipopt_alerts
config disable_decode_drops

###################################
                                  #
# Configure the detection engine  #
# Use lower memory models         #
                                  #
###################################

config detection: search-method {$snort_performance} max_queue_events 5
config event_queue: max_queue 8 log 3 order_events content_length

#Configure dynamic loaded libraries
dynamicpreprocessor directory /usr/local/lib/snort/dynamicpreprocessor
dynamicengine directory /usr/local/lib/snort/dynamicengine
dynamicdetection directory /usr/local/lib/snort/dynamicrules

###################
                  #
# Flow and stream #
                  #
###################

preprocessor frag3_global: max_frags 8192
preprocessor frag3_engine: policy bsd detect_anomalies

preprocessor stream5_global: track_tcp yes, track_udp yes, track_icmp yes

preprocessor stream5_tcp: policy BSD, ports both all, {$def_max_queued_bytes_type}{$def_max_queued_segs_type}
preprocessor stream5_udp:
preprocessor stream5_icmp:

	{$def_perform_stat_type}

	{$def_http_inspect_type}

	{$def_other_preprocs_type}

	{$def_ftp_preprocessor_type}

	{$def_smtp_preprocessor_type}

	{$def_sf_portscan_type}

	{$def_dce_rpc_2_type}

	{$def_dns_preprocessor_type}

##############################
                             #
# NEW                        #
# Ignore SSL and Encryption  #
                             #
##############################

preprocessor ssl: ports { {$def_ssl_ports_ignore_type} }, trustservers, noinspect_encrypted

#####################
                    #
# Snort Output Logs #
                    #
#####################

	$snortunifiedlogbasic_type
	$snortalertlogtype_type
	$alertsystemlog_type
	$tcpdumplog_type
	$snortmysqllog_info_chk
	$snortunifiedlog_type
	$spoink_type
						
#################
                #
# Misc Includes #
                #
#################

include /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/reference.config
include /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/classification.config
{$snort_preprocessor_decoder_rules}

$threshold_file_name

# Snort user pass through configuration
{$snort_config_pass_thru}

###################
                  #
# Rules Selection #
                  #
###################

	{$selected_rules_sections}

EOD;

	return $snort_conf_text;
}

/* hide progress bar */
function hide_progress_bar_status() {
	global $snort_filename, $snort_filename_md5, $console_mode;

	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
}

/* unhide progress bar */
function unhide_progress_bar_status() {
	global $snort_filename, $snort_filename_md5, $console_mode;

	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='visible';\n</script>";
}

/* update both top and bottom text box during an operation */
function update_all_status($status) {
	global $snort_filename, $snort_filename_md5, $console_mode;

	ob_flush();
	if(!$console_mode) {
		update_status($status);
		update_output_window($status);
	}
}

?>
