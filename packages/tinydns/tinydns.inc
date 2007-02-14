<?php

/* $Id$ */
/*
	tinydns.inc
	Copyright (C) 2006 Scott Ullrich
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

require_once("filter.inc");

function tinydns_custom_php_install_command() {
	global $g, $config;
	conf_mount_rw();
	$fd = fopen("/usr/local/etc/rc.d/svscan.sh", "w");
	if(!$fd) {
		log_error("Could not open /usr/local/etc/rc.d/svscan.sh for writing.");
		return;
	}

	$ip = $config['installedpackages']['tinydns']['config'][0]['ipaddress'];

	$minsegment = "10240";
	$maxfilesize = "10240";
	$maxsegment  = "20480";
	$maxfd = "100";
	$maxchild = "40";

	if($config['installedpackages']['tinydns']['config'][0]['minsegment'])
		$minsegment = $config['installedpackages']['tinydns']['config'][0]['minsegment'];

	if($config['installedpackages']['tinydns']['config'][0]['maxfilesize'])
		$maxfilesize = $config['installedpackages']['tinydns']['config'][0]['maxfilesize'];

	if($config['installedpackages']['tinydns']['config'][0]['maxsegment'])
		$maxsegment = $config['installedpackages']['tinydns']['config'][0]['maxsegment'];

	if($config['installedpackages']['tinydns']['config'][0]['maxfd'])
		$maxfd = $config['installedpackages']['tinydns']['config'][0]['maxfd'];

	if($config['installedpackages']['tinydns']['config'][0]['maxchild'])
		$maxchild = $config['installedpackages']['tinydns']['config'][0]['maxchild'];

	if($config['installedpackages']['tinydns']['config'][0]['refreshinterval'])
		$refreshinterval = $config['installedpackages']['tinydns']['config'][0]['refreshinterval'];

	$svscan = <<<EOD
#!/bin/sh

# PROVIDE: svscan
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

. /etc/rc.subr

name="svscan"
rcvar=`set_rcvar`
command="/usr/local/bin/svscan"
svscan_enable=\${svscan_enable-"YES"}
svscan_servicedir=\${svscan_servicedir-"/service"}

start_cmd="svscan_start"
stop_postcmd="svscan_stop_post"

load_rc_config \$name

required_dirs="\${svscan_servicedir}"

svscan_start () {
        echo "Starting svscan."
        /usr/bin/env \
        PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
        /usr/sbin/daemon -f /bin/sh -c "\$command \$svscan_servicedir 2>&1 | /usr/local/bin/readproctitle service errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &" > /dev/null
        minicron {$refreshinterval} /var/run/ping_hosts.pid "/etc/ping_hosts.sh; cd /etc/tinydns/root && /usr/local/bin/tinydns-data"
}

svscan_stop_post () {
        echo "Stopping svscan."
        find -L "\$svscan_servicedir" -mindepth 1 -maxdepth 2 -type d \( \! -path "\$svscan_servicedir/*/*" -or -name 'log' \) -print0 | xargs -0 /usr/local/bin/svc -dx
        PIDTOKILL=`cat /var/run/ping_hosts.pid`
        kill $PIDTOKILL
}

run_rc_command "\$1"

EOD;

	fwrite($fd, $svscan);
	fclose($fd);
	conf_mount_ro();
	filter_configure();

}

function tinydns_custom_php_deinstall_command() {
	global $g, $config;
	conf_mount_rw();
	/* destroy all daemontools items */
	exec("/usr/sbin/pw groupdel Gtinydns");
	exec("/usr/sbin/pw groupdel Gdnslog");
	exec("/usr/sbin/pw userdel Gtinydns");
	exec("/usr/sbin/pw userdel Gdnslog");
	exec("rm /usr/local/www/*tinydns*");
	exec("rm /usr/local/pkg/*tinydns*");
	exec("rm /usr/local/pkg/pf/*tinydns*");
	conf_mount_ro();
	filter_configure();
}

