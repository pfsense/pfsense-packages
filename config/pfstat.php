#!/usr/local/bin/php
<?php
/*
    pkg.php
    Copyright (C) 2004 Scott Ullrich
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

function gentitle_pkg($pgname) {
	global $config;
	return $config['system']['hostname'] . "." . $config['system']['domain'] . " - " . $pgname;
}

$pgtitle = "Firewall: NAT: Port Forward";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php
include("fbegin.inc");
?>
<p class="pgtitle"><?=$title?></p>
<?php if ($savemsg) print_info_box($savemsg); ?>

<?php
if($config['installedpackages']['pfstat']['config'] <> "") {
  foreach($config['installedpackages']['pfstat']['config'] as $graph) {
	echo "<table BORDERCOLOR=\"#990000\" width=\"100%\" border=\"1\" cellpadding=\"0\" cellspacing=\"0\">";
	echo "<tr bgcolor='#990000'><td><center><font color='white'>" . $graph['graphname'] . "</td></tr>\n";
	echo "<td><center><table width=\"100%\"><tr><td width='100%'>";
	echo "<center><br><a href=\"/pfstat/" . $graph['imagename'] . "\"><img border=\"0\" width=\"500\" \"height\" src='/pfstat/" . $graph['imagename'] . "'></a>";
	echo "</td></tr>";
	echo "<tr bgcolor='#990000'><td><center><font color='white'>" . $graph['description'] . "</td></tr>";
	echo "</table>\n";
	echo "</td></tr>\n";
	echo "</table>&nbsp;<br>";
  }
} else {

echo "<center>There are currently no graphs defined.";

}
?>

<?php include("fend.inc"); ?>
</body>
</html>
