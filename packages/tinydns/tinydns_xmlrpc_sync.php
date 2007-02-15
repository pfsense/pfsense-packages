<?php

/* $Id$ */
/*
	tinydns_xmlrcpc_sync.php
	Copyright (C) 2006 Scott Ullrich
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

require_once("/usr/local/pkg/tinydns.inc");
require_once("service-utils.inc");

if(!$config) {
 	log_error("\$config is not enabled from tinydns_xmlrpc_sync.php!!");
} else {
	if(!$g['booting'])
		tinydns_do_xmlrpc_sync();
}

	if($config['installedpackages']['carpsettings']['config'])
		$password = $config['installedpackages']['carpsettings']['config'][0]['password'];
	if($config['installedpackages']['carpsettings']['config'])
		$syncip = $config['installedpackages']['carpsettings']['config'][0]['synchronizetoip'];
	if($config['installedpackages']['carpsettings']['config'])
		$syncxmlrpc = $config['installedpackages']['tinydns']['config'][0]['syncxmlrpc'];
	
	/* option enabled? */
	if($syncxmlrpc)
		if($syncip)
			if($password)
				tinydns_do_xmlrpc_sync($syncip, $password)

?>