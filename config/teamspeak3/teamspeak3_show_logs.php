<?php
/* $Id$ */
/* ========================================================================== */
/*
	teamspeak3_show_logs.php
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

if(isset($_GET['downloadlog']))
{
	teamspeak3_download_log_file($_GET['downloadlog']);
	exit;
}

require("guiconfig.inc");

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
	?>	
	
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td class="tabnavtbl">
				<?php				
					$tab_array = array();
					$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=teamspeak3.xml&amp;id=0");
					$tab_array[] = array(gettext("Services"), false, "/status_services.php");				
					$tab_array[] = array(gettext("Logs"), true, "/teamspeak3_show_logs.php");
					$tab_array[] = array(gettext("Backup &amp; Restore"), false, "/teamspeak3_backup_restore.php");
					$tab_array[] = array(gettext("Install or update"), false, "/teamspeak3_install.php");
					display_top_tabs($tab_array);				
				?>
			</td>
		</tr>
	</table>	
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td class="tabcont">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">              
					<tr>
						<td>
							<p>Here you download the TeamSpeak 3 server log files. Logging options can be changed using the ts3server.ini configuration file (<a href="/pkg_edit.php?xml=teamspeak3.xml&amp;id=0">Setting Tab</a>). Some virtual TeamSpeak server options can also be changed by using the TeamSpeak 3 client. Right click on your server name (after connecting to your server), click on "Edit Virtual Server" and switch to the "Logs" tab. Please note: this requires TeamSpeak 3 server administrator privileges.</p>
						</td>
					</tr>
				</table>
				<br />
				<table width="100%" border="0" cellpadding="0" cellspacing="0">
					<tr>
						<td class="listhdrr">Log file</td>
						<td width="80" class="listhdrr">Action</td>
					</tr>
					<?php 
						$aLogFiles = teamspeak3_get_all_log_files();
						
						if(!empty($aLogFiles))
						{
							foreach($aLogFiles as $aLogFile)
							{
								echo "<tr><td class=\"listr\">{$aLogFile}</td><td class=\"listr\"><a href=\"/teamspeak3_show_logs.php?downloadlog={$aLogFile}\">Download</a></td></tr>\n";
							}
						}
					?>
				</table>
			</td>
		</tr>
	</table>
	<?php include("fend.inc"); ?>
</body>
</html>