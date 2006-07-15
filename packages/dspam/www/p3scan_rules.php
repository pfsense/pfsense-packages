/*
	p3scan_rules.inc
  part of pfSense (www.pfSense.com)
	Copyright (C) 2006 Daniel S. Haischt
	All rights reserved.

*/
$wanif = get_real_wan_interface();
$anchor = "natearly";
$natrules .= "rdr pass on {$wanif} proto tcp from <p3scan> to port pop3 -> 127.0.0.1 port 8110\n";
$label = "p3scan";
add_rule_to_anchor($anchor, $rule, $label);
