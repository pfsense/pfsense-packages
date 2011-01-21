<?php 


require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");

// unset crsf checks
if(isset($_POST['__csrf_magic'])) 
{
  unset($_POST['__csrf_magic']);
}

// Cancel echo back
if ($_POST['Cancel'] == 1) {

  if ($_POST['CancelType'] == '') {
    echo '
    {
    "snortpost": "fail" 
    }
    ';
    return false;
  }


  if ($_POST['CancelType'] == 'SnortSettings') {
  	snortSql_fetchSettingsAllToJson($_POST['CancelType'], '');
  	return true;
  }

  if ($_POST['CancelType'] == 'SnortWhitelist' || $_POST['CancelType'] == 'SnortWhitelistips') {
    snortSql_fetchSettingsAllToJson($_POST['CancelType'], $_POST['CancelName']);
    return true;
  }
 

}


// general settings save
if ($_POST['snortSaveSettings'] == 1) 
{
    	
		if ($_POST['dbTable'] == 'SnortSettings')
    {
 
      if ($_POST['ifaceTab'] == 'snort_interfaces_global')
      {    
        // checkboxes when set to off never get included in POST thus this code
        $_POST['emergingthreats'] = ($_POST['emergingthreats'] == '' ? off : $_POST['emergingthreats']);        
        $_POST['forcekeepsettings'] = ($_POST['forcekeepsettings'] == '' ? off : $_POST['forcekeepsettings']);

      }
      
      if ($_POST['ifaceTab'] == 'snort_alerts_blocked') 
      {
        
        if (isset($_POST['alertnumber']))
          $_POST['arefresh'] = ($_POST['arefresh'] == '' ? off : $_POST['arefresh']);
          
        if (isset($_POST['blertnumber']))
          $_POST['brefresh'] = ($_POST['brefresh'] == '' ? off : $_POST['brefresh']);          
          
      }

      // unset POSTs that are markers not in db
      unset($_POST['snortSaveSettings']);
      unset($_POST['ifaceTab']);
      
      // update date on every save
      $_POST['date'] = date(U);    
          
          
      //print_r($_POST);
      //return true;
    
      conf_mount_rw();
      snortSql_updateSettings($_POST, 'id', '1');
      conf_mount_ro();      
      
    } // end of dbTable SnortSettings
		
		echo '
		{
		"snortgeneralsettings": "success"	
		}
		';
		return true;
	
}

// Whitelist settings save
if ($_POST['snortSaveWhitelist'] == 1) 
{

  if ($_POST['ifaceTab'] == 'snort_interfaces_whitelist_edit') {
        
        if ($_POST['filename'] == '')
        {
          echo 'Error: No FileName';
          return false;
        }
        
          $_POST['wanips'] = ($_POST['wanips'] == '' ? off : $_POST['wanips']); 
          $_POST['wangateips'] = ($_POST['wangateips'] == '' ? off : $_POST['wangateips']);
          $_POST['wandnsips'] = ($_POST['wandnsips'] == '' ? off : $_POST['wandnsips']);
          $_POST['vips'] = ($_POST['vips'] == '' ? off : $_POST['vips']);
          $_POST['vpnips'] = ($_POST['vpnips'] == '' ? off : $_POST['vpnips']);  
 
  }
  
  // unset POSTs that are markers not in db
  unset($_POST['snortSaveWhitelist']);
  unset($_POST['ifaceTab']);

  $genSettings = $_POST;
  unset($genSettings['list']);
  
  $genSettings['date'] = date(U);
  
  //conf_mount_rw();
  snortSql_updateSettings($genSettings, 'uuid', $genSettings['uuid']);
  if ($_POST['list'] != '')
  {
    snortSql_updateWhitelistIps($_POST['dbTable'], $_POST['list'], $genSettings['filename']);
  }
  //conf_mount_ro();
  
    echo '
    {
    "snortgeneralsettings": "success" 
    }
    ';
    return true;

}

// Whitelist settings delete
if ($_POST['WhitelistDelRow'] == 1) 
{
  //conf_mount_rw();
  snortSql_updateWhitelistDelete($_POST['WhitelistTable'], $_POST['WhitelistUuid']);
  //conf_mount_ro();
}

// download code for alerts page
if ($_POST['snortlogsdownload'] == 1)
{
	conf_mount_rw();
	snort_downloadAllLogs();
	conf_mount_ro();

}

// download code for alerts page
if ($_POST['snortblockedlogsdownload'] == 1)
{
	conf_mount_rw();
	snort_downloadBlockedIPs();
	conf_mount_ro();

}


// code neeed to be worked on when finnished rules code
if ($_POST['snortlogsdelete'] == 1)
{
	
	conf_mount_rw();
	snortDeleteLogs();
	conf_mount_ro();
}

// flushes snort2c table
if ($_POST['snortflushpftable'] == 1)
{
	
	conf_mount_rw();
	snortRemoveBlockedIPs();
	conf_mount_ro();
}

// reset db reset_snortgeneralsettings
if ($_POST['reset_snortgeneralsettings'] == 1)
{

	conf_mount_rw();
	reset_snortgeneralsettings();
	conf_mount_ro();	
	
}


?>










