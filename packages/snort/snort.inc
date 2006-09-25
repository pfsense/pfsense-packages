<?php

function sync_package_snort() {
	$first = 0;
	/* if list */
	$iflist = array("lan" => "LAN");
	for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++)
		$iflist['opt' . $i] = "opt{$i}";
	$whitelist = fopen("/var/db/whitelist","w");
	if(!$whitelist) {
		log_error("Cannot open whitelist for /var/db/writing.");
		exit;
	}
	foreach($iflist as $if) {
		/* XXX: write out if subnet */
	}
	fclose($whitelist);
	foreach($_POST['interface_array'] as $iface) {
		$if = convert_friendly_interface_to_real_interface_name($iface);
		if($if) {
			$ifaces_final .= " -i " . $if;
			$first = 1;
		}
	}
	$start  = "snort -c /usr/local/etc/snort/rules/snort.conf -l /var/log/snort " . $ifaces_final . " -D";
	$start .= ";snort2c -s -w /var/db/whitelist -a /var/log/snort/alert";
	write_rcfile(array(
			"file" => "snort.sh",
			"start" => $start,
			"stop" => "/usr/bin/killall snort; killall snort2c"
		)
	);
	start_service("snort");
}

function download_latest_rules() {
	$oinkid = "";
	$dl = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-CURRENT.tar.gz";
	$dl_md5 = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/snortrules-snapshot-CURRENT.tar.gz.md5";

}

?>