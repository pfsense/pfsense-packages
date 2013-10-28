<?php
/* $Id$ */
/* ========================================================================== */
/*
    ossec_rules.php
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

if($_POST['rulefile']) {
	$rulefile = $_POST['rulefile'];
} elseif($_POST['togglerule']) {
	ossec_toggle_rule_file($_POST['togglerule']);
	$rulefile = $_POST['togglerule'];
} elseif($_POST['searchrules']) {
	$searchrules = $_POST['searchrules'];
}

$rulefiles = (array)$config['installedpackages']['ossecrulefiles']['config'];

$pgtitle = "Services: OSSEC Rules";
include("head.inc");
?>                                                                          
<body link="#000000" vlink="#000000" alink="#000000">
<style type="text/css" media="screen">
	.custombutton {
		padding:0;
		margin:0;
		border:none;
		background:none;
		cursor:pointer;
	}    
	/* alternate cursor style for ie */  
	* html .custombutton {cursor:hand;} 
</style>
<?php include("fbegin.inc"); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
	<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("General", false, "/pkg_edit.php?xml=ossec.xml&amp;id=0");
	$tab_array[] = array("Rules", true, "/ossec_rules.php");
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
	$tab_array[] = array("Log Viewer", false, "/ossec_log_viewer.php");
	display_top_tabs($tab_array);
?>
	</td></tr>
  	<tr><td>
	<div id="mainarea">
		<table id="maintable" name="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
			<?php
				$installed_rules = ossec_get_rule_files();
				
				if(!empty($installed_rules)) {
					if(empty($searchrules)) {
						echo "<tr><td width=\"10%\">Rule File</td>";
						echo "<td width=\"60%\"><form action=\"ossec_rules.php\" method=\"post\" name=\"iform\"><select name=\"rulefile\" onchange=\"this.form.submit()\">";
				
						if(empty($rulefile))
							$rulefile = $installed_rules[0];
					
						foreach($installed_rules as $installed_rule) {
							if($installed_rule == $rulefile) {
								echo "<option value=\"$installed_rule\" selected=\"selected\">$installed_rule</option>\n";
							} else {
								echo "<option value=\"$installed_rule\">$installed_rule</option>\n";
							}
						}
					
						echo "</select></form></td>";
						echo "<td width=\"30%\" align=\"right\"><form action=\"ossec_rules.php\" method=\"post\" name=\"iform\">";
						echo "<input type=\"hidden\" name=\"togglerule\" value=\"$rulefile\" />";
						echo "<input type=\"submit\" class=\"formbtn\"";
						if(array_search($rulefile, $rulefiles) !== FALSE)
							echo "value=\"Disable Rule File\">";
						else
							echo "value=\"Enable Rule File\">";
						echo "</button></form></td>";
					}
					?>
					<tr><td colspan="3">
					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
					<?php
					if(file_exists(OSSEC_RULE_DIR . $rulefile)) {
						if(empty($searchrules))
							$rules = ossec_load_rule_file($rulefile);
						else
							$rules = ossec_search_rule_files($searchrules, NULL);
						
						$local_rules = (array)$config['installedpackages']['osseclocalrules']['config'];
						
						$next_id = key(array_slice($local_rules, -1, 1, TRUE)) + 1;
						
						if(!empty($rules)) {
							echo "<tr id=\"frheader\">";
							echo "<td width=\"20%\" class=\"listhdrr\">Rule ID</td>";
							echo "<td width=\"20%\" class=\"listhdrr\">Groups</td>";
							echo "<td width=\"50%\" class=\"listhdrr\">Description</td>";
							echo "<td width=\"10%\" class=\"list\">&nbsp;</td></tr>\n";
							
							foreach($rules as $rule) {
								if(empty($rule['description'])) {
									if($rule['filename'] == 'local_rules.xml')
										$rule['description'] = '[LOCAL RULE]';
									else
										$rule['description'] = '[SYSTEM RULE]';
								}
								
								echo "<tr><td class=\"listr\">" . $rule['ruleid'];
								if($searchrules)
									echo "<br />(" . $rule['filename'] .")</td>\n";
								else
									echo "</td>\n";
								echo "<td class=\"listr\">" . $rule['groups'] . "</td>\n";
								echo "<td class=\"listbg\">" . $rule['description'] . "</td>\n";
								echo "<td class=\"list\">\n";
								echo "<form action=\"pkg_edit.php?xml=ossec_local_rules.xml&id=$next_id\" method=\"post\" name=\"iform\">";
								echo "<input type=\"hidden\" name=\"ruleid\" value=\"" . $rule['ruleid'] . "\" />";
								echo "<input type=\"hidden\" name=\"level\" value=\"" . $rule['level'] . "\">";
								echo "<input type=\"hidden\" name=\"parameters\" value=\"" . htmlentities($rule['parameters']) . "\" />";
								echo "<input type=\"hidden\" name=\"groups\" value=\"" . $rule['groups'] . "\" />";
								echo "<input type=\"hidden\" name=\"maxsize\" value=\"" . $rule['maxsize'] . "\" />";
								echo "<input type=\"hidden\" name=\"frequency\" value=\"" . $rule['frequency'] . "\" />";
								echo "<input type=\"hidden\" name=\"timeframe\" value=\"" . $rule['timeframe'] . "\" />";
								echo "<input type=\"hidden\" name=\"ignore\" value=\"" . $rule['ignore'] . "\" />";
								echo "<input type=\"hidden\" name=\"overwrite\" value=\"" . $rule['overwrite'] . "\" />";
								echo "<input type=\"hidden\" name=\"description\" value=\"" . htmlentities($rule['description']) . "\" />";
								echo "<button type=\"submit\" class=\"custombutton\">";
								echo "<img src=\"./themes/pfsense_ng/images/icons/icon_e.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"click to edit rule\" />";
								echo "</button></form>";
								echo "</td></tr>\n";
							}
						} else {
							echo "<tr><td><span class=\"red\">No rules found.</span></td></tr>\n";
						}
					} else {
						echo "<tr><td><span class=\"red\">Rule file not found.</span></td></tr>\n";
					}
				} else {
					echo "<tr><td><span class=\"red\">No rule files found.</span></td></tr>\n";
				}
			?>
			</table>
			</td></tr>
			<tr><td width="10%">Rule Search</td><td width="90%"><form action="ossec_rules.php" method="post" name="iform">
			<input name="searchrules" value="<?=$searchrules?>" />
			<input type="submit" class="formbtn" value="Search">
			</form></td></tr>
		</table>
	</div>
	</td></tr>
</table>
<?php include("fend.inc"); ?>
</body>
