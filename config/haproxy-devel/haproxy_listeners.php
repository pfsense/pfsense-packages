<?php
/* $Id: load_balancer_virtual_server.php,v 1.6.2.1 2006/01/02 23:46:24 sullrich Exp $ */
/*
	haproxy_listeners.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2013 PiBa-NL
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
$shortcut_section = "haproxy";
require_once("guiconfig.inc");
require_once("haproxy.inc");
require_once("certs.inc");
require_once("haproxy_utils.inc");
require_once("pkg_haproxy_tabs.inc");

if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}
$a_frontend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

if ($_POST) {
	$pconfig = $_POST;

	if ($_POST['apply']) {
		$result = haproxy_check_and_run($savemsg, true);
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
	if (isset($a_frontend[$id])) {
		if (!$input_errors) {
			unset($a_frontend[$id]);
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
	
$pgtitle = "Services: HAProxy: Frontends";
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
<?php if (file_exists($d_haproxyconfdirty_path)): ?>
<?php print_info_box_np("The haproxy configuration has been changed.<br/>You must apply the changes in order for them to take effect.");?><br/>
<?php endif; ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
	haproxy_display_top_tabs_active($haproxy_tab_array['haproxy'], "frontend");
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
		  <td width="10%" class="listhdrr">Backend</td>
		  <td width="20%" class="listhdrr">Parent</td>
		  <td width="5%" class="list"></td>
		</tr>
<?php
		
		function sort_sharedfrontends(&$a, &$b) {
			// make sure the 'primary frontend' is the first in the array, after that sort by name.
			if ($a['secondary'] != $b['secondary'])
				return $a['secondary'] > $b['secondary'] ? 1 : -1;
			if ($a['name'] != $b['name'])
				return $a['name'] > $b['name'] ? 1 : -1;
			return 0;
		}
		
		$a_frontend_grouped = array();
		foreach($a_frontend as &$frontend2) {
			$ipport = get_frontend_ipport($frontend2, true);
			$frontend2['ipport'] = $ipport;
			$a_frontend_grouped[$ipport][] = $frontend2;
		}
		ksort($a_frontend_grouped);
		
		$img_cert = "/themes/{$g['theme']}/images/icons/icon_frmfld_cert.png";
		$img_adv = "/themes/{$g['theme']}/images/icons/icon_advanced.gif";
		$img_acl = "/themes/{$g['theme']}/images/icons/icon_ts_rule.gif";
		$last_frontend_shared = false;
		foreach ($a_frontend_grouped as $a_frontend) {
			usort($a_frontend,'sort_sharedfrontends');
			if (count($a_frontend) > 1 || $last_frontend_shared) {
				?> <tr class="<?=$textgray?>"><td colspan="7">&nbsp;</td></tr> <?	
			}
			$last_frontend_shared = count($a_frontend) > 1;
			foreach ($a_frontend as $frontend) {
				$frontendname = $frontend['name'];
				$textgray = $frontend['status'] != 'active' ? " gray" : "";
				?>
				<tr class="<?=$textgray?>">
				  <td class="listlr" style="<?=$frontend['secondary']=='yes'?"visibility:hidden;":""?>" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<?=$frontend['secondary']!='yes'?"yes":"no";?>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<? 
					if (strtolower($frontend['type']) == "http" && $frontend['ssloffload']) {
						$cert = lookup_cert($frontend['ssloffloadcert']);
						$descr = htmlspecialchars($cert['descr']);
						$certs = $frontend['ha_certificates']['item'];
						if (is_array($certs)){
							if (count($certs) > 0){
								foreach($certs as $certitem){
									$cert = lookup_cert($certitem['ssl_certificate']);
									$descr .= "\n".htmlspecialchars($cert['descr']);
								}
							}
						}
						echo '<img src="'.$img_cert.'" title="SSL offloading cert: '.$descr.'" alt="SSL offloading" border="0" height="16" width="16" />';
					}
					
					$acls = get_frontend_acls($frontend);
					$isaclset = "";
					foreach ($acls as $acl) {
						$isaclset .= "&#10;" . htmlspecialchars($acl['descr']);
					}
					
					if ($isaclset) 
						echo "<img src=\"$img_acl\" title=\"" . gettext("acl's used") . ": {$isaclset}\" border=\"0\" />";
						
					$isadvset = "";
					if ($frontend['advanced_bind']) $isadvset .= "Advanced bind: ".htmlspecialchars($frontend['advanced_bind'])."\r\n";
					if ($frontend['advanced']) $isadvset .= "Advanced pass thru setting used\r\n";
					if ($isadvset)
						echo "<img src=\"$img_adv\" title=\"" . gettext("Advanced settings set") . ": {$isadvset}\" border=\"0\" />";
					
					$backend_serverpool = $frontend['backend_serverpool'];
					$backend = get_backend($backend_serverpool );
					$servers = $backend['ha_servers']['item'];
					$backend_serverpool_hint = gettext("Servers in pool:");
					if (is_array($servers)){
						foreach($servers as $server){
							$backend_serverpool_hint .= "\n".$server['address'].":".$server['port'];
						}
					}
					?>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<?=$frontend['name'];?>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<?=$frontend['desc'];?>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<?=str_replace(" ","&nbsp;",$frontend['ipport']);?>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<?=$frontend['type']?>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<div title='<?=$backend_serverpool_hint;?>'>
					<?=$frontend['backend_serverpool']?>
					</div>
				  </td>
				  <td class="listlr" ondblclick="document.location='haproxy_listeners_edit.php?id=<?=$frontendname;?>';">
					<?=$frontend['secondary'] == 'yes' ? $frontend['primary_frontend'] : "";?>
				  </td>
				  <td class="list" nowrap>
					<table border="0" cellspacing="0" cellpadding="1">
					  <tr>
						<td valign="middle"><a href="haproxy_listeners_edit.php?id=<?=$frontendname;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"  title="<?=gettext("edit frontend");?>" width="17" height="17" border="0" /></a></td>
						<td valign="middle"><a href="haproxy_listeners.php?act=del&amp;id=<?=$frontendname;?>" onclick="return confirm('Do you really want to delete this entry?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="<?=gettext("delete frontend");?>"  width="17" height="17" border="0" /></a></td>
						<td valign="middle"><a href="haproxy_listeners_edit.php?dup=<?=$frontendname;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("clone frontend");?>" width="17" height="17" border="0" /></a></td>
					  </tr>
					</table>
				  </td>
				</tr><?php
			}
		} ?>
			<tfoot>
			<tr>
			  <td class="list" colspan="8"></td>
			  <td class="list">
				<table border="0" cellspacing="0" cellpadding="1">
				  <tr>
					<td valign="middle"><a href="haproxy_listeners_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("add new frontend");?>"  width="17" height="17" border="0" /></a></td>
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
