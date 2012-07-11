<?php
/*
 * snort_rulesets.php
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009 Robert Zelaya
 * Copyright (C) 2011 Ermal Luci
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g;

$snortdir = SNORTDIR;

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
	header("Location: /snort/snort_interfaces.php");
	exit;
}

if (isset($id) && $a_nat[$id]) {
	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];

	/* convert fake interfaces to real */
	$if_real = snort_get_real_interface($pconfig['interface']);

	$iface_uuid = $a_nat[$id]['uuid'];
}

$if_friendly = snort_get_friendly_interface($pconfig['interface']);
$pgtitle = "Snort: Interface {$if_friendly} Categories";


/* Check if the rules dir is empy if so warn the user */
/* TODO give the user the option to delete the installed rules rules */
$isrulesfolderempty = exec("ls -A {$snortdir}/snort_{$iface_uuid}_{$if_real}/rules/*.rules");
if ($isrulesfolderempty == "") {
	$isrulesfolderempty = exec("ls -A {$snortdir}/rules/*.rules");
	if ($isrulesfolderempty == "") {
		include_once("head.inc");
		include("fbegin.inc");

		echo "<p class=\"pgtitle\">";
		if($pfsense_stable == 'yes'){echo $pgtitle;}
		echo "</p>\n";

		echo "<body link=\"#000000\" vlink=\"#000000\" alink=\"#000000\">";

		echo "
	<table width=\"99%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
		<tr><td>\n";

		$tab_array = array();
		$tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
		$tab_array[] = array(gettext("If Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
		$tab_array[] = array(gettext("Categories"), true, "/snort/snort_rulesets.php?id={$id}");
		$tab_array[] = array(gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
		$tab_array[] = array(gettext("Servers"), false, "/snort/snort_define_servers.php?id={$id}");
		$tab_array[] = array(gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
		$tab_array[] = array(gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
		display_top_tabs($tab_array);
		echo " 
		</td></tr>
		  <tr>\n
		    <td>\n
				<div id=\"mainarea\">\n
					<table id=\"maintable\" class=\"tabcont\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
						<tr>\n
							<td>\n
		# The rules directory is empty. {$snortdir}/snort_{$iface_uuid}_{$if_real}/rules \n
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
	} else {
		/* Make sure that we have the rules */
		mwexec("/bin/cp {$snortdir}/rules/*.rules {$snortdir}/snort_{$iface_uuid}_{$if_real}/rules", true);
	}
}

/* alert file */
if ($_POST["Submit"]) {
	$enabled_items = "";
	$isfirst = true;
	if (is_array($_POST['toenable']))
		$enabled_items = implode("||", $_POST['toenable']);
	else
		$enabled_items = $_POST['toenable'];
	$a_nat[$id]['rulesets'] = $enabled_items;

	write_config();
	sync_snort_package_config();

	header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
	header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
	header( 'Cache-Control: no-store, no-cache, must-revalidate' );
	header( 'Cache-Control: post-check=0, pre-check=0', false );
	header( 'Pragma: no-cache' );
	header("Location: /snort/snort_rulesets.php?id=$id");
	exit;
}

$enabled_rulesets = $a_nat[$id]['rulesets'];
if($enabled_rulesets)
	$enabled_rulesets_array = explode("||", $enabled_rulesets);

include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php include("fbegin.inc"); ?>
<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php

echo "<form action=\"snort_rulesets.php?id={$id}\" method=\"post\" name=\"iform\" id=\"iform\">";

?> <?php

/* Display message */

if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
}

if ($savemsg) {
	print_info_box($savemsg);
}

?>

<table width="99%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[] = array(gettext("If Settings"), false, "/snort/snort_interfaces_edit.php?id={$id}");
        $tab_array[] = array(gettext("Categories"), true, "/snort/snort_rulesets.php?id={$id}");
        $tab_array[] = array(gettext("Rules"), false, "/snort/snort_rules.php?id={$id}");
        $tab_array[] = array(gettext("Servers"), false, "/snort/snort_define_servers.php?id={$id}");
        $tab_array[] = array(gettext("Preprocessors"), false, "/snort/snort_preprocessors.php?id={$id}");
        $tab_array[] = array(gettext("Barnyard2"), false, "/snort/snort_barnyard.php?id={$id}");
        display_top_tabs($tab_array);
?>
</td></tr>
	<tr>
		<td>
		<div id="mainarea2">
		<table id="maintable" class="tabcont" width="100%" border="0"
			cellpadding="0" cellspacing="0">
			<tr>
				<td>
				<table id="sortabletable1" class="sortable" width="100%" border="0"
					cellpadding="0" cellspacing="0">
					<tr id="frheader">
						<td width="5%" class="listhdrr">Enabled</td>
						<td class="listhdrr"><?php if($snort_arch == 'x86'){echo 'Ruleset: Rules that end with "so.rules" are shared object rules.';}else{echo 'Shared object rules are "so.rules" ';}?></td>
						<!-- <td class="listhdrr">Description</td> -->
					</tr>
				<?php
					$dh  = opendir("{$snortdir}/snort_{$iface_uuid}_{$if_real}/rules/");
					while (false !== ($filename = readdir($dh))) {
						$files[] = basename($filename);
					}
					sort($files);
					foreach($files as $file) {
						if(!stristr($file, ".rules"))
						continue;
						echo "<tr>\n";
						echo "<td align=\"center\" valign=\"top\">";
						if(is_array($enabled_rulesets_array))
						if(in_array($file, $enabled_rulesets_array)) {
							$CHECKED = " checked=\"checked\"";
						} else {
							$CHECKED = "";
						}
						else
						$CHECKED = "";
						echo "	\n<input type='checkbox' name='toenable[]' value='$file' {$CHECKED} />\n";
						echo "</td>\n";
						echo "<td>\n";
						echo "<a href='snort_rules.php?id={$id}&openruleset={$snortdir}/snort_{$iface_uuid}_{$if_real}/rules/" . urlencode($file) . "'>{$file}</a>\n";
						echo "</td>\n</tr>\n\n";
					}
				?>
				</table>
				</td>
			</tr>
			<tr>
				<td>&nbsp;</td>
			</tr>
			<tr>
				<td>Check the rulesets that you would like Snort to load at startup.</td>
			</tr>
			<tr>
				<td>&nbsp;</td>
			</tr>
			<tr>
				<td><input value="Save" type="submit" name="Submit" id="Submit" /></td>
			</tr>
		</table>
		</div>
		</td>
	</tr>
</table>
</form>
<?php
include("fend.inc");
?>
</body>
</html>
