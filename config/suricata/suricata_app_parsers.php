<?php
/*
 * suricata_app_parsers.php
 * part of pfSense
 *
 * Copyright (C) 2014 Bill Meeks
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
require_once("/usr/local/pkg/suricata/suricata.inc");

global $g, $rebuild_rules;

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
        header("Location: /suricata/suricata_interfaces.php");
        exit;
}

if (!is_array($config['installedpackages']['suricata']))
	$config['installedpackages']['suricata'] = array();
if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();

// Initialize HTTP libhtp engine arrays if necessary
if (!is_array($config['installedpackages']['suricata']['rule'][$id]['libhtp_policy']['item']))
	$config['installedpackages']['suricata']['rule'][$id]['libhtp_policy']['item'] = array();

$a_nat = &$config['installedpackages']['suricata']['rule'];

$libhtp_engine_next_id = count($a_nat[$id]['libhtp_policy']['item']);

$pconfig = array();
if (isset($id) && $a_nat[$id]) {
	/* Get current values from config for page form fields */
	$pconfig = $a_nat[$id];

	// See if Host-OS policy engine array is configured and use
	// it; otherwise create a default engine configuration.
	if (empty($pconfig['libhtp_policy']['item'])) {
		$default = array( "name" => "default", "bind_to" => "all", "personality" => "IDS", 
				  "request-body-limit" => 4096, "response-body-limit" => 4096, 
				  "double-decode-path" => "no", "double-decode-query" => "no" );
		$pconfig['libhtp_policy']['item'] = array();
		$pconfig['libhtp_policy']['item'][] = $default;
		if (!is_array($a_nat[$id]['libhtp_policy']['item']))
			$a_nat[$id]['libhtp_policy']['item'] = array();
		$a_nat[$id]['libhtp_policy']['item'][] = $default;
		write_config();
		$libhtp_engine_next_id++;
	}
	else
		$pconfig['libhtp_policy'] = $a_nat[$id]['libhtp_policy'];
}

// Check for returned "selected alias" if action is import
if ($_GET['act'] == "import" && isset($_GET['varname']) && !empty($_GET['varvalue'])) {
		$pconfig[$_GET['varname']] = $_GET['varvalue'];
}

if ($_GET['act'] && isset($_GET['eng_id'])) {

	$natent = array();
	$natent = $pconfig;

	if ($_GET['act'] == "del_libhtp_policy")
		unset($natent['libhtp_policy']['item'][$_GET['eng_id']]);

	if (isset($id) && $a_nat[$id]) {
		$a_nat[$id] = $natent;
		write_config();
	}

	header("Location: /suricata/suricata_app_parsers.php?id=$id");
	exit;
}

if ($_POST['ResetAll']) {

	/* Reset all the settings to defaults */
	$pconfig['asn1_max_frames'] = "256";

	/* Log a message at the top of the page to inform the user */
	$savemsg = gettext("All flow and stream settings have been reset to their defaults.");
}
elseif ($_POST['Submit']) {
	$natent = array();
	$natent = $pconfig;

	// TODO: validate input values
	if (!is_numeric($_POST['asn1_max_frames'] ) || $_POST['asn1_max_frames'] < 1)
		$input_errors[] = gettext("The value for 'ASN1 Max Frames' must be all numbers and greater than 0.");

	/* if no errors write to conf */
	if (!$input_errors) {
		if ($_POST['asn1_max_frames'] != "") { $natent['asn1_max_frames'] = $_POST['asn1_max_frames']; }else{ $natent['asn1_max_frames'] = "256"; }

		/**************************************************/
		/* If we have a valid rule ID, save configuration */
		/* then update the suricata.conf file and rebuild */
		/* the rules for this interface.                  */
		/**************************************************/
		if (isset($id) && $a_nat[$id]) {
			$a_nat[$id] = $natent;
			write_config();
			$rebuild_rules = true;
			suricata_generate_yaml($natent);
			$rebuild_rules = false;
		}

		header( 'Expires: Sat, 26 Jul 1997 05:00:00 GMT' );
		header( 'Last-Modified: ' . gmdate( 'D, d M Y H:i:s' ) . ' GMT' );
		header( 'Cache-Control: no-store, no-cache, must-revalidate' );
		header( 'Cache-Control: post-check=0, pre-check=0', false );
		header( 'Pragma: no-cache' );
		header("Location: suricata_app_parsers.php?id=$id");
		exit;
	}
}

$if_friendly = suricata_get_friendly_interface($pconfig['interface']);
$pgtitle = gettext("Suricata: Interface {$if_friendly} - Layer 7 Application Parsers");
include_once("head.inc");
?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>
<?php if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}


	/* Display Alert message */

	if ($input_errors) {
		print_input_errors($input_errors); // TODO: add checks
	}

	if ($savemsg) {
		print_info_box($savemsg);
	}

?>

<script type="text/javascript" src="/javascript/autosuggest.js">
</script>
<script type="text/javascript" src="/javascript/suggestions.js">
</script>

<form action="suricata_app_parsers.php" method="post"
	enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Suricata Interfaces"), true, "/suricata/suricata_interfaces.php");
	$tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
	$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php");
	$tab_array[] = array(gettext("Suppress"), false, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php");
	display_top_tabs($tab_array);
	echo '</td></tr>';
	echo '<tr><td>';
	$menu_iface=($if_friendly?substr($if_friendly,0,5)." ":"Iface ");
	$tab_array = array();
	$tab_array[] = array($menu_iface . gettext("Settings"), false, "/suricata/suricata_interfaces_edit.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Categories"), false, "/suricata/suricata_rulesets.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Rules"), false, "/suricata/suricata_rules.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Flow/Stream"), false, "/suricata/suricata_flow_stream.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("App Parsers"), true, "/suricata/suricata_app_parsers.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Variables"), false, "/suricata/suricata_define_vars.php?id={$id}");
	$tab_array[] = array($menu_iface . gettext("Barnyard2"), false, "/suricata/suricata_barnyard.php?id={$id}");
	display_top_tabs($tab_array);
