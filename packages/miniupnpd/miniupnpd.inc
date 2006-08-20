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
		if(is_array($_POST['interface_array'])) {
			foreach($_POST['interface_array'] as $iface) {
				$if = convert_friendly_interface_to_real_interface_name($iface);
				if($if) {
				    //if($first == 1)
					//$ifaces_final .= ",";
					$addr = find_interface_ip($if);
				    $ifaces_final .= " -i {$wanif} -a {$addr}";
				    $first = 1;
				} else {
					log_error("Could not resolve real interfaec {$iface}");
				}
			}
			$start = "/usr/local/sbin/miniupnpd -p 2869{$ifaces_final}";
			write_rcfile(array(
				    "file" => "miniupnpd.sh",
				    "start" => $start,
				    "stop" => "/usr/bin/killall miniupnpd"
			    )
			);
		}
		start_service("miniupnpd");
		restart_service("miniupnpd");
		config_unlock();
		conf_mount_ro();
	}
?>