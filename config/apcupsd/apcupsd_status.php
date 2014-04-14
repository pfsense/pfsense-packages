<?php
/*
	apcupsd_status.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2013 Danilo G. Baio <dbaio@bsd.com.br>
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

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;
	
$pgtitle = "Services: Apcupsd (Status)";
include("head.inc");

function puts( $arg ) { echo "$arg\n"; }	

?>

<style>
<!--

input {
   font-family: courier new, courier;
   font-weight: normal;
   font-size: 9pt;
}

pre {
   border: 2px solid #435370;
   background: #F0F0F0;
   padding: 1em;
   font-family: courier new, courier;
   white-space: pre;
   line-height: 10pt;
   font-size: 10pt;
}

.label {
   font-family: tahoma, verdana, arial, helvetica;
   font-size: 11px;
   font-weight: bold;
}

.button {
   font-family: tahoma, verdana, arial, helvetica;
   font-weight: bold;
   font-size: 11px;
}

-->
</style>
</head>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

	<?php include("fbegin.inc"); ?>
	
	<?php if($one_two): ?>
	<p class="pgtitle"><?=$pgtitle?></font></p>
	<?php endif; ?>


<div id="mainlevel">
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
		<?php
			$tab_array = array();
			$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=apcupsd.xml&amp;id=0");
			$tab_array[] = array(gettext("Status"), true, "/apcupsd_status.php");
			display_top_tabs($tab_array);
		?>
			</td></tr>
		</table>
</div>

<div id="mainarea" style="padding-top: 0px; padding-bottom: 0px; ">
	<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
		<tr><td>
<?php
   puts("<pre>");
   putenv("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
   $ph = popen('apcaccess 2>&1', "r" );
   while ($line = fgets($ph)) echo htmlspecialchars($line);
   pclose($ph);
   puts("</pre>");
?>
		</td></tr>
	</table>
</div>	
<?php 
include("fend.inc");
?>
</body>
</html>
