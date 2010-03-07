<?php
/* $Id$ */
/*

	originally part of m0n0wall (http://m0n0.ch/wall)
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

require("guiconfig.inc");
require("/usr/local/pkg/snort/snort_gui.inc");
include_once("/usr/local/pkg/snort/snort.inc");

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

if (!is_array($config['installedpackages']['snortglobal']['rule']))
	$config['installedpackages']['snortglobal']['rule'] = array();

$a_nat = &$config['installedpackages']['snortglobal']['rule'];

/* if a custom message has been passed along, lets process it */
if ($_GET['savemsg'])
	$savemsg = $_GET['savemsg'];

if ($_POST) {

	$pconfig = $_POST;

	if ($_POST['apply']) {

		write_config();

		$retval = 0;

		if(stristr($retval, "error") <> true)
		    $savemsg = get_std_save_message($retval);
		else
		    $savemsg = $retval;

		unlink_if_exists("/tmp/config.cache");
		$retval |= filter_configure();

		if ($retval == 0) {
			if (file_exists($d_natconfdirty_path))
				unlink($d_natconfdirty_path);
			if (file_exists($d_filterconfdirty_path))
				unlink($d_filterconfdirty_path);
		}

	}
}

if (isset($_POST['del_x'])) {
    /* delete selected rules */
    if (is_array($_POST['rule']) && count($_POST['rule'])) {
	    foreach ($_POST['rule'] as $rulei) {


		    /* dont flood the syslog code */
		    exec("/bin/cp /var/log/system.log /var/log/system.log.bk");
		    exec("/bin/sh /usr/local/etc/rc.d/snort.sh stop $rulei");

		    /* stop syslog flood code */		
		    $if_real_wan_rulei = $a_nat[$rulei]['interface'];
		    $if_real_wan_rulei2 = convert_friendly_interface_to_real_interface_name2($if_real_wan_rulei);
		    exec("/bin/cp /var/log/system.log /var/log/snort/snort_sys_$rulei$if_real.log");
		    exec("/usr/bin/killall syslogd");
		    exec("/usr/sbin/clog -i -s 262144 /var/log/system.log");
		    exec("/usr/sbin/syslogd -c -ss -f /var/etc/syslog.conf");
		    sleep(2);
		    exec("/bin/cp /var/log/system.log.bk /var/log/system.log");
		    $after_mem = exec("/usr/bin/top | /usr/bin/grep Wired | /usr/bin/awk '{ print $2 }'");
		    exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'MEM after {$rulei}{$if_real} STOP {$after_mem}'");				
		    exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'Interface Rule removed for {$rulei}{$if_real}...'");

		    unset($a_nat[$rulei]);

	    }



	    conf_mount_rw();
	    exec("/bin/rm -r /usr/local/etc/snort/snort_$rulei$if_real");
	    exec("/bin/rm /var/log/snort/snort.u2_$rulei$if_real*");
	    conf_mount_ro();
		
	    write_config();
	    // touch($d_natconfdirty_path);
	    header("Location: /snort/snort_interfaces.php");
	    exit;
	}

} else {

        /* yuck - IE won't send value attributes for image buttons, while Mozilla does - so we use .x/.y to find move button clicks instead... */
        unset($movebtn);
        foreach ($_POST as $pn => $pd) {
                if (preg_match("/move_(\d+)_x/", $pn, $matches)) {
                        $movebtn = $matches[1];
                        break;
                }
        }
        /* move selected rules before this rule */
        if (isset($movebtn) && is_array($_POST['rule']) && count($_POST['rule'])) {
                $a_nat_new = array();

                /* copy all rules < $movebtn and not selected */
                for ($i = 0; $i < $movebtn; $i++) {
                        if (!in_array($i, $_POST['rule']))
                                $a_nat_new[] = $a_nat[$i];
                }

                /* copy all selected rules */
                for ($i = 0; $i < count($a_nat); $i++) {
                        if ($i == $movebtn)
                                continue;
                        if (in_array($i, $_POST['rule']))
                                $a_nat_new[] = $a_nat[$i];
                }

                /* copy $movebtn rule */
                if ($movebtn < count($a_nat))
                        $a_nat_new[] = $a_nat[$movebtn];

                /* copy all rules > $movebtn and not selected */
                for ($i = $movebtn+1; $i < count($a_nat); $i++) {
                        if (!in_array($i, $_POST['rule']))
                                $a_nat_new[] = $a_nat[$i];
                }
                $a_nat = $a_nat_new;
                write_config();
                touch($d_natconfdirty_path);
                header("Location: snort_interfaces.php");
				
                exit;
        }
}


