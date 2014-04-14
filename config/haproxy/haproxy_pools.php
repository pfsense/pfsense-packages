<?php
/* $Id: load_balancer_virtual_server.php,v 1.6.2.1 2006/01/02 23:46:24 sullrich Exp $ */
/*
	haproxy_pools.php
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
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

require_once("guiconfig.inc");
require_once("haproxy.inc");

$d_haproxyconfdirty_path = $g['varrun_path'] . "/haproxy.conf.dirty";

if (!is_array($config['installedpackages']['haproxy']['ha_pools']['item'])) {
	$config['installedpackages']['haproxy']['ha_pools']['item'] = array();
}
if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}

$a_pools = &$config['installedpackages']['haproxy']['ha_pools']['item'];
$a_backends = &$config['installedpackages']['haproxy']['ha_backends']['item'];

if ($_POST) {
	$pconfig = $_POST;

	if ($_POST['apply']) {
		$retval = 0;
		config_lock();
		$retval = haproxy_configure();
		config_unlock();
		$savemsg = get_std_save_message($retval);
		unlink_if_exists($d_haproxyconfdirty_path);
	}
}

if ($_GET['act'] == "del") {
	if (isset($a_pools[$_GET['id']])) {
		unset($a_pools[$_GET['id']]);
		write_config();
		touch($d_haproxyconfdirty_path);
	}
	header("Location: haproxy_pools.php");
	exit;
}

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
	$one_two = true;
	
$pgtitle = "Services: HAProxy: Server pools";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
<form action="haproxy_pools.php" method="post">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_haproxyconfdirty_path)): ?><p>
<?php print_info_box_np("The virtual pool configuration has been changed.<br>You must apply the changes in order for them to take effect.");?><br>
<?php endif; ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
	$tab_array[] = array("Settings", false, "haproxy_global.php");
        $tab_array[] = array("Listener", false, "haproxy_listeners.php");
	$tab_array[] = array("Server Pool", true, "haproxy_pools.php");
	display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
              <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                  <td width="30%" class="listhdrr">Name</td>
                  <td width="10%" class="listhdrr">Servers</td>
                  <td width="40%" class="listhdrr">Listener</td>
                  <td width="10%" class="list"></td>
				</tr>
<?php
		 $i = 0;
		 foreach ($a_pools as $pool):

		 $fe_list = "";
		 $sep = "";
		 foreach ($a_backends as $backend) {
			 if($backend['backend_serverpool'] == $pool['name']) {
				 $fe_list .= $sep . $backend['name'];
				 $sep = ", ";
			 }
		 }
		 $textss = $textse = "";
		 if ($fe_list == "") {
			 $textss = "<span class=\"gray\">";
			 $textse = "</span>";
		 }
		 if (is_array($pool['ha_servers']))
			 $count = count($pool['ha_servers']['item']);
		 else
			 $count = 0;
?>
                <tr>
                  <td class="listlr" ondblclick="document.location='haproxy_pool_edit.php?id=<?=$i;?>';">
			<?=$textss . $pool['name'] . $textse;?>
                  </td>
                  <td class="listlr" ondblclick="document.location='haproxy_pool_edit.php?id=<?=$i;?>';">
			<?=$textss . $count . $textse;?>
                  </td>
                  <td class="listlr" ondblclick="document.location='haproxy_pool_edit.php?id=<?=$i;?>';">
			<?=$textss . $fe_list . $textse;?>
                  </td>
                  <td class="list" nowrap>
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <td valign="middle"><a href="haproxy_pool_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                        <td valign="middle"><a href="haproxy_pools.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this entry?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                        <td valign="middle"><a href="haproxy_pool_edit.php?dup=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
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
                        <td valign="middle"><a href="haproxy_pool_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                    </table>
                  </td>
                </tr>
              </table>
	   </div>
	</table>
            </form>
<?php include("fend.inc"); ?>
</body>
</html>
