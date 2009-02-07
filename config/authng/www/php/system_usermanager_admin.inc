<?php
/* $Id$ */
/* ========================================================================== */
/*
    system_usermanager_admin.inc
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
          <td>
<?php
    $tab_array = array();
    $tab_array[] = array(gettext("Users"), true, "system_usermanager.php");
    $tab_array[] = array(gettext("Group"), false, "system_groupmanager.php");
    $tab_array[] = array(gettext("Settings"), false, "system_usermanager_settings.php");
    display_top_tabs($tab_array);
?>
          </td>
      </tr>
      <tr>
          <td>
              <div id="mainarea">
                  <table class="tabcont" width="100%" border="0" cellpadding="6" cellspacing="0">
                      <tr>
                          <td>
                          <?php
                              if ($_GET['act'] == "new" || $_GET['act'] == "edit" || $input_errors) {
                                  if ($_GET['act'] == "edit") {
                                      if (isset($id) && $a_user[$id]) {
                                          $pconfig['usernamefld'] = $a_user[$id]['name'];
                                          $pconfig['fullname'] = $a_user[$id]['fullname'];
                                          $pconfig['groupname'] = $a_user[$id]['groupname'];
                                          $pconfig['utype'] = $a_user[$id]['scope'];
                                          $pconfig['authorizedkeys'] = base64_decode($a_user[$id]['authorizedkeys']);
                                      }
                                  } else if ($_GET['act'] == "new") {
                                    /* set this value cause the text field is read only
                                     * and the user should not be able to mess with this
                                     * setting.
                                     */
                                    $pconfig['utype'] = "user";
                                  }

                                  // finally pull in the php file containing the appropriate markup
                                  require_once("system_usermanager_admin_newedit.inc");
                          ?>
                          <?php
                              } else {
                                  // finally pull in the php file containing the appropriate markup
                                  require_once("system_usermanager_admin_index.inc");
                          ?>
                          <?php } ?>
                          </td>
                      </tr>
                  </table>
              </div> <!-- MAINAREA DIV -->
          </td>
      </tr>
  </table>
