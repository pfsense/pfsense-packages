<?php
/*
 * snort_passlist_edit.php
 * Copyright (C) 2004 Scott Ullrich
 * Copyright (C) 2011-2012 Ermal Luci
 * Copyright (C) 2014 Bill Meeks
 * All rights reserved.
 *
 * originially part of m0n0wall (http://m0n0.ch/wall)
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * All rights reserved.
 *
 * modified for the pfsense snort package
 * Copyright (C) 2009-2010 Robert Zelaya.
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

if ($_POST['cancel']) {
	header("Location: /snort/snort_passlist.php");
	exit;
}

if (!is_array($config['installedpackages']['snortglobal']['whitelist']))
	$config['installedpackages']['snortglobal']['whitelist'] = array();
if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
	$config['installedpackages']['snortglobal']['whitelist']['item'] = array();
$a_passlist = &$config['installedpackages']['snortglobal']['whitelist']['item'];

if (isset($_POST['id']) && is_numericint($_POST['id']))
	$id = $_POST['id'];
elseif (isset($_GET['id']) && is_numericint($_GET['id']))
	$id = htmlspecialchars($_GET['id']);

/* Should never be called without identifying list index, so bail */
if (is_null($id)) {
	header("Location: /snort/snort_interfaces_whitelist.php");
	exit;
}

/* If no entry for this passlist, then create a UUID and treat it like a new list */
if (!isset($a_passlist[$id]['uuid'])) {
	$passlist_uuid = 0;
	while ($passlist_uuid > 65535 || $passlist_uuid == 0) {
		$passlist_uuid = mt_rand(1, 65535);
		$pconfig['uuid'] = $passlist_uuid;
		$pconfig['name'] = "passlist_{$passlist_uuid}";
	}
} else
	$passlist_uuid = $a_passlist[$id]['uuid'];

/* returns true if $name is a valid name for a pass list file name or ip */
function is_validpasslistname($name) {
	if (!is_string($name))
		return false;

	if (!preg_match("/[^a-zA-Z0-9\_\.\/]/", $name))
		return true;

	return false;
}

if (isset($id) && $a_passlist[$id]) {
	/* old settings */
	$pconfig = array();
	$pconfig['name'] = $a_passlist[$id]['name'];
	$pconfig['uuid'] = $a_passlist[$id]['uuid'];
	$pconfig['detail'] = $a_passlist[$id]['detail'];
	$pconfig['address'] = $a_passlist[$id]['address'];
	$pconfig['descr'] = html_entity_decode($a_passlist[$id]['descr']);
	$pconfig['localnets'] = $a_passlist[$id]['localnets'];
	$pconfig['wanips'] = $a_passlist[$id]['wanips'];
	$pconfig['wangateips'] = $a_passlist[$id]['wangateips'];
	$pconfig['wandnsips'] = $a_passlist[$id]['wandnsips'];
	$pconfig['vips'] = $a_passlist[$id]['vips'];
	$pconfig['vpnips'] = $a_passlist[$id]['vpnips'];
}

// Check for returned "selected alias" if action is import
if ($_GET['act'] == "import") {
	if ($_GET['varname'] == "address" && isset($_GET['varvalue']))
		$pconfig[$_GET['varname']] = htmlspecialchars($_GET['varvalue']);
}

