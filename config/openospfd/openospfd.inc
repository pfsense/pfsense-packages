<?php
/*
	openospfd.inc
	Copyright (C) 2010 Ermal Luçi
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
require_once("config.inc");
require_once("functions.inc");
require_once("service-utils.inc");

function ospfd_display_friendlyiface () {
        global $evaledvar, $config, $g;

	if (intval($g['latest_config']) >= 6)
		return;

        if ($evaledvar) {
                foreach ($evaledvar as $idx => $field) {
                        if ($field['interface']) {
                                if (empty($config['interfaces'][$field['interface']]['descr']))
                                        $evaledvar[$idx]['interface'] = strtoupper($evaledvar[$idx]['interface']);
                                else
                                        $evaledvar[$idx]['interface'] = $config['interfaces'][$field['interface']]['descr'];
                        }
                }
        }
}

function ospfd_install_conf() {
	global $config, $g, $input_errors;
	
	conf_mount_rw();
	
	if ($config['installedpackages']['ospfd']['rawconfig'] && $config['installedpackages']['ospfd']['rawconfig']['item']) {
		// if there is a raw config specifyed in tthe config.xml use that instead of the assisted config
		$conffile = implode("\n",$config['installedpackages']['ospfd']['rawconfig']['item']);
		//$conffile = $config['installedpackages']['ospfd']['rawconfig'];
	} else {
		// generate ospfd.conf based on the assistant
		if($config['installedpackages']['ospfd']['config']) 
			$ospfd_conf = &$config['installedpackages']['ospfd']['config'][0];
		else {
			log_error("OpenOSPFd: No config data found.");
			return;
		}
    
		$conffile = "# This file was created by the pfSense package manager.  Do not edit!\n\n";
    
		// Specify router id
		if($ospfd_conf['routerid']) 
			$conffile .= "router-id {$ospfd_conf['routerid']}\n";

		if ($ospfd_conf['updatefib'])
			$conffile .= "fib-update no\n";

		if (is_array($ospfd_conf['row'])) {
			foreach ($ospfd_conf['row'] as $redistr) {
				if (empty($redistr['routevalue']))
					continue;
				if (isset($redistr['redistribute']))
					$conffile .= "no ";
				$conffile .= "redistribute {$redistr['routevalue']}\n";
			}
		}

		if ($ospfd_conf['redistributeconnectedsubnets'])
			$conffile .= "redistribute connected\n";

		if ($ospfd_conf['redistributedefaultroute'])
			$conffile .= "redistribute default\n";

		if ($ospfd_conf['redistributestatic'])
			$conffile .= "redistribute static\n";
		
		if ($ospfd_conf['spfholdtime'])
			$conffile .= "spf-holdtime {$ospfd_conf['spfholdtime']}\n";

		if ($ospfd_conf['spfdelay'])
			$conffile .= "spf-delay {$ospfd_conf['spfdelay']}\n";

		if ($ospfd_conf['rfc1583'])
			$conffile .= "rfc1583compat yes\n";

		$conffile .= "area {$ospfd_conf['area']} {\n";

		if ($config['installedpackages']['ospfdinterfaces']['config']) {
			foreach ($config['installedpackages']['ospfdinterfaces']['config'] as $conf) {
				if (intval($g['latest_config']) >= 6) {
					$ospfdif = get_real_interface($conf['interface']);
					if ($ospfdif)
						$conffile .= "\tinterface {$ospfdif}";
					else
						continue;
				} else
					$conffile .= "\tinterface " . $config['interfaces'][$conf['interface']]['if'];
				$options_present = false;
				$ifconfigs = "";
				if (!empty($conf['metric'])) {
					$options_present = true;
					$ifconfigs .= "\t\tmetric {$conf['metric']}\n";
				}
				if (!empty($conf['hellointervalinseconds'])) {
					$options_present = true;
					$ifconfigs .= "\t\thello-interval {$conf['hellointervalinseconds']}\n";
				}
				if ($conf['md5password'] && !empty($conf['password'])) {
					$options_present = true;
					$ifconfigs .= "\t\tauth-type crypt\n";
					$ifconfigs .= "\t\tauth-md 1 \"" . substr($conf['password'], 0, 15) . "\"\n";
					$ifconfigs .= "\t\tauth-md-keyid 1\n";
				} else if (!empty($conf['password'])) {
					$options_present = true;
					$ifconfigs .= "\t\tauth-type simple\n";
					$ifconfigs .= "\t\tauth-key \"" . substr($conf['password'], 0, 8) . "\"\n";
				}
				if (!empty($conf['routerpriorityelections'])) {
					$options_present = true;
					$ifconfigs .= "\t\trouter-priority {$conf['routerpriorityelections']}\n";
				}
				if (!empty($conf['retransmitinterval'])) {
					$options_present = true;
					$ifconfigs .= "\t\tretransmit-interval {$conf['retransmitinterval']}\n";
				}
				if (!empty($conf['deadtimer'])) {
					$options_present = true;
					$ifconfigs .= "\t\trouter-dead-time {$conf['deadtimer']}\n";
                }
				if (!empty($conf['passive'])) {
					$options_present = true;
					$ifconfigs .= "\t\tpassive\n";
				}
				if (!empty($ifconfigs)) {
					$conffile .= " {\n {$ifconfigs} \n";
					if ($options_present == true)
						$conffile .= "\t}\n";
				} else
					$conffile .= "\n";
			}
		}
		$conffile .= "}\n";
	}

	$fd = fopen("/usr/local/etc/ospfd.conf", "w");
	
	// Write out the configuration file
	fwrite($fd, $conffile);
	
	// Close file handle
	fclose($fd);
	
	// Create rc.d file
	$rc_file_stop = "killall ospfd";
	$rc_file_start = <<<EOF
/usr/local/sbin/ospfctl reload
if [ "$0" != "0" ]; then
	/bin/pkill -x ospfd
	/usr/local/sbin/ospfd -f /usr/local/etc/ospfd.conf
fi

EOF;
	write_rcfile(array(
			"file" => "ospfd.sh",
			"start" => $rc_file_start,
			"stop" =>  $rc_file_stop
		)
	);
	
	// Ensure files have correct permissions
	exec("chmod a+rx /usr/local/etc/rc.d/ospfd.sh");
	exec("chmod a-rw /usr/local/etc/ospfd.conf");
	exec("chmod u+rw /usr/local/etc/ospfd.conf");

	// Kick off newly created rc.d script
	exec("/usr/local/etc/rc.d/ospfd.sh start");

	// Back to RO mount for NanoBSD and friends
	conf_mount_ro();
}

function ospfd_validate_interface() {
	global $config, $g, $id, $input_errors;

	if ($config['installedpackages']['ospfdinterfaces']['config']) {
		foreach ($config['installedpackages']['ospfdinterfaces']['config'] as $index => $conf) {
			if ($index == 0)
				continue;
			if ($id != $index && $conf['interface'] == $_POST['interface'])
				$input_errors[] = "Interface {$_POST['interface']} is already configured.";
		}
	}
	if ($_POST['md5password'] && empty($_POST['password']))
		$input_errors[] = "Please input a password.";
}

function ospfd_validate_input() {
	global $config, $g, $input_errors;

	if (!empty($_POST['routerid']) && !is_ipaddr($_POST['routerid']))
		$input_errors[] = "Router ID must be an address.";
	if (!is_ipaddr($_POST['area']))
		$input_errors[] = "Area needs to be a valid ip_address.";
	if ($_POST['spfholdtime'] <> "" && ($_POST['spfholdtime'] < 1 || $_POST['spfholdtime'] > 5))
		$input_errors[] = "SPF holdtime needs to be between 1 and 5.";
	if ($_POST['spfdelay'] <> "" && ($_POST['spfdelay'] < 1 || $_POST['spfdelay'] > 10))
		$input_errors[] = "SPF delay needs to be between 1 and 10.";
	if (!$config['installedpackages']['ospfdinterfaces']['config'])
		$input_errors[] = "Please select an interface to use for OpenOSPFd.";
}

// get the raw ospfd confi file for manual inspection/editing
function ospfd_get_raw_config() {
	return file_get_contents("/usr/local/etc/ospfd.conf");
}

// serialize the raw ospfd confi file to config.xml
function ospfd_put_raw_config($conffile) {
	global $config;
	if ($conffile == "")
		unset($config['installedpackages']['ospfd']['rawconfig']);
	else {
		$config['installedpackages']['ospfd']['rawconfig'] = array();
		$config['installedpackages']['ospfd']['rawconfig']['item'] = explode("\n",$_POST['ospfd_raw']);
		$config['installedpackages']['ospfd']['rawconfig'] = $conffile;
	}
}

?>
