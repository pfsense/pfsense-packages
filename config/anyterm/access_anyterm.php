<?php
/*
    access_anyterm.php
	pfSense package (http://www.pfSense.com)
	Copyright (C) 2009 Scott Ullrich <sullrich@pfsense.org>
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

require("guiconfig.inc");

if($config['installedpackages']['anyterm']['config'][0]['stunnelport']) {
	$port = $config['installedpackages']['anyterm']['config'][0]['stunnelport'];
	$httpors = "https";
} elseif ($config['installedpackages']['anyterm']['config'][0]['port']) {
	$port = $config['installedpackages']['anyterm']['config'][0]['port'];
	$httpors = "http";
} else {
	/* No port defined, redirect to Anyterm settings for now */
	Header("/pkg_edit.php?xml=anyterm.xml&id=0");
}

if (is_alias($port) && alias_get_type($name) == "port")
	$port = alias_expand($port);

if (is_numericint($port) && $port <= 65535) {
	$location = "{$_SERVER['SERVER_ADDR']}:{$port}/anyterm.html";
	Header("Location: {$httpors}://{$location}");
} else {
	/* Port defined but not valid, redirect to Anyterm settings for now */
	Header("/pkg_edit.php?xml=anyterm.xml&id=0");
}

?>
