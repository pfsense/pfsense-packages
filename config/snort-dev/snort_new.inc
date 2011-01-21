<?php

// unset crsf checks
if(isset($_POST['__csrf_magic'])) 
{
  unset($_POST['__csrf_magic']);
}

// fetch db Settings NONE Json
function snortSql_fetchAllSettings($table, $type, $id_uuid) 
{

  if ($table == '') 
  {
    return false;
  }
  
  $db = sqlite_open('/usr/local/pkg/snort/snortDB');
  
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
function snortSql_updateSettings($settings, $type, $id_uuid) {
  
  $db = '/usr/local/pkg/snort/snortDB';
  $mydb = sqlite_open("$db");
  $table = $settings['dbTable'];

  // unset POSTs that are markers not in db
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
function snortSql_fetchAllWhitelistTypes($table) 
{

  if ($table == '') 
  {
    return false;
  }
  
  $db = sqlite_open('/usr/local/pkg/snort/snortDB');


    $result = sqlite_query($db,
            "SELECT description, filename, uuid, id FROM {$table} where id > 0;
     ");
     
     $chktable = sqlite_fetch_all($result, SQLITE_ASSOC);
 
  if ($chktable == '')
  {
    return false;
  }
 
    foreach ($chktable  as $value)
    { 
      
      $filename2 = $value['filename'];
    
        $result2 = sqlite_query($db,
                "SELECT ip FROM {$table}ips WHERE filename = \"{$filename2}\" LIMIT 4;
         ");
    
        $chktable2 = sqlite_fetch_all($result2, SQLITE_ASSOC); 
 
        $final2 = array('id' => $value['id']);
        $final2['date'] = $value['date']; 
        $final2['uuid'] = $value['uuid'];    
        $final2['filename'] = $value['filename'];
        $final2['description'] = $value['description'];
          
        $final2['list'] = $chktable2;   
        
        $final[] = $final2;
    
    } // end foreach

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

// Whitelist Delete
function snortSql_updateWhitelistDelete($table, $uuid)
{
  
    $db = '/usr/local/pkg/snort/snortDB';
    $mydb = sqlite_open("$db");
    
    $query = sqlite_query($mydb, // @ supress warnings usonly in production
    "SELECT filename FROM {$table} WHERE uuid = '{$uuid}'; 
    ");
    
    $query_ck_filename = sqlite_fetch_array($query, SQLITE_ASSOC);         
    
    if (!empty($query_ck_filename['filename']))
    {

      $query2 = sqlite_query($mydb, // @ supress warnings usonly in production
      "DELETE FROM {$table} WHERE uuid = '{$uuid}';
      ");

      if (sqlite_changes($mydb) < 1)
      {
        sqlite_close($mydb);
        return 'Error in query';   
      }  
       
      $query3 = sqlite_query($mydb, // @ supress warnings usonly in production
      "SELECT ip FROM {$table}ips WHERE filename = '{$query_ck_filename['filename']}';
      ");
      
      $query_rm_ips_ck = sqlite_fetch_array($query3, SQLITE_ASSOC);        

      if (!empty($query_rm_ips_ck))
      {
                        
        $query = sqlite_query($mydb, // @ supress warnings usonly in production
        "DELETE FROM {$table}ips WHERE filename = '{$query_ck_filename['filename']}';
        ");
        
        if (sqlite_changes($mydb) < 1)
        {
          sqlite_close($mydb);
          return 'Error in query';   
        }
        
      }        
      
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