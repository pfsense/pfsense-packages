<?php
/* $Id$ */
/*
	firewall_aliases.php
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
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort.inc");


if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
	$config['installedpackages']['snortglobal']['whitelist']['item'] = array();

//aliases_sort(); << what ?
$a_whitelist = &$config['installedpackages']['snortglobal']['whitelist']['item'];

if (isset($config['installedpackages']['snortglobal']['whitelist']['item'])) {
$id_gen = count($config['installedpackages']['snortglobal']['whitelist']['item']);
}else{
$id_gen = '0';
}

$d_whitelistdirty_path = '/var/run/snort_whitelist.dirty';

if ($_POST) {

	$pconfig = $_POST;

	if ($_POST['apply']) {
		$retval = 0;

		if(stristr($retval, "error") <> true)
		    $savemsg = get_std_save_message($retval);
		else
		    $savemsg = $retval;
		if ($retval == 0) {
			if (file_exists($d_whitelistdirty_path))
				unlink($d_whitelistdirty_path);
		}
	}
}

if ($_GET['act'] == "del") {
	if ($a_whitelist[$_GET['id']]) {
		/* make sure rule is not being referenced by any nat or filter rules */

			unset($a_whitelist[$_GET['id']]);
			write_config();
			filter_configure();
			touch($d_whitelistdirty_path);
			header("Location: /snort/snort_interfaces_whitelist.php");
			exit;
	}
}

$pgtitle = "Services: Snort: Whitelist";
include("/usr/local/pkg/snort/snort_head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

		<script>
			jQuery(document).ready(function(){
			
				//Examples of how to assign the ColorBox event to elements
				jQuery(".example8").colorbox({width:"820px", height:"700px", iframe:true, overlayClose:false});
				
			});
		</script>

<?php 
include("fbegin.inc");
echo $snort_general_css;
?>

<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<div class="body2">

<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<form action="/snort/snort_interfaces_whitelist.php" method="post">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_whitelistdirty_path)): ?><p>
<?php print_info_box_np("The white list has been changed.<br>You must apply the changes in order for them to take effect.");?>
<?php endif; ?>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
  <td class="tabnavtbl">

<div class="snorttabs" style="margin:1px 0px; width:775px;">
<!-- Tabbed bar code-->
<ul class="snorttabs">
    <li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
	<li><a href="/snort/snort_interfaces_global.php"><span>Global Settings</span></a></li>
    <li><a href="/snort/snort_download_updates.php"><span>Updates</span></a></li>
    <li><a href="/snort/snort_alerts.php"><span>Alerts</span></a></li>
	<li><a href="/snort/snort_blocked.php"><span>Blocked</span></a></li>
    <li class="snorttabs_active"><a href="/snort/snort_interfaces_whitelist.php"><span>Whitelists</span></a></li>
    <li><a href="/snort/snort_interfaces_suppress.php"><span>Suppress</span></a></li>
	<li><a class="example8" href="/snort/help_and_info.php"><span>Help</span></a></li>
  </ul>
</div> 
  
</td>
</tr>
<tr>
<td class="tabcont">

<table width="100%" border="0" cellpadding="0" cellspacing="0">

<tr>
  <td width="20%" class="listhdrr">File Name</td>
  <td width="40%" class="listhdrr">Values</td>
  <td width="40%" class="listhdr">Description</td>
  <td width="10%" class="list">
  </td>
</tr>
	  <?php $i = 0; foreach ($a_whitelist as $list): ?>
<tr>
  <td class="listlr" ondblclick="document.location='snort_interfaces_whitelist_edit.php?id=<?=$i;?>';">
    <?=htmlspecialchars($list['name']);?>
  </td>
  <td class="listr" ondblclick="document.location='snort_interfaces_whitelist_edit.php?id=<?=$i;?>';">
      <?php
	$addresses = implode(", ", array_slice(explode(" ", $list['address']), 0, 10));
	echo $addresses;
	if(count($addresses) < 10) {
		echo " ";
	} else {
		echo "...";
	}
    ?>
  </td>
  <td class="listbg" ondblclick="document.location='snort_interfaces_whitelist_edit.php?id=<?=$i;?>';">
    <font color="#FFFFFF">
    <?=htmlspecialchars($list['descr']);?>&nbsp;
  </td>
  <td valign="middle" nowrap class="list">
    <table border="0" cellspacing="0" cellpadding="1">
      <tr>
        <td valign="middle"><a href="snort_interfaces_whitelist_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="edit whitelist"></a></td>
        <td><a href="/snort/snort_interfaces_whitelist.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this whitelist? All elements that still use it will become invalid (e.g. snort rules will fall back to the default whitelist)!')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" title="delete whitelist"></a></td>
      </tr>
    </table>
  </td>
</tr>
	  <?php $i++; endforeach; ?>
<tr>
  <td class="list" colspan="3"></td>
  <td class="list">
    <table border="0" cellspacing="0" cellpadding="1">
      <tr>
	<td valign="middle" width="17">&nbsp;</td>
        <td valign="middle"><a href="snort_interfaces_whitelist_edit.php?id=<?php echo $id_gen;?> "><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="add a new list"></a></td>
      </tr>
    </table>
  </td>
</tr>
</table>
  </td>
  </tr>
  </table>
<br>
<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
<td width="100%"><span class="vexpl"><span class="red"><strong>Note:</strong></span>
   <p><span class="vexpl">Here you can create whitelist files for your snort package rules.<br>Please add all the ips or networks you want to protect against snort block decisions.<br>Remember that the default whitelist only includes local networks.<br>Be careful, it is very easy to get locked out of you system.</span></p>
</td>
</table>
</form>

</div>

<?php include("fend.inc"); ?>
</body>
</html>
