<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_nfs_export.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2006 Daniel S. Haischt <me@daniel.stefan.haischt.name>
    All rights reserved.

    Based on FreeNAS (http://www.freenas.org)
    Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */

$pgtitle = array(gettext("Services"),
                 gettext("NFS"),
                 gettext("Exports"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['mounts']['mount']))
  $freenas_config['mounts']['mount'] = array();

mount_sort();

$a_mount = &$freenas_config['mounts']['mount'];

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
  
  if (!$input_errors)
  {
    if($_POST['apply']) {
      $retval = 0;
      if(!file_exists($d_sysrebootreqd_path)) {
        config_lock();
        services_nfs_configure();
        services_zeroconf_configure();
        config_unlock();
      }
  
      $savemsg = get_std_save_message($retval);
  
      if(0 == $retval) {
        if(file_exists($d_nfsexportconfdirty_path))
          unlink($d_nfsexportconfdirty_path);
      }
    }
  }
}

if($_GET['act'] == "ret") {
  pfSenseHeader("services_nfs_export.php");
  exit;
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */

echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<form id="iform" name="iform" action="services_nfs_export.php" method="post">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_nfsexportconfdirty_path)): ?>
<?php print_info_box_np(gettext("The exports have been modified.") . "<br />" .
                        gettext("You must apply the changes in order for them to take effect."));?>
<?php endif; ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<div id="inputerrors"></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabnavtbl">
<?php
  $tab_array = array();
  $tab_array[0] = array(gettext("Settings"), false, "services_nfs.php");
  $tab_array[1] = array(gettext("Exports"),   true,  "services_nfs_export.php");
  display_top_tabs($tab_array);
?>  
    </td>
  </tr>
  <tr> 
    <td>
    <div id="mainarea">
    <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
      <tr>
        <td width="20%" class="listhdrr"><?= gettext("Export"); ?></td>
        <td width="25%" class="listhdrr"><?= gettext("Description"); ?></td>
        <td width="20%" class="listhdrr"><?= gettext("Export To"); ?></td>
        <td width="10%" class="list"></td>
      </tr>
      <?php $i = 0; foreach($a_mount as $mountv): ?>
      <tr>
        <td class="listr">/mnt/<?=htmlspecialchars($mountv['sharename']);?></td>
        <td class="listr"><?=htmlspecialchars($mountv['desc']);?></td>
        <td class="listbg" style="color: #FFFFFF;"><?= isset($mountv['nfs']['networks']) ? str_replace(",", "<br />", htmlspecialchars($mountv['nfs']['networks'])) : gettext("None"); ?></td>
        <td valign="middle" nowrap class="list">
          <?php if(isset($freenas_config['nfs']['enable']))
          echo("<a href='services_nfs_export_edit.php?id={$i}'><img src='./themes/" . $g['theme'] . "/images/icons/icon_e.gif' alt='" . gettext("Edit Export") . "' title='" . gettext("Edit Export") . "' width='17' height='17' border='0' /></a>");
          ?>
        </td>
      </tr>
      <?php $i++; endforeach; ?>
    </table>
    </div>
    </td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
