<?php
/*
	postfix_view_config.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2010 Marcello Coutinho <marcellocoutinho@gmail.com>
	based on varnish_view_config.
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

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
	$one_two = true;

$pgtitle = "Postfix: View Configuration";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="postfix_view_config.php" method="post">
	
<div id="mainlevel">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=postfix.xml&id=0");
	$tab_array[] = array(gettext("ACLs / Filter Maps"), false, "/pkg_edit.php?xml=postfix_acl.xml&id=0");
	$tab_array[] = array(gettext("Valid Recipients"), false, "/pkg_edit.php?xml=postfix_recipients.xml&id=0");
	$tab_array[] = array(gettext("Antispam"), false, "/pkg_edit.php?xml=postfix_antispam.xml&id=0");
	$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=postfix_sync.xml&id=0");
	$tab_array[] = array(gettext("View config files"), true, "/postfix_view_config.php");
	
	display_top_tabs($tab_array);
?>
		</td></tr>
 		<tr>
    		<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
						<td class="tabcont" >
						<input type="button" onClick="location.href='./postfix_view_config.php?file=0'" value="main.cf">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=1'" value="master.cf">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=2'" value="relay_recipients">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=3'" value="header_check">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=4'" value="mime_check">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=5'" value="body_check">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=6'" value="client CIDR">&nbsp;
						<input type="button" onClick="location.href='./postfix_view_config.php?file=7'" value="client PCRE">&nbsp;
						</td>
							</tr>
							<tr>						
     						<td class="tabcont" >
									<textarea id="varnishlogs" rows="50" cols="100%">
<?php
	$files_array[]="/usr/local/etc/postfix/main.cf";
	$files_array[]="/usr/local/etc/postfix/master.cf";
	$files_array[]="/usr/local/etc/postfix/relay_recipientes";
	$files_array[]="/usr/local/etc/postfix/header_check";
	$files_array[]="/usr/local/etc/postfix/mime_check";
	$files_array[]="/usr/local/etc/postfix/body_check";
	$files_array[]="/usr/local/etc/postfix/cal_cidr";
	$files_array[]="/usr/local/etc/postfix/cal_pcre";
	$id=($_REQUEST['file']?$_REQUEST['file']:"0");
	$config_file = file_get_contents("$files_array[$id]");
	echo $files_array[$id]."\n".$config_file;
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
