#!/usr/local/bin/php -q
<?php
/*
	openssl dhparams wrapper for pfSense.
	
	gen_dhparams.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2015 Gekkenuhis
	Copyright (C) 2015 ESF, LLC
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

// General parameter validation
if (($argc != 3 && $argc != 5) || in_array($argv[1], array('--help', '--h')) || !in_array($argv[1], array(1024, 2048, 4096))) {
	showHelp();
	return;
}

// Validate the user and group
if ($argc == 5 && (posix_getpwnam($argv[3]) == FALSE || posix_getgrnam($argv[4]) == FALSE)) {
	echo "Error: invalid user or group\r\n";
	echo "\r\n";
	showHelp();
	return;
}

run($argc, $argv);

function run($argc, $argv) {
	require_once("config.inc");
	require_once("pfsense-utils.inc");
		
	$dhfile = $argv[2];
	$pathinfo = pathinfo($dhfile);
	
	if (!empty($dhfile) && !empty($pathinfo["dirname"]) && !empty($pathinfo["basename"]) && is_dir($pathinfo["dirname"])) {
		echo "Generating a DH Parameter of " . $argv[1] . " bits.\r\n";
		
		// Mount the filesystem rw
		conf_mount_rw();
		
		// Get a temp file
		$tmpfile = get_tmp_file();
		
		// Generate the DHParams at a lower priority
		exec("/usr/bin/nice -n -1 openssl dhparam -out {$tmpfile} {$argv[1]} >/dev/null 2>&1");
		// Move the temp file to the destination
		exec("mv {$tmpfile} {$dhfile}");
		
		// Check the file exists
		if (file_exists($dhfile)) {
			// Check or the ownership needs to be set
			if($argc == 5) {
				chown($dhfile, $argv[3]);
				chgrp($dhfile, $argv[4]);
			}
			
			// Set the file permissions
			chmod($dhfile, 0644);
		} else {
			echo "Target file not found..\r\n";
		}
		
		// Mount the filesystem ro
		conf_mount_ro();
		echo "Done.";
	} else {
		echo "Error: Invalid destination path.\r\n";
	}
}

function showHelp() {
	echo "Usage: gen_dhparams.php <numbits> <path> (<user> <group>)\r\n";
	echo "Genetares a DH Parameter file.\r\n";
	echo "\r\n";
	echo "<numbits>\t= 1024, 2048 or 4096 bits.\r\n";
	echo "<path>\t\t= The location to store the DH Parmeter file.\r\n";
	echo "<user>\t\t= The user that will own the file. (optional)\r\n";
	echo "<group>\t\t= The group that will own the file. (optional)\r\n";
	echo "--help, --h\t= This help.\r\n";
	return;
}

?>