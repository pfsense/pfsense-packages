<?php

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2")) 
	require("crypt_acb.php");

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
if(!file_exists("/cf/conf/lastpfSbackup.txt")) {
	conf_mount_rw();
	touch("/cf/conf/lastpfSbackup.txt");
	conf_mount_ro();
}

$last_backup_date 	= str_replace("\n", "", file_get_contents("/cf/conf/lastpfSbackup.txt"));
$last_config_change = $config['revision']['time'];
$hostname  			= $config['system']['hostname'];
$reason	   			= $config['revision']['description'];
$username 			= $config['installedpackages']['autoconfigbackup']['config'][0]['username'];
$password  			= $config['installedpackages']['autoconfigbackup']['config'][0]['password'];
$encryptpw 			= $config['installedpackages']['autoconfigbackup']['config'][0]['crypto_password'];

// Define upload_url, must be present after other variable definitions due to username, password
$upload_url = "https://{$username}:{$password}@portal.pfsense.org/pfSconfigbackups/backup.php";

if(!$username or !$password or !$encryptpw) {

	$notice_text =  "Either the username, password or encryption password is not set for Automatic Configuration Backup.  ";
	$notice_text .= "Please correct this in Diagnostics -> AutoConfigBackup -> Settings.";
	log_error($notice_text);
	file_notice("AutoConfigBackup", $notice_text, $notice_text, "");

} else {
	/* If configuration has changed, upload to pfS */
	if($last_backup_date <> $last_config_change) {

			// Mount RW (if needed)
			conf_mount_rw();
			// Lock config
			config_lock();

			$notice_text = "Beginning http://portal.pfsense.org configuration backup.";
			log_error($notice_text);
			update_filter_reload_status($notice_text);

			// Encrypt config.xml
			$data = file_get_contents("/cf/conf/config.xml");
			$data = encrypt_data($data, $encryptpw);
			tagfile_reformat($data, $data, "config.xml");

			$post_fields = array(
			                         'reason'		=> urlencode($reason),  
			                         'hostname'		=> urlencode($hostname),  
			                         'configxml'	=> urlencode($data)
			                    );
		
			//url-ify the data for the POST  
			foreach($post_fields as $key=>$value) 
				$fields_string .= $key.'='.$value.'&'; 
			rtrim($fields_string,'&');
		
			// Check configuration into the BSDP repo
			$curl_session = curl_init();
			curl_setopt($curl_session, CURLOPT_URL, $upload_url);  
			curl_setopt($curl_session, CURLOPT_POST, count($post_fields));  
			curl_setopt($curl_session, CURLOPT_POSTFIELDS, $fields_string);
			curl_setopt($curl_session, CURLOPT_RETURNTRANSFER, 1);		
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

			if(!strstr($data, "500")) {
				$notice_text = "An error occured while uploading your pfSense configuration to portal.pfsense.org";
				log_error($notice_text . " - " . $data);
				file_notice("autoconfigurationbackup", $notice_text, $data, "");			
				update_filter_reload_status($notice_text . " - " . $data);	
			} else {
				// Update last pfS backup time
				$fd = fopen("/cf/conf/lastpfSbackup.txt", "w");
				fwrite($fd, $config['revision']['time']);
				fclose($fd);				
				$notice_text = "End of portal.pfsense.org configuration backup (success).";
				log_error($notice_text);
				update_filter_reload_status($notice_text);	
			}

			// Unlock config
			config_unlock();
			// Mount image RO (if needed)
			conf_mount_ro();

	} else {
		log_error("No http://portal.pfsense.org backup required.");
	}

}

?>