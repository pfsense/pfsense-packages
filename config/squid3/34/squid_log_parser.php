#!/usr/local/bin/php -q
<?php
/* ========================================================================== */
/*
	squid_log_parser.php
	part of pfSense (http://www.pfSense.com)
	Copyright (C) 2012-2014 Marcello Coutinho
	Copyright (C) 2012-2014 Carlos Cesario - carloscesario@gmail.com
	All rights reserved.
                                                                              */
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

# ------------------------------------------------------------------------------
# Simple Squid Log parser to rewrite line with date/time human readable
# Usage:  cat /var/squid/log/access.log | parser_squid_log.php
# ------------------------------------------------------------------------------

$logline = fopen("php://stdin", "r");
while(!feof($logline)) {
	$line = fgets($logline);
	$line = rtrim($line);
	if ($line != "") {
		$fields = explode(' ', $line);
		// Apply date format
		$fields[0] = date("d.m.Y H:i:s",$fields[0]);
		foreach($fields as $field) {
		  // Write the Squid log line with date/time human readable
		  echo "{$field} ";
		}
		echo "\n";
	}
}
fclose($logline);
?>