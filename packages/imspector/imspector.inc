<?php
	require_once("config.inc");
	require_once("functions.inc");

	/* IMSpector */

	function imspector_notice ($msg) { syslog(LOG_NOTICE, "imspector: $msg"); return; }
	function imspector_warn ($msg) { syslog(LOG_WARNING, "imspector: $msg"); return; }
	
	function imspector_config ($name) {
		global $config;
		if($config['installedpackages']['imspector']['config'][0]["{$name}"])
			return $config['installedpackages']['imspector']['config'][0]["{$name}"];
		else
			return NULL;
	}

	function write_imspector_config($conf_file, $conf_text) {
		$conf = fopen($conf_file, "w");
		if(!$conf) {
			imspector_warn("Could not open $conf_file for writing.");
			exit;
		}
		fwrite($conf, $conf_text);
		fclose($conf);	
	}	

	function imspector_pf_rdr($iface, $port) {
		return "rdr on {$iface} inet proto tcp from any to any port = {$port} -> 127.0.0.1 port 16667\n";
	}

	function imspector_pf_rule($iface, $port) {
		return "pass in quick on {$iface} inet proto tcp from any to any port {$port} keep state\n";
	}	

	function imspector_proto_to_port ($proto)
	{
		switch ($proto) {
			case "msn":
				return 1863;
			case "icqaim":
				return 5190;
			case "yahoo":
				return 5050;
			case "irc":
				return 6667;
			default:
				return NULL;
		}
	}
	
	function imspector_remove_rules() {
		$anchor = "imspector";
		exec("/sbin/pfctl -a " . $anchor . " -Fr"); 
		exec("/sbin/pfctl -a " . $anchor . " -Fn");
	}	

	function before_form_imspector($pkg) {
		global $config;
		
	}

	function validate_form_imspector($post, $input_errors) {

	}

	function sync_package_imspector() {
		global $config;
		global $input_errors;
		$configfile = "/usr/local/etc/imspector.conf";
		$proto="";
		$pf_rules="";

		imspector_notice("Syncing package");

		imspector_remove_rules();

		conf_mount_rw();
		config_lock();			

		if(imspector_config("proto_msn")) $proto .= "msn,";
		if(imspector_config("proto_icqaim")) $proto .= "icqaim,";
		if(imspector_config("proto_yahoo")) $proto .= "yahoo,";
		if(imspector_config("proto_irc")) $proto .= "irc,";
		
		if($proto != "") 
			$proto_array = explode(",",$proto);

		if(imspector_config("iface_array"))
			$iface_array = explode(",",imspector_config("iface_array"));

		if($iface_array && $proto_array) {	
			for($i=1;$i<=2;$i++) {			
				foreach($iface_array as $iface) {
					$if = convert_friendly_interface_to_real_interface_name($iface);
					/* above function returns iface if fail */
					if($if!=$iface) {
						$addr = find_interface_ip($if);
						/* non enabled interfaces are displayed in list on imspector settings page */
						/* check that the interface has an ip address before adding parameters */
						if($addr) {
							foreach($proto_array as $proto) {
								if($i==1 && imspector_proto_to_port($proto))
									$pf_rules .= imspector_pf_rdr($if,imspector_proto_to_port($proto));
								elseif ($i==2 && imspector_proto_to_port($proto))
									$pf_rules .= imspector_pf_rule($if,imspector_proto_to_port($proto));
							}							
						}
					}
				}
			}

			if($pf_rules != "") {
				exec("echo \"{$pf_rules}\" | /sbin/pfctl -a imspector -f -");

				$configtext = "plugin_dir=/usr/local/lib/imspector\n";
				if(imspector_config("log_file"))
					$configtext .= "file_logging_dir=/var/log/imspector\n";
				write_imspector_config($configfile,$configtext);					
					
				$stop = <<<EOD
if [ `pgrep imspector | wc -l` != 0  ]; then
		/usr/bin/killall imspector
		while [ `pgrep imspector | wc -l` != 0 ]; do
			sleep 1
		done	
	fi
EOD;
					
				$start = $stop."\n\t/usr/local/sbin/imspector -c \"{$configfile}\"";

				write_rcfile(array(
					    "file" => "imspector.sh",
					    "start" => $start,
					    "stop" => $stop
				    )
				);
			}
		
			if((int)exec("pgrep imspector | wc -l") == 0 || $_POST['iface_array']) {
				imspector_notice("Starting service");
				if(imspector_config("log_file"))
					exec("mkdir -p /var/log/imspector");
				start_service("imspector");
			} 	
		}

		if(!$iface_array || !$proto_array || $pf_rules = "") {
			/* no parameters user does not want imspector running */
			/* lets stop the service and remove the rc file */
		
			stop_service("imspector");
			imspector_warn("No interfaces and/or protocols stopping service");
			exec("rm -f /usr/local/etc/rc.d/imspector*");
			exec("rm -f {$configfile}");
		}	

		config_unlock();
		conf_mount_ro();		
	}
?>
