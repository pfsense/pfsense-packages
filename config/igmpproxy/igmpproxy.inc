<?php
/*
    igmpproxy.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2009 Ermal Luçi
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

function igmpproxy_install() {
	
	if (file_exists("/tmp/firewall_rules_edit.tmp"))
		exec("/bin/cp /tmp/firewall_rules_edit.tmp /usr/local/www/firewall_rules_edit.php");
	if (file_exists("/tmp/filter.tmp"))
		exec("/bin/cp /tmp/filter.tmp /etc/inc/filter.inc");
}

function igmpproxy_resync() {
        global $config, $g;

        /* kill any running igmpproxy */
        killbyname("igmpproxy");

        $igmpconf = <<<EOD

##------------------------------------------------------
## Enable Quickleave mode (Sends Leave instantly)
##------------------------------------------------------
quickleave

EOD;

	if (!is_array($config['installedpackages']['igmpproxy']['config']))
		return 0;

	$igmpcfg =& $config['installedpackages']['igmpproxy']['config'];
        foreach ($igmpcfg as $igmpcf) {
                $realif = convert_friendly_interface_to_real_interface_name($igmpcf['igmpname']);
                if (empty($igmpcf['igmpthreshold']))
                        $threshld = 1;
                else
                        $threshld = $igmpcf['igmpthreshold'];
                $igmpconf .= "phyint {$realif} {$igmpcf['igmptype']} ratelimit 0 threshold {$threshld}\n";

		if (is_array($igmpcf['row'])) {
			foreach ($igmpcf['row'] as $igmpaddr)
                                $igmpconf .= "altnet {$igmpaddr['igmpnetaddr']}\n";
		}
                $igmpconf .= "\n";
        }

        $igmpfl = fopen($g['tmp_path'] . "/igmpproxy.conf", "w");
        if (!$igmpfl) {
                log_error("Could not write Igmpproxy configuration file!");
                return;
        }
        fwrite($igmpfl, $igmpconf);
        fclose($igmpfl);

        mwexec("/usr/local/sbin/igmpproxy -c " . $g['tmp_path'] . "/igmpproxy.conf");
        log_error("Started Igmpproxy service sucsesfully.");

        return 0;
}

?>
