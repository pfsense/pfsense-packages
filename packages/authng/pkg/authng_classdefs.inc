<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng_classdefs.inc
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

class SingletonInterface extends Object {
        function __construct() {
                // Perform object initialization here.
        }

        function &__getInstanceImp($name) {
                static $instances = array();
                if (!isset($instances[$name])) {
                    $instances[$name] = new $name(); // No changes necessary here.
                }
                return $instances[$name];
        }

        function &getInstance() {
                trigger_error('SingletonInterface::getInstance() needs to be overridden in a subclass.', E_USER_ERROR);
        }
}

class AuthMethodFactory extends SingletonInterface {
        function __construct() {
                // Perform object initialization here.
                parent::__construct();
        }

        function &getInstance() {
                return parent::__getInstanceImp('AuthMethodFactory');
        }
        
        function &getBackendByName($name) {
            $result = null;
            
            /* Each name links to an entry in config.xml
             * Example: <auth_method>session</auth_method>
             */
            switch ($name) {
                case "htpasswd":
                    $result = new HtpasswdBackend();
                    break;
                case "pam":
                    $result = new PamBackend();
                    break;
                case "radius":
                    $result = new RadiusBackend();
                    break;
                case "passwd":
                    $result = new PasswdBackend();
                    break;
                case "ldap":
                    $result = new LdapBackend();
                    break;
                default:
            }
            
            return $result;
        }
}

class BackendFactory extends SingletonInterface {
        function __construct() {
                // Perform object initialization here.
                parent::__construct();
        }

        function &getInstance() {
                return parent::__getInstanceImp('BackendFactory');
        }
        
        function &getAuthMethodByName($name) {
            $result = null;
            
            /* Each name links to an entry in config.xml
             * Example: <backing_method>htpasswd</backing_method>
             */
            switch ($name) {
                case "session":
                    $result = new SessionAuthMethod();
                    break;
                case "basic":
                    $result = new BasicAuthMethod();
                    break;
                default:
            }
            
            return $result;
        }
}

class AuthngAuxiliary {
        /* ========================================================================== */
        /* ==                         Auxiliary Functions                          == */
        /* ========================================================================== */
        function &getSystemAdminNames() {
              global $config, $g, $userindex;
              $adminUsers = array();

              if (is_array($config['system']['user'])) {
                    foreach($config['system']['user'] as $user){
                          if (isSystemAdmin($user['name'])) {
                            $adminUsers[] = $user['name'];
                          }
                    } // end foreach
              } // end if

          return $adminUsers;
        }  // end function
        
        function assignUID($username = "") {
              global $userindex, $config, $g;

              if ($username == "") { return; }

              $nextuid = $config['system']['nextuid'];
              $user =& $config['system']['user'][$userindex[$username]];

              if (empty($user['uid'])) {
                    $user['uid'] = $nextuid;
                    $nextuid++;
                    $config['system']['nextuid'] = $nextuid;

                    write_config();

                    return $user;
              } // end if
        }  // end function
}

