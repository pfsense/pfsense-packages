
require("/usr/local/pkg/snort.inc");

if($config['interfaces']['wan']['ipaddr'] == "pppoe" or
   $config['interfaces']['wan']['ipaddr'] == "dhcp") {
		log_error("Snort has detected an IP address change.  Reloading.");
		stop_service("snort");
		create_snort_conf();
		sleep(5);
		start_service("snort");
}

