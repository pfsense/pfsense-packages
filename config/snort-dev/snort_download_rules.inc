#!/usr/local/bin/php
<?php

/*
 * WARNING: THIS FILE SHOULD NEVER BE IN WWWW DIR
 * 
 */

//require_once("/usr/local/pkg/snort/snort_new.inc");

// fetch db Settings NONE Json
function snortSql_fetchAllSettings($dbname, $table, $type, $id_uuid) 
{

	if ($dbname == '' || $table == '' || $type == '') 
	{
		return false;
	}
	  
	$db = sqlite_open("/usr/local/pkg/snort/$dbname");
	
	  
	if ($type == 'id')
	{   
		$result = sqlite_query($db,
		"SELECT * FROM {$table} where id = '{$id_uuid}';
		");
	}

	
	if ($type == 'id')
	{
		$chktable = sqlite_fetch_array($result, SQLITE_ASSOC);
	}
		
	sqlite_close($db);
	  
	return $chktable;
  
  
} // end func

function snortCmpareMD5($type, $path1, $path2, $filename_md5)
{
	update_output_window2('ms2', 'Checking ' . $filename_md5 . ' MD5...');
	
	if (file_exists("{$path1}/{$filename_md5}"))
	{
		
		if ($type == 'string')
		{
			$md5_check_new = @file_get_contents("{$path1}/{$filename_md5}");
			$md5_check_old = @file_get_contents("{$path2}/{$filename_md5}");
			if ($md5_check_new != $md5_check_old || !file_exists("$path2}/{$filename_md5}"))
			{
				update_output_window2('ms2', 'MD5s do not match...');
				return false;
			}
		}
		
		if ($type == 'md5')
		{		
			//md5 snortrules-snapshot-2905.tar.gz | awk '{print $4}'
			$md5_check_new2 = exec("/sbin/md5 {$path1}/{$filename_md5} | '{print $4}'");
			$md5_check_old2 = exec("/sbin/md5 {$path2}/{$filename_md5} | '{print $4}'");
			if ($md5_check_new != $md5_check_old)
			{
				update_output_window2('ms2', 'MD5s do not match...');
				return false;
			}
		}
	}		
		
		
		return true;
}


/*
 *   update_output_window: update bottom textarea dynamically.
 */
function update_output_window2($type, $text)
{

	if ($GLOBALS['tmp']['snort']['downloadupdate']['console'] != 'on')
	{		
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

	if ($type === 'percent2')
	{
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE {$table} SET date = '{$addDate}', percent = '{$value}', filesize = '{$file_size}', downloaded = '{$downloaded}' where filename = '{$filename}';
		");
	}    
    
    
	if ($type === 'percent')
	{
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE {$table} SET date = '{$addDate}', percent = '{$value}' where filename = '{$filename}';
		");
	}

	if ($type === 'msg1')
	{
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE SnortDownloadsMsg SET date = '{$addDate}', msg = '{$value}' where id = '1';
		");
	}

	if ($type === 'msg2')
	{
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE SnortDownloadsMsg SET date = '{$addDate}', msg = '{$value}' where id = '2';
		");
	}

	if ($type === 'working')
	{
		$query_ck = sqlite_query($db, // @ supress warnings usonly in production
		"UPDATE {$table} SET date = '{$addDate}', working = '{$value}' where filename = '{$filename}';
		");
	}	
	
      if (sqlite_changes($db) < 1)
      {
        sqlite_close($db);
        return 'Error in query';   
      }  
	    
	sqlite_close($db);
    
    
}	
	
function sendUpdateSnortLogDownload($console)
{
	
	if ($console === 'console')
	{
		$GLOBALS['tmp']['snort']['downloadupdate']['console'] = 'on';	
	}

	
	// set page vars	
	$generalSettings = snortSql_fetchAllSettings('snortDB', 'SnortSettings', 'id', '1');	
	
	// Setup file names and dir
	$tmpfname = '/usr/local/etc/snort/snort_download';
	$snortdir = '/usr/local/etc/snort';
	$snortdir_wan = '/usr/local/etc/snort'; /// WHAT ?
	$snort_filename_md5 = 'snortrules-snapshot-2905.tar.gz.md5';
	$snort_filename = 'snortrules-snapshot-2905.tar.gz';
	$emergingthreats_filename_md5 = 'emerging.rules.tar.gz.md5';
	$emergingthreats_filename = 'emerging.rules.tar.gz';
	$pfsense_rules_filename_md5 = 'pfsense_rules.tar.gz.md5';
	$pfsense_rules_filename = 'pfsense_rules.tar.gz';
	
	
	/* Set user agent to Mozilla */
	ini_set('user_agent','Mozilla/4.0 (compatible; MSIE 6.0)');
	ini_set("memory_limit","150M");
	
	
	// Get file that does not use redirects, mostly for none snort.org downloads
	function snort_file_get_contents($tmpfname, $snort_filename, $snort_UrlGet)
	{
		if (!file_exists("{$tmpfname}/{$snort_filename}") || filesize("{$tmpfname}/{$snort_filename}") <= 0)
		{
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
	        
	        if ($GLOBALS['tmp']['snort']['downloadupdate']['console'] != 'on')
	        {
	        	snortSql_updateRuleSetList('percent2', $percent, $file_size, $downloaded, $GLOBALS['tmp']['snort']['downloadupdate']['workingfile']); // write out percent to db
			}else{
				echo "\n" . 'percent: ' . $percent . ' filesize: ' . $file_size . ' downloaded: ' . $downloaded;
			}
	}
	
	
	function read_body_firmware($ch, $string) {
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
	
	function download_file_with_progress_bar2($url_file, $destination, $workingfile, $readbody = 'read_body2') {
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

		/*
		if (!file_exists("{$tmpfname}/{$snort_filename}"))
		{
			$GLOBALS['tmp']['snort']['downloadupdate']['workingfile'] = $snort_filename;
			snortSql_updateRuleSetList('working', 'on', '', '', $snort_filename); // finish downloading
			//download_file_with_progress_bar2("http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", $tmpfname . "/{$snort_filename}", "read_body_firmware");
			download_file_with_progress_bar2("http://theseusnetworking.com/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}", $tmpfname, $snort_filename, "read_body_firmware");
			snortSql_updateRuleSetList('percent', '100', '', '', $snort_filename); // finsh percent
			snortSql_updateRuleSetList('working', 'off', '', '', $snort_filename); // finish downloading
		}
		*/
	

	
		// rm all tmp filea
		exec("/bin/rm -r $tmpfname/\*");

		// Set all downloads to be true so NO download by default
		$snort_md5_check_ok = true;
		$emerg_md5_check_ok = true;
		$pfsense_md5_check_ok = true;
		
		/* define checks */		
		$snortdownload = $generalSettings['snortdownload'];
		//$oinkid = $generalSettings['oinkmastercode'];
		$oinkid = '55a1b7a1291b55ac3c157124133744cfc386bb83'; // remove when finished testing				
		
		$emergingthreats = $generalSettings['emergingthreatsdownload'];
		$emergingthreatscode = $generalSettings['emergingthreatscode'];

		

		if ($oinkid == '' && $snortdownload == 'on')
		{
			update_output_window2('ms1', 'You must obtain an oinkid from snort.org and set its value in the Snort settings tab.');
			exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'You must obtain an oinkid from snort.org and set its value in the Snort settings tab.'");
			exit;
		}
		
		if ($snortdownload != "on" && $emergingthreats != "on")
		{
			update_output_window2('ms1', 'SnortStartup: No rules have been selected to download.');
			exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'No rules have been selected to download.'");
			exit;
		}		
		
		/*
		 *  Check MD5s and MARK
		 * 
		 */
		update_output_window2('ms1', 'Starting MD5 checks.....');
		
		// download snort.org md5 and compare
		snort_file_get_contents($tmpfname, $snort_filename_md5, 'http://www.snort.org/pub-bin/oinkmaster.cgi/' . $oinkid);
		
		// if snort.org md5 do not match
		if(!snortCmpareMD5('string', $tmpfname, $snortdir, $snort_filename_md5))
		{
			$snort_md5_check_ok = false;
		}

		// download emergingthreats.net md5 and compare
		snort_file_get_contents($tmpfname, $emergingthreats_filename_md5, 'http://rules.emergingthreats.net/open/snort-2.9.0');
		
		// if emergingthreats.net md5 do not match
		if(!snortCmpareMD5('string', $tmpfname, $snortdir, $emergingthreats_filename_md5))
		{
			$emerg_md5_check_ok = false;
		}
		
		// download pfsense.org md5 and compare
		snort_file_get_contents($tmpfname, $pfsense_rules_filename_md5, 'http://www.pfsense.com/packages/config/snort/pfsense_rules');
		
		// if pfsense.org md5 do not match
		if(!snortCmpareMD5('string', $tmpfname, $snortdir, $pfsense_rules_filename_md5))
		{
			$pfsense_md5_check_ok = false;
		}

		
	

// ----------------------------------------------------- End Code	--------------------------------------------
			
} // -------------------- END Main function ------------	
	
$argv[1] = 'console';

sendUpdateSnortLogDownload($argv[1]);	// start main function
	
	
	
	


?>