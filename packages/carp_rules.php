
/*
	carp_rules.inc
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

/* return if there are no carp configured items */
if($config['installedpackages']['carp']['config'] <> "") {
    mwexec("/sbin/pfctl -a carp -Fr");
    /* carp records exist, lets process */
    $wan_interface = get_real_wan_interface();
    $i = 0;
    $ifdescrs = array('wan', 'lan');
    for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
	$ifdescrs['opt' . $j] = "opt" . $j;
    }
    foreach ($ifdescrs as $ifdescr => $ifname) {
	$interface = convert_friendly_interface_to_real_interface_name($ifname);
	add_rule_to_anchor("carp", "pass quick on {$interface} proto carp keep state", $interface . "carp41");
    }
    if($config['installedpackages']['carp']['config'] <> "")
	foreach($config['installedpackages']['carp']['config'] as $carp) {
	    $ip = $carp['ipaddress'];
	    $int = find_ip_interface($ip);
	    $carp_int = find_carp_interface($ip);
	    add_rule_to_anchor("carp", "pass quick on {$carp_int} proto carp from {$carp_int}:network to 224.0.0.18 keep state \(no-sync\)", $carp_int . "24");
	    add_rule_to_anchor("carp", "pass out quick on {$carp_int} keep state", $carp_int . "1");
	    add_rule_to_anchor("carp", "pass quick on {$carp_int} proto carp keep state", $carp_int . "41");
	    if($int <> false and $int <> $wan_interface) {
		$ipnet = convert_ip_to_network_format($ip, $carp['netmask']);
		$rule = "nat on {$int} inet from {$ipnet} to any -> ({$carp_int}) \n";
		add_rule_to_anchor("natrules", $rule, $ip);
	    }
	}
    add_rule_to_anchor("carp", "pass quick on pfsync0 keep state", "pfsync0" . "3");
    if($config['installedpackages']['carp']['config'] <> "")
	foreach($config['installedpackages']['carpsettings']['config'] as $carp)
	    $carp_sync_int = convert_friendly_interface_to_real_interface_name($carp['pfsyncinterface']);
    if($carp_sync_int <> "") {
	add_rule_to_anchor("carp", "pass quick on {$carp_sync_int} proto pfsync", $carp_sync_int . "3");
    }
}