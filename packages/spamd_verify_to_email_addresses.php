<?php

/*
 *    pfSense spamd mousetrap
 *    (C)2006 Scott Ullrich
 *    
 *    Reads in an external list of c/r
 *    seperated valid e-mail addresses
 *    and then looks to see waiting grey-
 *    listed servers.   if the server is
 *    sending to an invalid e-mail address
 *    then add them to spamtrap.
 *
 *    XXX:
 *        * Add flag to blacklist a server after receiving X
 *          attempts at a delivery with invalid to: addresses.
 *         
 */
  
require("config.inc");
require("functions.inc");

/* path to script that outputs c/r seperated e-mail addresses */
$server_to_pull_data_from = "http://10.0.0.11/exchexp.asp";

/* to enable debugging, change false to true */
$debug = true;

/* fetch down the latest list from server */
if($debug) {
    /* fetch without quiet mode */
    exec("fetch $quiet -o /tmp/emaillist.txt {$server_to_pull_data_from}");
} else {
    /* fetch with quiet mode */
    exec("fetch -q -o /tmp/emaillist.txt {$server_to_pull_data_from}");
}

/* test if file exists, if not, bail. */
if(!file_exists("/tmp/emaillist.txt")) {
    if($debug)
        echo "Could not fetch $server_to_pull_data_from\n";
    exit;
}

/* clean up and split up results */
$fetched_file = strtolower(file_get_contents("/tmp/emaillist.txt"));
$valid_list = split("\n", $fetched_file);
$grey_hosts = split("\n", `spamdb | grep GREY`);

if($debug) {
    /* echo out all our valid hosts */
    foreach($valid_list as $valid) 
        echo "VALID: ||$valid||\n";
}

/* traverse list and find the dictionary attackers, etc */
foreach($grey_hosts as $grey) {
    if(trim($grey) == "")
        continue;
    /* clean up and further break down values */
    $grey_lower = strtolower($grey);
    $grey_lower = str_replace("<","",$grey_lower);
    $grey_lower = str_replace(">","",$grey_lower);
    $grey_split = split("\|", $grey_lower);
    $email_from = strtolower($grey_split[2]);
    $email_to   = strtolower($grey_split[3]);
    $server_ip  = strtolower($grey_split[1]);
    if($debug) 
        echo "Testing $email_from | $email_to \n";
    if (in_array($email_to, $valid_list)) {
        if($debug) 
            echo "$email_to is in the valid list\n";
    } else {
        /* spammer picked the wrong person to mess with */
        if($server_ip) {
            echo "/usr/local/sbin/spamdb -T -a $server_ip\n";
            $result = exec("/usr/local/sbin/spamdb -T -a $server_ip\n");
        } else {
            if($debug) 
                echo "Could not locate server ip address.";
        }
        if($debug) 
            echo "Script result code: {$result}\n";
    }
}

?>