class AuthngPrivilege {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $id;
        var $name;
        var $description;

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function AuthngPrivilege() {
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        function getId() {
                return $this->id;
        }

        function setId($id) {
                $this->id = $id;
        }

        function getName() {
                return $this->name;
        }

        function setName($name) {
                $this->name = $name;
        }

        function getDescription() {
                return $this->description;
        }

        function setDescription($desc) {
                $this->description = $desc;
        }
}

class SystemPrivileges {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $privileges = array();

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function SystemPrivileges() {
                $newPriv = new Privilege();
                $newPriv->setId("lockwc");
                $newPriv->setName("Lock webConfigurator");
                $newPriv->setDescription("Indicates whether this user will lock access to the webConfigurator for other users.");

                $this->privileges[$newPriv->getId()] = $newPriv;

                $newPriv = new Privilege();
                $newPriv->setId("lock-ipages");
                $newPriv->setName("Lock individual pages");
                $newPriv->setDescription("Indicates whether this user will lock individual " .
                                         "HTML pages after having accessed a particular page" .
                                         "(the lock will be freed if the user leaves or " .
                                         "saves the page form).");

                $this->privileges[$newPriv->getId()] = $newPriv;

                $newPriv = new Privilege();
                $newPriv->setId("hasshell");
                $newPriv->setName("Has shell access");
                $newPriv->setDescription("Indicates whether this user is able to login for " .
                                         "example via SSH.");

                $this->privileges[$newPriv->getId()] = $newPriv;

                $newPriv = new Privilege();
                $newPriv->setId("copyfiles");
                $newPriv->setName("Is allowed to copy files");
                $newPriv->setDescription("Indicates whether this user is allowed to copy files " .
                                         "onto the {$g['product_name']} appliance via SCP/SFTP. " .
                                         "If you are going to use this privilege, you must install " .
                                         "scponly on the appliance (Hint: pkg_add -r scponly).");

                $this->privileges[$newPriv->getId()] = $newPriv;

                $newPriv = new Privilege();
                $newPriv->setId("isroot");
                $newPriv->setName("Is root user");
                $newPriv->setDescription("This user is associated with the UNIX root user " .
                                         "(you should associate this privilege only with one " .
                                         "single user).");

                $this->privileges[$newPriv->getId()] = $newPriv;
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        function getPrivileges() {
                return $this->privileges;
        }

        function setPrivileges($privs) {
                $this->privileges = $privs;
        }

        function getPrivilegeById($id) {
                return $this->privileges[$id];
        }

        function setPrivilegeById($privilege, $id) {
                return $this->privileges[$id] = $privilege;
        }
}

class AuthngUser {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $name;
        var $fullname;
        var $scope;
        var $groupname;
        var $password;
        var $uid;
        var $systemAdmin = false;
        var $unixRoot = false;
        var $privileges = array();

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function AuthngUser() {
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */
        
        function isSystemAdmin() {
                return $this->systemAdmin;
        }
        
        function setIsSystemAdmin($flag = false) {
                $this->systemAdmin = $flag;
        }
        
        function isUNIXRoot() {
                return $this->unixRoot;
        }
        
        function setIsUNIXRoot($flag = false) {
                $this->unixRoot = $flag;
        }

        function getName() {
                return $this->name;
        }

        function setName($name) {
                $this->name = $name;
        }

        function getFullname() {
                return $this->fullname;
        }

        function setFullname($name) {
                $this->fullname = $name;
        }

        function getScope() {
                return $this->scope;
        }

        function setScope($scope) {
                $this->scope = $scope;
        }

        function getGroupname() {
                return $this->groupname;
        }

        function setGroupname($name) {
                $this->groupname = $name;
        }

        function getPassword() {
                return $this->password;
        }

        function setPassword($pwd) {
                $this->password = $pwd;
        }

        function getUid() {
                return $this->uid;
        }

        function setUid($uid) {
                $this->uid = $uid;
        }

        function getPrivileges() {
                return $this->privileges;
        }

        function setPrivileges($privs) {
                $this->privileges = $privs;
        }

        function addPrivilege($priv) {
                $this->privileges[] = $priv;
        }
}

class AuthngGroup {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $name;
        var $description;
        var $scope;
        var $pages = array();
        var $home;
        var $gid;

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function AuthngGroup() {
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        function getName() {
                return $this->name;
        }

        function setName($name) {
                $this->name = $name;
        }

        function getDescription() {
                return $this->description;
        }

        function setDescription($desc) {
                $this->description = $desc;
        }

        function getScope() {
                return $this->scope;
        }

        function setScope($scope) {
                $this->scope = $scope;
        }

        function getPages() {
                return $this->pages;
        }

        function setPages($pages) {
                $this->pages = $pages;
        }
        function getHome() {
                return $this->home;
        }

        function setHome($home) {
                $this->home = $home;
        }

        function getGid() {
                return $this->gid;
        }

        function setGid($gid) {
                $this->gid = $gid;
        }

        function addPage($page) {
                $this->pages[] = $page;
        }
}

?>