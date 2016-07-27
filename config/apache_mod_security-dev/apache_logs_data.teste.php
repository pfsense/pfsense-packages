<?php
/* ========================================================================== */
/*
	apache_logs_data.php
	part of pfSense (http://www.pfSense.com)
	Copyright (C) 2012 Marcello Coutinho
	Copyright (C) 2012 Carlos Cesario
	All rights reserved.
                                                                              */
/* ========================================================================== */
/*
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	 1. Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	 2. Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

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
/* ========================================================================== */
# ------------------------------------------------------------------------------
# Defines 
# ------------------------------------------------------------------------------
require_once("guiconfig.inc");

# ------------------------------------------------------------------------------
# Requests
# ------------------------------------------------------------------------------

if ($_GET) {
	# Actions
	$filter = preg_replace('/(@|!|>|<)/',"",htmlspecialchars($_GET['strfilter']));
	$logtype = strtolower($_GET['logtype']);
    switch ($logtype) {
		case 'access':
			//192.168.15.227 - - [02/Jul/2012:19:57:29 -0300] "OPTIONS * HTTP/1.0" 200 - "-" "Apache/2.2.22 (FreeBSD) mod_ssl/2.2.22 OpenSSL/0.9.8q (internal dummy connection)"
			$regex = '/^(\S+) (\S+) (\S+) \[([^:]+):(\d+:\d+:\d+) ([^\]]+)\] \"(\S+) (.*?) (\S+)\" (\S+) (\S+) "([^"]*)" "([^"]*)"$/i';
			
			// Define log file
			$log='/var/log/httpd-access.log';

			//fetch lines
			$logarr=fetch_log($log);

			/*
			// Print lines
			foreach ($logarr as $logent) { 
			// Split line by space delimiter
				$logline  = preg_split("/\n/", $logent);

				// Apply filter and color
				// Need validate special chars
				if ($filter != "")
					$logline = preg_replace("@($filter)@i","<span><font color='red'>$1</font></span>",$logline);

				echo $logline[0]."\n<br/>";
			}
			*/
			$x=1;
			foreach ($logarr as $logent) {
				
				$logline  = preg_split("/\n/", $logent);
					if (preg_match($regex, $logline[0],$line)) {
							echo "campo 1: $line[1] <br/>";
							echo "campo 2: $line[2] <br/>";
							echo "campo 3: $line[3] <br/>";
							echo "campo 4: $line[4] <br/>";
							echo "campo 5: $line[5] <br/>";
							echo "campo 6: $line[6] <br/>";
							echo "campo 7: $line[7] <br/>";
							echo "campo 8: $line[8] <br/>";
							echo "campo 9: $line[9] <br/>";
							echo "campo 10: $line[10] <br/>";
							echo "campo 11: $line[11] <br/>";
							echo "campo 12: $line[12] <br/>";
							echo "campo 13: $line[13] <br/>";
					}
			echo "$x ===================<br>";
						$x++;
			}


		break;
		
		case 'error':
			//[Wed Jul 04 20:22:28 2012] [error] [client 187.10.53.87] proxy: DNS lookup failure for: 192.168.15.272 returned by / 
			$regex = $regex = '/^\[([^\]]+)\] \[([^\]]+)\] (?:\[client ([^\]]+)\])?\s*(.*)$/i';
			
			// Define log file
			$log='/var/log/httpd-error.log';

			//fetch lines
			$logarr=fetch_log($log);

			/*
			// Print lines
			foreach ($logarr as $logent) { 
			// Split line by space delimiter
				$logline  = preg_split("/\n/", $logent);

				// Apply filter and color
				// Need validate special chars
				if ($filter != "")
					$logline = preg_replace("@($filter)@i","<spam><font color='red'>$1</font></span>",$logline);

				echo $logline[0]."\n<br/>";
			}
			*/
			$x=1;
			foreach ($logarr as $logent) {
				
				$logline  = preg_split("/\n/", $logent);
					if (preg_match($regex, $logline[0],$line)) {
							echo "campo 1: $line[1] <br/>";
							echo "campo 2: $line[2] <br/>";
							echo "campo 3: $line[3] <br/>";
							echo "campo 4: $line[4] <br/>";
							echo "campo 5: $line[5] <br/>";
							echo "campo 6: $line[6] <br/>";
							echo "campo 7: $line[7] <br/>";
							echo "campo 8: $line[8] <br/>";
							echo "campo 9: $line[9] <br/>";
							echo "campo 10: $line[10] <br/>";
							echo "campo 11: $line[11] <br/>";
							echo "campo 12: $line[12] <br/>";
							echo "campo 13: $line[13] <br/>";
					}
			echo "$x ===================<br>";
						$x++;
			}


		break;
	}
}

# ------------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------------



// Show Squid Logs
function fetch_log($log){
	global $filter;
	// Get Data from form post
    $lines = $_GET['maxlines'];
    if (preg_match("/!/",htmlspecialchars($_GET['strfilter'])))
    	$grep_arg="-iv";
    else
    	$grep_arg="-i";
		
    // Get logs based in filter expression
    if($filter != "") {
        exec("tail -2000 {$log} | /usr/bin/grep {$grep_arg} " . escapeshellarg($filter). " | tail -r -n {$lines}" , $logarr);     
    }
    else {
        exec("tail -r -n {$lines} {$log}", $logarr);
    }
	// return logs
	return $logarr;
}



foreach ($config['installedpackages']['apachevirtualhost']['config'] as $virtualhost){
    if (is_array($virtualhost['row']) && $virtualhost['enable'] == 'on'){
        if (preg_match("/(\S+)/",base64_decode($virtualhost['primarysitehostname']),$matches)) {
            echo $matches[1]."<br>";
        }
    }
}
?>
