<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_samba_share_edit.php
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
                 gettext("CIFS"),
                 gettext("Shares"),
                 gettext("Edit"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

$id = $_GET['id'];
if (isset($_POST['id']))
  $id = $_POST['id'];

if (!is_array($freenas_config['mounts']['mount']))
  $freenas_config['mounts']['mount'] = array();

mount_sort();

if(!is_array($freenas_config['samba']['hidemount']))
  $freenas_config['samba']['hidemount'] = array();

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  
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
    if(!$_POST['browseable']) {
      $freenas_config['samba']['hidemount'] = array_merge($freenas_config['samba']['hidemount'],array($freenas_config['mounts']['mount'][$id]['sharename']));
    } else {
      if(is_array($freenas_config['samba']['hidemount']) && in_array($freenas_config['mounts']['mount'][$id]['sharename'],$freenas_config['samba']['hidemount'])) {
        $freenas_config['samba']['hidemount'] = array_diff($freenas_config['samba']['hidemount'],array($freenas_config['mounts']['mount'][$id]['sharename']));
      }
    }
  
    touch($d_smbshareconfdirty_path);
    write_config();
    pfSenseHeader("services_samba_share.php");
    exit;
  }
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
  <div id="inputerrors"></div>
  <form id="iform" name="iform" action="disks_mount_edit.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr> 
        <td width="22%" valign="top" class="vncellreq"><?= gettext("Share Name"); ?></td>
        <td width="78%" class="vtable"> 
          <input type="text" class="formfld file" size="30" value="<?=htmlspecialchars($freenas_config['mounts']['mount'][$id]['sharename']);?>" disabled="disabled" />
        </td>
      </tr>
      <tr> 
        <td width="22%" valign="top" class="vncellreq"><?= gettext("Description"); ?></td>
        <td width="78%" class="vtable"> 
          <input type="text" class="formfld unknown" size="30" value="<?=htmlspecialchars($freenas_config['mounts']['mount'][$id]['desc']);?>" disabled="disabled">
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncell"><?= gettext("Browseable"); ?></td>
        <td width="78%" class="vtable">
          <select name="browseable" class="formselect" id="browseable">
            <?php
              $text = array(gettext("Yes"),gettext("No"));
              $vals = explode(" ","1 0"); $j = 0;
              for($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if(is_array($freenas_config['samba']['hidemount']) && in_array($freenas_config['mounts']['mount'][$id]['sharename'],$freenas_config['samba']['hidemount'])) echo "selected=\"selected\"";?>> 
              <?=htmlspecialchars($text[$j]);?>
            </option>
            <?php endfor;?>
          </select>
          <br><?= gettext("This controls whether this share is seen in the list of available shares in a net view and in the browse list."); ?>
        </td>
      </tr>
      <tr> 
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%"> <input name="Submit" type="submit" class="formbtn" value="<?= gettext("Save"); ?>"> 
          <?php if(isset($id)): ?>
          <input name="id" type="hidden" value="<?=$id;?>"> 
          <?php endif; ?>
        </td>
      </tr>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
</body>
</html>
