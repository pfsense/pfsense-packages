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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_build.inc");

// unset crsf checks
if(isset($_POST['__csrf_magic']))  {
  unset($_POST['__csrf_magic']);
}


function snortJsonReturnCode($returnStatus)
{		
	if ($returnStatus == true) {
		echo '{"snortgeneralsettings":"success","snortMiscTabCall":"true"}';
		return true;
	}else{
		echo '{"snortgeneralsettings":"fail"}';
		return false;
	}
}

// snortsam save settings
if ($_POST['snortSamSaveSettings'] == 1) {

	unset($_POST['snortSamSaveSettings']);
		
	if ($_POST['ifaceTab'] === 'snort_rulesets_ips') {
		function snortSamRulesetSaveFunc() 
		{	
			print_r($_POST);
		}
		snortSamRulesetSaveFunc();
	}
	
	if ($_POST['ifaceTab'] === 'snort_rules_ips') {
		function snortSamRulesSaveFunc() 
		{	
			snortSql_updateRulesSigsIps();
		}
		snortSamRulesSaveFunc();
	}	
	
}

// row from db by uuid
if ($_POST['snortSidRuleEdit'] == 1) {
	
	function snortSidRuleEditFunc() 
	{
	
		unset($_POST['snortSidRuleEdit']);		
		snortSidStringRuleEditGUI();
	
	}	
	snortSidRuleEditFunc();
	
}

	
// row from db by uuid
if ($_POST['snortSaveRuleSets'] == 1) {	

		if ($_POST['ifaceTab'] == 'snort_rulesets' || $_POST['ifaceTab'] == 'snort_rulesets_ips') {
			
			function snortSaveRuleSetsRulesetsFunc()
			{
				// unset POSTs that are markers not in db
				unset($_POST['snortSaveRuleSets']);
				unset($_POST['ifaceTab']);		
				
				// save to database
				snortJsonReturnCode(snortSql_updateRuleSetList());
				
				// only build if uuid is valid
				if (!empty($_POST['uuid'])) {
					build_snort_settings($_POST['uuid']);
				}
			}
			snortSaveRuleSetsRulesetsFunc();	
		}		
		
		if ($_POST['ifaceTab'] == 'snort_rules') {
			function snortSaveRuleSetsRulesFunc()
			{			
				// unset POSTs that are markers not in db
				unset($_POST['snortSaveRuleSets']);
				unset($_POST['ifaceTab']);
				
				snortJsonReturnCode(snortSql_updateRuleSigList());
			}
			snortSaveRuleSetsRulesFunc();
		}	
	
} // END of rulesSets	

// row from db by uuid
if ($_POST['RMlistDelRow'] == 1) {
	
	
	function RMlistDelRowFunc() 
	{
	
		$rm_row_list = snortSql_fetchAllSettings($_POST['RMlistDB'], $_POST['RMlistTable'], 'uuid', $_POST['RMlistUuid']);	
			
		// list rules in the default dir
		if ($_POST['RMlistTable'] == 'SnortIfaces') {
			
			$snortRuleDir = '/usr/local/etc/snort/sn_' . $_POST['RMlistUuid'];
			
			exec('/bin/rm -r ' . $snortRuleDir);
		}
		
		// rm ruledb and files
		if ($_POST['RMlistTable'] == 'Snortrules') {
			
			// remove db tables vals
			snortSql_updatelistDelete($_POST['RMlistDB'], 'SnortruleSets', 'rdbuuid', $_POST['RMlistUuid']);
			snortSql_updatelistDelete($_POST['RMlistDB'], 'SnortruleGenIps', 'rdbuuid', $_POST['RMlistUuid']);	
			snortSql_updatelistDelete($_POST['RMlistDB'], 'SnortruleSetsIps', 'rdbuuid', $_POST['RMlistUuid']);			
			snortSql_updatelistDelete($_POST['RMlistDB'], 'SnortruleSigs', 'rdbuuid', $_POST['RMlistUuid']);
			
			// remove dir
			$snortRuleDir = "/usr/local/etc/snort/snortDBrules/DB/{$_POST['RMlistUuid']}";
			exec('/bin/rm -r ' . $snortRuleDir);
		}	
			
		if ($_POST['RMlistTable'] == 'SnortWhitelist') {
			snortSql_updatelistDelete($_POST['RMlistDB'], 'SnortWhitelistips', 'filename', $rm_row_list['filename']);	
		}		
		
		snortJsonReturnCode(snortSql_updatelistDelete($_POST['RMlistDB'], $_POST['RMlistTable'], 'uuid', $_POST['RMlistUuid']));
	
	}	
	RMlistDelRowFunc();
	
}


