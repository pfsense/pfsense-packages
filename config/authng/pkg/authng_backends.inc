<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng_backends.inc
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

class AbstractBackend {
    function authenticate($username, $passwd) {
        trigger_error('AbstractBackend::authenticate() needs to be overridden in a subclass.', E_USER_ERROR);
    }
}

class HtpasswdBackend extends AbstractBackend {
    function HtpasswdBackend() {
    }
    
    function authenticate($username, $passd) {
        $authfile = file("/var/run/htpasswd");

        /* sanity check to ensure that /usr/local/www/.htpasswd doesn't exist */
        unlink_if_exists("/usr/local/www/.htpasswd");

        $matches="";
        if(!($line = array_shift(preg_grep("/^$username:.*$/", $authfile))))
              return false;

        /* Get crypted password */
        preg_match("/^$username:((\\$1\\$[.\d\w_\/]{8}\\$)[.\d\w_\/]{22})$/", $line, $matches);
        $pass = $matches[1];
        $salt = $matches[2];

        /* Encrypt entered password with salt
        * And finally validate password
        */
        if ($pass == crypt($passwd, $salt))
            return true;
        else
            return false;
    }
}

class PasswdBackend extends AbstractBackend {
    function PasswdBackend() {
    }
    
    function authenticate($username, $passd) {
        $authfile = file("/etc/master.passwd");

        $matches="";

        /* Check to see if user even exists */
        if(!($line = array_shift(preg_grep("/^$username:.*$/", $authfile))))
            return false;

        /* Get crypted password */
        preg_match("/^$username:((\\$1\\$[.\d\w_\/]{8}\\$)[.\d\w_\/]{22})$/", $line, $matches);
        $pass = $matches[1];
        $salt = $matches[2];

        /* Encrypt entered password with salt
        * And finally validate password
        */
        if ($pass == crypt($passwd, $salt))
            return true;
        else
            return false;
    }
}

class PamBackend extends AbstractBackend {
    function PamBackend() {
    }
    
    function authenticate($username, $passd) {
        /* we do not support blank pwds, don't we? */
        if ($username == "" || passwd == "") { return false; }

        if(! extension_loaded( 'pam_auth' )) {
            if(! @dl( 'pam_auth.so' )) {
                return false;
            } else {
                /* no php file no auth, sorry */
                if (! file_exists("/etc/pam.d/php")) {
                    if (! file_exists("/etc/pam.d")) { mkdir("/etc/pam.d"); }

                    $pam_php = <<<EOD
# /etc/pam.d/php
#
# note: both an auth and account entry are required

# auth
auth            required        pam_nologin.so          no_warn
auth            sufficient      pam_opie.so             no_warn no_fake_prompts
auth            requisite       pam_opieaccess.so       no_warn allow_local
auth            required        pam_unix.so             no_warn try_first_pass

# account
account         required        pam_unix.so

# session
session         required        pam_permit.so

# password
password        required        pam_unix.so             no_warn try_first_pass

EOD;

                    file_put_contents("/etc/pam.d/php", $pam_php);
                } // end if

                if (pam_auth($username, $passwd, &$error)) {
                    return true;
                } else {
                    return false;
                }
            } // end if
        } // end if
    } // end function
}

class RadiusBackend extends AbstractBackend {
    function RadiusBackend() {
    }
    
    function authenticate($username, $passwd) {
        global $config, $debug;
        $ret = false;
        $radiusservers = $config['system']['radius']['servers'];

        $rauth = new Auth_RADIUS_PAP($username, $passwd);
        foreach ($radiusservers as $radsrv) {
            // Add a new server to our instance
            $rauth->addServer($radsrv['ipaddr'], $radsrv['port'], $radsrv['sharedsecret']);
        }

        if (!$rauth->start()) {
            $retvalue['auth_val'] = 1;
            $retvalue['error'] = $rauth->getError();
            if ($debug)
                printf("Radius start: %s", $retvalue['error']);
        }

        // XXX - billm - somewhere in here we need to handle securid challenge/response

        // Send request
        $result = $rauth->send();
        
        if (PEAR::isError($result)) {
            $retvalue['auth_val'] = 1;
            $retvalue['error'] = $result->getMessage();
            if ($debug)
                printf("Radius send failed: %s", $retvalue['error']);
        } else if ($result === true) {
            $retvalue['auth_val'] = 2;
            if ($debug)
                printf (gettext("Radius Auth succeeded"));
            $ret = true;
        } else {
            $retvalue['auth_val'] = 3;
            if ($debug)
                printf (gettext("Radius Auth rejected"));
        }
        // close OO RADIUS_AUTHENTICATION
        $rauth->close();

        return $ret;
    } // end function
}

class LdapBackend extends AbstractBackend {
    function LdapBackend() {
    }
    
    function authenticate($username, $passwd) {
        $ldapserver = $config['system']['ldap']['server'];
        $ldapport = isset($config['system']['ldap']['port']) ? $config['system']['ldap']['server'] : 389;
        $retval = false;
        
        $connection = ldap_connect($ldapserver, $ldapport)
            or die("Could not connect to $ldaphost");
            
        if ($connection) {
            $bind = ldap_bind($connection);
            
            if ($bind) {
                $basedn = $config['system']['ldap']['basedn'];
                $result = ldap_search($connection, $basedn, "uid={$username}");
                $info = ldap_get_entries($connection, $result);
                $userPassword = $info[0]['userPassword'];
                
                if ($userPassword == $passwd) {
                    $retval = true;
                } else {
                    $retval = false;
                }
            } // end if
        } // end if
        
        return $retval;
    }
}
?>