<?php
/* $Id$ */
/*
	firewall_aliases_edit.php
	Copyright (C) 2004 Scott Ullrich
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
require_once("/usr/local/pkg/snort/snort.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
	$config['installedpackages']['snortglobal']['whitelist']['item'] = array();

$a_whitelist = &$config['installedpackages']['snortglobal']['whitelist']['item'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];


/* gen uuid for each iface !inportant */
if ($config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'] == '') {
	//$snort_uuid = gen_snort_uuid(strrev(uniqid(true)));
$whitelist_uuid = 0;
while ($whitelist_uuid > 65535 || $whitelist_uuid == 0) {
	$whitelist_uuid = mt_rand(1, 65535);
	$pconfig['uuid'] = $whitelist_uuid;
	}
}

if ($config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'] != '') {
	$whitelist_uuid = $config['installedpackages']['snortglobal']['whitelist']['item'][$id]['uuid'];
}	
	
$pgtitle = "Services: Snort: Whitelist: Edit $whitelist_uuid";

$d_snort_whitelist_dirty_path = '/var/run/snort_whitelist.dirty';

/* returns true if $name is a valid name for a whitelist file name or ip */
function is_validwhitelistname($name) {
	if (!is_string($name))
		return false;

	if (!preg_match("/[^a-zA-Z0-9\.\/]/", $name))
		return true;

	return false;
}
	
	
if (isset($id) && $a_whitelist[$id]) {
	
	/* old settings */
	$pconfig['name'] = $a_whitelist[$id]['name'];
	$pconfig['uuid'] = $a_whitelist[$id]['uuid'];
	$pconfig['detail'] = $a_whitelist[$id]['detail'];
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
	if ($address[1])
		$addresssubnettest = true;
	else
		$addresssubnettest = false;	
}

	/* this will exec when alert says apply */
	if ($_POST['apply']) {
		
		if (file_exists("$d_snort_whitelist_dirty_path")) {
			
			write_config();
			
			sync_snort_package_config();
			sync_snort_package();

			unlink("$d_snort_whitelist_dirty_path");
			
		}
		
	}

