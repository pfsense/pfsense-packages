<?php
/* $Id$ */

/*
        squid_auth.inc
        part of pfSense (www.pfSense.com)

        Copyright (C) 2005 Michael Capp <michael.capp@gmail.com>
        All rights reserved.

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

function global_eval_auth_options() 
{
	global $config;
	conf_mount_rw();
	config_lock();
	
	switch ($config['installedpackages']['squidauth']['config'][0]['auth_method']) {
		case "none":
			dynamic_auth_content("pkg_edit");
			dynamic_no_auth();
			break;
		case "local_auth":
			dynamic_auth_content("pkg");
			/* create empty passwd file to prevent stat error with squid reload */
			touch ("/usr/local/etc/squid/advanced/ncsa/passwd");
			dynamic_local_auth();
			break;
		case "ldap_bind":
			dynamic_auth_content("pkg_edit");	
			dynamic_ldap_auth();
			break;
		case "domain_auth":
			$filecontents = file("/usr/local/pkg/squid_auth.xml");
			dynamic_auth_content("pkg_edit");
			dynamic_domain_auth();
			break;
		case "radius_auth":
			$filecontents = file("/usr/local/pkg/squid_auth.xml");
			dynamic_auth_content("pkg_edit");					
			dynamic_radius_auth();
			break;
		default:
			$filecontents = file("/usr/local/pkg/squid_auth.xml");
			dynamic_auth_content("pkg_edit");					
			dynamic_no_auth();
			break;
	}
	
	config_unlock();
	conf_mount_ro();
	
} /* end function global_eval_auth_options */
	
