#!/usr/local/bin/php -q
<?php
require_once("config.inc");
require_once("globals.inc");
require_once("notices.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if (($pf_version < 2.1)) {
	$error = "Sending e-mail on this version of pfSense is not supported. Please use pfSense 2.1 or later";
	log_error($error);
	echo "{$error}\n";
	return;
}

$options = getopt("s::");

$message = "";

if($options['s'] <> "") {
	$subject = $options['s'];
}


$in = file("php://stdin");
foreach($in as $line){
	$line = trim($line);
	if (       (substr($line, 0, 6) == "From: ")
		|| (substr($line, 0, 6) == "Date: ")
		|| (substr($line, 0, 4) == "To: "))
		continue;
	if (empty($subject) && (substr($line, 0, 9) == "Subject: ")) {
		$subject = substr($line, 9);
		continue;
	}
	$message .= "$line\n";
}

if (!empty($subject))
	send_smtp_message($message, $subject);
else
	send_smtp_message($message);
?>