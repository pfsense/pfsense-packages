<?php
/* $Id$ */
/* ========================================================================== */
/*
	teamspeak3_install.php
	Copyright (C) 2014 Sander Peterse
	All rights reserved.
                                                                              */
/* ========================================================================== */
/*
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1.	Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

	2.	Redistributions in binary form must reproduce the above copyright
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
/* ========================================================================== */

require_once("/usr/local/pkg/teamspeak3.inc");

require("guiconfig.inc");

$pgtitle = "TeamSpeak 3";
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

	<?php include("fbegin.inc"); ?>
	
	<?php 
	if(!empty($_FILES["serverfile"]) && $_FILES["serverfile"]["error"] == 0)
	{
		$sErrorMessage = "";
		if(teamspeak3_install_from_file($_FILES["serverfile"], &$sErrorMessage))
		{
			print_info_box("Succesfully installed or updated Teamspeak 3 server.");
		}
	
		if(!empty($sErrorMessage))
		{
			print_info_box("Installation failed: ".$sErrorMessage);
		}
	}	
	?>
	
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td class="tabnavtbl">
				<?php				
					$tab_array = array();
					$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=teamspeak3.xml&amp;id=0");
					$tab_array[] = array(gettext("Services"), false, "/status_services.php");
					$tab_array[] = array(gettext("Logs"), false, "/teamspeak3_show_logs.php");
					$tab_array[] = array(gettext("Backup &amp; Restore"), false, "/teamspeak3_backup_restore.php");				
					$tab_array[] = array(gettext("Install or update"), true, "/teamspeak3_install.php");
					display_top_tabs($tab_array);				
				?>
			</td>
		</tr>
	</table>
	<table class="tabcont" align="center" width="100%" border="0"
		cellpadding="6" cellspacing="0">		
		<tr>
			<td colspan="2" class="listtopic">Install or update TeamSpeak 3 Server</td>
		</tr>
		<form action="/teamspeak3_install.php" method="post" name="iform" enctype="multipart/form-data">
			<tr>
				<td width="22%" valign="baseline" class="vncell">&nbsp;</td>
				<td width="78%" class="vtable">Open a <strong>teamspeak3-server_freebsd-xxx-3.xx.xx.xx.tar.gz</strong>
					archive and click the button below to upload the (new) server files. <br /> <br />
					<p>
						<input name="serverfile" type="file" class="formfld unknown" id="serverfile" size="40">
					</p>
					<p>
						<input name="uploadserverfiles" type="submit" class="formbtn"
							id="uploadserverfiles" value="Upload server files">
					</p>
					<p><strong><span class="red">Note:</span></strong><br />The file (tar.gz archive) should contain the TeamSpeak 3 (FreeBSB) server files. Please download it from <a href="http://www.teamspeak.com/?page=downloads" target="_blank">http://www.teamspeak.com/?page=downloads</a>. Make sure you select the correct operating system (FreeBSD) and system architecture (<?php echo teamspeak3_current_architecture(); ?>) when downloading this file.</p>
					<?php if(!teamspeak3_is_new_install()) { ?><p><strong><span class="red">Important:</span></strong><br />Always create a <a href="/teamspeak3_backup_restore.php">backup</a> of your Teamspeak 3 configuration, files and database before uploading (new) files.</p><?php } ?>
				</td>
			</tr>
		</form>
	</table>
	<?php include("fend.inc"); ?>
</body>
</html>