<?php 


require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");

// unset crsf checks
if(isset($_POST['__csrf_magic'])) 
{
  unset($_POST['__csrf_magic']);
}


function snortJsonReturnCode($returnStatus)
{		
	if ($returnStatus == true)
	{
		echo '{"snortgeneralsettings":"success","snortUnhideTabs":"true"}';
	}else{
		echo '{"snortgeneralsettings":"fail"}';
	}		
}

// row from db by uuid
if ($_POST['snortSidRuleEdit'] == 1)
{
	
	unset($_POST['snortSidRuleEdit']);
	
	snortSidStringRuleEditGUI();
	
}

	
// row from db by uuid
if ($_POST['snortSaveRuleSets'] == 1)
{	

	if ($_POST['ifaceTab'] == 'snort_rulesets')
	{	
		// unset POSTs that are markers not in db
		unset($_POST['snortSaveRuleSets']);
		unset($_POST['ifaceTab']);		
		
		snortJsonReturnCode(snortSql_updateRuleSetList());

	}
	
	
	if ($_POST['ifaceTab'] == 'snort_rules')
	{	
		// unset POSTs that are markers not in db
		unset($_POST['snortSaveRuleSets']);
		unset($_POST['ifaceTab']);
		
		snortJsonReturnCode(snortSql_updateRuleSigList());
	}	
	
	
} // END of rulesSets	

// row from db by uuid
if ($_POST['RMlistDelRow'] == 1)
{
	
	
	if ($_POST['RMlistTable'] == 'Snortrules' || $_POST['RMlistTable'] == 'SnortSuppress')
	{	
		
		// list rules in the default dir
		$a_list = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'uuid', $_POST['RMlistUuid']);
		$snortRuleDir = '/usr/local/etc/snort/sn_' . $_POST['RMlistUuid'] . '_' . $a_list['interface'];
		
		exec('/bin/rm -r ' . $snortRuleDir);
				
		snortSql_updatelistDelete('SnortruleSets', 'ifaceuuid', $_POST['RMlistUuid']);
	  	snortSql_updatelistDelete('SnortruleSigs', 'ifaceuuid', $_POST['RMlistUuid']);
	  	snortSql_updatelistDelete('Snortrules', 'uuid', $_POST['RMlistUuid']);
	  	
	  	snortJsonReturnCode(true);
	  	
	}
	
	if ($_POST['RMlistTable'] == 'SnortSuppress')
	{
	  	snortJsonReturnCode(snortSql_updatelistDelete($_POST['RMlistTable'], 'uuid', $_POST['RMlistUuid']));
	}	
	
	
	
	if ($_POST['RMlistTable'] == 'SnortWhitelist')
	{
		$fetchExtraWhitelistEntries = snortSql_fetchAllSettings($_POST['RMlistDB'], $_POST['RMlistTable'], 'uuid', $_POST['RMlistUuid']);
		
		snortJsonReturnCode(snortSql_updatelistDelete('SnortWhitelistips', 'filename', $fetchExtraWhitelistEntries['filename']));	
		snortJsonReturnCode(snortSql_updatelistDelete($_POST['RMlistTable'], 'uuid', $_POST['RMlistUuid']));

	}
	
}


