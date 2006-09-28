
require("/usr/local/pkg/snort.inc");

if($config['interfaces']['wan']['ipaddr'] == "pppoe" or
   $config['interfaces']['wan']['ipaddr'] == "dhcp") {
		stop_service("snort");
		create_snort_conf();
		sleep(1);
		start_service("snort");


}

