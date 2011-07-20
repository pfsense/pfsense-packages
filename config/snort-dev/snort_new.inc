<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
*/


// unset crsf checks
if(isset($_POST['__csrf_magic'])) {
  unset($_POST['__csrf_magic']);
}

//require_once("pfsense-utils.inc");
require_once("config.inc");
require_once("functions.inc");


// create and cp to tmp db dir
if (!file_exists('/var/snort/')) {
	exec('/bin/mkdir -p /var/snort/');
}

if (file_exists('/usr/local/pkg/snort/snortDBtemp')) {
	exec('/bin/cp /usr/local/pkg/snort/snortDBtemp /var/snort/snortDBtemp');
}

// Wites selected sig  to file
function snortSidStringRuleEditGUI()
{

	$workingFile = '/usr/local/etc/snort/sn_' . $_POST['snortSidRuleIface'] . '/rules/' . $_POST['snortSidRuleFile'];
	
	$splitcontents = split_rule_file($workingFile);
	
	if (!empty($splitcontents)) {
		$sidLinePosPre = exec('/usr/bin/sed -n /sid:' . $_POST['snortSidNum'] . '\;/= ' . $workingFile);
		$sidLinePos = $sidLinePosPre - 1;
		
		$splitcontents[$sidLinePos] = $_POST['sidstring'];
		
		
		write_rule_file($splitcontents, $workingFile);
		
		return true;
	}
	
	return false;
	
}	

function sendSidStringRuleEditGUI()
{
	
	$sidCall = exec('sed -n "/alert.*sid:' . $_GET['sid'] . ';.*/p" /usr/local/etc/snort/sn_' . $_GET['snortIface'] . '/rules/' . $_GET['snortRuleFile']);
	$sidCallJsonFilter = escapeJsonString($sidCall);
	
	echo '{"sidstring":' . '"' . $sidCallJsonFilter . '","sid":' . '"' . $_GET['sid'] . '"}';
	return true;
}

// create new Ifac dirs and soft links
function createNewIfaceDir($pathToSnortDir, $newSnortDir) {
	
	exec("/bin/mkdir -p {$pathToSnortDir}/{$newSnortDir}");
	
	// create rules dir soft link if setting is default
	if ($_POST['ruledbname'] === 'default' || empty($_POST['ruledbname'])) {
		if (!file_exists("{$pathToSnortDir}/sn_{$_POST['uuid']}/rules") && file_exists('/usr/local/etc/snort/snortDBrules/DB/default/rules')) {
			exec("/bin/ln -s {$pathToSnortDir}/snortDBrules/DB/default/rules {$pathToSnortDir}/sn_{$_POST['uuid']}/rules");		
		}
	}		
	
	// create rules dir soft link if setting is not default
	if ($_POST['ruledbname'] !== 'default' || $_POST['ruledbname'] != '') {
		if (!file_exists("{$pathToSnortDir}/sn_{$_POST['uuid']}/rules") && file_exists("{$pathToSnortDir}/snortDBrules/DB/{$_POST['ruledbname']}/rules")) {
			exec("/bin/ln -s {$pathToSnortDir}/snortDBrules/DB/{$_POST['ruledbname']}/rules {$pathToSnortDir}/sn_{$_POST['uuid']}/rules");		
		}
	}

	// cp new rules 
	exec("/bin/cp {$pathToSnortDir}/etc/*.config {$pathToSnortDir}/sn_{$_POST['uuid']}");
	exec("/bin/cp {$pathToSnortDir}/etc/*.conf {$pathToSnortDir}/sn_{$_POST['uuid']}");
	exec("/bin/cp {$pathToSnortDir}/etc/*.map {$pathToSnortDir}/sn_{$_POST['uuid']}");
	exec("/bin/cp {$pathToSnortDir}/etc/generators {$pathToSnortDir}/sn_{$_POST['uuid']}");
	exec("/bin/cp {$pathToSnortDir}/etc/sid {$pathToSnortDir}/sn_{$_POST['uuid']}");
} // end of func

function escapeJsonString($escapeString)
{
	$search = array('\\', '\n', '\r', '\u', '\t', '\f', '\b', '/', '"');
	$replace = array('\\\\', '\\n', '\\r', '\\u', '\\t', '\\f', '\\b', '\/', '\"');
	$encoded_string = str_replace($search, $replace, $escapeString);
	
	return $encoded_string;
	
}

