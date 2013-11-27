<?php
/*
 * snort_check_for_rule_updates.php
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009 Robert Zelaya
 * Copyright (C) 2011-2012 Ermal Luci
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
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

require_once("functions.inc");
require_once("service-utils.inc");
require_once "/usr/local/pkg/snort/snort.inc";

global $g, $pkg_interface, $snort_gui_include, $rebuild_rules;

if (!defined("VRT_DNLD_URL"))
	define("VRT_DNLD_URL", "https://www.snort.org/reg-rules/");
if (!defined("ET_VERSION"))
	define("ET_VERSION", "2.9.0");
if (!defined("ET_BASE_DNLD_URL"))
	define("ET_BASE_DNLD_URL", "http://rules.emergingthreats.net/"); 
if (!defined("ETPRO_BASE_DNLD_URL"))
	define("ETPRO_BASE_DNLD_URL", "https://rules.emergingthreatspro.com/"); 
if (!defined("ET_DNLD_FILENAME"))
	define("ET_DNLD_FILENAME", "emerging.rules.tar.gz");
if (!defined("ETPRO_DNLD_FILENAME"))
	define("ETPRO_DNLD_FILENAME", "etpro.rules.tar.gz");
if (!defined("GPLV2_DNLD_FILENAME"))
	define("GPLV2_DNLD_FILENAME", "community-rules.tar.gz");
if (!defined("GPLV2_DNLD_URL"))
	define("GPLV2_DNLD_URL", "https://s3.amazonaws.com/snort-org/www/rules/community/");
if (!defined("RULES_UPD_LOGFILE"))
	define("RULES_UPD_LOGFILE", SNORTLOGDIR . "/snort_rules_update.log");
if (!defined("VRT_FILE_PREFIX"))
	define("VRT_FILE_PREFIX", "snort_");
if (!defined("GPL_FILE_PREFIX"))
	define("GPL_FILE_PREFIX", "GPLv2_");
if (!defined("ET_OPEN_FILE_PREFIX"))
	define("ET_OPEN_FILE_PREFIX", "emerging-");
if (!defined("ET_PRO_FILE_PREFIX"))
	define("ET_PRO_FILE_PREFIX", "etpro-");

$snortdir = SNORTDIR;
$snortlibdir = SNORTLIBDIR;
$snortlogdir = SNORTLOGDIR;
$snort_rules_upd_log = RULES_UPD_LOGFILE;

/* Save the state of $pkg_interface so we can restore it */
$pkg_interface_orig = $pkg_interface;
if ($snort_gui_include)
	$pkg_interface = "";
else
	$pkg_interface = "console";

/* define checks */
$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
$etproid = $config['installedpackages']['snortglobal']['etpro_code'];
$snortdownload = $config['installedpackages']['snortglobal']['snortdownload'];
$emergingthreats = $config['installedpackages']['snortglobal']['emergingthreats'];
$etpro = $config['installedpackages']['snortglobal']['emergingthreats_pro'];
$snortcommunityrules = $config['installedpackages']['snortglobal']['snortcommunityrules'];
$vrt_enabled = $config['installedpackages']['snortglobal']['snortdownload'];
$et_enabled = $config['installedpackages']['snortglobal']['emergingthreats'];

/* Working directory for downloaded rules tarballs */
$tmpfname = "{$snortdir}/tmp/snort_rules_up";

/* Grab the Snort binary version programmatically and use it to construct */
/* the proper Snort VRT rules tarball and md5 filenames.                  */
exec("/usr/local/bin/snort -V 2>&1 |/usr/bin/grep Version | /usr/bin/cut -c20-26", $snortver);
// Save the version with decimal delimiters for use in extracting the rules
$snort_version = $snortver[0];
// Create a collapsed version string for use in the tarball filename
$snortver[0] = str_replace(".", "", $snortver[0]);
$snort_filename = "snortrules-snapshot-{$snortver[0]}.tar.gz";
$snort_filename_md5 = "{$snort_filename}.md5";
$snort_rule_url = VRT_DNLD_URL;

/* Set up Emerging Threats rules filenames and URL */
if ($etpro == "on") {
	$emergingthreats_filename = ETPRO_DNLD_FILENAME;
	$emergingthreats_filename_md5 = ETPRO_DNLD_FILENAME . ".md5";
	$emergingthreats_url = ETPRO_BASE_DNLD_URL;
	$emergingthreats_url .= "{$etproid}/snort-" . ET_VERSION . "/";
	$emergingthreats = "on";
	$et_enabled= "on";
	$et_name = "Emerging Threats Pro";
	$et_md5_remove = ET_DNLD_FILENAME . ".md5";
	@unlink("{$snortdir}/{$et_md5_remove}");
}
else {
	$emergingthreats_filename = ET_DNLD_FILENAME;
	$emergingthreats_filename_md5 = ET_DNLD_FILENAME . ".md5";
	$emergingthreats_url = ET_BASE_DNLD_URL;
	// If using Sourcefire VRT rules with ET, then we should use the open-nogpl ET rules
	$emergingthreats_url .= $vrt_enabled == "on" ? "open-nogpl/" : "open/";
	$emergingthreats_url .= "snort-" . ET_VERSION . "/";
	$et_name = "Emerging Threats Open";
	$et_md5_remove = ETPRO_DNLD_FILENAME . ".md5";
	@unlink("{$snortdir}/{$et_md5_remove}");
}

