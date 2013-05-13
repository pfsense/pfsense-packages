#!/usr/local/bin/php -q
<?php

$NONINTERACTIVE_SCRIPT = TRUE;

$fp = fopen('php://stdin', 'r');
while($args = split(" ",trim(fgets($fp, 4096)))){
   print captive_ip_to_username($args);
}

function captive_ip_to_username($args){
   $current_sessions = file("/var/db/captiveportal.db");
   foreach($current_sessions as $session){
        list($a, $b, $IP_Address, $Mac_Address, $Username) = explode(",", $session,5);
        #this test allow access if user's ip is listed on captive portal
        #args array has (ip, site, protocol and port) passed by squid helper
        #include a more complex test here to allow or deny access based on username returned
        # this script will not return username to squid logs 
        if($IP_Address == $args[0]) return "OK\n";
        }
   return "ERR\n";
}

?>
