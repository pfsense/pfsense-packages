<?php

function frickin_custom_php_install_command() {
		$fout = fopen("/usr/local/etc/rc.d/frickin.sh", "w");
		fwrite($fout, "#!/bin/sh\n# This package was automatically generated\n# by the pfSense package system.\n\n");
		fwrite($fout, "# PACKAGE: Frickin Proxy\n");
		fwrite($fout, "# EXECUTABLE: frickin\n");
		fwrite($fout, "/usr/local/bin/frickin");
		fwrite($fout, " -s 127.0.0.1");
		fwrite($fout, " -c 20");
		fwrite($fout, " &\n");
		fclose($fout);
		mwexec("/usr/bin/killall frickin");
		mwexec("chmod a+rx /usr/local/etc/rc.d/frickin.sh");
		mwexec("/usr/local/etc/rc.d/frickin.sh");	
}

function custom_php_deinstall_command() {
	    unlink_if_exists("/usr/local/etc/rc.d/frickin.sh");
	    unlink_if_exists("/usr/local/bin/frickin");	
}

function frickin_generate_rules($type) {
	global $config;

	$frickin_conf = $config['installedpackages']['frickin']['config'][0];
	if (!is_service_running('frickin')) {
		log_error("Frickin is installed but not started.  Not installing redirect rules.");
		return;
	}

	$ifaces = explode(',', $frickin_conf['active_interface']);
	$ifaces = array_map('convert_friendly_interface_to_real_interface_name', $ifaces);

	switch($type) {
		case 'nat':
			foreach ($ifaces as $iface) {
				$rules .= "rdr on $iface proto tcp from any to !($iface) port 1723 -> 127.0.0.1\n";
				$rules .= "rdr on $iface proto gre from any to !($iface) -> 127.0.0.1\n";
			}
			break;
		case 'filter':
			break;
		default:
			break;
	}

	return $rules;
}

?>