/* Snort GPLv2 Community Rules filenames and URL */
$snort_community_rules_filename = GPLV2_DNLD_FILENAME;
$snort_community_rules_filename_md5 = GPLV2_DNLD_FILENAME . ".md5";
$snort_community_rules_url = GPLV2_DNLD_URL;

function snort_download_file_url($url, $file_out) {

	/************************************************/
	/* This function downloads the file specified   */
	/* by $url using the CURL library functions and */
	/* saves the content to the file specified by   */
	/* $file.                                       */
	/*                                              */
	/* This is needed so console output can be      */
	/* suppressed to prevent XMLRPC sync errors.    */
	/*                                              */
	/* It provides logging of returned CURL errors. */
	/************************************************/

	global $g, $config, $pkg_interface, $last_curl_error, $fout, $ch, $file_size, $downloaded, $first_progress_update;

	// Initialize required variables for the pfSense "read_body()" function
	$file_size  = 1;
	$downloaded = 1;
	$first_progress_update = TRUE;


	// Array of message strings for HTTP Response Codes
	$http_resp_msg = array( 200 => "OK", 202 => "Accepted", 204 => "No Content", 205 => "Reset Content", 
				206 => "Partial Content", 301 => "Moved Permanently", 302 => "Found", 
				305 => "Use Proxy", 307 => "Temporary Redirect", 400 => "Bad Request", 
				401 => "Unauthorized", 402 => "Payment Required", 403 => "Forbidden", 
				404 => "Not Found", 405 => "Method Not Allowed", 407 => "Proxy Authentication Required", 
				408 => "Request Timeout", 410 => "Gone", 500 => "Internal Server Error", 
				501 => "Not Implemented", 502 => "Bad Gateway", 503 => "Service Unavailable", 
				504 => "Gateway Timeout", 505 => "HTTP Version Not Supported" );

	$last_curl_error = "";

	$fout = fopen($file_out, "wb");
	if ($fout) {
		$ch = curl_init($url);
		if (!$ch)
			return false;
		curl_setopt($ch, CURLOPT_FILE, $fout);

		// NOTE: required to suppress errors from XMLRPC due to progress bar output
		if ($g['snort_sync_in_progress'])
			curl_setopt($ch, CURLOPT_HEADER, false);
		else {
			curl_setopt($ch, CURLOPT_HEADERFUNCTION, 'read_header');
			curl_setopt($ch, CURLOPT_WRITEFUNCTION, 'read_body');
		}

		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Win64; x64; Trident/6.0)");
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 30);
		curl_setopt($ch, CURLOPT_TIMEOUT, 0);

		// Use the system proxy server setttings if configured
		if (!empty($config['system']['proxyurl'])) {
			curl_setopt($ch, CURLOPT_PROXY, $config['system']['proxyurl']);
			if (!empty($config['system']['proxyport']))
				curl_setopt($ch, CURLOPT_PROXYPORT, $config['system']['proxyport']);
			if (!empty($config['system']['proxyuser']) && !empty($config['system']['proxypass'])) {
				@curl_setopt($ch, CURLOPT_PROXYAUTH, CURLAUTH_ANY | CURLAUTH_ANYSAFE);
				curl_setopt($ch, CURLOPT_PROXYUSERPWD, "{$config['system']['proxyuser']}:{$config['system']['proxypass']}");
			}
		}

		$counter = 0;
		$rc = true;
		// Try up to 4 times to download the file before giving up
		while ($counter < 4) {
			$counter++;
			$rc = curl_exec($ch);
			if ($rc === true)
				break;
			log_error(gettext("[Snort] Rules download error: " . curl_error($ch)));
			log_error(gettext("[Snort] Will retry in 15 seconds..."));
			sleep(15);
		}
		if ($rc === false)
			$last_curl_error = curl_error($ch);
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		if (isset($http_resp_msg[$http_code]))
			$last_curl_error = $http_resp_msg[$http_code];
		curl_close($ch);
		fclose($fout);

		// If we had to try more than once, log it
		if ($counter > 1)
			log_error(gettext("File '" . basename($file_out) . "' download attempts: {$counter} ..."));
		return ($http_code == 200) ? true : $http_code;
	}
	else {
		$last_curl_error = gettext("Failed to create file " . $file_out);
		log_error(gettext("[Snort] Failed to create file {$file_out} ..."));
		return false;
	}
}

