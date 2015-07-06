<?php
/*
 * suricata_etiqrisk_update.php
 *
 * Significant portions of this code are based on original work done
 * for the Snort package for pfSense from the following contributors:
 * 
 * Copyright (C) 2005 Bill Marquette <bill.marquette@gmail.com>.
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009 Robert Zelaya Sr. Developer
 * Copyright (C) 2012 Ermal Luci
 * All rights reserved.
 *
 * Adapted for Suricata by:
 * Copyright (C) 2014 Bill Meeks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

require_once("config.inc");
require_once("functions.inc");
require_once("/usr/local/pkg/suricata/suricata.inc");
require("/usr/local/pkg/suricata/suricata_defs.inc");

/*************************************************************************
 * Hack for backwards compatibility with older 2.1.x pfSense versions    *
 * that did not contain the new "download_file()" utility function       *
 * present in 2.2 and higher.                                            *
 *************************************************************************/
if(!function_exists("download_file")) {
	function download_file($url, $destination, $verify_ssl = false, $connect_timeout = 60, $timeout = 0) {
		global $config, $g;

		$fp = fopen($destination, "wb");

		if (!$fp)
			return false;

		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, $verify_ssl);
		curl_setopt($ch, CURLOPT_FILE, $fp);
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, $connect_timeout);
		curl_setopt($ch, CURLOPT_TIMEOUT, $timeout);
		curl_setopt($ch, CURLOPT_HEADER, false);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($ch, CURLOPT_USERAGENT, $g['product_name'] . '/' . rtrim(file_get_contents("/etc/version")));

		if (!empty($config['system']['proxyurl'])) {
			curl_setopt($ch, CURLOPT_PROXY, $config['system']['proxyurl']);
			if (!empty($config['system']['proxyport']))
				curl_setopt($ch, CURLOPT_PROXYPORT, $config['system']['proxyport']);
			if (!empty($config['system']['proxyuser']) && !empty($config['system']['proxypass'])) {
				@curl_setopt($ch, CURLOPT_PROXYAUTH, CURLAUTH_ANY | CURLAUTH_ANYSAFE);
				curl_setopt($ch, CURLOPT_PROXYUSERPWD, "{$config['system']['proxyuser']}:{$config['system']['proxypass']}");
			}
		}

		@curl_exec($ch);
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		fclose($fp);
		curl_close($ch);
		return ($http_code == 200) ? true : $http_code;
	}
}

function suricata_check_iprep_md5($filename) {

	/**********************************************************/
	/* This function attempts to download the MD5 hash for    */
	/* the passed file and compare its contents to the        */
	/* currently stored hash file to see if a new file has    */
	/* been posted.                                           */
	/*                                                        */
	/* On Entry: $filename = IPREP file to check ('md5sum'    */
	/*                       is auto-appended to the supplied */
	/*                       filename.)                       */
	/*                                                        */
	/*  Returns: TRUE if new rule file download required.     */
	/*           FALSE if rule download not required or an    */
	/*           error occurred.                              */
	/**********************************************************/

	global $iqRisk_tmppath, $iprep_path;
	$new_md5 = $old_md5 = "";
	$et_iqrisk_url = str_replace("_xxx_", $config['installedpackages']['suricata']['config'][0]['iqrisk_code'], ET_IQRISK_DNLD_URL);

	if (download_file("{$et_iqrisk_url}{$filename}.md5sum", "{$iqRisk_tmppath}{$filename}.md5") == true) {
		if (file_exists("{$iqRisk_tmppath}{$filename}.md5"))
			$new_md5 = trim(file_get_contents("{$iqRisk_tmppath}{$filename}.md5"));
		if (file_exists("{$iprep_path}{$filename}.md5"))
			$old_md5 = trim(file_get_contents("{$iprep_path}{$filename}.md5"));
		if ($new_md5 != $old_md5)
			return TRUE;
		else
			log_error(gettext("[Suricata] IPREP file '{$filename}' is up to date."));
	}
	else
		log_error(gettext("[Suricata] An error occurred downloading {$et_iqrisk_url}{$filename}.md5sum for IPREP.  Update of {$filename} file will be skipped."));

	return FALSE;
}

/**********************************************************************
 * Start of main code                                                 *
 **********************************************************************/
global $g, $config;
$iprep_path = SURICATA_IPREP_PATH;
$iqRisk_tmppath = "{$g['tmp_path']}/IQRisk/";
$success = FALSE;

if (!is_array($config['installedpackages']['suricata']['config'][0]))
	$config['installedpackages']['suricata']['config'][0] = array();

