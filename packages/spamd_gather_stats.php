<?php

/* read in spamd log file */
if(file_exists("/var/log/spamd.log"))
    $log_array = split("\n", file_get_contents("/var/log/spamd.log"));

/* read in our mini database of values */
if(file_exists("/var/log/spamd_rrd_stats.txt"))
    $rrd_stats = split("\n", file_get_contents("/var/log/spamd_rrd_stats.txt"));

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
        //echo "Connect time: $connect_time\n";
    }
}

$open_connections = 0;
$average_connect_time = 0;

$total_connections = count($connect_times);
//echo "Total connections: $total_connections\n";

/* loop through, how many connections are open */
foreach($connections as $c) {
    if($c == true)
        $open_connections++;
}

/* loop through, how many connections are open */
foreach($connect_times as $c) {
    $average_connect_time = $average_connect_time + $c;
}

echo $open_connections;
echo ":";
echo round(($average_connect_time / $total_connections));

exit;

?>