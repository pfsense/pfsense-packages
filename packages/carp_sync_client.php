#!/usr/bin/php
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

if($already_processed != 1)
    if($config['installedpackages']['carpsettings']['config'] != "") {
	$already_processed = 1;
	foreach($config['installedpackages']['carpsettings']['config'] as $carp) {
	    if($carp['synchronizetoip'] <> "" ) {
		/* lets sync! */
		$synchronizetoip = $carp['synchronizetoip'];
		if($carp['synchronizerules'] <> "") {
		    $current_rules_section = backup_config_section("filter");
		    /* generate firewall rules xml */
		    $fout = fopen("{$g['tmp_path']}/filter_section.txt","w");
		    fwrite($fout, $current_rules_section);
		    fclose($fout);
		    mwexec("/usr/bin/scp {$g['tmp_path']}/filter_section.txt root@{$synchronizetoip}:/tmp/");
		    unlink("{$g['tmp_path']}/filter_section.txt");
		}
		if($carp['synchronizenat'] <> "") {
		    $current_nat_section = backup_config_section("nat");
		    /* generate nat rules xml */
		    $fout = fopen("{$g['tmp_path']}/nat_section.txt","w");
		    fwrite($fout, $current_nat_section);
		    fclose($fout);
		    mwexec("/usr/bin/scp {$g['tmp_path']}/nat_section.txt root@{$synchronizetoip}:/tmp/");
		    unlink("{$g['tmp_path']}/nat_section.txt");
		}
		if($carp['synchronizealiases'] <> "") {
		    $current_aliases_section = backup_config_section("aliases");
		    /* generate aliases xml */
		    $fout = fopen("{$g['tmp_path']}/aliases_section.txt","w");
		    fwrite($fout, $current_aliases_section);
		    fclose($fout);
		    mwexec("/usr/bin/scp {$g['tmp_path']}/aliases_section.txt root@{$synchronizetoip}:/tmp/");
		    unlink("{$g['tmp_path']}/aliases_section.txt");
		}
		if($carp['synchronizetrafficshaper'] <> "") {
		    $current_trafficshaper_section = backup_config_section("shaper");
		    /* generate aliases xml */
		    $fout = fopen("{$g['tmp_path']}/shaper_section.txt","w");
		    fwrite($fout, $current_trafficshaper_section);
		    fclose($fout);
		    mwexec("/usr/bin/scp {$g['tmp_path']}/shaper_section.txt root@{$synchronizetoip}:/tmp/");
		    unlink("{$g['tmp_path']}/shaper_section.txt");
		}
		/* copy configuration to remote host */
		mwexec("/usr/bin/ssh {$synchronizetoip} /usr/local/pkg/carp_sync_server.php");
	    }
	}
    }

