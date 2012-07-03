<?php
/* $Id$ */
/*
 firewall_aliases_edit.php
 Copyright (C) 2004 Scott Ullrich
 Copyright (C) 2011 Ermal Luci
 All rights reserved.

 originially part of m0n0wall (http://m0n0.ch/wall)
 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 modified for the pfsense snort package
 Copyright (C) 2009-2010 Robert Zelaya.
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

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort.inc");

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

/* gen uuid for each iface !inportant */
if ($config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'] == '') {
	$whitelist_uuid = 0;
	while ($whitelist_uuid > 65535 || $whitelist_uuid == 0) {
		$whitelist_uuid = mt_rand(1, 65535);
		$pconfig['uuid'] = $whitelist_uuid;
	}
} else if ($config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'] != '') {
	$whitelist_uuid = $config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'];
}

$d_snort_whitelist_dirty_path = '/var/run/snort_whitelist.dirty';

/* returns true if $name is a valid name for a whitelist file name or ip */
function is_validwhitelistname($name, $type) {
	if (!is_string($name))
		return false;

	if ($type === 'name' && !preg_match("/[^a-zA-Z0-9\_]/", $name))
		return true;
	
	if ($type === 'ip' && !preg_match("/[^a-zA-Z0-9\:\,\.\/]/", $name))
		return true;
	
	if ($type === 'detail' && !preg_match("/[^a-zA-Z0-9\:\,\.\+\s\-\']/", $name))
		return true;	

	return false;
}

if (isset($id) && $a_whitelist[$id]) {

	/* old settings */
	$pconfig = array();
	$pconfig['name'] = $a_whitelist[$id]['name'];
	$pconfig['uuid'] = $a_whitelist[$id]['uuid'];
	$pconfig['detail'] = $a_whitelist[$id]['detail'];
	$pconfig['addressuuid'] = $a_whitelist[$id]['addressuuid'];
	$pconfig['snortlisttype'] = $a_whitelist[$id]['snortlisttype'];
	$pconfig['address'] = $a_whitelist[$id]['address'];
	$pconfig['descr'] = html_entity_decode($a_whitelist[$id]['descr']);
	$pconfig['wanips'] = $a_whitelist[$id]['wanips'];
	$pconfig['wangateips'] = $a_whitelist[$id]['wangateips'];
	$pconfig['wandnsips'] = $a_whitelist[$id]['wandnsips'];
	$pconfig['vips'] = $a_whitelist[$id]['vips'];
	$pconfig['vpnips'] = $a_whitelist[$id]['vpnips'];
	$addresses = explode(' ', $pconfig['address']);
	$address = explode(" ", $addresses[0]);
}

