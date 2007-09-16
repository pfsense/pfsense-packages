<?php
/* $Id$ */
/* ========================================================================== */
/*
    system_usermanager_admin_edituser.php
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
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
  <td width="35%" class="listhdrr">Username</td>
  <td width="20%" class="listhdrr">Full name</td>
  <td width="20%" class="listhdrr">Group</td>
  <td width="10%" class="list"></td>
</tr>
<?php
    $i = 0;
    foreach($a_user as $userent):
?>
<tr ondblclick="document.location.herf = 'system_usermanager.php?act=edit&id=<?=$i;?>';">
  <td class="listlr">
    <table border="0" cellpadding="0" cellspacing="0">
      <tr>
        <td align="left" valign="middle">
          <?php if($userent['scope'] == "user"): ?>
          <img src="/themes/<?=$g['theme'];?>/images/icons/icon_system-user.png" alt="User" title="User" border="0" height="20" width="20" />
          <?php else: ?>
          <img src="/themes/<?=$g['theme'];?>/images/icons/icon_system-user-grey.png" alt="User" title="User" border="0" height="20" width="20" />
          <?php endif; ?>
          &nbsp;
        </td>
        <td align="left" valign="middle">
          <?=htmlspecialchars($userent['name']);?>
        </td>
      </tr>
    </table>
  </td>
  <td class="listr"><?=htmlspecialchars($userent['fullname']);?>&nbsp;</td>
  <td class="listbg">
    <font color="white"><?=htmlspecialchars($userent['groupname']);?></font>&nbsp;
  </td>
  <td valign="middle" nowrap class="list">
    <a href="system_usermanager.php?act=edit&id=<?=$i;?>">
      <img src="/themes/<?= $g['theme'];?>/images/icons/icon_e.gif" title="edit user" alt="edit user" width="17" height="17" border="0" />
    </a>
    <?php if($userent['scope'] == "user"): ?>
    &nbsp;
    <a href="system_usermanager.php?act=del&what=user&id=<?=$i;?>" onclick="return confirm('<?=gettext("Do you really want to delete this User?");?>')">
      <img src="/themes/<?= $g['theme'];?>/images/icons/icon_x.gif" title="delete user" alt="delete user" width="17" height="17" border="0" />
    </a>
    <?php endif; ?>
  </td>
</tr>
<?php
    $i++;
    endforeach;
?>
<tr>
  <td class="list" colspan="3"></td>
  <td class="list">
    <a href="system_usermanager.php?act=new">
      <img src="/themes/<?= $g['theme'];?>/images/icons/icon_plus.gif" title="add user" alt="add user" width="17" height="17" border="0" />
    </a>
  </td>
</tr>
<tr>
  <td colspan="3">
    <p>
      <?=gettext("Additional webConfigurator users can be added here.  User permissions are determined by the admin group they are a member of.");?>
    </p>
    <p>
      <?=gettext("An user icon that appears grey indicates that it is a system user and thus it's only possible to modified a subset of the regular user data. Additionally such an user can't be deleted.");?>
    </p>
  </td>
</tr>
</table>