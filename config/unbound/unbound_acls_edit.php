<?php
/* $Id$ */
/*
	unbound_acls_edit.php
	part of pfSense (http://www.pfsense.com)
	Copyright (C) 2011 Warren Baker (warren@decoy.co.za)
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

if (!is_array($config['installedpackages']['unboundacls']['config'])) {
	$config['installedpackages']['unboundacls']['config'] = array();
}

$a_acl = &$config['installedpackages']['unboundacls']['config'];

$id = $_GET['id'];
if (is_numeric($_POST['id']))
	$id = $_POST['id'];

if (isset($id) && $a_acl[$id]) {

	if (!isset($a_acl[$id]['aclaction']))
		$pconfig['aclaction'] = "allow";
	else
		$pconfig['aclaction'] = $a_acl[$id]['aclaction'];

	$pconfig['descr'] = $a_acl[$id]['descr'];

} else {
	/* defaults */
	$pconfig['aclaction'] = "allow";
}

if ($_POST) {
	
	print_r($_POST);
	exit;
	unset($input_errors);
	$pconfig = $_POST;
	
	/* input validation */
	$reqdfields = explode(" ", "src");
	$reqdfieldsn = explode(",", "Source");
	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	if ($pconfig['ipprotocol'] == "inet") {
		if(!is_ipaddr($pconfig['inetsrc']))
			$input_errors[] = gettext("You must enter a valid IPv4 IP address.");
	} else {
		if(!is_ipaddrv6($pconfig['inet6src']))
			$input_errors[] = gettext("You must enter a valid IPv6 IP address.");
	}


	if (!$input_errors) {
		$aclent = array();
		$aclent['aclaction'] = $_POST['aclaction'];
				
		if ($pconfig['ipprotocol'] == "inet") {
			$aclent['acl_network'] = $_POST['inetsrc']."/".$_POST['inetsrcmask'];

		} else {
			$aclent['acl_network'] = $_POST['inet6src']."/".$_POST['inet6srcmask'];

		}
		strncpy($aclent['descr'], $_POST['descr'], 52);
		
			
		if ($_POST['disabled'])
			$aclent['disabled'] = true;
		else
			unset($aclent['disabled']);
	


		if (isset($id) && $a_acl[$id])
			$a_acl[$id] = $aclent;
		else {
			if (is_numeric($after))
				array_splice($a_acl, $after+1, 0, array($aclent));
			else
				$a_acl[] = $aclent;
		}

		write_config();

		header("Location: unbound_acls.php");
		exit;
	}
}

$pgtitle = array(gettext("Services"),gettext("Unbound ACLs"),gettext("Edit"));
$statusurl = "unbound_status.php";
$logurl = "diag_pkglogs.php?pkg=Unbound";

$page_filename = "unbound_acls_edit.php";
include("head.inc");

?>
<script type="text/javascript" language="javascript" src="/javascript/row_helper_dynamic.js">

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="Effect.Appear('inet', { duration : 0.5 });">
<script language="JavaScript"> 
function doFade( optionValue )
{
	switch( optionValue )
	{
		case "inet" :
		Effect.Appear('IPV4',{ duration: 0.5 });
		Effect.Fade('IPV6', { duration: 0.1 });
		break;

		case "inet6" :
		Effect.Appear('IPV6',{ duration: 0.5 });
		Effect.Fade('IPV4', { duration: 0.1 });
		break;
									
	}
} 
</script>

<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>

