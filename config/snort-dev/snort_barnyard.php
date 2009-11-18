<?php
/* $Id$ */
/*
	snort_interfaces.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
	Copyright (C) 2008-2009 Robert Zelaya.
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

/* 

TODO: Nov 12 09
Clean this code up its ugly
Important add error checking

*/

require("guiconfig.inc");

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
//nat_rules_sort();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (isset($_GET['dup'])) {
        $id = $_GET['dup'];
        $after = $_GET['dup'];
}

if (isset($id) && $a_nat[$id]) {

	/* new options */
	$pconfig['barnyard_enable'] = $a_nat[$id]['barnyard_enable'];
	$pconfig['barnyard_mysql'] = $a_nat[$id]['barnyard_mysql'];
	/* old options */
	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['descr'] = $a_nat[$id]['descr'];
	$pconfig['performance'] = $a_nat[$id]['performance'];
	$pconfig['blockoffenders7'] = $a_nat[$id]['blockoffenders7'];
	$pconfig['snortalertlogtype'] = $a_nat[$id]['snortalertlogtype'];
	$pconfig['alertsystemlog'] = $a_nat[$id]['alertsystemlog'];
	$pconfig['tcpdumplog'] = $a_nat[$id]['tcpdumplog'];
	$pconfig['snortunifiedlog'] = $a_nat[$id]['snortunifiedlog'];
	$pconfig['flow_depth'] = $a_nat[$id]['flow_depth'];
		
	if (!$pconfig['interface'])
		$pconfig['interface'] = "wan";
} else {
	$pconfig['interface'] = "wan";
}

if (isset($_GET['dup']))
	unset($id);

if ($_POST) {

	/* check for overlaps */
	foreach ($a_nat as $natent) {
		if (isset($id) && ($a_nat[$id]) && ($a_nat[$id] === $natent))
			continue;
		if ($natent['interface'] != $_POST['interface'])
			continue;
	}

/* if no errors write to conf */
	if (!$input_errors) {
		$natent = array();
		/* repost the options already in conf */
		$natent['enable'] = $pconfig['enable'];
		$natent['interface'] = $pconfig['interface'];
		$natent['descr'] = $pconfig['descr'];
		$natent['performance'] = $pconfig['performance'];
		$natent['blockoffenders7'] = $pconfig['blockoffenders7'];
		$natent['snortalertlogtype'] = $pconfig['snortalertlogtype'];
		$natent['alertsystemlog'] = $pconfig['alertsystemlog'];
		$natent['tcpdumplog'] = $pconfig['tcpdumplog'];
		$natent['flow_depth'] = $pconfig['flow_depth'];
		/* post new options */
		$natent['barnyard_enable'] = $_POST['barnyard_enable'] ? on : off;
		$natent['barnyard_mysql'] = $_POST['barnyard_mysql'] ? $_POST['barnyard_mysql'] : $pconfig['barnyard_mysql'];
		if ($_POST['barnyard_enable'] == "on") { $natent['snortunifiedlog'] = on; }else{ $natent['snortunifiedlog'] = off; } if ($_POST['barnyard_enable'] == "") { $natent['snortunifiedlog'] = off; }

		if (isset($id) && $a_nat[$id])
			$a_nat[$id] = $natent;
		else {
			if (is_numeric($after))
				array_splice($a_nat, $after+1, 0, array($natent));
			else
				$a_nat[] = $natent;
		}

		/* enable this if you want the user to aprove changes */
		// touch($d_natconfdirty_path);

		write_config();
        
		/* after click go to this page */
		header("Location: snort_barnyard.php?id=$id");
		exit;
	}
}

$pgtitle = "Services: Snort Barnyard2 Edit";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php 
include("fbegin.inc");
?>
<style type="text/css">
.alert {
 position:absolute;
 top:10px;
 left:0px;
 width:94%;
background:#FCE9C0;
background-position: 15px; 
border-top:2px solid #DBAC48;
border-bottom:2px solid #DBAC48;
padding: 15px 10px 85% 50px;
}
</style> 
<noscript><div class="alert" ALIGN=CENTER><img src="/themes/nervecenter/images/icons/icon_alert.gif"/><strong>Please enable JavaScript to view this content</CENTER></div></noscript>
<script language="JavaScript">
<!--

