<?php
/* $Id: load_balancer_pool_edit.php,v 1.24.2.23 2007/03/03 00:07:09 smos Exp $ */
/*
	haproxy_servers_edit.php
	part of pfSense (http://www.pfsense.com/)
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
	$pconfig['cookie'] = $a_server[$id]['cookie'];
	$pconfig['status'] = $a_server[$id]['status'];
}

$changedesc = "Services: HAProxy: Servers: ";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;
	
	$reqdfields = explode(" ", "name address port backend weight cookie");
	$reqdfieldsn = explode(",", "Name,Address,Port,Backend,Weight,Cookie");		

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['name']))
		$input_errors[] = "The field 'Name' contains invalid characters.";

	if (preg_match("/[^a-zA-Z0-9\.]/", $_POST['address']))
		$input_errors[] = "The field 'Address' contains invalid characters.";

	if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['cookie']))
		$input_errors[] = "The field 'Cookie' contains invalid characters.";

	if (!is_numeric($_POST['port']))
		$input_errors[] = "The field 'Port' value is not a number.";
	else {
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

	if (!$input_errors) {
		$server = array();
		if(isset($id) && $a_server[$id])
			$server = $a_server[$id];
			
		if($server['name'] != "")
			$changedesc .= " modified '{$server['name']}' pool:";
		
		update_if_changed("name", $server['name'], $_POST['name']);
		update_if_changed("address", $server['address'], $_POST['address']);
		update_if_changed("port", $server['port'], $_POST['port']);
		update_if_changed("backend", $server['backend'], $_POST['backend']);
		update_if_changed("cookie", $server['cookie'], $_POST['cookie']);
		update_if_changed("weight", $server['weight'], $_POST['weight']);
		update_if_changed("status", $server['status'], $_POST['status']);
		
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

$pgtitle = "HAProxy: Server: Edit";
include("head.inc");

?>

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

<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>

	<form action="haproxy_servers_edit.php" method="post" name="iform" id="iform">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit haproxy server</td>
		</tr>	
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="16" maxlength="16">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Address</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="address" type="text" <?if(isset($pconfig['address'])) echo "value=\"{$pconfig['address']}\"";?> size="64">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Port</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="port" type="text" <?if(isset($pconfig['port'])) echo "value=\"{$pconfig['port']}\"";?> size="5">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Backend</td>
			<td width="78%" class="vtable" colspan="2">
				<select name="backend">
				<?php 
					$i = 0; 
					if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
						$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
					}
					$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

					foreach ($a_backend as $backend) {
				?>
				<option value="<?=$backend['name'];?>" <?php if($backend['name']==$pconfig['backend']) echo "SELECTED";?>><?=$backend['name'];?></option>
				<?php } ?>
				</select>
				<div></div>
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
				  operational server possessing the same value will be selected. In return, in
				  cookie insertion or rewrite modes, this value will be assigned to the cookie
				  sent to the client. There is nothing wrong in having several servers sharing
				  the same cookie value, and it is in fact somewhat common between normal and
				  backup servers. See also the "cookie" keyword in backend section.
				
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncell">Weight</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="weight" type="text" <?if(isset($pconfig['weight'])) echo "value=\"{$pconfig['weight']}\"";?>size="64"><br/>
				The default weight is 1, and the maximal value is 255.<br/>
				NOTE: If this 
					  parameter is used to distribute the load according to server's capacity, it 
					  is recommended to start with values which can both grow and shrink, for 
					  instance between 10 and 100 to leave enough room above and below for later 
					  adjustments.
				
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save">
				<?php if (isset($id) && $a_server[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>">
				<?php endif; ?>
			</td>
		</tr>
	</table>
	</form>
<br>
<?php include("fend.inc"); ?>
</body>
</html>
