<?php
/*
 * Copyright (C) 2004 Scott Ullrich
 * Copyright (C) 2011-2012 Ermal Luci
 * All rights reserved.
 *
 * originially part of m0n0wall (http://m0n0.ch/wall)
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * All rights reserved.
 *
 * modified for the pfsense snort package
 * Copyright (C) 2009-2010 Robert Zelaya.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort.inc");

if (!is_array($config['installedpackages']['snortglobal']['suppress']))
	$config['installedpackages']['snortglobal']['suppress'] = array();
if (!is_array($config['installedpackages']['snortglobal']['suppress']['item']))
	$config['installedpackages']['snortglobal']['suppress']['item'] = array();
$a_suppress = &$config['installedpackages']['snortglobal']['suppress']['item'];
$id_gen = count($config['installedpackages']['snortglobal']['suppress']['item']);

if ($_GET['act'] == "del") {
	if ($a_suppress[$_GET['id']]) {
		/* make sure rule is not being referenced by any nat or filter rules */

		unset($a_suppress[$_GET['id']]);
		write_config();
		header("Location: /snort/snort_interfaces_suppress.php");
		exit;
	}
}

$pgtitle = "Services: Snort: Suppression";
include_once("head.inc");

?>

<body link="#000000" vlink="#000000" alink="#000000">

<?php
include_once("fbegin.inc");
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
?>

<form action="/snort/snort_interfaces_suppress.php" method="post"><?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr><td>
<?php
        $tab_array = array();
        $tab_array[0] = array(gettext("Snort Interfaces"), false, "/snort/snort_interfaces.php");
        $tab_array[1] = array(gettext("Global Settings"), false, "/snort/snort_interfaces_global.php");
        $tab_array[2] = array(gettext("Updates"), false, "/snort/snort_download_updates.php");
        $tab_array[3] = array(gettext("Alerts"), false, "/snort/snort_alerts.php");
        $tab_array[4] = array(gettext("Blocked"), false, "/snort/snort_blocked.php");
        $tab_array[5] = array(gettext("Whitelists"), false, "/snort/snort_interfaces_whitelist.php");
        $tab_array[6] = array(gettext("Suppress"), true, "/snort/snort_interfaces_suppress.php");
        $tab_array[7] = array(gettext("Sync"), false, "/pkg_edit.php?xml=snort/snort_sync.xml");
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
		ondblclick="document.location='snort_interfaces_suppress_edit.php?id=<?=$i;?>';">
		<?=htmlspecialchars($list['name']);?></td>
	<td class="listbg"
		ondblclick="document.location='snort_interfaces_suppress_edit.php?id=<?=$i;?>';">
	<font color="#FFFFFF"> <?=htmlspecialchars($list['descr']);?>&nbsp;
	</td>

	<td valign="middle" nowrap class="list">
	<table border="0" cellspacing="0" cellpadding="1">
		<tr>
			<td valign="middle"><a
				href="snort_interfaces_suppress_edit.php?id=<?=$i;?>"><img
				src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"
				width="17" height="17" border="0" title="<?php echo gettext("edit whitelist"); ?>"></a></td>
			<td><a
				href="/snort/snort_interfaces_suppress.php?act=del&id=<?=$i;?>"
				onclick="return confirm('<?php echo gettext("Do you really want to delete this whitelist? All elements that still use it will become invalid (e.g. snort rules will fall back to the default whitelist)!"); ?>')"><img
				src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif"
				width="17" height="17" border="0" title="<?php echo gettext("delete whitelist"); ?>"></a></td>
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
				href="snort_interfaces_suppress_edit.php?id=<?php echo $id_gen;?> "><img
				src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif"
				width="17" height="17" border="0" title="<?php echo gettext("add a new list"); ?>"></a></td>
		</tr>
	</table>
	</div>
	</td>
</tr>
</table>
</td></tr>
<tr>
	<td colspan="3" width="100%"><br/><span class="vexpl"><span class="red"><strong><?php echo gettext("Note:"); ?></strong></span>
	<p><span class="vexpl"><?php echo gettext("Here you can create event filtering and " .
	"suppression for your snort package rules."); ?><br/><br/>
	<?php echo gettext("Please note that you must restart a running Interface so that changes can " .
	"take effect."); ?></span></p></td>
</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
