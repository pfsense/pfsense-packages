<?php
/* $Id$ */
/* ========================================================================== */
/*
    ossec_decoders.php
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

if($_POST['decoderfile']) {
	$decoderfile = $_POST['decoderfile'];
} elseif($_POST['searchdecoders']) {
	$searchdecoders = $_POST['searchdecoders'];
}

$pgtitle = "Services: OSSEC Decoders";
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
	$tab_array[] = array("Rules", false, "/ossec_rules.php");
	$tab_array[] = array("Decoders", true, "/ossec_decoders.php");
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
				$installed_decoders = ossec_get_decoder_files();
				
				if(!empty($installed_decoders)) {
					if(empty($searchdecoders)) {
						echo "<tr><td width=\"10%\">Decoder File</td>";
						echo "<td width=\"90%\"><form action=\"ossec_decoders.php\" method=\"post\" name=\"iform\"><select name=\"decoderfile\" onchange=\"this.form.submit()\">";
				
						if(empty($decoderfile))
							$decoderfile = $installed_decoders[0];
					
						foreach($installed_decoders as $installed_decoder) {
							if($installed_decoder == $decoderfile) {
								echo "<option value=\"$installed_decoder\" selected=\"selected\">$installed_decoder</option>\n";
							} else {
								echo "<option value=\"$installed_decoder\">$installed_decoder</option>\n";
							}
						}
					
						echo "</select></form></td>";
					}
					?>
					<tr><td colspan="2">
					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
					<?php
					if(file_exists(OSSEC_ETC_DIR . $decoderfile)) {
						if(empty($searchdecoders))
							$decoders = ossec_load_decoder_file($decoderfile);
						else
							$decoders = ossec_search_decoder_files($searchdecoders, NULL);
						
						$local_decoders = (array)$config['installedpackages']['osseclocaldecoders']['config'];
						
						$next_id = key(array_slice($local_decoders, -1, 1, TRUE)) + 1;
						
						if(!empty($decoders)) {
							echo "<tr id=\"frheader\">";
							echo "<td width=\"40%\" class=\"listhdrr\">Decoder Name</td>";
							echo "<td width=\"50%\" class=\"listhdrr\">Description</td>";
							echo "<td width=\"10%\" class=\"list\">&nbsp;</td></tr>\n";
							
							foreach($decoders as $decoder) {
								if(empty($decoder['description'])) {
									if($decoder['filename'] == 'local_decoder.xml')
										$decoder['description'] = '[LOCAL DECODER]';
									else
										$decoder['description'] = '[SYSTEM DECODER]';
								}
								
								echo "<tr><td class=\"listr\">" . $decoder['name'];
								if($searchdecoders)
									echo "<br />(" . $decoder['filename'] .")</td>\n";
								else
									echo "</td>\n";
								echo "<td class=\"listbg\">" . $decoder['description'] . "</td>\n";
								echo "<td class=\"list\">\n";
								echo "<form action=\"pkg_edit.php?xml=ossec_local_decoders.xml&id=$next_id\" method=\"post\" name=\"iform\">";
								echo "<input type=\"hidden\" name=\"name\" value=\"" . $decoder['name'] . "\" />";
								echo "<input type=\"hidden\" name=\"parameters\" value=\"" . htmlentities($decoder['parameters']) . "\" />";
								echo "<input type=\"hidden\" name=\"description\" value=\"" . htmlentities($decoder['description']) . "\" />";
								echo "<button type=\"submit\" class=\"custombutton\">";
								echo "<img src=\"./themes/pfsense_ng/images/icons/icon_e.gif\" width=\"17\" height=\"17\" border=\"0\" title=\"click to edit decoder\" />";
								echo "</button></form>";
								echo "</td></tr>\n";
							}
						} else {
							echo "<tr><td><span class=\"red\">No decoders found.</span></td></tr>\n";
						}
					} else {
						echo "<tr><td><span class=\"red\">Decoder file not found.</span></td></tr>\n";
					}
				} else {
					echo "<tr><td><span class=\"red\">No decoder files found.</span></td></tr>\n";
				}
			?>
			</table>
			</td></tr>
			<tr><td width="10%">Decoder Search</td><td width="90%"><form action="ossec_decoders.php" method="post" name="iform">
			<input name="searchdecoders" value="<?=$searchdecoders?>" />
			<input type="submit" class="formbtn" value="Search">
			</form></td></tr>
		</table>
	</div>
	</td></tr>
</table>
<?php include("fend.inc"); ?>
</body>
