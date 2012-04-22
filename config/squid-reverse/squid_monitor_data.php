<?php 
/* $Id$ */
/* ========================================================================== */
/*
    squid_monitor_data.php
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
if ($_POST) {
    switch (strtolower($_POST['program'])) {
       case 'squid':
            showSquid();            
       break;
       case 'sguard';
            showSGuard();
       break;
    }
}



// Show Squid Logs
function showSquid() {
    echo "<tr>";
    echo "<td class=\"listhdrr\">Date</td>";
    echo "<td class=\"listhdrr\">IP</td>";
    echo "<td class=\"listhdrr\">Status</td>";
    echo "<td class=\"listhdrr\">Address</td>";
    echo "<td class=\"listhdrr\">User</td>";
    echo "<td class=\"listhdrr\">Destination</td>";
    echo "</tr>";
    
    // Get Data from form post
    $lines = $_POST['maxlines'];
    $filter = $_POST['strfilter'];

    if ($filter != "") {
        $exprfilter = "| grep -i $filter";
    } else {
        $exprfilter = "";
    }

    // TODO FIX:
    // Remove the hard link (maybe, get from config)
    //
    exec("tail -r -n $lines /var/squid/logs/access.log $exprfilter",$logarr);

    foreach ($logarr as $logent) { 
        $logline  = preg_split("/\s+/", $logent);

        if ($filter != "")
            $logline = preg_replace("/$filter/","<spam style='color:red'>$filter</spam>",$logline);

        echo "<tr>\n";
        echo "<td class=\"listr\">".date("d/m/y H:i:s",$logline[0])."</td>\n";
        echo "<td class=\"listr\">".$logline[2]."</td>\n";
        echo "<td class=\"listr\">".$logline[3]."</td>\n";
        echo "<td class=\"listr\" nowrap>".$logline[6]."</td>\n";
        echo "<td class=\"listr\">".$logline[7]."</td>\n";
        echo "<td class=\"listr\">".$logline[8]."</td>\n";
        echo "</tr>\n";
    }
}

// Show SquidGuard Logs
function showSGuard() {


	echo "<tr>";
	echo "<td class=\"listhdrr\">Date</td>";
	echo "<td class=\"listhdrr\">Hour</td>";
	echo "<td class=\"listhdrr\">ACL</td>";
	echo "<td class=\"listhdrr\">Address</td>";
	echo "<td class=\"listhdrr\">Host</td>";
	echo "<td class=\"listhdrr\">User</td>";
	echo "</tr>";


    // Get Data from form post
    $lines = $_POST['maxlines'];
    $filter = $_POST['strfilter'];

    if ($filter != "") {
        $exprfilter = "| grep -i $filter";
    } else {
        $exprfilter = "";
    }

    // TODO FIX:
    // Remove the hard link (maybe, get from config)
    //
    exec("tail -r -n $lines /var/squidGuard/log/block.log $exprfilter",$logarr);

    foreach ($logarr as $logent) { 
        $logline  = preg_split("/\s+/", $logent);

        if ($filter != "")
            $logline = preg_replace("/$filter/","<spam style='color:red'>$filter</spam>",$logline);

        echo "<tr>\n";
        echo "<td class=\"listr\">".$logline[0]."</td>\n";
        echo "<td class=\"listr\">".$logline[1]."</td>\n";
        echo "<td class=\"listr\">".$logline[3]."</td>\n";
        echo "<td class=\"listr\">".$logline[4]."</td>\n";
        echo "<td class=\"listr\">".$logline[5]."</td>\n";
        echo "<td class=\"listr\">".$logline[6]."</td>\n";
        echo "</tr>\n";
    }
}

?>
