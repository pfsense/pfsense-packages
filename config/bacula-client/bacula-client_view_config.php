<?php
/*
	bacula-client_view_config.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2012 M�rcio Carlos Ant�o
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

if ($pf_version > 2.0)
	define('BACULA_LOCALBASE', '/usr/pbi/bacula-' . php_uname("m"));
else
  define('BACULA_LOCALBASE','/usr/local');
	
$pgtitle = "Bacula-Client: View Configuration";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="bacula-client_view_config.php" method="post">
	
<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Directors"), false, "/pkg.php?xml=bacula-client.xml");
	$tab_array[] = array(gettext("FileDaemon"), false, "/pkg_edit.php?xml=bacula-client_fd.xml");
	$tab_array[] = array(gettext("View Configuration"), true, "/bacula-client_view_config.php");
	display_top_tabs($tab_array);
?>
		</td></tr>
 		<tr>
    		<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
     						<td class="tabcont" >
									<textarea id="varnishlogs" rows="50" cols="87%">
<?php 
	$config_file = file_get_contents(BACULA_LOCALBASE."/etc/bacula-fd.conf");
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