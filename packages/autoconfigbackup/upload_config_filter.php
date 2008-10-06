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


$last_backup_date 	= $config['system']['lastpfSbackup'];
$last_config_change = $config['revision']['time'];

/* If configuration has changed, upload to pfS */
if($last_backup_date <> $last_config_change) {

	$hostname = $config['system']['hostname'];
	$username = $config['installedpackages']['pfSautoconfigbackup']['config']['username'];
	$password = $config['installedpackages']['pfSautoconfigbackup']['config']['password'];
	$reason	  = $config['revision']['description'];

	$upload_url = "https://{$username}:{$password}@portal.pfsense.org/pfSconfigbackups/backup.php";

	$curl_Session = curl_init($upload_url);
	curl_setopt ($curl_Session, CURLOPT_POST, 1);
	curl_setopt ($curl_Session, CURLOPT_POSTFIELDS, "reason={$reason}&configxml={$configxml}&hostname={$hostname}");
	curl_setopt ($curl_Session, CURLOPT_FOLLOWLOCATION, 1);
	curl_exec 	($curl_Session);
	curl_close 	($curl_Session);

}

?>