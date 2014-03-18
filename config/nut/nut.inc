<?php
/*
	nut.inc
	part of pfSense (https://www.pfsense.org/)

	Copyright (C) 2007 Ryan Wagoner <rswagoner@gmail.com>.
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

	require_once("config.inc");
	require_once("functions.inc");

	/* Nut */

	define('NUT_RCFILE', '/usr/local/etc/rc.d/nut.sh');

	if (substr(trim(file_get_contents("/etc/version")),0,3) == "2.0") {
		define('NUT_DIR','/usr/local/etc/nut');
	} else {
		define('NUT_DIR', '/usr/pbi/nut-' . php_uname("m") . '/etc/nut');
	}

	function nut_notice ($msg) { syslog(LOG_NOTICE, "nut: {$msg}"); return; }
	function nut_warn ($msg) { syslog(LOG_WARNING, "nut: {$msg}"); return; }

	function nut_action ($action) {
		if (file_exists(NUT_RCFILE))
			mwexec(NUT_RCFILE.' '.$action);
	}

	function nut_config ($name) {
		global $config;
		if($config['installedpackages']['nut']['config'][0]["{$name}"])
			return $config['installedpackages']['nut']['config'][0]["{$name}"];
		return null;
	}	

	function nut_config_sub ($name,$len) {
		global $config;
		if(nut_config($name))
			return substr(nut_config($name),0,strlen(nut_config($name))-$len);
		return null;
	}	
	
	function nut_write_config($file, $text, $mask = null, $uid = null) {
		$conf = fopen($file, "w");
		if(!$conf) {
			nut_warn("Could not open {$file} for writing.");
			exit;
		}
		fwrite($conf, $text);
		fclose($conf);
		if($mask) chmod($file, $mask);
		if($uid) chown($file, $uid);
	}

	function nut_validate_ip($ip,$check_cdir) {
		/* validate cdir */	
		if($check_cdir)	{
			$ip_array = explode("/",$ip);
			if(count($ip_array) == 2) {
				if($ip_array[1] < 1 || $ip_array[1] > 32)
					return false;
			} else
				if(count($ip_array) != 1)
					return false;
		} else
			$ip_array[] = $ip;

		/* validate ip */
		if(!eregi("^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$", $ip_array[0]))
			return false;
		foreach(explode(".", $ip_array[0]) as $sub)
			if($sub < 0 || $sub > 256)
				return false;
		return true;
	}
	
	function before_form_nut($pkg) {
		/* return available serial ports */
		
		$serial_types = array("sio", "cua", "tty");
		$ignore_files = array(".lock", ".init");

		foreach($serial_types as $st) {
			$devices = glob("/dev/{$st}*");
			foreach($devices as $line) {
				if($line != '') {
					$ignore = false;
					foreach($ignore_files as $if) {
						if(strstr($line, $if)) {
							$ignore = true;
							continue;
						}
					}
					if($ignore == false) {
						$names[] = str_replace("/dev/", "", $line);
						$values[] = $line;
					}
				}
			}
		}

		/* find correct field */
		$i = 0;
		foreach ($pkg['fields']['field'] as $field) {
			if ($field['fieldname'] == 'port')
				break;
			$i++;
		}	
		$field = &$pkg['fields']['field'][$i];	

		/* add available serial ports */
		for ($i = 0; $i < count($values) ; $i++)
			$field['options']['option'][] = array('name' => $names[$i], 'value' => $values[$i]);
	}

	function validate_form_nut($post, $input_errors) {
		global $config;

		/* monitor remote validation */
		if($post['monitor'] == 'remote') {
			if(!$post['remotename'])
				$input_errors[] = 'You must specify a name in the \'Remote UPS Name\' field';
			if($post['remotename'] && !eregi('^[a-z0-9_.-]+$', $post['remotename']))
				$input_errors[] = 'Only [Aa-Zz], [0-9], and [-_] accepted in the \'Remote UPS Name\' field';		
			if(!$post['remoteaddr'] || !nut_validate_ip($post['remoteaddr'],false))
				$input_errors[] = 'You must specify a valid address \'Remote UPS Address\' field';
			if(!$post['remoteuser'])
				$input_errors[] = 'You must specify a name in the \'Remote UPS User\' field';
			if(!$post['remotepass'])
				$input_errors[] = 'You must specify a name in the \'Remote UPS Password\' field';
		}

		/* monitor local validation */
		elseif($post['monitor'] == 'local') {
			if(!$post['name'])
				$input_errors[] = 'You must specify a name in the \'Local UPS Name\' field';
			if($post['name'] && !eregi('^[a-z0-9_.-]+$', $post['name']))
				$input_errors[] = 'Only [Aa-Zz], [0-9], and [-_] accepted in the \'Local UPS Name\' field';			
			if(!$post['driver'])
				$input_errors[] = 'You must select a driver in the \'Local UPS Driver\' field';
			if(!$post['port'])
				$input_errors[] = 'You must select a port in the \'Local UPS Port\' field';
		}
	}

	function deinstall_package_nut() {
		nut_action('stop');

		@unlink(NUT_RCFILE);
		@unlink(NUT_DIR.'/upsmon.conf');
		@unlink(NUT_DIR.'/ups.conf');
		@unlink(NUT_DIR.'/upsd.conf');
		@unlink(NUT_DIR.'/upsd.users');

		exec('rm -rf /var/db/nut');
	}	

	function sync_package_nut_remote() {
		$remotename = nut_config('remotename');
		$remoteaddr = nut_config('remoteaddr');
		$remoteuser = nut_config('remoteuser');
		$remotepass = nut_config('remotepass');
		$shutdownflag  = (nut_config('powerdown') == 'on') ? '-p' : '-h';

		if(!($remotename && $remoteaddr && $remoteuser && $remotepass))
			return false;

		/* upsmon.conf */
		$upsmon_conf = <<<EOD
MONITOR {$remotename}@{$remoteaddr} 1 {$remoteuser} {$remotepass} slave
MINSUPPLIES 1
SHUTDOWNCMD "/sbin/shutdown {$shutdownflag} +0"
POWERDOWNFLAG /etc/killpower
EOD;
	
		$stop = <<<EOD
if [ `pgrep upsmon | wc -l` != 0 ]; then
		/usr/bin/killall upsmon
		while [ `pgrep upsmon | wc -l` != 0 ]; do 
			sleep 1
		done
	fi
EOD;
	
		$start = $stop."\n\t/usr/local/sbin/upsmon {$remotename}@{$remoteaddr}\n";

		/* write out configuration */
		conf_mount_rw();
		nut_write_config(NUT_DIR.'/upsmon.conf', $upsmon_conf, 0640, 'uucp');
		write_rcfile(array(
			'file' => 'nut.sh',
	    	'start' => $start,
	    	'stop' => $stop
	   		)
		);	
		conf_mount_ro();

		return true;
	}

	function sync_package_nut_local() {
		$name = nut_config('name');
		$driver = nut_config_sub('driver', 2);
		$port = nut_config('port');
		$upstype = nut_config_sub('upstype', 3);
		$cable = nut_config_sub('cable', 3);
		$allowuser = nut_config('allowuser');
		$allowpass = nut_config('allowpass');
		$shutdownflag  = (nut_config('powerdown') == 'on') ? '-p' : '-h';


		if(!($name && $driver && $port))
			return false;

		/* some installs are missing the lock dir */
		if(!is_dir('/var/spool/lock')) {
			@mkdir('/var/spool');
			mkdir('/var/spool/lock');
			chown('/var/spool/lock', 'uucp');
			chgrp('/var/spool/lock', 'dialer');
		}

		/* determine permissions for port */
		if($port != 'auto') {
			$port_rel = "chown root {$port}";
			$port_set = "chown uucp {$port}";
		}

		/* ups.conf */
		/* for usb ups run as root */
		if($port == 'auto') {
			$ups_conf = "user=root\n";
			$ovr_user = '-u root';
		}
		$ups_conf .= "[{$name}]\n";
		$ups_conf .= "driver={$driver}\n";
		$ups_conf .= "port={$port}\n";
		if($cable)
			$ups_conf .= "cable={$cable}\n";
		if($upstype)
			$ups_conf .= "upstype={$upstype}\n";

		/* upsd.conf */
		$upsd_conf = "LISTEN 127.0.0.1\n";
		$upsd_conf .= "LISTEN ::1\n";

		$password = uniqid("nut");
		/* upsd.users */
		$upsd_users = "[monuser]\n";
		$upsd_users .= "password = {$password}\n";
		$upsd_users .= "upsmon master\n";
		if($allowuser && $allowpass) {
			$upsd_users .= "\n[$allowuser]\n";
			$upsd_users .= "password = $allowpass\n";
			$upsd_users .= "upsmon master\n";
		}

		/* upsmon.conf */
		$upsmon_conf = <<<EOD
MONITOR {$name}@localhost 1 monuser {$password} master
MINSUPPLIES 1
SHUTDOWNCMD "/sbin/shutdown {$shutdownflag} +0"
POWERDOWNFLAG /etc/killpower
EOD;

		$stop = <<<EOD
	if [ `pgrep upsmon | wc -l` != 0 ]; then
		echo stopping upsmon
		/usr/bin/killall upsmon
		while [ `pgrep upsmon | wc -l` != 0 ]; do
			sleep 1
		done
	fi
	if [ `pgrep upsd | wc -l` != 0 ]; then
		echo stopping upsd
		/usr/bin/killall upsd
	fi
	if [ `pgrep {$driver} | wc -l` != 0 ]; then
		echo stopping {$driver}
		/usr/local/libexec/nut/upsdrvctl stop
	fi
	sleep 1
	if [ `pgrep {$driver} | wc -l` != 0 ]; then
		echo forcing {$driver} termination
		/usr/bin/killall {$driver}
		while [ `pgrep {$driver} | wc -l` != 0 ]; do
			sleep 1
		done
	fi
	{$port_rel}
EOD;

		$start = <<<EOD
if [ `pgrep {$driver} | wc -l` != 0 ]; then
	{$stop}
	fi
	{$port_set}
	echo starting {$driver}
	if /usr/local/libexec/nut/upsdrvctl start; then
		echo starting upsd
		/usr/local/sbin/upsd {$ovr_user}
		echo starting upsmon
		/usr/local/sbin/upsmon {$name}@localhost
	else
		echo {$driver} failed to start
	fi
EOD;

		/* write out configuration */
		conf_mount_rw();
		nut_write_config(NUT_DIR.'/ups.conf', $ups_conf);
		nut_write_config(NUT_DIR.'/upsd.conf', $upsd_conf, 0640, 'uucp');
		nut_write_config(NUT_DIR.'/upsd.users', $upsd_users, 0640, 'uucp');
		nut_write_config(NUT_DIR.'/upsmon.conf', $upsmon_conf, 0640, 'uucp');
		write_rcfile(array(
			'file' => 'nut.sh',
    		'start' => $start,
	    	'stop' => $stop
	   		)
		);
		conf_mount_ro();

		return true;
	}

	function sync_package_nut_snmp() {
		$name = nut_config('snmpname');
		$driver = "snmp-ups";
		$port = nut_config('snmpaddr');
		$shutdownflag  = (nut_config('powerdown') == 'on') ? '-p' : '-h';
		$snmpmib = nut_config('snmpmib');
		$snmpversion = nut_config('snmpversion');
		$snmpcommunity = nut_config('snmpcommunity');
		$snmpfreq = nut_config('snmpfreq');
		$snmpdisabletransfer = (nut_config('snmpdisabletransfer') == 'on');
		$allowuser = nut_config('allowuser');
		$allowpass = nut_config('allowpass');

		if(!($name && $driver && $port))
			return false;

		/* ups.conf */
		$ups_conf = "user=root\n";
		$ovr_user = '-u root';
		$ups_conf .= "[{$name}]\n";
		$ups_conf .= "driver={$driver}\n";
		$ups_conf .= "port={$port}\n";
		if($snmpmib)
			$ups_conf .= "mibs={$snmpmib}\n";
		if($snmpversion)
			$ups_conf .= "snmp_version={$snmpversion}\n";
		if($snmpcommunity)
			$ups_conf .= "community={$snmpcommunity}\n";
		if($snmpfreq)
			$ups_conf .= "pollfreq={$snmpfreq}\n";
		if($snmpdisabletransfer)
			$ups_conf .= "notransferoids=true\n";

		/* upsd.conf */
		$upsd_conf = "LISTEN 127.0.0.1\n";
		$upsd_conf .= "LISTEN ::1\n";

		$password = uniqid("nut");
		/* upsd.users */
		$upsd_users = "[monuser]\n";
		$upsd_users .= "password = {$password}\n";
		$upsd_users .= "upsmon master\n";
		if($allowuser && $allowpass) {
			$upsd_users .= "\n[$allowuser]\n";
			$upsd_users .= "password = $allowpass\n";
			$upsd_users .= "upsmon master\n";
		}

		/* upsmon.conf */
		$upsmon_conf = <<<EOD
MONITOR {$name}@localhost 1 monuser {$password} master
MINSUPPLIES 1
SHUTDOWNCMD "/sbin/shutdown {$shutdownflag} +0"
POWERDOWNFLAG /etc/killpower
EOD;

		$stop = <<<EOD
	if [ `pgrep upsmon | wc -l` != 0 ]; then
		echo stopping upsmon
		/usr/bin/killall upsmon
		while [ `pgrep upsmon | wc -l` != 0 ]; do
			sleep 1
		done
	fi
	if [ `pgrep upsd | wc -l` != 0 ]; then
		echo stopping upsd
		/usr/bin/killall upsd
	fi
	if [ `pgrep {$driver} | wc -l` != 0 ]; then
		echo stopping {$driver}
		/usr/local/libexec/nut/upsdrvctl stop
	fi
	sleep 1
	if [ `pgrep {$driver} | wc -l` != 0 ]; then
		echo forcing {$driver} termination
		/usr/bin/killall {$driver}
		while [ `pgrep {$driver} | wc -l` != 0 ]; do
			sleep 1
		done
	fi
	{$port_rel}
EOD;

		$start = <<<EOD
if [ `pgrep {$driver} | wc -l` != 0 ]; then
	{$stop}
	fi
	{$port_set}
	echo starting {$driver}
	if /usr/local/libexec/nut/upsdrvctl start; then
		echo starting upsd
		/usr/local/sbin/upsd {$ovr_user}
		echo starting upsmon
		/usr/local/sbin/upsmon {$name}@localhost
	else
		echo {$driver} failed to start
	fi
EOD;

		/* write out configuration */
		conf_mount_rw();
		nut_write_config(NUT_DIR.'/ups.conf', $ups_conf);
		nut_write_config(NUT_DIR.'/upsd.conf', $upsd_conf, 0640, 'uucp');
		nut_write_config(NUT_DIR.'/upsd.users', $upsd_users, 0640, 'uucp');
		nut_write_config(NUT_DIR.'/upsmon.conf', $upsmon_conf, 0640, 'uucp');
		write_rcfile(array(
			'file' => 'nut.sh',
		'start' => $start,
		'stop' => $stop
			)
		);
		conf_mount_ro();

		return true;
	}

	function sync_package_nut() {
		global $config;
		global $input_errors;

		config_lock();

		nut_action('stop');

		/* create state path */
		if(!is_dir('/var/db/nut')) {
			mkdir('/var/db/nut');
			chmod('/var/db/nut', 0700);
			chown('/var/db/nut', 'uucp');
		}

		if(nut_config('monitor') == 'remote')
			$return = sync_package_nut_remote();
		elseif(nut_config('monitor') == 'local')
			$return = sync_package_nut_local();
		elseif(nut_config('monitor') == 'snmp')
			$return = sync_package_nut_snmp();

		if($return && $_POST['monitor']) {
			/* only start if changing settings as we have a startup script for system boot */
			/* this prevents service from starting / stopping / starting on boot */
				
			nut_notice('Starting service');
			nut_action('start');
			if((int)exec('pgrep upsmon | wc -l') == 0)
				nut_notice('Service failed to start: check configuration');

		} elseif(!$return && file_exists(NUT_RCFILE)) {
			/* no parameters user does not want nut running */
			/* lets stop the service and remove the rc file */

			if(!nut_config('monitor'))
				nut_notice('Service stopped: nut disabled');
			else
				nut_notice('Service stopped: check configuration');

			conf_mount_rw();
			unlink(NUT_RCFILE);
			unlink(NUT_DIR.'/upsmon.conf');
			/* might not always exist */
			@unlink(NUT_DIR.'/ups.conf');
			@unlink(NUT_DIR.'/upsd.conf');
			@unlink(NUT_DIR.'/upsd.users');
			exec('rm -rf /var/db/nut');
			conf_mount_ro();
		}
		
		config_unlock();
	}
?>
