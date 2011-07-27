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

session_start(); // alwaya at the very top of a php page or "Cannot send session cache limiter - headers already sent"

// get json blocls sids
if ($_GET['snortsamjson'] == 1) {
	
	exec('cat /usr/local/etc/snort/sn_6TPXv7a/rules/dbBlockSplit/splitSidblock' . $_GET['fileid'] . '.block', $output);
	echo $output[0];
	
}


// upload created log tar to user
if ($_GET['snortGetUpdate'] == 1) {
	
	$tmpfname = "/usr/local/etc/snort/snort_download";
	$snort_filename = "snortrules-snapshot-2905.tar.gz";
	
	
	$snortSessionPath = $_SESSION['tmp']['snort']['snort_download_updates'];
	
	if (!file_exists("{$tmpfname}/{$snort_filename}")) {		
	
		if ($snortSessionPath['download']['working'] != '1') {
			unset($_SESSION['tmp']);
			$snortSessionPath['download']['working'] = '1';
			sendUpdateSnortLogDownload();		
		}
	
	}
	
	$time = time();
	while((time() - $time) < 30) 
	{
		
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
if ($_GET['snortlogdownload'] == 1) {
	
	sendFileSnortLogDownload();

}


// send Json sid string
if ($_GET['snortGetSidString'] == 1) {
	
	// unset 
	unset($_GET['snortGetSidString']);
	
	// get the SID string from file	
	sendSidStringRuleEditGUI();

}















?>