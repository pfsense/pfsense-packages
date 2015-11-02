<?php
/*
	tinydns.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2006, 2007, 2008, 2009 Scott Ullrich
	Copyright (C) 2007 Goffredo Andreone
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
if(!function_exists("filter_configure")) 
	require_once("filter.inc");
	
if(!function_exists("get_real_wan_interface")) 
	require_once("interfaces.inc");
	
function tinydns_validate() {
	global $input_errors, $config;

	if ($_POST['enableforwarding']) {
		if (isset($config['dnsmasq']['enable']))
			$input_errors[] = "You have system dns-forwarder active. Disable it before enabling DNS-Cache Forwarders.";
	}
}

function tinydns_custom_php_install_command() {
	global $g, $config;
	conf_mount_rw();
	$fd = fopen("/usr/local/etc/rc.d/svscan.sh", "w");
	if(!$fd) {
		log_error("Could not open /usr/local/etc/rc.d/svscan.sh for writing.");
		return;
	}

	// Ensure svscan.sh has a+rx
	exec("chmod a+rx /usr/local/etc/rc.d/svscan.sh");

	$ipaddress = $config['installedpackages']['tinydns']['config'][0]['ipaddress'];

	$enableipmonitoring = $config['installedpackages']['tinydns']['config'][0]['enableipmonitoring'];
	if($config['installedpackages']['tinydns']['config'][0]['refreshinterval'])
		$refreshinterval = $config['installedpackages']['tinydns']['config'][0]['refreshinterval'];

	$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
	if ($pf_version == "2.2") {
		$svscan_path = "/usr/pbi/djbdns-" . php_uname("m") . "/local/bin/svscan";
		$svc_path = "/usr/pbi/djbdns-" . php_uname("m") . "/local/bin/svc";
		$tinydns_path = "/usr/pbi/djbdns-" . php_uname("m") . "/local/bin";
	} else if ($pf_version == "2.1") {
		$svscan_path = "/usr/pbi/djbdns-" . php_uname("m") . "/bin/svscan";
		$svc_path = "/usr/pbi/djbdns-" . php_uname("m") . "/bin/svc";
		$tinydns_path = "/usr/pbi/djbdns-" . php_uname("m") . "/bin";
	} else {
		$svscan_path = "/usr/local/bin/svscan";
		$svc_path = "/usr/local/bin/svc";
		$tinydns_path = "/usr/local/bin";
	}

	$svscan = <<<EOD
#!/bin/sh

# PROVIDE: svscan
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

. /etc/rc.subr

name="svscan"
rcvar="\${name}_enable"
command="{$svscan_path}"
svscan_enable=\${svscan_enable-"YES"}
svscan_servicedir=\${svscan_servicedir-"{$g['varrun_path']}/service"}
logdir="/var/log/svscan"

start_cmd="svscan_start"
stop_postcmd="svscan_stop_post"

load_rc_config \$name

required_dirs="\${svscan_servicedir}"

svscan_start () {
        echo "Starting svscan."
        mkdir -p \$logdir
        /usr/bin/env \
        PATH={$tinydns_path}:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
        /usr/sbin/daemon -f /bin/sh -c "\$command \$svscan_servicedir 2>&1 | /usr/local/bin/multilog t \$logdir" > /dev/null

EOD;
if ($enableipmonitoring) {
	$svscan .= <<<EOD
       /usr/local/bin/minicron {$refreshinterval} {$g['varrun_path']}/ping_hosts.pid "/usr/local/bin/ping_hosts.sh"
EOD;
}
$svscan .= <<<EOD

}

svscan_stop_post () {
        echo "Stopping svscan."
        find -L "\$svscan_servicedir" -mindepth 1 -maxdepth 2 -type d \( \! -path "\$svscan_servicedir/*/*" -or -name 'log' \) -print0 | xargs -0 {$svc_path} -dx
		/bin/pkill -F /var/run/ping_hosts.pid
}

run_rc_command "\$1"

EOD;

	fwrite($fd, $svscan);
	fclose($fd);

	$filename = "tinydns.sh";
	$start = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