// limit the length of the given string to $MAX_LENGTH char
function trimLength($s) {  


  $MAX_LENGTH = 13; 
  $str_to_count = $s; 
  if (strlen($str_to_count) <= $MAX_LENGTH) { 
    return $s; 
  } 

  $s2 = substr($str_to_count, 0, $MAX_LENGTH - 3); 
  $s2 .= "..."; 
  return $s2; 
}


// builds base array with sid etc....
function newFilterRuleSig($baseruleArray)
{

	function get_middle($source, $beginning, $ending, $init_pos) 
	{
	   $beginning_pos = strpos($source, $beginning, $init_pos);
	   $middle_pos = $beginning_pos + strlen($beginning);
	   $ending_pos = strpos($source, $ending, $beginning_pos);
	   $middle = substr($source, $middle_pos, $ending_pos - $middle_pos);
	   return $middle;
	}
	
	
	$i = 0;
	$newSigArray[] = array();
	foreach ( $baseruleArray as $value )
	{
		if (preg_match('/^# alert/', $value) || preg_match('/^alert/', $value)) {
		
			// add sid
			$newSigArray[$i]['sid'] = get_middle($value, 'sid:', ';', 0);
			
			// remove whitespaces
			$rmWhitespaces = preg_replace('/\s\s+/', ' ', $value);		
			// remove whitespace betwin # aerrt
			$rmAlertWhitespace = preg_replace('/^# alert/', '#alert', $rmWhitespaces);				
			$splitcontents = explode(' ', $rmAlertWhitespace);	
					
			// enable or disable
			if ($splitcontents[0] === '#alert') {
				$newSigArray[$i]['enable'] = 'off';
			}else{
				$newSigArray[$i]['enable'] = 'on';
			}
			
			// proto
			$newSigArray[$i]['proto'] = $splitcontents[1];
			
			// source
			$newSigArray[$i]['src'] = trimLength($splitcontents[2]);
			
			// source port
			$newSigArray[$i]['srcport'] = trimLength($splitcontents[3]);
	
			// Destination
			$newSigArray[$i]['dst'] = trimLength($splitcontents[5]);
			
			// Destination port
			$newSigArray[$i]['dstport'] = trimLength($splitcontents[6]);
			
			// sig message 
			$newSigArray[$i]['msg'] = get_middle($value, 'msg:"', '";', 0);
		
		}
			
		$i++;
					
	}
	
	return $newSigArray;
}


function split_rule_file($workingFile)
{
	$filehandle = fopen($workingFile, "r");
	$contents = fread($filehandle, filesize($workingFile));

	fclose ($filehandle);

	$delimiter = "\n";

	$splitcontents = explode($delimiter, $contents);

	return $splitcontents;
}


// write rule file to disk
function write_rule_file($content_changed, $received_file)
{
	//read snort file with writing enabled
	$filehandle = fopen($received_file, "w");

	//delimiter for each new rule is a new line
	$delimiter = "\n";

	//implode the array back into a string for writing purposes
	$fullfile = implode($delimiter, $content_changed);

	//write data to file
	fwrite($filehandle, $fullfile);

	//close file handle
	fclose($filehandle);

}


