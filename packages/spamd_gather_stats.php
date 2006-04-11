#!/usr/local/bin/php -q

<?php
/* $Id$ */
/*
	spamd_gather_stats.php
	Copyright (C) 2006 Scott Ullrich
	All rights reserved.

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

/* read in spamd log file */
if(file_exists("/var/log/spamd.log"))
    $log_array = split("\n", file_get_contents("/var/log/spamd.log"));

/* variable to keep track of connections */
$connections = array();

/* array to track average connection time */
$connect_times = array();

foreach($log_array as $la) {
    /* no watson, this is not the city of angels */
    if (preg_match("/.*spamd\[.*\]\:\s(.*)\: connected\s\((.*)\/(.*)\)/", $la, $matches)) {
        /* we matched a connect */
        $ip = $matches[1];
        $current_connections = $matches[2];
        $max_connections = $matches[2];
        $connections[$ip] = false;
    } else if (preg_match("/.*spamd\[.*\]\:\s(.*)\: disconnected\safter\s(.*)\sseconds\./", $la, $matches)) {
        /* we matched a disconnect */
        $ip = $matches[1];
        $connect_time = $matches[2];
        $connections[$ip] = true;
        $connect_times[$ip] = $connect_time;
    }
}

$open_connections = 0;
$average_connect_time = 0;

$total_connections = count($connect_times);

/* loop through, how many connections are open */
foreach($connections as $c) {
    if($c == true)
        $open_connections++;
}

/* loop through, how many connections are open */
foreach($connect_times as $c) {
    $average_connect_time = $average_connect_time + $c;
}

echo "N:";
echo $open_connections;
echo ":";
echo round(($average_connect_time / $total_connections));

exit;

?>