<form action="unbound_acls_edit.php" method="post" name="iform" id="iform">
<input type='hidden' name="aclid" value="<?=(isset($pconfig['aclid']) && $pconfig['aclid']>0)?htmlspecialchars($pconfig['aclid']):''?>">

	<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td colspan="2" valign="top" class="listtopic"><?=gettext("Edit ACL");?></td>
		</tr>	
    	<tr>
			<td width="22%" valign="top" class="vncellreq"><?=gettext("Action");?></td>
			<td width="78%" class="vtable">
				<select name="aclaction" class="formselect">
					<?php $types = explode(",", "Deny,Refuse,Allow,Allow Snoop"); foreach ($types as $type): ?>
					<option value="<?=strtolower($type);?>" <?php if (strtolower($type) == strtolower($pconfig['type'])) echo "selected"; ?>>
					<?=htmlspecialchars($type);?>
					</option>
					<?php endforeach; ?>
				</select>
				<br/>
				<span class="vexpl">
					<?=gettext("Choose what to do with DNS requests that match the criteria specified below.");?> <br/>
					<?=gettext("<b>Deny:</b> This actions stops queries from hosts within the netblock defined below.");?> <br/>
					<?=gettext("<b>Refuse:</b> This actions also stops queries from hosts within the netblock defined below, but sends back DNS rcode REFUSED error message back tot eh client.");?> <br/>
					<?=gettext("<b>Allow:</b> This actions allows queries from hosts within the netblock defined below.");?> <br/>
					<?=gettext("<b>Allow Snoop:</b> This actions allows recursive and nonrecursive access from hosts within the netblock defined below. Used for cache snooping and ideally should only be configured for your administrative host.");?> <br/>
				</span>
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq"><?=gettext("Disabled");?></td>
			<td width="78%" class="vtable">
				<input name="disabled" type="checkbox" id="disabled" value="yes" <?php if ($pconfig['disabled']) echo "checked"; ?>>
				<strong><?=gettext("Disable this ACL");?></strong><br />
				<span class="vexpl"><?=gettext("Set this option to disable this ACL without removing it from the list.");?></span>
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq"><?=gettext("TCP/IP Version");?></td>
			<td width="78%" class="vtable">
				<select name="ipprotocol" class="formselect" onchange="doFade( this.value )">
				<?php      $ipproto = array('inet' => 'IPv4','inet6' => 'IPv6');
					foreach ($ipproto as $proto => $name): ?>
						<option value="<?=$proto;?>"
							<?php if ($proto == $pconfig['ipprotocol']): ?>
								selected="selected"
							<?php endif; ?>
							><?=$name;?></option>
					<?php endforeach; ?>
				</select>
				<strong><?=gettext("Select the Internet Protocol version this rule applies to");?></strong><br/>
			</td>
        </tr>
		<tr>
			<td width="22%" valign="top" class="vncellreq"><?=gettext("Networks");?></td>
			<td width="78%" class="vtable">
				<script type="text/javascript" language='javascript'> 
							<!--
							rowname[0] = "acl_network";
							rowtype[0] = "input";
							rowname[1] = "mask";
							rowtype[1] = "select";
							-->
							</script> 
			
				<table border="0" cellspacing="0" cellpadding="0" id="IPV4">
					<tr>
						<td>&nbsp;<?=gettext("Network");?></td>
						<td>&nbsp;&nbsp;<?=gettext("CIDR");?></td>
					</tr>
					<tr>
						<td><input name="src" type="text" id="src" size="20" value="<?php echo htmlspecialchars($pconfig['src']);?>"> / </td>
						<td>&nbsp;
							<select name="srcmask" class="formselect" id="srcmask">
<?php						for ($i = 31; $i > 0; $i--): ?>
								<option value="<?=$i;?>" <?php if ($i == $pconfig['srcmask']) echo "selected"; ?>><?=$i;?></option>
<?php 						endfor; ?>
							</select>
						</td>
					</tr>
				</table>
				
				<table border="0" cellspacing="0" cellpadding="0" id="IPV6">
					<tr>
						<td><?=gettext("Type:");?>&nbsp;&nbsp;</td>
						<td>
							<select name="inet6srctype" class="formselect" onChange="typeselinet6_change()">
								<option value="network"><?=gettext("Network");?></option>
							</select>
						</td>
					</tr>
					<tr>
						<td><?=gettext("Network:");?>&nbsp;&nbsp;</td>
						<td>
							<input autocomplete='off' name="inet6src" type="text" id="inet6src" size="20" value="<?php echo htmlspecialchars($pconfig['inet6src']);?>"> /
							<select name="inet6srcmask" class="formselect" id="inet6srcmask">
							<?php	for ($i = 128; $i > 0; $i--): ?>
								<option value="<?=$i;?>" <?php if ($i == $pconfig['inet6srcmask']) echo "selected"; ?>><?=$i;?></option>
							<?php	endfor; ?>
							</select>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell"><?=gettext("Description");?></td>
			<td width="78%" class="vtable">
				<input name="descr" type="text" class="formfld unknown" id="descr" size="52" maxlength="52" value="<?=htmlspecialchars($pconfig['descr']);?>">
				<br />
				<span class="vexpl"><?=gettext("You may enter a description here for your reference.");?></span>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
		<tr>
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				&nbsp;<br>&nbsp;
				<input name="Submit" type="submit" class="formbtn" value="<?=gettext("Save"); ?>">  <input type="button" class="formbtn" value="<?=gettext("Cancel"); ?>" onclick="history.back()">
<?php			if (isset($id) && $a_filter[$id]): ?>
					<input name="id" type="hidden" value="<?=htmlspecialchars($id);?>">
<?php 			endif; ?>
				<input name="after" type="hidden" value="<?=htmlspecialchars($after);?>">
			</td>
		</tr>
	</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