<?php
	require_once(\"functions.inc\");
	require_once(\"/usr/local/pkg/tinydns.inc\");
	tinydns_custom_php_changeip_command();
	tinydns_create_zone_file();\n";
	if ($enableipmonitoring) {
		$start .= 	"tinydns_setup_ping_items();\n";
	}
	$start .= "?>
ENDPHP\n";

	$stop = "";
	$interfaces = explode(",", $config['installedpackages']['tinydns']['config'][0]['interface']);
	foreach ($interfaces as $dnsidx => $dnsif)
		$stop .= "\t{$svc_path} -dx {$g['varrun_path']}/service/dnscache{$dnsidx}\n";

	$stop .= <<<ENDSH
	{$svc_path} -dx {$g['varrun_path']}/service/tinydns
	{$svc_path} -dx {$g['varrun_path']}/service/axfrdns
	/bin/rm -rf {$g['varrun_path']}/service/tinydns 2>/dev/null
	/bin/rm -rf {$g['varetc_path']}/tinydns 2>/dev/null
	/bin/rm -rf {$g['varrun_path']}/dnscache* 2>/dev/null
	/bin/rm -rf {$g['varetc_path']}/dnscache* 2>/dev/null
	/bin/rm -rf {$g['varetc_path']}/axfrdns 2>/dev/null
	/bin/rm -rf {$g['varrun_path']}/axfrdns 2>/dev/null

ENDSH;

	write_rcfile(array(
		"file" => $filename,
		"start" => $start,
		"stop" => $stop
		)
	);

	conf_mount_ro();
	filter_configure();
}

function tinydns_custom_php_deinstall_command() {
	global $g, $config;

	conf_mount_rw();

	$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
	if ($pf_version == "2.2") {
		$svc_path = "/usr/pbi/djbdns-" . php_uname("m") . "/local/bin/svc";
	} else if ($pf_version == "2.1") {
		$svc_path = "/usr/pbi/djbdns-" . php_uname("m") . "/bin/svc";
	} else {
		$svc_path = "/usr/local/bin/svc";
	}	

	/* destroy all daemontools items */
	exec("{$svc_path} -xk {$g['varetc_path']}/tinydns*");
	exec("{$svc_path} -xk {$g['varetc_path']}/dnscache*");
	exec("/usr/local/etc/rc.d/tinydns.sh stop");
	exec("/usr/local/etc/rc.d/svscan.sh stop");
	exec("/usr/bin/killall supervise");
	exec("/usr/sbin/pw groupdel Gtinydns");
	exec("/usr/sbin/pw groupdel Gdnscache");
	exec("/usr/sbin/pw groupdel Gdnslog");
	exec("/usr/sbin/pw userdel Gtinydns");
	exec("/usr/sbin/pw userdel Gdnscache");
	exec("/usr/sbin/pw userdel Gdnslog");
	exec("/usr/sbin/pw groupdel Gaxfrdns");
	exec("rm /usr/local/www/*tinydns*");
	exec("rm /usr/local/pkg/*tinydns*");
	exec("rm /usr/local/pkg/pf/*tinydns*");
	exec("rm -rf {$g['varetc_path']}/tinydns {$g['varrun_path']}/service/tinydns {$g['varetc_path']}/axfrdns");
	exec("rm -rf {$g['varetc_path']}/dnscache* {$g['varrun_path']}/service/dnscache*");
	filter_configure();
	conf_mount_ro();
}

function tinydns_upgrade_config() {
	/* Remove obsolete XMLRPC sync configuration */
	if (is_array($config['installedpackages']['tinydnssync']['config'])) {
		if (isset($config['installedpackages']['tinydnssync']['config'][0]['ipaddress'])) {
			unset($config['installedpackages']['tinydnssync']['config'][0]['ipaddress']);
		}
		if (isset($config['installedpackages']['tinydnssync']['config'][0]['ipaddress2'])) {
			unset($config['installedpackages']['tinydnssync']['config'][0]['ipaddress2']);
		}
		if (isset($config['installedpackages']['tinydnssync']['config'][0]['ipaddress3'])) {
			unset($config['installedpackages']['tinydnssync']['config'][0]['ipaddress3']);
		}
		if (isset($config['installedpackages']['tinydnssync']['config'][0]['password'])) {
			unset($config['installedpackages']['tinydnssync']['config'][0]['password']);
		}
		if (isset($config['installedpackages']['tinydnssync']['config'][0]['password2'])) {
			unset($config['installedpackages']['tinydnssync']['config'][0]['password2']);
		}
		if (isset($config['installedpackages']['tinydnssync']['config'][0]['password3'])) {
			unset($config['installedpackages']['tinydnssync']['config'][0]['password3']);
		}
	}
	if (is_array($config['installedpackages']['tinydns']['config'])) {
		if (isset($config['installedpackages']['tinydns']['config'][0]['syncxmlrpc'])) {
			unset($config['installedpackages']['tinydns']['config'][0]['syncxmlrpc']);
		}
	}
}

function tinydns_custom_php_changeip_command() {
	global $g, $config;
	conf_mount_rw();

	/* For now force $dnsserverip to to 127.0.0.1 unless a separate IP is specified */
	$localhost = "127.0.0.1";
	$dnsserverip = $localhost;
	if($config['installedpackages']['tinydns']['config'][0]['ipaddress'] != $localhost AND $config['installedpackages']['tinydns']['config'][0]['ipaddress'] != "")
		$dnsserverip = $config['installedpackages']['tinydns']['config'][0]['ipaddress'];
	if($config['installedpackages']['tinydns']['config'][0]['regdhcpstatic'] OR $config['installedpackages']['tinydns']['config'][0]['regdhcp']) 
		$dnsserverip = $localhost;
	if (!is_ipaddr($dnsserverip))
		$dnsserverip = $localhost;
	$config['installedpackages']['tinydns']['config'][0]['ipaddress'] = $dnsserverip;

	$updatecron = $config['installedpackages']['tinydns']['config'][0]['updatecron'];

	/* Populate Zone Transfer array */
	$ztipaddress = populate_zt_array();

	/* setup daemon tools service area */
	if(!is_dir("{$g['varrun_path']}/service"))	
		exec("/bin/mkdir -p {$g['varrun_path']}/service");

	if (!exec("/usr/sbin/pw usershow Gtinydns")) {
		exec("/usr/sbin/pw useradd Gtinydns");
	}
	if (!exec("/usr/sbin/pw usershow Gdnslog")) {
		exec("/usr/sbin/pw useradd Gdnslog");
	}
	if (!exec("/usr/sbin/pw usershow Gdnscache")) {
		exec("/usr/sbin/pw useradd Gdnscache");
	}
	if (!exec("/usr/sbin/pw usershow Gaxfrdns")) {
		exec("/usr/sbin/pw useradd Gaxfrdns");
	}
	exec("/bin/rm -r {$g['varetc_path']}/dnscache*");
	exec("/bin/rm -r {$g['varrun_path']}/service/dnscache*");

	/* TinyDNS Server */
	exec("/usr/local/bin/tinydns-conf Gtinydns Gdnslog {$g['varetc_path']}/tinydns {$dnsserverip}");
	exec("/bin/ln -s {$g['varetc_path']}/tinydns {$g['varrun_path']}/service/");

	/* AXFRDNS - Zone transfers */
	if(is_array($ztipaddress)) {
		exec("/usr/local/bin/axfrdns-conf Gaxfrdns Gdnslog {$g['varetc_path']}/axfrdns {$g['varetc_path']}/tinydns {$dnsserverip}");
		exec("/bin/ln -s {$g['varetc_path']}/axfrdns {$g['varrun_path']}/service/");
	}

	exec("echo {$dnsserverip} > {$g['varetc_path']}/tinydns/env/IP");
	exec("{$svc_path} -xk {$g['varrun_path']}/service/tinydns");
		

	if(!empty($config['installedpackages']['tinydns']['config'][0]['enableforwarding'])) {
		$interfaces = explode(",", $config['installedpackages']['tinydns']['config'][0]['interface']);
		exec("rm -rf {$g['varetc_path']}/dnscache* {$g['varrun_path']}/service/dnscache*");
		foreach ($interfaces as $dnsidx => $dnsif) {
			$dnscacheif = convert_friendly_interface_to_real_interface_name($dnsif);
			$dnscacheip = find_interface_ip($dnscacheif);
			$ipmask = find_interface_subnet($dnscacheif);

			if (is_ipaddrv4($ipaddress)) {
				$arr = tinydns_get_ip_subnet_arpa($ipaddress, $ipmask);
			} elseif (is_ipaddrv6($ipaddress)) {
				$arr = tinydns_get_ip6_subnet_arpa($ipaddress, $ipmask);
			}

			$dnsuserip = $arr[0];
			//exec("/bin/mkdir -p {$g['varetc_path']}/dnscache{$dnsidx}");
			exec("/usr/local/bin/dnscache-conf Gdnscache Gdnslog {$g['varetc_path']}/dnscache{$dnsidx} {$dnscacheip}");

			if (!is_dir("{$g['varetc_path']}/dnscache{$dnsidx}/env"))
				exec("/bin/mkdir -p {$g['varetc_path']}/dnscache{$dnsidx}/env");
			exec("echo {$dnscacheip} > {$g['varetc_path']}/dnscache{$dnsidx}/env/IP");
			tinydns_dnscache_forwarding_servers($dnsidx);
			if (!is_dir("{$g['varetc_path']}/dnscache{$dnsidx}/root/ip"))
				exec("/bin/mkdir -p {$g['varetc_path']}/dnscache{$dnsidx}/root/ip");

			/* tell dnscache to answer queries */
			if ( $config['installedpackages']['tinydns']['config'][0]['dnscache_listen'] != "" ) {
				$dnscache_listen_ips = explode (" ", $config['installedpackages']['tinydns']['config'][0]['dnscache_listen']);

				foreach ($dnscache_listen_ips as $dnscache_listen_ip) {
					exec("/usr/bin/touch {$g['varetc_path']}/dnscache{$dnsidx}/root/ip/{$dnscache_listen_ip}");
				}

			} else {
				exec("/usr/bin/touch {$g['varetc_path']}/dnscache{$dnsidx}/root/ip/{$dnscacheip}");
			}

			tinydns_create_soa_domain_list($dnsserverip, $dnsidx);

			/* setup symlink to start dnscache */
			exec("/bin/ln -s {$g['varetc_path']}/dnscache{$dnsidx} {$g['varrun_path']}/service/");
			
			exec("{$svc_path} -xk {$g['varetc_path']}/dnscache{$dnsidx}");
		}
	} else {
		$interfaces = explode(",", $config['installedpackages']['tinydns']['config'][0]['interface']);
		foreach ($interfaces as $dnsidx => $dnsif) {
			if(is_dir("{$g['varetc_path']}/dnscache{$dnsidx}"))
				dnscache_use_root_servers($dnsidx);
		}
		exec("{$svc_path} -xk {$g['varetc_path']}/dnscache*");
	}

	/* a wee bit of magic - something seems to start up a broken dnscache, the below kills it, and svc restarts it correctly. */ 
	exec ("ps aux | grep multilog |  awk '{print $2}' | xargs -n1 kill -9");
	exec ("ps aux | grep dnscache |  awk '{print $2}' | xargs -n1 kill -9");
	exec ("ps aux | grep tinydns |  awk '{print $2}' | xargs -n1 kill -9");

	conf_mount_ro();
	filter_configure();
}

function populate_zt_array() {
	global $g, $config;	
	/* Populate Zone Transfer array */
	if($config['installedpackages']['tinydns']['config'][0]['row']) {
		$ztipaddress = array();
		foreach($config['installedpackages']['tinydns']['config'][0]['row'] as $zt) {
			$tmp = array();
			$tmp['ztipaddress'] = $zt['ztipaddress'];
			$tmp['dnszone'] = $zt['dnszone'];
			$ztipaddress[] = $tmp;
		}
	}
	return $ztipaddress;
}

function tinydns_setup_axfrdns() {
	global $g, $config;

	/* Populate Zone Transfer array */
	$ztipaddress = populate_zt_array();
	if(!is_array($ztipaddress)) 
		return;
	$fd = fopen("{$g['varetc_path']}/axfrdns/tcp","w");
	if(!$fd) {
		log_error("Could not open {$g['varetc_path']}/axfrdns/tcp for writing");
		return;
	}
	foreach($ztipaddress as $zt) {
		if($zt['ztipaddress'] && $zt['dnszone'])
			$zonet = "{$zt['ztipaddress']}:allow";
			if($zt['dnszone'] <> "*")
				$zonet .= ",AXFR=\"{$zt['dnszone']}\"";
			fwrite($fd, $zone . "\n");
	}
	fclose($fd);
	// Recompile database
	exec("cd {$g['varrun_path']}/service/axfrdns && /usr/local/bin/tinydns-data");
}

function tinydns_get_record_status($record, $pingthreshold = "", $wanpingthreshold = "") {
	global $g, $config;
	if(file_exists("/var/db/pingstatus/{$record}")) {
		$status = "";
		$status = file_get_contents("/var/db/pingstatus/{$record}");
		if(stristr($status,"DOWN"))
			return "DOWN";
	}
	if($pingthreshold) {
		$current_ms = "";
		if(file_exists("var/db/pingmsstatus/$record"))
			$current_ms = file_get_contents("/var/db/pingmsstatus/$record");
		if($pingthreshold > $current_ms)
			return "DOWN";
	}
	if($wanpingthreshold) {
		$current_avg = "";
		if(file_exists("/var/db/wanaverage"))
			$current_avg = file_get_contents("/var/db/wanaverage");
		if($wanpingthreshold > $current_avg)
			return "DOWN";
	}
	return "UP";
}

function tinydns_get_backup_record($record, $recordtype) {
	global $g, $config;
	if($config['installedpackages']['tinydnsdomains']) {
		foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
			if($domain['hostname'] == $record && $domain['recordtype'] == $recordtype) {
				/* if no failover host exists, simply return original ipaddress */
				if(!$domain['row'])
					return $domain['ipaddress'];
				foreach($domain['row'] as $row) {
					$status = tinydns_get_record_status($row['monitorip']);
					if($status == "UP")
						return $row['failoverip'];
				}
			}
		}
	}
	return $record;
}

