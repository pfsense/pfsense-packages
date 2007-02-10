<?php
	require_once("config.inc");
	require_once("functions.inc");

	/* IMSpector */

	define('IMSPECTOR_RCFILE', '/usr/local/etc/rc.d/imspector.sh');
	define('IMSPECTOR_CONFIG','/usr/local/etc/imspector/imspector.conf');

	function imspector_notice ($msg) { syslog(LOG_NOTICE, "imspector: $msg"); return; }
	function imspector_warn ($msg) { syslog(LOG_WARNING, "imspector: $msg"); return; }

	function imspector_action ($action) {
		if (file_exists(IMSPECTOR_RCFILE))
			mwexec(IMSPECTOR_RCFILE.' '.$action);
	}	

	function imspector_running () {
		if((int)exec("pgrep imspector | wc -l") > 0)
			return true;
		return false;
	}
	
	function imspector_config ($name) {
		global $config;
		if($config['installedpackages']['imspector']['config'][0]["{$name}"])
			return $config['installedpackages']['imspector']['config'][0]["{$name}"];
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
		return "rdr pass on {$iface} inet proto tcp from any to any port = {$port} -> 127.0.0.1 port 16667\n";
	}

	function imspector_pf_rule($iface, $port) {
		return "pass in quick on {$iface} inet proto tcp from any to any port {$port} keep state\n";
	}		

	function imspector_proto_to_port ($proto)
	{
		switch ($proto) {
			case "msn":
				return 1863;
			case "icq":
				return 5190;
			case "yahoo":
				return 5050;
			case "irc":
				return 6667;
			default:
				return NULL;
		}
	}

	function before_form_imspector($pkg) {
		global $config;
		
	}

	function validate_form_imspector($post, $input_errors) {
		if($post['iface_array'])
			foreach($post['iface_array'] as $iface)
				if($iface == "wan")
					$input_errors[] = 'It is a security risk to specify WAN in the \'Interface\' field';
	}

	function sync_package_imspector() {
		global $config;
		global $input_errors;

		config_lock();

		/* remove existing rules */
		exec("/sbin/pfctl -a imspector -Fr");
		exec("/sbin/pfctl -a imspector -Fn");		

		if(imspector_config("enable") && imspector_config("proto_array"))
			$proto_array = explode(",",imspector_config("proto_array"));

		if(imspector_config("enable") && imspector_config("iface_array"))
			$iface_array = explode(",",imspector_config("iface_array"));

		if($iface_array && $proto_array) {
			foreach($iface_array as $iface) {
				$if = convert_friendly_interface_to_real_interface_name($iface);
				/* above function returns iface if fail */
				if($if!=$iface) {
					$addr = find_interface_ip($if);
					/* non enabled interfaces are displayed in list on imspector settings page */
					/* check that the interface has an ip address before adding parameters */
					if($addr) {
						foreach($proto_array as $proto) {
							if(imspector_proto_to_port($proto))	{
								/* we can use rdr pass to auto create the filter rule */
								$pf_rules .= imspector_pf_rdr($if,imspector_proto_to_port($proto));
							}
						}
						if(!$ifaces_active)
							$ifaces_active = "{$iface}";
						else
							$ifaces_active .= ", {$iface}";
					} else {
						imspector_warn("Interface {$iface} has no ip address, ignoring");
					}
				} else {
					imspector_warn("Could not resolve real interface for {$iface}");
				}
			}

			if($pf_rules) {
				exec("echo \"{$pf_rules}\" | /sbin/pfctl -a imspector -f -");

				$configtext = "plugin_dir=/usr/local/lib/imspector\n";

				foreach($proto_array as $proto)
					$configtext .= "{$proto}_protocol=on\n";

				if(imspector_config("filter_badwords")) 
					$configtext .= "badwords_filename=/usr/local/etc/imspector/badwords.txt\n";		
					
				if(imspector_config("log_file")) {
					exec("mkdir -p /var/log/imspector");
					$configtext .= "file_logging_dir=/var/log/imspector\n";
				}
					
				if(imspector_config("log_mysql")) {
					$configtext .= "mysql_server=".imspector_config("mysql_server")."\n";
					$configtext .= "mysql_database=".imspector_config("mysql_database")."\n";
					$configtext .= "mysql_username=".imspector_config("mysql_username")."\n";
					$configtext .= "mysql_password=".imspector_config("mysql_password")."\n";
				}

				/* XXX: for now just enable tracing of errors */
				if(1) {
					exec("mkdir -p /tmp/trace");
					$configtext .= "icq_trace_error=on\n";
				}

				/* generate rc file start and stop */
				$stop = <<<EOD
if [ `pgrep imspector | wc -l` != 0  ]; then
		/usr/bin/killall imspector
		while [ `pgrep imspector | wc -l` != 0 ]; do
			sleep 1
		done	
	fi
EOD;

				$start = $stop."\n\tldconfig -m /usr/local/lib/mysql\n";
				$start .= "\t/usr/local/sbin/imspector -c \"".IMSPECTOR_CONFIG."\"";

				/* write out the configuration */
				conf_mount_rw();
				write_imspector_config(IMSPECTOR_CONFIG,$configtext);
				write_rcfile(array(
					    "file" => "imspector.sh",
					    "start" => $start,
					    "stop" => $stop
				    )
				);
				conf_mount_ro();

				/* if imspector not running start it */
				if(!imspector_running()) {
					imspector_notice("Starting service on interface: {$ifaces_active}");
					imspector_action("start");
				}
				/* or restart imspector if settings were changed */
				elseif($_POST['iface_array']) {
					imspector_notice("Restarting service on interface: {$ifaces_active}");
					imspector_action("restart");
				}				
			}
		}

		if(!$iface_array || !$proto_array || !$pf_rules) {
			/* no parameters user does not want imspector running */
			/* lets stop the service and remove the rc file */

			if(file_exists("/usr/local/etc/rc.d/imspector.sh")) {
				if(!imspector_config("enable"))
					imspector_notice("Stopping service: imspector disabled");
				else
					imspector_notice("Stopping service: no interfaces and/or protocols selected");
					
				imspector_action("stop");

				conf_mount_rw();
				unlink(IMSPECTOR_RCFILE);
				unlink(IMSPECTOR_CONFIG);
				conf_mount_ro();
			}			
		}	

		config_unlock();		
	}
?>
