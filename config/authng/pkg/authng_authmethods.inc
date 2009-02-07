<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng_authmethods.inc
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

class AbstractAuthMethod {
    function authenticate($backend) {
        trigger_error('AbstractAuthMethod::authenticate() needs to be overridden in a subclass.', E_USER_ERROR);
    }
}

class BasicAuthMethod extends AbstractAuthMethod {
    function authenticate($backend) {
        global $HTTP_SERVER_VARS;

        /* Check for AUTH_USER */
        if ($HTTP_SERVER_VARS['PHP_AUTH_USER'] <> "") {
            $HTTP_SERVER_VARS['AUTH_USER'] = $HTTP_SERVER_VARS['PHP_AUTH_USER'];
            $HTTP_SERVER_VARS['AUTH_PW'] = $HTTP_SERVER_VARS['PHP_AUTH_PW'];
        }
        if (!isset($HTTP_SERVER_VARS['AUTH_USER'])) {
            require_once("authng_authgui.inc");
            header("WWW-Authenticate: Basic realm=\".\"");
            header("HTTP/1.0 401 Unauthorized");
            display_error_form("401", gettext("You must enter valid credentials to access this resource."));
            exit;
        } else {
            return $backend($HTTP_SERVER_VARS['AUTH_USER'],$HTTP_SERVER_VARS['AUTH_PW']);
        }
    }
}

class SessionAuthMethod extends AbstractAuthMethod {
    function authenticate($backend) {
        global $g, $HTTP_SERVER_VARS, $userindex, $config;

        session_start();

        /* Validate incoming login request */
        if (isset($_POST['login'])) {
            if ($backend($_POST['usernamefld'], $_POST['passwordfld'])) {
                $_SESSION['Logged_In'] = "True";
                $_SESSION['Username'] = $_POST['usernamefld'];
                $_SESSION['last_access'] = time();
            } else {
                $_SESSION['Login_Error'] = "Username or password incorrect.";
            }
        }

        /* Show login page if they aren't logged in */
        if (empty($_SESSION['Logged_In'])) {

            /* Don't display login forms to AJAX */
            if (isAjax())
              return false;

            require_once("authng_authgui.inc");
            display_login_form();
            return false;
        } else {
            /* If session timeout isn't set, we don't mark sessions stale */
            if (!isset($config['system']['webgui']['session_timeout']) or
                $config['system']['webgui']['session_timeout'] == 0 or
                $config['system']['webgui']['session_timeout'] == "")
                  $_SESSION['last_access'] = time();
            else
                /* Check for stale session */
                if ($_SESSION['last_access'] < (time() - ($config['system']['webgui']['session_timeout'] * 60)))
                    $_GET['logout'] = true;
            else
                /* only update if it wasn't ajax */
                if (!isAjax())
                    $_SESSION['last_access'] = time();

            /* user hit the logout button */
            if (isset($_GET['logout'])) {
                if (hasLockAbility($_SESSION['Username'])) {
                    unlink_if_exists("{$g['tmp_path']}/webconfigurator.lock");
                }

                /* wipe out $_SESSION */
                $_SESSION = array();

                if (isset($_COOKIE[session_name()])) {
                    setcookie(session_name(), '', time()-42000, '/');
                }

                /* and destroy it */
                session_destroy();

                $scriptName = split("/", $_SERVER["SCRIPT_FILENAME"]);
                $scriptElms = count($scriptName);
                $scriptName = $scriptName[$scriptElms-1];

                if (isAjax())
                    return false;
                    
              /* redirect to page the user is on, it'll prompt them to login again */
              pfSenseHeader($scriptName);

              return false;

            /* user wants to explicitely delete the log file.
             * Requires a particular privilege.
             */
            } else if ($_GET['deletelock'] && hasLockAbility($_SESSION['Username'])) {
                unlink_if_exists("{$g['tmp_path']}/webconfigurator.lock");
                $HTTP_SERVER_VARS['AUTH_USER'] = $_SESSION['Username'];
                return true;

            /* this is for debugging purpose if you do not want to use Ajax
             * to submit a HTML form. It basically disables the observation
             * of the submit event and hence does not trigger Ajax.
             */
            } else if ($_GET['disable_ajax']) {
                $_SESSION['NO_AJAX'] = "True";
                $HTTP_SERVER_VARS['AUTH_USER'] = $_SESSION['Username'];
                return true;

            /* Same to re-enable Ajax.
             */
            } else if ($_GET['enable_ajax']) {
                unset($_SESSION['NO_AJAX']);
                $HTTP_SERVER_VARS['AUTH_USER'] = $_SESSION['Username'];
                return true;

            /* user wants to explicitely create a lock.
             * Requires a particular privilege.
             */
            } else if ($_GET['createlock'] && hasLockAbility($_SESSION['Username'])) {
                $fd = fopen("{$g['tmp_path']}/webconfigurator.lock", "w");
                fputs($fd, "{$_SERVER['REMOTE_ADDR']} (" .
                         getRealName($_SESSION['Username']) . ")");
                fclose($fd);
                /* if the user did delete the lock manually, do not
                * re-create it while the session is valide.
                */
                $_SESSION['Lock_Created'] = "True";
                $HTTP_SERVER_VARS['AUTH_USER'] = $_SESSION['Username'];
                return true;

            /* proceed with the login process */
            } else {
               /* if the user is allowed to create a lock,
                * create it once per session.
                */
                if (hasLockAbility($_SESSION['Username']) &&
                  ! isset($_SESSION['Lock_Created'])) {

                    $fd = fopen("{$g['tmp_path']}/webconfigurator.lock", "w");
                    fputs($fd, "{$_SERVER['REMOTE_ADDR']} (" .
                               getRealName($_SESSION['Username']) . ")");
                    fclose($fd);
                    /* if the user did delete the lock manually, do not
                     * re-create it while the session is valide.
                     */
                    $_SESSION['Lock_Created'] = "True";

                /* give regular users a chance to automatically invalidate
                * a lock if its older than a particular time.
                */
                } else if (! hasLockAbility($_SESSION['Username']) &&
                            file_exists("{$g['tmp_path']}/webconfigurator.lock")) {

                    $offset = 12; //hours
                    $mtime = filemtime("{$g['tmp_path']}/webconfigurator.lock");
                    $now_minus_offset = mktime(date("H") - $offset, 0, 0, date("m"), date("d"), date("Y"));

                    if (($mtime - $now_minus_offset) < $mtime) {
                        require_once("auth/authgui.inc");
                        display_login_form();
                        return false;
                    } else {
                       /* file is older than mtime + offset which may
                        * indicate a stale lockfile, hence we are going
                        * to remove it.
                        */
                        unlink_if_exists("{$g['tmp_path']}/webconfigurator.lock");
                    }
                }

                $HTTP_SERVER_VARS['AUTH_USER'] = $_SESSION['Username'];
                return true;
            } // end if
        } // end if
    } // end function
}

?>