// Save ruleSets settings
function snortSql_updateRuleSigList()
{
	
	// selected snort rule file	
	$workingFile = "/usr/local/etc/snort/snortDBrules/DB/{$_SESSION['snort']['tmp']['snort_rules']['rdbuuid']}/rules/{$_SESSION['snort']['tmp']['snort_rules']['rulefile']}";
	
	$splitcontents = split_rule_file($workingFile);
	
	// open rule file and change enable/disable sids
	function read_rule_file($splitcontents, $enableSigsArray, $disableSigsArray)
	{			
		
		foreach ($splitcontents as $sigLine)
		{
			$replaceChars = array('/sid:/', '/;/');
			preg_match('/sid:[0-9]*;/', $sigLine, $matches);
			$sidLine = preg_replace($replaceChars, '', $matches[0]);
			
			
			if (empty($sidLine)) {
				$tempstring[] = $sigLine;
			}else{
	
				if (in_array($sidLine, $enableSigsArray)) {
					$tempstring[] = str_replace("# alert", "alert", $sigLine);
				}
				
				if (in_array($sidLine, $disableSigsArray)) {
					$tempstring[] = str_replace("alert", "# alert", $sigLine);
				}
				
				if (!in_array($sidLine, $enableSigsArray) && !in_array($sidLine, $disableSigsArray)) {
					$tempstring[] = $sigLine;
				}			
			}		
		}
			
		return $tempstring;
	}
	
	// build user selected enbled and disabled arrays
    $enableSigsArray = array();
	$disableSigsArray = array();
	
	if (!isset($_POST['filenamcheckbox2'])) {
		$_POST['filenamcheckbox2'] = array();
	}
	
	$newFilterRuleSigArray = newFilterRuleSig($splitcontents);
	
	foreach ($newFilterRuleSigArray as $sigArray)
	{		
		// enable sig
		if(in_array($sigArray['sid'], $_POST['filenamcheckbox2']) && $sigArray['enable'] == 'off') {
			$enableSigsArray[] = $sigArray['sid'];			
		}
		
		// disable sig
		if(!in_array($sigArray['sid'], $_POST['filenamcheckbox2']) && $sigArray['enable'] == 'on') {
			$disableSigsArray[] = $sigArray['sid'];
		}		
	}
		
	// read rule file change disable/enable then write to file if arrays  are not empty
	if (!empty($enableSigsArray) || !empty($disableSigsArray)) {
		write_rule_file(read_rule_file($splitcontents, $enableSigsArray, $disableSigsArray), $workingFile);
	}
	
	// Insert into the DB for oinkmaster
	
	function sql_EnableDisabeSid($SigArray, $OnOff)
	{
	
		$dbname = $_SESSION['snort']['tmp']['snort_rules']['dbName'];
		$table =  $_SESSION['snort']['tmp']['snort_rules']['dbTable'];
		$rdbuuid = $_SESSION['snort']['tmp']['snort_rules']['rdbuuid'];
		$rulefile = $_SESSION['snort']['tmp']['snort_rules']['rulefile'];
		$addDate = date(U);
		
		// dont let user pick the DB path
	    $db = sqlite_open("/usr/local/pkg/snort/{$dbname}");
	    
	    foreach ($SigArray as $mDEanbled)
	    {
	
			$resultid = sqlite_query($db,
				"SELECT id FROM {$table} WHERE signatureid = '{$mDEanbled}' AND signaturefilename = '{$rulefile}';
			");
			
			$chktable = sqlite_fetch_all($resultid, SQLITE_ASSOC);
			
			 if (empty($chktable)) {
			 	
				$query_ck = sqlite_query($db, // @ supress warnings usonly in production
				"INSERT INTO {$table} (date, rdbuuid, signatureid, signaturefilename, enable) VALUES ('{$addDate}', '{$rdbuuid}', '{$mDEanbled}', '{$rulefile}', '{$OnOff}');
				");		 	
			 	
			 }else{
				if ($chktable[0]['enable'] != $OnOff) {
					$query_ck = sqlite_query($db, // @ supress warnings usonly in production
					"UPDATE {$table} SET date = {$addDate}, enable = '{$OnOff}' WHERE signatureid = '{$mDEanbled}' AND signaturefilename = '{$rulefile}'; 
					");
				} 	
			 	
			 	
			 }
	    
	    	
	    }    
	    
	    sqlite_close($db);
    
	} // snd of function
	
	sql_EnableDisabeSid($enableSigsArray, 'on');
	sql_EnableDisabeSid($disableSigsArray, 'off');
	
	
	return true;

    
} // END Save ruleSets settings

// Save ruleSets settings
function snortSql_updateRuleSetList()
{
	
	$dbname = $_POST['dbName'];
	$table = $_POST['dbTable'];
	$ruleSetfilenames = $_POST['filenamcheckbox'];
	$rdbuuid = $_POST['rdbuuid'];
	
	$addDate = date(U);

	// dont let user pick the DB path
    $db = sqlite_open("/usr/local/pkg/snort/{$dbname}"); 
    
    if (empty($ruleSetfilenames)) {
    	$ruleSetfilenames = array();
    }

	// foreach selected rulesets do this
    if (!empty($ruleSetfilenames)) {
	    foreach ($ruleSetfilenames as $ruleSetfilename) 
	    {
	
			$resultid = sqlite_query($db,
				"SELECT id, enable FROM {$table} WHERE rulesetname = '{$ruleSetfilename}' and rdbuuid = '{$rdbuuid}';
			");			
		    
			$chktable = sqlite_fetch_all($resultid, SQLITE_ASSOC);   
		
			if (empty($chktable)) {
				
				$rulesetUuid = genAlphaNumMixFast(11, 14);
				
				$query_ck = sqlite_query($db, // @ supress warnings usonly in production
				"INSERT INTO {$table} (date, uuid, rdbuuid, rulesetname, enable) VALUES ('{$addDate}', '{$rulesetUuid}', '{$rdbuuid}', '{$ruleSetfilename}', 'on');
				");

			}else{					
				if ($chktable[0]['enable'] == 'off') {					
					$query_ck = sqlite_query($db, // @ supress warnings usonly in production
					"UPDATE {$table} SET enable = 'on' WHERE id = '{$chktable[0]['id']}'; 
					");			
				}			
			}
	    }	
	} // end foreach if

			
			// clean database of old names	and turn rulesets off
    		$listDir = snortScanDirFilter("/usr/local/etc/snort/snortDBrules/DB/{$rdbuuid}/rules/", '\.rules');
    
		    $resultAllRulesetname = sqlite_query($db,
		            "SELECT rulesetname FROM {$table} WHERE rdbuuid = '{$rdbuuid}';
		     ");
		    
		    $chktable2 = sqlite_fetch_all($resultAllRulesetname, SQLITE_ASSOC);		
		    
		    
		    if (!empty($chktable2))
		    {
			    foreach ($chktable2 as $value) 
			    {
					
			    	if(!in_array($value['rulesetname'], $listDir))
			    	{	    		
						$deleteMissingRuleset = sqlite_query($db, // @ supress warnings use only in production
						"DELETE FROM {$table} WHERE rulesetname = '{$value['rulesetname']}' and rdbuuid = '{$rdbuuid}';
						");
			    	}
			    	
			    	if(!in_array($value['rulesetname'], $ruleSetfilenames))
			    	{
						$ruleSetisOff = sqlite_query($db, // @ supress warnings usonly in production
						"UPDATE {$table} SET enable = 'off' WHERE rulesetname = '{$value['rulesetname']}' and rdbuuid = '{$rdbuuid}'; 
						");	    		
			    	}	    	
			    }
		    }   
	    
	  	sqlite_close($db);
	  	
	  	return true;
    
} // END Save ruleSets settings


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

	if (empty($dbname) || empty($table) || empty($type)) {
		return false;
	}
	  
	$db = sqlite_open("/usr/local/pkg/snort/$dbname");

	if ($type == 'All') {
		
		$result = sqlite_query($db,
		"SELECT * FROM {$table} WHERE id > 0;
		");
		
	}else{
		
		$result = sqlite_query($db,
		"SELECT * FROM {$table} where {$type} = '{$id_uuid}';
		");
			
	}	
	
	if ($type == 'id' || $type == 'uuid') {
		$chktable = sqlite_fetch_array($result, SQLITE_ASSOC);
	}
	
	if ($type == 'All' || $type == 'ifaceuuid' || $type == 'ruledbname' || $type == 'rdbuuid' || $type == 'filename') {
		$chktable = sqlite_fetch_all($result, SQLITE_ASSOC);
	}
		
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
function snortSql_updateSettings($type, $id_uuid) 
{
	$dbname = $_POST['dbName'];
	$settings = $_POST;
	
	// update date on every save
	$_POST['date'] = date(U); 
  
  $db = "/usr/local/pkg/snort/$dbname";
  $mydb = sqlite_open("$db");
  $table = $settings['dbTable'];

  // unset POSTs that are markers not in db
  unset($settings['dbName']);
  unset($settings['dbTable']);

  // START add new row if not set
  if ($type == 'uuid') {
  
    $query_ck = sqlite_query($mydb, // @ supress warnings usonly in production
    "SELECT * FROM {$table} WHERE uuid = '{$id_uuid}';
    ");
        
    $query_ckFinal = sqlite_fetch_all($query_ck, SQLITE_ASSOC);  

    if (empty($query_ckFinal)) {
    
      $query_ck = sqlite_query($mydb, // @ supress warnings usonly in production
      "INSERT INTO {$table} (date, uuid) VALUES ('{$settings['date']}', '{$settings['uuid']}');
      ");
      
      if (sqlite_changes($mydb) < 1) {
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
    
    if (!empty($kv[$i])) 
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

  if (empty($table)) {
    return false;
  }
  
  $db = sqlite_open('/usr/local/pkg/snort/snortDB');


    $result = sqlite_query($db,
            "SELECT * FROM {$table} where id > 0;
     ");
     
     $chktable = sqlite_fetch_all($result, SQLITE_ASSOC);
 
  if (empty($chktable)) {
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
function snortSql_updateWhitelistIps($newPostListips)
{
	
	if(empty($newPostListips))
	{
		return true;
	}
	
	$table = $_POST['dbTable'];
	$filename = $_POST['filename'];	
  
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
function snortSql_updatelistDelete($databse, $table, $type, $uuid_filename)
{
	
    $db = "/usr/local/pkg/snort/{$databse}";
    
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
    
    if ($type == 'ifaceuuid')
    {    
		$query = sqlite_query($mydb, // @ supress warnings use only in production
		"DELETE FROM {$table} WHERE ifaceuuid = '{$uuid_filename}';
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
	
	if (empty($file_name)) {
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

			if (empty($id)) {
				$id = 0;
			}

			$id += 1;

			$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
			$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

			if ($snort_uuid != '')
			{
				if ($config['installedpackages']['snortglobal']['rule'][$id]['snortunifiedlog'] == 'on')
				{
					$snort_log_file_u2 = "{$snort_uuid}.u2.";
					$snort_list_u2 = snort_file_list($snort_log_dir, $snort_log_file_u2);
					if (is_array($snort_list_u2)) {
						usort($snort_list_u2, "snort_file_sort");
						$snort_u2_rm_list = snort_build_order($snort_list_u2);
						snort_remove_files($snort_u2_rm_list, $snort_u2_rm_list[0]);
					}
				}else{
					exec("/bin/rm $snort_log_dir/snort_{$snort_uuid}.u2*");
				}

				if ($config['installedpackages']['snortglobal']['rule'][$id]['tcpdumplog'] == 'on')
				{
					$snort_log_file_tcpd = "{$snort_uuid}.tcpdump.";
					$snort_list_tcpd = snort_file_list($snort_log_dir, $snort_log_file_tcpd);
					if (is_array($snort_list_tcpd)) {
						usort($snort_list_tcpd, "snort_file_sort");
						$snort_tcpd_rm_list = snort_build_order($snort_list_tcpd);
						snort_remove_files($snort_tcpd_rm_list, $snort_tcpd_rm_list[0]);
					}
				}else{
					exec("/bin/rm $snort_log_dir/snort_{$snort_uuid}.tcpdump*");
				}

				/* create barnyard2 configuration file */
				//if ($config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'] == 'on')
				//create_barnyard2_conf($id, $if_real, $snort_uuid);

				if ($config['installedpackages']['snortglobal']['rule'][$id]['perform_stat'] == on)
				{
					exec("/bin/echo '' > /var/log/snort/snort_{$snort_uuid}.stats");
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
	
	if (empty($name)) {
		return false;
	}
	
	if (!is_string($name)) {
		return false;
	}
	
	if (preg_match("/\s+/", $name)) {
		return false;
	}

	if (!preg_match("/[^a-zA-Z0-9\-_]/", $name)) {
		return true;
	}

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
    
    if (is_array($rand_keys)) {
      foreach ($rand_keys as $key)
      {
        $randAlpaNum .= $pool[$key];
      }
    }else{
        $randAlpaNum .= $pool[$rand_keys];  
    }

  return str_shuffle($randAlpaNum);

}

// scan a dir, build array with filetr
function snortScanDirFilter($path, $filtername)
{
	// list rules in the default dir
	$listDir = array();
	$listDir = scandir("{$path}");

	if (empty($filtername)) {
			
		return $listDir;
		
	}else{
	
		$pattern = "/{$filtername}/";
	 	foreach ( $listDir as $val )
	 	{
	 		if (preg_match($pattern, $val)) {
	 			$filterDirList[] = $val;			
	 		}
	 	}
		unset($listDir);		
	}
		return $filterDirList;		
}

?>

