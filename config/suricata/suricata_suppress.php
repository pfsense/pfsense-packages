<?php
/*
	suricata_suppress.php

	Copyright (C) 2014 Bill Meeks
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
require_once("/usr/local/pkg/suricata/suricata.inc");

if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();
if (!is_array($config['installedpackages']['suricata']['suppress']))
	$config['installedpackages']['suricata']['suppress'] = array();
if (!is_array($config['installedpackages']['suricata']['suppress']['item']))
	$config['installedpackages']['suricata']['suppress']['item'] = array();
$a_suppress = &$config['installedpackages']['suricata']['suppress']['item'];
$id_gen = count($config['installedpackages']['suricata']['suppress']['item']);

function suricata_suppresslist_used($supplist) {

	/****************************************************************/
	/* This function tests if the passed Suppress List is currently */
	/* assigned to an interface.  It returns TRUE if the list is    */
	/* in use.                                                      */
	/*                                                              */
	/* Returns:  TRUE if list is in use, else FALSE                 */
	/****************************************************************/

	global $config;

	$suricataconf = $config['installedpackages']['suricata']['rule'];
	if (empty($suricataconf))
		return false;
	foreach ($suricataconf as $value) {
		if ($value['suppresslistname'] == $supplist)
			return true;
	}
	return false;
}

if ($_GET['act'] == "del") {
	if ($a_suppress[$_GET['id']]) {
		// make sure list is not being referenced by any Suricata-configured interface
		if (suricata_suppresslist_used($a_suppress[$_GET['id']]['name'])) {
			$input_errors[] = gettext("ERROR -- Suppress List is currently assigned to an interface and cannot be removed!");
		}
		else {
			unset($a_suppress[$_GET['id']]);
			write_config();
			header("Location: /suricata/suricata_suppress.php");
			exit;
		}
	}
}

$pgtitle = gettext("Suricata: Suppression Lists");
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include_once("fbegin.inc");
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
if ($input_errors) {
	print_input_errors($input_errors);
}

?>

<form action="/suricata/suricata_suppress.php" method="post"><?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
	$tab_array[] = array(gettext("Suricata Interfaces"), false, "/suricata/suricata_interfaces.php");
	$tab_array[] = array(gettext("Global Settings"), false, "/suricata/suricata_global.php");
	$tab_array[] = array(gettext("Update Rules"), false, "/suricata/suricata_download_updates.php");
	$tab_array[] = array(gettext("Alerts"), false, "/suricata/suricata_alerts.php?instance={$instanceid}");
	$tab_array[] = array(gettext("Suppress"), true, "/suricata/suricata_suppress.php");
	$tab_array[] = array(gettext("Logs Browser"), false, "/suricata/suricata_logs_browser.php");
	display_top_tabs($tab_array);
?>
</td>
</tr>
<tr><td><div id="mainarea">
<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td width="30%" class="listhdrr"><?php echo gettext("File Name"); ?></td>
	<td width="60%" class="listhdr"><?php echo gettext("Description"); ?></td>
	<td width="10%" class="list"></td>
</tr>
<?php $i = 0; foreach ($a_suppress as $list): ?>
<tr>
	<td class="listlr"
		ondblclick="document.location='suricata_suppress_edit.php?id=<?=$i;?>';">
		<?=htmlspecialchars($list['name']);?></td>
	<td class="listbg"
		ondblclick="document.location='suricata_suppress_edit.php?id=<?=$i;?>';">
	<font color="#FFFFFF"> <?=htmlspecialchars($list['descr']);?>&nbsp;</font>
	</td>

	<td valign="middle" nowrap class="list">
	<table border="0" cellspacing="0" cellpadding="1">
		<tr>
			<td valign="middle"><a
				href="suricata_suppress_edit.php?id=<?=$i;?>"><img
				src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"
				width="17" height="17" border="0" title="<?php echo gettext("edit Suppress List"); ?>"></a></td>
			<td><a
				href="/suricata/suricata_suppress.php?act=del&id=<?=$i;?>"
				onclick="return confirm('<?php echo gettext("Do you really want to delete this Suppress List?"); ?>')"><img
				src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif"
				width="17" height="17" border="0" title="<?php echo gettext("delete Suppress List"); ?>"></a></td>
		</tr>
	</table>
	</td>
</tr>
<?php $i++; endforeach; ?>
<tr>
	<td class="list" colspan="2"></td>
	<td  class="list">
	<table border="0" cellspacing="0" cellpadding="1">
		<tr>
			<td valign="middle" width="17">&nbsp;</td>
			<td valign="middle"><a
				href="suricata_suppress_edit.php?id=<?php echo $id_gen;?> "><img
				src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif"
				width="17" height="17" border="0" title="<?php echo gettext("add a new list"); ?>"></a></td>
		</tr>
	</table>
	</td>
</tr>
</table>
</div>
</td></tr>
<tr>
	<td colspan="3" width="100%"><br/><span class="vexpl"><span class="red"><strong><?php echo gettext("Note:"); ?></strong></span>
	<p><?php echo gettext("Here you can create event filtering and " .
	"suppression for your Suricata package rules."); ?><br/><br/>
	<?php echo gettext("Please note that you must restart a running Interface so that changes can " .
	"take effect."); ?></p></span></td>
</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
