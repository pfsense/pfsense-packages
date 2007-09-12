<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng_peers.inc
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

require_once("authng_classdefs.inc");

class PeerFactory extends SingletonInterface {
        function __construct() {
                // Perform object initialization here.
                parent::__construct();
        }

        function &getInstance() {
                return parent::__getInstanceImp('PeerFactory');
        }
        
        function &getGroupPeerByPrincipalStore($store) {
            $result = null;
            
            /* Each name links to an entry in config.xml
             * Example: <principal_store>xml</principal_store>
             */
            switch ($store) {
                case "xml":
                    $result = new XMLGroupPeer();
                    break;
                case "ldap":
                    trigger_error('PeerFactory::getGroupPeerByPrincipal() LDAP peer type is not supported.', E_USER_ERROR);
                    break;
                case "db":
                    trigger_error('PeerFactory::getGroupPeerByPrincipal() DB peer type is not supported.', E_USER_ERROR);
                    break;
                default:
            }
            
            return $result;
        }
        
        function &getUserPeerByPrincipalStore($store) {
            $result = null;
            
            /* Each name links to an entry in config.xml
             * Example: <principal_store>xml</principal_store>
             */
            switch ($store) {
                case "xml":
                    $result = new XMLUserPeer();
                    break;
                case "ldap":
                    trigger_error('PeerFactory::getGroupPeerByPrincipal() LDAP peer type is not supported.', E_USER_ERROR);
                    break;
                case "db":
                    trigger_error('PeerFactory::getGroupPeerByPrincipal() DB peer type is not supported.', E_USER_ERROR);
                    break;
                default:
            }
            
            return $result;
        }
}

/**
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 * @abstract
 */
class AbstractPrivilegePeer {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $privilege_index;
        var $privileges;
        var $userPeer;

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function AbstractPrivilegePeer() {
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */
        
        function setUserPeer($peer) {
            $this->userPeer = $peer;
        }
        
        function getUserPeer() {
            return $this->userPeer;
        }

        /**
         * @return mixed int array of priv indexes
         */
        function getPrivilegeIndex() {
                return $this->privilege_index;
        }

        /**
         * @param string a priv name
         * @return int the index that corresponds to a username
         */
        function getPrivilegeIndexByID($id) {
                return $this->privilege_index[$id];
        }

        /**
         * @param int an index
         * @return mixed an instance of AuthngPrivilege
         */
        function getPrivilegeByIndex($index) {
                return $this->privileges[$index];
        }
}

/**
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 * @abstract
 */
class AbstractUserPeer {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $user_index;
        var $users;

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function AbstractUserPeer() {
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        /**
         * @return mixed int array of user indexes
         */
        function getUserIndex() {
                return $this->user_index;
        }

        /**
         * @param string a username
         * @return int the index that corresponds to a username
         */
        function getUserIndexByName($username) {
                return $this->user_index[$username];
        }

        /**
         * @param int an index
         * @return mixed an instance of AuthngUser
         */
        function getUserByIndex($index) {
                return $this->users[$index];
        }
        
        function getUserByName($username) {
                return $this->users[$username];
        }
        
        function isSystemAdmin($username) {
                $result = false;
                $user = $this->getUserByName($username);
             
                if ($user) {
                    $result = $user->isSystemAdmin();
                }
                
                return $result;
        }
}

/**
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 * @abstract
 */
class AbstractGroupPeer {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        var $group_index;
        var $groups;

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function AbstractGroupPeer() {
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        function getGroupIndex() {
                return $this->group_index;
        }

        function getGroupIndexByName($groupname) {
                return $this->group_index[$groupname];
        }

        function getGroupByIndex($index) {
                return $this->groups[$index];
        }
        
        function getGroupByName($groupname) {
                return $this->groups[$groupname];
        }
        
        function getGroupHomePage($groupname) {
                $result = false;
                $group = $this->getGroupByName($groupname);
             
                if ($group) {
                    $result = $group->getHome();
                }
                
                return $result;
        }
}

/**
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 */
class XMLPrivilegePeer extends AbstractPrivilegePeer {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function XMLPrivilegePeer($userPeer) {
                global $g, $config;
                
                parent::AbstractPrivilegePeer();
                
                $this->setUserPeer($peer);
                
                foreach ($peer->users as $userent) {
                    foreach ($userent->getPrivileges() as $privent) {
                        $this->privileges[$userent->getName()] = $privent;
                    }
                }
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        /* ========================================================================== */
        /* ==                            Helper Methods                            == */
        /* ========================================================================== */

        function addPrivilegeFromEnt(&$ent) {
                $newPrivilege = new AuthngUser();
                $newPrivilege->setId($ent['id']);
                $newPrivilege->setName($ent['name']);
                $newPrivilege->setDescription($ent['description']);
                $newPrivilege->setPassword($ent['password']);
                $newPrivilege->setUid($ent['uid']);

                $this->privileges[] = $newPrivilege;
        }

