#!/usr/local/bin/php -q
<?php
require_once("config.inc");
require_once("globals.inc");
require_once("sasl.inc");
require_once("smtp.inc");

$message = "";
$subject = "";
$to = "";
$cc = "";
$bcc = "";
$replyto = "";

$isheader = true;

$in = file("php://stdin");
foreach ($in as $line) {
	$line = rtrim($line);
	if ($line == "") {
		$isheader = false;
		continue;
	}
	if ($isheader) {
		if (preg_match('/^([^:]+): (.*)$/', $line, $matches)) {
			switch ($matches[1]) {
			case "From":
				break;
			case "Subject":
				$subject = $matches[2];
				break;
			case "To":
				$to = $matches[2];
				break;
			case "Cc":
				$cc = $matches[2];
				break;
			case "Bcc":
				$bcc = $matches[2];
				break;
			case "Reply-To":
				$replyto = $matches[2];
				break;
			}
		}
	} else {
		$message .= "$line\n";
	}
}

send_mail_message($message, $subject, $to, $cc, $bcc, $replyto);

function send_mail_message($message, $subject, $to, $cc, $bcc, $replyto) {
	global $config, $g;

	if (!$config['notifications']['smtp']['ipaddress'])
		return;

	if (!$config['notifications']['smtp']['notifyemailaddress'])
		return;

	$smtp = new smtp_class;

	$smtp->host_name = $config['notifications']['smtp']['ipaddress'];
	$smtp->host_port = empty($config['notifications']['smtp']['port']) ? 25 : $config['notifications']['smtp']['port'];

	$smtp->direct_delivery = 0;
	$smtp->ssl = (isset($config['notifications']['smtp']['ssl'])) ? 1 : 0;
	$smtp->tls = (isset($config['notifications']['smtp']['tls'])) ? 1 : 0;
	$smtp->debug = 0;
	$smtp->html_debug = 0;
	$smtp->localhost=$config['system']['hostname'].".".$config['system']['domain'];

	if (isset($config['notifications']['smtp']['fromaddress'])) {
		$from = $config['notifications']['smtp']['fromaddress'];
	} else {
		$from = "pfsense@{$config['system']['hostname']}.{$config['system']['domain']}";

	}

	// Use SMTP Auth if fields are filled out
	if (isset($config['notifications']['smtp']['username']) &&
	   isset($config['notifications']['smtp']['password'])) {
		if (isset($config['notifications']['smtp']['authentication_mechanism'])) {
			$smtp->authentication_mechanism = $config['notifications']['smtp']['authentication_mechanism'];
		} else {
			$smtp->authentication_mechanism = "PLAIN";
		}
		$smtp->user = $config['notifications']['smtp']['username'];
		$smtp->password = $config['notifications']['smtp']['password'];
	}

	$headers = array(
		"From: {$from}",
		"To: {$to}"
	);

	$recipients = preg_split('/\s*,\s*/', trim($to));

	if ($cc <> "") {
		$headers[] = "Cc: {$cc}";
		$recipients = array_merge($recipients, preg_split('/\s*,\s*/', trim($cc)));
	}

	if ($bcc <> "") {
		$recipients = array_merge($recipients, preg_split('/\s*,\s*/', trim($bcc)));
	}

	if ($replyto <> "") {
		$headers[] = "Reply-To: {$replyto}";
	}

	$headers[] = "Subject: {$subject}";
	$headers[] = "Date: ".date("r");

	if ($smtp->SendMessage($from, $recipients, $headers, $message)) {
		log_error(sprintf(gettext("Message sent to %s OK"), $to));
		return;
	} else {
		log_error(sprintf(gettext('Could not send the message to %1$s -- Error: %2$s'), $to, $smtp->error));
		return(sprintf(gettext('Could not send the message to %1$s -- Error: %2$s'), $to, $smtp->error));
	}
}
?>
