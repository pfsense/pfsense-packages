<?php 
/* $Id$ */
/* ========================================================================== */
/*
    squid_monitor.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 ccesario @ pfsense forum
    All rights reserved.

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
if ($_POST) {
	# Actions
    switch (strtolower($_POST['program'])) {
       case 'squid':
            showSquid();            
       break;
       case 'sguard';
            showSGuard();
       break;
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


// Show Squid Logs
function showSquid() {
    // Define log file
    $squid_log='/var/squid/logs/access.log';

	echo "<tr valign=\"top\">\n";    
	echo "<td class=\"listhdrr\">".gettext("Date")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("IP")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("Status")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("Address")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("User")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("Destination")."</td>\n";
	echo "</tr>\n";

    // Get Data from form post
    $lines = $_POST['maxlines'];
    $filter = $_POST['strfilter'];


    // Get logs based in filter expression 
    if($filter != "") {
        exec("tail -r -n $lines $squid_log | php -q parser_squid_log.php | grep -i ". escapeshellarg(htmlspecialchars($filter)), $logarr); 
    }
    else {
        exec("tail -r -n $lines $squid_log | php -q parser_squid_log.php", $logarr);
    }

    // Print lines
    foreach ($logarr as $logent) { 
        // Split line by space delimiter
		$logline  = preg_split("/\s+/", $logent);

        // Apply date format to first line
        //$logline[0] = date("d.m.Y H:i:s",$logline[0]);

        // Word wrap the URL 
        $logline[7] = htmlentities($logline[7]);
        $logline[7] = html_autowrap($logline[7]);

        // Remove /(slash) in destination row
		$logline_dest =  preg_split("/\//", $logline[9]);

        // Apply filter and color
		// Need validate special chars
		if ($filter != "")
            $logline = preg_replace("/$filter/i","<spam><font color='red'>$filter</font></span>",$logline);


		echo "<tr valign=\"top\">\n";
		echo "<td class=\"listlr\" nowrap>{$logline[0]} {$logline[1]}</td>\n";
		echo "<td class=\"listr\">{$logline[3]}</td>\n";
		echo "<td class=\"listr\">{$logline[4]}</td>\n";
        echo "<td class=\"listr\" width=\"*\">{$logline[7]}</td>\n";
		echo "<td class=\"listr\">{$logline[8]}</td>\n";
		echo "<td class=\"listr\">{$logline_dest[1]}</td>\n";
		echo "</tr>\n";
    }
}

// Show SquidGuard Logs
function showSGuard() {
    // Define log file
    $sguard_log='/var/squidGuard/log/block.log';

	echo "<tr valign=\"top\">\n";
	echo "<td class=\"listhdrr\">".gettext("Date-Time")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("ACL")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("Address")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("Host")."</td>\n";
	echo "<td class=\"listhdrr\">".gettext("User")."</td>\n";
	echo "</tr>\n";

    // Get Data from form post
    $lines = $_POST['maxlines'];
    $filter = $_POST['strfilter'];

    // Get logs based in filter expression
    if($filter != "") {
        exec("tail -r -n $lines $sguard_log | grep -i ". escapeshellarg(htmlspecialchars($filter)), $logarr);
    }
    else {
        exec("tail -r -n $lines $sguard_log", $logarr);
    }


    // Print lines
    foreach ($logarr as $logent) { 
        // Split line by space delimiter
	    $logline  = preg_split("/\s+/", $logent);

        // Apply time format
        $logline[0] = date("d.m.Y", strtotime($logline[0]));   

    	// Word wrap the URL 
        $logline[4] = htmlentities($logline[4]);
        $logline[4] = html_autowrap($logline[4]);


        // Apply filter color
		// Need validate special chars
        if ($filter != "")
            $logline = preg_replace("/$filter/","<spam><font color='red'>$filter</font></span>",$logline);

        echo "<tr>\n";
        echo "<td class=\"listlr\" nowrap>{$logline[0]} {$logline[1]}</td>\n";
        echo "<td class=\"listr\">{$logline[3]}</td>\n";
		echo "<td class=\"listr\" width=\"*\">{$logline[4]}</td>\n";
        echo "<td class=\"listr\">{$logline[5]}</td>\n";
        echo "<td class=\"listr\">{$logline[6]}</td>\n";
        echo "</tr>\n";
    }
}

?>