function snort_check_rule_md5($file_url, $file_dst, $desc = "") {

	/**********************************************************/
	/* This function attempts to download the passed MD5 hash */
	/* file and compare its contents to the currently stored  */
	/* hash file to see if a new rules file has been posted.  */
	/*                                                        */
	/* On Entry: $file_url = URL for md5 hash file            */
	/*           $file_dst = Temp destination to store the    */
	/*                       downloaded hash file             */
	/*           $desc     = Short text string used to label  */
	/*                       log messages with rules type     */
	/*                                                        */
	/*  Returns: TRUE if new rule file download required.     */
	/*           FALSE if rule download not required or an    */
	/*           error occurred.                              */
	/**********************************************************/

	global $pkg_interface, $snort_rules_upd_log, $last_curl_error;

	$snortdir = SNORTDIR;
	$filename_md5 = basename($file_dst);

	if ($pkg_interface <> "console")
		update_status(gettext("Downloading {$desc} md5 file..."));
	error_log(gettext("\tDownloading {$desc} md5 file {$filename_md5}...\n"), 3, $snort_rules_upd_log);
	$rc = snort_download_file_url($file_url, $file_dst);

	// See if download from URL was successful
	if ($rc === true) {
		if ($pkg_interface <> "console")
			update_status(gettext("Done downloading {$filename_md5}."));
		error_log("\tChecking {$desc} md5 file...\n", 3, $snort_rules_upd_log);

		// check md5 hash in new file against current file to see if new download is posted
		if (file_exists("{$snortdir}/{$filename_md5}")) {
			$md5_check_new = file_get_contents($file_dst);
			$md5_check_old = file_get_contents("{$snortdir}/{$filename_md5}");
			if ($md5_check_new == $md5_check_old) {
				if ($pkg_interface <> "console")
					update_status(gettext("{$desc} are up to date..."));
				log_error(gettext("[Snort] {$desc} are up to date..."));
				error_log(gettext("\t{$desc} are up to date.\n"), 3, $snort_rules_upd_log);
				return false;
			}
			else
				return true;
		}
		return true;
	}
	else {
		error_log(gettext("\t{$desc} md5 download failed.\n"), 3, $snort_rules_upd_log);
		$snort_err_msg = gettext("Server returned error code {$rc}.");
		if ($pkg_interface <> "console") {
			update_status(gettext("{$desc} md5 error ... Server returned error code {$rc} ..."));
			update_output_window(gettext("{$desc} will not be updated.\n\t{$snort_err_msg}")); 
		}
		log_error(gettext("[Snort] {$desc} md5 download failed..."));
		log_error(gettext("[Snort] Server returned error code {$rc}..."));
		error_log(gettext("\t{$snort_err_msg}\n"), 3, $snort_rules_upd_log);
		if ($pkg_interface == "console")
			error_log(gettext("\tServer error message was: {$last_curl_error}\n"), 3, $snort_rules_upd_log);
		error_log(gettext("\t{$desc} will not be updated.\n"), 3, $snort_rules_upd_log);
		return false;
	}
}

