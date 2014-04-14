<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_servers_edit.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2013 Marcello Coutinho
	Copyright (C) 2009 Scott Ullrich <sullrich@pfsense.com>
	Copyright (C) 2008 Remco Hoef <remcoverhoef@pfsense.com>
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
	AUTHOR BE LIABLE FOR ANY DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
$shortcut_section = "haproxy";
require("guiconfig.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";
$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

if (!is_array($config['installedpackages']['haproxy']['ha_servers']['item'])) {
	$config['installedpackages']['haproxy']['ha_servers']['item'] = array();
}

$a_server = &$config['installedpackages']['haproxy']['ha_servers']['item'];

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($id) && $a_server[$id]) {
	$pconfig['name'] = $a_server[$id]['name'];
	$pconfig['address'] = $a_server[$id]['address'];
	$pconfig['port'] = $a_server[$id]['port'];
	$pconfig['backend'] = $a_server[$id]['backend'];
	$pconfig['weight'] = $a_server[$id]['weight'];
	$pconfig['checkinter'] = $a_server[$id]['checkinter'];
	$pconfig['cookie'] = $a_server[$id]['cookie'];
	$pconfig['status'] = $a_server[$id]['status'];
	$pconfig['advanced'] = base64_decode($a_server[$id]['advanced']);
}

$changedesc = "Services: HAProxy: Servers: ";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	$reqdfields = explode(" ", "name address weight");
	$reqdfieldsn = explode(",", "Name,Address,Weight");		

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";

	if (preg_match("/[^a-zA-Z0-9\.]/", $_POST['address']))
		$input_errors[] = "The field 'Address' contains invalid characters.";

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['cookie']))
		$input_errors[] = "The field 'Cookie' contains invalid characters.";

	if ($_POST['port'] && !is_numeric($_POST['port']))
		$input_errors[] = "The field 'Port' value is not a number.";
	else {
		if ($_POST['port'])
			if (!($_POST['port']>=1 && $_POST['port']<=65535))
				$input_errors[] = "The field 'Port' value must be between 1 and 65535.";
	}
	
	if (!is_numeric($_POST['weight']))
		$input_errors[] = "The field 'Weight' value is not a number.";
	else {
		if (!($_POST['weight']>=1 && $_POST['weight']<=256))
			$input_errors[] = "The field 'Weight' value must be between 1 and 256.";
	}
	
	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_servers']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_servers']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This server name has already been used.  Server names must be unique.";

	$backend = "";
	for($x=0; $x<299; $x++) {
		$comd = "\$backends = \$_POST['backend" . $x . "'];";
		eval($comd);
		if($backends) 
			$backend .= "$backends ";
	}
	$backend = trim($backend);

	if (!$input_errors) {
		$server = array();
		if(isset($id) && $a_server[$id])
			$server = $a_server[$id];
			
		if($server['name'] != "")
			$changedesc .= " modified '{$server['name']}' pool:";

		update_if_changed("name", $server['name'], $_POST['name']);
		update_if_changed("port", $server['port'], $_POST['port']);
		update_if_changed("backend", $server['backend'], $backend);
		update_if_changed("cookie", $server['cookie'], $_POST['cookie']);
		update_if_changed("weight", $server['weight'], $_POST['weight']);
		update_if_changed("status", $server['status'], $_POST['status']);
		update_if_changed("address", $server['address'], $_POST['address']);
		update_if_changed("advanced", $server['advanced'], base64_encode($_POST['advanced']));
		update_if_changed("checkinter", $server['checkinter'], $_POST['checkinter']);

		if (isset($id) && $a_server[$id]) {
			$a_server[$id] = $server;
		} else {
			$a_server[] = $server;
		}

		if ($changecount > 0) {
			touch($d_haproxyconfdirty_path);
			write_config($changedesc);			
			/*
			echo "<PRE>";
			print_r($config);
			echo "</PRE>";
			*/
		}

		header("Location: haproxy_servers.php");
		exit;
	}
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "HAProxy: Server: Edit";
include("head.inc");

row_helper();

?>

