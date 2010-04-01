<?php

/* $Id$ */
/*
	snort_dynamic_ip_reload.php
	Copyright (C) 2009 Robert Zeleya
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

/* NOTE: this file gets included from the pfSense filter.inc plugin process */
/* NOTE: file location /usr/local/pkg/pf, all files in pf dir get exec on filter reloads */

require_once("/usr/local/pkg/snort/snort.inc");

/* get the varibles from the command line */
/* Note: snort.sh sould only be using this */
//$id = $_SERVER["argv"][1];
//$if_real = $_SERVER["argv"][2];

//$test_iface = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];

//if ($id == "" || $if_real == "" || $test_iface == "") {
//	exec("/usr/bin/logger -p daemon.info -i -t SnortDynIP \"ERORR starting snort_dynamic_ip_reload.php\"");
//	exit;
//	}

sync_snort_package_empty();

?>