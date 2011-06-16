#!/usr/local/bin/php
<?php

/*
 * WARNING: THIS FILE SHOULD NEVER BE IN WWWW DIR
 * 
 */

// fetch db Settings NONE Json
function snortSql_fetchAllSettings($dbname, $table, $type, $id_uuid) 
{

	if ($dbname == '' || $table == '' || $type == ''){
		return false;
	}
	  
	if ($dbname === 'snortDB') {
		$db = sqlite_open("/usr/local/pkg/snort/$dbname");
	}
	
	if ($dbname === 'snortDBtemp') {
		$db = sqlite_open("/var/snort/$dbname");
	}	
	  
	if ($type == 'id'){   
		$result = sqlite_query($db,
		"SELECT * FROM {$table} where id = '{$id_uuid}';
		");
	}

	if ($type == 'uuid'){   
		$result = sqlite_query($db,
		"SELECT * FROM {$table} where uuid = '{$id_uuid}';
		");
	}

	if ($type == 'filename'){   
		$result = sqlite_query($db,
		"SELECT * FROM {$table} where filename = '{$id_uuid}';
		");
	}	
	
	
	if ($type == 'id' || $type == 'uuid' || $type == 'filename'){
		$chktable = sqlite_fetch_array($result, SQLITE_ASSOC);
	}
		
	sqlite_close($db);
	  
	return $chktable;
  
  
} // end func

function snortCmpareMD5($type, $path1, $path2, $filename_md5)
{
	update_output_window2('ms2', 'Checking ' . $filename_md5 . ' MD5...');
	
	if (file_exists("{$path1}/{$filename_md5}")){
		
		if ($type == 'string'){
			$md5_check_new = @file_get_contents("{$path1}/{$filename_md5}");
			$md5_check_old = @file_get_contents("{$path2}/{$filename_md5}");
			if ($md5_check_new !== $md5_check_old){
				update_output_window2('ms2', "$filename_md5 MD5s do not match...");
				return false;
			}
		}
		
		if ($type == 'md5'){		
			//md5 snortrules-snapshot-2905.tar.gz | awk '{print $4}'
			$md5_check_new2 = exec("/sbin/md5 {$path1}/{$filename_md5} | /usr/bin/awk '{print $4}'");
			$md5_check_old2 = exec("/sbin/md5 {$path2}/{$filename_md5} | /usr/bin/awk '{print $4}'");
			if ($md5_check_new != $md5_check_old){
				update_output_window2('ms2', "$filename_md5 MD5s do not match...");
				return false;
			}
		}
		
		if ($type == 'md5FileChk') {		
				//md5 snortrules-snapshot-2905.tar.gz | awk '{print $4}'
				$md5_check_new = trim(exec("/sbin/md5 {$path1}/{$filename_md5} | /usr/bin/awk '{print $4}'"));
				
				$md5_check_old = exec("/bin/cat {$path1}/{$filename_md5}.md5");
				
				$md5_check_old2 = trim(preg_replace('/"/', '', $md5_check_old));
				
				if ($md5_check_new != $md5_check_old2){
					update_output_window2('ms2', "$filename_md5 MD5s do not match...");
					return false;
				}
		}		
		
		
		
	}
		
		update_output_window2('ms2', "$filename_md5 MD5 File Check Passed...");		 
		return true;
}


/*
 *   update_output_window: update bottom textarea dynamically.
 */
function update_output_window2($type, $text)
{

	if ($GLOBALS['tmp']['snort']['downloadupdate']['console'] != 'on'){		
		snortSql_updateRuleSetList($type, $text, '', '', $GLOBALS['tmp']['snort']['downloadupdate']['workingfile']); // write out msg to db	
	}else{
		echo "\n" . $type . ': ' . $text;
	}
	
}

function snortSql_updateRuleSetList($type, $value, $file_size, $downloaded, $filename)
{	

	$dbname = 'snortDBtemp';
	$table = 'SnortDownloads';
	$addDate = date(U);
	
	// do let user pick the DB path
    $db = sqlite_open("/var/snort/{$dbname}");

	if ($type === 'percent2'){
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE {$table} SET date = '{$addDate}', percent = '{$value}', filesize = '{$file_size}', downloaded = '{$downloaded}' where filename = '{$filename}';
		");
	}    
    
    
	if ($type === 'percent'){
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE {$table} SET date = '{$addDate}', percent = '{$value}' where filename = '{$filename}';
		");
	}

	if ($type === 'msg1'){
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE SnortDownloadsMsg SET date = '{$addDate}', msg = '{$value}' where id = '1';
		");
	}

	if ($type === 'msg2'){
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE SnortDownloadsMsg SET date = '{$addDate}', msg = '{$value}' where id = '2';
		");
	}

	/*
	 * INPORTANT:
	 * Register worker to prevent loops and ghost process
	 * Needs to be watched, 
	 */
	
	if ($type === 'working'){
		
		$getmypid = getmypid();
		$getmyfilename = $_SERVER['SCRIPT_NAME'];		
		
	    $resultChk = sqlite_query($db,
	    "SELECT * FROM RegisterWorker WHERE uuid = 'jdjEf!773&h3bhFd6A';
	    ");
	
	    $resultChkFinal = sqlite_fetch_all($resultChk, SQLITE_ASSOC);		

	    if (!empty($resultChkFinal)) {	    
			$query_ck = sqlite_query($db, // @ supress warnings usonly in production
			"UPDATE RegisterWorker SET date = '{$addDate}', processid = '{$getmypid}', filename = '{$getmyfilename}', working = '{$value}' where uuid = 'jdjEf!773&h3bhFd6A';
			");		
	    }else{	
			$query_ck = sqlite_query($db, // @ supress warnings usonly in production
			"INSERT INTO RegisterWorker (date, processid, filename, working, uuid) VALUES ('{$addDate}', '{$getmypid}', '{$getmyfilename}', '{$value}', 'jdjEf!773&h3bhFd6A');
			");	
	    }		
	}

	
	if ($type === 'snortWait'){
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE {$table} SET waittime = '{$addDate}' where filename = '{$filename}';
		");
	}	
	
      if (sqlite_changes($db) < 1){
        sqlite_close($db);
        return 'Error in query';   
      }  
	    
	sqlite_close($db);
    
    
}	

// returns array that matches pattern, option to replace objects in matches
function snortScanDirFilter($arrayList, $pattmatch, $pattreplace, $pattreplacewith)
{
	foreach ( $arrayList as $val )
	{
		if (preg_match($pattmatch, $val, $matches)) {
			if ($pattreplace != '') {
				$matches2 = preg_replace($pattreplace, $pattreplacewith, $matches[0]);
				$filterDirList[] = $matches2;
			}else{
				$filterDirList[] = $matches[0];
			}
		}
	}	
	return $filterDirList;
}

// set page vars	
$generalSettings = snortSql_fetchAllSettings('snortDB', 'SnortSettings', 'id', '1');	

// Setup file names and dir
$tmpfname = '/usr/local/etc/snort/snort_download';
$snortdir = '/usr/local/etc/snort';
$snortdir_rules = '/usr/local/etc/snort/snort_rules';
$emergingdir_rules = '/usr/local/etc/snort/emerging_rules';
$pfsensedir_rules = '/usr/local/etc/snort/pfsense_rules';
$customdir_rules = '/usr/local/etc/snort/custom_rules';
$snort_filename_md5 = 'snortrules-snapshot-2905.tar.gz.md5';
$snort_filename = 'snortrules-snapshot-2905.tar.gz';
$emergingthreats_filename_md5 = 'emerging.rules.tar.gz.md5';
$emergingthreats_filename = 'emerging.rules.tar.gz';
$pfsense_rules_filename_md5 = 'pfsense_rules.tar.gz.md5';
$pfsense_rules_filename = 'pfsense_rules.tar.gz';

// START of MAIN function
function sendUpdateSnortLogDownload($console)
{
	
	if ($console === 'console'){
		$GLOBALS['tmp']['snort']['downloadupdate']['console'] = 'on';	
	}
	
	//bring in the global vars 
	global $generalSettings, $tmpfname, $snortdir, $snortdir_rules, $emergingdir_rules, $pfsensedir_rules, $customdir_rules, $snort_filename_md5, $snort_filename, $emergingthreats_filename_md5, $emergingthreats_filename, $pfsense_rules_filename_md5, $pfsense_rules_filename;
	
	/*  Make shure snortdir exits */
	if (!file_exists("{$snortdir}")) {
		exec("/bin/mkdir -p {$snortdir}");
	}
	if (!file_exists("{$tmpfname}")) {
		exec("/bin/mkdir -p {$tmpfname}");
	}
	if (!file_exists("{$snortdir_rules}")) {
		exec("/bin/mkdir -p {$snortdir_rules}");
	}
	if (!file_exists("{$emergingdir_rules}")) {
		exec("/bin/mkdir -p {$emergingdir_rules}");
	}
	if (!file_exists("{$pfsensedir_rules}")) {
		exec("/bin/mkdir -p {$pfsensedir_rules}");
	}
	if (!file_exists("{$customdir_rules}")) {
		exec("/bin/mkdir -p {$customdir_rules}");
	}
	if (!file_exists("{$snortdir}/signatures")) {
	exec("/bin/mkdir -p {$snortdir}/signatures");
	}
	if (!file_exists('/usr/local/lib/snort/dynamicrules/')) {
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
	}
	
	
	/* Set user agent to Mozilla */
	ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
	ini_set("memory_limit","150M");
	
	
	// Get file that does not use redirects, mostly for none snort.org downloads
	function snort_file_get_contents($tmpfname, $snort_filename, $snort_UrlGet)
	{
		if (!file_exists("{$tmpfname}/{$snort_filename}") || filesize("{$tmpfname}/{$snort_filename}") <= 0){
			update_output_window2('ms2', 'Downloading ' . $snort_filename.  ' MD5...');
			ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
			$file = file_get_contents("$snort_UrlGet/{$snort_filename}"); // use a @ infront of file_get_contents when in production
			$f = fopen("{$tmpfname}/{$snort_filename}", 'w');
			fwrite($f, $file);
			fclose($f);
			update_output_window2('ms2', 'Finnished Downloading ' . $snort_filename.  ' MD5...');
		}	
	}
	
	function read_header2($ch, $string) {
	        global $file_size, $fout;
	        $length = strlen($string);
	        $regs = "";
	        ereg("(Content-Length:) (.*)", $string, $regs);
	        if($regs[2] <> "") {
	                $file_size = intval($regs[2]);
	        }
	        ob_flush();
	        return $length;
	}
	
	function read_body2($ch, $string) {
	        global $fout, $file_size, $downloaded, $sendto, $static_status, $static_output, $lastseen;
	                global $pkg_interface;
	        $length = strlen($string);
	        $downloaded += intval($length);
	        if($file_size > 0) {
	                $downloadProgress = round(100 * (1 - $downloaded / $file_size), 0);
	                $downloadProgress = 100 - $downloadProgress;
	        } else
	                $downloadProgress = 0;
	        if($lastseen <> $downloadProgress and $downloadProgress < 101) {
	                if($sendto == "status") {
	                                        if($pkg_interface == "console") {
	                                                if(substr($downloadProgress,2,1) == "0" || count($downloadProgress) < 2) {
	                                $tostatus = $static_status . $downloadProgress . "%";
	                                update_status($tostatus);
	                                                }
	                                        } else {
	                        $tostatus = $static_status . $downloadProgress . "%";
	                        update_status($tostatus);
	                                        }
	                } else {
	                                        if($pkg_interface == "console") {
	                                                if(substr($downloadProgress,2,1) == "0" || count($downloadProgress) < 2) {
	                                $tooutput = $static_output . $downloadProgress . "%";
	                                update_output_window($tooutput);
	                                                }
	                                        } else {
	                        $tooutput = $static_output . $downloadProgress . "%";
	                        update_output_window($tooutput);
	                                        }
	                }
	                update_progress_bar($downloadProgress);
	                $lastseen = $downloadProgress;
	        }
	        if($fout)
	                fwrite($fout, $string);
	        ob_flush();
	        return $length;
	}
	
	/*
	 * update_progress_bar($percent): updates the javascript driven progress bar.
	 */
	function update_progress_bar2($percent, $file_size, $downloaded)
	{
	        if($percent > 100) $percent = 1;
	        
	        if ($GLOBALS['tmp']['snort']['downloadupdate']['console'] != 'on') {
	        	snortSql_updateRuleSetList('percent2', $percent, $file_size, $downloaded, $GLOBALS['tmp']['snort']['downloadupdate']['workingfile']); // write out percent to db
			}else{
				echo "\n" . 'percent: ' . $percent . ' filesize: ' . $file_size . ' downloaded: ' . $downloaded;
			}
	}
	
	
	function read_body_firmware($ch, $string) 
	{
		global $fout, $file_size, $downloaded, $counter;
		$length = strlen($string);
		$downloaded += intval($length);
		$downloadProgress = round(100 * (1 - $downloaded / $file_size), 0);
		$downloadProgress = 100 - $downloadProgress;
		$counter++;
		if($counter > 150) {
			update_progress_bar2($downloadProgress, $file_size, $downloaded);
			flush();
			$counter = 0;
		}
		fwrite($fout, $string);
		return $length;
	}
	
	function download_file_with_progress_bar2($url_file, $destination, $workingfile, $readbody = 'read_body2') 
	{
		global $ch, $fout, $file_size, $downloaded;
		$file_size  = 1;
		$downloaded = 1;
		$destination_file = $destination . '/' . $workingfile;
		
		/* open destination file */
		$fout = fopen($destination_file, "wb");
	
		/*
		 *      Originally by Author: Keyvan Minoukadeh
		 *      Modified by Scott Ullrich to return Content-Length size
		 */
	
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $url_file);
		curl_setopt($ch, CURLOPT_HEADERFUNCTION, 'read_header2');
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		curl_setopt($ch, CURLOPT_WRITEFUNCTION, $readbody);
		curl_setopt($ch, CURLOPT_NOPROGRESS, '1');
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, '5');
		curl_setopt($ch, CURLOPT_TIMEOUT, 0);
	
		curl_exec($ch);
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		if($fout)
		fclose($fout);
		curl_close($ch);
		return ($http_code == 200) ? true : $http_code;
	}
	
// ----------------------------------------------------- Begin Code	--------------------------------------------	

	
		// rm all tmp filea
		exec("/bin/rm -r $tmpfname/*");

		// Set all downloads to be false, download by default
		
		$snort_md5_check_ok = false;
		$emerg_md5_check_ok = false;
		$pfsense_md5_check_ok = false;
		
		// define checks
		$oinkid = $generalSettings['oinkmastercode'];
		
		$emergingthreatscode = $generalSettings['emergingthreatscode'];

		// dsable downloads if there settings are off
		if 	($generalSettings['snortdownload'] === 'off') {
			$snort_md5_check_ok = true;
		}
		
		if 	($generalSettings['emergingthreatsdownload'] == 'off') {
			$emerg_md5_check_ok = true;
		}				

		if ($oinkid == '' && $generalSettings['snortdownload'] === 'off') {
			update_output_window2('ms1', 'You must obtain an oinkid from snort.org and set its value in the Snort settings tab.');
			exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'You must obtain an oinkid from snort.org and set its value in the Snort settings tab.'");
			return false;
		}
		
		if ($emergingthreatscode == '' && $generalSettings['snortdownload'] === 'pro') {
			update_output_window2('ms1', 'You must obtain an emergingthreat pro id from emergingthreatspro.com and set its value in the Snort settings tab.');
			exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'You must obtain an emergingthreat pro id from emergingthreatspro.com and set its value in the Snort settings tab.'");
			return false;
		}		
		
		if ($generalSettings['snortdownload'] === 'off' && $generalSettings['emergingthreatsdownload'] === 'off') { // note: basic and pro
			update_output_window2('ms1', 'SnortStartup: No rules have been selected to download.');
			exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'No rules have been selected to download.'");
			return false;
		}		
		
		/*
		 *  Check MD5s and MARK
		 * 
		 */
		
		update_output_window2('ms1', 'Starting MD5 checks...');		
		
		// check is we need to wait
		update_output_window2('ms2', 'Checking Wait Status for Snort.org...');
		$getSnort_filename_Waittime_chk = snortSql_fetchAllSettings('snortDBtemp', 'SnortDownloads', 'filename', $snort_filename);
		
		if (date(U) > $getSnort_filename_Waittime_chk['waittime'] + 900) {
			update_output_window2('ms2', 'Snort.org Wait Time Status: OK...');
		}else{
			update_output_window2('ms2', 'Snort.org Wait Time Status: Wait 15 min Please...');
			$snort_md5_check_ok = true;
			$snort_wait = true;
		}

		// check is we need to wait
		update_output_window2('ms2', 'Checking Wait Status for Emergingthreats.net...');
		$getEmergingthreats_filename_Waittime_chk = snortSql_fetchAllSettings('snortDBtemp', 'SnortDownloads', 'filename', $emergingthreats_filename);		

		if (date(U) > $getEmergingthreats_filename_Waittime_chk['waittime'] + 900) {
			update_output_window2('ms2', 'Emergingthreats.net Wait Time Status: OK...');
		}else{
			update_output_window2('ms2', 'Emergingthreats.net Wait Time Status: Wait 15 min Please...');
			$emerg_md5_check_ok = true;
			$emerg_wait = true;
		}
		
		// if all rules need wait stop
		if ($snort_wait === true && $emerg_wait === true) {			
			return false;
		}
		
		// download snort.org md5 and compare
		if ($snort_md5_check_ok === false) {
		
			snort_file_get_contents($tmpfname, $snort_filename_md5, 'http://www.snort.org/pub-bin/oinkmaster.cgi/' . $oinkid);
			
			// if snort.org md5 do not match
			if(snortCmpareMD5('string', $tmpfname, $snortdir_rules, $snort_filename_md5)) {
				$snort_md5_check_ok = true;
			}
		
		}		

		// download emergingthreats.net md5 and compare
		if ($emerg_md5_check_ok === false) {
		
			snort_file_get_contents($tmpfname, $emergingthreats_filename_md5, 'http://rules.emergingthreats.net/open/snort-2.9.0');
			
			// if emergingthreats.net md5 do not match
			if(snortCmpareMD5('string', $tmpfname, $emergingdir_rules, $emergingthreats_filename_md5)) {
				$emerg_md5_check_ok = true;
			}
		
		}
		
		// download pfsense.org md5 and compare
		snort_file_get_contents($tmpfname, $pfsense_rules_filename_md5, 'http://www.pfsense.com/packages/config/snort/pfsense_rules');
		
		// if pfsense.org md5 do not match
		if(snortCmpareMD5('string', $tmpfname, $pfsensedir_rules, $pfsense_rules_filename_md5)) {
			$pfsense_md5_check_ok = true;
		}
		
		/*
		 * If all rule type is not check clean up. 
		 */
				
		/*  Make Clean Snort Directory emergingthreats not checked */
		if ($snort_md5_check_ok === false && $emergingthreatsdownload === 'off') {
			update_output_window2('ms1', 'Cleaning the emergingthreats Directory...');
			exec("/bin/rm {$snortdir}/emerging_rules/*.rules");
			exec("/bin/rm {$snortdir}/version.txt");
			update_output_window2('ms2', 'Done cleaning emrg direcory.');
		}
		
		/*  Make Clean Snort Directory snort.org not checked */
		if ($emerg_md5_check_ok === false && $snortdownload !== 'on') {
			update_output_window2('ms1', 'Cleaning the snort Directory...');
			exec("/bin/rm {$snortdir}/snort_rules/*.rules");
			exec("/bin/rm {$snortdir}/snortrules-snapshot-2905.tar.gz.md5");
			update_output_window2('ms2', 'Done cleaning snort direcory.');
		}		
		
		
		/* Check if were up to date exits */
		if ($snort_md5_check_ok === true && $emerg_md5_check_ok === true && $pfsense_md5_check_ok === true) {
				update_output_window2('ms1', 'Your rules are up to date...');
				return false;
		}		
		
		
		/* You are Not Up to date, always stop snort when updating rules for low end machines */;
		update_output_window2('ms1', 'You are NOT up to date...');
		update_output_window2('ms2', 'Stopping Snort and Barnyard2 service...');
		$chk_if_snort_up = exec('pgrep -x snort');
		$chk_if_barnyad_up = exec('pgrep -x barnyad2');
		if ($chk_if_snort_up != '') {
			exec('/usr/bin/touch /tmp/snort_download_halt.pid'); // IMPORTANT: incase of script crash or error, Mabe use DB
			exec('/usr/bin/killall snort');
			if ($chk_if_barnyad_up != ''){
				exec('/usr/bin/killall barnyad2');
			}	
			sleep(2);
		}
		
		
		/* download snortrules file */
		if ($snort_md5_check_ok === false) {
		
					$GLOBALS['tmp']['snort']['downloadupdate']['workingfile'] = $snort_filename;
					update_output_window2('ms1', 'Snort.org: Starting Download...');
					download_file_with_progress_bar2("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", $tmpfname, $snort_filename, "read_body_firmware");
					//download_file_with_progress_bar2("http://theseusnetworking.com/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", $tmpfname, $snort_filename, "read_body_firmware");
					snortSql_updateRuleSetList('percent', '100', '', '', $snort_filename); // finsh percent	
					update_output_window2('ms1', 'Snort.org: Finished Download...');
					
					// if md5 does not match then the file is bad or snort.org says wait 15 min
					update_output_window2('ms1', 'Snort.org MD5 File Check ...');
					if (!snortCmpareMD5('md5FileChk', $tmpfname, '', $snort_filename)) {
						
						$snort_filename_wait_ck = exec("/usr/bin/egrep '\bYou must wait 15\b' {$tmpfname}/{$snort_filename}");						
						if ($snort_filename_wait_ck != '') {
							update_output_window2('ms2', 'Snort.org: You must wait 15 min...');
						}					
		
						// disable snort.org download
						$snort_md5_check_ok = true;
						$snort_filename_corrupted = true;
					
					}else{
						snortSql_updateRuleSetList('snortWait', '', '', '', $snort_filename); // Register Worker off
					}			
		}		
		
		/* download emergingthreats file */
		if ($emerg_md5_check_ok === false) {
		
					$GLOBALS['tmp']['snort']['downloadupdate']['workingfile'] = $emergingthreats_filename;
					update_output_window2('ms1', 'Emergingthreats.net: Starting Download...');
					download_file_with_progress_bar2("http://rules.emergingthreats.net/open/snort-2.9.0/{$emergingthreats_filename}", $tmpfname, $emergingthreats_filename, "read_body_firmware");
					snortSql_updateRuleSetList('percent', '100', '', '', $emergingthreats_filename); // finsh percent	
					update_output_window2('ms1', 'Emergingthreats.net: Finished Download...');
					
					// if md5 does not match then the file is bad or snort.org says wait 15 min
					update_output_window2('ms1', 'Emergingthreats MD5 File Check ...');
					if (!snortCmpareMD5('md5FileChk', $tmpfname, '', $emergingthreats_filename)) {				
		
						// disable snort.org download
						$emerg_md5_check_ok = true;
						$emerg_filename_corrupted = true;
					
					}else{
						snortSql_updateRuleSetList('snortWait', '', '', '', $emergingthreats_filename); // Register Worker off
					}			
		}
		
		/* download pfsense rule file */
		if ($pfsense_md5_check_ok === false) {
		
					$GLOBALS['tmp']['snort']['downloadupdate']['workingfile'] = $pfsense_rules_filename;
					update_output_window2('ms1', 'pfSense.org: Starting Download...');
					download_file_with_progress_bar2("http://www.pfsense.com/packages/config/snort/pfsense_rules/{$pfsense_rules_filename}", $tmpfname, $pfsense_rules_filename, "read_body_firmware");
					snortSql_updateRuleSetList('percent', '100', '', '', $pfsense_rules_filename); // finsh percent	
					update_output_window2('ms1', 'pfSense.org: Finished Download...');
					
					// if md5 does not match then the file is bad or snort.org says wait 15 min
					update_output_window2('ms1', 'pfSense.org MD5 File Check ...');
					if (!snortCmpareMD5('md5FileChk', $tmpfname, '', $pfsense_rules_filename)) {				
		
						// disable snort.org download
						$pfsense_md5_check_ok = true;
					
					}else{
						snortSql_updateRuleSetList('snortWait', '', '', '', $pfsense_rules_filename); // Register Worker off
					}			
		}			
		
		// if both files are corrupted stop
		if ($snort_filename_corrupted === true && $emerg_filename_corrupted === true) {
			update_output_window2('ms1', 'Snort.org and Emergingthreats.net files are corrupted.');
			update_output_window2('ms2', 'Stoping Script...');
			return false;
		}	
		
		/*
		 * START: Untar Files
		 */				

		// Untar snort rules file individually to help people with low system specs
		if ($snort_md5_check_ok === false && file_exists("{$tmpfname}/{$snort_filename}")) {						

				update_output_window2('ms1', 'Extracting Snort.org rules...');
				update_output_window2('ms2', 'May take a while...');
				
			function build_SnortRuleDir() 
			{
				global $tmpfname, $snortdir, $snortdir_rules, $snort_filename;
				
					// find out if were in 1.2.3-RELEASE
					$pfsense_ver_chk = exec('/bin/cat /etc/version');
					if ($pfsense_ver_chk === '1.2.3-RELEASE') {
						$pfsense_stable = 'yes';
					}else{
						$pfsense_stable = 'no';
					}	
			
					// get the system arch
					$snort_arch_ck = exec('/usr/bin/uname -m');
					if ($snort_arch_ck === 'i386') {
						$snort_arch = 'i386';
					}else{
						$snort_arch = 'x86-64'; // amd64
					}								
				
					if ($pfsense_stable === 'yes') {
						$freebsd_version_so = 'FreeBSD-7-3';
					}else{
						$freebsd_version_so = 'FreeBSD-8-1';
					}
					
					// extract snort.org rules and  add prefix to all snort.org files
					exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir_rules} rules/");
					
					$snort_dirList = scandir("{$snortdir_rules}/rules"); // Waning: only in php 5
					$snortrules_filterList = snortScanDirFilter($snort_dirList, '/.*\.rules/', '/\.rules/', '');
					
					if (!empty($snortrules_filterList)) {		
						foreach ($snortrules_filterList as $snort_rule_move)
						{	
							exec("/bin/mv -f {$snortdir_rules}/rules/{$snort_rule_move}.rules {$snortdir_rules}/rules/snort_{$snort_rule_move}.rules");	
						}
					}
					
					// extract so rules
					exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir_rules} so_rules/precompiled/$freebsd_version_so/$snort_arch/2.9.0.5/");
					exec("/bin/mv -f {$snortdir_rules}/so_rules/precompiled/$freebsd_version_so/$snort_arch/2.9.0.5/* /usr/local/lib/snort/dynamicrules/");
					
					// list so_rules and exclude dir
					exec("/usr/bin/tar --exclude='precompiled' --exclude='src' -tf {$tmpfname}/{$snort_filename} so_rules", $so_rules_list);
					
					$so_rulesPattr = array('/\//', '/\.rules/');
					$so_rulesPattw = array('', '');				
					
					// build list of so rules				
					$so_rules_filterList = snortScanDirFilter($so_rules_list, '/\/.*\.rules/', $so_rulesPattr, $so_rulesPattw);
					
					if (!empty($so_rules_filterList)) {
						// cp rule to so tmp dir
						foreach ($so_rules_filterList as $so_rule)
						{
							
							exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir_rules} so_rules/{$so_rule}.rules");			
							
						}
						// mv and rename so rules
						foreach ($so_rules_filterList as $so_rule_move)
						{	
							exec("/bin/mv -f {$snortdir_rules}/so_rules/{$so_rule_move}.rules {$snortdir_rules}/rules/snort_{$so_rule_move}.so.rules");	
						}
					}
					
					exec("/bin/rm -r {$snortdir_rules}/so_rules");
		
					// extract base etc files
					exec("/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C {$snortdir} etc/");
					exec("/bin/mv -f {$snortdir}/etc/* {$snortdir}");
					exec("/bin/rm -r {$snortdir}/etc");
					
			}
			build_SnortRuleDir();
			// cp md5 to main snort dir
			exec("/bin/cp {$tmpfname}/{$snort_filename_md5} {$snortdir_rules}/{$snort_filename_md5}");		
			update_output_window2('ms2', 'Done extracting Snort.org Rules.');
		}
		
		/* Untar emergingthreats rules to tmp */
		if ($emerg_md5_check_ok === false && file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
			if (file_exists("{$tmpfname}/{$emergingthreats_filename}")) {
				update_output_window2('ms1', 'Extracting Emergingthreats Rules...');
				update_output_window2('ms2', 'May take a while...');
				exec("/usr/bin/tar xzf {$tmpfname}/{$emergingthreats_filename} -C {$emergingdir_rules} rules/");
				exec("/bin/cp {$tmpfname}/{$emergingthreats_filename_md5} {$emergingdir_rules}/{$emergingthreats_filename_md5}");	
				update_output_window2('ms2', 'Done extracting Emergingthreats.net Rules.');
			}
		}
		
		/* Untar Pfsense rules to tmp */
		if ($pfsense_md5_check_ok === false && file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
			if (file_exists("{$tmpfname}/{$pfsense_rules_filename}")) {
				update_output_window2('ms1', 'Extracting Pfsense rules...');
				update_output_window2('ms1', 'May take a while...');
				exec("/usr/bin/tar xzf {$tmpfname}/{$pfsense_rules_filename} -C {$pfsensedir_rules} rules/");
				exec("/bin/cp {$tmpfname}/{$pfsense_rules_filename_md5} {$pfsensedir_rules}/{$pfsense_rules_filename_md5}");
				update_output_window2('ms2', 'Done extracting pfSense.org Rules.');
				
			}
		}		

		/* double make shure cleanup emerg rules that dont belong */
		if (file_exists("/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so")) {
			exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so");
			exec("/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example\*");
		}
		
		/* make shure default rules are in the right format */
		update_output_window2('ms1', 'Reformatting Rules To One Standard...');
		update_output_window2('ms2', 'Please Wait...');
		exec("/usr/local/bin/perl -pi -e 's/#alert/# alert/g' {$snortdir_rules}/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/##alert/# alert/g' {$snortdir_rules}/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/## alert/# alert/g' {$snortdir_rules}/rules/*.rules");
		
		exec("/usr/local/bin/perl -pi -e 's/#alert/# alert/g' {$emergingdir_rules}/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/##alert/# alert/g' {$emergingdir_rules}/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/## alert/# alert/g' {$emergingdir_rules}/rules/*.rules");

		exec("/usr/local/bin/perl -pi -e 's/#alert/# alert/g' {$pfsensedir_rules}/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/##alert/# alert/g' {$pfsensedir_rules}/rules/*.rules");
		exec("/usr/local/bin/perl -pi -e 's/## alert/# alert/g' {$pfsensedir_rules}/rules/*.rules");
		update_output_window2('ms2', 'Done...');		
		
		/* create a msg-map for snort  */
		update_output_window2('ms1', 'Updating Alert Sid Messages...');
		update_output_window2('ms2', 'Please Wait...');
		exec("/usr/local/bin/perl /usr/local/bin/create-sidmap.pl {$snortdir_rules}/rules > /usr/local/etc/snort/sid-msg.map");		
		exec("/usr/local/bin/perl /usr/local/bin/create-sidmap.pl {$emergingdir_rules}/rules >> /usr/local/etc/snort/sid-msg.map");
		exec("/usr/local/bin/perl /usr/local/bin/create-sidmap.pl {$pfsensedir_rules}/rules >> /usr/local/etc/snort/sid-msg.map");
		update_output_window2('ms2', 'Done...');
		
		
		
// ----------------------------------------------------- End Code	--------------------------------------------
			
} // -------------------- END Main function ------------	
	
//$argv[1] = 'console';

	$getWorkerStat = snortSql_fetchAllSettings('snortDBtemp', 'RegisterWorker', 'uuid', 'jdjEf!773&h3bhFd6A');
	
	if ($getWorkerStat['working'] !== 'on') {
		snortSql_updateRuleSetList('working', 'on', '', '', ''); // Register Worker on
		sendUpdateSnortLogDownload($argv[1]);	// start main function
		snortSql_updateRuleSetList('working', 'off', '', '', ''); // Register Worker off
	}	
	
	
	


?>