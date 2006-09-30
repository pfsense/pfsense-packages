
require("/usr/local/pkg/snort.inc");

log_error("[SNORT] Snort_dynamic_ip_reload.php is starting.");

if($config['interfaces']['wan']['ipaddr'] == "pppoe" or
   $config['interfaces']['wan']['ipaddr'] == "dhcp") {
		log_error("Snort has detected a dynamic wan address. Reloading configuration.");
		stop_service("snort");
		create_snort_conf();
		sleep(5);
		start_service("snort");
}

log_error("[SNORT] Snort_dynamic_ip_reload.php is ending.");

