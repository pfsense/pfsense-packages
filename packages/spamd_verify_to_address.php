#!/usr/local/bin/php -q
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

if($debug)
        echo "Downloading current valid email list...\n";
/* fetch down the latest list from server */
if($debug) {
    /* fetch without quiet mode */
    exec("fetch -o /tmp/emaillist.txt {$server_to_pull_data_from}");
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

if($fetched_file == "")
        exit(-1);

if($debug) {
    /* echo out all our valid hosts */
    foreach($valid_list as $valid)
        echo "VALID: ||$valid||\n";
}

$current_blacklist = split("\n", `cat /var/db/blacklist.txt`);

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
    if(in_array($server_ip, $current_blacklist)) {
        if($debug)
                echo "$server_ip already in blacklist.\n";
        continue;
    }
    if($debug)
        echo "Testing $email_from | $email_to \n";
    if (in_array($email_to, $valid_list)) {
        if($debug)
            echo "$email_to is in the valid list\n";
    } else {
        /* spammer picked the wrong person to mess with */
        if($server_ip) {
            if($debug)
                    echo "/usr/local/sbin/spamdb -a $server_ip -t\n";
            exec("/usr/local/sbin/spamdb -d {$server_ip} 2>/dev/null");
            exec("/usr/local/sbin/spamdb -d {$server_ip} -T 2>/dev/null");
            exec("/usr/local/sbin/spamdb -d {$server_ip} -t 2>/dev/null");
            if($debug)
                echo "/usr/local/sbin/spamdb -a \"<$email_to>\" -T\n";
            exec("/usr/local/sbin/spamdb -a \"<$email_to>\" -T");
            system("echo $server_ip >> /var/db/blacklist.txt");
            $result = mwexec("/usr/local/sbin/spamdb -a $server_ip -t");
        } else {
            if($debug)
                echo "Could not locate server ip address.";
        }
        if($debug)
            echo "Script result code: {$result}\n";
    }
}

mwexec("killall -HUP spamlogd");

if($debug) {
    echo "Items trapped:              ";
    system("spamdb | grep TRAPPED | wc -l");
    echo "Items spamtrapped:          ";
    system("spamdb | grep SPAMTRAP | wc -l");
}

mwexec("/sbin/pfctl -q -t blacklist -T replace -f /var/db/blacklist.txt");
mwexec("/sbin/pfctl -t blacklist -T show | cut -d\" \" -f4 > /var/db/blacklist.txt");

if($debug) {
        echo "Items in blacklist.txt: ";
        system("/sbin/pfctl -t blacklist -T show | wc -l");
}

?>
