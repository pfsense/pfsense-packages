#!/usr/local/bin/php
<?php 
/* $Id$ */
/*
	disks_manage.php
	part of FreeNAS (http://www.freenas.org)
	Copyright (C) 2005-2006 Olivier Cochard <olivier@freenas.org>.
	All rights reserved.
	
	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
	All rights reserved.
	
	Copyright (C) 2003-2005 Manuel Kasper <mk@neon1.net>.
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

$pgtitle = array(gettext("System"),
                 gettext("Disks"),
                 gettext("Management"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['disks']['disk']))
	$freenas_config['disks']['disk'] = array();
	
disks_sort();

$a_disk_conf = &$freenas_config['disks']['disk'];

if ($_POST) {

	unset($input_errors);

	/* input validation */

	if ($_POST['apply']) {
		$retval = 0;
		if (! file_exists($d_sysrebootreqd_path)) {
			config_lock();
			/* reload all components that mount disk */
			// disks_mount_all();
			/* Is formated?: If not create FS */
			/* $retval = disk_disks_create_ufs(); */
			
			config_unlock();
		}
		$savemsg = get_std_save_message($retval);
		if ($retval == 0) {
			if (file_exists($d_diskdirty_path)) {
				unlink($d_diskdirty_path);
      }
		}
	}

	/* if this is an AJAX caller then handle via JSON */
	if(isAjax() && is_array($input_errors)) {		
		input_errors2Ajax($input_errors);		
		exit;	
	}
	
	if (!$input_errors) {
		/* No errors detected, so update the config */
	}
}

if ($_GET['act'] == "del") {
	if ($a_disk_conf[$_GET['id']]) {
		unset($a_disk_conf[$_GET['id']]);
		write_config();
		touch($d_diskdirty_path);
		pfSenseHeader("disks_manage.php");
		exit;
	}
}

/* if ajax is calling, give them an update message */
if(isAjax())
	print_info_box_np($savemsg);

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="disks_manage.php" method="post" name="iform" id="iform">
<?php if (file_exists($d_diskdirty_path)): ?>
<?php print_info_box_np(gettext("The disk list has been changed.") . "<br />" .
                        gettext("You must apply the changes in order for them to take effect."));?>
<?php endif; ?>

<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Manage"),          true,  "disks_manage.php");
	$tab_array[1] = array(gettext("Format"),          false, "disks_manage_init.php");
	$tab_array[2] = array(gettext("iSCSI Initiator"), false, "disks_manage_iscsi.php");
	display_top_tabs($tab_array);
?>  
  </td></tr>
  <tr> 
    <td>
	<div id="mainarea">
        <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
       <tr> 
	<td class="listhdrr"><?=gettext("Disk");?></td>
	<td class="listhdrr"><?=gettext("Size");?></td>
  <td class="listhdrr"><?=gettext("Description");?></td>
  <td class="listhdrr"><?=gettext("Standby time");?></td>
  <td class="listhdrr"><?=gettext("File system");?></td>
  <td class="listhdrr"><?=gettext("Status");?></td>
	<td class="list">&nbsp;</td>
  </tr>
  <?php $i = 0; foreach ($a_disk_conf as $disk): ?>
  <tr> 
	  <td valign="middle" class="listr">
      <?=htmlspecialchars($disk['name']);?>
		</td>
	  <td valign="middle" class="listr">
      <?=htmlspecialchars($disk['size']);?>
		</td>
	  <td valign="middle" class="listr">
      <?=htmlspecialchars($disk['desc']);?>&nbsp;
		</td>
	  <td valign="middle" class="listr">
      <?php
        if ($disk['harddiskstandby']) {
				  $value = $disk['harddiskstandby'];
					//htmlspecialchars($value);
					echo $value;
				} else {
					echo "Always on";
				}
			?>
		</td>
	  <td valign="middle" class="listr">
      <?= ($disk['fstype']) ? $disk['fstype']: gettext("unknown or unformatted"); ?>
		</td>
	  <td valign="middle" class="listr">
      <?php
        $stat = disks_status($disk);
        echo $stat;
      ?>
		</td>
		<td valign="middle" class="list"> 
      <a href="disks_manage_edit.php?id=<?=$i;?>">
        <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="<?=gettext("edit disk");?>" width="17" height="17" border="0" alt="" />
      </a>
		  <a href="disks_manage.php?act=del&id=<?=$i;?>">
        <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="<?=gettext("delete disk");?>" width="17" height="17" border="0" alt="" />
      </a> 
		</td>
  </tr>
  <?php $i++; endforeach; ?>
  <tr>
	  <td class="list" colspan="6"></td>
	  <td class="list" nowrap>
	    <a href="disks_manage_edit.php">
        <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("add disk");?>" width="17" height="17" border="0" alt="" />
      </a>
	  </td>
  </tr>
</table>
</div>
    </td>
	</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
