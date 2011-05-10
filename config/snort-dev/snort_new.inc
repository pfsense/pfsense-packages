<?php

// unset crsf checks
if(isset($_POST['__csrf_magic'])) 
{
  unset($_POST['__csrf_magic']);
}


function snortSql_fetchAllInterfaceRules($table, $dbname)
{
	// do let user pick the DB path
    $db = sqlite_open("/usr/local/pkg/snort/{$dbname}"); 
    
    $result = sqlite_query($db,
            "SELECT * FROM {$table} WHERE id > 0;
     ");
    
	    $chktable = sqlite_fetch_all($result, SQLITE_ASSOC);   
	    
	  	sqlite_close($db);
	  
	  	return $chktable;  
  
}


// fetch db Settings NONE Json
function snortSql_fetchAllSettings($dbname, $table, $type, $id_uuid) 
{

  if ($table == '') 
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
  
  if ($type == 'uuid')
  {   
    $result = sqlite_query($db,
            "SELECT * FROM {$table} where uuid = '{$id_uuid}';
     ");
  }  

  $chktable = sqlite_fetch_array($result, SQLITE_ASSOC);

  sqlite_close($db);
  
  return $chktable;
  
  
} // end func

// fetch db list settings NONE Json
function snortSql_fetchAllSettingsList($table, $listFilename)
{

    $db = sqlite_open('/usr/local/pkg/snort/snortDB'); 

    $result = sqlite_query($db,
            "SELECT * FROM {$table} WHERE filename = \"{$listFilename}\";
     ");

    $chktable = sqlite_fetch_all($result, SQLITE_ASSOC);   
    
  sqlite_close($db);
  
  return $chktable;
  
}

// Update settings to database
function snortSql_updateSettings($dbname, $settings, $type, $id_uuid) {
  
  $db = "/usr/local/pkg/snort/$dbname";
  $mydb = sqlite_open("$db");
  $table = $settings['dbTable'];

  // unset POSTs that are markers not in db
  unset($settings['dbName']);
  unset($settings['dbTable']);

  // START add new row if not set
  if ($type == 'uuid')
  {
  
    $query_ck = sqlite_query($mydb, // @ supress warnings usonly in production
    "SELECT * FROM {$table} WHERE uuid = '{$id_uuid}';
    ");
        
    $query_ckFinal = sqlite_fetch_all($query_ck, SQLITE_ASSOC);  

    if (empty($query_ckFinal))
    {
    
      $query_ck = sqlite_query($mydb, // @ supress warnings usonly in production
      "INSERT INTO {$table} (date, uuid) VALUES ('{$settings['date']}', '{$settings['uuid']}');
      ");
      
      if (sqlite_changes($mydb) < 1)
      {
        sqlite_close($mydb);
        return 'Error in query';   
      }           
      
    }             
    
  }

    // START add values to row
    $kv = array();
    foreach ($settings as $key => $value) 
    {
      $kv[] = $key;
      $val[] = $value;
    }
  
  $countKv = count($kv);
  
  $i = -1;
  while ($i < $countKv) 
  {
    
    $i++;
    
    if ($kv[$i] != '' && $val[$i] != '') 
    {     

      if ($type == 'id')
      {
        $query = sqlite_query($mydb, // @ supress warnings usonly in production
          "UPDATE {$table} SET {$kv[$i]} = '{$val[$i]}' WHERE id = '{$id_uuid}'; 
          ");
      }
      
      if ($type == 'uuid')
      {       
        $query = sqlite_query($mydb, // @ supress warnings usonly in production
          "UPDATE {$table} SET {$kv[$i]} = '{$val[$i]}' WHERE uuid = '{$id_uuid}'; 
          ");
      }         
        
      if (sqlite_changes($mydb) < 1)
      {
        sqlite_close($mydb);
        return 'Error in query';      
      }
              
    }    
  } // end while
  
  sqlite_close($mydb);
  return true; 
    
}


// fetch for snort_interfaces_whitelist.php NONE Json
// use sqlite_fetch_array for single and sqlite_fetch_all for lists
function snortSql_fetchAllWhitelistTypes($table, $table2) 
{

  if ($table == '') 
  {
    return false;
  }
  
  $db = sqlite_open('/usr/local/pkg/snort/snortDB');


    $result = sqlite_query($db,
            "SELECT * FROM {$table} where id > 0;
     ");
     
     $chktable = sqlite_fetch_all($result, SQLITE_ASSOC);
 
  if ($chktable == '')
  {
    return false;
  }
 
 	if ($table2 != '')
 	{
	    foreach ($chktable  as $value)
	    { 
	      
	      $filename2 = $value['filename'];
	    
	        $result2 = sqlite_query($db,
	                "SELECT ip FROM {$table2} WHERE filename = \"{$filename2}\" LIMIT 4;
	         ");
	    
	        $chktable2 = sqlite_fetch_all($result2, SQLITE_ASSOC); 
	 
	        $final2 = array('id' => $value['id']);
	        $final2['date'] = $value['date']; 
	        $final2['uuid'] = $value['uuid'];    
	        $final2['filename'] = $value['filename'];
	        $final2['description'] = $value['description'];
	        $final2['snortlisttype'] = $value['snortlisttype'];
	        
	          
	        $final2['list'] = $chktable2;   
	        
	        $final[] = $final2;
	    
	    } // end foreach
 	}else{
 		$final = $chktable;
 	}
  sqlite_close($db);
  
  return $final;
  
  
} // end func


// Save Whitelistips Settings
function snortSql_updateWhitelistIps($table, $newPostListips, $filename)
{
  
    $db = '/usr/local/pkg/snort/snortDB';
    $mydb = sqlite_open("$db");
    $tableips = $table . 'ips';
    $date = date(U);

    // remove list array that has nul ip
    foreach ($newPostListips as $ipsListEmpty)
    {   
      if (!empty($ipsListEmpty['ip'])) 
      {      
       $genList[] = $ipsListEmpty;      
      }    
    }    
    unset($newPostListips); 
    
    // remove everything if nothing is in the post
    if (empty($genList)) 
    {
      
      $query = sqlite_query($mydb, // @ supress warnings use only in production
      "DELETE FROM {$tableips} WHERE filename = '{$filename}';
      ");
      
      sqlite_close($mydb);
      return true;
      
    }

    // START Remove entries from DB
    $resultUuid = sqlite_query($mydb,
    "SELECT uuid FROM {$tableips} WHERE filename = '{$filename}';
    ");

    $resultUuidFinal = sqlite_fetch_all($resultUuid, SQLITE_ASSOC);
    
    if (!empty($genList) && !empty($resultUuidFinal))
    { 
    
      foreach ($resultUuidFinal as $list3)
      {
        $uuidListDB[] = $list3['uuid'];
      }
  
      foreach ($genList as $list2)
      {
        $uuidListPOST[] = $list2['uuid'];
      }
      
        // create diff array
        $uuidDiff = array_diff($uuidListDB, $uuidListPOST);
      
        // delet diff list objs
        if ($uuidDiff != '')
        {
          foreach ($uuidDiff  as $list4)
          {
      
          // remove everything 
          $query = sqlite_query($mydb, // @ supress warnings use only in production
          "DELETE FROM {$tableips} WHERE uuid = '{$list4}';
          ");
      
          } // end foreach
        }        
    }
    
    // START add entries/updates to DB
    foreach ($genList as $list)
    {
            
      if ($list['uuid'] == 'EmptyUUID')
      {
          
       $uuid = genAlphaNumMixFast(28, 28);
       $list['uuid'] = $uuid;
            
        $query = sqlite_query($mydb, // @ supress warnings use only in production
          "INSERT INTO {$tableips} (date, uuid, filename) VALUES ('{$date}', '{$uuid}', '{$filename}'); 
          ");     
        
        if (sqlite_changes($mydb) < 1)
        {
          sqlite_close($mydb);
          return 'Error in query';   
        }

        foreach ($list as $key => $value)
        {
            
            if ($key != '') 
            {
            
            $query = sqlite_query($mydb, // @ supress warnings usonly in production
              "UPDATE {$tableips} SET {$key} ='{$value}' WHERE uuid = '{$uuid}'; 
              ");    
              
              if (sqlite_changes($mydb) < 1)
              {
                sqlite_close($mydb);
                return 'Error in query';   
              }
              
            }        
          
        } // end foreach
        
      }else{

        $uuid = $list['uuid'];
        
        foreach ($list as $key => $value)
        {
          
            $query = sqlite_query($mydb, // @ supress warnings usonly in production
              "UPDATE {$tableips} SET {$key} ='{$value}', date = '{$date}' WHERE uuid = '{$uuid}'; 
              ");     
            
            if (sqlite_changes($mydb) < 1)
            {
              sqlite_close($mydb);
              return 'Error in query';   
            }          
          
        } // end foreach
        
      } // end main if
        
    } // end Main foreach
     
      sqlite_close($mydb);
      return true; 
      
} // end of func

// RMlist Delete
function snortSql_updatelistDelete($usrDB, $table, $type, $uuid_filename)
{
  
    $db = "/usr/local/pkg/snort/$usrDB";
    $mydb = sqlite_open("$db");

    if ($type == 'uuid')
    {
	    $query = sqlite_query($mydb, // @ supress warnings usonly in production
	    "DELETE FROM {$table} WHERE uuid = '{$uuid_filename}'; 
	    ");
    }

    if ($type == 'filename')
    {    
		$query = sqlite_query($mydb, // @ supress warnings use only in production
		"DELETE FROM {$table} WHERE filename = '{$uuid_filename}';
		");
    }
    
      if (sqlite_changes($mydb) < 1)
      {
        sqlite_close($mydb);
        return 'Error in query';   
      } 
    
    sqlite_close($mydb);
    return true;
  
} // END main func

// create dropdown list
function snortDropDownList($list, $setting) {
  foreach ($list as $iday => $iday2) {
  
    echo "\n" . "<option value=\"{$iday}\"";  if($iday == $setting) echo " selected "; echo '>' . htmlspecialchars($iday2) . '</option>' . "\r";            
      
  } 
}

// downlod all snort logs
function snort_downloadAllLogs() {
	
	$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
	$file_name = "snort_logs_{$save_date}.tar.gz";
	
	exec('/bin/rm /tmp/snort_logs_*.gz'); // remove old file
	exec('/bin/rm /tmp/snort_blocked_*.gz'); // remove old file
	exec('/bin/rm /tmp/snort_block.pf'); // remove old file
	exec('/bin/rm -r /tmp/snort_blocked'); // remove old file
	exec("/usr/bin/tar cfz /tmp/snort_logs_{$save_date}.tar.gz /var/log/snort");
	
	if (file_exists("/tmp/snort_logs_{$save_date}.tar.gz")) {
		echo "
			{
			\"snortdownload\": \"success\",
			\"downloadfilename\": \"{$save_date}\"	
			}
			";
		return true;
	}else{
		return false;
	}
}

// send log files to browser GET function
function sendFileSnortLogDownload() {
	//ob_start(); //importanr or other post will fail
	$file_name_date = $_GET['snortlogfilename'];
	
	$file_name1 = "/tmp/snort_logs_{$file_name_date}.tar.gz";
	$file_name2 = "/tmp/snort_blocked_{$file_name_date}.tar.gz";
	
	if (file_exists($file_name1)) {
		$file_name = "snort_logs_{$file_name_date}.tar.gz";
	}
	
	if (file_exists($file_name2)) {
		$file_name = "snort_blocked_{$file_name_date}.tar.gz";
	}
	
	if ($file_name == '') {
		echo 'Error no saved file.';
		return false;
	}	

	if(file_exists("/tmp/{$file_name}"))
	{
		$file = "/tmp/{$file_name}";
		header("Expires: Mon, 26 Jul 1997 05:00:00 GMT\n");
		header("Pragma: private"); // needed for IE
		header("Cache-Control: private, must-revalidate"); // needed for IE
		header('Content-type: application/force-download');
		header('Content-Transfer-Encoding: Binary');
		header("Content-length: ".filesize($file));
		header("Content-disposition: attachment; filename = {$file_name}");
		readfile("$file");
		exec("/bin/rm /tmp/{$file_name}");
		//od_end_clean(); //importanr or other post will fail
	}else{
		echo 'Error no saved file.';
		return false;
	}
}

// Warning code not finnish untill rule code is DONE !
// Delete Snort logs
function snortDeleteLogs() {
	if(file_exists('/var/log/snort/alert'))
	{
		exec('/bin/echo "" > /var/log/snort/alert');
		//post_delete_logs();
		exec('/usr/sbin/chown snort:snort /var/log/snort/*');
		exec('/bin/chmod 660 /var/log/snort/*');
		sleep(2);
		exec('/usr/bin/killall -HUP snort');
	}
	
	echo '
	{
	"snortdelete": "success"	
	}
	';
	return true;
	
}

// Warning code not finnish untill rule code is DONE !
// code neeed to be worked on when finnished rules code
function post_delete_logs()
{
	global $config, $g;


	$snort_log_dir = '/var/log/snort';

	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule']))
	{


		$rule_array = $config['installedpackages']['snortglobal']['rule'];
		$id = -1;
		foreach ($rule_array as $value)
		{

			if ($id == '') {
				$id = 0;
			}

			$id += 1;

			$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
			$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);
			$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

			if ($if_real != '' && $snort_uuid != '')
			{
				if ($config['installedpackages']['snortglobal']['rule'][$id]['snortunifiedlog'] == 'on')
				{
					$snort_log_file_u2 = "{$snort_uuid}_{$if_real}.u2.";
					$snort_list_u2 = snort_file_list($snort_log_dir, $snort_log_file_u2);
					if (is_array($snort_list_u2)) {
						usort($snort_list_u2, "snort_file_sort");
						$snort_u2_rm_list = snort_build_order($snort_list_u2);
						snort_remove_files($snort_u2_rm_list, $snort_u2_rm_list[0]);
					}
				}else{
					exec("/bin/rm $snort_log_dir/snort_{$snort_uuid}_{$if_real}.u2*");
				}

				if ($config['installedpackages']['snortglobal']['rule'][$id]['tcpdumplog'] == 'on')
				{
					$snort_log_file_tcpd = "{$snort_uuid}_{$if_real}.tcpdump.";
					$snort_list_tcpd = snort_file_list($snort_log_dir, $snort_log_file_tcpd);
					if (is_array($snort_list_tcpd)) {
						usort($snort_list_tcpd, "snort_file_sort");
						$snort_tcpd_rm_list = snort_build_order($snort_list_tcpd);
						snort_remove_files($snort_tcpd_rm_list, $snort_tcpd_rm_list[0]);
					}
				}else{
					exec("/bin/rm $snort_log_dir/snort_{$snort_uuid}_{$if_real}.tcpdump*");
				}

				/* create barnyard2 configuration file */
				//if ($config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'] == 'on')
				//create_barnyard2_conf($id, $if_real, $snort_uuid);

				if ($config['installedpackages']['snortglobal']['rule'][$id]['perform_stat'] == on)
				{
					exec("/bin/echo '' > /var/log/snort/snort_{$snort_uuid}_{$if_real}.stats");
				}
			}
		}
	}
}

// END General Functions

// downlod all blocked ips to log
function snort_downloadBlockedIPs() {
	
	exec('/bin/rm /tmp/snort_logs_*.gz'); // remove old file
	exec('/bin/rm /tmp/snort_blocked_*.gz'); // remove old file
	exec('/bin/rm /tmp/snort_block.pf'); // remove old file
	exec('/bin/rm -r /tmp/snort_blocked'); // remove old file
	$save_date = exec('/bin/date "+%Y-%m-%d-%H-%M-%S"');
	$file_name = "snort_blocked_{$save_date}.tar.gz";
	exec('/bin/mkdir /tmp/snort_blocked');
	exec('/sbin/pfctl -t snort2c -T show > /tmp/snort_block.pf');

	$blocked_ips_array_save = str_replace('   ', '', array_filter(explode("\n", file_get_contents('/tmp/snort_block.pf'))));

	if ($blocked_ips_array_save[0] != '')
	{
		/* build the list */
		$counter = 0;
		foreach($blocked_ips_array_save as $fileline3)
		{
			$counter++;
			exec("/bin/echo  $fileline3 >> /tmp/snort_blocked/snort_block.pf");
		}
	}

	exec("/usr/bin/tar cfz /tmp/snort_blocked_{$save_date}.tar.gz /tmp/snort_blocked");
	
	if (file_exists("/tmp/snort_blocked_{$save_date}.tar.gz")) {
		echo "
			{
			\"snortdownload\": \"success\",
			\"downloadfilename\": \"{$save_date}\"	
			}
			";
		return true;
	}else{
		return false;
	}

}

// flush all ips from snort2c table
function snortRemoveBlockedIPs() {	
	
	exec("/sbin/pfctl -t snort2c -T flush");
	
	echo '
	{
	"snortdelete": "success"	
	}
	';
	return true;
	
}

/* returns true if $name is a valid name for a whitelist file name or ip */
function is_validFileName($name) {
	
	if ($name == '')
		return false;
	
	if (!is_string($name))
		return false;
	
	if (preg_match("/\s+/", $name))
		return false;

	if (!preg_match("/[^a-zA-Z0-9\-_]/", $name))
		return true;

	return false;
}

/* gen Alpha Num Mix for uuids or anything random, NEVER USE rand() */
/* mt_rand/mt_srand is insecure way to gen random nums and strings, when posible use /dev/random or /dev/urandom */
function genAlphaNumMixFast($min = 14, $max = 28)
{
    
    // gen random lenth
    mt_srand(crc32(microtime()));    
    $num = mt_rand($min, $max);
    // reseed
    mt_srand();      
    
    // Gen random string
    $num = $num > 36 ? 30 : $num;
    
    $pool = array_merge(range('A', 'Z'), range(0, 9), range('a', 'z'));
    
    $rand_keys = array_rand($pool, $num);
 
    $randAlpaNum = '';
    
    if (is_array($rand_keys))
    {
      foreach ($rand_keys as $key)
      {
        $randAlpaNum .= $pool[$key];
      }
    }else{
        $randAlpaNum .= $pool[$rand_keys];  
    }

  return str_shuffle($randAlpaNum);

}


?>