// general settings save
if ($_POST['snortSaveSettings'] == 1)  {
	
	function snortSaveSettingsFunc() 
	{
	
		// Save ruleDB settings
		if ($_POST['dbTable'] == 'Snortrules') {
			
			unset($_POST['snortSaveSettings']);		
			unset($_POST['ifaceTab']);
			
			if (!is_dir("/usr/local/etc/snort/snortDBrules/DB/{$_POST['uuid']}/rules")) {				
				
				// creat iface dir and ifcae rules dir
				exec("/bin/mkdir -p /usr/local/etc/snort/snortDBrules/DB/{$_POST['uuid']}/rules");
				
				
				// NOTE: code only works on php5
				$listSnortRulesDir = snortScanDirFilter('/usr/local/etc/snort/snortDBrules/snort_rules/rules', '\.rules');
				$listEmergingRulesDir = snortScanDirFilter('/usr/local/etc/snort/snortDBrules/emerging_rules/rules', '\.rules');
				$listPfsenseRulesDir = snortScanDirFilter('/usr/local/etc/snort/snortDBrules/pfsense_rules/rules', '\.rules');
				
				if (!empty($listSnortRulesDir)) {											
					exec("/bin/cp -R /usr/local/etc/snort/snortDBrules/snort_rules/rules/* /usr/local/etc/snort/snortDBrules/DB/{$_POST['uuid']}/rules");					
				}
				if (!empty($listEmergingRulesDir)) {											
					exec("/bin/cp -R /usr/local/etc/snort/snortDBrules/emerging_rules/rules/* /usr/local/etc/snort/snortDBrules/DB/{$_POST['uuid']}/rules");					
				}								
				if (!empty($listPfsenseRulesDir)) {											
					exec("/bin/cp -R /usr/local/etc/snort/snortDBrules/pfsense_rules/rules/* /usr/local/etc/snort/snortDBrules/DB/{$_POST['uuid']}/rules");					
				}
						
				
			} //end of mkdir		
			
			snortJsonReturnCode(snortSql_updateSettings('uuid', $_POST['uuid']));		
			
		}
		
		// Save general settings
		if ($_POST['dbTable'] == 'SnortSettings') {
			 
			if ($_POST['ifaceTab'] == 'snort_interfaces_global') {    
				// checkboxes when set to off never get included in POST thus this code      
				$_POST['forcekeepsettings'] = ($_POST['forcekeepsettings'] == '' ? off : $_POST['forcekeepsettings']);		
			}
			      
			if ($_POST['ifaceTab'] == 'snort_alerts') {
			        
				if (!isset($_POST['arefresh']))
					$_POST['arefresh'] = ($_POST['arefresh'] == '' ? off : $_POST['arefresh']);
				       		          
			}
			      
			if ($_POST['ifaceTab'] == 'snort_blocked') {
			          
				if (!isset($_POST['brefresh']))
					$_POST['brefresh'] = ($_POST['brefresh'] == '' ? off : $_POST['brefresh']);          
			          
			}
			
			//if (empty($_POST['oinkmastercode'])) {
			//	$_POST['oinkmastercode'] = 'empty';
			//}
			
			// unset POSTs that are markers not in db
			unset($_POST['snortSaveSettings']);		
			unset($_POST['ifaceTab']);
			      		    
			
			snortJsonReturnCode(snortSql_updateSettings('id', '1'));		
		      
		} // end of dbTable SnortSettings
	
	    // Save rule settings on the interface edit tab
		if ($_POST['dbTable'] == 'SnortIfaces') {
	    	
		    // snort interface edit
			if ($_POST['ifaceTab'] == 'snort_interfaces_edit') {
				
				function SnortIfaces_Snort_Interfaces_edit()
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
					* make dir for the new iface, if iface exists or rule dir has changed redo soft link
					* may need to move this as a func to new_snort.inc
					*/
					$newSnortDir = 'sn_' . $_POST['uuid'];
					$pathToSnortDir = '/usr/local/etc/snort';			
						
					// creat iface dir and ifcae rules dir
					if (!is_dir("{$pathToSnortDir}/{$newSnortDir}")) {
						createNewIfaceDir($pathToSnortDir, $newSnortDir);
					} //end of mkdir
					
					snortRulesCreateSoftlink();
	
				}
				SnortIfaces_Snort_Interfaces_edit();						
			          
			} // end of snort_interfaces_edit		
			
			// snort preprocessor edit
			if ($_POST['ifaceTab'] == 'snort_preprocessors') {
				
				function SnortIfaces_Snort_PreprocessorsFunc()
				{				
					if (!isset($_POST['dce_rpc_2'])) {
					 	$_POST['dce_rpc_2'] = ($_POST['dce_rpc_2'] == '' ? off : $_POST['dce_rpc_2']);
					}
					 	
					if (!isset($_POST['dns_preprocessor'])) {
					 	$_POST['dns_preprocessor'] = ($_POST['dns_preprocessor'] == '' ? off : $_POST['dns_preprocessor']);
					}
					 	
					if (!isset($_POST['ftp_preprocessor'])) {
					 	$_POST['ftp_preprocessor'] = ($_POST['ftp_preprocessor'] == '' ? off : $_POST['ftp_preprocessor']);
					}
					 	
					if (!isset($_POST['http_inspect'])) {
					 	$_POST['http_inspect'] = ($_POST['http_inspect'] == '' ? off : $_POST['http_inspect']);
					}
					 	
					if (!isset($_POST['other_preprocs'])) {
					 	$_POST['other_preprocs'] = ($_POST['other_preprocs'] == '' ? off : $_POST['other_preprocs']);
					}
					 	
					if (!isset($_POST['perform_stat'])) {
					 	$_POST['perform_stat'] = ($_POST['perform_stat'] == '' ? off : $_POST['perform_stat']);
					}
					 	
					if (!isset($_POST['sf_portscan'])) {
					 	$_POST['sf_portscan'] = ($_POST['sf_portscan'] == '' ? off : $_POST['sf_portscan']);
					}
					 	
					if (!isset($_POST['smtp_preprocessor'])) {
					 	$_POST['smtp_preprocessor'] = ($_POST['smtp_preprocessor'] == '' ? off : $_POST['smtp_preprocessor']);
					}

				}
				SnortIfaces_Snort_PreprocessorsFunc();				
			}
	
			// snort barnyard edit
			if ($_POST['ifaceTab'] == 'snort_barnyard') {				
				function SnortIfaces_Snort_Barnyard()
				{
					// make shure iface is lower case
					$_POST['interface'] = strtolower($_POST['interface']);
					
					if (!isset($_POST['barnyard_enable'])) {
					 	$_POST['barnyard_enable'] = ($_POST['barnyard_enable'] == '' ? off : $_POST['barnyard_enable']);
					}
				}
				SnortIfaces_Snort_Barnyard();				
			}
			
			
			// unset POSTs that are markers not in db
			unset($_POST['snortSaveSettings']);
			unset($_POST['ifaceTab']);
		      
			snortJsonReturnCode(snortSql_updateSettings('uuid', $_POST['uuid']));
			build_snort_settings($_POST['uuid']);   
	      
	    } // end of dbTable SnortIfaces

	}	
	snortSaveSettingsFunc();
			
} // STOP General Settings Save

