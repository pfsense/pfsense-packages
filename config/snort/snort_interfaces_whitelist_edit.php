<?php
/*
 * snort_interfaces_whitelist_edit.php
 * Copyright (C) 2004 Scott Ullrich
 * Copyright (C) 2011-2012 Ermal Luci
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

if (!is_array($config['installedpackages']['snortglobal']['whitelist']))
	$config['installedpackages']['snortglobal']['whitelist'] = array();
if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
	$config['installedpackages']['snortglobal']['whitelist']['item'] = array();
$a_whitelist = &$config['installedpackages']['snortglobal']['whitelist']['item'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
	header("Location: /snort/snort_interfaces_whitelist.php");
	exit;
}

if (empty($config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'])) {
	$whitelist_uuid = 0;
	while ($whitelist_uuid > 65535 || $whitelist_uuid == 0) {
		$whitelist_uuid = mt_rand(1, 65535);
		$pconfig['uuid'] = $whitelist_uuid;
	}
} else
	$whitelist_uuid = $config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'];

/* returns true if $name is a valid name for a whitelist file name or ip */
function is_validwhitelistname($name) {
	if (!is_string($name))
		return false;

	if (!preg_match("/[^a-zA-Z0-9\_\.\/]/", $name))
		return true;

	return false;
}

if (isset($id) && $a_whitelist[$id]) {
	/* old settings */
	$pconfig = array();
	$pconfig['name'] = $a_whitelist[$id]['name'];
	$pconfig['uuid'] = $a_whitelist[$id]['uuid'];
	$pconfig['detail'] = $a_whitelist[$id]['detail'];
	$pconfig['address'] = $a_whitelist[$id]['address'];
	$pconfig['descr'] = html_entity_decode($a_whitelist[$id]['descr']);
	$pconfig['localnets'] = $a_whitelist[$id]['localnets'];
	$pconfig['wanips'] = $a_whitelist[$id]['wanips'];
	$pconfig['wangateips'] = $a_whitelist[$id]['wangateips'];
	$pconfig['wandnsips'] = $a_whitelist[$id]['wandnsips'];
	$pconfig['vips'] = $a_whitelist[$id]['vips'];
	$pconfig['vpnips'] = $a_whitelist[$id]['vpnips'];
}

if ($_POST['submit']) {
	conf_mount_rw();

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	$reqdfields = explode(" ", "name");
	$reqdfieldsn = explode(",", "Name");
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if(strtolower($_POST['name']) == "defaultwhitelist")
		$input_errors[] = gettext("Whitelist file names may not be named defaultwhitelist.");

	if (is_validwhitelistname($_POST['name']) == false)
		$input_errors[] = gettext("Whitelist file name may only consist of the characters \"a-z, A-Z, 0-9 and _\". Note: No Spaces or dashes. Press Cancel to reset.");

	/* check for name conflicts */
	foreach ($a_whitelist as $w_list) {
		if (isset($id) && ($a_whitelist[$id]) && ($a_whitelist[$id] === $w_list))
			continue;

		if ($w_list['name'] == $_POST['name']) {
			$input_errors[] = gettext("A whitelist file name with this name already exists.");
			break;
		}
	}

	if ($_POST['address'])
		if (!is_alias($_POST['address']))
			$input_errors[] = gettext("A valid alias need to be provided");

	if (!$input_errors) {
		$w_list = array();
		/* post user input */
		$w_list['name'] = $_POST['name'];
		$w_list['uuid'] = $whitelist_uuid;
		$w_list['localnets'] = $_POST['localnets']? 'yes' : 'no';
		$w_list['wanips'] = $_POST['wanips']? 'yes' : 'no';
		$w_list['wangateips'] = $_POST['wangateips']? 'yes' : 'no';
		$w_list['wandnsips'] = $_POST['wandnsips']? 'yes' : 'no';
		$w_list['vips'] = $_POST['vips']? 'yes' : 'no';
		$w_list['vpnips'] = $_POST['vpnips']? 'yes' : 'no';

		$w_list['address'] = $_POST['address'];
		$w_list['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");
		$w_list['detail'] = $final_address_details;

		if (isset($id) && $a_whitelist[$id])
			$a_whitelist[$id] = $w_list;
		else
			$a_whitelist[] = $w_list;

		write_config();

		/* create whitelist and homenet file  then sync files */
		sync_snort_package_config();

		header("Location: /snort/snort_interfaces_whitelist.php");
		exit;
	}
}

$pgtitle = "Services: Snort: Whitelist: Edit $whitelist_uuid";
include_once("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" >

<?php
include("fbegin.inc");
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
if ($input_errors) print_input_errors($input_errors);
if ($savemsg)
	print_info_box($savemsg);
?>
<script type="text/javascript" src="/javascript/autosuggest.js">
</script>
<script type="text/javascript" src="/javascript/suggestions.js">
</script>
<form action="snort_interfaces_whitelist_edit.php" method="post" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabcont">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td colspan="2" valign="top" class="listtopic"><?php echo gettext("Add the name and " .
		"description of the file."); ?></td>
	</tr>
	<tr>
		<td valign="top" class="vncellreq"><?php echo gettext("Name"); ?></td>
		<td class="vtable"><input name="name" type="text" id="name"
			size="40" value="<?=htmlspecialchars($pconfig['name']);?>" /> <br />
		<span class="vexpl"> <?php echo gettext("The list name may only consist of the " .
		"characters \"a-z, A-Z, 0-9 and _\"."); ?>&nbsp;&nbsp;<span class="red"><?php echo gettext("Note:"); ?> </span>
		<?php echo gettext("No Spaces or dashes."); ?> </span></td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Description"); ?></td>
		<td width="78%" class="vtable"><input name="descr" type="text"
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
		<td width="22%" valign="top" class="vncellreq">
		<div id="addressnetworkport"><?php echo gettext("Alias Name:"); ?></div>
		</td>
		<td width="78%" class="vtable">
		<input autocomplete="off" name="address" type="text" class="formfldalias" id="address" size="30" value="<?=htmlspecialchars($pconfig['address']);?>" />
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top">&nbsp;</td>
		<td width="78%">
			<input id="submit" name="submit" type="submit" class="formbtn" value="Save" />
			<input id="cancelbutton" name="cancelbutton" type="button" class="formbtn" value="Cancel" onclick="history.back()" />
			<input name="id" type="hidden" value="<?=$id;?>" />
		</td>
	</tr>
</table>
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