function snort_fetch_new_rules($file_url, $file_dst, $file_md5, $desc = "") {

	/**********************************************************/
	/* This function downloads the passed rules file and      */
	/* compares its computed md5 hash to the passed md5 hash  */
	/* to verify the file's integrity.                        */
	/*                                                        */
	/* On Entry: $file_url = URL of rules file                */
	/*           $file_dst = Temp destination to store the    */
	/*                       downloaded rules file            */
	/*           $file_md5 = Expected md5 hash for the new    */
	/*                       downloaded rules file            */
	/*           $desc     = Short text string for use in     */
	/*                       log messages                     */
	/*                                                        */
	/*  Returns: TRUE if download was successful.             */
	/*           FALSE if download was not successful.        */
	/**********************************************************/

	global $pkg_interface, $snort_rules_upd_log, $last_curl_error;

	$snortdir = SNORTDIR;
	$filename = basename($file_dst);

	if ($pkg_interface <> "console")
		update_status(gettext("There is a new set of {$desc} posted. Downloading..."));
	log_error(gettext("[Snort] There is a new set of {$desc} posted. Downloading {$filename}..."));
	error_log(gettext("\tThere is a new set of {$desc} posted.\n"), 3, $snort_rules_upd_log);
	error_log(gettext("\tDownloading file '{$filename}'...\n"), 3, $snort_rules_upd_log);
       	$rc = snort_download_file_url($file_url, $file_dst);

	// See if the download from the URL was successful
	if ($rc === true) {
		if ($pkg_interface <> "console")
			update_status(gettext("Done downloading {$desc} file."));
		log_error("[Snort] {$desc} file update downloaded successfully");
		error_log(gettext("\tDone downloading rules file.\n"),3, $snort_rules_upd_log);
	
		// Test integrity of the rules file.  Turn off update if file has wrong md5 hash
		if ($file_md5 != trim(md5_file($file_dst))){
			if ($pkg_interface <> "console")
				update_output_window(gettext("{$desc} file MD5 checksum failed..."));
			log_error(gettext("[Snort] {$desc} file download failed.  Bad MD5 checksum..."));
        	        log_error(gettext("[Snort] Downloaded File MD5: " . md5_file($file_dst)));
			log_error(gettext("[Snort] Expected File MD5: {$file_md5}"));
			error_log(gettext("\t{$desc} file download failed.  Bad MD5 checksum.\n"), 3, $snort_rules_upd_log);
			error_log(gettext("\tDownloaded {$desc} file MD5: " . md5_file($file_dst) . "\n"), 3, $snort_rules_upd_log);
			error_log(gettext("\tExpected {$desc} file MD5: {$file_md5}\n"), 3, $snort_rules_upd_log);
			error_log(gettext("\t{$desc} file download failed.  {$desc} will not be updated.\n"), 3, $snort_rules_upd_log);
			return false;
		}
		return true;
	}
	else {
		if ($pkg_interface <> "console")
			update_output_window(gettext("{$desc} file download failed..."));
		log_error(gettext("[Snort] {$desc} file download failed... server returned error '{$rc}'..."));
		error_log(gettext("\t{$desc} file download failed.  Server returned error {$rc}.\n"), 3, $snort_rules_upd_log);
		if ($pkg_interface == "console")
			error_log(gettext("\tThe error text was: {$last_curl_error}\n"), 3, $snort_rules_upd_log);
		error_log(gettext("\t{$desc} will not be updated.\n"), 3, $snort_rules_upd_log);
		return false;
	}

}

/* Start of main code */
conf_mount_rw();

/*  remove old $tmpfname files */
if (is_dir("{$tmpfname}"))
	exec("/bin/rm -r {$tmpfname}");

/*  Make sure required snortdirs exsist */
exec("/bin/mkdir -p {$snortdir}/rules");
exec("/bin/mkdir -p {$snortdir}/signatures");
exec("/bin/mkdir -p {$snortdir}/preproc_rules");
exec("/bin/mkdir -p {$tmpfname}");
exec("/bin/mkdir -p {$snortlibdir}/dynamicrules");
exec("/bin/mkdir -p {$snortlogdir}");

/* See if we need to automatically clear the Update Log based on 1024K size limit */
if (file_exists($snort_rules_upd_log)) {
	if (1048576 < filesize($snort_rules_upd_log))
		exec("/bin/rm -r {$snort_rules_upd_log}");
}

/* Log start time for this rules update */
error_log(gettext("Starting rules update...  Time: " . date("Y-m-d H:i:s") . "\n"), 3, $snort_rules_upd_log);
$last_curl_error = "";

/*  Check for and download any new Snort VRT sigs */
if ($snortdownload == 'on') {
	if (snort_check_rule_md5("{$snort_rule_url}{$snort_filename_md5}/{$oinkid}/", "{$tmpfname}/{$snort_filename_md5}", "Snort VRT rules")) {
		/* download snortrules file */
		$file_md5 = trim(file_get_contents("{$tmpfname}/{$snort_filename_md5}"));
		if (!snort_fetch_new_rules("{$snort_rule_url}{$snort_filename}/{$oinkid}/", "{$tmpfname}/{$snort_filename}", $file_md5, "Snort VRT rules"))
			$snortdownload = 'off';
	}
	else
		$snortdownload = 'off';
}

/*  Check for and download any new Snort GPLv2 Community Rules sigs */
if ($snortcommunityrules == 'on') {
	if (snort_check_rule_md5("{$snort_community_rules_url}{$snort_community_rules_filename_md5}", "{$tmpfname}/{$snort_community_rules_filename_md5}", "Snort GPLv2 Community Rules")) {
		/* download Snort GPLv2 Community Rules file */
		$file_md5 = trim(file_get_contents("{$tmpfname}/{$snort_community_rules_filename_md5}"));
		if (!snort_fetch_new_rules("{$snort_community_rules_url}{$snort_community_rules_filename}", "{$tmpfname}/{$snort_community_rules_filename}", $file_md5, "Snort GPLv2 Community Rules"))
			$snortcommunityrules = 'off';
	}
	else
		$snortcommunityrules = 'off';
}