function dynamic_no_auth() {
	global $config;
	conf_mount_rw();
	$fout = fopen("/usr/local/pkg/squid_extauth.xml", "w");
	fwrite($fout, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
	fwrite($fout, "<packagegui>\n");
	fwrite($fout, "	<name>squidextnoauth</name>\n");
	fwrite($fout, "	<title>Services: Proxy Server -> Extended Authentication Settings</title>\n");
	fwrite($fout, "	<configpath>installedpackages->package->squidextnoauth->configuration->settings</configpath>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<aftersaveredirect>/pkg_edit.php?xml=squid_extauth.xml&amp;id=0</aftersaveredirect>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<tabs>\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>General Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_ng.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");		
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Upstream Proxy</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_upstream.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Cache Mgmt</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_cache.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");			
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Network Access Control</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_nac.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Traffic Mgmt</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_traffic.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Auth Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_auth.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Extended Auth Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_extauth.xml&amp;id=0</url>\n");
	fwrite($fout, "			<active/>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	</tabs>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<fields>\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>No Authentication Defined</fielddescr>\n");
	fwrite($fout, "			<fieldname>no_auth</fieldname>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "	</fields>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<custom_add_php_command_late>\n");
	fwrite($fout, "		require_once(\"/usr/local/pkg/squid_ng.inc\");");
	fwrite($fout, "\n");
	fwrite($fout, "		global_write_squid_config();\n");
	fwrite($fout, "		mwexec(\"/usr/local/sbin/squid -k reconfigure\");\n");
	fwrite($fout, "	</custom_add_php_command_late>\n");
	fwrite($fout, "\n");
	fwrite($fout, "</packagegui>\n");
	fclose($fout);
	
	/* mount filesystem read-only */
	conf_mount_ro();
}		

function dynamic_local_auth() {
	global $config;
	conf_mount_rw();

	$fout = fopen("/usr/local/pkg/squid_extauth.xml", "w");
	
	fwrite($fout, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
	fwrite($fout, "\n");
	fwrite($fout, "<packagegui>\n");
	fwrite($fout, "	<name>squidextlocalauth</name>\n");
	fwrite($fout, "	<title>Services: Proxy Server -> Extended Auth Settings</title>\n");
	fwrite($fout, "	<version>2.5.10_4</version>\n");
	fwrite($fout, "	<configpath>installedpackages->package->squidextlocalauth->configuration->settings</configpath>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<files></files>\n");
	fwrite($fout, "	<menu></menu>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<aftersaveredirect>/pkg.php?xml=squid_extauth.xml&amp;id=0</aftersaveredirect>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<tabs>\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>General Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_ng.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Upstream Proxy</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_upstream.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Cache Mgmt</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_cache.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Network Access Control</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_nac.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Traffic Mgmt</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_traffic.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Auth Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_auth.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Extended Auth Settings</text>\n");
	fwrite($fout, "			<url>/pkg.php?xml=squid_extauth.xml&amp;id=0</url>\n");
	fwrite($fout, "			<active/>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	</tabs>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<adddeleteeditpagefields>\n");
	fwrite($fout, "			<columnitem>\n");
	fwrite($fout, "			<fielddescr>Username</fielddescr>\n");
	fwrite($fout, "			<fieldname>username</fieldname>\n");
	fwrite($fout, "		</columnitem>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<columnitem>\n");
	fwrite($fout, "			<fielddescr>Description</fielddescr>\n");
	fwrite($fout, "			<fieldname>description</fieldname>\n");
	fwrite($fout, "		</columnitem>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<columnitem>\n");
	fwrite($fout, "			<fielddescr>Restriction Group</fielddescr>\n");
	fwrite($fout, "			<fieldname>group</fieldname>\n");
	fwrite($fout, "		</columnitem>\n");
	fwrite($fout, "	</adddeleteeditpagefields>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<fields>\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Username</fielddescr>\n");
	fwrite($fout, "			<fieldname>username</fieldname>\n");
	fwrite($fout, "			<type>input</type>\n");
	fwrite($fout, "			<size>15</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Password</fielddescr>\n");
	fwrite($fout, "			<fieldname>password</fieldname>\n");
	fwrite($fout, "			<type>password</type>\n");
	fwrite($fout, "			<size>8</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Description (Optional)</fielddescr>\n");
	fwrite($fout, "			<fieldname>description</fieldname>\n");
	fwrite($fout, "			<type>input</type>\n");
	fwrite($fout, "			<size>30</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Restriction Group</fielddescr>\n");
	fwrite($fout, "			<fieldname>group</fieldname>\n");
	fwrite($fout, "			<type>select</type>\n");
	fwrite($fout, "			<options>\n");
	fwrite($fout, "				<option><name>Standard</name><value>Standard</value></option>\n");
	fwrite($fout, "				<option><name>Extended</name><value>Extended</value></option>\n");
	fwrite($fout, "			</options>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	</fields>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<custom_add_php_command_late>\n");
	fwrite($fout, "		require_once(\"/usr/local/pkg/squid_ng.inc\");\n");
	fwrite($fout, "\n");
	fwrite($fout, "		mod_htpasswd();\n");
	fwrite($fout, "		global_write_squid_config();\n");
	fwrite($fout, "		mwexec(\"/usr/local/sbin/squid -k reconfigure\");\n");
	fwrite($fout, "	</custom_add_php_command_late>\n");
	fwrite($fout, "\n");
	fwrite($fout, "</packagegui>\n");
		
	fclose($fout);
	
	/* mount filesystem read-only */
	conf_mount_ro();
}

function dynamic_ldap_auth() {
	global $config;
	conf_mount_rw();	

	$fout = fopen("/usr/local/pkg/squid_extauth.xml", "w");
	
	fwrite($fout, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
	fwrite($fout, "\n");
	fwrite($fout, "<packagegui>\n");
	fwrite($fout, "	<name>squidextldapauth</name>\n");
	fwrite($fout, "	<title>Services: Proxy Server -> Extended Auth Settings</title>\n");
	fwrite($fout, "	<version>2.5.11</version>\n");
	fwrite($fout, "	<configpath>installedpackages->package->squidextldapauth->configuration->settings</configpath>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<files></files>\n");
	fwrite($fout, "	<menu></menu>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<aftersaveredirect>/pkg_edit.php?xml=squid_extauth.xml&amp;id=0</aftersaveredirect>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<tabs>\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>General Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_ng.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Upstream Proxy</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_upstream.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");		
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Cache Mgmt</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_cache.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Network Access Control</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_nac.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Traffic Mgmt</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_traffic.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Auth Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_auth.xml&amp;id=0</url>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");		
	fwrite($fout, "		<tab>\n");
	fwrite($fout, "			<text>Extended Auth Settings</text>\n");
	fwrite($fout, "			<url>/pkg_edit.php?xml=squid_extauth.xml&amp;id=0</url>\n");
	fwrite($fout, "			<active/>\n");
	fwrite($fout, "		</tab>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	</tabs>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<fields>\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Base DN</fielddescr>\n");
	fwrite($fout, "			<fieldname>ldap_basedn</fieldname>\n");
	fwrite($fout, "			<description>This is the base where the LDAP search starts.  All subsequent organizational units (OUs)will be included.  Example: \"ou=users,o=company\" will search for users in and under the specified company.</description>\n");
	fwrite($fout, "			<type>input</type>\n");
	fwrite($fout, "			<size>50</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>LDAP Server</fielddescr>\n");
	fwrite($fout, "			<fieldname>ldap_server</fieldname>\n");
	fwrite($fout, "			<description>This is the LDAP server that the bind will be attempted against.</description>\n");
	fwrite($fout, "			<type>input</type>\n");
	fwrite($fout, "			<size>20</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>LDAP Type</fielddescr>\n");
	fwrite($fout, "			<fieldname>ldap_type</fieldname>\n");
	fwrite($fout, "			<description>This specifies the supported LDAP types.</description>\n");
	fwrite($fout, "			<type>select</type>\n");
	fwrite($fout, "			<options>\n");
	fwrite($fout, "				<option><name>Active Directory</name><value>active_directory</value></option>\n");
	fwrite($fout, "				<option><name>Novell eDirectory</name><value>novell_edirectory</value></option>\n");
	fwrite($fout, "				<option><name>LDAP v2</name><value>ldap_v2</value></option>\n");
	fwrite($fout, "				<option><name>LDAP v3</name><value>ldap_v3</value></option>\n");
	fwrite($fout, "			</options>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>LDAP Port</fielddescr>\n");
	fwrite($fout, "			<fieldname>ldap_port</fieldname>\n");
	fwrite($fout, "			<description>This is the port that LDAP bind will attempt on.  The default is \"389\".</description>\n");
	fwrite($fout, "			<type>input</type>\n");
	fwrite($fout, "			<size>5</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Bind DN Username</fielddescr>\n");
	fwrite($fout, "			<fieldname>bind_dn_username</fieldname>\n");
	fwrite($fout, "			<description>If \"anonymous bind\" is not supported, please specify the bind username that can access the Base DN hierarchy.</description>\n");
	fwrite($fout, "			<type>input</type>\n");
	fwrite($fout, "			<size>30</size>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "		<field>\n");
	fwrite($fout, "			<fielddescr>Bind DN Password</fielddescr>\n");
	fwrite($fout, "			<fieldname>bind_dn_password</fieldname>\n");
	fwrite($fout, "			<description>This is the associated password with the Bind DN Username previously specified.</description>\n");
	fwrite($fout, "			<type>password</type>\n");
	fwrite($fout, "		</field>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	</fields>\n");
	fwrite($fout, "\n");
	fwrite($fout, "	<custom_add_php_command_late>\n");
	fwrite($fout, "		require_once(\"/usr/local/pkg/squid_ng.inc\");\n");
	fwrite($fout, "\n");
	fwrite($fout, "		mod_htpasswd();\n");
	fwrite($fout, "\n");
	fwrite($fout, "		global_write_squid_config();\n");
	fwrite($fout, "		mwexec(\"/usr/local/sbin/squid -k reconfigure\");\n");
	fwrite($fout, "	</custom_add_php_command_late>\n");
	fwrite($fout, "\n");
	fwrite($fout, "</packagegui>\n");
	
	fclose($fout);
	
	/* mount filesystem read-only */
	conf_mount_ro();
}

/* dynamically re-writes all squid xml files to handle adddeletecolumnitems properly */
function dynamic_auth_content($pkgvar) {
	
	switch ($pkgvar) {
		case "pkg":
			if ($handle = opendir("/usr/local/pkg")) {
				while (($file = readdir($handle)) != false) {
					if (stristr($file, "squid_") && stristr($file, ".xml")) {
						$filecontents = file("/usr/local/pkg/" . $file);
						$fout = fopen("/usr/local/pkg/" . $file, "w");
						foreach($filecontents as $line) {
							if (stristr($line, "<url>/pkg_edit.php?xml=squid_extauth.xml&amp;id=0</url>")) {
								fwrite($fout, "			<url>/pkg.php?xml=squid_extauth.xml&amp;id=0</url>\n");
							} else {
								fwrite($fout, $line);
							}
						}
					}
				}
			}		
			break;
		
		case "pkg_edit":
			if ($handle = opendir("/usr/local/pkg")) {
				while (($file = readdir($handle)) != false) {
					if (stristr($file, "squid_") && stristr($file, ".xml")) {
						$filecontents = file("/usr/local/pkg/" . $file);
						$fout = fopen("/usr/local/pkg/" . $file,"w");
						foreach($filecontents as $line) {
							if (stristr($line, "<url>/pkg.php?xml=squid_extauth.xml&amp;id=0</url>")) {
								fwrite($fout, "			<url>/pkg_edit.php?xml=squid_extauth.xml&amp;id=0</url>\n");
							} else {
								fwrite($fout, $line);
							}
						}
					}
				}
			}
			break;
	}
			
} /* end function dynamic_auth_content */
?>