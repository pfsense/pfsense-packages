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

		$post_fields = array(
		                         'reason'=>urlencode($reason),  
		                         'hostname'=>urlencode($hostname),  
		                         'configxml'=>urlencode($configxml)
		                    );
		
		//url-ify the data for the POST  
		foreach($post_fields as $key=>$value) { $fields_string .= $key.'='.$value.'&'; }  
		rtrim($fields_string,'&');
		
		// Check configuration into the BSDP repo
		$curl_session = curl_init();
		curl_setopt($curl_session, CURLOPT_URL, $upload_url);  
		curl_setopt($curl_session, CURLOPT_POST, count($post_fields));  
		curl_setopt($curl_session, CURLOPT_POSTFIELDS, $fields_string);
		curl_setopt($curl_session, CURLOPT_SSL_VERIFYPEER, 0);
		$data = curl_exec($curl_session);
		if (curl_errno($curl_session)) {
			$fd = fopen("/tmp/backupdebug.txt", "w");
			fwrite($fd, $upload_url . "" . $fields_string . "\n\n");
			fwrite($fd, $data);
			fwrite($fd, curl_error($curl_session));
			fclose($fd);
		} else {
		    curl_close($curl_session);
		}
		
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
} else {
	log_error("No portal.pfsense.org backup required.");
}

?>