/*  Check for and download any new Emerging Threats Rules sigs */
if ($emergingthreats == 'on') {
	if (snort_check_rule_md5("{$emergingthreats_url}{$emergingthreats_filename_md5}", "{$tmpfname}/{$emergingthreats_filename_md5}", "{$et_name} rules")) {
		/* download Emerging Threats rules file */
		$file_md5 = trim(file_get_contents("{$tmpfname}/{$emergingthreats_filename_md5}"));
		if (!snort_fetch_new_rules("{$emergingthreats_url}{$emergingthreats_filename}", "{$tmpfname}/{$emergingthreats_filename}", $file_md5, "{$et_name} rules"))
			$emergingthreats = 'off';
	}
	else
		$emergingthreats = 'off';
}

/* Untar Snort GPLv2 Community rules file to tmp */
if ($snortcommunityrules == 'on') {
	safe_mkdir("{$snortdir}/tmp/community");
	if (file_exists("{$tmpfname}/{$snort_community_rules_filename}")) {
		if ($pkg_interface <> "console") {
			update_status(gettext("Extracting Snort GPLv2 Community Rules..."));
			update_output_window(gettext("Installing Snort GPLv2 Community Rules..."));
		}
		error_log(gettext("\tExtracting and installing Snort GPLv2 Community Rules...\n"), 3, $snort_rules_upd_log);
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_community_rules_filename} -C {$snortdir}/tmp/community/");

		$files = glob("{$snortdir}/tmp/community/community-rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/" . GPL_FILE_PREFIX . "{$newfile}");
		}
                /* base etc files for Snort GPLv2 Community rules */
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/community/community-rules/{$file}"))
				@copy("{$snortdir}/tmp/community/community-rules/{$file}", "{$snortdir}/tmp/" . GPL_FILE_PREFIX . "{$file}");
		}
		/*  Copy snort community md5 sig to snort dir */
		if (file_exists("{$tmpfname}/{$snort_community_rules_filename_md5}")) {
			if ($pkg_interface <> "console")
				update_status(gettext("Copying md5 signature to snort directory..."));
			@copy("{$tmpfname}/{$snort_community_rules_filename_md5}", "{$snortdir}/{$snort_community_rules_filename_md5}");
		}
		if ($pkg_interface <> "console") {
			update_status(gettext("Extraction of Snort GPLv2 Community Rules completed..."));
			update_output_window(gettext("Installation of Snort GPLv2 Community Rules file completed..."));
		}
		error_log(gettext("\tInstallation of Snort GPLv2 Community Rules completed.\n"), 3, $snort_rules_upd_log);
		exec("rm -r {$snortdir}/tmp/community");
	}
}

/* Untar Emerging Threats rules file to tmp */
if ($emergingthreats == 'on') {
	safe_mkdir("{$snortdir}/tmp/emerging");
	if (file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
		if ($pkg_interface <> "console") {
			update_status(gettext("Extracting {$et_name} rules..."));
			update_output_window(gettext("Installing {$et_name} rules..."));
		}
		error_log(gettext("\tExtracting and installing {$et_name} rules...\n"), 3, $snort_rules_upd_log);
		exec("/usr/bin/tar xzf {$tmpfname}/{$emergingthreats_filename} -C {$snortdir}/tmp/emerging rules/");

		/* Remove the old Emerging Threats rules files */
		$eto_prefix = ET_OPEN_FILE_PREFIX;
		$etpro_prefix = ET_PRO_FILE_PREFIX;
		array_map('unlink', glob("{$snortdir}/rules/{$eto_prefix}*.rules"));
		array_map('unlink', glob("{$snortdir}/rules/{$etpro_prefix}*.rules"));
		array_map('unlink', glob("{$snortdir}/rules/{$eto_prefix}*ips.txt"));
		array_map('unlink', glob("{$snortdir}/rules/{$etpro_prefix}*ips.txt"));

		$files = glob("{$snortdir}/tmp/emerging/rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			if ($etpro == "on")
				@copy($file, "{$snortdir}/rules/" . ET_PRO_FILE_PREFIX . "{$newfile}");
			else
				@copy($file, "{$snortdir}/rules/{$newfile}");
		}
		/* IP lists for Emerging Threats rules */
		$files = glob("{$snortdir}/tmp/emerging/rules/*ips.txt");
		foreach ($files as $file) {
			$newfile = basename($file);
			if ($etpro == "on")
				@copy($file, "{$snortdir}/rules/" . ET_PRO_FILE_PREFIX . "{$newfile}");
			else
				@copy($file, "{$snortdir}/rules/" . ET_OPEN_FILE_PREFIX . "{$newfile}");
		}
                /* base etc files for Emerging Threats rules */
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/emerging/rules/{$file}"))
				@copy("{$snortdir}/tmp/emerging/rules/{$file}", "{$snortdir}/tmp/ET_{$file}");
		}

		/*  Copy emergingthreats md5 sig to snort dir */
		if (file_exists("{$tmpfname}/{$emergingthreats_filename_md5}")) {
			if ($pkg_interface <> "console")
				update_status(gettext("Copying md5 signature to snort directory..."));
			@copy("{$tmpfname}/{$emergingthreats_filename_md5}", "{$snortdir}/{$emergingthreats_filename_md5}");
		}
		if ($pkg_interface <> "console") {
			update_status(gettext("Extraction of {$et_name} rules completed..."));
			update_output_window(gettext("Installation of {$et_name} rules completed..."));
		}
		error_log(gettext("\tInstallation of {$et_name} rules completed.\n"), 3, $snort_rules_upd_log);
		exec("rm -r {$snortdir}/tmp/emerging");
	}
}

