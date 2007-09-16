<?php
/* $Id$ */
/* ========================================================================== */
/*
    system_usermanager_admin_newuser.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2007 Daniel S. Haischt <me@daniel.stefan.haischt.name>
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
/* ======================= P A G E   F R A G M E N T ======================== */
/* ========================================================================== */
?>

<div id="inputerrors"></div>
<table width="100%" border="0" cellpadding="6" cellspacing="0">
  <tr>
    <td width="22%" valign="top" class="vncellreq"><?=gettext("Username");?></td>
    <td width="78%" class="vtable">
      <input name="usernamefld" type="text" class="formfld user" id="usernamefld" size="20" value="<?=htmlspecialchars($pconfig['usernamefld']);?>" <?php if ($pconfig['utype'] == "system") { echo "readonly=\"readonly\" "; }?>/>
      <input name="oldusername" type="hidden" id="oldusername" value="<?=htmlspecialchars($pconfig['usernamefld']);?>" />
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncellreq" rowspan="2"><?=gettext("Password");?></td>
    <td width="78%" class="vtable">
      <input name="passwordfld1" type="password" class="formfld pwd" id="passwordfld1" size="20" value="" />
    </td>
  </tr>
  <tr>
    <td width="78%" class="vtable">
      <input name="passwordfld2" type="password" class="formfld pwd" id="passwordfld2" size="20" value="" />&nbsp;<?= gettext("(confirmation)"); ?>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncell"><?=gettext("Full name");?></td>
    <td width="78%" class="vtable">
      <input name="fullname" type="text" class="formfld unknown" id="fullname" size="20" value="<?=htmlspecialchars($pconfig['fullname']);?>" <?php if ($pconfig['utype'] == "system") { echo "readonly=\"readonly\" "; }?>/>
      <br />
      <?=gettext("User's full name, for your own information only");?>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top" class="vncell"><?=gettext("User type");?></td>
    <td width="78%" class="vtable">
      <input name="utype" type="text" class="formfld unknown" id="utype" size="20" value="<?=htmlspecialchars($pconfig['utype']);?>" readonly="readonly" />
      <br />
      <?=gettext("Indicates whether this is a system (aka non-deletable) user or a user created by a particular user.");?>
    </td>
  </tr>
  <?php if (isSystemAdmin($HTTP_SERVER_VARS['AUTH_USER'])): ?>
  <tr>
    <td width="22%" valign="top" class="vncell"><?=gettext("User Privileges");?></td>
    <td width="78%" class="vtable">
      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
          <td width="5%" class="listhdrr"><?=gettext("ID");?></td>
          <td width="30%" class="listhdrr"><?=gettext("Name");?></td>
          <td width="40%" class="listhdrr"><?=gettext("Description");?></td>
          <td width="5%" class="list"></td>
        </tr>

        <?php if(is_array($t_privs)): ?>
        <?php $i = 0; foreach ($t_privs as $priv): ?>
        <?php if($priv['id'] <> ""): ?>
        <tr>
          <td class="listlr" <?php if($a_user[$id]['scope'] == "user") echo "ondblclick=\"document.location='system_usermanager_edit.php?id={$i}&userid={$id}&useract={$_GET['act']}';\""; ?>>
            <?=htmlspecialchars($priv['id']);?>
          </td>
          <td class="listlr" <?php if($a_user[$id]['scope'] == "user") echo "ondblclick=\"document.location='system_usermanager_edit.php?id={$i}&userid={$id}&useract={$_GET['act']}';\""; ?>>
            <?=htmlspecialchars($priv['name']);?>
          </td>
          <td class="listbg" <?php if($a_user[$id]['scope'] == "user") echo "ondblclick=\"document.location='system_usermanager_edit?id={$i}&userid={$id}&useract={$_GET['act']}';\""; ?>>
            <font color="#FFFFFF"><?=htmlspecialchars($priv['descr']);?>&nbsp;</font>
          </td>
          <td valign="middle" nowrap class="list">
            <?php if($a_user[$id]['scope'] == "user"): ?>
            <table border="0" cellspacing="0" cellpadding="1">
              <tr>
                <td valign="middle"><a href="system_usermanager_edit.php?id=<?=$i;?>&userid=<?= $id ?>&useract=<?= $_GET['act'] ?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0" alt="" /></a></td>
                <td valign="middle"><a href="system_usermanager.php?act=del&privid=<?=$i;?>&what=priv&id=<?= $id ?>" onclick="return confirm('<?=gettext("Do you really want to delete this mapping?");?>')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0" alt="" /></a></td>
              </tr>
            </table>
            <?php endif; ?>
          </td>
        </tr>
        <?php endif; ?>
        <?php $i++; endforeach; ?>
        <?php endif; ?>

        <?php if($a_user[$id]['scope'] == "user"): ?>
        <tr>
          <td class="list" colspan="3"></td>
          <td class="list">
            <table border="0" cellspacing="0" cellpadding="1">
              <tr>
                <td valign="middle"><a href="system_usermanager_edit.php?userid=<?= $id ?>&useract=<?= $_GET['act'] ?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0" alt="" /></a></td>
              </tr>
            </table>
          </td>
        </tr>
        <?php endif; ?>
      </table>
    </td>
  </tr>
  <?php endif; ?>
  <?php if (isset($config['system']['ssh']['sshdkeyonly'])): ?>
  <tr>
    <td width="22%" valign="top" class="vncell"><?=gettext("Authorized keys");?></td>
    <td width="78%" class="vtable">
      <textarea name="authorizedkeys" cols="65" rows="7" id="authorizedkeys" class="formfld_cert" wrap="off"><?=htmlspecialchars($pconfig['authorizedkeys']);?></textarea>
      <br />
      <?=gettext("Paste an authorized keys file here.");?>
    </td>
  </tr>
  <?php endif; ?>
  <tr>
    <td width="22%" valign="top" class="vncell"><?=gettext("Group Name");?></td>
    <td width="78%" class="vtable">
      <select name="groupname" class="formselect" id="groupname" <?php if ($pconfig['utype'] == "system") { echo "disabled=\"disabled\" "; } ?>>
      <?php foreach ($config['system']['group'] as $group): ?>
        <option value="<?=$group['name'];?>" <?php if ($group['name'] == $pconfig['groupname']) { echo "selected"; } ?>>
              <?=htmlspecialchars($group['name']);?>
        </option>
      <?php endforeach;?>
      </select>
      <br />
      <?=gettext("The admin group to which this user is assigned.");?>
    </td>
  </tr>
  <tr>
    <td width="22%" valign="top">&nbsp;</td>
    <td width="78%">
      <input id="submit" name="save" type="submit" class="formbtn" value="Save" />
      <?php if (isset($id) && $a_user[$id]): ?>
      <input name="id" type="hidden" value="<?=$id;?>" />
      <?php endif;?>
    </td>
  </tr>
</table>
