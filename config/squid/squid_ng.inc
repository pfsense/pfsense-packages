<?php
/* $Id$ */

/*
        squid_ng.inc
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

if(!function_exists("filter_configure")) 
	require_once("filter.inc");

function global_write_squid_config()
{
	global $config;
	conf_mount_rw();
	config_lock();

    /* define squid configuration file in variable for replace function */
    $squidconfig = "/usr/local/etc/squid/squid.conf";

    /* squid.xml values */
    $active_interface      = $config['installedpackages']['squid']['config'][0]['active_interface'];
    $transparent_proxy     = $config['installedpackages']['squid']['config'][0]['transparent_proxy'];
    $log_enabled           = $config['installedpackages']['squid']['config'][0]['log_enabled'];
    $urlfilter_enable      = $config['installedpackages']['squid']['config'][0]['urlfilter_enable'];
    $accesslog_disabled    = $config['installedpackages']['squid']['config'][0]['accesslog_disabled'];
    $log_query_terms       = $config['installedpackages']['squid']['config'][0]['log_query_terms'];
    $log_user_agents       = $config['installedpackages']['squid']['config'][0]['log_user_agents'];
    $proxy_port            = $config['installedpackages']['squid']['config'][0]['proxy_port'];
    $visible_hostname      = $config['installedpackages']['squid']['config'][0]['visible_hostname'];
    $cache_admin_email     = $config['installedpackages']['squid']['config'][0]['cache_admin_email'];
    $error_language 	   = $config['installedpackages']['squid']['config'][0]['error_language'];
    $cachemgr_enabled	   = $config['installedpackages']['squid']['config'][0]['cachemgr_enabled'];

    /* squid_upstream.xml values */
    $proxy_forwarding	   = $config['installedpackages']['squidupstream']['config'][0]['proxy_forwarding'];
    $client_ip_forwarding  = $config['installedpackages']['squidupstream']['config'][0]['client_ip_forwarding'];
    $user_forwarding 	   = $config['installedpackages']['squidupstream']['config'][0]['user_forwarding'];
    $upstream_proxy 	   = $config['installedpackages']['squidupstream']['config'][0]['upstream_proxy'];
    $upstream_proxy_port   = $config['installedpackages']['squidupstream']['config'][0]['upstream_proxy_port'];
    $upstream_username     = $config['installedpackages']['squidupstream']['config'][0]['upstream_username'];
    $upstream_password 	   = $config['installedpackages']['squidupstream']['config'][0]['upstream_psasword'];

    /* squid_cache.xml values */
    $memory_cache_size     = $config['installedpackages']['squidcache']['config'][0]['memory_cache_size'];
    $harddisk_cache_size   = $config['installedpackages']['squidcache']['config'][0]['harddisk_cache_size'];
    $minimum_object_size   = $config['installedpackages']['squidcache']['config'][0]['minimum_object_size'];
    $maximum_object_size   = $config['installedpackages']['squidcache']['config'][0]['maximum_object_size'];
    $level_subdirs 		   = $config['installedpackages']['squidcache']['config'][0]['level_subdirs'];
    $memory_replacement    = $config['installedpackages']['squidcache']['config'][0]['memory_replacement'];
    $cache_replacement 	   = $config['installedpackages']['squidcache']['config'][0]['cache_replacement'];
    $domain 			   = $config['installedpackages']['squidcache']['config'][0]['domain'];
    $enable_offline 	   = $config['installedpackages']['squidcache']['config'][0]['enable_offline'];

    /* squid_nac.xml values */
    $allowed_subnets	   = $config['installedpackages']['squidnac']['config'][0]['allowed_subnets'];
    $unrestricted_ip_addr  = $config['installedpackages']['squidnac']['config'][0]['unrestricted_ip_address'];
    $unrestricted_mac_addr = $config['installedpackages']['squidnac']['config'][0]['unrestricted_mac_addresses'];
    $banned_ip_addr		   = $config['installedpackages']['squidnac']['config'][0]['banned_ip_addresses'];
    $banned_mac_addr 	   = $config['installedpackages']['squidnac']['config'][0]['banned_mac_addresses'];
    $override_hosts		   = $config['installedpackages']['squidnac']['config'][0]['override_hosts'];

    /* squid_traffic.xml values */
    $max_download_size     = $config['installedpackages']['squidtraffic']['config'][0]['max_download_size'];
    $max_upload_size       = $config['installedpackages']['squidtraffic']['config'][0]['max_upload_size'];
    $dl_overall            = $config['installedpackages']['squidtraffic']['config'][0]['dl_overall'];
    $dl_per_host           = $config['installedpackages']['squidtraffic']['config'][0]['dl_per_host'];
    $throttle_binary_files = $config['installedpackages']['squidtraffic']['config'][0]['throttle_binary_files'];
    $throttle_cd_images    = $config['installedpackages']['squidtraffic']['config'][0]['throttle_cd_images'];
    $throttle_multimedia   = $config['installedpackages']['squidtraffic']['config'][0]['throttle_multimedia'];

    /* squid_auth.xml values */
	$auth_method           = $config['installedpackages']['squidauth']['config'][0]['auth_method'];
	$auth_processes        = $config['installedpackages']['squidauth']['config'][0]['auth_processes'];
	$auth_cache_ttl        = $config['installedpackages']['squidauth']['config'][0]['auth_cache_ttl'];
	$limit_ip_addr         = $config['installedpackages']['squidauth']['config'][0]['limit_ip_addr'];
	$user_ip_cache_ttl     = $config['installedpackages']['squidauth']['config'][0]['user_ip_cache_ttl'];
	$req_unrestricted_auth = $config['installedpackages']['squidauth']['config'][0]['req_unrestricted_auth'];
	$auth_realm_prompt     = $config['installedpackages']['squidauth']['config'][0]['auth_realm_prompt'];
	$no_domain_auth        = $config['installedpackages']['squidauth']['config'][0]['no_domain_auth'];
	$min_pass_length       = $config['installedpackages']['squidauth']['config'][0]['min_pass_length'];
	$bypass_extended       = $config['installedpackages']['squidauth']['config'][0]['bypass_extended'];

	/* squid_extauth.xml (ldap) values */
	$ldap_basedn		   = $config['installedpackages']['squidextldapauth']['config'][0]['ldap_basedn'];
	$ldap_server		   = $config['installedpackages']['squidextldapauth']['config'][0]['ldap_server'];
	$ldap_type			   = $config['installedpackages']['squidextldapauth']['config'][0]['ldap_type'];
	$ldap_port			   = $config['installedpackages']['squidextldapauth']['config'][0]['ldap_port'];
	$bind_dn_username	   = $config['installedpackages']['squidextldapauth']['config'][0]['bind_dn_username'];
	$bind_dn_password	   = $config['installedpackages']['squidextldapauth']['config'][0]['bind_dn_password'];

	/* squid_extauth.xml (radius) values */
	$radius_server		   = $config['installedpackages']['squidextradiusauth']['config'][0]['radius_server'];
	$radius_port		   = $config['installedpackages']['squidextradiusauth']['config'][0]['radius_port'];
	$radius_identifier	   = $config['installedpackages']['squidextradiusauth']['config'][0]['radius_identifier'];
	$radius_secret		   = $config['installedpackages']['squidextradiusauth']['config'][0]['radius_secret'];

	/* static variable assignments for directory mapping */
	$acldir    = "/usr/local/etc/squid/advanced/acls";
	$ncsadir   = "/usr/local/etc/squid/advanced/ncsa";
	$ntlmdir   = "/usr/local/etc/squid/advanced/ntlm";
	$radiusdir = "/usr/local/etc/squid/advanced/radius";

    $fout = fopen($squidconfig, "w");

	$config_array = array('shutdown_lifetime 5 seconds' . "\n\n");

        if (isset($cachemgr_enabled) && ($cachemgr_enabled == "on")) {
		mwexec("cp /usr/local/libexec/squid/cachemgr.cgi /usr/local/www/cachemgr.cgi");
		mwexec("chmod a+rx /usr/local/www/cachemgr.cgi");
        } else {
		mwexec("rm -f /usr/local/www/cachemgr.cgi");
	}
        unset($cachemgr_enabled);

	if (!isset($icp_port) or ($icp_port == "")) {
		$icp_port = "3130";
	}
	$config_array[] = 'icp_port ' . $icp_port . "\n";
	unset($icp_port);

	if(!isset($proxy_port) or ($proxy_port == "")) {
		$proxy_port = "3128";
	}

	if (isset($transparent_proxy) && ($transparent_proxy != "on")) {
		$int = convert_friendly_interface_to_real_interface_name($active_interface);
		$listen_ip = find_interface_ip($int);

		$config_array[] = 'http_port ' . $listen_ip . ':' . $proxy_port . "\n\n";
		$config_array[] = 'acl QUERY urlpath_regex cgi-bin \?' . "\n";
		$config_array[] = 'no_cache deny QUERY' . "\n\n";
	}
	$config_array[] = 'http_port 127.0.0.1:' . $proxy_port . "\n\n";
	unset($proxy_port);

	if (isset($domain) && ($domain !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$aclout = fopen($acldir . "/dst_nocache.acl","w");

		$domain_array = split("; ",$domain);
		foreach ($domain_array as $no_cache_domain) {
			fwrite($aclout, $no_cache_domain . "\n");
		}

		fclose($aclout);

		$config_array[] = 'acl no_cache_domains dstdomain "' . $acldir . '/dst_nocache.acl"' . "\n";
		$config_array[] = 'no_cache deny no_cache_domains' . "\n\n";
	}
	unset($no_cache_domain);
	unset($domain_array);
	unset($domain);

	$config_array[] = 'cache_effective_user squid' . "\n";
	$config_array[] = 'cache_effective_group squid' . "\n\n";
	$config_array[] = 'pid_filename /var/run/squid.pid' . "\n\n";

	if (!isset($memory_cache_size) or ($memory_cache_size == "")) {
		$memory_cache_size = "8";
	}
	$config_array[] = 'cache_mem ' . $memory_cache_size . ' MB' . "\n";
	unset($memory_cache_size);

	if (!isset($harddisk_cache_size) or ($harddisk_cache_size == "")) {
		$harddisk_cache_size = "500";
	}

	if (!isset($level_subdirs) or ($level_subdirs == "")) {
		$level_subdirs = "16";
	}

	$config_array[] = 'cache_dir diskd /var/squid/cache ' . $harddisk_cache_size . ' ' . $level_subdirs . ' 256' . "\n\n";
	unset($harddisk_cache_size);
	unset($level_subdirs);

	if (!isset($error_language) or ($error_language == "")) {
		$error_language = "English";
	}
	$config_array[] = 'error_directory /usr/local/etc/squid/errors/' . $error_language . "\n\n";
	unset($error_language);

	if (isset($offline_mode) && ($offline_mode == "on")) {
		$config_array[] = 'offline_mode on' . "\n\n";
	} else {
		$config_array[] = 'offline_mode off' . "\n\n";
	}

	if (!isset($memory_replacement) or ($memory_replacement == "")) {
		$memory_replacement = "heap GDSF";
	}
	$config_array[] = 'memory_replacement_policy ' . $memory_replacement . "\n";
	unset($memory_replacement);

	if (!isset($cache_replacement) or ($cache_replacement == "")) {
		$cache_replacement="heap GDSF";
	}
	$config_array[] = 'cache_replacement_policy ' . $cache_replacement . "\n\n";
	unset($cache_replacement);

	if (isset($accesslog_disabled) && ($accesslog_disabled == "on")) {
		$config_array[] = 'cache_access_log none' . "\n";
	} else {
		$config_array[] = 'cache_access_log /var/log/access.log' . "\n";
	}
	$config_array[] = 'cache_log /var/log/cache.log' . "\n";
	$config_array[] = 'cache_store_log none' . "\n";
	unset($accesslog_disabled);
	unset($log_enabled);

	if (isset($log_query_terms) && ($log_query_terms == "on")) {
		$config_array[] = 'strip_query_terms off' . "\n";
	} else {
		$config_array[] = 'strip_query_terms on' . "\n";
	}
	unset($log_query_terms);

	$config_array[] = 'useragent_log /var/log/useragent.log' . "\n\n";
	unset($log_user_agents);

	$config_array[] = 'log_mime_hdrs off' . "\n";
	$config_array[] = 'emulate_httpd_log on' . "\n";

	switch ($user_forwarding) {
		case "on":
			$config_array[] = 'forwarded_for on' . "\n\n";
			break;
		case "off":
			$config_array[] = 'forwarded_for off' . "\n\n";
			break;
		default:
			$config_array[] = 'forwarded_for off' . "\n\n";
			break;
	}
	unset($user_forwarding);

	switch ($auth_method) {
		case "none":
			break;
		case "local_auth":
			$config_array[] = 'auth_param basic program /usr/local/libexec/squid/ncsa_auth /usr/local/etc/squid/advanced/ncsa/passwd' . "\n";
			if (!isset($auth_processes) or ($auth_processes == "")) {
				$auth_processes = "5";
			}
			$config_array[] = 'auth_param basic children ' . $auth_processes . "\n";

			if (!isset($auth_realm_prompt) or ($auth_realm_prompt == "")) {
				$auth_realm_prompt = "pfSense Advanced Proxy";
			}
			$config_array[] = 'auth_param basic realm ' . $auth_realm_prompt . "\n";

			if (!isset($auth_cache_ttl) or ($auth_cache_ttl == "")) {
				$auth_cache_ttl = "60";
			}
			$config_array[] = 'auth_param basic credentialsttl ' . $auth_cache_ttl . ' minutes' . "\n\n";
			$config_array[] = 'acl for_inetusers proxy_auth REQUIRED' . "\n\n";

			unset($auth_realm_prompt);
			unset($auth_processes);
			unset($auth_cache_ttl);

			break;
		case "radius_auth";
			$config_array[] = 'auth_param basic program /usr/local/libexec/squid/squid_rad_auth -h ' . $radius_server . ' -p ' . $radius_port . ' -i ' . $radius_identifier . ' -w ' . $radius_secret . "\n";
			if (!isset($auth_processes) or ($auth_processes == "")) {
				$auth_processes = "5";
			}
			$config_array[] = 'auth_param basic children ' . $auth_processes . "\n";

			if (!isset($auth_realm_prompt) or ($auth_realm_prompt == "")) {
				$auth_realm_prompt = "pfSense Advanced Proxy";
			}
			$config_array[] = 'auth_param basic realm ' . $auth_realm_prompt . "\n";

			if (!isset($auth_cache_ttl) or ($auth_cache_ttl == "")) {
				$auth_cache_ttl = "60";
			}
			$config_array[] = 'auth_param basic credentialsttl ' . $auth_cache_ttl . ' minutes' . "\n\n";
			$config_array[] = 'acl for_inetusers proxy_auth REQUIRED' . "\n\n";

			unset($auth_realm_prompt);
			unset($auth_processes);
			unset($auth_cache_ttl);

			break;
		case "ldap_bind";
			$config_array[] = 'auth_param basic program /usr/local/libexec/squid_ldap_auth -b "' . $ldap_basedn . '" -D "' . $bind_dn_username . '" -w "' . $bind_dn_password . '" -f "(&(objectClass=person)(cn=%s))" -u -cn -P "' . $ldap_server . ":" . $ldap_port . "\n";
			$config_array[] = 'auth_param basic program /usr/local/libexec/squid/squid_ldap_auth';
			$config_array[] = ' -b "' . $ldap_basedn . '"';
			$config_array[] = ' -D "' . $bind_dn_username . '"';
			$config_array[] = " -w " . $bind_dn_password;
			$config_array[] = ' -f "(&(objectClass=person)(cn=%s))"';
			$config_array[] = " -u cn -P " . $ldap_server . ":" . $ldap_port . "\n";

			if (!isset($auth_processes) or ($auth_processes == "")) {
				$auth_processes = "5";
			}
			$config_array[] = 'auth_param basic children ' . $auth_processes . "\n";

			if (!isset($auth_realm_prompt) or ($auth_realm_prompt == "")) {
				$auth_realm_prompt = "pfSense Advanced Proxy";
			}
			$config_array[] = 'auth_param basic realm ' . $auth_realm_prompt . "\n";

			if (!isset($auth_cache_ttl) or ($auth_cache_ttl == "")) {
				$auth_cache_ttl = "60";
			}
			$config_array[] = 'auth_param basic credentialsttl ' . $auth_cache_ttl . ' minutes' . "\n\n";
			$config_array[] = 'acl for_inetusers proxy_auth REQUIRED' . "\n\n";

			unset($auth_realm_prompt);
			unset($auth_processes);
			unset($auth_cache_ttl);

			break;
		case "windows_auth";
			break;
	}

	if (isset($throttle_binary_files) && ($throttle_binary_files == "on")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$binary_out = "\.bin$\n\.cab$\n\.gz$\n\.rar$\n\.sea$\n\.tar$\n\.tgz$\n\.zip$\n";

		$throttle_out = fopen($acldir . "/dst_throttle_binary.acl", "w");
		fwrite($throttle_out, $binary_out);
		fclose($throttle_out);
		$config_array[] = 'acl for_throttled_binary url_regex -i "' . $acldir . '/dst_throttle_binary.acl"' . "\n";
	} else {
		if (file_exists($acldir . "/dst_throttle_binary.acl")) unlink($acldir . "/dst_throttle_binary.acl");
	}
	unset($throttle_binary_files);
	unset($throttle_out);
	unset($binary_out);

	if (isset($throttle_cd_images) && ($throttle_cd_images == "on")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$cd_out = "\.b5t$\n\.bin$\n\.bwt$\n\.cdi$\n\.cue$\n\.gho$\n\.img$\n\.iso$\n\.mds$\n\.nrg$\n\.pqi$\n";

		$throttle_out = fopen($acldir . "/dst_throttle_cd.acl","w");
		fwrite($throttle_out, $cd_out);
		fclose($throttle_out);
		$config_array[] = 'acl for_throttled_cd url_regex -i "' . $acldir . '/dst_throttle_cd.acl"' . "\n";
	} else {
		if (file_exists($acldir . "/dst_throttle_cd.acl")) {
			unlink($acldir . "/dst_throttle_cd.acl");
		}
	}
	unset($throttle_cd_images);
	unset($throttle_out);
	unset($cd_out);

	if (isset($throttle_multimedia) && ($throttle_multimedia == "on")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$multimedia_out = "\.aiff?$\n\.asf$\n\.avi$\n\.divx$\n\.mov$\n\.mp3$\n\.mpe?g$\n\.qt$\n\.ra?m$\n";

		$throttle_out = fopen($acldir . "/dst_throttle_multimedia.acl","w");
		fwrite($throttle_out, $multimedia_out);
		fclose($throttle_out);
		$config_array[] = 'acl for_throttled_multimedia url_regex -i "' . $acldir . '/dst_throttle_multimedia.acl"' . "\n";
	} else {
		if (file_exists($acldir . "/dst_throttle_multimedia.acl")) {
			unlink($acldir . "/dst_throttle_multimedia.acl");
		}
	}
	unset($throttle_multimedia);
	unset($multimedia_out);
	unset($throttle_out);

	$config_array[] = 'acl within_timeframe time MTWHFAS 00:00-24:00' . "\n\n";

    /* obtain interface subnet and address for Squid rules */
    $lactive_interface = strtolower($active_interface);

    $lancfg = $config['interfaces'][$lactive_interface];
    $lanif = $lancfg['if'];
    $lanip = $lancfg['ipaddr'];
    $lansa = gen_subnet($lancfg['ipaddr'], $lancfg['subnet']);
    $lansn = $lancfg['subnet'];

	$config_array[] = 'acl all src 0.0.0.0/0.0.0.0' . "\n";
	$config_array[] = 'acl localnet src ' . $lansa . '/' . $lansn . "\n";
	$config_array[] = 'acl localhost src 127.0.0.1/255.255.255.255' . "\n";
	$config_array[] = 'acl SSL_ports port 443 563 873 # https, snews, rsync' . "\n";
	$config_array[] = 'acl Safe_ports port 80 # http' . "\n";
	$config_array[] = 'acl Safe_ports port 21 # ftp' . "\n";
	$config_array[] = 'acl Safe_ports port 443 563 873 # https, snews, rsync' . "\n";
	$config_array[] = 'acl Safe_ports port 70 # gopher' . "\n";
	$config_array[] = 'acl Safe_ports port 210 # wais' . "\n";
	$config_array[] = 'acl Safe_ports port 1025-65535 # unregistered ports' . "\n";
	$config_array[] = 'acl Safe_ports port 280 # http-mgmt' . "\n";
	$config_array[] = 'acl Safe_ports port 488 # gss-http' . "\n";
	$config_array[] = 'acl Safe_ports port 591 # filemaker' . "\n";
	$config_array[] = 'acl Safe_ports port 777 # multiling http' . "\n";
	$config_array[] = 'acl Safe_ports port 800 # Squids port (for icons)' . "\n\n";

	/* allow access through proxy for custom admin port */
    $custom_port = $config['system']['webgui']['port'];
	if (isset($custom_port) && ($custom_port !== "")) {
		$config_array[] = 'acl pf_admin_port port	' . $custom_port . "\n";
		unset($custom_port);
	} else {
		$admin_protocol = $config['system']['webgui']['protocol'];
		switch ($admin_protocol) {
			case "http";
				$config_array[] = 'acl pf_admin_port port 80' ."\n";
				break;
			case "https";
				$config_array[] = 'acl pf_admin_port port 443' . "\n";
				break;
			default;
				$config_array[] = 'acl pf_admin_port port 80' . "\n";
				break;
		}
		unset($admin_protocol);
	}

	/* define override hosts as specified in squid_nac.xml */
	if (isset($override_hosts) && ($override_hosts !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$aclout = fopen($acldir . "/src_override_hosts.acl", "w");

		$override_hosts_array = split("; ", $override_hosts);
		foreach ($override_hosts_array as $ind_override_host) {
			fwrite($aclout, $ind_override_host . "\n");
		}

		fclose($aclout);

		$config_array[] = 'acl override_hosts src "/usr/local/etc/squid/advanced/acls/src_override_hosts.acl"' . "\n";
	}
	/* clear variables */
	unset($override_hosts_array);
	unset($ind_override_host);
	unset($override_hosts);

	/* define subnets allowed to utilize proxy service */
	if (isset($allowed_subnets) && ($allowed_subnets !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
			mwexec("touch {$acldir}/src_subnets.acl");
		}

		$aclout = fopen($acldir . "/src_subnets.acl","w");

		$allowed_subnets_array = split("; ",$allowed_subnets);
		foreach ($allowed_subnets_array as $ind_allowed_subnets) {
			fwrite($aclout, $ind_allowed_subnets . "\n");
		}

		fclose($aclout);
	} else {

		$aclout = fopen($acldir . "/src_subnets.acl","w");
		fwrite($aclout, $lansa . "/" . $lansn . "\n");
		fclose($aclout);
	}

	$config_array[] = 'acl pf_networks src "/usr/local/etc/squid/advanced/acls/src_subnets.acl"' . "\n";

	unset($allowed_subnets_array);
	unset($ind_allowed_subnets);
	unset($allowed_subnets);

	/* define ip addresses that have 'unrestricted' access */
	if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$aclout = fopen($acldir . "/src_unrestricted_ip.acl","w");

		$unrestricted_ip_array = split("; ",$unrestricted_ip_addr);
		foreach ($unrestricted_ip_array as $ind_unrestricted_ip) {
			fwrite($aclout, $ind_unrestricted_ip . "\n");
		}

		fclose($aclout);

		$config_array[] = 'acl pf_unrestricted_ip src "/usr/local/etc/squid/advanced/acls/src_unrestricted_ip.acl"' . "\n";
	}
	unset($unrestricted_ip_array);
	unset($unrestricted_ip_addr);
	unset($ind_unrestricted_ip);

	/* define mac addresses that have 'unrestricted' access */
	if (isset($unrestricted_mac_addr) && ($unrestricted_mac_addr !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$aclout = fopen($acldir . "/src_unrestricted_mac.acl","w");

		$unrestricted_mac_array = split("; ",$unrestricted_mac_addr);
		foreach ($unrestricted_mac_array as $ind_unrestricted_mac) {
			fwrite($aclout, $ind_unrestricted_mac . "\n");
		}

		fclose($aclout);

		$config_array[] = 'acl pf_unrestricted_mac src "/usr/local/etc/squid/advanced/acls/src_unrestricted_mac.acl"' . "\n";
	}
	unset($unrestricted_mac_array);
	unset($unrestricted_mac_addr);
	unset($ind_unrestricted_mac);

	/* define ip addresses that are banned from using the proxy service */
	if (isset($banned_ip_addr) && ($banned_ip_addr !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$aclout = fopen($acldir . "/src_banned_ip.acl","w");

		$banned_ip_array = split("; ",$banned_ip_addr);
		foreach ($banned_ip_array as $ind_banned_ip) {
			fwrite($aclout, $ind_banned_ip . "\n");
		}

		fclose($aclout);

		$config_array[] = 'acl pf_banned_ip src "/usr/local/etc/squid/advanced/acls/src_banned_ip.acl"' . "\n";
	}
	unset($banned_ip_array);
	unset($banned_ip_addr);
	unset($ind_banned_ip);

	/* define mac addresses that are banned from using the proxy service */
	if (isset($banned_mac_addr) && ($banned_mac_addr !== "")) {
		if (!file_exists($acldir)) {
			mwexec("/bin/mkdir -p " . $acldir);
		}

		$aclout = fopen($acldir . "/src_banned_mac.acl","w");

		$banned_mac_array = split("; ",$banned_mac_addr);
		foreach ($banned_mac_array as $ind_banned_mac) {
			fwrite($aclout, $ind_banned_mac . "\n");
		}

		fclose($aclout);

		$config_array[] = 'acl pf_banned_mac src "/usr/local/etc/squid/advanced/acls/src_banned_mac.acl"' . "\n";
	}
	unset($banned_mac_array);
	unset($banned_mac_addr);
	unset($ind_banned_mac);

	$config_array[] = 'acl pf_ips	dst ' . $lanip . "\n";
    $config_array[] = 'acl CONNECT method CONNECT' . "\n\n";

    if (isset($auth_method) && ($auth_method == "none")) {
    	$config_array[] = 'http_access allow localnet' . "\n";
    }
    $config_array[] = 'http_access allow localhost' . "\n";

	if (isset($override_hosts) && ($override_hosts !== "")) {
		$config_array[] = 'http_access allow override_hosts' . "\n";
	}
	$config_array[] = "\n";

	switch ($config['system']['webgui']['protocol']) {
		case "http":
			$config_array[] = 'http_access allow pf_ips' . "\n";
			$config_array[] = 'http_access allow pf_admin_port' . "\n";
			$config_array[] = 'http_access deny !pf_networks' . "\n\n";
			break;
		case "https":
			$config_array[] = 'http_access allow CONNECT pf_ips' . "\n";
			$config_array[] = 'http_access allow CONNECT pf_admin_port' . "\n";
			$config_array[] = 'http_access deny  CONNECT !pf_networks' . "\n\n";
			break;
	}

    $config_array[] = 'http_access deny !Safe_ports' . "\n";
    $config_array[] = 'http_access deny CONNECT !SSL_ports' . "\n\n";

    if (isset($auth_method) && ($auth_method != "none")) {
    	$config_array[] = 'http_access allow pf_networks for_inetusers within_timeframe' . "\n";
    }

    $config_array[] = 'http_access deny all' . "\n\n";

	if (isset($dl_overall) && ($dl_overall !== "") and isset($dl_per_host) && ($dl_per_host == "")) {
       	$config_array[] = 'delay_pools 1' . "\n";
       	$config_array[] = 'delay_class 1 3' . "\n";

       	if ($dl_overall == "unlimited") {
       		$config_array[] = 'delay_parameters 1 -1/-1 -1/-1 ' . ($dl_overall * 125) . '/' . ($dl_overall * 250) . "\n";
       	} else {
       		$config_array[] = 'delay_parameters 1 ' . ($dl_overall * 125) . '/' . ($dl_overall * 250) . ' -1/-1 -1/-1' . "\n";
       	}

   		/* if no unrestricted ip addresses are defined; this line is ignored */
 		if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr == "")) $config_array[] = 'delay_access 1 deny pf_unrestricted_ip' . "\n";

   		/* this will define bandwidth delay restrictions for specified throttles */
   		if (isset($throttle_binary_files) && ($throttle_binary_files == "on")) {
   			$config_array[] = 'delay_access 1 allow all for_throttled_binary' . "\n";
   		}
   		if (isset($throttle_cd_images) && ($throttle_cd_images == "on")) {
   			$config_array[] = 'delay_access 1 allow all for_throttled_cd' . "\n";
   		}
   		if (isset($throttle_multimedia) && ($throttle_multimedia == "on")) {
   			$config_array[] = 'delay_access 1 allow all for_throttled_multimedia' . "\n";
   		} else {
   			$config_array[] = 'delay_access 1 allow all' . "\n";
   		}
   		$config_array[] = 'delay_initial_bucket_level 100%' . "\n\n";
    }

    if (isset($dl_per_host) && ($dl_per_host !== "") and isset($dl_overall) && ($dl_overall == "")) {
       	$config_array[] = 'delay_pools 1' . "\n";
      	$config_array[] = 'delay_class 1 3' . "\n";

      	if ($dl_per_host == "unlimited") {
      		$config_array[] = 'delay_parameters 1 ' . ($dl_per_host * 125) . '/' . ($dl_per_host * 250) . '-1/-1 -1/-1' . "\n";
      	} else {
       		$config_array[] = 'delay_parameters 1 -1/-1 -1/-1 ' . ($dl_per_host * 125) . '/' . ($dl_per_host * 250) . "\n";
       	}

   		/* if no unrestricted ip addresses are defined; this line is ignored */
 		if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) $config_array[] = 'delay_access 1 deny pf_unrestricted_ip' . "\n";

   		/* this will define bandwidth delay restrictions for specified throttles */
   		if ($throttle_binary_files == "on") {
   			$config_array[] = 'delay_access 1 allow all for_throttled_binary' . "\n";
   		}
   		if ($throttle_cd_images == "on") {
   			$config_array[] = 'delay_access 1 allow all for_throttled_cd' . "\n";
   		}
   		if ($throttle_multimedia == "on") {
   			$config_array[] = 'delay_access 1 allow all for_throttled_multimedia' ."\n";
   		} else {
   			$config_array[] = 'delay_access 1 allow all' . "\n";
   		}
       	$config_array[] = 'delay_initial_bucket_level 100%' . "\n\n\n";
    }

    if (isset($dl_overall) && ($dl_overall !== "") and isset($dl_per_host) && ($dl_per_host !== "")) {
    	/* if no bandwidth restrictions are specified, then these parameters are not necessary */
    	if ($dl_overall !== "unlimited" and $dl_per_host !== "unlimited") {

   			if ((isset($dl_overall) && ($dl_overall == "unlimited")) and (isset($dl_per_host) && ($dl_per_host !== ""))) {
   				$config_array[] = 'delay_pools 1' . "\n";
   				$config_array[] = 'delay_class 1 3' . "\n";
   				$config_array[] = 'delay_parameters 1 -1/-1 -1/-1 ' . ($dl_per_host * 125) . '/' . ($dl_overall * 250) . "\n";
   			} elseif (isset($dl_overall) && ($dl_overall !== "") and isset($dl_per_host) && ($dl_per_host == "unlimited")) {
   				$config_array[] = 'delay_pools 1' . "\n";
   				$config_array[] = 'delay_class 1 3' . "\n";
   				$config_array[] = 'delay_parameters 1 ' . ($dl_overall * 125) . '/' . ($dl_overall * 250) . ' -1/-1 -1/-1' . "\n";
   			}
   		}

   		if ($dl_overall !== "unlimited" and $dl_per_host !== "unlimited") {

   			/* if no unrestricted ip addresses are defined; this line is ignored */
   			if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) $config_array[] = 'delay_access 1 deny pf_unrestricted_ip' . "\n";

   			/* this will define bandwidth delay restrictions for specified throttles */
   			if ($throttle_binary_files == "on") {
   				$config_array[] = 'delay_access 1 allow all for_throttled_binary' . "\n";
   			}
   			if ($throttle_cd_images == "on") {
   				$config_array[] = 'delay_access 1 allow all for_throttled_cd' . "\n";
   			}
   			if ($throttle_multimedia == "on") {
   				$config_array[] = 'delay_access 1 allow all for_throttled_multimedia' . "\n";
   			} else {
   				$config_array[] = 'delay_access 1 allow all' . "\n";
   			}
   			$config_array[] = 'delay_initial_bucket_level 100%' . "\n\n";
   		}
    }

    $config_array[] = 'header_access X-Forwarded-For deny all' . "\n";
    $config_array[] = 'header_access Via deny all' . "\n\n";

	/* TODO: acl customization for snmp support */
	/* fwrite($fout, "\n"); */

	if (isset($urlfilter_enable) && ($urlfilter_enable == "on")) {
		$config_array[] = 'redirect_program /usr/sbin/squidGuard' . "\n";
		$config_array[] = 'redirect_children 5' . "\n\n";
	}

	if (isset($max_upload_size) && ($max_upload_size != "")) {
		$config_array[] = 'request_body_max_size ' . $max_download_size . 'KB' . "\n";
	}

	if (isset($max_download_size) && ($max_download_size != "")) {
		if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) $config_array[] = 'reply_body_max_size 0 allow pf_unrestricted_ip' . "\n";
		/* fwrite($fout, "#reply_body_max_size 0 allow for_extended_users\n"); */
		$config_array[] = 'reply_body_max_size ' . $max_download_size * 1024 . ' allow all' . "\n\n";
	}

	/* set default value for maximum_object_size */
	if (!isset($maximum_object_size) or ($maximum_object_size == "")) {
		$maximum_object_size = "4096";
	}

	/* set default value for minimum_object_size */
	if (!isset($minimum_object_size) or ($minimum_object_size == "")) {
		$minimum_object_size = "0";
	}
	$config_array[] = 'maximum_object_size ' . $maximum_object_size . ' KB' . "\n";
	$config_array[] = 'minimum_object_size ' . $minimum_object_size . ' KB' . "\n\n";

	if (isset($proxy_forwarding) && ($proxy_forwarding == "on")) {
		$config_array[] = 'cache_peer ' . $upstream_proxy . ' parent ' . $upstream_proxy_port . ' 3130 login=' . upstream_username . ':' . upstream_password . ' default no-query' . "\n";
		$config_array[] = 'never_direct allow all' . "\n";
	}
	unset($proxy_forwarding);


	/* define default ruleset for transparent proxy operation */
	if (isset($transparent_proxy) && ($transparent_proxy == "on")) {
       	$config_array[] = 'httpd_accel_host virtual' . "\n";
      	$config_array[] = 'httpd_accel_port 80' . "\n";
       	$config_array[] = 'httpd_accel_with_proxy on' . "\n";
       	$config_array[] = 'httpd_accel_uses_host_header on' . "\n\n";
    }
   	unset($transparent_proxy);


    /* define visible hostname */
	if (isset($visible_hostname) && ($visible_hostname !== "")) {
		$config_array[] = 'visible_hostname ' . $visible_hostname . "\n";
	}
	unset($visible_hostname);

	/* define cache administrators email address within error messages */
	if (isset($cache_admin_email) && ($cache_admin_email !== "")) {
		$config_array[] = 'cache_mgr ' . $cache_admin_email . "\n\n";
	}
	unset($cache_admin_email);

	/* write configuration file */
	foreach ($config_array as $config_item)
	{
		fwrite($fout, trim($config_item));

		if (stristr($config_item, "\n"))
		{
			for ($i = 1; $i < count(explode("\n", $config_item)); $i++)
			{
				fwrite($fout, "\n");
			}
		}

	}
	fclose($fout);

	conf_mount_ro();
	config_unlock();

	touch($squidconfig);
} /* end function write_squid_config */