/* Untar Snort rules file to tmp */
if ($snortdownload == 'on') {
	if (file_exists("{$tmpfname}/{$snort_filename}")) {
		/* Currently, only FreeBSD-8-1 and FreeBSD-9-0 precompiled SO rules exist from Snort.org */
		/* Default to FreeBSD 8.1, and then test for FreeBSD 9.x */
		$freebsd_version_so = 'FreeBSD-8-1';
		if (substr(php_uname("r"), 0, 1) == '9')
			$freebsd_version_so = 'FreeBSD-9-0';

		/* Remove the old Snort rules files */
		$vrt_prefix = VRT_FILE_PREFIX;
		array_map('unlink', glob("{$snortdir}/rules/{$vrt_prefix}*.rules"));

		if ($pkg_interface <> "console") {
			update_status(gettext("Extracting Snort VRT rules..."));
			update_output_window(gettext("Installing Sourcefire VRT rules..."));
		}
		error_log(gettext("\tExtracting and installing Snort VRT rules...\n"), 3, $snort_rules_upd_log);
		/* extract snort.org rules and add prefix to all snort.org files */
		safe_mkdir("{$snortdir}/tmp/snortrules");
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp/snortrules rules/");
		$files = glob("{$snortdir}/tmp/snortrules/rules/*.rules");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/" . VRT_FILE_PREFIX . "{$newfile}");
		}
		/* IP lists */
		$files = glob("{$snortdir}/tmp/snortrules/rules/*.txt");
		foreach ($files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/rules/{$newfile}");
		}
		exec("rm -r {$snortdir}/tmp/snortrules");
		/* extract so rules */
		if ($pkg_interface <> "console") {
			update_status(gettext("Extracting Snort VRT Shared Objects rules..."));
			update_output_window(gettext("Installing precompiled Shared Objects rules for {$freebsd_version_so}..."));
		}
		exec("/bin/mkdir -p {$snortlibdir}/dynamicrules/");
		error_log(gettext("\tUsing Snort VRT precompiled SO rules for {$freebsd_version_so} ...\n"), 3, $snort_rules_upd_log);
		$snort_arch = php_uname("m");
		$nosorules = false;
		if ($snort_arch  == 'i386'){
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/precompiled/{$freebsd_version_so}/i386/{$snort_version}/");
			exec("/bin/cp {$snortdir}/tmp/so_rules/precompiled/{$freebsd_version_so}/i386/{$snort_version}/*.so {$snortlibdir}/dynamicrules/");
		} elseif ($snort_arch == 'amd64') {
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp so_rules/precompiled/{$freebsd_version_so}/x86-64/{$snort_version}/");
			exec("/bin/cp {$snortdir}/tmp/so_rules/precompiled/{$freebsd_version_so}/x86-64/{$snort_version}/*.so {$snortlibdir}/dynamicrules/");
		} else
			$nosorules = true;
		exec("rm -r {$snortdir}/tmp/so_rules");
		if ($nosorules == false) {
			/* extract so stub rules, rename and copy to the rules folder. */
			if ($pkg_interface <> "console")
			        update_status(gettext("Copying Snort VRT Shared Objects rules..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp --exclude precompiled/ --exclude src/ so_rules/");
			$files = glob("{$snortdir}/tmp/so_rules/*.rules");
			foreach ($files as $file) {
				$newfile = basename($file, ".rules");
				@copy($file, "{$snortdir}/rules/" . VRT_FILE_PREFIX . "{$newfile}.so.rules");
			}
			exec("rm -r {$snortdir}/tmp/so_rules");
		}
		/* extract base etc files */
		if ($pkg_interface <> "console") {
		        update_status(gettext("Extracting Snort VRT config and map files..."));
			update_output_window(gettext("Copying config and map files..."));
		}
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp etc/");
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/etc/{$file}"))
				@copy("{$snortdir}/tmp/etc/{$file}", "{$snortdir}/tmp/VRT_{$file}");
		}
		exec("rm -r {$snortdir}/tmp/etc");
		/* Untar snort signatures */
		$signature_info_chk = $config['installedpackages']['snortglobal']['signatureinfo'];
		if ($premium_url_chk == 'on') {
			if ($pkg_interface <> "console")
				update_status(gettext("Extracting Snort VRT Signatures..."));
			exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} doc/signatures/");
			if ($pkg_interface <> "console")
				update_status(gettext("Done extracting Signatures."));

			if (is_dir("{$snortdir}/doc/signatures")) {
				if ($pkg_interface <> "console")
					update_status(gettext("Copying Snort VRT signatures..."));
				exec("/bin/cp -r {$snortdir}/doc/signatures {$snortdir}/signatures");
				if ($pkg_interface <> "console")
					update_status(gettext("Done copying signatures."));
			}
		}
		/* Extract the Snort preprocessor rules */
		if ($pkg_interface <> "console")
			update_output_window(gettext("Extracting preprocessor rules files..."));
		exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir}/tmp preproc_rules/");

		if (file_exists("{$tmpfname}/{$snort_filename_md5}")) {
			if ($pkg_interface <> "console")
				update_status(gettext("Copying md5 signature to snort directory..."));
			@copy("{$tmpfname}/{$snort_filename_md5}", "{$snortdir}/{$snort_filename_md5}");
		}
		if ($pkg_interface <> "console") {
			update_status(gettext("Extraction of Snort VRT rules completed..."));
			update_output_window(gettext("Installation of Sourcefire VRT rules completed..."));
		}
		error_log(gettext("\tInstallation of Snort VRT rules completed.\n"), 3, $snort_rules_upd_log);
	}
}

