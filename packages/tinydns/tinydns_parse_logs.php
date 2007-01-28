#!/usr/local/bin/php
<?php

/* $Id$ */
/*
	tinydns_parse_logs.inc
	Copyright (C) 2006 Scott Ullrich
	part of pfSense
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

$query_type = array(
        "0001" => "A",
        "0002" => "NS",
        "0005" => "CNAME",
        "0006" => "SOA",
        "000c" => "PTR",
        "000f" => "MX",
        "0010" => "TXT",
        "001c" => "AAAA",
        "0021" => "RT",
        "0026" => "A6",
        "00fb" => "IXFR",
        "00fc" => "AXFR",
        "00ff" => "*"
					);

$results = array(
     	"+" => "responded",
        "-" => "not_authority",
        "I" => "not_implemented/invalid",
        "C" => "wrong_class",
        "/" => "not_parsed"
		);

$fp = fopen('php://stdin', 'r');

/* loop through stdin and process text */
while (!feof($fp)) {
	$stdintext = chop(fgets($fp));
	preg_match('/^(\S+ \S+) ([^:]+):([^:]+):([^:]+) (\S+) (\S+) (\S+)$/', $stdintext, $items);
	$stamp = $items[1];
	$rawip = $items[2];
	$port = $items[3];
	$id = $items[4];
	$result = $items[5];
	$type = $items[6];
	$name = $items[7];
	if (isset($query_type[$type]))
		$qtype = $query_type[$type];
	else
		$qtype = $type;
	$desc = $results[$result];
	$ip = decodeipaddress($rawip);
	//echo "RAWIP: $rawip $ip";
	printf("%s %15.15s:%-4.4s %-8.8s %-24.24s %s\n",$stamp,$ip,hexdec($port),$qtype,$desc,$name);
}

function decodeipaddress($text) {
	preg_match('/(..)(..)(..)(..)/', $text, $hexitems);
	$ipaddr = "";
	unset($hexitems[0]);
	$isfirst = true;
	foreach($hexitems as $hexitem) {
		if(!$isfirst)
			$ipaddr .= ".";
		$ipaddr .= hexdec($hexitem);
		$isfirst = false;
	}
	return $ipaddr;
}

fclose($fp);

?>