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

	$svscan = <<<EOD
#!/bin/sh

# PROVIDE: svscan
# REQUIRE: LOGIN
# KEYWORD: FreeBSD

MINSEGMENT=10240
MAXSEGMENT=20480
MAXFILESIZE=10240
MAXFD=100
MAXCHILD=40

# ulimits
ulimit -d \${{$minsegment}}
ulimit -f \${{$maxfilesize}}
ulimit -m \${{$maxsegment}}
ulimit -n \${{$$maxfd}}
ulimit -s \${{$minsegment}}
ulimit -u \${{$maxchild}}

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
}

svscan_stop_post () {
        echo "Stopping svscan."
        find -L "\$svscan_servicedir" -mindepth 1 -maxdepth 2 -type d \( \! -path "\$svscan_servicedir/*/*" -or -name 'log' \) -print0 | xargs -0 /usr/local/bin/svc -dx
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
	exec("/bin/ln -s /etc/tinydns /service");
	sleep(5);
	exec("/usr/local/bin/svstat /service/tinydns");
	if($updatecron) {
		$text_1   = "*/1		 *	 	*	 	*	 	*	 	root	 /usr/bin/nice -n20 /etc/ping_hosts.sh";
		$text_5   = "*/5		 *	 	*	 	*	 	*	 	root	 /usr/bin/nice -n20 /etc/ping_hosts.sh";
		$filenamea = "/etc/crontab";
		remove_text_from_file($filenamea, $text_1);
		remove_text_from_file($filenamea, $text_5);
		add_text_to_file($filenamea, $text_1);
	} else {
		$text_1   = "*/1		 *	 	*	 	*	 	*	 	root	 /usr/bin/nice -n20 /etc/ping_hosts.sh";
		$text_5   = "*/5		 *	 	*	 	*	 	*	 	root	 /usr/bin/nice -n20 /etc/ping_hosts.sh";
		$filenamea = "/etc/crontab";
		remove_text_from_file($filenamea, $text_1);
		remove_text_from_file($filenamea, $text_5);
		add_text_to_file($filenamea, $text_5);
	}
	exec("killall -HUP cron");
	conf_mount_ro();
	filter_configure();
}

function tinydns_get_record_status($record, $pingthreshold = "", $wanpingthreshold = "") {
	global $g, $config;
	if(file_exists("/var/db/pingstatus/{$record}")) {
		$status = file_get_contents("/var/db/pingstatus/{$record}");
		if(stristr($status,"DOWN"))
			return false;
	}
	if($pingthreshold) {
		$current_ms = file_get_contents("/var/db/pingmsstatus/$record");
		if($pingthreshold > $current_ms)
			return false;
	}
	if($wanpingthreshold) {
		$current_avg = file_get_contents("/var/db/wanaverage");
		if($wanpingthreshold > $current_avg)
			return false;
	}
	return true;
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
					if($status)
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
	/* XXX: make this work with other packages */
	$fd = fopen("/var/db/pkgpinghosts", "w");
	if(!$fd) {
		log_error("Could not open /var/db/pkgpinghosts for writing.");
		return;
	}
	/*   write out each ip address so ping_hosts.sh can begin monitoring ip
	 *   status and create a database of the status information that we can use.
	 */
	foreach($config['installedpackages']['tinydnsdomains']['config'] as $domain) {
		fwrite($fd, $ip . "|" . $domain['ipaddress'] . "|3|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php\n");
		if($domain['row']) {
			foreach($domain['row'] as $row) {
				if($row['pingthreshold'])
					$pingthreshold = $row['pingthreshold'];
				else
					$row['pingthreshold'] = "";
				fwrite($fd, $ip . "|" . $row['failoverip'] . "|3|/usr/local/pkg/tinydns_down.php|/usr/local/pkg/tinydns_up.php|{$pingthreshold}\n");
			}
		}
	}
	fclose($fd);
	conf_mount_ro();
}

function tinydns_create_zone_file() {
	global $g, $config;
	conf_mount_rw();
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
			$status = tinydns_get_record_status($ipaddress);
			if(!$status)
					$ipaddress = tinydns_get_backup_record($ipaddress);
			$record_data = tinydns_get_rowline_data($ipaddress, $domain['recordtype'], $ttl, $hostname);
			if($record_data)
				fwrite($fd, $record_data . "\n");
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
						if($status) {
							$record_data = tinydns_get_rowline_data($row['failoverip'], $domain['recordtype'], $ttl, $hostname);
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
	conf_mount_ro();
}

/* formats data as a tinydns data row item */
function tinydns_get_rowline_data($recordip, $recordtype, $ttl, $hostname) {
		if($ttl)
			$ttl_string = "::{$ttl}";
		else
			$ttl_string = "";
		switch ($recordtype) {
			case "SOA":
				$record_data = ".{$hostname}::{$recordip}";
				break;
			case "MX":
				$record_data = "@{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "A":
				$record_data = "+{$hostname}:{$recordip}{$ttl_string}";
				break;
			case "PTR":
				$record_data = "={$hostname}:{$recordip}{$ttl_string}";
				break;
			case "CNAME":
				$record_data = "C{$hostname}:{$recordip}{$ttl_string}";
				break;
		}
		return $record_data;
}

?>