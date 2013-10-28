<?php
/* $Id$ */
/* ========================================================================== */
/*
    ossec_log_viewer.xml
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

require_once('globals.inc');
require("guiconfig.inc");
require("/usr/local/pkg/ossec.inc");

$log_root = OSSEC_BASE . "/logs";
$default_logfile = "ossec.log";
$multiline_logs = array("alerts/alerts.log");

if($_POST['logfile'])
	$logfile = $_POST['logfile'];
else
	$logfile = $default_logfile;
 
if($_POST['limit'])
	$limit = intval($_POST['limit']);
else
	$limit = "10";

if($_POST['archives'])
	$archives = true;

if($_POST['filter'])
	$filter = $_POST['filter'];

if($_POST['not'])
	$not = true;

$log_messages = array();
$log_files = array();
$log_path = $log_root . "/" . $logfile;

if(file_exists($log_path) && (filesize($log_path) > 0)) {
	$grepcmd = "grep -ih";
	
	$log_dir = dirname($log_path);
	
	if($archives && ($log_dir != $log_root))
		$grepcmd = "cd $log_dir && find -E . -regex '.*\.(log|log\.gz)' -exec zgrep -ih";
	
	if(!in_array($logfile, $multiline_logs)) {
		if(isset($filter) && $not) {
			$grepcmd = "$grepcmd -v '$filter'";
		} else {
			$grepcmd = "$grepcmd '$filter'";
		}
	} else {
		$grepcmd = "$grepcmd ''";
	}
	
	if($archives && ($log_dir != $log_root))
		$grepcmd = $grepcmd . " {} \;";
	else
		$grepcmd = $grepcmd . " " . $log_path;
	
	if(in_array($logfile, $multiline_logs)) {
		$grepcmd = $grepcmd . " | awk '/^\$/{f=0;print};f||/^.+\$/{f=1;printf $0 \" \"}'";
		
		if(isset($filter) && $not) {
			$grepcmd = "$grepcmd | grep -i -v '$filter'";
		} else {
			$grepcmd = "$grepcmd | grep -i '$filter'";
		}
	}
	
	$log_lines = trim(shell_exec("$grepcmd | sed 's/^[^0-9A-Za-z]*//g' | wc -l"));
	$log_output = trim(shell_exec("$grepcmd | sed 's/^[^0-9a-zA-Z]*//g' | sort -r +0 -2 | head -n $limit"));
	
	if(!empty($log_output)) {
		$log_messages = explode("\n", $log_output);
		$log_messages_count = sizeof($log_messages);
	}
}

$pgtitle = "Services: OSSEC Log Viewer";
include("head.inc");
?>                                                                          
<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<form action="ossec_log_viewer.php" method="post" name="iform">
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
	$tab_array[] = array("Agent Control", false, "/ossec_agent_control.php");
	$tab_array[] = array("Log Viewer", true, "/ossec_log_viewer.php");
	display_top_tabs($tab_array);
?>
	</td></tr>
  	<tr><td>
	<div id="mainarea">
		<table id="maintable" name="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
			
			<table>
				<tr><td width="22%">Log File</td><td width="78%"><select name="logfile">
				<?php
				$log_files = ossec_get_log_files();
				foreach($log_files as $log_file) {
					if($log_file['path'] == $logfile) {
						echo "<option value=\"" . $log_file['path'] . "\" selected=\"selected\">" . $log_file['name'] . "</option>\n";
					} else {
						echo "<option value=\"" . $log_file['path'] . "\">" . $log_file['name'] . "</option>\n";
					}
				}
				?>
				</select></td></tr>
				<tr><td width="22%">Limit</td><td width="78%"><select name="limit">
				<?php
				$limit_options = array("10", "20", "50");
				foreach($limit_options as $limit_option) {
					if($limit_option == $limit) {
						echo "<option value=\"$limit_option\" selected=\"selected\">$limit_option</option>\n";
					} else {
						echo "<option value=\"$limit_option\">$limit_option</option>\n";
					}
				}
				?>
				</select></td></tr>
				<tr><td width="22%">Include Archives</td><td width="78%"><input type="checkbox" name="archives" <?php if($archives) echo " CHECKED"; ?> /></td></tr>
				<tr><td colspan="2">
				<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
				<?php
				if(!empty($log_messages)) {
					echo "<tr><td class=\"listtopic\">Showing $log_messages_count of $log_lines messages</td></tr>\n";
					foreach($log_messages as $log_message) {	
						echo "<tr><td class=\"listr\">$log_message</td></tr>\n";
					}
				} else {
					echo "<tr><td><span class=\"red\">No log messages found or log file is empty.</span></td></tr>\n";
				}
				?>
				</table>
				</td></tr>
				<tr><td width="22%">Filter</td><td width="78%"><input name="filter" value="<?=$filter?>" /></td></tr>
				<tr><td width="22%">Inverse Filter (NOT)</td><td width="78%"><input type="checkbox" name="not" <?php if($not) echo " CHECKED"; ?> /></td></tr>
				<tr><td colspan="2"><input type="submit" value="Refresh" /></td></tr>
			</table>
	
			</td></tr>
		</table>
	</div>
	</td></tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
