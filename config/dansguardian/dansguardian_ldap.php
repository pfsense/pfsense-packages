#!/usr/local/bin/php -f 
<?php

// based on http://samjlevy.com/2011/02/using-php-and-ldap-to-list-of-members-of-an-active-directory-group/
// pfsense integration by marcelloc and ccesario
/* $Id$ */
/* ========================================================================== */
/*
    dansguardian_ldap.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 Marcello Coutinho
    
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

require_once("/etc/inc/util.inc");
require_once("/etc/inc/functions.inc");
require_once("/etc/inc/pkg-utils.inc");
require_once("/etc/inc/globals.inc");

function explode_dn($dn, $with_attributes=0)
{
    $result = ldap_explode_dn($dn, $with_attributes);
    if (is_array($result))
      foreach($result as $key => $value) {
         $result[$key] = $value;
     }
    return $result;
}

function get_ldap_members($group,$user,$password) {
	global $ldap_host;
	global $ldap_dn;
	$LDAPFieldsToFind = array("member");
	$ldap = ldap_connect($ldap_host) or die("Could not connect to LDAP");

	// OPTIONS TO AD
	ldap_set_option($ldap, LDAP_OPT_PROTOCOL_VERSION,3);
	ldap_set_option($ldap, LDAP_OPT_REFERRALS,0);

	ldap_bind($ldap, $user, $password) or die("Could not bind to LDAP");

	$results = ldap_search($ldap,$ldap_dn,"cn=" . $group,$LDAPFieldsToFind);
	
	$member_list = ldap_get_entries($ldap, $results);
	$group_member_details = array();
	if (is_array($member_list[0]))
	  foreach($member_list[0] as $list)
		if (is_array($list)) 
			foreach($list as $member) {
				$member_dn = explode_dn($member);
				$member_cn = str_replace("CN=","",$member_dn[0]);
				$member_search = ldap_search($ldap, $ldap_dn, "(CN=" . $member_cn . ")");
				$member_details = ldap_get_entries($ldap, $member_search);
				$group_member_details[] = array($member_details[0]['samaccountname'][0],
												$member_details[0]['displayname'][0]);
			}
	ldap_close($ldap);
	array_shift($group_member_details);
	return $group_member_details;
	ldap_unbind($ldap);
}

// Read Pfsense config 
global $config,$g;

#mount filesystem writable
conf_mount_rw();

$id=0;
$apply_config=0;
if (is_array($config['installedpackages']['dansguardiangroups']['config']))
	foreach($config['installedpackages']['dansguardiangroups']['config'] as $group) {
		#ignore default group
		if ($id > 0)
			if ($argv[1] == "" || $argv[1] == $group['name']){
	   		$members="";
	   		$ldap_servers= explode (',',$group['ldap']);
	   		echo  "Group : " . $group['name']."\n";
	   		if (is_array($config['installedpackages']['dansguardianldap']['config']))
	   			foreach ($config['installedpackages']['dansguardianldap']['config'] as $server){
		   			if (in_array($server['dc'],$ldap_servers)){
		   				$ldap_dn = $server['dn'];
		   				$ldap_host=$server['dc'];
		   				$mask=(empty($server['mask'])?"USER":$server['mask']);
				   		$result = get_ldap_members($group['name'],$server['username'].','.$server['dn'],$server['password']);
		   				foreach($result as $key => $value) {
			    			if (preg_match ("/\w+/",$value[0])){
			    				#var_dump($value);
			    				$name= preg_replace('/[^(\x20-\x7F)]*/','', $value[1]);
			    				$pattern[0]="/USER/";
			    				$pattern[1]="/,/";
			    				$pattern[2]="/NAME/";
			    				$replace[0]=$value[0];
			    				$replace[1]="\n";
			    				$replace[2]="$name";
		    	  				$members .= preg_replace($pattern,$replace,$mask)."\n";
			    				}
		   					}
		   			}
	   			}
		   	if (!empty($members)){
		   	$import_users = explode("\n", $members);
			asort($import_users);
			$members=base64_encode(implode("\n", $import_users));
		   	if($config['installedpackages']['dansguardianusers']['config'][0][strtolower($group['name'])] != $members){
		   		$config['installedpackages']['dansguardianusers']['config'][0][strtolower($group['name'])] = $members;
		   		$apply_config++;
		   		}
		   	}
			}
		$id++;			
	}
if ($apply_config > 0){
	print "user list from LDAP is different from current group, applying new configuration...";
	write_config();
	include("/usr/local/pkg/dansguardian.inc");
	sync_package_dansguardian();
	print "done\n";
}

#mount filesystem read-only
conf_mount_ro();

?>