function enable_change(enable_change) {
	endis = !(document.iform.barnyard_enable.checked || enable_change);
	// make shure a default answer is called if this is envoked.
	endis2 = (document.iform.barnyard_enable);

<?php
/* make shure all the settings exist or function hide will not work */
/* if $id is emty allow if and discr to be open */
if($id != "") 
{
echo "	
	document.iform.interface.disabled = endis2;\n";
}
?>
    document.iform.barnyard_mysql.disabled = endis;
}
//-->
</script>
<p class="pgtitle"><?=$pgtitle?></p>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<form action="snort_barnyard.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
if($id != "") 
{

    $tab_array = array();
    $tab_array[] = array("Snort Interfaces", false, "/snort_interfaces.php");
    $tab_array[] = array("If Settings", false, "/snort_interfaces_edit.php");
    $tab_array[] = array("Categories", false, "/snort/snort_{$snortIf}_{$id}/snort_rulesets_{$snortIf}_{$id}.php");
    $tab_array[] = array("Rules", false, "/snort/snort_{$snortIf}_{$id}/snort_rules_{$snortIf}_{$id}.php");
    $tab_array[] = array("Servers", false, "/pkg_edit.php?xml=snort/snort_{$snortIf}_{$id}/snort_define_servers_{$snortIf}_{$id}.xml&amp;id=0");
    $tab_array[] = array("Barnyard2", false, "/pkg_edit.php?xml=snort/snort_{$snortIf}_{$id}/snort_barnyard2_{$snortIf}_{$id}.xml&id=0");
    $tab_array[] = array("Barnyard2", false, "/pkg_edit.php?xml=snort/snort_{$snortIf}_{$id}/snort_barnyard2_{$snortIf}_{$id}.xml&id=0");
    $tab_array[] = array("Barnyard2", true, "/pkg_edit.php?xml=snort/snort_{$snortIf}_{$id}/snort_barnyard2_{$snortIf}_{$id}.xml&id=0");
    display_top_tabs($tab_array);	

}
?>
</td>
</tr>
				<tr>
				<td class="tabcont">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
				<?php
				/* display error code if there is no id */
				if($id == "") 
				{
				echo "				
				<style type=\"text/css\">
				.noid {
				position:absolute;
				top:10px;
				left:0px;
				width:94%;
				background:#FCE9C0;
				background-position: 15px; 
				border-top:2px solid #DBAC48;
				border-bottom:2px solid #DBAC48;
				padding: 15px 10px 85% 50px;
				}
				</style> 
				<div class=\"alert\" ALIGN=CENTER><img src=\"/themes/nervecenter/images/icons/icon_alert.gif\"/><strong>You can not edit options without an interface ID.</CENTER></div>\n";
				
				}
				?>
				<tr>
				<td width="22%" valign="top" class="vtable">&nbsp;</td>
				<td width="78%" class="vtable">
					<?php
					// <input name="enable" type="checkbox" value="yes" checked onClick="enable_change(false)">
					// care with spaces
					if ($pconfig['barnyard_enable'] == "on")
					$checked = checked;
					if($id != "") 
					{
					$onclick_enable = "onClick=\"enable_change(false)\">";
					}
					echo "
					<input name=\"barnyard_enable\" type=\"checkbox\" value=\"on\" $checked $onclick_enable
					<strong>Enable Barnyard2 on this Interface</strong><br>
					This will enable barnyard2 for this interface. You will also have to set the database credentials.</td>\n\n";
					?>
				</tr>
				<tr>
                  <td width="22%" valign="top" class="vncell">Interface</td>
                  <td width="78%" class="vtable">
					<select name="interface" class="formfld">
						<?php
						$interfaces = array('wan' => 'WAN', 'lan' => 'LAN', 'pptp' => 'PPTP', 'pppoe' => 'PPPOE');
						for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++) {
							$interfaces['opt' . $i] = $config['interfaces']['opt' . $i]['descr'];
						}
						foreach ($interfaces as $iface => $ifacename): ?>
						<option value="<?=$iface;?>" <?php if ($iface == $pconfig['interface']) echo "selected"; ?>>
						<?=htmlspecialchars($ifacename);?>
						</option>
						<?php endforeach; ?>
					</select><br>
                     <span class="vexpl">Choose which interface this rule applies to.<br>
                     Hint: in most cases, you'll want to use WAN here.</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top" class="vncell">Log to a Mysql Database</td>
                  <td width="78%" class="vtable">
                    <input name="barnyard_mysql" type="text" class="formfld" id="barnyard_mysql" size="40" value="<?=htmlspecialchars($pconfig['barnyard_mysql']);?>">
                    <br> <span class="vexpl">Example: output database: log, mysql, dbname=snort user=snort host=localhost password=xyz</span></td>
                </tr>
                <tr>
                  <td width="22%" valign="top">&nbsp;</td>
                  <td width="78%">
                    <input name="Submit" type="submit" class="formbtn" value="Save"> <input name="Submit2" type="submit" class="formbtn" value="Start" onClick="enable_change(true)"> <input type="button" class="formbtn" value="Cancel" onclick="history.back()">
                    <?php if (isset($id) && $a_nat[$id]): ?>
                    <input name="id" type="hidden" value="<?=$id;?>">
                    <?php endif; ?>
                  </td>
                </tr>
	<tr>
	  <td width="22%" valign="top">&nbsp;</td>
	  <td width="78%"><span class="vexpl"><span class="red"><strong>Note:</strong></span>
	  <br>
		Please save your settings befor you click start. </td>
	</tr>
  </table>
  </table>
</form>

<script language="JavaScript">
<!--
enable_change(false);
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