function custom_php_install_command() {
		/* write initial static config for transparent proxy */
		write_static_squid_config();

		touch("/tmp/custom_php_install_command");

		/*    make sure this all exists, see:
		 *    https://forum.pfsense.org/index.php?topic=23.msg2391#msg2391
		 */
		update_output_window("Setting up Squid environment...");
		mwexec("mkdir -p /var/squid");
		mwexec("chown squid:squid /var/squid");
		mwexec("mkdir -p /var/squid/logs");
		mwexec("chown squid:squid /var/squid/logs");
		mwexec("mkdir -p /var/squid/cache");
		mwexec("chown squid:squid /var/squid/cache");
		mwexec("mkdir -p /usr/local/etc/squid/advanced");
		mwexec("chown squid:squid /usr/local/etc/squid/advanced");
		mwexec("mkdir -p /usr/local/etc/squid/advanced/acls");
		mwexec("chown squid:squid /usr/local/etc/squid/advanced/acls");
		mwexec("touch /usr/local/etc/squid/advanced/acls/src_subnets.acl");
		mwexec("chown squid:squid /usr/local/etc/squid/advanced/acls/src_subnets.acl");
		mwexec("touch /usr/local/etc/squid/advanced/acls/src_unrestricted_ip.acl");
		mwexec("chown squid:squid /usr/local/etc/squid/advanced/acls/src_unrestricted_ip.acl");
		mwexec("cp /usr/local/etc/squid/mime.conf.default /usr/local/etc/squid/mime.conf");


		/* set a few extra items noted by regan */
		update_output_window("Creating logs and setting user information...");
		$fdsquid = fopen("/usr/local/etc/rc.d/aSquid.sh", "w");
		fwrite($fdsquid, "#/bin/sh\n");
		fwrite($fdsquid, "# \n");
		fwrite($fdsquid, "# This file was created by the pfSense package system\n");
		fwrite($fdsquid, "# Sets up squid option on each bootup that are not persistent\n");
		fwrite($fdsquid, "# \n\n");
		fwrite($fdsquid, "chown squid:wheel /dev/pf\n");
		fwrite($fdsquid, "chmod ug+rw /dev/pf\n");
		fwrite($fdsquid, "touch /var/log/useragent.log\n");
		fwrite($fdsquid, "touch /var/log/access.log\n");
		fwrite($fdsquid, "touch /var/log/cache.log\n");
		fwrite($fdsquid, "chown squid:wheel /var/log/cache.log\n");
		fwrite($fdsquid, "chown squid:wheel /var/log/access.log\n");
		fwrite($fdsquid, "chown squid:wheel /var/log/useragent.log\n");
		fwrite($fdsquid, "\n");
		fclose($fdsquid);
		mwexec("chmod a+rx /usr/local/etc/rc.d/aSquid.sh");
		mwexec("/usr/local/etc/rc.d/aSquid.sh");

		update_output_window("Creating Proxy Server initialization scripts...");
		$start = "touch /tmp/ro_root_mount; /usr/local/sbin/squid -D; touch /tmp/filter_dirty";
		$stop = "/usr/local/sbin/squid -k shutdown";
		write_rcfile(array(
					"file" => "squid.sh",
					"start" => $start,
					"stop" => $stop
				)
		);

		mwexec("chmod 755 /usr/local/etc/rc.d/squid.sh");

		/* create log directory hierarchies if they don't exist */
		update_output_window("Creating required directory hierarchies...");

		if (!file_exists("/var/squid/logs")) {
			mwexec("mkdir -p /var/squid/logs");
		}
		mwexec("/usr/sbin/chown squid:squid /var/squid/logs");


		if (!file_exists("/var/squid/cache")) {
			mwexec("mkdir -p /var/squid/cache");
		}
		mwexec("/usr/sbin/chown squid:squid /var/squid/cache");

		if (!file_exists("/usr/local/etc/squid/advanced/acls")) {
			mwexec("mkdir -p /usr/local/etc/squid/advanced/acls");
		}
		mwexec("/usr/sbin/chown squid:squid /usr/local/etc/squid/advanced/acls");

		if (!file_exists("/usr/local/etc/squid/advanced/ncsa")) {
			mwexec("mkdir -p /usr/local/etc/squid/advanced/ncsa");
		}
		mwexec("/usr/sbin/chown squid:squid /usr/local/etc/squid/advanced/ncsa");

		if (!file_exists("/usr/local/etc/squid/advanced/ntlm")) {
			mwexec("mkdir -p /usr/local/etc/squid/advanced/ntlm");
		}
		mwexec("/usr/sbin/chown squid:squid /usr/local/etc/squid/advanced/ntlm");

		if (!file_exists("/usr/local/etc/squid/advanced/radius")) {
			mwexec("mkdir -p /usr/local/etc/squid/advanced/radius");
		}
		mwexec("/usr/sbin/chown squid:squid /usr/local/etc/squid/advanced/radius");

		$devfs_file = fopen("/etc/devfs.conf", "a");
		fwrite($devfs_file, "\n# Allow squid to query the packet filter bymaking is group-accessable. ");
		fwrite($devfs_file, "own pf root:squid");
		fwrite($devfs_file, "perm pf 0640");
		fclose($devfs_file);

		update_output_window("Initializing Cache... This may take a moment...");
		mwexec("/usr/local/sbin/squid -z");

		update_output_window("Starting Proxy Server...");
		start_service("squid");
}