if ($_POST['submit']) {

	conf_mount_rw();

	unset($input_errors);
	$pconfig = $_POST;

	//input validation
	$reqdfields = explode(" ", "name"); // post name required
	$reqdfieldsn = explode(",", "Name"); // error msg name

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if(strtolower($_POST['name']) == "defaultwhitelist")
		$input_errors[] = "Whitelist file names may not be named defaultwhitelist.";

	
	if (is_validwhitelistname($_POST['name'], 'name') == false)
		$input_errors[] = "Whitelist name may only consist of the characters a-z, A-Z and 0-9. Note: No Spaces.";
	
	if (is_validwhitelistname($_POST['descr'], 'detail') == false)
		$input_errors[] = "Whitelist description name may only consist of the characters [a-z, A-Z 0-9 + , :]. Note: No Spaces.";	

	// check for name conflicts
	foreach ($a_whitelist as $w_list) {
		if (isset($id) && ($a_whitelist[$id]) && ($a_whitelist[$id] === $w_list))
			continue;

		if ($w_list['name'] == $_POST['name']) {
			$input_errors[] = "A whitelist file name with this name already exists.";
			break;
		}
	}

	// build string lists
	if (!empty($pconfig[addresses])) {
		$countArray = count($pconfig[addresses]);
		$i = 0;
		
		foreach ($pconfig[addresses] as $address) {
	
			$i++;
						
			if (is_validwhitelistname($address[address], 'ip') == false) {
					$input_errors[] = "List of IPs may only consist of the characters [. : 0-9]. Note: No Spaces.";
			}
		
			if (is_validwhitelistname($address[detail], 'detail') == false) {
				$input_errors[] = "List of IP descriptions may only consist of the characters [a-z, A-Z 0-9 + , : ' -].";
			}				
					
			if (!empty($address[address]) && !empty($address[uuid])) {
			
				$final_address_ip .= $address[address];
				
				$final_address_uuid .= $address[uuid];
				
				if (empty($address[detail])) {
					$final_address_details .= "Entry added " . date('r');
				}else{
					$final_address_details .= $address[detail];
				}
				
				if($i < $countArray){
					$final_address_ip .= ',';
					$final_address_details .= '||';
					$final_address_uuid .= '||';
				}
			}		
		}		
	}
	
	$w_list = array();
	// post user input
	$w_list['name'] = $_POST['name'];
	$w_list['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");		
	$w_list['uuid'] = $whitelist_uuid;
	$w_list['snortlisttype'] = $_POST['snortlisttype'];
	$w_list['wanips'] = $_POST['wanips']? 'yes' : 'no';
	$w_list['wangateips'] = $_POST['wangateips']? 'yes' : 'no';
	$w_list['wandnsips'] = $_POST['wandnsips']? 'yes' : 'no';
	$w_list['vips'] = $_POST['vips']? 'yes' : 'no';
	$w_list['vpnips'] = $_POST['vpnips']? 'yes' : 'no';
	
	$w_list['addressuuid'] = $final_address_uuid;
	$w_list['address'] = $final_address_ip;
	$w_list['detail'] = $final_address_details;	
	
	if (empty($final_address_ip) && $w_list['wanips'] === 'no' && $w_list['wangateips'] === 'no' && $w_list['wandnsips'] === 'no' && $w_list['vips'] === 'no' && $w_list['vpnips'] === 'no')
		$input_errors[] = "You must add a \"auto generated ip\" or a \"custom ip\"! ";
		
	if (!$input_errors) {
		if (isset($id) && $a_whitelist[$id])
			$a_whitelist[$id] = $w_list;
		else
			$a_whitelist[] = $w_list;

		write_config();

		// create whitelist and homenet file  then sync files
		sync_snort_package_config();

		header("Location: /snort/snort_interfaces_whitelist.php");
		exit;
	} else {
		
		$pconfig['wanips'] = $a_whitelist[$id]['wanips'];
		$pconfig['wangateips'] = $a_whitelist[$id]['wangateips'];
		$pconfig['wandnsips'] = $a_whitelist[$id]['wandnsips'];
		$pconfig['vips'] = $a_whitelist[$id]['vips'];
		$pconfig['vpnips'] = $a_whitelist[$id]['vpnips'];
		
		$pconfig['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");
		$pconfig['address'] = $final_address_ip;
		$pconfig['detail'] = $final_address_details;
		$pconfig['addressuuid'] = $final_address_uuid;

		$input_errors[] = 'Press Cancel to reset.';
	}

}

$pgtitle = "Services: Snort: Whitelist: Edit $whitelist_uuid";
include_once("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" >

<?php
include("fbegin.inc");
echo $snort_general_css;
?>

<?php
	/* Display Alert message */
	if ($input_errors)
		print_input_errors($input_errors); // TODO: add checks

	if ($savemsg)
		print_info_box($savemsg);

?>
<div id="inputerrors"></div>

<form action="snort_interfaces_whitelist_edit.php?id=<?=$id?>" method="post" name="iform" id="iform">

<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
        $tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
        $tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
        $tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
        $tab_array[5] = array(gettext("Whitelists"), true, "/snort/snort_interfaces_whitelist.php");
        $tab_array[6] = array(gettext("Suppress"), false, "/snort/snort_interfaces_suppress.php");
        $tab_array[7] = array(gettext("Help"), false, "/snort/help_and_info.php");
        display_top_tabs($tab_array);
?>
		</td>
</tr>
	<tr>
		<td class="tabcont">

		<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add the name and
				description of the file.</td>
			</tr>
			<tr>
				<td valign="top" class="vncellreq2">Name</td>
				<td class="vtable"><input name="name" type="text" id="name"
					size="40" value="<?=htmlspecialchars($pconfig['name']);?>" /> <br />
				<span class="vexpl"> The list name may only consist of the
				characters a-z, A-Z and 0-9. <span class="red">Note: </span> No
				Spaces. </span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Description</td>
				<td width="78%" class="vtable"><input name="descr" type="text"
					id="descr" size="40" value="<?=$pconfig['descr'];?>" /> <br />
				<span class="vexpl"> You may enter a description here for your
				reference (not parsed). </span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">List Type</td>
				<td width="78%" class="vtable">

				<div
					style="padding: 5px; margin-top: 16px; margin-bottom: 16px; border: 1px dashed #ff3333; background-color: #eee; color: #000; font-size: 8pt;"
					id="itemhelp"><strong>WHITELIST:</strong>&nbsp;&nbsp;&nbsp;This
				list specifies addresses that Snort Package should not block.<br>
				<br>
				<strong>NETLIST:</strong>&nbsp;&nbsp;&nbsp;This list is for defining
				addresses as $HOME_NET or $EXTERNAL_NET in the snort.conf file.</div>

				<select name="snortlisttype" class="formfld" id="snortlisttype">
				<?php
				$interfaces4 = array('whitelist' => 'WHITELIST', 'netlist' => 'NETLIST');
				foreach ($interfaces4 as $iface4 => $ifacename4): ?>
					<option value="<?=$iface4;?>"
					<?php if ($iface4 == $pconfig['snortlisttype']) echo "selected"; ?>>
						<?=htmlspecialchars($ifacename4);?></option>
						<?php endforeach; ?>
				</select> <span class="vexpl"> &nbsp;&nbsp;&nbsp;Choose the type of
				list you will like see in your <span class="red">Interface Edit Tab</span>.
				</span></td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add auto generated
				ips.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">WAN IPs</td>
				<td width="78%" class="vtable"><input name="wanips" type="checkbox"
					id="wanips" size="40" value="yes"
					<?php if($pconfig['wanips'] == 'yes'){ echo "checked";} if($pconfig['wanips'] == ''){ echo "checked";} ?> />
				<span class="vexpl"> Add WAN IPs to the list. </span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Wan Gateways</td>
				<td width="78%" class="vtable"><input name="wangateips"
					type="checkbox" id="wangateips" size="40" value="yes"
					<?php if($pconfig['wangateips'] == 'yes'){ echo "checked";} if($pconfig['wangateips'] == ''){ echo "checked";} ?> />
				<span class="vexpl"> Add WAN Gateways to the list. </span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Wan DNS servers</td>
				<td width="78%" class="vtable"><input name="wandnsips"
					type="checkbox" id="wandnsips" size="40" value="yes"
					<?php if($pconfig['wandnsips'] == 'yes'){ echo "checked";} if($pconfig['wandnsips'] == ''){ echo "checked";} ?> />
				<span class="vexpl"> Add WAN DNS servers to the list. </span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Virtual IP Addresses</td>
				<td width="78%" class="vtable"><input name="vips" type="checkbox"
					id="vips" size="40" value="yes"
					<?php if($pconfig['vips'] == 'yes'){ echo "checked";} if($pconfig['vips'] == ''){ echo "checked";} ?> />
				<span class="vexpl"> Add Virtual IP Addresses to the list. </span></td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">VPNs</td>
				<td width="78%" class="vtable"><input name="vpnips" type="checkbox"
					id="vpnips" size="40" value="yes"
					<?php if($pconfig['vpnips'] == 'yes'){ echo "checked";} if($pconfig['vpnips'] == ''){ echo "checked";} ?> />
				<span class="vexpl"> Add VPN Addresses to the list. </span></td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Add your own custom
				ips.</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncellreq2">
				<div id="addressnetworkport">IP or CIDR items</div>
				</td>
				<td width="78%" class="vtable">
				<table id="maintable">
					<tbody>
						<tr>
							<td colspan="4">
							<div
								style="padding: 5px; margin-top: 16px; margin-bottom: 16px; border: 1px dashed #ff3333; background-color: #eee; color: #000; font-size: 8pt;"
								id="itemhelp">For <strong>WHITELIST's</strong> enter <strong>ONLY
							IPs not CIDRs</strong>. Example: 192.168.4.1<br>
							<br>
							For <strong>NETLIST's</strong> you may enter <strong>IPs and
							CIDRs</strong>. Example: 192.168.4.1 or 192.168.4.0/24</div>
							</td>
						</tr>
						<tr>
							<td>
							<div id="onecolumn">IP or CIDR</div>
							</td>
							<td>
							<div id="threecolumn">Add a Description or leave blank and a date
							will be added.</div>
							</td>
						</tr>

					<?php
						/* cleanup code */
						$counter = 0;
						if (!empty($pconfig['address'])):
						
						$addressArray = explode(',', $pconfig['address']);
						$detailArray = explode('||', $pconfig['detail']);
						$RowUUIDArray = explode('||', $pconfig['addressuuid']);

						foreach($addressArray as $address):
							if (!empty($address)):
							$detail = $detailArray[$counter];
							$rowaddressuuid= $RowUUIDArray[$counter];
					?>
					<tr id="<?=$rowaddressuuid?>">
						<td><input autocomplete="off" name="addresses[<?=$rowaddressuuid;?>][address]" class="formfld unknown" size="30" value="<?=$address;?>" type="text"></td>
						<td><input autocomplete="off" name="addresses[<?=$rowaddressuuid;?>][detail]" class="formfld unknown" size="50" value="<?=$detail;?>" type="text"></td>
						<td><img id="<?=$rowaddressuuid;?>" class="icon_x removeRow" src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" alt="" title="remove entry" border="0"></td>
						<td><input name="addresses[<?=$rowaddressuuid;?>][uuid]" value="<?=$rowaddressuuid;?>" type="hidden"></td>
					</tr>				
												
					<?php
						$counter++;
						endif;
						endforeach; 
						endif;
					?>
					</tbody>
				</table>
				<img id="addNewRow" class="icon_x" border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" /></td>
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
		</td>
	</tr>
</table>
</form>

<script type="text/javascript">


/*! Needs to be watched not my code <- IMPORTANT
* JavaScript UUID Generator, v0.0.1
*
* Copyright (c) 2009 Massimo Lombardo.
* Dual licensed under the MIT and the GNU GPL licenses.
*/			

function genUUID() {
    var uuid = (function () {
        var i,
            c = "89ab",
            u = [];
        for (i = 0; i < 36; i += 1) {
            u[i] = (Math.random() * 16 | 0).toString(16);
        }
        u[8] = u[13] = u[18] = u[23] = "";
        u[14] = "4";
        u[19] = c.charAt(Math.random() * 4 | 0);
        return u.join("");
    })();
    return {
        toString: function () {
            return uuid;
        },
        valueOf: function () {
            return uuid;
        }
    }
};

					
	jQuery(".icon_x").live('mouseover', function() {
		jQuery(this).css('cursor', 'pointer');
	});

	
	jQuery('#addNewRow').live("click", function(){
		
		var addRowCount = genUUID();
			
		jQuery('#maintable > tbody').append(
				"\n" + '<tr id="' + addRowCount + '">' + "\n" +
				'<td><input autocomplete="off" name="addresses[' + addRowCount + '][address]" class="formfld unknown" size="30" value="" type="text"></td>' + "\n" +
				'<td><input autocomplete="off" name="addresses[' + addRowCount + '][detail]" class="formfld unknown" size="50" value="" type="text"></td>' + "\n" +
				'<td><img id="' + addRowCount + '" class="icon_x removeRow" border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" alt="" title="remove entry" /></td>' + "\n" +
				'<td><input name="addresses[' + addRowCount + '][uuid]" type="hidden" value="' + addRowCount + '" /></td>' + "\n" +
				'</tr>' + "\n"						
			);
	});


    jQuery(".removeRow").live('click', function(){
    	jQuery("#" + this.id).remove();
    });	
	
</script>

<?php include("fend.inc"); ?>
</body>
</html>
