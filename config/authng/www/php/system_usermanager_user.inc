<?php
/* $Id$ */
/* ========================================================================== */
/*
    system_usermanager_user.inc
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
      <td colspan="2" valign="top" class="listtopic"><?=$HTTP_SERVER_VARS['AUTH_USER']?>'s Password</td>
    </tr>
    <tr>
      <td width="22%" valign="top" class="vncell" rowspan="2">Password</td>
      <td width="78%" class="vtable">
        <input name="passwordfld1" type="password" class="formfld pwd" id="passwordfld1" size="20" />
      </td>
    </tr>
    <tr>
      <td width="78%" class="vtable">
        <input name="passwordfld2" type="password" class="formfld pwd" id="passwordfld2" size="20" />
        &nbsp;<?=gettext("(confirmation)");?>
        <br />
        <span class="vexpl"><?=gettext("Select a new password");?></span>
      </td>
    </tr>
    <tr>
      <td width="22%" valign="top">&nbsp;</td>
      <td width="78%">
        <input name="save" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
      </td>
    </tr>
  </table>
