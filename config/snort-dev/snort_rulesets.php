<?php
/* $Id$ */
/*
	snort_rulesets.php
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2009 Robert Zelaya
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
require_once("filter.inc");
require_once("service-utils.inc");

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}

//nat_rules_sort();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
	

if (isset($id) && $a_nat[$id]) {

	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
}

/* convert fake interfaces to real */
$if_real = convert_friendly_interface_to_real_interface_name($pconfig['interface']);

//if(!is_dir("/usr/local/etc/snort/rules"))
//	exec('mkdir /usr/local/etc/snort/rules/');

/* Check if the rules dir is empy if so warn the user */
/* TODO give the user the option to delete the installed rules rules */
$isrulesfolderempty = exec("ls -A /usr/local/etc/snort/snort_{$id}{$if_real}/rules/*.rules");
if ($isrulesfolderempty == "") {

include("head.inc");
include("fbegin.inc");

echo "<body link=\"#000000\" vlink=\"#000000\" alink=\"#000000\">";

echo "<script src=\"/row_toggle.js\" type=\"text/javascript\"></script>\n
<script src=\"/javascript/sorttable.js\" type=\"text/javascript\"></script>\n
<table width=\"99%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
   <tr>\n
   		<td>\n";

    $tab_array = array();
    $tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
    $tab_array[] = array("If Settings", false, "/snort/snort_interfaces_edit.php?id={$id}");
    $tab_array[] = array("Categories", true, "/snort/snort_rulesets.php?id={$id}");
    $tab_array[] = array("Rules", false, "/snort/snort_rules.php?id={$id}");
    $tab_array[] = array("Servers", false, "/snort/snort_define_servers.php?id={$id}");
    $tab_array[] = array("Preprocessors", false, "/snort/snort_preprocessors.php?id={$id}");
    $tab_array[] = array("Barnyard2", false, "/snort/snort_barnyard.php?id={$id}");
    display_top_tabs($tab_array);

echo  		"</td>\n
  </tr>\n
  <tr>\n
    <td>\n
		<div id=\"mainarea\">\n
			<table id=\"maintable\" class=\"tabcont\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
				<tr>\n
					<td>\n
# The rules directory is empty. /usr/local/etc/snort/snort_{$id}{$if_real}/rules \n
		    		</td>\n
		  		</tr>\n
			</table>\n
		</div>\n
	</td>\n
  </tr>\n
</table>\n
\n
</form>\n
\n
<p>\n\n";

echo "Please click on the Update Rules tab to install your selected rule sets. $isrulesfolderempty";
include("fend.inc");

echo "</body>";
echo "</html>";

exit(0);

}

if($_POST) {
	$enabled_items = "";
	$isfirst = true;
	if (is_array($_POST['toenable'])) {
	foreach($_POST['toenable'] as $toenable) {
		if(!$isfirst)
			$enabled_items .= "||";
		$enabled_items .= "{$toenable}";
		$isfirst = false;
	}
	}else{
	$enabled_items = $_POST['toenable'];
	}
	$a_nat[$id]['rulesets'] = $enabled_items;
	write_config();
//	stop_service("snort");
//	create_snort_conf();
//	sleep(2);
//	start_service("snort");
	$savemsg = "The snort ruleset selections have been saved.";
}

$enabled_rulesets = $a_nat[$id]['rulesets'];
if($enabled_rulesets)
	$enabled_rulesets_array = split("\|\|", $enabled_rulesets);

$pgtitle = "Snort: Interface $id$if_real Categories";
include("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php

echo "<form action=\"snort_rulesets.php?id={$id}\" method=\"post\" name=\"iform\" id=\"iform\">";

?>

<script src="/row_toggle.js" type="text/javascript"></script>
<script src="/javascript/sorttable.js" type="text/javascript"></script>
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
   <tr>
   		<td>
<?php
    $tab_array = array();
    $tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
    $tab_array[] = array("If Settings", false, "/snort/snort_interfaces_edit.php?id={$id}");
    $tab_array[] = array("Categories", true, "/snort/snort_rulesets.php?id={$id}");
    $tab_array[] = array("Rules", false, "/snort/snort_rules.php?id={$id}");
    $tab_array[] = array("Servers", false, "/snort/snort_define_servers.php?id={$id}");
    $tab_array[] = array("Preprocessors", false, "/snort/snort_preprocessors.php?id={$id}");
    $tab_array[] = array("Barnyard2", false, "/snort/snort_barnyard.php?id={$id}");
    display_top_tabs($tab_array);
?>
  		</td>
  </tr>
  <tr>
    <td>
		<div id="mainarea">
			<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td>
						<table id="sortabletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
						    <tr id="frheader">
								<td width="5%" class="listhdrr">Enabled</td>
								<td class="listhdrr">Ruleset: Rules that end with "so.rules" are shared object rules.</td>
								<!-- <td class="listhdrr">Description</td> -->
							</tr>
<?php
	$dir = "/usr/local/etc/snort/snort_{$id}{$if_real}/rules/";
	$dh  = opendir($dir);
	while (false !== ($filename = readdir($dh))) {
   		$files[] = $filename;
	}
	sort($files);
	foreach($files as $file) {
		if(!stristr($file, ".rules"))
			continue;
		echo "<tr>";
		echo "<td align=\"center\" valign=\"top\">";
		if(is_array($enabled_rulesets_array))
			if(in_array($file, $enabled_rulesets_array)) {
				$CHECKED = " checked=\"checked\"";
			} else {
				$CHECKED = "";
			}
		else
			$CHECKED = "";
		echo "	<input type='checkbox' name='toenable[]' value='$file' {$CHECKED} />";
		echo "</td>";
		echo "<td>";
		echo "<a href='snort_rules.php?openruleset=/usr/local/etc/snort/snort_{$id}{$if_real}/rules/" . urlencode($file) . "'>{$file}</a>";
		echo "</td>";
		//echo "<td>";
		//echo "description";
		//echo "</td>";
	}

?>
						</table>
		    		</td>
		  		</tr>
		  		<tr><td>&nbsp;</td></tr>
		  		<tr><td>Check the rulesets that you would like Snort to load at startup.</td></tr>
		  		<tr><td>&nbsp;</td></tr>
		  		<tr><td><input value="Save" type="submit" name="save" id="save" /></td></tr>
			</table>
		</div>
	</td>
  </tr>
</table>

</form>

<p><b>NOTE:</b> You can click on a ruleset name to edit the ruleset.

<?php include("fend.inc"); ?>

</body>
</html>

<?php

	function get_snort_rule_file_description($filename) {
		$filetext = file_get_contents($filename);

	}

?>