        function setPrivilegeID($id, $name, $username) {
                $userid = getPrivilegeIndexByName($username);
                $user = $config['system']['user'][$userid];
        }
        
        function setFullName($id, $name) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['fullname'] = $name;
        }
        
        function setGroupName($id, $name) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['groupname'] = $name;
        }
        
        function setPassword($id, $pwd) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['password'] = $pwd;
        }
        
        function setUid($id, $uid) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['uid'] = $uid;
        }
}

/**
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 */
class XMLUserPeer extends AbstractUserPeer {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function XMLUserPeer() {
                global $g, $config;
                
                parent::AbstractUserPeer();

                if (isset($config['system']['user'])) {
                        $i = 0;

                        foreach($config['system']['user'] as $userent) {
                                $this->user_index[$userent['name']] = $i;
                                $this->addUserFromEnt($userent);
                                $i++;
                        }
                }                                
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        /* ========================================================================== */
        /* ==                            Helper Methods                            == */
        /* ========================================================================== */

        function addUserFromEnt(&$ent) {
            print "HURTZ";
                $newUser = new AuthngUser();
                $newUser->setName($ent['name']);
                $newUser->setFullname($ent['fullname']);
                $newUser->setGroupname($ent['groupname']);
                $newUser->setPassword($ent['password']);
                $newUser->setUid($ent['uid']);

                if ($ent['priv'] && is_array($ent['priv'])) {
                        foreach ($ent['priv'] as $privent) {
                                $newPrivilege = new Privilege();
                                $newPrivilege->setId($privent['id']);
                                $newPrivilege->setName($privent['name']);
                                $newPrivilege->setDescription($privent['description']);
                                
                                $newUser->addPrivilege($newPrivilege);
                        }
                }

                $this->users["${ent['name']}"] = $newUser;
        }

        function setUserName($id, $name) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['name'] = $name;
        }
        
        function setFullName($id, $name) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['fullname'] = $name;
        }
        
        function setGroupName($id, $name) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['groupname'] = $name;
        }
        
        function setPassword($id, $pwd) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['password'] = $pwd;
        }
        
        function setUid($id, $uid) {
                $userid = getUserIndexByName($id);
                $config['system']['user'][$userid]['uid'] = $uid;
        }
}

/**
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 */
class XMLGroupPeer extends AbstractGroupPeer {
        /* ========================================================================== */
        /* ==                          Class Members                               == */
        /* ========================================================================== */

        /* ========================================================================== */
        /* ==                            Constructor                               == */
        /* ========================================================================== */

        function XMLGroupPeer() {
                global $g, $config;
                
                parent::AbstractGroupPeer();

                if (isset($config['system']['group'])) {
                        $i = 0;

                        foreach($config['system']['group'] as $groupent) {
                                $this->group_index[$groupent['name']] = $i;
                                $i++;
                        }
                }
        }

        /* ========================================================================== */
        /* ==                              Accessors                               == */
        /* ========================================================================== */

        /* ========================================================================== */
        /* ==                            Helper Methods                            == */
        /* ========================================================================== */

        function addGroupFromEnt(&$ent) {
                $newGoup = new AuthngGroup();
                $newGoup->setName($ent['name']);
                $newGoup->setDescription($ent['description']);
                $newGoup->setScope($ent['scope']);
                $newGoup->setHome($ent['home']);
                $newGoup->setGid($ent['gid']);

                if ($ent['pages'] && is_array($ent['gid'])) {
                        foreach ($ent['pages'] as $pageent) {
                                $newGoup->addPage($pageent);
                        }
                }

                $this->groups["${ent['name']}"] = $newGoup;
        }

        function setGroupName($id, $name) {
                $groupid = getGroupIndexByName($id);
                $config['system']['group'][$groupid]['name'] = $name;
        }

        function setGroupDescription($id, $desc) {
                $groupid = getGroupIndexByName($id);
                $config['system']['group'][$groupid]['description'] = $desc;
        }

        function setGroupScope($id, $scope) {
                $groupid = getGroupIndexByName($id);
                $config['system']['group'][$groupid]['scope'] = $scope;
        }

        function setGroupHome($id, $home) {
                $groupid = getGroupIndexByName($id);
                $config['system']['group'][$groupid]['home'] = $home;
        }

        function setGroupGid($id, $gid) {
                $groupid = getGroupIndexByName($id);
                $config['system']['group'][$groupid]['gid'] = $gid;
        }

        function addPageToGroup($id, $page) {
                $groupid = getGroupIndexByName($id);
                $config['system']['group'][$groupid]['pages'][] = $page;
        }
}
?>