function custom_php_deinstall_command() {
	update_output_window("Stopping proxy service...");
	stop_service("squid");
	sleep(1);
	/* brute force any remaining squid processes out */
	mwexec("/usr/bin/killall squid");
	mwexec("/usr/bin/killall pinger");
	update_output_window("Recursively removing directories hierarchies.  If existant, log files in /var/squid/logs will remain...");
	mwexec("rm -rf /var/squid/cache");
	update_output_window("Removing configuration files...");
	unlink_if_exists("/usr/local/etc/rc.d/squid.sh");
	unlink_if_exists("/usr/local/libexec/squid");
	unlink_if_exists("/usr/local/etc/rc.d/aSquid.sh");
	mwexec("rm -f /usr/local/etc/rc.d/squid*");
	mwexec("rm -f /usr/local/www/cachemgr.cgi");
	filter_configure();
}

function write_static_squid_config() {
	touch("/tmp/write_static_squid_config");
	global $config;
	$lancfg = $config['interfaces']['lan'];
	$lanif = $lancfg['if'];
	$lanip = $lancfg['ipaddr'];
	$lansa = gen_subnet($lancfg['ipaddr'], $lancfg['subnet']);
	$lansn = $lancfg['subnet'];

	$fout = fopen("/usr/local/etc/squid/squid.conf","w");
	fwrite($fout, "#\n");
	fwrite($fout, "# This file was automatically generated by the pfSense package manager.\n");
	fwrite($fout, "# This default policy enables transparent proxy with no local disk logging.\n");
	fwrite($fout, "#\n");

	/* set # of dns children */
	fwrite($fout, "dns_children 15\n");

	fwrite($fout, "shutdown_lifetime 5 seconds\n");
	fwrite($fout, "icp_port 0\n");
	fwrite($fout, "\n");

	fwrite($fout, "acl QUERY urlpath_regex cgi-bin \?\n");
	fwrite($fout, "no_cache deny QUERY\n");
	fwrite($fout, "\n");

	fwrite($fout, "pid_filename /var/run/squid.pid\n");
	fwrite($fout, "\n");

	fwrite($fout, "cache_mem 24 MB\n");
	fwrite($fout, "cache_dir diskd /var/squid/cache 500 16 256\n");
	fwrite($fout, "\n");

	fwrite($fout, "error_directory /usr/local/etc/squid/errors/English\n");
	fwrite($fout, "\n");

	fwrite($fout, "memory_replacement_policy heap GDSF\n");
	fwrite($fout, "cache_replacement_policy heap GDSF\n");
	fwrite($fout, "\n");

	fwrite($fout, "cache_access_log none\n");
	fwrite($fout, "cache_log none\n");
	fwrite($fout, "cache_store_log none\n");
	fwrite($fout, "\n");

	fwrite($fout, "log_mime_hdrs off\n");
	fwrite($fout, "emulate_httpd_log on\n");
	fwrite($fout, "forwarded_for off\n");
	fwrite($fout, "\n");

	fwrite($fout, "acl within_timeframe time MTWHFAS 00:00-24:00\n");
	fwrite($fout, "\n");

	fwrite($fout, "acl all src 0.0.0.0/0.0.0.0\n");
	fwrite($fout, "acl localnet src " . $lansa . "/" . $lansn . "\n");
	fwrite($fout, "acl localhost src 127.0.0.1/255.255.255.255\n");
	fwrite($fout, "acl SSL_ports port 443 563 873 # https, snews, rsync\n");
	fwrite($fout, "acl Safe_ports port 80 # http\n");
	fwrite($fout, "acl Safe_ports port 21 # ftp\n");
	fwrite($fout, "acl Safe_ports port 443 563 873 # https, snews, rsync\n");
	fwrite($fout, "acl Safe_ports port 70 # gopher\n");
	fwrite($fout, "acl Safe_ports port 210 # wais\n");
	fwrite($fout, "acl Safe_ports port 1025-65535 # unregistered ports\n");
	fwrite($fout, "acl Safe_ports port 280 # http-mgmt\n");
	fwrite($fout, "acl Safe_ports port 488 # gss-http\n");
	fwrite($fout, "acl Safe_ports port 591 # filemaker\n");
	fwrite($fout, "acl Safe_ports port 777 # multiling http\n");
	fwrite($fout, "acl Safe_ports port 800 # Squids port (for icons)\n");
	fwrite($fout, "\n");

	fwrite($fout, "acl CONNECT method CONNECT\n");
	fwrite($fout, "\n");

	fwrite($fout, "#access to squid; local machine; no restrictions\n");
	fwrite($fout, "http_access allow localnet\n");
	fwrite($fout, "http_access allow localhost\n");
	fwrite($fout, "\n");

	fwrite($fout, "#Deny non web services\n");
	fwrite($fout, "http_access deny !Safe_ports\n");
	fwrite($fout, "http_access deny CONNECT !SSL_ports\n");
	fwrite($fout, "\n");

	fwrite($fout, "#Set custom configured ACLs\n");
	fwrite($fout, "http_access deny all\n");
	fwrite($fout, "visible_hostname pfSense\n");
	fwrite($fout, "\n");

	fwrite($fout, "cache_effective_user squid\n");
	fwrite($fout, "cache_effective_group squid\n");
	fwrite($fout, "\n");

	fwrite($fout, "maximum_object_size 4096 KB\n");
	fwrite($fout, "minimum_object_size 0 KB\n");
	fwrite($fout, "\n");

	fwrite($fout, "request_body_max_size 0 KB\n");
	fwrite($fout, "reply_body_max_size 0 allow all\n");
	fwrite($fout, "\n");

	fwrite($fout, "httpd_accel_host virtual\n");
	fwrite($fout, "httpd_accel_port 80\n");
	fwrite($fout, "httpd_accel_with_proxy on\n");
	fwrite($fout, "httpd_accel_uses_host_header on\n");

	fclose($fout);
}

