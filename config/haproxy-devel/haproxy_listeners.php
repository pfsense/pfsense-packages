<?php
/* $Id: load_balancer_virtual_server.php,v 1.6.2.1 2006/01/02 23:46:24 sullrich Exp $ */
/*
	haproxy_baclkends.php
	part of pfSense (http://www.pfsense.com/)
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
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

require_once("guiconfig.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

require_once("haproxy.inc");
require_once("certs.inc");

if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}
$a_backend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

if ($_POST) {
	$pconfig = $_POST;

	if ($_POST['apply']) {
		$retval = 0;
		config_lock();
		$retval = haproxy_configure();
		config_unlock();

		$result = haproxy_check_writtenconfig_error($messages);
		$savemsg = $messages;
		if ($result)
			unlink_if_exists($d_haproxyconfdirty_path);
	}
} else {
	$result = haproxy_check_config($retval);
	if ($result)
		$savemsg = gettext($result);
}

$id = $_GET['id'];
$id = get_frontend_id($id);
	
if ($_GET['act'] == "del") {
	if (isset($a_backend[$id])) {
		if (!$input_errors) {
			unset($a_backend[$id]);
			write_config();
			touch($d_haproxyconfdirty_path);
		}
		header("Location: haproxy_listeners.php");
		exit;
	}
}

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
	$one_two = true;
	
$pgtitle = "Services: HAProxy: Listener";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="haproxy_listeners.php" method="post">
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_haproxyconfdirty_path)): ?><p>
<?php print_info_box_np("The haproxy configuration has been changed.<br>You must apply the changes in order for them to take effect.");?><br>
<?php endif; ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
	$tab_array[] = array("Settings", false, "haproxy_global.php");
        $tab_array[] = array("Listener", true, "haproxy_listeners.php");		
	$tab_array[] = array("Server Pool", false, "haproxy_pools.php");
	display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
	  <table class="tabcont sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
		  <td width="5%" class="listhdrr">Primary</td>
		  <td width="20%" class="listhdrr">Advanced</td>
		  <td width="20%" class="listhdrr">Name</td>
		  <td width="30%" class="listhdrr">Description</td>
		  <td width="20%" class="listhdrr">Address</td>
		  <td width="5%" class="listhdrr">Type</td>
		  <td width="10%" class="listhdrr">Server&nbsp;pool</td>
		  <td width="20%" class="listhdrr">Parent</td>
		  <td width="5%" class="list"></td>
		</tr>
<?php
		
		function sort_backends(&$a, &$b) {
			if ($a['ipport'] != $b['ipport'])
				return $a['ipport'] > $b['ipport'] ? 1 : -1;
			if ($a['secondary'] != $b['secondary'])
				return $a['secondary'] > $b['secondary'] ? 1 : -1;
			if ($a['name'] != $b['name'])
				return $a['name'] > $b['name'] ? 1 : -1;
			return 0;
		}
		foreach($a_backend as &$backend2) {
			$backend2['ipport'] = get_frontend_ipport($backend2);
		}
		usort($a_backend,'sort_backends');
		
		$certimg = "/themes/{$g['theme']}/images/icons/icon_frmfld_cert.png";
		unset($ipport_previous);
		foreach ($a_backend as $backend):
			$backendname = $backend['name'];
			$textgray = $backend['status'] != 'active' ? " gray" : "";
			if (isset($ipport_previous ) && $backend['ipport'] != $ipport_previous):
			?>
				<tr class="<?=$textgray?>"><td collspan="7">&nbsp;</td></tr>
			<?
			endif;
			$ipport_previous = $backend['ipport'];
			?>
			<tr class="<?=$textgray?>">
			  <td class="listlr" style="<?=$backend['secondary']=='yes'?"visibility:hidden;":""?>" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['secondary']!='yes'?"yes":"no";?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<? if (strtolower($backend['type']) == "http" && $backend['ssloffload']):
					$cert = lookup_cert($backend['ssloffloadcert']);?>
					<img src="<?=$certimg;?>" alt="SSL offloading" title="SSL offloading cert: '<?=$cert['descr'];?>'" border="0" height="16" width="16" />
				<? endif;?>
				<?
				$acls = get_frontent_acls($backend);
				$isadvset = "";
				foreach ($acls as $acl) {
					$isadvset .= "&#10;" . $acl['descr'];
				}
				if ($isadvset) 
					echo "<img src=\"./themes/{$g['theme']}/images/icons/icon_advanced.gif\" title=\"" . gettext("advanced settings set") . ": {$isadvset}\" border=\"0\">";
				?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['name'];?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['desc'];?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['ipport'];?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['type']?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['backend_serverpool']?>
			  </td>
			  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$backendname;?>';">
				<?=$backend['secondary'] == 'yes' ? $backend['primary_frontend'] : "";?>
			  </td>
			  <td class="list" nowrap>
				<table border="0" cellspacing="0" cellpadding="1">
				  <tr>
					<td valign="middle"><a href="haproxy_listeners_edit.php?id=<?=$backendname;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
					<td valign="middle"><a href="haproxy_listeners.php?act=del&id=<?=$backendname;?>" onclick="return confirm('Do you really want to delete this entry?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
					<td valign="middle"><a href="haproxy_listeners_edit.php?dup=<?=$backendname;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
				  </tr>
				</table>
			  </td>
			</tr>
			<?php endforeach; ?>
			<tfoot>
			<tr>
			  <td class="list" colspan="8"></td>
			  <td class="list">
				<table border="0" cellspacing="0" cellpadding="1">
				  <tr>
					<td valign="middle"><a href="haproxy_listeners_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
				  </tr>
				</table>
			  </td>
			</tr>
			</tfoot>
		  </table>
	   </div>
	</table>
	</form>
<?php include("fend.inc"); ?>
</body>
</html>
