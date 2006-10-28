<?php
	require_once("config.inc");
	require_once("functions.inc");

	function upnp_notice ($msg) { syslog(LOG_NOTICE, "miniupnpd: $msg"); return; }
	function upnp_warn ($msg) { syslog(LOG_WARNING, "miniupnpd: $msg"); return; }

	/* Miniupnp */
	function sync_package_miniupnpd() {
		global $config;
		global $input_errors;
		$ifaces_final = "";		
		$wanif = get_real_wan_interface();

		upnp_notice("Syncing package");

		conf_mount_rw();
		config_lock();	

		/* since config is written before this file invoked we don't need to read post data */
		if($config['installedpackages']['miniupnpd']['config'][0]['overridewanip'])
			$overridewanip = $config['installedpackages']['miniupnpd']['config'][0]['overridewanip'];
		if($config['installedpackages']['miniupnpd']['config'][0]['iface_array'])
			$iface_array = explode(",",$config['installedpackages']['miniupnpd']['config'][0]['iface_array']);

		if($iface_array) {			
			foreach($iface_array as $iface) {
				$if = convert_friendly_interface_to_real_interface_name($iface);
				/* above function returns iface if fail */
				if($if!=$iface) {
					$addr = find_interface_ip($if);
					/* non enabled interfaces are displayed in list on miniupnpd settings page */
					/* check that the interface has an ip address before adding parameters */
					if($addr) {
						upnp_notice("Active on {$iface} interface");
			    		$ifaces_final .= " -i {$wanif} -a {$addr}";
					} else {
						upnp_warn("Interface {$iface} has no ip address");
					}
				} else {
					upnp_warn("Could not resolve real interface {$iface}");
				}
			}

			if($ifaces_final) {
				/* valid paramters lets create rc file and start miniupnpd */
				
				$start = "if [ `pgrep miniupnpd | wc -l` != 0  ]; then\n";
				$start .= "/usr/bin/killall miniupnpd\n";
				$start .= "# Clear existing rules and rdr entries \n";
				$start .= "/sbin/pfctl -aminiupnpd -Fr 2>&1 >/dev/null\n";
				$start .= "/sbin/pfctl -aminiupnpd -Fn 2>&1 >/dev/null\n";
				$start .= "while [ `pgrep miniupnpd | wc -l` != 0 ]; do sleep 1; done\n";
				$start .= "fi\n";
				$start .= "/usr/local/sbin/miniupnpd -p 2869{$ifaces_final}";

				/* override wan ip address, common for carp, etc */
				if($overridewanip)
					$start .= " -o {$overridewanip}";

				$stop = "/usr/bin/killall miniupnpd \n";			
				$stop .= "# Clear existing rules and rdr entries \n";				
				$stop .= "/sbin/pfctl -aminiupnpd -Fr 2>&1 >/dev/null\n";
				$stop .= "/sbin/pfctl -aminiupnpd -Fn 2>&1 >/dev/null\n";
				$stop .= "while [ `pgrep miniupnpd | wc -l` != 0 ]; do sleep 1; done";
				write_rcfile(array(
					    "file" => "miniupnpd.sh",
					    "start" => $start,
					    "stop" => $stop
				    )
				);

				/* if not ONE instance running lets start */
				/* or if $_POST data as user is changing settings */
				if((int)exec("pgrep miniupnpd | wc -l") != 1 || $_POST['iface_array']) {
					upnp_notice("Starting service");
					start_service("miniupnpd");
				} 	
			} 
		}

		if(!$iface_array || !$ifaces_final) {
			/* no parameters user does not want miniupnpd running */
			/* lets stop the service and remove the rc file */
		
			stop_service("miniupnpd");
			upnp_warn("No interfaces stopping service");
			exec("rm -f /usr/local/etc/rc.d/miniupnpd*");
		}
		
		config_unlock();
		conf_mount_ro();
	}
?>
