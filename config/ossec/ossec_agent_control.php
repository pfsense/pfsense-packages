<?php
/* $Id$ */
/* ========================================================================== */
/*
    ossec_agent_control.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Lance Leger
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */

require("guiconfig.inc");
require("/usr/local/pkg/ossec.inc");

if($_GET['act'])
	$savemsg = ossec_control_agent($_GET['act'], $_GET['id']);

$agents = (array)$config['installedpackages']['ossecagents']['config'];

$pgtitle = "Services: OSSEC: Agent Control";
include("head.inc");
?>                                                                          
<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
	<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("General", false, "/pkg_edit.php?xml=ossec.xml&amp;id=0");
	$tab_array[] = array("Rules", false, "/ossec_rules.php");
	$tab_array[] = array("Decoders", false, "/ossec_decoders.php");
	$tab_array[] = array("Syschecks", false, "/pkg.php?xml=ossec_syschecks.xml");
	$tab_array[] = array("Rootchecks", false, "/pkg.php?xml=ossec_rootchecks.xml");
	$tab_array[] = array("Active Response", false, "/pkg.php?xml=ossec_active_response.xml");
	$tab_array[] = array("Commands", false, "/pkg.php?xml=ossec_commands.xml");
	$tab_array[] = array("Local Rules", false, "/pkg.php?xml=ossec_local_rules.xml");
	$tab_array[] = array("Local Decoders", false, "/pkg.php?xml=ossec_local_decoders.xml");
	$tab_array[] = array("Local Files", false, "/pkg.php?xml=ossec_local_files.xml");
	$tab_array[] = array("Local Variables", false, "/pkg.php?xml=ossec_local_variables.xml");
	$tab_array[] = array("Agents", false, "/pkg.php?xml=ossec_agents.xml");
	$tab_array[] = array("Agent Control", true, "/agent_control.php");
	$tab_array[] = array("Log Viewer", false, "/ossec_log_viewer.php");
	display_top_tabs($tab_array);
?>
	</td></tr>
  	<tr><td>
	<div id="mainarea">
		<table id="maintable" name="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
				<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
				<?php
				if(!empty($agents)) {
					echo "<tr id=\"frheader\"><td width=\"4%\" class=\"list\">&nbsp;</td>";
					echo "<td width=\"8%\" class=\"listhdrr\">ID</td>";
					echo "<td width=\"12%\" class=\"listhdrr\">Name</td>";
					echo "<td width=\"12%\" class=\"listhdrr\">IP Address</td>";
					echo "<td width=\"10%\" class=\"listhdrr\">OS</td>";
					echo "<td width=\"8%\" class=\"listhdrr\">Version</td>";
					echo "<td width=\"12%\" class=\"listhdrr\">Last Keep Alive</td>";
					echo "<td width=\"12%\" class=\"listhdrr\">Last Syscheck</td>";
					echo "<td width=\"12%\" class=\"listhdrr\">Last Rootcheck</td>";
					echo "<td width=\"10%\" class=\"list\">&nbsp;</td></tr>\n";
					foreach($agents as $agent) {
						list($ac_id, $ac_name, $ac_ip, $ac_status, $ac_os, $ac_version, $ac_keepalive, $ac_syscheck, $ac_rootcheck) = explode(",", trim(shell_exec(OSSEC_BASE . "/bin/agent_control -i " . $agent['agentid'] . " -s")));
						
						echo "<tr><td class=\"listt\" align=\"center\">\n";
						
						if($ac_status == 'Active')
							echo "<img src=\"./themes/pfsense_ng/images/icons/icon_pass.gif\" width=\"11\" height=\"11\" border=\"0\" title=\"Active\">";
						elseif($ac_status == 'Disconnected')
							echo "<img src=\"./themes/pfsense_ng/images/icons/icon_block.gif\" width=\"11\" height=\"11\" border=\"0\" title=\"Disconnected\">";
						else
							echo "<img src=\"./themes/pfsense_ng/images/icons/icon_block_d.gif\" width=\"11\" height=\"11\" border=\"0\" title=\"Never connected\">";
						
						echo "</td>\n";
						echo "<td class=\"listr\">" . $agent['agentid'] . "</td>\n";
						echo "<td class=\"listr\">" . $agent['name'] . "</td>\n";
						echo "<td class=\"listr\">" . $agent['ip'] . "</td>\n";
						echo "<td class=\"listr\">" . $ac_os . "</td>\n";
						echo "<td class=\"listr\">" . $ac_version . "</td>\n";
						echo "<td class=\"listr\">" . $ac_keepalive . "</td>\n";
						echo "<td class=\"listr\">" . $ac_syscheck . "</td>\n";
						echo "<td class=\"listr\">" . $ac_rootcheck . "</td>\n";
						echo "<td class=\"list\">\n";
						echo "<a href=\"ossec_agent_control.php?act=extract_key&id=" . $agent['agentid'] . "\">";
						echo "<img src=\"./themes/pfsense_ng/images/icons/icon_logs.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"click to view agent key\"></a>";
						echo "<a href=\"ossec_agent_control.php?act=start_checks&id=" . $agent['agentid']  . "\">";
						echo "<img src=\"./themes/pfsense_ng/images/icons/icon_service_start.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"click to force start agent checks\"></a>";
						echo "<a href=\"ossec_agent_control.php?act=restart_agent&id=" . $agent['agentid'] . "\">";
						echo "<img src=\"./themes/pfsense_ng/images/icons/icon_service_restart.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"click to restart agent\"></a>";
						echo "</td></tr>\n";
					}
					echo "<tr id=\"fr1\">";
					echo "<td class=\"list\"></td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">&nbsp;</td>";
					echo "<td class=\"list\">";
					echo "<a href=\"ossec_agent_control.php?act=start_checks\">";
					echo "<img src=\"./themes/pfsense_ng/images/icons/icon_service_start.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"click to force start agent checks\"></a>";
					echo "</td></tr>";
				} else {
					echo "<tr><td><span class=\"red\">No agents found.</span></td></tr>\n";
				}
				?>
				</table>
			</td></tr>
		</table>
	</div>
	</td></tr>
</table>
<?php include("fend.inc"); ?>
</body>
