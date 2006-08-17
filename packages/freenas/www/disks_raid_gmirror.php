<?php
/* $Id$ */
/*
	disks_manage_edit.php
	part of FreeNAS (http://www.freenas.org)
	Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
	All rights reserved.
	
	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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
                 gettext("GEOM Mirror"),
                 gettext("RAID"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['gmirror']['vdisk']))
	$freenas_config['gmirror']['vdisk'] = array();

gmirror_sort();

$raidstatus=get_sraid_disks_list();

$a_raid = &$freenas_config['gmirror']['vdisk'];

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  
  $pconfig = $_POST;

  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
	if ($_POST['apply']) {
		$retval = 0;
		if (!file_exists($d_sysrebootreqd_path))
		{
			config_lock();
			/* reload all components that create raid device */
			disks_raid_gmirror_configure();
			config_unlock();
			write_config();
		}
		$savemsg = get_std_save_message($retval);
		if ($retval == 0) {
			if (file_exists($d_raidconfdirty_path))
				unlink($d_raidconfdirty_path);
		}
	}
}

if ($_GET['act'] == "del") {
	if ($a_raid[$_GET['id']]) {
		$raidname=$a_raid[$_GET['id']]['name'];
		disks_raid_gmirror_delete($raidname);
		unset($a_raid[$_GET['id']]);
		write_config();
		pfSenseHeader("disks_raid_gmirror.php");
		exit;
	}
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>

<form action="disks_raid_gmirror.php" method="post" name="iform" id="iform">
<?php if (file_exists($d_diskdirty_path)): ?>
<?php print_info_box_np(gettext("The Raid configuration has been changed.") . "<br />" .
                        gettext("You must apply the changes in order for them to take effect."));?>
<?php endif; ?>

<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Geom Mirror"),           true,  "disks_raid_gmirror.php");
	$tab_array[1] = array(gettext("Geom Vinum (unstable)"), false, "disks_raid_gvinum.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr>
    <td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[0] = array(gettext("Manage RAID"), true,  "disks_raid_gmirror.php");
	$tab_array[1] = array(gettext("Format RAID"), false, "disks_raid_gmirror_init.php");
	$tab_array[2] = array(gettext("Tools"),       false, "disks_raid_gmirror_tools.php");
  $tab_array[3] = array(gettext("Information"), false, "disks_raid_gmirror_infos.php");
	display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr> 
    <td>
	  <div id="mainarea">
    <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
      <tr> 
      	<td class="listhdrr"><?=gettext("Volume name");?></td>
      	<td class="listhdrr"><?=gettext("Type");?></td>
        <td class="listhdrr"><?=gettext("Size");?></td>
        <td class="listhdrr"><?=gettext("Status");?></td>
      	<td class="list">&nbsp;</td>
      </tr>
      <?php $i = 0; foreach ($a_raid as $raid): ?>
      <tr> 
    	  <td valign="middle" class="listr">
          <?=htmlspecialchars($raid['name']);?>
    		</td>
    	  <td valign="middle" class="listr">
          <?=htmlspecialchars($raid['type']);?>
    		</td>
    	  <td valign="middle" class="listr">
          <?php
		        $raidconfiguring = file_exists($d_raidconfdirty_path) &&
                               in_array($raid['name']."\n",file($d_raidconfdirty_path));
            if ($raidconfiguring)
						  echo gettext("configuring");
					  else {
						  $tempo=$raid['name'];						
						  echo "{$raidstatus[$tempo]['size']}";
						}
          ?>&nbsp;
    		</td>
    	  <td valign="middle" class="listr">
          <?php
            if ($raidconfiguring)
					  	echo gettext("configuring");
					  else {
						  echo "{$raidstatus[$tempo]['desc']}";
						}
				  ?>&nbsp;
    		</td>
    		<td valign="middle" class="list"> 
          <a href="disks_raid_gmirror_edit.php?id=<?=$i;?>">
            <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="<?=gettext("edit raid");?>" width="17" height="17" border="0" alt="" />
          </a>
    		  <a href="disks_raid_gmirror.php?act=del&id=<?=$i;?>">
            <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" title="<?=gettext("delete raid");?>" width="17" height="17" border="0" alt="" />
          </a> 
    		</td>
      </tr>
      <?php $i++; endforeach; ?>
      <tr>
    	  <td class="list" colspan="4"></td>
    	  <td class="list" nowrap>
    	    <a href="disks_raid_gmirror_edit.php">
            <img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" title="<?=gettext("add disk");?>" width="17" height="17" border="0" alt="" />
          </a>
    	  </td>
      </tr>
      <tr>
        <td align="left" valign="top" colspan="5">
            <span class="red">
              <strong><?= gettext("Note:"); ?></strong>
            </span>
            <br />
            <span class="vexpl"><?= gettext("Optional configuration step: Configuring a virtual RAID disk using your"); ?></span>
            <br />
            <span class="vexpl"><a href="disks_manage.php"><?= gettext("previsously configured disk."); ?></a></span>
            <br />
            <span class="vexpl"><?= gettext("Wait for the \"up\" status before format it and mount it!."); ?></span>
          </span>
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
