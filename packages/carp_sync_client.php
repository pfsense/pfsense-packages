#!/usr/bin/php
/*
	carp_sync.php
        part of pfSense (www.pfSense.com)
	Copyright (C) 2005 Scott Ullrich (sullrich@gmail.com) and Colin Smith (ethethlay@gmail.com)
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

	TODO:
		* SSL support!

*/

require_once("xmlrpc_client.inc"); // Include client classes from our XMLRPC implementation.
require_once("xmlparse_pkg.inc");  // Include pfSense helper functions.
require_once("config.inc");
require_once("functions.inc");

function carp_sync_xml($url, $password, $section, $section_xml) {
	$params = array(new XML_R

if($already_processed != 1)
    if($config['installedpackages']['carpsettings']['config'] <> "" and
      is_array($config['installedpackages']['carpsettings']['config'])) {
	$already_processed = 1;
	foreach($config['installedpackages']['carpsettings']['config'] as $carp) {
	    if($carp['synchronizetoip'] <> "" ) {
		$synchronizetoip = $carp['synchronizetoip'];
		if($carp['synchronizerules'] <> "" and is_array($config['filter'])) {
		    $current_rules_section = backup_config_section("filter");
		    //$current_rules_section = str_replace("<?xml version=\"1.0\"?>", "", $current_rules_section);
		}
		if($carp['synchronizenat'] <> "" and is_array($config['nat'])) {
		    $current_nat_section = backup_config_section("nat");
		    $current_nat_section = str_replace("<?xml version=\"1.0\"?>", "", $current_nat_section);
		    /* generate nat rules xml */
		    $fout = fopen("{$g['tmp_path']}/nat_section.txt","w");
		    fwrite($fout, $current_nat_section);
		    fclose($fout);
                    $files_to_copy .= " {$g['tmp_path']}/nat_section.txt";
		}
		if($carp['synchronizealiases'] <> "" and is_array($config['aliases'])) {
		    $current_aliases_section = backup_config_section("aliases");
		    $current_aliases_section = str_replace("<?xml version=\"1.0\"?>", "", $current_aliases_section);
		    /* generate aliases xml */
		    $fout = fopen("{$g['tmp_path']}/aliases_section.txt","w");
		    fwrite($fout, $current_aliases_section);
		    fclose($fout);
                    $files_to_copy .= " {$g['tmp_path']}/aliases_section.txt";
		}
		if($carp['synchronizetrafficshaper'] <> "" and is_array($config['shaper'])) {
		    $current_trafficshaper_section = backup_config_section("shaper");
		    $current_trafficshaper_section = str_replace("<?xml version=\"1.0\"?>", "", $current_trafficshaper_section);
		    /* generate aliases xml */
		    $fout = fopen("{$g['tmp_path']}/shaper_section.txt","w");
		    fwrite($fout, $current_trafficshaper_section);
		    fclose($fout);
                    $files_to_copy .= " {$g['tmp_path']}/shaper_section.txt";
		}
		/* copy configuration to remote host */
		/*
		    mwexec("/usr/bin/scp {$files_to_copy} root@{$synchronizetoip}:/tmp/");
		    unlink_if_exists("{$g['tmp_path']}/filter_section.txt");
		    unlink_if_exists("{$g['tmp_path']}/nat_section.txt");
		    unlink_if_exists("{$g['tmp_path']}/aliases_section.txt");
		    unlink_if_exists("{$g['tmp_path']}/shaper_section.txt");
		*/
		/* execute configuration on remote host */
		mwexec("/usr/bin/scp {$files_to_copy} root@{$synchronizetoip}:/tmp/ && /usr/bin/ssh {$synchronizetoip} /usr/local/pkg/carp_sync_server.php &");
	    }
	}
    }

