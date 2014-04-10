<?php
/*
	varnish_view_config.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
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

$pgtitle = "Varnish: View Configuration";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="varnishstat_view_config.php" method="post">
	
<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Backends"), false, "/pkg.php?xml=varnish_backends.xml");
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=varnish_settings.xml&id=0");
	$tab_array[] = array(gettext("Custom VCL"), false, "/pkg_edit.php?xml=varnish_custom_vcl.xml&id=0");
	$tab_array[] = array(gettext("LB Directors"), false, "/pkg.php?xml=varnish_lb_directors.xml");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=varnish_sync.xml&id=0");
	$tab_array[] = array(gettext("View Configuration"), true, "/varnish_view_config.php");
	$tab_array[] = array(gettext("VarnishSTAT"), false, "/varnishstat.php");
	display_top_tabs($tab_array);
?>
		</td></tr>
 		<tr>
    		<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
     						<td class="tabcont" >
									<textarea id="varnishlogs" rows="50" cols="100%">
<?php 
	$config_file = file_get_contents("/var/etc/default.vcl");
	echo $config_file;
?>
									</textarea>
							</td>
						</tr>
					</table>
				</div>
			</td>
		</tr>
	</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