function tinydns_custom_php_changeip_command() {
	global $g, $config;
	conf_mount_rw();
	$ip = $config['installedpackages']['tinydns']['config'][0]['ipaddress'];
	$updatecron = $config['installedpackages']['tinydns']['config'][0]['updatecron'];
	/* setup daemon tools service area */
	exec("/usr/sbin/pw useradd Gtinydns");
	exec("/usr/sbin/pw useradd Gdnslog");
	exec("/usr/local/bin/tinydns-conf Gtinydns Gdnslog /etc/tinydns {$ip}");
	exec("/bin/mkdir /service");
	exec("/bin/ln -s /etc/tinydns /service/");
	exec("echo $ip > /service/tinydns/env/IP");
	exec("killall -9 tinydns");
	conf_mount_ro();
	filter_configure();
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

function tinydns_get_backup_record($record) {
	global $g, $config;
	if($config['installedpackages']['tinydnsdomains']) {
		foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
			if($domain['ipaddress'] == $record) {
				/* if no failover host exists, simply return original record */
				if(!$domain['row'])
					return $record;
				foreach($domain['row'] as $row) {
					$status = tinydns_get_record_status($row['failoverip']);
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
	conf_mount_rw();
	$processed = array();
	/* XXX: make this work with other packages */
	$fd = fopen("/var/db/pkgpinghosts", "w");
	if(!$fd) {
		log_error("Could not open /var/db/pkgpinghosts for writing.");
		return;
	}
	config_lock();
	/*   write out each ip address so ping_hosts.sh can begin monitoring ip
	 *   status and create a database of the status information that we can use.
	 */
	foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
		if(!in_array($domain['ipaddress'], $processed)) {
			fwrite($fd, $ip . "|" . $domain['ipaddress'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php\n");
			$processed[] = $domain['ipaddress'];
		}
		if($domain['monitorip'] <> "")
			$monitorip = $domain['monitorip'];
		if($domain['row']) {
			foreach($domain['row'] as $row) {
				if($row['pingthreshold'])
					$pingthreshold = $row['pingthreshold'];
				else
					$row['pingthreshold'] = "";
				if($row['monitorip']) {
					if(!in_array($row['monitorip'], $processed)) {
						fwrite($fd, $ip . "|" . $row['monitorip'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}\n");
						$processed[] = $row['monitorip'];
					}
				} else {
					if(!in_array($monitorip, $processed)) {
						fwrite($fd, $ip . "|" . $monitorip . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}\n");
						$processed[] = $monitorip;
					}
				}
			}
		}
		if($domain['monitorip']) {
			if(!in_array($domain['monitorip'], $processed)) {
				fwrite($fd, $ip . "|" . $domain['monitorip'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}\n");
				$processed[] = $domain['monitorip'];
			}
		} else {
			if(!in_array($row['failoverip'], $processed)) {
				fwrite($fd, $ip . "|" . $row['failoverip'] . "|1|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}\n");
				$processed[] = $row['failoverip'];
			}
		}
	}
	fclose($fd);
	config_unlock();
	conf_mount_ro();
}

function tinydns_create_zone_file() {
	global $g, $config;
	conf_mount_rw();
	if(file_exists("/tmp/config.cache"))
		unlink("/tmp/config.cache");
	parse_config(true);
	config_lock();
	if(file_exists("/service/tinydns/root/data"))
		exec("rm -f /service/tinydns/root/data");
	if(!is_dir("/service/tinydns/root"))
		return;
	$fd = fopen("/service/tinydns/root/data", "w");
	if(!$fd) {
		log_error("Could not open /service/tinydns/root/data for writing.");
		return;
	}
	if($config['installedpackages']['tinydnsdomains']) {
		foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
			$record_data = "";
			$hostname = $domain['hostname'];
			$ipaddress = $domain['ipaddress'];
			$ttl = $domain['ttl'];
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
					$ipaddress = tinydns_get_backup_record($ipaddress);
					if($debug)
						log_error("tinydns_get_backup_record returned $ipaddress ");
				}
			}
			$record_data = tinydns_get_rowline_data($ipaddress, $domain['recordtype'], $ttl, $hostname, $domain['rdns']);
			if($record_data) {
				fwrite($fd, $record_data . "\n");
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
						if($status == "DOWN") {
							$record_data = tinydns_get_rowline_data($row['failoverip'], $domain['recordtype'], $ttl, $hostname, "");
							fwrite($fd, $record_data . "\n");
						}
					}
				}
			}
		}
	}
	fclose($fd);
	/* tell tinydns to reload zone file */
	exec("cd /service/tinydns/root && /usr/local/bin/tinydns-data");
	config_unlock();
	conf_mount_ro();
}

/* formats data as a tinydns data row item */
/* A full description of the data format is available at 'man tinydns-data' */
function tinydns_get_rowline_data($recordip, $recordtype, $ttl, $hostname,$rdns) {
		if($ttl)
			$ttl_string = ":{$ttl}";
		else
			$ttl_string = "";
		switch ($recordtype) {
			case "SOA":
				$record_data = ".{$hostname}::{$recordip}{$ttl_string}";
				break;
			case "NS":
				$record_data = "&{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "MX":
				$record_data = "@{$hostname}:{$recordip}::{$ttl_string}";
				break;
			case "PTR":
				/* "^" creates "PTR" record only to allow reverse DNS */
				$record_data = "^{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "A":
				/* "=" creates both "A" and "PTR" records to allow both forward and reverse DNS */
				if($rdns)
				$record_data = "={$hostname}:{$recordip}{$ttl_string}";
				else
				/* "+" creates "A" records only to allow forward DNS */
					$record_data = "+{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "CNAME":
				$record_data = "C{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "TXT":
				/* "'" creates "TXT" record that can be used for SPF */
				$record_data = "'{$hostname}:{$recordip}{$ttl_string}";
				break;
		}
		return $record_data;
}

function tinydns_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;


	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") {
			$port = "80";
		} else {
			$port = "443";
		}
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['installedpackages']['tinydns'] = $config['installedpackages']['tinydns'];
	$xml['installedpackages']['tinydnsdomains'] = $config['installedpackages']['tinydnsdomains'];

	//print_r($xml);

	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	$method = 'pfsense.merge_config_section';

	/* Sync! */
	log_error("Beginning TinyDNS XMLRPC sync to {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occured while attempting tinydns XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} elseif($resp->faultCode()) {
		$error = "An error code was received while attempting tinydns XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "tinydns Settings Sync", "");
	} else {
		log_error("tinydns XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	log_error("[tinydns] tinydns_xmlrpc_sync.php is ending.");
}

?>