function mod_htpasswd() {
	global $config;
	conf_mount_rw();
	config_lock();

	if (!file_exists("/usr/local/etc/squid/advanced/ncsa")) mwexec("mkdir -p /usr/local/etc/squid/advanced/ncsa");

	$passfile = fopen("/usr/local/etc/squid/advanced/ncsa/passwd", "w+");

	if (isset($config['installedpackages']['squidextlocalauth']['config']) && $config['installedpackages']['squidextlocalauth']['config'] != "") {
		foreach($config['installedpackages']['squidextlocalauth']['config'] as $rowhelper) {
			$encpass = generate_htpasswd($rowhelper['username'], $rowhelper['password']);
			fwrite($passfile, $rowhelper['username'] . ":" . $encpass . "\n");
		}
	}

	fclose($passfile);

	conf_mount_ro();
	config_unlock();
}

function generate_htpasswd($username, $password) {
	$all = explode( " ",
        "a b c d e f g h i j k l m n o p q r s t u v w x y z "
      . "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z "
        . "0 1 2 3 4 5 6 7 8 9");

    for ($i = 0; $i < 9; $i++) {
    	srand((double)microtime()*1000000);
    	$randy = rand(0,61);
    	$seed .= $all[$randy];
    }

    $crypt = crypt($password, "$1$$seed");
    return $crypt;
}

?>
