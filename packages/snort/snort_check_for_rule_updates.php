#!/usr/local/bin/php -f
<?php

/* $Id$ */
/*
	snort_check_for_rule_updates.php
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

$console_mode = true;

require_once("config.inc");
require_once("functions.inc");
require_once("/usr/local/pkg/snort.inc");
require_once("service-utils.inc");

$last_ruleset_download = $config['installedpackages']['snort']['last_ruleset_download'];
$text = file_get_contents("http://www.snort.org/pub-bin/downloads.cgi");
if (preg_match_all("/.*RELEASED\: (.*)\</", $text, $matches))
        $last_update_date = trim($matches[1][0]);
$date1ts = strtotime($last_update_date);
$date2ts = strtotime($last_ruleset_download);
/* is there a newer ruleset available? */
if($date1ts > $date2ts or !$last_ruleset_download) {
	if(!$oinkid) {
		log_error("Oinkid is not defined.  We cannot automatically update the ruleset.");
		echo "Oinkid is not defined.  We cannot automatically update the ruleset.";
		exit;
	}
	echo "Downloading snort rule updates...";
	/* setup some variables */
	$snort_filename = "snortrules-snapshot-CURRENT.tar.gz";
	$snort_filename_md5 = "snortrules-snapshot-CURRENT.tar.gz.md5";
	$dl = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename}";
	$dl_md5 = "http://www.snort.org/pub-bin/oinkmaster.cgi/{$oinkid}/{$snort_filename_md5}";

	/* multi user system, request new filename and create directory */
	$tmpfname = tempnam("/tmp", "snortRules");
	exec("/bin/rm -rf {$tmpfname};/bin/mkdir -p {$tmpfname}");

	/* download snort rules */
	exec("fetch -q -o {$tmpfname}/{$snort_filename} $dl");
	verify_downloaded_file($tmpfname . "/{$snort_filename}");

	/* download snort rules md5 file */
	$static_output = gettext("Downloading current snort rules md5... ");
	exec("fetch -q -o {$tmpfname}/{$snort_filename_md5} $dl_md5");
	verify_downloaded_file($tmpfname . "/{$snort_filename_md5}");

	/* verify downloaded rules signature */
	verify_snort_rules_md5($tmpfname);

	/* extract rules */
	extract_snort_rules_md5($tmpfname);

	$config['installedpackages']['snort']['last_ruleset_download'] = date("Y-m-d");
	write_config();

	stop_service("snort");
	sleep(2);
	start_service("snort");

	/* cleanup temporary directory */
	exec("/bin/rm -rf {$tmpfname};");
	echo "Rules are now up to date.\n";
} else {
	echo "Rules are up to date.\n";
}

?>