// general settings save
if ($_POST['snortSaveSettings'] == 1) 
{
	
	// Save general settings
	if ($_POST['dbTable'] == 'SnortSettings')
	{
		 
		if ($_POST['ifaceTab'] == 'snort_interfaces_global')
		{    
			// checkboxes when set to off never get included in POST thus this code      
			$_POST['forcekeepsettings'] = ($_POST['forcekeepsettings'] == '' ? off : $_POST['forcekeepsettings']);		
		}
		      
		if ($_POST['ifaceTab'] == 'snort_alerts') 
		{
		        
			if (!isset($_POST['arefresh']))
				$_POST['arefresh'] = ($_POST['arefresh'] == '' ? off : $_POST['arefresh']);
			       		          
		}
		      
		if ($_POST['ifaceTab'] == 'snort_blocked') 
		{
		          
			if (!isset($_POST['brefresh']))
				$_POST['brefresh'] = ($_POST['brefresh'] == '' ? off : $_POST['brefresh']);          
		          
		}
		
		// unset POSTs that are markers not in db
		unset($_POST['snortSaveSettings']);		
		unset($_POST['ifaceTab']);
		      		    
		
		snortJsonReturnCode(snortSql_updateSettings('id', '1'));		
	      
	} // end of dbTable SnortSettings

    // Save rule settings on the interface edit tab
	if ($_POST['dbTable'] == 'Snortrules')
    {
    	
	    // snort interface edit
		if ($_POST['ifaceTab'] == 'snort_interfaces_edit') 
		{
		        
			if (!isset($_POST['enable']))
			 	$_POST['enable'] = ($_POST['enable'] == '' ? off : $_POST['enable']);
			          
			if (!isset($_POST['blockoffenders7']))
				$_POST['blockoffenders7'] = ($_POST['blockoffenders7'] == '' ? off : $_POST['blockoffenders7']);
	
			if (!isset($_POST['alertsystemlog']))
				$_POST['alertsystemlog'] = ($_POST['alertsystemlog'] == '' ? off : $_POST['alertsystemlog']);  
	
			if (!isset($_POST['tcpdumplog']))
				$_POST['tcpdumplog'] = ($_POST['tcpdumplog'] == '' ? off : $_POST['tcpdumplog']); 
	
			if (!isset($_POST['snortunifiedlog']))
				$_POST['snortunifiedlog'] = ($_POST['snortunifiedlog'] == '' ? off : $_POST['snortunifiedlog']);
				
			// convert textbox to base64
			$_POST['configpassthru'] = base64_encode($_POST['configpassthru']);
			
			/*
			 * make dir for the new iface
			 * may need to move this as a func to new_snort,inc
			 */
			
			$newSnortDir = 'sn_' . $_POST['uuid'] . '_' . $_POST['interface'];
			
			if (!is_dir("/usr/local/etc/snort/{$newSnortDir}")) {				
				
				// creat iface dir and ifcae rules dir
				exec("/bin/mkdir -p /usr/local/etc/snort/{$newSnortDir}/rules");
				
				// NOTE: code only works on php5
				$listSnortRulesDir = snortScanDirFilter('/usr/local/etc/snort/snort_rules/rules', '\.rules');
				$listEmergingRulesDir = snortScanDirFilter('/usr/local/etc/snort/emerging_rules/rules', '\.rules');
				$listPfsenseRulesDir = snortScanDirFilter('/usr/local/etc/snort/pfsense_rules/rules', '\.rules');
				
				if (!empty($listSnortRulesDir)) {											
					exec("/bin/cp -R /usr/local/etc/snort/snort_rules/rules/* /usr/local/etc/snort/{$newSnortDir}/rules");					
				}
				if (!empty($listEmergingRulesDir)) {											
					exec("/bin/cp -R /usr/local/etc/snort/emerging_rules/rules/* /usr/local/etc/snort/{$newSnortDir}/rules");					
				}								
				if (!empty($listPfsenseRulesDir)) {											
					exec("/bin/cp -R /usr/local/etc/snort/pfsense_rules/rules/* /usr/local/etc/snort/{$newSnortDir}/rules");					
				}
				
				
			} //end of mkdir
		          
		} // end of snort_interfaces_edit		
		
		// snort preprocessor edit
		if ($_POST['ifaceTab'] == 'snort_preprocessors') 
		{

			if (!isset($_POST['dce_rpc_2']))
			 	$_POST['dce_rpc_2'] = ($_POST['dce_rpc_2'] == '' ? off : $_POST['dce_rpc_2']);
			 	
			if (!isset($_POST['dns_preprocessor']))
			 	$_POST['dns_preprocessor'] = ($_POST['dns_preprocessor'] == '' ? off : $_POST['dns_preprocessor']);
			 	
			if (!isset($_POST['ftp_preprocessor']))
			 	$_POST['ftp_preprocessor'] = ($_POST['ftp_preprocessor'] == '' ? off : $_POST['ftp_preprocessor']);
			 	
			if (!isset($_POST['http_inspect']))
			 	$_POST['http_inspect'] = ($_POST['http_inspect'] == '' ? off : $_POST['http_inspect']);
			 	
			if (!isset($_POST['other_preprocs']))
			 	$_POST['other_preprocs'] = ($_POST['other_preprocs'] == '' ? off : $_POST['other_preprocs']);
			 	
			if (!isset($_POST['perform_stat']))
			 	$_POST['perform_stat'] = ($_POST['perform_stat'] == '' ? off : $_POST['perform_stat']);
			 	
			if (!isset($_POST['sf_portscan']))
			 	$_POST['sf_portscan'] = ($_POST['sf_portscan'] == '' ? off : $_POST['sf_portscan']);
			 	
			if (!isset($_POST['smtp_preprocessor']))
			 	$_POST['smtp_preprocessor'] = ($_POST['smtp_preprocessor'] == '' ? off : $_POST['smtp_preprocessor']);			
			
		}

		// snort barnyard edit
		if ($_POST['ifaceTab'] == 'snort_barnyard') 
		{
			// make shure iface is lower case
			$_POST['interface'] = strtolower($_POST['interface']);
			
			if (!isset($_POST['barnyard_enable']))
			 	$_POST['barnyard_enable'] = ($_POST['barnyard_enable'] == '' ? off : $_POST['barnyard_enable']);	
			
		}
		
		
	      // unset POSTs that are markers not in db
	      unset($_POST['snortSaveSettings']);
	      unset($_POST['ifaceTab']);
	      
	      snortJsonReturnCode(snortSql_updateSettings('uuid', $_POST['uuid']));	      
      
    } // end of dbTable Snortrules    		
			
} // STOP General Settings Save

// Suppress settings save
if ($_POST['snortSaveSuppresslist'] == 1) 
{

	// post for supress_edit	
	if ($_POST['ifaceTab'] == 'snort_interfaces_suppress_edit') 
	{
		
	    // make sure filename is valid  
		if (!is_validFileName($_POST['filename']))
		{
			echo 'Error: FileName';
			return false;
		}
		
		// unset POSTs that are markers not in db
		unset($_POST['snortSaveSuppresslist']);
		unset($_POST['ifaceTab']);
		
		// convert textbox to base64
		$_POST['suppresspassthru'] = base64_encode($_POST['suppresspassthru']);
		
		// Write to database
		snortJsonReturnCode(snortSql_updateSettings('uuid', $_POST['uuid']));		
	  	  
	}
	
}

// Whitelist settings save
if ($_POST['snortSaveWhitelist'] == 1) 
{

  if ($_POST['ifaceTab'] == 'snort_interfaces_whitelist_edit') {
        
		if (!is_validFileName($_POST['filename']))
		{
			echo 'Error: FileName';
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
  
  // Split the POST for 2 arraus
  $whitelistIPs = $_POST['list'];
  unset($_POST['list']);
  
  
  if (snortSql_updateSettings('uuid', $_POST['uuid']) && snortSql_updateWhitelistIps($whitelistIPs))
  {
  	snortJsonReturnCode(true);
  }else{
  	snortJsonReturnCode(false);
  }


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