?>
</td></tr>
<tr><td><div id="mainarea">
<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>

		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Abstract Syntax One Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Asn1 Max Frames"); ?></td>
		<td width="78%" class="vtable">
			<input name="asn1_max_frames" type="text" class="formfld unknown" id="asn1_max_frames" size="9"
			value="<?=htmlspecialchars($pconfig['asn1_max_frames']);?>">&nbsp;
			<?php echo gettext("Limit for max number of asn1 frames to decode.  Default is ") . 
			"<strong>" . gettext("256") . "</strong>" . gettext(" frames."); ?><br/><br/>
			<?php echo gettext("To protect itself, Suricata will inspect only the maximum asn1 frames specified.  ") . 
			gettext("Application layer protocols such as X.400 electronic mail, X.500 and LDAP directory services, ") . 
			gettext("H.323 (VoIP), and SNMP, use ASN.1 to describe the protocol data units (PDUs) they exchange."); ?>
		</td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Host-Specific HTTP Server Settings"); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Server Configuration"); ?></td>
		<td width="78%" class="vtable">
			<table width="95%" align="left" id="libhtpEnginesTable" style="table-layout: fixed;" border="0" cellspacing="0" cellpadding="0">
				<colgroup>
					<col width="45%" align="left">
					<col width="45%" align="center">
					<col width="10%" align="right">
				</colgroup>
			   <thead>
				<tr>
					<th class="listhdrr" axis="string"><?php echo gettext("Name");?></th>
					<th class="listhdrr" axis="string"><?php echo gettext("Bind-To Address Alias");?></th>
					<th class="list" align="right"><a href="suricata_import_aliases.php?id=<?=$id?>&eng=libhtp_policy">
					<img src="../themes/<?= $g['theme'];?>/images/icons/icon_import_alias.gif" width="17" 
					height="17" border="0" title="<?php echo gettext("Import server configuration from existing Aliases");?>"></a>
					<a href="suricata_libhtp_policy_engine.php?id=<?=$id?>&eng_id=<?=$libhtp_engine_next_id?>">
					<img src="../themes/<?= $g['theme'];?>/images/icons/icon_plus.gif" width="17" 
					height="17" border="0" title="<?php echo gettext("Add a new server configuration");?>"></a></th>
				</tr>
			   </thead>
			<?php foreach ($pconfig['libhtp_policy']['item'] as $f => $v): ?>
				<tr>
					<td class="listlr" align="left"><?=gettext($v['name']);?></td>
					<td class="listbg" align="center"><?=gettext($v['bind_to']);?></td>
					<td class="listt" align="right"><a href="suricata_libhtp_policy_engine.php?id=<?=$id;?>&eng_id=<?=$f;?>">
					<img src="/themes/<?=$g['theme'];?>/images/icons/icon_e.gif"  
					width="17" height="17" border="0" title="<?=gettext("Edit this server configuration");?>"></a>
			<?php if ($v['bind_to'] <> "all") : ?> 
					<a href="suricata_app_parsers.php?id=<?=$id;?>&eng_id=<?=$f;?>&act=del_libhtp_policy" onclick="return confirm('Are you sure you want to delete this entry?');">
					<img src="/themes/<?=$g['theme'];?>/images/icons/icon_x.gif" width="17" height="17" border="0" 
					title="<?=gettext("Delete this server configuration");?>"></a>
			<?php else : ?>
					<img src="/themes/<?=$g['theme'];?>/images/icons/icon_x_d.gif" width="17" height="17" border="0" 
					title="<?=gettext("Default server configuration cannot be deleted");?>">
			<?php endif ?>
					</td>
				</tr>
			<?php endforeach; ?>
			</table>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%">
			<input name="Submit" type="submit" class="formbtn" value="Save" title="<?php echo 
			gettext("Save flow and stream settings"); ?>">
			<input name="id" type="hidden" value="<?=$id;?>">&nbsp;&nbsp;&nbsp;&nbsp;
			<input name="ResetAll" type="submit" class="formbtn" value="Reset" title="<?php echo 
			gettext("Reset all settings to defaults") . "\" onclick=\"return confirm('" . 
			gettext("WARNING:  This will reset ALL App Parsers settings to their defaults.  Click OK to continue or CANCEL to quit.") . 
			"');\""; ?>></td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%"><span class="vexpl"><span class="red"><strong><?php echo gettext("Note: "); ?></strong></span></span>
			<?php echo gettext("Please save your settings before you exit.  Changes will rebuild the rules file.  This "); ?>
			<?php echo gettext("may take several seconds.  Suricata must also be restarted to activate any changes made on this screen."); ?></td>
	</tr>
</table>
</div>
</td></tr></table>
</form>
<script type="text/javascript">
function wopen(url, name, w, h)
{
	// Fudge factors for window decoration space.
	// In my tests these work well on all platforms & browsers.
	w += 32;
	h += 96;
	var win = window.open(url,
			      name, 
			      'width=' + w + ', height=' + h + ', ' +
			      'location=no, menubar=no, ' +
			      'status=no, toolbar=no, scrollbars=yes, resizable=yes');
	    win.resizeTo(w, h);
	    win.focus();
}

</script>
<?php include("fend.inc"); ?>
</body>
</html>
