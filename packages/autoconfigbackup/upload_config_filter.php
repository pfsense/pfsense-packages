<?php

/*
 *   pfSense upload config to pfSense.org script
 *   This file plugs into filter.inc (/usr/local/pkg/pf)
 *   and runs every time the running firewall filter changes.
 * 
 *   Written by Scott Ullrich
 *   (C) 2008 BSD Perimeter LLC 
 *
 */

// Define some needed variables
$last_backup_date 	= str_replace("\n", "", file_get_contents("/cf/conf/lastpfSbackup.txt"));
$last_config_change = $config['revision']['time'];
$hostname  			= $config['system']['hostname'];
$username 			= $config['installedpackages']['autoconfigbackup']['config'][0]['username'];
$password  			= $config['installedpackages']['autoconfigbackup']['config'][0]['password'];
$encryptpw 			= $config['installedpackages']['autoconfigbackup']['config'][0]['crypto_password'];
$reason	   			= $config['revision']['description'];

// Define upload_url, must be present after other variable definitions due to username, password
$upload_url = "https://{$username}:{$password}@portal.pfsense.org/pfSconfigbackups/backup.php";

/* If configuration has changed, upload to pfS */
if($last_backup_date <> $last_config_change) {
	if($username && $password && $encryptpw) {

		// Mount RW (if needed)
		conf_mount_rw();
		// Lock config
		config_lock();
		
		log_error("Beginning portal.pfsense.org configuration backup.");

		// Encrypt config.xml
		$data = file_get_contents("/cf/conf/config.xml");
		$configxml = encrypt_data($data, $encryptpw);
		tagfile_reformat($data, $data, "config.xml");

		// Check configuration into the BSDP repo
		$curl_Session = curl_init($upload_url);
		curl_setopt($curl_Session, CURLOPT_POST, 1);
		curl_setopt($curl_Session, CURLOPT_POSTFIELDS, "reason={$reason}&configxml={$configxml}&hostname={$hostname}");
		curl_setopt($curl_Session, CURLOPT_FOLLOWLOCATION, 1);
		$data = curl_exec($curl_Session);
		curl_close($curl_Session);
		
		// Update last pfS backup time
		$fd = fopen("/cf/conf/lastpfSbackup.txt", "w");
		fwrite($fd, $config['revision']['time']);
		fclose($fd);

		log_error("End of portal.pfsense.org configuration backup.");

		// Unlock config
		config_unlock();
		// Mount image RO (if needed)
		conf_mount_ro();

	}
}

?>