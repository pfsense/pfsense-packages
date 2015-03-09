<?php
function sync_package_ftpproxy() {
	conf_mount_rw();
	config_lock();
	global $config;
	$cf = $config['installedpackages']['ftpclientproxy']['config'][0];

	/* Proxy is not enabled, kill the daemon and issue a filter reload. */
	if ($cf["proxy_enable"] != "on") {
		mwexec("/usr/bin/killall -9 ftp-proxy");
		filter_configure();
		return;
	}
	$interface_list = explode(",", $cf['localints']);
	/* Bail if there is nothing to do */
	if (empty($interface_list)) {
		log_error("FTP Proxy cannot sync: No interfaces selected.");
		return;
	}

	$start = "/usr/bin/killall -9 ftp-proxy\n";
	$start .= "\t/usr/sbin/ftp-proxy ";

	if ($cf["anononly"] == "on") {
		$start .= " -A ";
	}
	if (is_ipaddr($cf["sourceaddr"])) {
		$start .= " -a " . escapeshellarg($cf["sourceaddr"]);
	}
	if (is_port($cf["bindport"])) {
		$start .= " -p " . escapeshellarg($cf["bindport"]);
	}
	if (is_numeric($cf["maxessions"]) && ($cf["maxessions"] >= 1) && ($cf["maxessions"] <= 500)) {
		$start .= " -m " . escapeshellarg($cf["maxessions"]);
	}
	if (!empty($cf["tsq"])) {
		$start .= " -q " . escapeshellarg($cf["tsq"]);
	}
	if ($cf["src20"] == "on") {
		$start .= " -r ";
	}
	if (is_numeric($cf["idletimeout"]) && ($cf["idletimeout"] > 0) && ($cf["idletimeout"] <= 86400)) {
		$start .= " -t " . escapeshellarg($cf["idletimeout"]);
	}
	if ($cf["log"] == "on") {
		$start .= " -v ";
	}
	$start .= "\n";

	write_rcfile(array(
			"file" => "ftp-proxy.sh",
			"start" => $start,
			"stop" => "/usr/bin/killall -9 ftp-proxy"
		)
	);
	restart_service("ftp-proxy");
	conf_mount_ro();
	config_unlock();
	filter_configure();
}

function validate_form_ftpproxy($post, &$input_errors) {
	if (empty($post["localints"])) {
		$input_errors[] = 'One or more Local Interfaces must be selected';
	}
	if (!empty($post["sourceaddr"]) && !is_ipaddr($post["sourceaddr"])) {
		$input_errors[] = 'You must specify a valid ip address in the \'Source Address\' field';
	}
	if (!empty($post["bindport"]) && !is_port($post["bindport"])) {
		$input_errors[] = 'You must specify a valid port number in the \'Bind Port\' field';
	}
	if (!empty($post["maxessions"]) && (!is_numeric($post["maxessions"]) || ($post["maxessions"] < 1) || ($post["maxessions"] > 500))) {
		$input_errors[] = 'You must specify a valid number in the \'Max Sessions\' field (Between 1 and 500)';
	}
	if (!empty($post["idletimeout"]) && (is_numeric($post["idletimeout"]) || ($post["idletimeout"] <= 0) || ($post["idletimeout"] > 86400))) {
		$input_errors[] = 'You must specify a valid number in the \'Idle Timeout\' field (Between 1 and 86400)';
	}
}

function ftpproxy_get_port() {
	global $config;
	$cf = $config['installedpackages']['ftpclientproxy']['config'][0];
	if (!empty($cf["bindport"]) && is_port($cf["bindport"])) {
		return $cf["bindport"];
	} else {
		return 8021;
	}
}

function ftpproxy_generate_rules($type) {
	global $config;
	$cf = $config['installedpackages']['ftpclientproxy']['config'][0];
	$interface_list = explode(",", $cf['localints']);

	/* Proxy is not enabled, therefore, no rules/anchors. */
	if ($cf["proxy_enable"] != "on") {
		return;
	}

	/* Bail if there is nothing to do */
	if (empty($interface_list)) {
		log_error("FTP Proxy cannot sync: No interfaces selected.");
		return;
	}

	$rules = "";
	switch ($type) {
		case "nat":
			$rules .= "nat-anchor \"ftp-proxy/*\"\n";
			$rules .= "rdr-anchor \"ftp-proxy/*\"\n";

			foreach ($interface_list as $interface_friendly) {
				if (empty($interface_friendly)) {
					continue;
				}
				$interface = get_real_interface($interface_friendly);
				if (empty($interface)) {
					continue;
				}
				$rules .= "rdr pass on {$interface} inet proto tcp from any to any port 21 -> 127.0.0.1 port " . ftpproxy_get_port() . "\n";
			}
			break;
		case "filter":
			$rules .= "anchor \"ftp-proxy/*\"\n";
			// $rules = "pass out proto tcp from any to any port 21\n";
			break;

	}
	return $rules;
}
?>