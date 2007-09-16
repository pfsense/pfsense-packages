<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng_usermanager.inc
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

function initUserFromGetVars() {
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
}
function processUserManagerPostVarsUser() {
    if (isset($_POST['save'])) {
        unset($input_errors);

        /* input validation */
        $reqdfields = explode(" ", "passwordfld1");
        $reqdfieldsn = explode(",", "Password");

        do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

        if ($_POST['passwordfld1'] != $_POST['passwordfld2'])
            $input_errors[] = "The passwords do not match.";

        if (!$input_errors) {
            // all values are okay --> saving changes
            $config['system']['user'][$userindex[$HTTP_SERVER_VARS['AUTH_USER']]]['password'] = crypt(trim($_POST['passwordfld1']));

            write_config();

            sync_webgui_passwords();

            $retval = system_password_configure();
            $savemsg = get_std_save_message($retval);
            $savemsg = "Password successfully changed<br />";
        }
    }
}

function processUserManagerPostVarsAdmin() {
    $id = $_GET['id'];
    if (isset($_POST['id']))
        $id = $_POST['id'];

    if (!is_array($config['system']['user'])) {
        $config['system']['user'] = array();
    }

    admin_users_sort();
    $a_user = &$config['system']['user'];
    $t_privs = $a_user[$id]['priv'];

    if ($_GET['act'] == "del" && $_GET['what'] == "user") {
        if ($a_user[$_GET['id']]) {
            $userdeleted = $a_user[$_GET['id']]['name'];
            unset($a_user[$_GET['id']]);
            write_config();
            $retval = system_password_configure();
            $savemsg = get_std_save_message($retval);
            $savemsg = gettext("User") . " " . $userdeleted . " " . gettext("successfully deleted") . "<br />";
        }
    } else if ($_GET['act'] == "del" && $_GET['what'] == "priv") {
        if ($t_privs[$_GET['privid']]) {
            $privdeleted = $t_privs[$_GET['privid']]['id'];
            unset($t_privs[$_GET['privid']]);
            write_config();
            $_GET['act'] = "edit";
            $retval = 0;
            $savemsg = get_std_save_message($retval);
            $savemsg = gettext("Privilege") . " " . $privdeleted . " " . gettext("of user") . " " . $a_user[$_GET['id']]['name'] . " " . gettext("successfully deleted") . "<br />";
        }
    }

    if ($_POST) {
        unset($input_errors);
        $pconfig = $_POST;

        /* input validation */
        if (isset($id) && ($a_user[$id])) {
            $reqdfields = explode(" ", "usernamefld");
            $reqdfieldsn = explode(",", "Username");
        } else {
            $reqdfields = explode(" ", "usernamefld passwordfld1");
            $reqdfieldsn = explode(",", "Username,Password");
        }

        do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

        if (hasShellAccess($_POST['usernamefld'])) {
          if (preg_match("/[^a-zA-Z0-9\.\-_]/", $_POST['usernamefld']))
              $input_errors[] = gettext("The username contains invalid characters.");
        } else {
          if (preg_match("/[^a-zA-Z0-9\@\.\-_]/", $_POST['usernamefld']))
              $input_errors[] = gettext("The username contains invalid characters.");
        }

        if (($_POST['passwordfld1']) && ($_POST['passwordfld1'] != $_POST['passwordfld2']))
            $input_errors[] = gettext("The passwords do not match.");

        if (!$input_errors && !(isset($id) && $a_user[$id])) {
            /* make sure there are no dupes */
            foreach ($a_user as $userent) {
                if ($userent['name'] == $_POST['usernamefld']) {
                    $input_errors[] = gettext("Another entry with the same username already exists.");
                    break;
                }
            }
        }

        if ($pconfig['utype'] <> "system" && !isset($groupindex[$_POST['groupname']])) {
            $input_errors[] = gettext("group does not exist, please define the group before assigning users.");
        }

        if (isset($config['system']['ssh']['sshdkeyonly']) &&
            empty($_POST['authorizedkeys'])) {
          $input_errors[] = gettext("You must provide an authorized key otherwise you won't be able to login into this system.");
        }

        /* if this is an AJAX caller then handle via JSON */
        if (isAjax() && is_array($input_errors)) {
            input_errors2Ajax($input_errors);
            exit;
        }

        if (!$input_errors) {
            if (isset($id) && $a_user[$id])
                $userent = $a_user[$id];

            /* the user did change his username */
            if ($_POST['usernamefld'] <> $_POST['oldusername']) {
                $_SERVER['REMOTE_USER'] = $_POST['usernamefld'];
            }

            $userent['name'] = $_POST['usernamefld'];
            $userent['fullname'] = $_POST['fullname'];
            if ($pconfig['utype'] <> "system") {
              $userent['groupname'] = $_POST['groupname'];
            }
            isset($_POST['utype']) ? $userent['scope'] = $_POST['utype'] : $userent['scope'] = "system";

            if ($_POST['passwordfld1'])
                $userent['password'] = crypt($_POST['passwordfld1']);

            if(isset($config['system']['ssh']['sshdkeyonly'])) {
                $userent['authorizedkeys'] = base64_encode($_POST['authorizedkeys']);
            }

            if (isset($id) && $a_user[$id])
                $a_user[$id] = $userent;
            else
                $a_user[] = $userent;

            write_config();
            $retval = system_password_configure();
            sync_webgui_passwords();

            pfSenseHeader("system_usermanager.php");
        }
    }
}

/**
 * getWindowJSScriptRefs()
 *
 * @return
 */
function getWindowJSScriptRefs(){
    $result = array('<script type="text/javascript" src="/javascripts/windows-js/javascript/effects.js"></script>',
                    '<script type="text/javascript" src="/javascripts/windows-js/javascript/window.js"></script>',
                    '<script type="text/javascript" src="/javascripts/windows-js/javascript/window_effects.js"></script>',
                    '<script type="text/javascript" src="/javascripts/windows-js/javascript/window_effects.js"></script>',
                    '<script type="text/javascript" src="/javascripts/windows-js/javascript/debug.js"></script>');

    return $result;
}

/**
 * openNoUserDefsDialog()
 *
 * @param mixed $effectClass
 * @return
 */
function openNoUserDefsDialog($effectClass) {
    if (empty($config['installedpackages']['authng']['config'])) {
        $alertMessage = gettext("No users or group found. You will be forwarded to the AuthNG wizard to be able to define users and groups.");
        $dialogScript = "
            <script type='text/javascript'>
                function forwardToWizard() {
                    window.location.href = '/wizard.php?xml=authng_wizard.xml';
                }

                function openNoUserDefsDialog(html) {
                    var effect = new PopupEffect(html, {className: '${effectClass}'});
                    Dialog.alert('${alertMessage},{className:'alphacube', width: 400, height:null, showEffect:effect.show.bind(effect), hideEffect:effect.hide.bind(effect), onOk:forwardToWizard});
                }
            </script>
        ";

        return $dialogScript;
    }
}

?>