// Suppress settings save
if ($_POST['snortSaveSuppresslist'] == 1) {
	
	function snortSaveSuppresslistFunc() 
	{

		// post for supress_edit	
		if ($_POST['ifaceTab'] == 'snort_interfaces_suppress_edit') {
			
		    // make sure filename is valid  
			if (!is_validFileName($_POST['filename'])) {
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
	snortSaveSuppresslistFunc();
	
}

// Whitelist settings save
if ($_POST['snortSaveWhitelist'] == 1) {
	
	function snortSaveWhitelistFunc() 
	{

		if ($_POST['ifaceTab'] == 'snort_interfaces_whitelist_edit') {
		        
			if (!is_validFileName($_POST['filename'])) {
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
	  
	  
		if (snortSql_updateSettings('uuid', $_POST['uuid']) && snortSql_updateWhitelistIps($whitelistIPs)) {
			snortJsonReturnCode(true);
		}else{
			snortJsonReturnCode(false);
		}
  
	}
	snortSaveWhitelistFunc();

}

// download code for alerts page
if ($_POST['snortlogsdownload'] == 1) {
	
	function snortlogsdownloadFunc()
	{
		conf_mount_rw();
		snort_downloadAllLogs();
		conf_mount_ro();
	}
	snortlogsdownloadFunc();

}

// download code for alerts page
if ($_POST['snortblockedlogsdownload'] == 1) {

	function snortblockedlogsdownloadFunc()
	{	
		conf_mount_rw();
		snort_downloadBlockedIPs();
		conf_mount_ro();
	}
	snortblockedlogsdownloadFunc();

}


// code neeed to be worked on when finnished rules code
if ($_POST['snortlogsdelete'] == 1) {
	
	function snortlogsdeleteFunc()
	{	
		conf_mount_rw();
		snortDeleteLogs();
		conf_mount_ro();
	}
	snortlogsdeleteFunc();
}

// flushes snort2c table
if ($_POST['snortflushpftable'] == 1) {
	
	function snortflushpftableFunc()
	{
		conf_mount_rw();
		snortRemoveBlockedIPs();
		conf_mount_ro();
	}
	snortflushpftableFunc();
}

// reset db reset_snortgeneralsettings
if ($_POST['reset_snortgeneralsettings'] == 1) {

	function reset_snortgeneralsettingsFunc()
	{
		conf_mount_rw();
		reset_snortgeneralsettings();
		conf_mount_ro();
	}
	reset_snortgeneralsettingsFunc();	
	
}


?>










