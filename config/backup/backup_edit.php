<?php 
/* $Id$ */
/*

	backup_edit.php
	Copyright (C) 2008 Mark J Crane
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
require("/usr/local/pkg/backup.inc");


$a_backup = &$config['installedpackages']['backup']['config'];

$id = $_GET['id'];
if (isset($_POST['id'])) {
	$id = $_POST['id'];
}

if ($_GET['act'] == "del") {
	if ($_GET['type'] == 'backup') {
		if ($a_backup[$_GET['id']]) {
			conf_mount_rw();
			unset($a_backup[$_GET['id']]);
			write_config();
			backup_sync_package();
			header("Location: backup.php");
			conf_mount_ro();
			exit;
		}
	}
}

if (isset($id) && $a_backup[$id]) {

	$pconfig['name'] = $a_backup[$id]['name'];
	$pconfig['path'] = $a_backup[$id]['path'];
	$pconfig['enabled'] = $a_backup[$id]['enabled'];
	$pconfig['description'] = $a_backup[$id]['description'];

}

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;
  
	if (!$input_errors) {

		$ent = array();
		$ent['name'] = $_POST['name'];
		$ent['path'] = $_POST['path'];
		$ent['enabled'] = $_POST['enabled'];
		$ent['description'] = $_POST['description'];

		if (isset($id) && $a_backup[$id]) {
		  	//update
      		$a_backup[$id] = $ent;
		}
		else {
		  	//add 
			$a_backup[] = $ent;
		}

		write_config();
		backup_sync_package();

		header("Location: backup.php");
		exit;
	}
}

include("head.inc");

?>

<script type="text/javascript" language="JavaScript">

function show_advanced_config() {
	document.getElementById("showadvancedbox").innerHTML='';
	aodiv = document.getElementById('showadvanced');
	aodiv.style.display = "block";
</script>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle">Backup: Edit</p>
<?php if ($input_errors) print_input_errors($input_errors); ?>


<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td class="tabnavtbl">
<?php

	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/packages/backup/backup.php");
	display_top_tabs($tab_array);

?>
</td></tr>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
	 <td class="tabcont" >

	<!--
	<table width="100%" border="0" cellpadding="6" cellspacing="0">              
	  <tr>
		<td><p><span class="vexpl"><span class="red"><strong>PHP<br>
			</strong></span>
			</p></td>
	  </tr>
	</table>
	-->
	<br />

		<form action="backup_edit.php" method="post" name="iform" id="iform">
			<table width="100%" border="0" cellpadding="6" cellspacing="0">

			<tr>
			  <td width="25%" valign="top" class="vncellreq">Name</td>
			  <td width="75%" class="vtable"> 
				<input name="name" type="text" class="formfld" id="name" size="40" value="<?=htmlspecialchars($pconfig['name']);?>">
			  </td>
			</tr> 

			<tr>
				<td width="22%" valign="top" class="vncellreq">Path</td>
				<td width="78%" class="vtable">
					<input name="path" type="text" class="formfld" id="path" size="40" value="<?=htmlspecialchars($pconfig['path']);?>">
				</td>
			</tr>
			<tr>
			  <td width="22%" valign="top" class="vncellreq">Enabled</td>
			  <td width="78%" class="vtable"> 
				<?php                
				echo "              <select name='enabled' class='formfld'>\n";
				echo "                <option></option>\n";
				switch (htmlspecialchars($pconfig['enabled'])) {
				case "true":
					echo "              <option value='true' selected='yes'>true</option>\n";
					echo "              <option value='false'>false</option>\n";
					break;
				case "false":
					echo "              <option value='true'>true</option>\n";
					echo "              <option value='false' selected='yes'>false</option>\n";

					break;
				default:
					echo "              <option value='true' selected='yes'>true</option>\n";
					echo "              <option value='false'>false</option>\n";
				}
				echo "              </select>\n";
				?>          
			  </td>
			</tr>
			<tr>
			  <td width="25%" valign="top" class="vncellreq">Description</td>
			  <td width="75%" class="vtable"> 
				<input name="description" type="text" class="formfld" id="description" size="40" value="<?=htmlspecialchars($pconfig['description']);?>">
				<br><span class="vexpl">Enter the description here.<br></span>
			  </td>
			</tr>
			
			<tr>
			  <td valign="top">&nbsp;</td>
			  <td>
				<input name="Submit" type="submit" class="formbtn" value="Save"> <input class="formbtn" type="button" value="Cancel" onclick="history.back()">
				<?php if (isset($id) && $a_backup[$id]): ?>
				  <input name="id" type="hidden" value="<?=$id;?>">
				<?php endif; ?>
			  </td>
			</tr>
			</table>
		</form>

	  <br>
	  <br>
	  <br>
	  <br>
	  <br>
	  <br>

	 </td>
	</tr>
</table>

</div>

<?php include("fend.inc"); ?>
</body>
</html>
