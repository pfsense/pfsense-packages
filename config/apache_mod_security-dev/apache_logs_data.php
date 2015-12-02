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

	 1. Redistributions of source code MUST retain the above copyright notice,
		this list of conditions and the following disclaimer.

	 2. Redistributions in binary form MUST reproduce the above copyright
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
	$filter = preg_replace('/(@|!|>|<)/',"",htmlspecialchars($_REQUEST['strfilter']));
	$logtype = strtolower($_REQUEST['logtype']);
	
	// Get log type (access or error)
    if ($logtype == "error")
    	$error="-error";
   	
	// Define log file name
	$logfile ='/var/log/httpd-'. preg_replace("/(\s|'|\"|;)/","",$_REQUEST['logfile']) . $error.'.log';
   
	if ($logfile == '/var/log/httpd-access-error.log')
		$logfile = '/var/log/httpd-error.log';
   	
	//debug
   	echo "<tr valign=\"top\">\n";
	echo "<td colspan=\"5\" class=\"listlr\" align=\"center\" nowrap >$logfile</td>\n";
    if (file_exists($logfile)){
	
		switch ($logtype) {
		
			case 'access':
				//show table headers
				show_tds(array("Time","Host","Response","Method","Request"));

				//fetch lines
				$logarr=fetch_log($logfile);

				// Print lines
				foreach ($logarr as $logent) {
				// Split line by space delimiter
					$logline  = preg_split("/\n/", $logent);
					/*
					field 1: 189.29.36.26 
					field 2: - 
					field 3: - 
					field 4: 04/Jul/2012 
					field 5: 10:54:39 
					field 6: -0300 
					field 7: GET 
					field 8: / 
					field 9: HTTP/1.1 
					field 10: 303 
					field 11: - 
					field 12: - 
					field 13: Mozilla/5.0 (X11; Linux i686) AppleWebKit/535.19 (KHTML, like Gecko) Ubuntu/12.04 Chromium/18.0.1025.151 Chrome/18.0.1025.151 Safari/535.19 
					*/
					$regex = '/^(\S+) (\S+) (\S+) \[([^:]+):(\d+:\d+:\d+) ([^\]]+)\] \"(\S+) (.*?) (\S+)\" (\S+) (\S+) "([^"]*)" "([^"]*)"$/';
					if (preg_match($regex, $logline[0],$line)) {
						// Apply filter and color
						if ($filter != "")
							$line = preg_replace("@($filter)@i","<span><font color='red'>$1</font></span>",$line);
						$agent_info="onmouseover=\"jQuery('#browserinfo').empty().html('{$line[13]}');\"\n";
						echo "<tr valign=\"top\" $agent_info>\n";
						echo "<td class=\"listlr\" align=\"center\" nowrap>{$line[5]}({$line[6]})</td>\n";
						echo "<td class=\"listr\" align=\"center\">{$line[1]}</td>\n";
						echo "<td class=\"listr\" align=\"center\">{$line[10]}</td>\n";
						echo "<td class=\"listr\" align=\"center\">{$line[7]}</td>\n";
						//echo "<td class=\"listr\" width=\"*\" onmouseout=\"this.style.color = ''; domTT_mouseout(this, event);\" onmouseover=\"domTT_activate(this, event, 'content', '{$line[13]}', 'trail', true, 'delay', 0, 'fade', 'both', 'fadeMax', 87, 'styleClass', 'niceTitle');\">{$line[8]}</td>\n";
						echo "<td class=\"listr\" width=\"*\">{$line[8]}</td>\n";
						echo "</tr>\n";
					}
				}
			break;
			
			case 'error':
				//show table headers
				show_tds(array("DateTime","Severity","Message"));

				//fetch lines
				$logarr=fetch_log($logfile);

				// Print lines
				foreach ($logarr as $logent) {
				// Split line by space delimiter
					$logline  = preg_split("/\n/", $logent);
					/*
					field 1: Wed Jul 04 20:22:28 2012 
					field 2: error 
					field 3: 187.10.53.87 
					field 4: proxy: DNS lookup failure for: 192.168.15.272 returned by / 
					*/
					$regex = '/^\[([^\]]+)\] \[([^\]]+)\] (?:\[client ([^\]]+)\])?\s*(.*)$/i';
					if (preg_match($regex, $logline[0],$line)) {
						// Apply filter and color
						if ($filter != "")
							$line = preg_replace("@($filter)@i","<spam><font color='red'>$1</font></span>",$line);

                        if ($line[3])
                            $line[3] = gettext("Client address:") . " [{$line[3]}]";

						echo "<tr valign=\"top\">\n";
						echo "<td class=\"listlr\" align=\"center\" nowrap>{$line[1]}</td>\n";
						echo "<td class=\"listr\" align=\"center\">{$line[2]}</td>\n";
						echo "<td class=\"listr\" width=\"*\">{$line[3]} {$line[4]}</td>\n";
						echo "</tr>\n";
					}
				}
			break;
		}
	}
}

# ------------------------------------------------------------------------------
# Functions
# ------------------------------------------------------------------------------

// From SquidGuard Package
function html_autowrap($cont)
{
	# split strings
	$p     = 0;
	$pstep = 25;
	$str   = $cont;
	$cont = '';
	for ( $p = 0; $p < strlen($str); $p += $pstep ) {
		$s = substr( $str, $p, $pstep );
		if ( !$s ) break;
			$cont .= $s . "<wbr/>";
	}
	return $cont;
}

// Show Logs
function fetch_log($log){
	global $filter;
	// Get Data from form post
    $lines = $_REQUEST['maxlines'];
    if (preg_match("/!/",htmlspecialchars($_REQUEST['strfilter'])))
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

function show_tds($tds){
	echo "<tr valign='top'>\n";
	foreach ($tds as $td){
		echo "<td class='listhdrr' align='center'>".gettext($td)."</td>\n";
	}
	echo "</tr>\n";
}

?>