// If auto-updates of ET IQRisk are disabled, then exit
if ($config['installedpackages']['suricata']['config'][0]['et_iqrisk_enable'] == "off")
	return(0);
else
	log_error(gettext("[Suricata] Updating the Emerging Threats IQRisk IP List..."));

// Construct the download URL using the saved ET IQRisk Subscriber Code
if (!empty($config['installedpackages']['suricata']['config'][0]['iqrisk_code'])) {
	$et_iqrisk_url = str_replace("_xxx_", $config['installedpackages']['suricata']['config'][0]['iqrisk_code'], ET_IQRISK_DNLD_URL);
}
else {
	log_error(gettext("[Suricata] No IQRisk subscriber code found!  Aborting scheduled update of Emerging Threats IQRisk IP List."));
	return(0);
}

// Download the IP List files to a temporary location
safe_mkdir("$iqRisk_tmppath");

// Test the posted MD5 checksum file against our local copy
// to see if an update has been posted for 'categories.txt'.
if (suricata_check_iprep_md5("categories.txt")) {
	log_error(gettext("[Suricata] An updated IPREP 'categories.txt' file is available...downloading new file."));
	if (download_file("{$et_iqrisk_url}categories.txt", "{$iqRisk_tmppath}categories.txt") != true)
		log_error(gettext("[Suricata] An error occurred downloading the 'categories.txt' file for IQRisk."));
	else {
		// If the files downloaded successfully, unpack them and store
		// the list files in the SURICATA_IPREP_PATH directory.
		if (file_exists("{$iqRisk_tmppath}categories.txt") && file_exists("{$iqRisk_tmppath}categories.txt.md5")) {
			$new_md5 = trim(file_get_contents("{$iqRisk_tmppath}categories.txt.md5"));
			if ($new_md5 == md5_file("{$iqRisk_tmppath}categories.txt")) {
				@rename("{$iqRisk_tmppath}categories.txt", "{$iprep_path}categories.txt");
				@rename("{$iqRisk_tmppath}categories.txt.md5", "{$iprep_path}categories.txt.md5");
				$success = TRUE;
				log_error(gettext("[Suricata] Successfully updated IPREP file 'categories.txt'."));
			}
			else
				log_error(gettext("[Suricata] MD5 integrity check of downloaded 'categories.txt' file failed!  Skipping update of this IPREP file."));
		}
	}
}

// Test the posted MD5 checksum file against our local copy
// to see if an update has been posted for 'iprepdata.txt.gz'.
if (suricata_check_iprep_md5("iprepdata.txt.gz")) {
	log_error(gettext("[Suricata] An updated IPREP 'iprepdata.txt' file is available...downloading new file."));
	if (download_file("{$et_iqrisk_url}iprepdata.txt.gz", "{$iqRisk_tmppath}iprepdata.txt.gz") != true)
		log_error(gettext("[Suricata] An error occurred downloading the 'iprepdata.txt.gz' file for IQRisk."));
	else {
		// If the files downloaded successfully, unpack them and store
		// the list files in the SURICATA_IPREP_PATH directory.
		if (file_exists("{$iqRisk_tmppath}iprepdata.txt.gz") && file_exists("{$iqRisk_tmppath}iprepdata.txt.gz.md5")) {
			$new_md5 = trim(file_get_contents("{$iqRisk_tmppath}iprepdata.txt.gz.md5"));
			if ($new_md5 == md5_file("{$iqRisk_tmppath}iprepdata.txt.gz")) {
				mwexec("/usr/bin/gunzip -f {$iqRisk_tmppath}iprepdata.txt.gz");
				@rename("{$iqRisk_tmppath}iprepdata.txt", "{$iprep_path}iprepdata.txt");
				@rename("{$iqRisk_tmppath}iprepdata.txt.gz.md5", "{$iprep_path}iprepdata.txt.gz.md5");
				$success = TRUE;
				log_error(gettext("[Suricata] Successfully updated IPREP file 'iprepdata.txt'."));
			}
			else
				log_error(gettext("[Suricata] MD5 integrity check of downloaded 'iprepdata.txt.gz' file failed!  Skipping update of this IPREP file."));
		}
	}
}

// Cleanup the tmp directory path
rmdir_recursive("$iqRisk_tmppath");

log_error(gettext("[Suricata] Emerging Threats IQRisk IP List update finished."));

// If successful, signal any running Suricata process to live reload the rules and IP lists
if ($success == TRUE && is_process_running("suricata")) {
	foreach ($config['installedpackages']['suricata']['rule'] as $value) {
		if ($value['enable_iprep'] == "on") {
			suricata_reload_config($value);
			sleep(2);
		}
	}
}

?>