if ($_POST['save']) {
	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	$reqdfields = explode(" ", "name");
	$reqdfieldsn = explode(",", "Name");
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if(strtolower($_POST['name']) == "defaultpasslist")
		$input_errors[] = gettext("Pass List file names may not be named defaultpasslist.");

	if (is_validpasslistname($_POST['name']) == false)
		$input_errors[] = gettext("Pass List file name may only consist of the characters \"a-z, A-Z, 0-9 and _\". Note: No Spaces or dashes. Press Cancel to reset.");

	/* check for name conflicts */
	foreach ($a_passlist as $w_list) {
		if (isset($id) && ($a_passlist[$id]) && ($a_passlist[$id] === $w_list))
			continue;

		if ($w_list['name'] == $_POST['name']) {
			$input_errors[] = gettext("A Pass List file name with this name already exists.");
			break;
		}
	}

	if ($_POST['address'])
		if (!is_alias($_POST['address']))
			$input_errors[] = gettext("A valid alias must be provided");

	if (!$input_errors) {
		$w_list = array();
		/* post user input */
		$w_list['name'] = $_POST['name'];
		$w_list['uuid'] = $passlist_uuid;
		$w_list['localnets'] = $_POST['localnets']? 'yes' : 'no';
		$w_list['wanips'] = $_POST['wanips']? 'yes' : 'no';
		$w_list['wangateips'] = $_POST['wangateips']? 'yes' : 'no';
		$w_list['wandnsips'] = $_POST['wandnsips']? 'yes' : 'no';
		$w_list['vips'] = $_POST['vips']? 'yes' : 'no';
		$w_list['vpnips'] = $_POST['vpnips']? 'yes' : 'no';

		$w_list['address'] = $_POST['address'];
		$w_list['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");
		$w_list['detail'] = $final_address_details;

		if (isset($id) && $a_passlist[$id])
			$a_passlist[$id] = $w_list;
		else
			$a_passlist[] = $w_list;

		write_config();

		/* create pass list and homenet file, then sync files */
		sync_snort_package_config();

		header("Location: /snort/snort_passlist.php");
		exit;
	}
}

$pgtitle = gettext("Snort: Pass List Edit - {$pconfig['name']}");
include_once("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" >

<?php
include("fbegin.inc");
if ($input_errors)
	print_input_errors($input_errors);
if ($savemsg)
	print_info_box($savemsg);
?>
<script type="text/javascript" src="/javascript/autosuggest.js">
</script>
<script type="text/javascript" src="/javascript/suggestions.js">
</script>
<form action="snort_passlist_edit.php" method="post" name="iform" id="iform">
<input name="id" type="hidden" value="<?=$id;?>" />
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
        $tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
        $tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
        $tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
        $tab_array[5] = array(gettext("Pass Lists"), true, "/snort/snort_passlist.php");
        $tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
	$tab_array[7] = array(gettext("IP Lists"), false, "/snort/snort_ip_list_mgmt.php");
	$tab_array[8] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
        display_top_tabs($tab_array,true);
?>
	</td>
</tr>
<tr><td><div id="mainarea">
<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Add the name and " .
		"description of the file."); ?></td>
	</tr>
	<tr>
		<td valign="top" class="vncellreq"><?php echo gettext("Name"); ?></td>
		<td class="vtable"><input name="name" type="text" id="name" class="formfld unknown" 
			size="40" value="<?=htmlspecialchars($pconfig['name']);?>" /> <br />
		<span class="vexpl"> <?php echo gettext("The list name may only consist of the " .
		"characters \"a-z, A-Z, 0-9 and _\"."); ?>&nbsp;&nbsp;<span class="red"><?php echo gettext("Note:"); ?> </span>
		<?php echo gettext("No Spaces or dashes."); ?> </span></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Description"); ?></td>
		<td width="78%" class="vtable"><input name="descr" type="text" class="formfld unknown" 
			id="descr" size="40" value="<?=$pconfig['descr'];?>" /> <br />
		<span class="vexpl"> <?php echo gettext("You may enter a description here for your " .
		"reference (not parsed)."); ?> </span></td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Add auto-generated IP Addresses."); ?></td>
	</tr>

	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Local Networks"); ?></td>
		<td width="78%" class="vtable"><input name="localnets" type="checkbox"
			id="localnets" size="40" value="yes"
			<?php if($pconfig['localnets'] == 'yes'){ echo "checked";} if($pconfig['localnets'] == ''){ echo "checked";} ?> />
		<span class="vexpl"> <?php echo gettext("Add firewall Local Networks to the list (excluding WAN)."); ?> </span></td>
	</tr>

	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("WAN IPs"); ?></td>
		<td width="78%" class="vtable"><input name="wanips" type="checkbox"
			id="wanips" size="40" value="yes"
			<?php if($pconfig['wanips'] == 'yes'){ echo "checked";} if($pconfig['wanips'] == ''){ echo "checked";} ?> />
		<span class="vexpl"> <?php echo gettext("Add WAN interface IPs to the list."); ?> </span></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("WAN Gateways"); ?></td>
		<td width="78%" class="vtable"><input name="wangateips"
			type="checkbox" id="wangateips" size="40" value="yes"
			<?php if($pconfig['wangateips'] == 'yes'){ echo "checked";} if($pconfig['wangateips'] == ''){ echo "checked";} ?> />
		<span class="vexpl"> <?php echo gettext("Add WAN Gateways to the list."); ?> </span></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("WAN DNS servers"); ?></td>
		<td width="78%" class="vtable"><input name="wandnsips"
			type="checkbox" id="wandnsips" size="40" value="yes"
			<?php if($pconfig['wandnsips'] == 'yes'){ echo "checked";} if($pconfig['wandnsips'] == ''){ echo "checked";} ?> />
		<span class="vexpl"> <?php echo gettext("Add WAN DNS servers to the list."); ?> </span></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Virtual IP Addresses"); ?></td>
		<td width="78%" class="vtable"><input name="vips" type="checkbox"
			id="vips" size="40" value="yes"
			<?php if($pconfig['vips'] == 'yes'){ echo "checked";} if($pconfig['vips'] == ''){ echo "checked";} ?> />
		<span class="vexpl"> <?php echo gettext("Add Virtual IP Addresses to the list."); ?> </span></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("VPNs"); ?></td>
		<td width="78%" class="vtable"><input name="vpnips" type="checkbox"
			id="vpnips" size="40" value="yes"
			<?php if($pconfig['vpnips'] == 'yes'){ echo "checked";} if($pconfig['vpnips'] == ''){ echo "checked";} ?> />
		<span class="vexpl"> <?php echo gettext("Add VPN Addresses to the list."); ?> </span></td>
	</tr>
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Add custom IP Addresses from configured Aliases."); ?></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell">
		<?php echo gettext("Assigned Aliases:"); ?>
		</td>
		<td width="78%" class="vtable">
		<input autocomplete="off" name="address" type="text" class="formfldalias" id="address" size="30" value="<?=htmlspecialchars($pconfig['address']);?>"
		title="<?=trim(filter_expand_alias($pconfig['address']));?>"/>
		&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" class="formbtns" value="Aliases" onclick="parent.location='snort_select_alias.php?id=0&type=host|network&varname=address&act=import&multi_ip=yes&returl=<?=urlencode($_SERVER['PHP_SELF']);?>'" 
		title="<?php echo gettext("Select an existing IP alias");?>"/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%">
			<input id="save" name="save" type="submit" class="formbtn" value="Save" />
			<input id="cancel" name="cancel" type="submit" class="formbtn" value="Cancel" />
		</td>
	</tr>
</table>
</div>
</td></tr>
</table>
</form>
<script type="text/javascript">
<?php
        $isfirst = 0;
        $aliases = "";
        $addrisfirst = 0;
        $aliasesaddr = "";
        if(isset($config['aliases']['alias']) && is_array($config['aliases']['alias']))
                foreach($config['aliases']['alias'] as $alias_name) {
			if ($alias_name['type'] != "host" && $alias_name['type'] != "network")
				continue;
                        if($addrisfirst == 1) $aliasesaddr .= ",";
                        $aliasesaddr .= "'" . $alias_name['name'] . "'";
                        $addrisfirst = 1;
                }
?>
        var addressarray=new Array(<?php echo $aliasesaddr; ?>);

function createAutoSuggest() {
<?php
	echo "objAlias = new AutoSuggestControl(document.getElementById('address'), new StateSuggestions(addressarray));\n";
?>
}

setTimeout("createAutoSuggest();", 500);

</script>
<?php include("fend.inc"); ?>
</body>
</html>
