#!/usr/local/bin/php

<?php

/*
	carp_sync.php
        part of pfSense (www.pfSense.com)
	Copyright (C) 2004 Scott Ullrich (sullrich@gmail.com)
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

require_once("config.inc");
require_once("xmlparse_pkg.inc");
require_once("filter.inc");

if($config['installedpackages']['carpsettings']['config'] != "")
    foreach($config['installedpackages']['carpsettings']['config'] as $carp)
	if($carp['synchronizerules'] <> "") {
	    $rules = return_filename_as_string("{$g['tmp_path']}/rules_section.txt");
	    $aliases = return_filename_as_string("{$g['tmp_path']}/aliases_section.txt");
	    $nat = return_filename_as_string("{$g['tmp_path']}/nat_section.txt");
	    $trafficshaper = return_filename_as_string("{$g['tmp_path']}/trafficshaper_section.txt");
	    if($rules <> "") {
		restore_config_section("filter", $rules);
		unlink("{$g['tmp_path']}/rules_section.txt");
	    }
	    if($aliases <> "") {
		restore_config_section("aliases", $aliases);
		unlink("{$g['tmp_path']}/aliases_section.txt");
	    }
	    if($nat <> "") {
		restore_config_section("nat", $nat);
		unlink("{$g['tmp_path']}/nat_section.txt");
	    }
	    if($trafficshaper <> "") {
	        restore_config_section("shaper", $trafficshaper);
		unlink("{$g['tmp_path']}/nat_section.txt");
	    }
	    filter_configure();
	}

?>