if ($_POST['submit']) {

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	$reqdfields = explode(" ", "name address");
	$reqdfieldsn = explode(",", "Name,Address");

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if(strtolower($_POST['name']) == "defaultwhitelist")
		$input_errors[] = "Whitelist file names may not be named defaultwhitelist.";

	$x = is_validwhitelistname($_POST['name']);
	if (!isset($x)) {
		$input_errors[] = "Reserved word used for whitelist file name.";
	} else {
		if (is_validwhitelistname($_POST['name']) == false)
			$input_errors[] = "Whitelist file name may only consist of the characters a-z, A-Z and 0-9 _. Note: No Spaces. Press Cancel to reset.";
	}
	
	if (is_validwhitelistname($_POST['address']) == false)
		$input_errors[] = "Whitelist address may only consist of the characters 0-9 and /. Note: No Spaces. Press Cancel to reset.";


	/* check for name conflicts */
	foreach ($a_whitelist as $w_list) {
		if (isset($id) && ($a_whitelist[$id]) && ($a_whitelist[$id] === $w_list))
			continue;

		if ($w_list['name'] == $_POST['name']) {
			$input_errors[] = "A whitelist file name with this name already exists.";
			break;
		}
	}

	
	$w_list = array();
	/* post user input */
	$w_list['name'] = $_POST['name'];
	$w_list['uuid'] = $whitelist_uuid;
	$w_list['snortlisttype'] = $_POST['snortlisttype'];
	$w_list['address'] = $_POST['address'];
	$w_list['wanips'] = $_POST['wanips']? yes : no;
	$w_list['wangateips'] = $_POST['wangateips']? yes : no;
	$w_list['wandnsips'] = $_POST['wandnsips']? yes : no;
	$w_list['vips'] = $_POST['vips']? yes : no;
	$w_list['vpnips'] = $_POST['vpnips']? yes : no;
	

	$address = $w_list['address'];
	$final_address_detail = mb_convert_encoding($_POST['detail'],"HTML-ENTITIES","auto");
  		if($final_address_detail <> "") {
	       	$final_address_details .= $final_address_detail;
	} else {
		$final_address_details .= "Entry added" . " ";
   			$final_address_details .= date('r');
		}
    	$final_address_details .= "||";
	$isfirst = 0;
	

		/* add another entry code */
		for($x=0; $x<299; $x++) {
			$comd = "\$subnet = \$_POST['address" . $x . "'];";
			eval($comd);
			$comd = "\$subnet_address = \$_POST['address_subnet" . $x . "'];";
			eval($comd);
			if($subnet <> "") {
				$address .= " ";
				$address .= $subnet;
				if($subnet_address <> "") $address .= "" . $subnet_address;

				/* Compress in details to a single key, data separated by pipes.
				   Pulling details here lets us only pull in details for valid
				   address entries, saving us from having to track which ones to
				   process later. */
	       $comd  =  "\$final_address_detail  =  mb_convert_encoding(\$_POST['detail"  .  $x  .  "'],'HTML-ENTITIES','auto');"; 
	       eval($comd);
	       if($final_address_detail <> "") {
	       $final_address_details .= $final_address_detail;
	       } else {
		       $final_address_details .= "Entry added" . " ";
		       $final_address_details .= date('r');
	       }
	       $final_address_details .= "||";
			}
		}

	if (!$input_errors) {
		$w_list['address'] = $address;
        $w_list['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");
		$w_list['detail'] = $final_address_details;

		if (isset($id) && $a_whitelist[$id])
			$a_whitelist[$id] = $w_list;
		else
			$a_whitelist[] = $w_list;

		touch($d_snort_whitelist_dirty_path);

		write_config();

		header("Location: /snort/snort_interfaces_whitelist_edit.php?id=$id");
		exit;		
	}
	//we received input errors, copy data to prevent retype
	else
	{
                $pconfig['descr']  =  mb_convert_encoding($_POST['descr'],"HTML-ENTITIES","auto");
                $pconfig['address'] = $address;
				$pconfig['detail'] = $final_address_details;
	}
}

include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php
	include("./snort_fbegin.inc");
	echo $jscriptstr;
?>

<script type="text/javascript" src="/snort/javascript/row_helper.js"></script>
<input type='hidden' name='address_type' value='textbox' />
<script type="text/javascript">
	rowname[0] = "address";
	rowtype[0] = "textbox";
	rowsize[0] = "30";

	rowname[1] = "detail";
	rowtype[1] = "textbox";
	rowsize[1] = "50";
</script>

<p class="pgtitle"><?=$pgtitle?></p>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<div id="inputerrors"></div>

<form action="snort_interfaces_whitelist_edit.php?id=<?=$id?>" method="post" name="iform" id="iform">

<?php
	/* Display Alert message */
	if ($input_errors) {
	print_input_errors($input_errors); // TODO: add checks
	}

	if ($savemsg) {
	print_info_box2($savemsg);
	}

	//if (file_exists($d_snortconfdirty_path)) {
	if (file_exists($d_snort_whitelist_dirty_path)) {
	echo '<p>';

		if($savemsg) {
			print_info_box_np2("{$savemsg}");
		}else{
			print_info_box_np2('
			The Snort configuration has changed and snort needs to be restarted on this interface.<br>
			You must apply the changes in order for them to take effect.<br>
			');
		}
	}
?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", false, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", true, "/snort/snort_interfaces_whitelist.php");
	$tab_array[] = array("Suppress", false, "/snort/snort_interfaces_suppress.php");
	$tab_array[] = array("Help", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>    </td></tr>
<tr>
<td class="tabcont">

<table width="100%" border="0" cellpadding="6" cellspacing="0">
  <tr>
      <td colspan="2" valign="top" class="listtopic">Add the name and description of the file.</td>
  </tr>
  <tr>
    <td valign="top" class="vncellreq">Name</td>
    <td class="vtable">
      <input name="name" type="text" id="name" size="40" value="<?=htmlspecialchars($pconfig['name']);?>" />
      <br />
      <span class="vexpl">
        The list name may only consist of the characters a-z, A-Z and 0-9. <span class="red">Note: </span> No Spaces.
      </span>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncell">Description</td>
    <td width="78%" class="vtable">
      <input name="descr" type="text"  id="descr" size="40" value="<?=$pconfig['descr'];?>" />
      <br />
      <span class="vexpl">
        You may enter a description here for your reference (not parsed).
      </span>
    </td>
  </tr>
	<tr>
	<td width="22%" valign="top" class="vncell">List Type</td>
	<td width="78%" class="vtable">
		<select name="snortlisttype" class="formfld" id="snortlisttype">
			<?php
				$interfaces4 = array('whitelist' => 'WHITELIST', 'netlist' => 'NETLIST');
				foreach ($interfaces4 as $iface4 => $ifacename4): ?>
				<option value="<?=$iface4;?>" <?php if ($iface4 == $pconfig['snortlisttype']) echo "selected"; ?>>
				<?=htmlspecialchars($ifacename4);?>
				</option>
			<?php endforeach; ?>
		</select><br>
		<span class="vexpl">Choose the type of list you will like see in your Interface Edit Tab.&nbsp;Hint: Best pratice is to test every list you make.
		</span>&nbsp;<span class="red">Note:</span>&nbsp;NETLIST's are only for defining snort.conf's external or home NETS.</td>
	</tr>
    <tr>
        <td colspan="2" valign="top" class="listtopic">Add auto generated ips.</td>
   </tr>
  <tr>
    <td width="22%" valign="top" class="vncell">WAN IPs</td>
    <td width="78%" class="vtable">
      <input name="wanips" type="checkbox"  id="wanips" size="40" value="yes" <?php if($pconfig['wanips'] == 'yes'){ echo "checked";} if($pconfig['wanips'] == ''){ echo "checked";} ?>/>
      <span class="vexpl">
        Add WAN IPs to the list.
      </span>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncell">Wan Gateways</td>
    <td width="78%" class="vtable">
      <input name="wangateips" type="checkbox"  id="wangateips" size="40" value="yes" <?php if($pconfig['wangateips'] == 'yes'){ echo "checked";} if($pconfig['wangateips'] == ''){ echo "checked";} ?>/>
      <span class="vexpl">
        Add WAN Gateways to the list.
      </span>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncell">Wan DNS servers</td>
    <td width="78%" class="vtable">
      <input name="wandnsips" type="checkbox"  id="wandnsips" size="40" value="yes" <?php if($pconfig['wandnsips'] == 'yes'){ echo "checked";} if($pconfig['wandnsips'] == ''){ echo "checked";} ?>/>
      <span class="vexpl">
        Add WAN DNS servers to the list.
      </span>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncell">Virtual IP Addresses</td>
    <td width="78%" class="vtable">
      <input name="vips" type="checkbox"  id="vips" size="40" value="yes" <?php if($pconfig['vips'] == 'yes'){ echo "checked";} if($pconfig['vips'] == ''){ echo "checked";} ?>/>
      <span class="vexpl">
        Add Virtual IP Addresses to the list.
      </span>
    </td>
  </tr>
   <tr>
    <td width="22%" valign="top" class="vncell">VPNs</td>
    <td width="78%" class="vtable">
      <input name="vpnips" type="checkbox"  id="vpnips" size="40" value="yes" <?php if($pconfig['vpnips'] == 'yes'){ echo "checked";} if($pconfig['vpnips'] == ''){ echo "checked";} ?>/>
      <span class="vexpl">
        Add VPN Addresses to the list.
      </span>
    </td>
  </tr>
  <tr>
     <td colspan="2" valign="top" class="listtopic">Add your own custom ips.</td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncellreq"><div id="addressnetworkport">IP or CIDR items</div></td>
    <td width="78%" class="vtable">
      <table id="maintable">
        <tbody>
          <tr>
            <td colspan="4">
      		    <div style="padding:5px; margin-top: 16px; margin-bottom: 16px; border:1px dashed #000066; background-color: #ffffff; color: #000000; font-size: 8pt;" id="itemhelp">
      		    Enter only ips and CIDRs, do not enter CIDRs for whitelists. Example: 192.168.4.1 or 192.168.4.0/24</div>
            </td>
          </tr>
          <tr>
            <td><div id="onecolumn">IP or CIDR</div></td>
           <td><div id="threecolumn">Add a Description or leave blank and a date will be added.</div></td>
          </tr>

			<?php
			/* cleanup code */
			$counter = 0;
			$address = $pconfig['address'];
			$item = explode(" ", $address);
			$item3 = explode("||", $pconfig['detail']);
			foreach($item as $ww) {
				$address = $item[$counter];
				$item4 = $item3[$counter];
				if($counter > 0) $tracker = $counter + 1;
			?>
          <tr>
            <td>
              <input name="address<?php echo $tracker; ?>" type="text"  id="address<?php echo $tracker; ?>" size="30" value="<?=htmlspecialchars($address);?>" />
            </td>
            <td>
              <input name="detail<?php echo $tracker; ?>" type="text"  id="detail<?php echo $tracker; ?>" size="50" value="<?=$item4;?>" />
            </td>
            <td>
    			  <?php
    				if($counter > 0)
    					echo "<input type=\"image\" src=\"/themes/".$g['theme']."/images/icons/icon_x.gif\" onclick=\"removeRow(this); return false;\" value=\"Delete\" />";
    			  ?>
			      </td>
          </tr>
			<?php
        $counter++;

        } // end foreach
      ?>
        </tbody>
		</table>
		<a onclick="javascript:addRowTo('maintable'); return false;" href="#"><img border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" />
      </a>
		</td>
  </tr>
  <tr>
    <td width="22%" valign="top">&nbsp;</td>
    <td width="78%">
      <input id="submit" name="submit" type="submit" class="formbtn" value="Save" />
      <input id="cancelbutton" name="cancelbutton" type="button" class="formbtn" value="Cancel" onclick="history.back()" />
      <?php if (isset($id) && $a_whitelist[$id]): ?>
      <input name="id" type="hidden" value="<?=$id;?>" />
      <?php endif; ?>
    </td>
  </tr>
</table>
  </td>
  </tr>
  </table>
</form>

<script type="text/javascript">
	/* row and col adjust when you add extra entries */
	field_counter_js = 2;
	rows = 1;
	totalrows = <?php echo $counter; ?>;
	loaded = <?php echo $counter; ?>;
</script>

<?php include("fend.inc"); ?>
</body>
</html>