function tinydns_setup_ping_items() {
	global $g, $config;

	if(!$config['installedpackages']['tinydnsdomains'])
		return;
	$wanif = get_real_wan_interface();
	$ip = find_interface_ip($wanif);

	$processed = array();
	/* XXX: make this work with other packages */
	$fd = fopen("{$g['vardb_path']}/pkgpinghosts", "w");
	if(!$fd) {
		log_error("Could not open {$g['vardb_path']}/pkgpinghosts for writing.");
		return;
	}

	/*   write out each ip address so ping_hosts.sh can begin monitoring ip
	 *   status and create a database of the status information that we can use.
	 */
	foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
		if(!$domain['monitorip'])
			continue;
		if($domain['recordtype'] == "AAAA")
			$inet = "inet6";
		else
			$inet = "";
		$monitorip = $domain['monitorip'];
		$interface = $domain['interface'];
		if($domain['row']) {
			foreach($domain['row'] as $row) {
				if($row['pingthreshold'])
					$pingthreshold = $row['pingthreshold'];
				else
					$pingthreshold = "";
				
				if($row['wanpingthreshold'])
					$wanpingthreshold = $row['wanpingthreshold'];
				else
					$wanpingthreshold = "";
				
				$info = get_interface_info($row['interface']);
				if($domain['recordtype'] == "AAAA")					
					$ip = $info['ipaddrv6'];
				else
					$ip = $info['ipaddr'];
				
				if($row['monitorip']) {
					if(!in_array($row['monitorip'], $processed)) {
						fwrite($fd, $ip . "|" . $row['monitorip'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}|{$wanpingthreshold}|{$inet}\n");
						$processed[] = $row['monitorip'];
					}
				} else {
					if(!in_array($monitorip, $processed)) {
						fwrite($fd, $ip . "|" . $monitorip . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}|{$wanpingthreshold}|{$inet}\n");
						$processed[] = $monitorip;
					}
				}
			}
		}
		
		$info = get_interface_info($domain['interface']);
		if($domain['recordtype'] == "AAAA") 			
			$ip = $info['ipaddrv6'];
		else
			$ip = $info['ipaddr'];
			
		if($domain['monitorip']) {
			if(!in_array($domain['monitorip'], $processed)) {
				fwrite($fd, $ip . "|" . $domain['monitorip'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}|{$wanpingthreshold}|{$inet}\n");
				$processed[] = $domain['monitorip'];
			}
		} else {
			if(!in_array($row['failoverip'], $processed)) {
				fwrite($fd, $ip . "|" . $row['failoverip'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}|{$wanpingthreshold}|{$inet}\n");
				$processed[] = $row['failoverip'];
			}
		}
	}
	fclose($fd);
}

function tinydns_create_zone_file() {
	global $g, $config;
	conf_mount_rw();
	if(file_exists("/tmp/config.cache"))
		unlink("/tmp/config.cache");
	if(file_exists("{$g['varrun_path']}/service/tinydns/root/data"))
		exec("rm -f {$g['varrun_path']}/service/tinydns/root/data");
	if(!is_dir("{$g['varrun_path']}/service/tinydns/root"))
		return;
	$fd = fopen("{$g['varrun_path']}/service/tinydns/root/data", "w");
	if(!$fd) {
		log_error("Could not open {$g['varrun_path']}/service/tinydns/root/data for writing.");
		return;
	}
	
	/* For now do not allow registration of 'local' DNS data if tinyDNS not bound to 127.0.0.1 */
	if($config['installedpackages']['tinydns']['config'][0]['ipaddress'] == "127.0.0.1") {
		/* Load the root servers if Forwarding is enabled  */
		/* Register LAN IP and SOA Forward and Reverse DNS recors in TinyDNS Server*/
		if(!empty($config['installedpackages']['tinydns']['config'][0]['enableforwarding'])) {
			$forwardingservers = tinydns_register_root_servers();
			if($forwardingservers) 
				fwrite($fd, $forwardingservers);
			if($config['system']['hostname']['domain']) {
				$dhcpdhostname = $config['system']['hostname'];		
				if($config['system']['domain'])
					$dhcpddomain = $config['system']['domain'];
				$interfaces = explode(",", $config['installedpackages']['tinydns']['config'][0]['interface']);
                		foreach ($interfaces as $dnsif) {
					$dnsrif = convert_friendly_interface_to_real_interface_name($dnsif);
                        		$dnsrip = find_interface_ip($dnsrif);
                               		$ipmask = find_interface_subnet($dnsrif);
					$dhcpdfqdn = "{$dhcpdhostname}.{$dhcpddomain}";
					tinydns_complete_soa_record($fd, $dnsrip, $ipmask, $dhcpdhostname, $dhcpddomain);
				}
			}
		}
	
		/* Register Static IPs */
        if($config['installedpackages']['tinydns']['config'][0]['regdhcpstatic']) {
		$interfaces = explode(",", $config['installedpackages']['tinydns']['config'][0]['interface']);
		foreach ($interfaces as $dnsif) {
			if (!is_array($config['dhcpd'][$dnsif]))
				continue;
			$zone =& $config['dhcpd'][$dnsif];
			if (!isset($zone['enable']))
				continue;
                        $dhcpdhostname = $config['system']['hostname'];
                        if ($zone['ddnsdomain']) 
                                $dhcpddomain = $zone['ddnsdomain'];
                        else
                                $dhcpddomain = $config['system']['domain'];
			
			$dnsrif = convert_friendly_interface_to_real_interface_name($dnsif);
                        $dnsrip = find_interface_ip($dnsrif);
                        $ipmask = find_interface_subnet($dnsrif);
                        $dhcpdfqdn = "{$dhcpdhostname}.{$dhcpddomain}";
                        tinydns_complete_soa_record($fd, $dnsrip, $ipmask, $dhcpdhostname, $dhcpddomain);
       
                        if(is_array($zone['staticmap'])) {
                                foreach($zone['staticmap'] as $dhcpdstatic) {
                                        $dhcpdhostname = $dhcpdstatic['hostname'];
                                        $dhcpdfqdn = "{$dhcpdhostname}.{$dhcpddomain}";
                                        $dhcpdlanip = $dhcpdstatic['ipaddr'];
                                        $dhcpda = "={$dhcpdfqdn}:{$dhcpdlanip}";
                                        if($dhcpdhostname)
                                                fwrite($fd, $dhcpda . "\n");
                                }
                        }
                }
        }
	
		/* Register Dynamic IPs */
		if($config['installedpackages']['tinydns']['config'][0]['regdhcp']) {
			$leasesfile = "{$g['dhcpd_chroot_path']}/var/db/dhcpd.leases";
			$fl = fopen($leasesfile, "r");
			if(!$fl) {
				log_error("Could not open {$leasesfile} for reading.");
				return;
			}
			tinydns_add_active_leases($fl,$fd, $dhcpddomain);
            fclose($fl);
		}
	}
		
	if($config['installedpackages']['tinydnsdomains']) {
		for($x=0; $x< count($config['installedpackages']['tinydnsdomains']['config']); $x++) {
			$domain = $config['installedpackages']['tinydnsdomains']['config'][$x];
			$record_data = "";
			$monitorip = "";
			$hostname = $domain['hostname'];
			$recordtype = $domain['recordtype'];
			$ipaddress = $domain['ipaddress'];
			$ttl = $domain['ttl'];
			$dist = $domain['dist'];
			/*   check record status, if it is down request
			 *   backup server if defined.
			 */
			if($domain['monitorip'])
				$monitorip = $domain['monitorip'];
			if($monitorip) {
				$status = tinydns_get_record_status($monitorip);
				if($status == "DOWN") {
					if($debug)
						log_error("$ipaddress monitor ip $monitorip is offline.");
					$ipaddress = tinydns_get_backup_record($hostname, $recordtype);
					if($debug)
						log_error("tinydns_get_backup_record for $hostname returned $ipaddress ");
				}
			}
			$record_data = tinydns_get_rowline_data($ipaddress, $domain['recordtype'], $ttl, $hostname, $domain['rdns'], $dist, $domain['src_port'], $domain['src_weight'], $domain['src_priority'], $domain['src_timestamp']);
			if($record_data) {
				fwrite($fd, $record_data . "\n");
				if ($domain['rdns'] || ($domain['recordtype'] == 'PTR')) {
					if (is_ipaddrv4($ipaddress)) {
						$rip = tinydns_get_ip_subnet_arpa($ipaddress, 32);
					} elseif (is_ipaddrv6($ipaddress)) {
						$rip = tinydns_get_ip6_subnet_arpa($ipaddress, 128);
					}

					if($rip)
						fwrite($fd, ".{$rip[1]}::{$config['system']['hostname']}.{$config['system']['domain']}\n");
				}
			}
			/* process load balanced items */
			if($domain['row']) {
				foreach($domain['row'] as $row) {
					if($row['loadbalance']) {
						if($row['pingthreshold'])
							$pingthreshold = $row['pingthreshold'];
						else
							$pingthreshold = "";
						if($row['wanpingthreshold'])
							$wanpingthreshold =	$row['wanpingthreshold'];
						else
							$wanpingthreshold = "";
						$status = tinydns_get_record_status($row['failoverip'], $pingthreshold, $wanpingthreshold);
						if($status == "UP") {
							$record_data = tinydns_get_rowline_data($row['failoverip'], $domain['recordtype'], $ttl, $hostname, "", $domain['rdns'], $dist, $domain['src_port'], $domain['src_weight'], $domain['src_priority'], $domain['src_timestamp']);
							fwrite($fd, $record_data . "\n");
						}
					}
				}
			}
		}
	}
	fclose($fd);
	/* tell tinydns to reload zone file */
	exec("cd {$g['varrun_path']}/service/tinydns/root && /usr/local/bin/tinydns-data");
	conf_mount_ro();
}

function tinydns_sync_on_changes() {
	global $g, $config;

	/* XMLRPC sync */
	if (is_array($config['installedpackages']['tinydnssync']['config'])) {
		$tinydns_sync = $config['installedpackages']['tinydnssync']['config'][0];
		$synconchanges = $tinydns_sync['synconchanges'];
		$synctimeout = $tinydns_sync['synctimeout'] ?: '250';
		switch ($synconchanges) {
			case "manual":
				if (is_array($tinydns_sync['row'])) {
					$rs = $tinydns_sync['row'];
				} else {
					log_error("[tinydns] XMLRPC sync is enabled but there are no hosts configured as replication targets.");
					return;
				}
				break;
			case "auto":
				if (is_array($config['hasync'])) {
					$system_carp = $config['hasync'];
					$rs[0]['ipaddress'] = $system_carp['synchronizetoip'];
					$rs[0]['username'] = $system_carp['username'];
					$rs[0]['password'] = $system_carp['password'];
					$rs[0]['syncdestinenable'] = FALSE;

					// XMLRPC sync is currently only supported over connections using the same protocol and port as this system
					if ($config['system']['webgui']['protocol'] == "http") {
						$rs[0]['syncprotocol'] = "http";
						$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '80';
					} else {
						$rs[0]['syncprotocol'] = "https";
						$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '443';
					}
					if ($system_carp['synchronizetoip'] == "") {
						log_error("[tinydns] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
						return;
					} else {
						$rs[0]['syncdestinenable'] = TRUE;
					}
				} else {
					log_error("[tinydns] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
					return;
				}
				break;
			default:
				return;
				break;
		}
		if (is_array($rs)) {
			log_error("[tinydns] XMLRPC sync is starting.");
			foreach ($rs as $sh) {
				// Only sync enabled replication targets
				if ($sh['syncdestinenable']) {
					$sync_to_ip = $sh['ipaddress'];
					$port = $sh['syncport'];
					$username = $sh['username'] ?: 'admin';
					$password = $sh['password'];
					$protocol = $sh['syncprotocol'];

					$error = '';
					$valid = TRUE;

					if ($password == "") {
						$error = "Password parameter is empty. ";
						$valid = FALSE;
					}
					if (!is_ipaddr($sync_to_ip) && !is_hostname($sync_to_ip) && !is_domain($sync_to_ip)) {
						$error .= "Misconfigured Replication Target IP Address or Hostname. ";
						$valid = FALSE;
					}
					if (!is_port($port)) {
						$error .= "Misconfigured Replication Target Port. ";
						$valid = FALSE;
					}
					if ($valid) {
						tinydns_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout);
					} else {
						log_error("[tinydns] XMLRPC sync with '{$sync_to_ip}' aborted due to the following error(s): {$error}");
					}
				}
			}
			log_error("[tinydns] tinydns_xmlrpc_sync.php is ending.");
		}
 	}
}

/* Do the actual XMLRPC sync. */
function tinydns_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout) {
	global $config, $g;

	/* Detect boot process, do nothing during boot. */
	if (function_exists("platform_booting")) {
		if (platform_booting()) {
			return;
		}
	} elseif ($g['booting']) {
		return;
	}

	if ($username == "" || $password == "" || $sync_to_ip == "" || $port == "" || $protocol == "") {
		log_error("[tinydns] A required XMLRPC sync parameter (username, password, replication target, port or protocol) is empty ... aborting pkg sync");
		return;
	}

	// Take care of IPv6 literal address
	if (is_ipaddrv6($sync_to_ip)) {
		$sync_to_ip = "[{$sync_to_ip}]";
	}

	$url = "{$protocol}://{$sync_to_ip}";

	/* XML will hold the sections to sync. */
	$xml = array();
	$xml['tinydnsdomains'] = $config['installedpackages']['tinydnsdomains'];
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($xml));

	/* Set a few variables needed for sync code */
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if ($g['debug']) {
		$cli->setDebug(1);
	}
	/* Send our XMLRPC message and timeout after defined sync timeout value */
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occured while attempting XMLRPC sync with {$url}:{$port}.";
		log_error("[tinydns] {$error}");
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[tinydns] {$error}");
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} else {
		log_error("[tinydns] XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* Tell tinydns to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('functions.inc');\n";
	$execcmd .= "require_once('/usr/local/pkg/tinydns.inc');\n";
	$execcmd .= "tinydns_custom_php_changeip_command();\n";
	$execcmd .= "tinydns_create_zone_file();\n";
	if ($config['installedpackages']['tinydns']['config'][0]['enableipmonitoring']) {
		$execcmd .= "tinydns_setup_ping_items();\n";
	}
	
	/* Assemble XMLRPC payload. */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($execcmd));

	log_error("[tinydns] XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communications error occured while attempting XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error("[tinydns] {$error}");
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[tinydns] {$error}");
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} else {
		log_error("[tinydns] XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}

/* formats data as a tinydns data row item */
function tinydns_get_rowline_data($recordip, $recordtype, $ttl, $hostname, $rdns, $dist, $srv_port = "5000", $srv_weight = "0", $srv_priority = "0", $srv_timestamp = "") {
		global $config;
		if($ttl)
			$ttl_string = ":{$ttl}";
		else
			$ttl_string = "";

		switch ($recordtype) {

			/* Note that some of these are simplistic versions of TinyDNS record handling.  Uber-users can always do "raw" entries...  */
			case "SOA":	
				//     .fqdn:ip:x:ttl:timestamp:lo
				if (is_ipaddrv6($hostname)) {
					list($rip, $rmask) = explode("/", $hostname);
					$rmask = empty($rmask) ? 128 : $rmask;
					$ip6_arpa = tinydns_get_ip6_subnet_arpa($rip, $rmask);
					$record_data = ".{$ip6_arpa[1]}::{$recordip}{$ttl_string}";
				} else {
					$record_data = ".{$hostname}::{$recordip}{$ttl_string}";
				}

				break;
			case "NS":
				//      &serious.panic.mil:1.8.248.6:a
				$record_data = "&{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "MX":
				//      @fqdn:ip:x:dist:ttl:timestamp:lo
				//               ^- This is a hostname
				if(!$dist)
					$dist = "10"; // default to 10 if no preference has been defined.  0 is ugly.

				// Check if the user entered a custom record data with a : (for ip:host), and if it is valid, use it.
				// If not, check if we were given an IP, if so, put it in the ip field and leave the x field blank.
				// Otherwise leave the IP blank and put in the hostname.
				if (strpos($recordip, ':') !== false) {
					list($rip, $rhost) = explode(':', $recordip);
					if ((empty($rip) && is_hostname($rhost))
						|| (empty($rhost) && is_ipaddr($rip))
						|| (is_ipaddr($rip) && is_hostname($rhost)))
						$record_string = $recordip;
					else
						break; // Don't add an invalid record.
				} elseif (is_ipaddr($recordip))
					$record_string = "{$recordip}:";
				else
					$record_string = ":{$recordip}";
				$record_data = "@{$hostname}:{$record_string}:{$dist}{$ttl_string}";
				break;
			case "PTR":
				/* "^" creates "PTR" record only to allow reverse DNS */
				//		^fqdn:p:ttl:timestamp:lo
				if (is_ipaddrv4($recordip)) {
					$record_data = "^{$hostname}:{$recordip}{$ttl_string}";
				} elseif (is_ipaddrv6($recordip)) {
					$ip6_arpa = tinydns_get_ip6_subnet_arpa($recordip, 128);
					$record_data = "^{$ip6_arpa[1]}:{$hostname}{$ttl_string}";
				}
				break;
			case "A":
				/* "=" creates both "A" and "PTR" records to allow both forward and reverse DNS */
				if($rdns) {
					//		=fqdn:ip:ttl:timestamp:lo
					$record_data = "={$hostname}:{$recordip}{$ttl_string}";
				} else {
					/* "+" creates "A" records only to allow forward DNS */
					//		+fqdn:ip:ttl:timestamp:lo
					$record_data = "+{$hostname}:{$recordip}{$ttl_string}";
				}
				break;
			case "AAAA":
				$recip_san = tinydns_get_ip6_format($recordip);
				$record_data = ":{$hostname}:28:{$recip_san}{$ttl_string}";
				if($rdns) {
					$ip6_arpa = tinydns_get_ip6_subnet_arpa($recordip, 128);
					// Need some way to add a second record...
					//$record_data = "\n^{$hostname}:{$ip6_arpa[1]}{$ttl_string}";
				}
				break;
			case "SRV":
					$recip_san = str_replace(":", "", $recordip);
					$record_data = "S{$hostname}:{$recip_san}{$rec_port}{$rec_weight}{$srv_priority}{$ttl_string}{$srv_timestamp}";
				break;
			case "CNAME":
				//     Cfqdn:p:ttl:timestamp:lo
				$record_data = "C{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "TXT":
				/* "'" creates "TXT" record                                                             */
				/* ":" creates a generic record entry, (and record code 16 below makes it a TXT record) */
				/* Q: Why bother with generic?                                                          */
				/* A: TinyDNS TXT records get split up every 127 chars and some clients have trouble re-assembling them.                               */
				/* TinyDNS generic records allow up to the maximum DNS record size of 255 chars but it is a hard limit, no splitting of larger strings */
				/* ...so try to always create the best record for the need                                                                             */
				/* Initial cleanup required for TXT records in TinyDNS where we substitute Octal escape codes for certain chars*/
				$saferecordip = str_replace(":", "\\072", $recordip);
				$saferecordip = str_replace(" ", "\\040", $saferecordip);
				$saferecordip = str_replace("\r", "\\015", $saferecordip);
				$saferecordip = str_replace("\n", "\\012", $saferecordip);
				/* Logically this should be comparing against 127 and 255 but PHP has a boundary error?                 */
				/* Boundary errors or not, 128 and 256 at least evaluate properly!!!                                    */
				/* Also note that reclen checks against the original string and not the "safe" one we generated above.  */
				$reclen = mb_strlen($recordip, '8bit');
				if($reclen > 128 && $reclen <= 256) {	
					/* TinyDNS generic records require an escaped Octal string length padded to three chars before the actual string!    */
					/* The logic here shouldn't ever require padding but including it anyway in case somebody changes code down the road */
					$reclen = str_pad(decoct($reclen),3,"0",STR_PAD_LEFT);
					//		:fqdn:n:rdata:ttl:timestamp:lo
					$record_data = ":{$hostname}:16:\\{$reclen}{$saferecordip}{$ttl_string}";
				} else {
					//		'fqdn:s:ttl:timestamp:lo
					$record_data = "'{$hostname}:{$saferecordip}{$ttl_string}";
				}	
				break;
			case "raw":
				/* We don't know or care what is in a raw entry, just pass it along as-is */
				$record_data = "{$recordip}";
				break;				
		}
		return $record_data;
}

/* Returns the last IP byte and the Trimmed IP*/
function tinydns_get_lastip_byte($ipsub) {
	$len= strlen($ipsub); 	
	$pos = strrpos($ipsub, ".");
	$last_byte = "";
	if ($pos === false) { 
			$last_byte = $ipsub; 
			return array ($last_byte,$ipsub); 
	} 
	$last_byte = substr($ipsub,$pos + 1);
	$ipsub = substr($ipsub,0,$pos);
	return array ($last_byte,$ipsub);
}

/* in-add.arpa IP calculated from D.C.B.A and Mask to A.B.C.D.in-addr.arpa */
/* subnet IP calculated from A.B.C.D and Mask */
function tinydns_get_ip_subnet_arpa($ip, $ipmask) {
	$ipsub = $ip;	
	$arpaip = "";	
	$array = tinydns_get_lastip_byte($ipsub);
	$a = $array[0];
	$ipsub = $array[1];
	$array = tinydns_get_lastip_byte($ipsub);
	$b = $array[0];
	$ipsub = $array[1];
	$array = tinydns_get_lastip_byte($ipsub);
	$c = $array[0];
	$ipsub = $array[1];
	$array = tinydns_get_lastip_byte($ipsub);
	$d = $array[0];
	$ipsub = $array[1];
	switch ($ipmask) {
		case ($ipmask <= 32 AND $ipmask > 24):
			$s = 32 - $ipmask;
			$a >> $s;
			$arpaip = "{$a}.{$b}.{$c}.{$d}.in-addr.arpa";
			$subnet = "{$d}.{$c}.{$b}.{$a}";
			break;
		case ($ipmask <= 24 AND $ipmask > 16):
			$s = 24 - $ipmask;
			$b >> $s;
			$arpaip = "{$b}.{$c}.{$d}.in-addr.arpa";
			$subnet = "{$d}.{$c}.{$b}";
			break;
		case ($ipmask <= 16 AND $ipmask > 8):
			$s = 16 - $ipmask;
			$c >> $s;
			$arpaip = "{$c}.{$d}.in-addr.arpa";
			$subnet = "{$d}.{$c}";
			break;
		case ($ipmask <= 8 AND $ipmask > 0):
			$s = 8 - $ipmask;
			$d >> $s;
			$arpaip = "{$d}.in-addr.arpa";
			$subnet = "{$d}";
			break;
		}
	return 	array($subnet,$arpaip);
}

/* ip6.arpa IP calculated from expanding IPv6 fully and reversing. */
function tinydns_get_ip6_subnet_arpa($ip, $ipmask=128) {
	require_once("IPv6.inc");
	$subnet = Net_IPv6::compress(Net_IPv6::getNetmask($ip, $ipmask));

	/* Uncompress the v6 IP so we have all of the sections we need */
	$fullip = explode(":", Net_IPv6::uncompress($ip));

	/* Expand even more so we have a digit in every possible place */
	foreach ($fullip as & $q) {
		$q = sprintf("%04s", $q);
	}

	/* Turn the IP into an array of digits and then trim off the bits that have been masked away. */
	$prefixlen = intval($ipmask/4);
	$fullip = array_slice(str_split(implode("", $fullip)), 0, $prefixlen);

	/* Reverse the IP and make a proper ip6.arpa of it */
	$arpaip = implode(".", array_reverse($fullip)) . ".ip6.arpa";
	return array($subnet, $arpaip);
}

function tinydns_get_ip6_format($ip) {
	require_once("IPv6.inc");
	$subnet = Net_IPv6::compress(Net_IPv6::getNetmask($ip, 128));

	/* Uncompress the v6 IP so we have all of the sections we need */
	$fullip = explode(":", Net_IPv6::uncompress($ip));

	/* Expand even more so we have a digit in every possible place */
	foreach ($fullip as & $q) {
		$q = sprintf("%04s", $q);
		$a = sprintf("\\%03lo", hexdec(substr($q, 0, 2)));
		$b = sprintf("\\%03lo", hexdec(substr($q, 2, 2)));
		$q = $a.$b;
	}

	/* Turn the IP into an array of digits and then trim off the bits that have been masked away. */
	$fullip = implode("", $fullip);

	return $fullip;
}

/* Create a Forward and a Reverse DNS (SOA, A, PTR) records for Fully Qualififed Domain Name*/
function tinydns_complete_soa_record($fd, $ip, $ipmask, $nsname, $domain) {
	global $config;
	$fqdn = "{$nsname}.{$domain}";

	if (is_ipaddrv4($ipaddress)) {
		$rip = tinydns_get_ip_subnet_arpa($ipaddress, $ipmask);
	} elseif (is_ipaddrv6($ipaddress)) {
		$rip = tinydns_get_ip6_subnet_arpa($ipaddress, $ipmask);
	}

	$soa = ".{$domain}::{$fqdn}";
	$rsoa = ".{$rip[1]}::{$fqdn}";
	$a = "={$fqdn}:{$ip}";
	if($fqdn)
		fwrite($fd, $soa . "\n");
	if($rip) 
		fwrite($fd, $rsoa . "\n");
	if($nsname) 
		fwrite($fd, $a . "\n");
}

/* Search for active leases in the dhcpd.leases file and add them to tinyDNS */
function tinydns_parse_leases($fl,
        $datekeys = array('starts', 'ends'),
        $stringkeys = array('binding state', 'client-hostname', 'hardware ethernet'))
{
    $entry = NULL;
    $result = array();

	while (!feof($fl)) {
		$line = fgets($fl, 4096);

        /* skip comments and empty lines */
        if (($line[0] == "#") OR ($line[0] == "\n"))
            continue;

        /* parse beginning of lease entry */
		if (1 == sscanf($line, "lease %[0-9.] {", $ip)) {
            $entry = array('ip' => $ip);
            continue;
		}

        /* parse end of lease entry */
        if($line[0] == "}") {
            $result[] = $entry;
            $entry = NULL;
            continue;
		}

        /* parse row with date value. dates are always in gmt! */
        foreach ($datekeys as $datekey) {
            if(7 == sscanf($line, "  ${datekey} %d %d/%d/%d %d:%d:%d;",
                $wd, $year, $month, $day, $hour, $minute, $sec))
            {
                $entry[$datekey] =
                    gmmktime($hour, $minute, $sec, $month, $day, $year);
                continue 2;
            }
        }

        /* parse standard row value */
        foreach ($stringkeys as $stringkey) {
            if(1 == sscanf($line, "  ${stringkey} %[^;];", $value)) {
                $entry[$stringkey] = trim($value, "\"");
                continue 2;
            }
        }
	}
    return $result;
}

/**
 * Filter out inactive and expired dhcpd leases
 *
 * Loop thru a sorted list of dhcp leases parsed using tinydns_parse_leases and
 * remove inactive and expired entries. Returns an unsorted subset of the
 * original leases array.
 */
function tinydns_filter_active_leases($leases, $now = NULL) {
    if ($now == NULL)
        $now = time();

    $result = array();
    foreach ($leases as $lease) {
        if ($lease['binding state'] == 'active' AND $lease['ends'] > $now)
            $result[$lease['ip']] = $lease;
        elseif ($lease['binding state'] == 'free')
            unset($result[$lease['ip']]);
    }
    return $result;
}

/**
 * Add active dhcp leases as tinydns host entries
 */
function tinydns_add_active_leases($fl, $fd, $leasedomain) {
    $leases = tinydns_parse_leases($fl);

    $leases = tinydns_filter_active_leases($leases);
    foreach ($leases as $lease) {
        if ($lease['binding state'] != 'active')
            continue;

        /* 
         * write tinydns host entry using 0 as ttl and the lease end as the 
         * timestamp in tai64 format. See tinydns-data and djbs tai64 site:
         * http://cr.yp.to/libtai/tai64.html
         */
        fwrite($fd, sprintf("=%s.%s:%s:0:4%015x\n", $lease['client-hostname'],
            $leasedomain, $lease['ip'], $lease['ends'] + 10));
    }
}

function tinydns_get_dns_record_type($tinydnsrecord) {
	$rtype = "";
	$rtype2 = "";
	$rdns = "";
	switch ($tinydnsrecord) {
		case($tinydnsrecord[0] == "."):
			$rtype = "SOA";
			$rtype2 = "NS";
			break;
		case($tinydnsrecord[0] == "="):
			$rtype = "A";
			$rtype2 = "PTR";
			$rdns = "on";
			break;
		case($tinydnsrecord[0] == "+"):
			$rtype = "A";
			break;
		case($tinydnsrecord[0] == ":"):
			$rtype = "AAAA";
			break;
		case($tinydnsrecord[0] == "@"):
			$rtype = "MX";
			break;
		case($tinydnsrecord[0] == "^"):
			$rtype = "PTR";
			$rdns = "on";
			break;
		case($tinydnsrecord[0] == "&"):
			$rtype = "NS";
			break;
		case($tinydnsrecord[0] == "'"):
			$rtype = "TXT";
			break;
		case($tinydnsrecord[0] == "C"):
			$rtype = "CNAME";
			break;
		case($tinydnsrecord[0] == "Z"):
			$rtype = "SOA";
			break;
		default:
			$rtype = "";
	}
	return array ($rtype, $rtype2, $rdns);
}

/* This function will be replaced by an auto detect DNS cache servers routine */
/* At the moment there is no tagging of DNSroute to a WAN port. It needs to be added  */
function tinydns_dnscache_forwarding_servers($index) {
	global $g, $config;

	exec("echo 1 > {$g['varetc_path']}/dnscache{$index}/env/FORWARDONLY");
	if (is_dir("{$g['varetc_path']}/dnscache{$index}/root/servers/")) {
		exec("rm -R {$g['varetc_path']}/dnscache/root/servers/");
	}
	exec("/bin/mkdir -p {$g['varetc_path']}/dnscache{$index}/root/servers/");
	if (!empty(glob("{$g['varetc_path']}/nameserver_*"))) {
		exec("/bin/cat {$g['varetc_path']}/nameserver_* > {$g['varetc_path']}/dnscache{$index}/root/servers/@");
	} else {
		$fw = fopen("{$g['varetc_path']}/dnscache{$index}/root/servers/@", "w");
		if (!$fw) {
			printf("Error: cannot open dnscache/root/servers/@ in tinydns_register_forwarding_servers().\n");
			return 1;
		}
		fwrite($fw, $config['system']['dnsserver'][0]);
		fclose($fw);
	}
}

/* This routine adds filenames to {$g['varetc_path']}/dnscache/root/servers/ with the contents pointing to the tinyDNS server */
function tinydns_create_soa_domain_list($dnsserverip, $index = 0) {
	global $g;
	if(file_exists("{$g['varrun_path']}/service/tinydns/root/data"))
		$tinydns_data = file_get_contents("{$g['varrun_path']}/service/tinydns/root/data");
	else
		$tinydns_data = "";
	$datalen = strlen($tinydns_data);
	$startofrecord = 0;
	while ($startofrecord < $datalen ) {	
		$endofrecord = strpos($tinydns_data,"\n",$startofrecord);
		$dnsrecord = substr($tinydns_data,$startofrecord,$endofrecord-$startofrecord);
		$startofrecord = $endofrecord + 1;
		
		$col1 = strpos($dnsrecord,":");
		$fqdn = substr($dnsrecord,1,$col1-1);
		if($fqdn) {
			$rtypes = tinydns_get_dns_record_type($dnsrecord);
			if($rtypes[0] == "SOA") {
				$fr = fopen("{$g['varetc_path']}/dnscache{$index}/root/servers/{$fqdn}", "w");
				if (! $fr) {
					printf("Error: cannot open {$g['varetc_path']}/dnscache{$index}/root/servers/{$fqdn} in tinydns_create_soa_domain_list().\n");
					return 1;
				}
				if($fqdn)fwrite($fr, $dnsserverip);
				fclose($fr);
			}
		}	
	}	
}

function tinydns_register_root_servers() {
	$rootservers =<<<EOD
&::a.root-servers.net
&::b.root-servers.net
&::c.root-servers.net
&::d.root-servers.net
&::e.root-servers.net
&::f.root-servers.net
&::g.root-servers.net
&::h.root-servers.net
&::i.root-servers.net
&::j.root-servers.net
&::k.root-servers.net
&::l.root-servers.net
&::m.root-servers.net
=a.root-servers.net:198.41.0.4
=b.root-servers.net:192.228.79.201
=c.root-servers.net:192.33.4.12
=d.root-servers.net:128.8.10.90
=e.root-servers.net:192.203.230.10
=f.root-servers.net:192.5.5.241
=g.root-servers.net:192.112.36.4
=h.root-servers.net:128.63.2.53
=i.root-servers.net:192.36.148.17
=j.root-servers.net:192.58.128.30
=k.root-servers.net:193.0.14.129
=l.root-servers.net:199.7.83.42
=m.root-servers.net:202.12.27.33

EOD;
	return $rootservers;
}

function dnscache_use_root_servers($index = 0) {
	global $g;

	$rootservers =<<<EOD
198.41.0.4
192.228.79.201
192.33.4.12
128.8.10.90
192.203.230.10
192.5.5.241
192.112.36.4
128.63.2.53
192.36.148.17
192.58.128.30
193.0.14.129
199.7.83.42
202.12.27.33

EOD;

	exec("echo 0 > {$g['varetc_path']}/dnscache{$index}/env/FORWARDONLY");
	if(is_dir("{$g['varetc_path']}/dnscache{$index}/root/servers/")) 
		exec("/bin/rm -R {$g['varetc_path']}/dnscache{$index}/root/servers/");
	exec("/bin/mkdir -p {$g['varetc_path']}/dnscache{$index}/root/servers/");
	$fr = fopen("{$g['varetc_path']}/dnscache{$index}/root/servers/@", "w");
	if (! $fr) {
		printf("Error: cannot write to {$g['varetc_path']}/dnscache{$index}/root/servers/@ in dnscache_use_root_servers().\n");
		return 1;
	}
	fwrite($fr, $rootservers);
	fclose($fr);
}

function tinydns_cleanup_addedit_form_record() {
	/* Clean some things up and simplify per limited subset of TinyDNS record syntax before saving.  */
	if($_POST['recordtype'] == "TXT") {
		/* TinyDNS provides surrounding quotes for TXT records automatically so we check & remove them here */
		if(substr($_POST['ipaddress'],-1) == "\"")
			$_POST['ipaddress'] = substr($_POST['ipaddress'],0,-1);		
		if(substr($_POST['ipaddress'],0,1) == "\"")
			$_POST['ipaddress'] = substr($_POST['ipaddress'],1);
		if(substr($_POST['ipaddress'],0,5) == "v=spf") {
			/* more cleanup specific to SPF records - strip newlines and carriage returns) */
			$_POST['ipaddress'] = str_replace("\r", "", $_POST['ipaddress']);
			$_POST['ipaddress'] = str_replace("\n", "", $_POST['ipaddress']);		
		}
	}
}

?>
