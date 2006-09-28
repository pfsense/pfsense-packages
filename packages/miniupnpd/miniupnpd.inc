<?php
	/* Miniupnp */
	function sync_package_miniupnpd() {
		global $config;
		global $input_errors;
		conf_mount_rw();
		config_lock();
		$miniupnpd_config =&$_POST;
		$if_final = "";
		$ifaces_final = "";
		$first = 0;
		$wanif = get_real_wan_interface();

		if($_POST['overridewanip'])
			$overridewanip = $_POST['overridewanip'];
		else if($config['installedpackages']['miniupnpd']['config'][0]['overridewanip'])
			$overridewanip = $config['installedpackages']['miniupnpd']['config'][0]['overridewanip'];

		if($_POST['interface_array'])
			$interface_array = $_POST['interface_array'];
		else if(is_array($config['installedpackages']['miniupnpd']['config'][0]['interface_array']))
			$interface_array = $config['installedpackages']['miniupnpd']['config'][0]['interface_array'];

		if($interface_array) {
			foreach($_POST['interface_array'] as $iface) {
				$if = convert_friendly_interface_to_real_interface_name($iface);
				if($if) {
					$addr = find_interface_ip($if);
				    $ifaces_final .= " -i {$wanif} -a {$addr}";
				    $first = 1;
				} else {
					log_error("Could not resolve real interface {$iface}");
				}
			}
			$start = "# Clear existing rules and rdr entries \n";
			$start .= "/sbin/pfctl -aminiupnpd -Fr 2>&1 >/dev/null\n";
			$start .= "/sbin/pfctl -aminiupnpd -Fn 2>&1 >/dev/null\n";
			$start .= "/usr/local/sbin/miniupnpd -p 2869{$ifaces_final}";

			/* override wan ip address, common for carp, etc */
			if($overridewanip)
				$start .= " -o {$overridewanip}";

			$stop .= "/usr/bin/killall miniupnpd \n";
			$stop .= "# Clear existing rules and rdr entries \n";
			$stop .= "/sbin/pfctl -aminiupnpd -Fr 2>&1 >/dev/null\n";
			$stop .= "/sbin/pfctl -aminiupnpd -Fn 2>&1 >/dev/null\n";
			write_rcfile(array(
				    "file" => "miniupnpd.sh",
				    "start" => $start,
				    "stop" => $stop
			    )
			);
		}
		start_service("miniupnpd");
		restart_service("miniupnpd");
		config_unlock();
		conf_mount_ro();
	}
?>