/* start/stop snort */
if ($_GET['act'] == "toggle" && $_GET['id'] != "")
{

	$if_real2 = convert_friendly_interface_to_real_interface_name($a_nat[$id]['interface']);
	$name = "{$id}{$if_real2}";
	$snort_pid = exec("pgrep -F /var/run/snort_{$if_real2}{$name}.pid snort");
	
	if ($snort_pid != "") {
		exec("/bin/sh /usr/local/etc/rc.d/snort.sh stop $name");
	}else{
		sync_snort_package_all();
		exec("/bin/sh /usr/local/etc/rc.d/snort.sh start $name");
	}
	header("Location: snort_interfaces.php");
}	

$pgtitle = "Services: Snort 2.8.5.2 pkg v. 1.9";
include("head.inc");

?>
<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>
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
padding: 15px 10px 50% 50px;
}
.listbg2 {
	border-right: 1px solid #999999;
	border-bottom: 1px solid #999999;
	font-size: 11px;
	background-color: #090;
	color: #000;	
	padding-right: 16px;
	padding-left: 6px;
	padding-top: 4px;
	padding-bottom: 4px;
}
.listbg3 {
	border-right: 1px solid #999999;
	border-bottom: 1px solid #999999;
	font-size: 11px;
	background-color: #777777;
	color: #000;	
	padding-right: 16px;
	padding-left: 6px;
	padding-top: 4px;
	padding-bottom: 4px;
}
</style> 
<noscript><div class="alert" ALIGN=CENTER><img src="../themes/nervecenter/images/icons/icon_alert.gif"/><strong>Please enable JavaScript to view this content</CENTER></div></noscript>

<form action="snort_interfaces.php" method="post" name="iform">
<script type="text/javascript" language="javascript" src="row_toggle.js">
</script>
<?php if (file_exists($d_natconfdirty_path)): ?><p>
<?php
	if($savemsg)
		print_info_box_np2("{$savemsg}<br>The Snort configuration has been changed.<br>You must apply the changes in order for them to take effect.");
	else
		print_info_box_np2("The Snort configuration has been changed.<br>You must apply the changes in order for them to take effect.");
?>
<?php endif; ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("Snort Interfaces", true, "/snort/snort_interfaces.php");
	$tab_array[] = array("Global Settings", false, "/snort/snort_interfaces_global.php");
	$tab_array[] = array("Rule Updates", false, "/snort/snort_download_rules.php");
	$tab_array[] = array("Alerts", false, "/snort/snort_alerts.php");
    $tab_array[] = array("Blocked", false, "/snort/snort_blocked.php");
	$tab_array[] = array("Whitelists", false, "/pkg.php?xml=/snort_whitelist.xml");
	$tab_array[] = array("Help & Info", false, "/snort/snort_help_info.php");
	display_top_tabs($tab_array);
