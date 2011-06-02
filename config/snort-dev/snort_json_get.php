<?php 

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_download_rules.inc");

session_start(); // alwaya at the very top of a php page or "Cannot send session cache limiter - headers already sent"

// upload created log tar to user
if ($_GET['snortGetUpdate'] == 1)
{
	
	$tmpfname = "/usr/local/etc/snort/snort_download";
	$snort_filename = "snortrules-snapshot-2905.tar.gz";
	
	
	$snortSessionPath = $_SESSION['tmp']['snort']['snort_download_updates'];
	
	if (!file_exists("{$tmpfname}/{$snort_filename}"))
	{		
	
		if ($snortSessionPath['download']['working'] != '1')
		{
			unset($_SESSION['tmp']);
			$snortSessionPath['download']['working'] = '1';
			sendUpdateSnortLogDownload();		
		}
	
	}
	
	$time = time();
	while((time() - $time) < 30) {
	    // query memcache, database, etc. for new data
	    $data = $datasource->getLatest();
	 
	    // if we have new data return it
	    if(!empty($data)) {
	        echo json_encode($data);
	        ob_flush(); 
	        flush();
	        break;
	    }
	 
	    usleep(25000);
	}	
	
} // end main if



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