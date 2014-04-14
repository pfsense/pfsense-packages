<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_pool_edit.php
	part of pfSense (https://www.pfsense.org/)
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

require("guiconfig.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

if (!is_array($config['installedpackages']['haproxy']['ha_pools']['item'])) {
	$config['installedpackages']['haproxy']['ha_pools']['item'] = array();
}

$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($_GET['dup']))
	$id = $_GET['dup'];

if (isset($id) && $a_pools[$id]) {
	$pconfig['name'] = $a_pools[$id]['name'];
	$pconfig['checkinter'] = $a_pools[$id]['checkinter'];
	$pconfig['monitor_uri'] = $a_pools[$id]['monitor_uri'];
	$pconfig['cookie'] = $a_pools[$id]['cookie'];
	$pconfig['status'] = $a_pools[$id]['status'];
	$pconfig['advanced'] = base64_decode($a_pools[$id]['advanced']);
	$pconfig['a_servers']=&$a_pools[$id]['ha_servers']['item'];	
}

if (isset($_GET['dup']))
	unset($id);

$changedesc = "Services: HAProxy: pools: ";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	$reqdfields = explode(" ", "name");
	$reqdfieldsn = explode(",", "Name");		

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";

	/* Ensure that our pool names are unique */
	for ($i=0; isset($config['installedpackages']['haproxy']['ha_pools']['item'][$i]); $i++)
		if (($_POST['name'] == $config['installedpackages']['haproxy']['ha_pools']['item'][$i]['name']) && ($i != $id))
			$input_errors[] = "This pool name has already been used.  Pool names must be unique.";

	$a_servers=array();			
	for($x=0; $x<99; $x++) {
		$server_name=$_POST['server_name'.$x];
		$server_address=$_POST['server_address'.$x];
		$server_port=$_POST['server_port'.$x];
		$server_weight=$_POST['server_weight'.$x];
		$server_backup=$_POST['server_backup'.$x];

		if ($server_address) {

			$server=array();
			$server['name']=$server_name;
			$server['address']=$server_address;
			$server['port']=$server_port;
			$server['weight']=$server_weight;
			$server['backup']=$server_backup;
			$a_servers[]=$server;

			if (preg_match("/[^a-zA-Z0-9\.\-_]/", $server_name))
				$input_errors[] = "The field 'Name' contains invalid characters.";
			if (preg_match("/[^a-zA-Z0-9\.\-_]/", $server_address))
				$input_errors[] = "The field 'Address' contains invalid characters.";

			if (!preg_match("/.{2,}/", $server_name))
				$input_errors[] = "The field 'Name' is required.";

			if (!preg_match("/.{2,}/", $server_address))
				$input_errors[] = "The field 'Address' is required.";

			if (!preg_match("/.{2,}/", $server_weight))
				$input_errors[] = "The field 'Weight' is required.";

			if (!is_numeric($server_weight))
				$input_errors[] = "The field 'Weight' value is not a number.";
			if ($server_port && !is_numeric($server_port))
				$input_errors[] = "The field 'Port' value is not a number.";
		}
	}

	if (!$input_errors) {
		$pool = array();
		if(isset($id) && $a_pools[$id])
			$pool = $a_pools[$id];
			
		if ($pool['name'] != $_POST['name']) {
			// name changed:
			if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
				$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
			}
			$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

			for ( $i = 0; $i < count($a_backend); $i++) {
				if ($a_backend[$i]['pool'] == $pool['name'])
					$a_backend[$i]['pool'] = $_POST['name'];
			}
		}

		if($pool['name'] != "")
			$changedesc .= " modified '{$pool['name']}' pool:";

		$pool['ha_servers']['item']=$a_servers;

		update_if_changed("name", $pool['name'], $_POST['name']);
		update_if_changed("status", $pool['status'], $_POST['status']);
		update_if_changed("cookie", $pool['cookie'], $_POST['cookie']);
		update_if_changed("advanced", $pool['advanced'], base64_encode($_POST['advanced']));
		update_if_changed("checkinter", $pool['checkinter'], $_POST['checkinter']);
		update_if_changed("monitor_uri", $pool['monitor_uri'], $_POST['monitor_uri']);

		if (isset($id) && $a_pools[$id]) {
			$a_pools[$id] = $pool;
		} else {
			$a_pools[] = $pool;
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

		header("Location: haproxy_pools.php");
		exit;
	}
	$pconfig['a_servers']=&$a_pools[$id]['ha_servers']['item'];	
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;

$pgtitle = "HAProxy: pool: Edit";
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
	rowname[0] = "server_name";
	rowtype[0] = "textbox";
	rowsize[0] = "30";
	rowname[1] = "server_address";
	rowtype[1] = "textbox";
	rowsize[1] = "30";
	rowname[2] = "server_port";
	rowtype[2] = "textbox";
	rowsize[2] = "5";
	rowname[3] = "server_weight";
	rowtype[3] = "textbox";
	rowsize[3] = "5";
	rowname[4] = "server_backup";
	rowtype[4] = "checkbox";
	rowsize[4] = "5";
</script>
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
	<form action="haproxy_pool_edit.php" method="post" name="iform" id="iform">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit HAProxy pool</td>
		</tr>	
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="16" maxlength="16">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Status</td>
			<td width="78%" class="vtable" colspan="2">
				<select name="status">
				<option value="active"  <?php if($pconfig['status']=='active') echo "SELECTED";?>>active</option>
				<option value="inactive" <?php  if($pconfig['status']=='inactive') echo "SELECTED";?>>inactive</option>
				</select>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Cookie</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="cookie" type="text" <?if(isset($pconfig['cookie'])) echo "value=\"{$pconfig['cookie']}\"";?>size="64"><br/>
				  This value will be checked in incoming requests, and the first
				  operational pool possessing the same value will be selected. In return, in
				  cookie insertion or rewrite modes, this value will be assigned to the cookie
				  sent to the client. There is nothing wrong in having several servers sharing
				  the same cookie value, and it is in fact somewhat common between normal and
				  backup servers. See also the "cookie" keyword in backend section.
				
			</td>
		</tr>
		<tr align="left">
			<td class="vncellreq" colspan="3">Server list</td>
		</tr>
		<tr>
			<td width="78%" class="vtable" colspan="2" valign="top">
			<table class="" width="100%" cellpadding="0" cellspacing="0" id='servertable'>
	                <tr>
	                  <td width="30%" class="">Name</td>
	                  <td width="30%" class="">Address</td>
	                  <td width="18%" class="">Port</td>
	                  <td width="18%" class="">Weight</td>
	                  <td width="5%" class="">Backup</td>
	                  <td width="4%" class=""></td>
			</tr>
			<?php 
			$a_servers=$pconfig['a_servers'];

			if (!is_array($a_servers)) {
				$a_servers=array();
			}

			$counter=0;
			foreach ($a_servers as $server) {
			?>
			<tr id="tr_view_<?=$counter;?>" name="tr_view_<?=$counter;?>">
			<td class="vtable"><?=$server['name']; ?></td>
			<td class="vtable"><?=$server['address']; ?></td>
			<td class="vtable"><?=$server['port']; ?></td>
			<td class="vtable"><?=$server['weight']; ?></td>
			<td class="vtable"><?=$server['backup']; ?></td>
			<td class="list">
			  <table border="0" cellspacing="0" cellpadding="1"><tr>
			  <td valign="middle">
			  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="edit entry" width="17" height="17" border="0" onclick="editRow(<?=$counter;?>); return false;">
			  </td>
			  <td valign="middle">
			  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="duplicate entry" width="17" height="17" border="0" onclick="dupRow(<?=$counter;?>, 'servertable'); return false;">
			  </td></tr></table>
			</td>
			</tr>
			<tr id="tr_edit_<?=$counter;?>" name="tr_edit_<?=$counter;?>" style="display: none;">
				<td class="vtable">
				  <input name="server_name<?=$counter;?>" id="server_name<?=$counter;?>" type="text" value="<?=$server['name']; ?>" size="30"/></td>
				<td class="vtable">
				  <input name="server_address<?=$counter;?>" id="server_address<?=$counter;?>" type="text" value="<?=$server['address']; ?>" size="30"/></td>
				<td class="vtable">
				  <input name="server_port<?=$counter;?>" id="server_port<?=$counter;?>" type="text" value="<?=$server['port']; ?>" size="5"/></td>
				<td class="vtable">
				  <input name="server_weight<?=$counter;?>" id="server_weight<?=$counter;?>" type="text" value="<?=$server['weight']; ?>" size="5"/></td>
				<td class="vtable">
				  <input name="server_backup<?=$counter;?>" id="server_backup<?=$counter;?>" type="checkbox" value="yes" <?php if ($server['backup']=='yes') echo "checked"; ?>/></td>
				<td class="list">
			         <table border="0" cellspacing="0" cellpadding="1"><tr>
			         <td valign="middle">
				  <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="removeRow(this); return false;">
			         </td>
			         <td valign="middle">
				 <img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="duplicate entry" width="17" height="17" border="0" onclick="dupRow(<?=$counter;?>, 'servertable'); return false;">
			         </td></tr></table>
				</td>
			</tr>
			<?php
			$counter++;
			}
			?>
			</table>
			<a onclick="javascript:addRowTo('servertable'); return false;" href="#">
			<img border="0" src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" alt="" title="add another entry" />
			</a>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Check freq</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="checkinter" type="text" <?if(isset($pconfig['checkinter'])) echo "value=\"{$pconfig['checkinter']}\"";?>size="20"> milliseconds
				<br/>Defaults to 1000 if left blank.
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Health check URI</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="monitor_uri" type="text" <?if(isset($pconfig['monitor_uri'])) echo "value=\"{$pconfig['monitor_uri']}\"";?>size="64">
				<br/>Defaults to / if left blank.
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
				<?php if (isset($id) && $a_pools[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>">
				<?php endif; ?>
			</td>
		</tr>
	</table>
	</form>
<br>
<?php include("fend.inc"); ?>
<script type="text/javascript">
	field_counter_js = 5;
	rows = 1;
	totalrows =  <?php echo $counter; ?>;
	loaded =  <?php echo $counter; ?>;
</script>
</body>
</html>

<?php

function row_helper() {
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
	var btable, btbody, btr, btd;

	d = document;
	tbody = d.getElementById(tableId).getElementsByTagName("tbody").item(0);
	tr = d.createElement("tr");
	totalrows++;
	for (i = 0; i < field_counter_js; i++) {
		td = d.createElement("td");
		if(rowtype[i] == 'textbox') {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
				"'></input><input size='" + rowsize[i] + "' name='" + rowname[i] + totalrows +
				"' id='" + rowname[i] + totalrows + "'></input> ";
		} else if(rowtype[i] == 'select') {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
				"'></input><select size='" + rowsize[i] + "' name='" + rowname[i] + totalrows +
			       	"' id='" + rowname[i] + totalrows + "'>$options</select> ";
		} else {
			td.innerHTML="<INPUT type='hidden' value='" + totalrows +"' name='" + rowname[i] + "_row-" + totalrows +
				"'></input><input type='checkbox' name='" + rowname[i] + totalrows +
				"' id='" + rowname[i] + totalrows + "' value='yes'></input> ";
		}
		td.setAttribute("class","vtable");
		tr.appendChild(td);
	}
	td = d.createElement("td");
	td.rowSpan = "1";
        td.setAttribute("class","list");

	// Recreate the button table.
	btable = document.createElement("table");
	btable.setAttribute("border", "0");
	btable.setAttribute("cellspacing", "0");
	btable.setAttribute("cellpadding", "1");
	btbody = document.createElement("tbody");
	btr = document.createElement("tr");
	btd = document.createElement("td");
	btd.setAttribute("valign", "middle");
	btd.innerHTML = '<img src="/themes/' + theme + '/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="removeRow(this); return false;">';
	btr.appendChild(btd);
	btd = document.createElement("td");
	btd.setAttribute("valign", "middle");
	btd.innerHTML = '<img src="/themes/' + theme + "/images/icons/icon_plus.gif\" title=\"duplicate entry\" width=\"17\" height=\"17\" border=\"0\" onclick=\"dupRow(" + totalrows + ", 'servertable'); return false;\">";
	btr.appendChild(btd);
	btbody.appendChild(btr);
	btable.appendChild(btbody);

	td.appendChild(btable);
	tr.appendChild(td);
	tbody.appendChild(tr);
    });
})();

function dupRow(rowId, tableId) {
    var dupEl;
    var newEl;

    addRowTo(tableId);
    for (i = 0; i < field_counter_js; i++) {
        dupEl = document.getElementById(rowname[i] + rowId);
        newEl = document.getElementById(rowname[i] + totalrows);
	if (dupEl && newEl)
	    if(rowtype[i] == 'checkbox')
		newEl.checked = dupEl.checked;
	    else
                newEl.value = dupEl.value;
    }
}

function removeRow(el) {
    var cel;
    // Break out of one table first
    while (el && el.nodeName.toLowerCase() != "table")
	    el = el.parentNode;
    while (el && el.nodeName.toLowerCase() != "tr")
	    el = el.parentNode;

    if (el && el.parentNode) {
	cel = el.getElementsByTagName("td").item(0);
	el.parentNode.removeChild(el);
    }
}
function editRow(num) {
    var trview = document.getElementById('tr_view_' + num);
    var tredit = document.getElementById('tr_edit_' + num);

    trview.style.display='none';
    tredit.style.display='';
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
