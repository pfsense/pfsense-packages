<?php
/*
	pfBlockerNG_Alerts_AR.php

	pfBlockerNG
	Copyright (c) 2015-2016 BBcan177@gmail.com
	All rights reserved.

	Portions of this code are based on original work done for
	pfSense from the following contributors:

	Javascript Hostname Lookup modifications by J. Nieuwenhuizen

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

// Auto-resolve hostnames
if (isset($_REQUEST['getpfhostname'])) {
	$getpfhostname = trim(htmlspecialchars($_REQUEST['getpfhostname']));
	if (strlen($getpfhostname) >= 8) {
		$hostname = htmlspecialchars(gethostbyaddr($getpfhostname), ENT_QUOTES);
	} else {
		$hostname = $getpfhostname;
	}
	if ($hostname == $getpfhostname) {
		$hostname = 'unknown';
	}
	echo $hostname;
}

?>
