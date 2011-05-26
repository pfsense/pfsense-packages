<?php 

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");







// upload created log tar to user
if ($_GET['snortlogdownload'] == 1)
{
	
	sendFileSnortLogDownload();

}


// send Json sid string
if ($_GET['snortGetSidString'] == 1)
{
	
	// unset 
	unset($_GET['snortGetSidString']);
	
	// get the SID string from file	
	sendSidStringRuleEditGUI();

}















?>