?>
 </td></tr>
	<br>
	<tr>
	<td>
	<div id="mainarea">
              <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr id="frheader">
				<td width="4%" class="list">&nbsp;</td>
		          <td width="1%" class="list">&nbsp;</td>
                  <td width="10%" class="listhdrr">If</td>
				  <td width="10%" class="listhdrr">Snort</td>
				  <td width="10%" class="listhdrr">Performance</td>
                  <td width="10%" class="listhdrr">Block</td>
				  <td width="10%" class="listhdrr">Barnyard2</td>
                  <td width="50%" class="listhdr">Description</td>
                  <td width="3%" class="list">
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
			<td width="17"></td>
					<td><a href="snort_interfaces_edit.php"><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
					</tr>
                    </table>
		  </td>
		</tr>
	<?php $nnats = $i = 0; foreach ($a_nat as $natent): ?>
                <tr valign="top" id="fr<?=$nnats;?>">
					<?php	
					/* convert fake interfaces to real and check if iface is up */
					$if_real = convert_friendly_interface_to_real_interface_name($natent['interface']);

					$snort_pid = exec("pgrep -F /var/run/snort_{$if_real}{$nnats}{$if_real}.pid snort");
					if ($snort_pid) {
						$class_color_up = "listbg2";
						$iconfn = "block";
					}else{
						$class_color_up = "listbg";
						$iconfn = "pass";
					}
					
					
					?>
                  <td class="listt"><a href="?act=toggle&id=<?=$i;?>"><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_<?=$iconfn;?>.gif" width="13" height="13" border="0" title="click to toggle start/stop snort"></a><input type="checkbox" id="frc<?=$nnats;?>" name="rule[]" value="<?=$i;?>" onClick="fr_bgcolor('<?=$nnats;?>')" style="margin: 0; padding: 0; width: 7px; height: 7px;"></td>
                  <td class="listt" align="center"></td>
                  <td class="<?=$class_color_up;?>" onClick="fr_toggle(<?=$nnats;?>)" id="frd<?=$nnats;?>" ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
		    <?php
			if (!$natent['interface'] || ($natent['interface'] == "wan"))
				echo "WAN";
			else if(strtolower($natent['interface']) == "lan")
				echo "LAN";
			else if(strtolower($natent['interface']) == "pppoe")
				echo "PPPoE";
			else if(strtolower($natent['interface']) == "pptp")
				echo "PPTP";
			else
				echo strtoupper($natent['interface']);
		    ?>
                  </td>
                  <td class="listr" onClick="fr_toggle(<?=$nnats;?>)" id="frd<?=$nnats;?>" ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
				  <?php
				  $check_snort_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['enable'];
				  if ($check_snort_info == "on")
					{
					$check_snort = enabled;
					} else {
					$check_snort = disabled;
					}
				  ?>
                    <?=strtoupper($check_snort);?>
                  </td>
                  <td class="listr" onClick="fr_toggle(<?=$nnats;?>)" id="frd<?=$nnats;?>" ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
				  <?php
				  $check_performance_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['performance'];
					if ($check_performance_info != "") {
						$check_performance = $check_performance_info;
					}else{
						$check_performance = "lowmem";
					}
				  ?>
                    <?=strtoupper($check_performance);?>
                  </td>
                  <td class="listr" onClick="fr_toggle(<?=$nnats;?>)" id="frd<?=$nnats;?>" ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
				  <?php
				  $check_blockoffenders_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['blockoffenders7'];
				  if ($check_blockoffenders_info == "on")
					{
					$check_blockoffenders = enabled;
					} else {
					$check_blockoffenders = disabled;
					}
				  ?>
                    <?=strtoupper($check_blockoffenders);?>
                  </td>
				  <?php
				 				  
					$byard_pid = exec("pgrep -F /var/run/barnyard2_{$nnats}{$if_real}.pid barnyard2");
					if ($byard_pid) {
						$class_color_upb = "listbg2";
					}else{
						$class_color_upb = "listbg";
					}
				   
				  ?>
				  <td class="<?=$class_color_upb;?>" onClick="fr_toggle(<?=$nnats;?>)" id="frd<?=$nnats;?>" ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
				  <?php	
				  $check_snortbarnyardlog_info = $config['installedpackages']['snortglobal']['rule'][$nnats]['barnyard_enable'];
				  if ($check_snortbarnyardlog_info == "on")
					{
						$check_snortbarnyardlog = strtoupper(enabled);
					}else{
						$check_snortbarnyardlog = strtoupper(disabled);
					}
				  ?>
                    <?php echo "$check_snortbarnyardlog";?>
                  </td>
                  <td class="listbg3" onClick="fr_toggle(<?=$nnats;?>)" ondblclick="document.location='snort_interfaces_edit.php?id=<?=$nnats;?>';">
		  <font color="#ffffff">
                    <?=htmlspecialchars($natent['descr']);?>&nbsp;
                  </td>
                  <td valign="middle" class="list" nowrap>
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <td><a href="snort_interfaces_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="edit rule"></a></td>
                      </tr>
                    </table>
		</tr>
  	     <?php $i++; $nnats++; endforeach; ?>
                <tr>
                  <td class="list" colspan="8"></td>
                  <td class="list" valign="middle" nowrap>
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <td><?php if ($nnats == 0): ?><img src="../themes/<?= $g['theme']; ?>/images/icons/icon_x_d.gif" width="17" height="17" title="delete selected rules" border="0"><?php else: ?><input name="del" type="image" src="../themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" title="delete selected mappings" onclick="return confirm('Do you really want to delete the selected Snort Rule?')"><?php endif; ?></td>
                      </tr>
                    </table>
				</td>
                </tr>
	</table>
	</div>
	</td>
  </tr>
</table>

<br>
  <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
	  <td width="100%"><span class="vexpl"><span class="red"><strong>Note:</strong></span>
	  <br>
		 This is the <strong>Snort Menu</strong>, displaying an overview of all interface settings.
		 <br>
		 Please edit the <strong>Global Settings</strong> tab before adding an interface.
		 <br><br>
		 <strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" title="Add Icon"> icon to add an interface.&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_pass.gif" width="13" height="13" border="0" title="Start Icon"> icon to <strong>start</strong> snort and barnyard.
		 <br>
		 <strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" title="Edit Icon"> icon to edit an interface and settings.&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<strong>Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_block.gif" width="13" height="13" border="0" title="Stop Icon"> icon to <strong>stop</strong> snort and barnyard.
		 <br>
		<strong> Click</strong> on the <img src="../themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" title="Delete Icon"> icon to delete an interface and settings.
</td>
    </table>
 
<?php
if ($pkg['tabs'] <> "") {
    echo "</td></tr></table>";
}
?>

</form>
<?php include("fend.inc"); ?>
</body>
</html>