function snort_apply_customizations($snortcfg, $if_real) {

	global $vrt_enabled;
	$snortdir = SNORTDIR;

	/* Update the Preprocessor rules for the master configuration and for the interface if Snort VRT rules are in use. */
	if ($vrt_enabled == 'on') {
		exec("/bin/mkdir -p {$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/preproc_rules");
		$preproc_files = glob("{$snortdir}/tmp/preproc_rules/*.rules");
		foreach ($preproc_files as $file) {
			$newfile = basename($file);
			@copy($file, "{$snortdir}/preproc_rules/{$newfile}"); 
			/* Check if customized preprocessor rule protection is enabled for interface before overwriting them. */
			if ($snortcfg['protect_preproc_rules'] <> 'on')
				@copy($file, "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/preproc_rules/{$newfile}");
		}
	}
	else {
		exec("/bin/mkdir -p {$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/preproc_rules");
	}

	snort_prepare_rule_files($snortcfg, "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}");

	/* Copy the master config and map files to the interface directory */
	@copy("{$snortdir}/classification.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/classification.config");
	@copy("{$snortdir}/gen-msg.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/gen-msg.map");
	@copy("{$snortdir}/reference.config", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/reference.config");
	@copy("{$snortdir}/unicode.map", "{$snortdir}/snort_{$snortcfg['uuid']}_{$if_real}/unicode.map");
}

if ($snortdownload == 'on' || $emergingthreats == 'on' || $snortcommunityrules == 'on') {

	if ($pkg_interface <> "console")
		update_status(gettext('Copying new config and map files...'));
	error_log(gettext("\tCopying new config and map files...\n"), 3, $snort_rules_upd_log);

        /* Determine which config and map file set to use for the master copy. */
        /* If the Snort VRT rules are not enabled, then use Emerging Threats.  */
        if (($vrt_enabled == 'off') && ($et_enabled == 'on')) {
		$cfgs = glob("{$snortdir}/tmp/*reference.config");
		$cfgs[] = "{$snortdir}/reference.config";
		snort_merge_reference_configs($cfgs, "{$snortdir}/reference.config");
		$cfgs = glob("{$snortdir}/tmp/*classification.config");
		$cfgs[] = "{$snortdir}/classification.config";
		snort_merge_classification_configs($cfgs, "{$snortdir}/classification.config");
		/* Use the unicode.map and gen-msg.map files from ET rules. */
		if (file_exists("{$snortdir}/tmp/ET_unicode.map"))
			@copy("{$snortdir}/tmp/ET_unicode.map", "{$snortdir}/unicode.map");
		if (file_exists("{$snortdir}/tmp/ET_gen-msg.map"))
			@copy("{$snortdir}/tmp/ET_gen-msg.map", "{$snortdir}/gen-msg.map");
        }
        elseif (($vrt_enabled == 'on') && ($et_enabled == 'off')) {
		foreach (array("classification.config", "reference.config", "gen-msg.map", "unicode.map") as $file) {
			if (file_exists("{$snortdir}/tmp/VRT_{$file}"))
				@copy("{$snortdir}/tmp/VRT_{$file}", "{$snortdir}/{$file}");
                }
        }
        elseif (($vrt_enabled == 'on') && ($et_enabled == 'on')) {
		/* Both VRT and ET rules are enabled, so build combined  */
		/* reference.config and classification.config files.     */
		$cfgs = glob("{$snortdir}/tmp/*reference.config");
		$cfgs[] = "{$snortdir}/reference.config";
		snort_merge_reference_configs($cfgs, "{$snortdir}/reference.config");
		$cfgs = glob("{$snortdir}/tmp/*classification.config");
		$cfgs[] = "{$snortdir}/classification.config";
		snort_merge_classification_configs($cfgs, "{$snortdir}/classification.config");
		/* Use the unicode.map and gen-msg.map files from VRT rules. */
		if (file_exists("{$snortdir}/tmp/VRT_unicode.map"))
			@copy("{$snortdir}/tmp/VRT_unicode.map", "{$snortdir}/unicode.map");
		if (file_exists("{$snortdir}/tmp/VRT_gen-msg.map"))
			@copy("{$snortdir}/tmp/VRT_gen-msg.map", "{$snortdir}/gen-msg.map");
        }

	/* Start the rules rebuild proccess for each configured interface */
	if (is_array($config['installedpackages']['snortglobal']['rule'])) {

		/* Set the flag to force rule rebuilds since we downloaded new rules,    */
		/* except when in post-install mode.  Post-install does its own rebuild. */
		if ($g['snort_postinstall'])
			$rebuild_rules = false;
		else
			$rebuild_rules = true;

		/* Create configuration for each active Snort interface */
		foreach ($config['installedpackages']['snortglobal']['rule'] as $id => $value) {
			$if_real = snort_get_real_interface($value['interface']);
			$tmp = "Updating rules configuration for: " . snort_get_friendly_interface($value['interface']) . " ...";
			if ($pkg_interface <> "console"){
				update_status(gettext($tmp));
				update_output_window(gettext("Please wait while Snort interface files are being updated..."));
			}
			snort_apply_customizations($value, $if_real);

			/*  Log a message in Update Log if protecting customized preprocessor rules. */
			$tmp = "\t" . $tmp . "\n";
			if ($value['protect_preproc_rules'] == 'on') {
				$tmp .= gettext("\tPreprocessor text rules flagged as protected and not updated for ");
				$tmp .= snort_get_friendly_interface($value['interface']) . "...\n";
			}
			error_log($tmp, 3, $snort_rules_upd_log);
		}
	}
	else {
		if ($pkg_interface <> "console") {
		        update_output_window(gettext("Warning:  No interfaces configured for Snort were found..."));
			update_output_window(gettext("No interfaces currently have Snort configured and enabled on them..."));
		}
		error_log(gettext("\tWarning:  No interfaces configured for Snort were found...\n"), 3, $snort_rules_upd_log);
	}

	/* Clear the rebuild rules flag.  */
	$rebuild_rules = false;

	/*  remove old $tmpfname files */
	if (is_dir("{$snortdir}/tmp")) {
		if ($pkg_interface <> "console")
			update_status(gettext("Cleaning up after rules extraction..."));
		exec("/bin/rm -r {$snortdir}/tmp");
	}

	/* Restart snort if already running and we are not rebooting to pick up the new rules. */
       	if (is_process_running("snort") && !$g['booting']) {
		if ($pkg_interface <> "console") {
			update_status(gettext('Restarting Snort to activate the new set of rules...'));
			update_output_window(gettext("Please wait ... restarting Snort will take some time..."));
		}
		error_log(gettext("\tRestarting Snort to activate the new set of rules...\n"), 3, $snort_rules_upd_log);
       		restart_service("snort");
		if ($pkg_interface <> "console")
		        update_output_window(gettext("Snort has restarted with your new set of rules..."));
       		log_error(gettext("[Snort] Snort has restarted with your new set of rules..."));
		error_log(gettext("\tSnort has restarted with your new set of rules.\n"), 3, $snort_rules_upd_log);
	}
	else {
		if ($pkg_interface <> "console")
			update_output_window(gettext("The rules update task is complete..."));
	}
}

if ($pkg_interface <> "console")
	update_status(gettext("The Rules update has finished..."));
log_error(gettext("[Snort] The Rules update has finished."));
error_log(gettext("The Rules update has finished.  Time: " . date("Y-m-d H:i:s"). "\n\n"), 3, $snort_rules_upd_log);
conf_mount_ro();

/* Restore the state of $pkg_interface */
$pkg_interface = $pkg_interface_orig;

?>
