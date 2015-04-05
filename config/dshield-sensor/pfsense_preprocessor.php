#!/usr/local/bin/php -q
<?php
/* $Id$ */
/*
	pfsense_preprocessor.php

	Copyright (C) 2015 Robert Nelson
	All rights reserved.
*/

include_once("functions.inc");
include_once("filter_log.inc");

$log = fopen("php://stdin", "r");
$lastline = "";
while(!feof($log)) {
	$line = fgets($log);
	$line = rtrim($line);
	$flent = parse_filter_line(trim($line));
	if ($flent != "") {
		switch ($flent['proto']) {
		case "TCP":
			$flags = (($flent['proto'] == "TCP") && !empty($flent['tcpflags'])) ? ":" . $flent['tcpflags'] : "";
			echo "{$flent['time']} {$flent['act']} {$flent['realint']} {$flent['proto']}{$flags} {$flent['src']} {$flent['dst']}\n";
			break;
		case "ICMP":
		case "ICMP6":
		case "ICMPv6":
			$type = "???";
			$code = "???";
			switch ($flent['icmp_type']) {
			case 'request':
				$type = "8";
				$code = "0";
				break;
			case 'reply':
				$type = "0";
				$code = "0";
				break;
			case 'unreachproto':
				$type = "3";
				$code = "2";
				break;
			case 'unreachport':
				$type = "3";
				$code = "3";
				break;
			case 'unreach':
				$type = "3";
				break;
			case 'timexceed':
				$type = "11";
				$code = "0";
				break;
			case 'paramprob':
				$type = "12";
				break;
			case 'redirect':
				$type = "5";
				break;
			case 'maskreply':
				$type = "18";
				$code = "0";
				break;
			case 'needfrag':
				$type = "3";
				$code = "4";
				break;
			case 'tstamp':
				$type = "13";
				$code = "0";
				break;
			case 'tstampreply':
				$type = "14";
				$code = "0";
				break;
			}
			echo "{$flent['time']} {$flent['act']} {$flent['realint']} {$flent['proto']} {$flent['src']}:{$type} {$flent['dst']}:{$code}\n";
			break;
		default:
			echo "{$flent['time']} {$flent['act']} {$flent['realint']} {$flent['proto']} {$flent['src']} {$flent['dst']}\n";
			break;
		}
		$flent = "";
	}
}
fclose($log); ?>
