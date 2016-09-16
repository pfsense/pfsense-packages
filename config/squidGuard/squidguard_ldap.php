#!/usr/local/bin/php
<?php
/* ====================================================================================== */
/*
 * based on http://samjlevy.com/2011/02/using-php-and-ldap-to-list-of-members-of-an-active-directory-group/
 squidguard_ldap.xml
 part of pfSense (https://www.pfSense.org/)
 Copyright (C) 2012-2013 ccesario
 Copyright (C) 2012-2016 Marcello Coutinho
 Copyright (C) 2016 ESF, LLC
 All rights reserved.
 */
/* ====================================================================================== */
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
/* ====================================================================================== */

// ldapsearch -x -h 192.168.11.1 -p 389 -b OU=Internet,DC=domain,DC=local -D CN=Proxyauth,OU=PROXY,DC=domain,DC=local -w PASS

require_once("/etc/inc/util.inc");
require_once("/etc/inc/functions.inc");
require_once("/etc/inc/pkg-utils.inc");
require_once("/etc/inc/globals.inc");

if (is_array($config['installedpackages']['squidguardgeneral']['config'])) {
	$sgg=$config['installedpackages']['squidguardgeneral']['config'][0];
	if ( $sgg['squidguard_enable'] == "on" && 
		 $sgg['ldap_enable'] == "on" && $sgg['ldapfetch'] == "batch" ) {
		
		if ($sg['striprealm'] == "on") {
			$user_mask = "USER";
		} else {
			$user_mask = "DOMAIN\USER";
		}
		
		if ($sgg['ldapbinddn'] != "") {
			$user_bind = $sgg['ldapbinddn'];
		} else {
			print "SquidGuard ldapbinddn not set\n";
			exit;
		}
		
		if ($sgg['ldapbindpass'] != "") {
			$password = $sgg['ldapbindpass'];
		} else {
			print "SquidGuard ldap pass not set\n";
			exit;
		}

		if ($sgg['ldapbatchou'] != "") {
			$ldap_dn = $sgg['ldapbatchou'];
		} else {
			print "SquidGuard ldap pass not set.\n";
			exit;
		}

		if ($sgg['ldapbatchserver'] != "") {
			$ldap_host = $sgg['ldapbatchserver'];
		} else {
			print "SquidGuard ldap host not set.\n";
			exit;
		}
		
	} else {
		print "Squidguard ldap not in batch mode\n";
		exit;
	}
} else {
	print "Squidguard config not found\n";
	exit;
}
#mount filesystem writable
conf_mount_rw();
function explode_dn($dn, $with_attributes=0) {
	$result = ldap_explode_dn($dn, $with_attributes);
	if (is_array($result)) {
		foreach($result as $key => $value) {
			$result[$key] = $value;
		}
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
	if (is_array($member_list[0])) {
		foreach($member_list[0] as $list) {
			if (is_array($list)) {
				foreach($list as $member) {
					$ldap_dn_user = preg_replace('/^cn=([^,]+),/i','',$member);
					$member_dn = explode_dn($member);
					if (!empty($member_dn[0])) {
						$member_cn = str_replace("CN=","",$member_dn[0]);
						$member_search = ldap_search($ldap, $ldap_dn_user, "(CN=" . $member_cn . ")");
						$member_details = ldap_get_entries($ldap, $member_search);
						// If group has another group as member (only 1 level)
						if(is_array($member_details[0]['member'])) {
							foreach($member_details[0]['member'] as $sub_member) {
								$sub_ldap_dn_user = preg_replace('/^cn=([^,]+),/i','',$sub_member);
								$sub_member_dn = explode_dn($sub_member);
								if (!empty($sub_member_dn[0])) {
									$sub_member_cn = str_replace("CN=","",$sub_member_dn[0]);
									$sub_member_search = ldap_search($ldap, $sub_ldap_dn_user, "(CN=" . $sub_member_cn . ")");
									$sub_member_details = ldap_get_entries($ldap, $sub_member_search);
									$group_member_details[] = array($sub_member_details[0]['samaccountname'][0]);
								}
							}
						} else {
						$group_member_details[] = array($member_details[0]['samaccountname'][0]);
						}
					}
				}
			}
		}
	}
	ldap_close($ldap);
	return $group_member_details;
}
//Log info
log_error("Running squidGuard LDAP batch fetch");
// Read Pfsense config
global $config,$g;
$id=0;
$apply_config=0;
if (is_array($config['installedpackages']['squidguardacl']['config'])) {
	foreach($config['installedpackages']['squidguardacl']['config'] as $group) {
		$members="";
		print "SquidGuard acl name:{$group['name']}\n";
		$group_list=explode(" ",$group['source']);
		if (is_array($group_list)) {
			foreach ($group_list as $group_OU) {
				if (preg_match ("/\[(\S+)\]/",$group_OU,$gm)){
					echo  "Group : " . $gm[1]."\n";
					$result = get_ldap_members($gm[1],$user_bind,$password);
					asort($result);
					foreach($result as $key => $value) {
						if (preg_match ("/\w+/",$value[0])) {
							$members .= strtolower($value[0])."\n";
							//user_mask was used here
							}
					}
				}
		 	}
		}
		if (!empty($members)) {
			$dir="/var/db/squidGuard.ldap";
			if (!is_dir($dir)) {
				mkdir("$dir",0775,true);
				chown($dir, "proxy");
			}
			$acl_file="{$dir}/{$group['name']}";
			if (file_exists($acl_file)) {
				$current_acl_members = md5(file_get_contents($acl_file));
			} else {
				$current_acl_members = "";
			}
			if($current_acl_members != md5($members)) {
				$msg_md5="md5 mismatch for {$group['name']} $current_acl_members <> ". md5($members);
				log_error($msg_md5);
				print "$msg_md5\n";
				file_put_contents($acl_file,$members,LOCK_EX);
				$apply_config++;
			}
		}
		echo "\t --> Members : " . $members . "\n\n";
		$id++;
	}
}
if ($apply_config > 0) {
	log_error("squidGuard LDAP sync: user group list from LDAP has changed, reloading squid config...");
	print "user list from LDAP is different from current group, applying new configuration...";
	//write_config();
	include("/usr/local/pkg/squidguard.inc");
	// teste a faster reload (squid -k reconfigure or killall -HUP squid)
	if (function_exists('squid_resync')) {
		squid_resync();
	}
	print "done\n";
}
#mount filesystem read-only
conf_mount_ro();
?>
