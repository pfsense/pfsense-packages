<?php
/*
	pre2upgrade.php
	Copyright (C) 2011 Jim Pingle
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
require_once("guiconfig.inc");

$pgtitle = "Diagnostics: Pre 2.0 Upgrade Check";
include("head.inc");

exec("/usr/local/bin/xmllint /conf/config.xml 2>&1", $out, $err);

if ($err) {
	$out = implode("\n", $out);
} else {
	$out = "OK";
}
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php
include("fbegin.inc");
?>
<p class="pgtitle"><?=$pgtitle?></p>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<div id="mainarea">
<table class="tabcont" colspan="3" cellpadding="3" width="100%">
<tr><td>
This configuration check will report any invalid XML. The most common cause of this is international/special characters inside of your configuration in places where they are not supported. You must remove these characters from your configuration before proceeding with the upgrade, or else you <b>will</b> have problems, as your config.xml is not well-formed. Once you have upgraded to 2.0 you can put the characters back in descriptions, as they are properly supported in the new format.
<br/><br/>
Config check output:
<font size="2">
<pre>
<?php echo htmlspecialchars($out); ?>
</pre>
</font>
<?php if ($err): ?>
<font color="red"><b>Please fix the errors found above.</b></font><br/>It may help to view a <a href="/diag_backup.php">config.xml backup file</a> to see where the characters are exactly.
<?php endif; ?>
<br/><br/>
Before proceeding with the upgrade, you should look over the upgrade guide on the doc wiki, which can be found here:<br/>
<a href="https://doc.pfsense.org/index.php/Upgrade_Guide">https://doc.pfsense.org/index.php/Upgrade_Guide</a>.
</td></tr>
</table>
</div>
</td></tr>
</table>

<?php include("fend.inc"); ?>
</body>
</html>