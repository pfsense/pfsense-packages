<?php
/* $Id$ */
/* ========================================================================== */
/*
	teamspeak3_backup_restore.php
	Copyright (C) 2013 Sander Peterse
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
require_once("guiconfig.inc");

$sActionMessage = null;
if(isset($_POST['download_ts3server_sqlitedb']))
{
	if(teamspeak3_download_backup("ts3server.sqlitedb", &$sActionMessage))
		exit;
}
else if(isset($_POST['download_ts3server_ini']))
{
	if(teamspeak3_download_backup("ts3server.ini", &$sActionMessage))
		exit;
}
else if(isset($_POST['download_ts3filebrowser']))
{
	if(teamspeak3_download_backup("ts3files.tar.gz", &$sActionMessage))
		exit;
}	
else if(isset($_POST['restorebackup']) && isset($_FILES['restorefile']))
{
	teamspeak3_restore_backup($_POST['restoretype'], $_FILES['restorefile'], &$sActionMessage);	
}

$closehead = true;
$pgtitle = "TeamSpeak 3";
include("head.inc");
?>	
	<style type="text/css">
		td.infoboxsave input[type=button] { font-size: 1.1em; }
	</style>
</head>

<?php
teamspeak3_custom_php_after_form_head();
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

	<?php 
	include("fbegin.inc"); 
		
	if(!empty($savemsg))
	{
		print_info_box($savemsg);
	}

	if($sActionMessage != null)
	{
		print_info_box($sActionMessage);
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
					$tab_array[] = array(gettext("Backup &amp; Restore"), true, "/teamspeak3_backup_restore.php");				
					$tab_array[] = array(gettext("Install or update"), false, "/teamspeak3_install.php");
					display_top_tabs($tab_array);				
				?>
			</td>
		</tr>
	</table>
	
	<table class="tabcont" align="center" width="100%" border="0"
		cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" class="listtopic">Backup TeamSpeak 3</td>
		</tr>
		<tr>
			<td width="22%" valign="baseline" class="vncell">&nbsp;</td>
			<td width="78%" class="vtable">
				<p>
					Here you can backup the most import configuration files. The &quot;<strong>ts3server.sqlitedb</strong>&quot;
					file contains all your information about channels, users etc. The
					&quot;<strong>ts3server.ini</strong>&quot; file contains the
					advanced server configuration. All other data (avatars &amp; file browser) 
					can be with the archive &quot;<strong>ts3files.tar.gz</strong>&quot;.<br /> <br /> 
					
					<form action="/teamspeak3_backup_restore.php" method="post" name="iform" enctype="multipart/form-data">					
						<input name="download_ts3server_sqlitedb" type="submit" class="formbtn" id="download" value="Download ts3server.sqlitedb">&nbsp;
						<input name="download_ts3server_ini" type="submit" class="formbtn" id="download" value="Download ts3server.ini">
						<input name="download_ts3filebrowser" type="submit" class="formbtn" id="download" value="Download ts3files.tar.gz">
					</form>
				</p>
				<p><strong><span class="red">Note:</span></strong><br />Downloading the <strong>ts3server.sqlite</strong> file will restart the TeamSpeak 3 service (when it is running).</p>
			</td>
		</tr>
		<tr>
			<td colspan="2" class="list" height="12">&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" class="listtopic">Restore TeamSpeak 3</td>
		</tr>
		<form action="/teamspeak3_backup_restore.php" method="post" name="iform" enctype="multipart/form-data">
			<tr>
				<td width="22%" valign="baseline" class="vncell">&nbsp;</td>
				<td width="78%" class="vtable">Open a <strong>ts3server.sqlitedb</strong>
					file and click the button below to restore the the file. <br /> <br />
					<p>
						<input name="restoretype" type="hidden" value="ts3server.sqlitedb" /> 
						<input name="restorefile" type="file" class="formfld unknown" id="restorefile1" size="40">
					</p>
					<p>
						<input name="restorebackup" type="submit"
							class="formbtn" id="restorebackup1" value="Restore ts3server.sqlitedb">
					</p>
					<p><strong><span class="red">Note:</span></strong><br />This will overwrite the current <strong>ts3server.sqlitedb</strong> file.</p>
				</td>
			</tr>
		</form>
		<form action="/teamspeak3_backup_restore.php" method="post" name="iform" enctype="multipart/form-data">
			<tr>
				<td width="22%" valign="baseline" class="vncell">&nbsp;</td>
				<td width="78%" class="vtable">Open a <strong>ts3server.ini</strong>
					file and click the button below to restore the the file. <br /> <br />
					<p>
						<input name="restoretype" type="hidden" value="ts3server.ini" /> 
						<input name="restorefile" type="file" class="formfld unknown" id="restorefile2" size="40">
					</p>
					<p>
						<input name="restorebackup" type="submit" class="formbtn"
							id="restorebackup2" value="Restore ts3server.ini">
					</p>
					<p><strong><span class="red">Note:</span></strong><br />This will overwrite the current <strong>ts3server.ini</strong> file.</p>
				</td>
			</tr>
		</form>
		<form action="/teamspeak3_backup_restore.php" method="post" name="iform" enctype="multipart/form-data">
			<tr>
				<td width="22%" valign="baseline" class="vncell">&nbsp;</td>
				<td width="78%" class="vtable">Open a <strong>ts3files.tar.gz</strong>
					archive and click the button below to restore all files. <br /> <br />
					<p>
						<input name="restoretype" type="hidden" value="ts3files.tar.gz" /> 
						<input name="restorefile" type="file" class="formfld unknown" id="restorefile3" size="40">
					</p>
					<p>
						<input name="restorebackup" type="submit" class="formbtn"
							id="restorebackup3" value="Restore ts3files.tar.gz">
					</p>
					<p><strong><span class="red">Note:</span></strong><br />This will overwrite <strong>all existing files</strong> in the file browser.</p>
				</td>
			</tr>
		</form>
	</table>
	<?php include("fend.inc"); ?>
</body>
</html>