<input type='hidden' name='address_type' value='textbox' />

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script type="text/javascript" language="javascript" src="pool.js"></script>

<script language="javascript">
function clearcombo(){
  for (var i=document.iform.serversSelect.options.length-1; i>=0; i--){
    document.iform.serversSelect.options[i] = null;
  }
  document.iform.serversSelect.selectedIndex = -1;
}
</script>
<script type="text/javascript">
	rowname[0] = "backend";
	rowtype[0] = "select";
	rowsize[0] = "1";
</script>
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
	<form action="haproxy_servers_edit.php" method="post" name="iform" id="iform">
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
		$tab_array[] = array("Settings", false, "haproxy_global.php");
        $tab_array[] = array("Frontends", false, "haproxy_frontends.php");
		$tab_array[] = array("Servers", true, "haproxy_servers.php");
		$tab_array[] = array("Sync", false, "pkg_edit.php?xml=haproxy_sync.xml");
		display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
		<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit HAProxy server</td>
		</tr>	
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="16" maxlength="16"><br>
			</td>
		</tr>
		<tr align="left">
		  <td width="22%" valign="top" class="vncellreq">Frontend(s)</td>
		  <td width="78%" class="vtable">
			<table id="frontendtable">
			  <tbody>
				<tr>
					<td><div id="onecolumn"></div></td>
				</tr>
				<?php
					$counter = 0;
					$tracker = 0;
					$backend = $pconfig['backend'];
					$item = explode(" ", $backend);
					foreach($item as $ww) {
						$address = $ww;
						if($counter > 0) 
							$tracker = $counter + 1;
				?>
				<tr>
					<td>
						<select name="backend<?php echo $tracker; ?>">
							<?php 
								$i = 0; 
								if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) 
									$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
								$backends = split(" ", $pconfig['backend']);
								foreach ($a_backend as $backend) {
							?>
							<option value="<?=$backend['name'];?>" <?php if($backend['name'] == $ww) echo "SELECTED";?>>
								<?=$backend['name'];?>
							</option>
							<?php } ?>
						</select><br>
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
			  <tfoot>
			  </tfoot>
		  </table>
			<a onclick="javascript:addRowTo('frontendtable'); return false;" href="#">
				<img border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" />
			</a><br/>
		</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq">
			<div id="addressnetworkport">
				IP Address
			</div>
		  </td>
		  <td width="78%" class="vtable" colspan="2">
			<input name="address" type="text" id="address" size="30" value="<?=htmlspecialchars($pconfig['address']);?>" /><br/>
		</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Port</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="port" type="text" <?if(isset($pconfig['port'])) echo "value=\"{$pconfig['port']}\"";?> size="5">
				<br/>
				NOTE: Leave blank to use Frontend port selection.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Status</td>
			<td width="78%" class="vtable" colspan="2">
				<select name="status">
				<option value="active"  <?php if($pconfig['status']=='active') echo "SELECTED";?>>active</option>
				<option value="backup" <?php  if($pconfig['status']=='backup') echo "SELECTED";?>>backup</option>
				<option value="disabled" <?php  if($pconfig['status']=='disabled') echo "SELECTED";?>>disabled</option>
				<option value="inactive" <?php  if($pconfig['status']=='inactive') echo "SELECTED";?>>inactive</option>
				</select>
			<br>Select Server Status</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Cookie</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="cookie" type="text" <?if(isset($pconfig['cookie'])) echo "value=\"{$pconfig['cookie']}\"";?>size="64"><br/>
				  This value will be checked in incoming requests, and the first
				  operational server possessing the same value will be selected. In return, in
				  cookie insertion or rewrite modes, this value will be assigned to the cookie
				  sent to the client. There is nothing wrong in having several servers sharing
				  the same cookie value, and it is in fact somewhat common between normal and
				  backup servers. See also the "cookie" keyword in backend section.
				<br/>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Check inter</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="checkinter" type="text" <?if(isset($pconfig['checkinter'])) echo "value=\"{$pconfig['checkinter']}\"";?>size="10">
				<br/>Defaults to 1000 if left blank.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Weight</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="weight" type="text" <?if(isset($pconfig['weight'])) echo "value=\"{$pconfig['weight']}\"";?>size="6"><br/>
				The default weight is 1, and the maximal value is 255.<br/>
				NOTE: If this 
					  parameter is used to distribute the load according to server's capacity, it 
					  is recommended to start with values which can both grow and shrink, for 
					  instance between 10 and 100 to leave enough room above and below for later 
					  adjustments.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Advanced pass thru</td>
			<td width="78%" class="vtable" colspan="2">
				<textarea name='advanced' rows="4" cols="70" id='advanced'><?php echo $pconfig['advanced']; ?></textarea>
				<br/>
				NOTE: paste text into this box that you would like to pass thru.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save">  
				<input type="button" class="formbtn" value="Cancel" onclick="history.back()">
				<?php if (isset($id) && $a_server[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>">
				<?php endif; ?>
			</td>
		</tr>
	</table>
	</div></td></tr></table>
	</form>
<br>
<?php include("fend.inc"); ?>
<script type="text/javascript">
	field_counter_js = 1;
	rows = 1;
	totalrows = <?php echo $counter; ?>;
	loaded = <?php echo $counter; ?>;
</script>
</body>
</html>

<?php

function row_helper() {
	global $pconfig, $a_backend;
	$options = "";
	if($a_backend) {
		foreach ($a_backend as $backend) {
			$options .= "<option value='{$backend['name']}'";
			if($backend['name'] == $pconfig['backend']) 
				$options .=  "SELECTED";
			$options .=  ">";
			$options .=  $backend['name'];
			$options .=  "</option>";
		}
	}
	
	echo <<<EOF
<script type="text/javascript">
// Global Variables
var rowname = new Array(99);
var rowtype = new Array(99);
var newrow  = new Array(99);
var rowsize = new Array(99);

for (i = 0; i < 99; i++) {
	rowname[i] = '';
	rowtype[i] = '';
	newrow[i] = '';
	rowsize[i] = '25';
}

var field_counter_js = 0;
var loaded = 0;
var is_streaming_progress_bar = 0;
var temp_streaming_text = "";

var addRowTo = (function() {
    return (function (tableId) {
	var d, tbody, tr, td, bgc, i, ii, j;
	d = document;
	tbody = d.getElementById(tableId).getElementsByTagName("tbody").item(0);
	tr = d.createElement("tr");
	totalrows++;
	for (i = 0; i < field_counter_js; i++) {
		td = d.createElement("td");
		if(rowtype[i] == 'textbox') {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows + "'></input><input size='" + rowsize[i] + "' name='" + rowname[i] + totalrows + "'></input> ";
		} else if(rowtype[i] == 'select') {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows + "'></input><select size='" + rowsize[i] + "' name='" + rowname[i] + totalrows + "'>$options</select> ";
		} else {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows + "'></input><input type='checkbox' name='" + rowname[i] + totalrows + "'></input> ";
		}
		tr.appendChild(td);
	}
	td = d.createElement("td");
	td.rowSpan = "1";

	td.innerHTML = '<input type="image" src="/themes/' + theme + '/images/icons/icon_x.gif" onclick="removeRow(this); return false;" value="Delete">';
	tr.appendChild(td);
	tbody.appendChild(tr);
    });
})();

function removeRow(el) {
    var cel;
    while (el && el.nodeName.toLowerCase() != "tr")
	    el = el.parentNode;

    if (el && el.parentNode) {
	cel = el.getElementsByTagName("td").item(0);
	el.parentNode.removeChild(el);
    }
}

function find_unique_field_name(field_name) {
	// loop through field_name and strip off -NUMBER
	var last_found_dash = 0;
	for (var i = 0; i < field_name.length; i++) {
		// is this a dash, if so, update
		//    last_found_dash
		if (field_name.substr(i,1) == "-" )
			last_found_dash = i;
	}
	if (last_found_dash < 1)
		return field_name;
	return(field_name.substr(0,last_found_dash));
}
</script>

EOF;

}

?>