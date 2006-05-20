<?php
function sync_package_havp() {
	conf_mount_rw();
	config_lock();
	global $config, $g;
	$fd = fopen("/etc/havp.config","w");
	fwrite($fd, "USER havp\n");
	fwrite($fd, "GROUP havp\n");
	fwrite($fd, "ACCESSLOG /var/log/havp/access.log\n");
	fwrite($fd, "ERRORLOG /var/log/havp/error.log\n");
	fwrite($fd, "DISPLAYINITIALMESSAGES false\n");
	fwrite($fd, "ENABLECLAMLIB true\n");
	fwrite($fd, "CLAMDBDIR /var/db/clamav/\n");
	fwrite($fd, "ENABLECLAMD false\n");
	fwrite($fd, "ENABLEFPROT false\n");
	fwrite($fd, "ENABLEAVG false\n");
	fwrite($fd, "ENABLEAVESERVER false\n");
	fwrite($fd, "ENABLESOPHIE false\n");
	fwrite($fd, "ENABLETROPHIE false\n");
	fwrite($fd, "ENABLENOD32 false\n");
	fwrite($fd, "ENABLEAVAST false\n");
        foreach($config['installedpackages']['havp']['config'] as $tmp) {
		fwrite($fd,"SERVERNUMBER " . $tmp['servernumber'] . "\n");
		fwrite($fd,"MAXSERVERS " . $tmp['maxservers'] . "\n");
		fwrite($fd,"PORT " . $tmp['port'] . "\n");
		if ($tmp['bind_address'] <> "")
		  fwrite($fd,"BIND_ADDRESS " . $tmp['bind_address'] . " \n");
		if ($tmp['source_address'] <> "")
		  fwrite($fd,"SOURCE_ADDRESS " . $tmp['source_address'] . " \n");
		if ($tmp['transparent'] == "on"){
		  fwrite($fd,"TRANSPARENT true\n");
		}
		else {
		  fwrite($fd,"TRANSPARENT false\n");
		}
		if ($tmp['range'] == "on"){
		  fwrite($fd,"RANGE true\n");
		}
		else {
		  fwrite($fd,"RANGE false\n");
		}
	}
	fclose($fd);
	$start = "/usr/local/sbin/havp &\n";
	$stop  = "/usr/bin/killall havp\n" .   "sleep 2";
	write_rcfile(array(
			   "file" => "havp.sh",
			   "start" => $start,
			   "stop" =>  $stop
			   )
		     );
	conf_mount_ro();
	config_unlock();
	unlink_if_exists("/usr/local/etc/havp/havp.config");
	mwexec("ln -sf /etc/havp.config /usr/local/etc/havp/havp.config");
	mwexec("/usr/local/etc/rc.d/havp.sh stop");
	mwexec("/usr/local/etc/rc.d/havp.sh start");
}

function havp_install_command() {
	global $config, $g;
	mwexec ("mkdir -p /var/log/havp/");
	mwexec ("mkdir -p /var/tmp/havp/");
	mwexec ("mkdir -p /var/run/havp/");
	mwexec ("chown havp:havp /var/log/havp");
	mwexec ("chown havp:havp /var/tmp/havp");
	mwexec ("chown havp:havp /var/run/havp");
	sync_package_havp();
}

function havp_deinstall_command() {
	global $config, $g;
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/havp.sh");
	unlink_if_exists("/usr/local/etc/havp/havp.config");
	conf_mount_ro();
}
function sync_package_havp_whitelist() {
	global $config;
	conf_mount_rw();
	config_lock();
	$fd = fopen("/usr/local/etc/havp/whitelist","w");
	if($config['installedpackages']['havpwhitelist']['config'] != "")
	    foreach($config['installedpackages']['havpwhitelist']['config'] as $tmp) {
		fwrite($fd, $tmp['url'] . "\n");
	    }
	fclose($fd);
	/* signal a reload of all files */
	conf_mount_ro();
	config_unlock();
}

function sync_package_havp_blacklist() {
	global $config;
	conf_mount_rw();
	config_lock();
	$fd = fopen("/usr/local/etc/havp/blacklist","w");
	if($config['installedpackages']['havpblacklist']['config'] != "")
	    foreach($config['installedpackages']['havpblacklist']['config'] as $tmp) {
		fwrite($fd, $tmp['url'] . "\n");
	    }
	fclose($fd);
	/* signal a reload of all files */
	conf_mount_ro();
	config_unlock();
}
?>