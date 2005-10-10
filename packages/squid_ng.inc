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

function global_write_squid_config() {
	conf_mount_rw();
	config_lock();
	
	global $config;
				
    /* define squid configuration file in variable for replace function */
    $squidconfig = "/usr/local/etc/squid/squid.conf";
            
    /* squid.xml values */
    $active_interface      = $config['installedpackages']['squid']['config'][0]['active_interface'];
    $transparent_proxy     = $config['installedpackages']['squid']['config'][0]['transparent_proxy'];
    $log_enabled           = $config['installedpackages']['squid']['config'][0]['log_enabled'];
    $urlfilter_enable      = $config['installedpackages']['squid']['config'][0]['urlfilter_enable'];
    $log_query_terms       = $config['installedpackages']['squid']['config'][0]['log_query_terms'];
    $log_user_agents       = $config['installedpackages']['squid']['config'][0]['log_user_agents'];
    $proxy_port            = $config['installedpackages']['squid']['config'][0]['proxy_port'];
    $visible_hostname      = $config['installedpackages']['squid']['config'][0]['visible_hostname'];
    $cache_admin_email     = $config['installedpackages']['squid']['config'][0]['cache_admin_email'];
    $error_language 	   = $config['installedpackages']['squid']['config'][0]['error_language'];
            
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
	
	/* static variable assignments for directory mapping */
	$acldir    = "/usr/local/etc/squid/advanced/acls";
	$ncsadir   = "/usr/local/etc/squid/advanced/ncsa";
	$ntlmdir   = "/usr/local/etc/squid/advanced/ntlm";
	$radiusdir = "/usr/local/etc/squid/advanced/radius";
			
    $fout = fopen($squidconfig,"w");
            
    /* option: shutdown_lifetime */
    fwrite($fout, "shutdown_lifetime 5 seconds\n");
    fwrite($fout, "\n");

	/* option: icp_port */	
	if($icp_port == "") $icp_port="3130"; 
	fwrite($fout, "icp_port " . $icp_port . "\n");
		
	/* option: proxy_port */
	if($proxy_port == "") $proxy_port="3128";
	if (isset($transparent_proxy) && ($transparent_proxy != "on")) {
		$int = convert_friendly_interface_to_real_interface_name($active_interface);
		$listen_ip = find_interface_ip($int);
		
		fwrite($fout, "http_port " . $listen_ip . ":" . $proxy_port . "\n");
		fwrite($fout, "\n");
			
		fwrite($fout, "acl QUERY urlpath_regex cgi-bin \?\n");
		fwrite($fout, "no_cache deny QUERY\n");		
	}
			
	/* option: acl no cache domains */
	if (isset($domain) && $domain !== "") {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
			$aclout = fopen($acldir . "/dst_nocache.acl","w");
			
			$domain_array = split(";",$domain);
			foreach ($domain_array as $no_cache_domain) {
			fwrite($aclout, $no_cache_domain . "\n");
		}
				
		fclose($aclout);
		
		fwrite($fout, 'acl no_cache_domains dstdomain "' . $acldir . '/dst_nocache.acl"' . "\n");
		fwrite($fout, "no_cache deny no_cache_domains\n");
	}
			
	fwrite($fout, "\n");
					
	fwrite($fout, "cache_effective_user squid\n");
	fwrite($fout, "cache_effective_group squid\n");
	fwrite($fout, "\n");
			
	fwrite($fout, "pid_filename /var/run/squid.pid\n");
	fwrite($fout, "\n");
			
	if ($memory_cache_size == "") $memory_cache_size="8";
	fwrite($fout, "cache_mem " . $memory_cache_size . " MB\n");
	if ($harddisk_cache_size == "") $harddisk_cache_size="500";
	if ($level_subdirs == "") $level_subdirs="16";
	fwrite($fout, "cache_dir diskd /var/squid/cache " . $harddisk_cache_size . " " . $level_subdirs . " 256\n");
	fwrite($fout, "\n");
			
	if ($error_language == "") $error_language="English";
	fwrite($fout, "error_directory /usr/local/etc/squid/errors/" . $error_language . "\n");
	fwrite($fout, "\n");
			
	if (isset($offline_mode) and ($offline_mode == "on")) {
		fwrite($fout, "offline_mode on\n");
		fwrite($fout, "\n");
	} else {
		fwrite($fout, "offline_mode off\n");
		fwrite($fout, "\n");
	}
			
	if (!isset($cache_replacement) or ($memory_replacement == "")) $memory_replacement="heap GDSF";
	fwrite($fout, "memory_replacement_policy " . $memory_replacement . "\n");
	if (!isset($cache_replacement) or ($cache_replacement == "")) $cache_replacement="heap GDSF";
	fwrite($fout, "cache_replacement_policy " . $cache_replacement . "\n");
	fwrite($fout, "\n");
			
	if ($log_enabled == "on" ) {
		if (!file_exists("/var/squid/logs")) {
			mwexec("mkdir -p /var/squid/logs");
			mwexec("chown squid:squid /var/squid/logs");
		}
		fwrite($fout, "cache_access_log /var/squid/logs/access.log\n");
		fwrite($fout, "cache_log /var/squid/logs/cache.log\n");
		fwrite($fout, "cache_store_log none\n");
	} else {
		fwrite($fout, "cache_access_log /dev/null\n");
		fwrite($fout, "cache_log /dev/null\n");
		fwrite($fout, "cache_store_log none\n");
	}

	if ($log_query_terms == "on") {
		fwrite($fout, "strip_query_terms off\n");
	} else {
		fwrite($fout, "strip_query_terms on\n");
	}
				
	if ($log_user_agents == "on") {
		if (!file_exists("/var/squid/logs")) mwexec("mkdir -p /var/squid/logs");
		fwrite($fout, "useragent_log /var/squid/logs/useragent.log\n");
	} 
	fwrite($fout, "\n");
			
	fwrite($fout, "log_mime_hdrs off\n");
	fwrite($fout, "emulate_httpd_log on\n");
	
	switch ($user_forwarding) {
		case "on":
			fwrite($fout, "forwarded_for on\n");
			break;
		case "off":
			fwrite($fout, "forwarded_for off\n");
			break;
		default:
			fwrite($fout, "forwarded_for off\n");
			break;
	}
	fwrite($fout, "\n");
	
	if ($no_auth == "on") {
		fwrite($fout, "\n");
	}
			
	switch ($auth_method) {
		case "none":
			break;
		case "local_auth":
			fwrite($fout, "auth_param basic program /usr/local/libexec/squid/ncsa_auth /usr/local/etc/squid/advanced/ncsa/passwd\n");
			if (!isset($auth_processes) or ($auth_processes == "")) $auth_processes = "5";
			fwrite($fout, "auth_param basic children " . $auth_processes . "\n");
			
			if (!isset($auth_realm_prompt) or ($auth_realm_prompt == "")) $auth_realm_prompt = "pfSense Advanced Proxy";
			fwrite($fout, "auth_param basic realm " . $auth_realm_prompt . "\n");
			
			if (!isset($auth_cache_ttl) or ($auth_cache_ttl == "")) $auth_cache_ttl = "60";
			fwrite($fout, "auth_param basic credentialsttl " . $auth_cache_ttl . " minutes\n");
			fwrite($fout, "\n");
			fwrite($fout, "acl for_inetusers proxy_auth REQUIRED\n");
			fwrite($fout, "\n");
			break;
		case "radius_auth";
			break;
		case "ldap_bind";
			/* fwrite($fout, 'auth_param basic program /usr/local/libexec/squid_ldap_auth -b "' . $ldap_basedn . '" -D "' . $bind_dn_username . '" -w "' . $bind_dn_password . '" -f "(&(objectClass=person)(cn=%s))" -u -cn -P "' . $ldap_server . ":" . $ldap_port . "\n"); */
			fwrite($fout, "auth_param basic program /usr/local/libexec/squid/squid_ldap_auth");
			fwrite($fout, ' -b "' . $ldap_basedn . '"');
			fwrite($fout, ' -D "' . $bind_dn_username . '"');
			fwrite($fout, " -w " . $bind_dn_password);
			fwrite($fout, ' -f "(&(objectClass=person)(cn=%s))"');
			fwrite($fout, " -u cn -P " . $ldap_server . ":" . $ldap_port . "\n");
			
			if (!isset($auth_processes) or ($auth_processes == "")) $auth_processes = "5";
			fwrite($fout, "auth_param basic children " . $auth_processes . "\n");
			
			if (!isset($auth_realm_prompt) or ($auth_realm_prompt == "")) $auth_realm_prompt = "pfSense Advanced Proxy";
			fwrite($fout, "auth_param basic realm " . $auth_realm_prompt . "\n");
			
			if (!isset($auth_cache_ttl) or ($auth_cache_ttl == "")) $auth_cache_ttl = "60";
			fwrite($fout, "auth_param basic credentialsttl " . $auth_cache_ttl . " minutes\n");
			fwrite($fout, "\n");
			fwrite($fout, "acl for_inetusers proxy_auth REQUIRED\n");
			fwrite($fout, "\n");			
			break;
		case "windows_auth";
			break;
		default:
			break;
	}
			
	if (isset($throttle_binary_files) && $throttle_binary_files == "on") {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
			$binary_out = "\.bin$\n\.cab$\n\.gz$\n\.rar$\n\.sea$\n\.tar$\n\.tgz$\n\.zip$\n";

			$throttle_out = fopen($acldir . "/dst_throttle_binary.acl", "w");
			fwrite($throttle_out, $binary_out);
			fclose($throttle_out);
			fwrite($fout, 'acl for_throttled_binary url_regex -i "' . $acldir . '/dst_throttle_binary.acl"' . "\n");
		} else {
			if (file_exists($acldir . "/dst_throttle_binary.acl")) unlink($acldir . "/dst_throttle_binary.acl");
	}
			
	if (isset($throttle_cd_images) && $throttle_cd_images == "on") {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
				
		$cd_out = "\.b5t$\n\.bin$\n\.bwt$\n\.cdi$\n\.cue$\n\.gho$\n\.img$\n\.iso$\n\.mds$\n\.nrg$\n\.pqi$\n";
				
		$throttle_out = fopen($acldir . "/dst_throttle_cd.acl","w");
		fwrite($throttle_out, $cd_out);
		fclose($throttle_out);
		fwrite($fout, 'acl for_throttled_cd url_regex -i "' . $acldir . '/dst_throttle_cd.acl"' . "\n");
	} else {
		if (file_exists($acldir . "/dst_throttle_cd.acl")) unlink($acldir . "/dst_throttle_cd.acl");
	}
			
	if (isset($throttle_multimedia) && $throttle_multimedia == "on") {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
			
		$multimedia_out = "\.aiff?$\n\.asf$\n\.avi$\n\.divx$\n\.mov$\n\.mp3$\n\.mpe?g$\n\.qt$\n\.ra?m$\n";			
			
		$throttle_out = fopen($acldir . "/dst_throttle_multimedia.acl","w");
		fwrite($throttle_out, $multimedia_out);
		fclose($throttle_out);
		fwrite($fout, 'acl for_throttled_multimedia url_regex -i "' . $acldir . 'dst_throttle_multimedia.acl"' . "\n");
	} else {
		if (file_exists($acldir . "/dst_throttle_multimedia.acl")) unlink($acldir . "/dst_throttle_multimedia.acl");			
	}	
			
	fwrite($fout, "acl within_timeframe time MTWHFAS 00:00-24:00\n");
	fwrite($fout, "\n");
			
    /* obtain interface subnet and address for Squid rules */
    $lactive_interface = strtolower($active_interface);
            
    $lancfg = $config['interfaces'][$lactive_interface];
    $lanif = $lancfg['if'];
    $lanip = $lancfg['ipaddr'];
    $lansa = gen_subnet($lancfg['ipaddr'], $lancfg['subnet']);
    $lansn = $lancfg['subnet'];			

	fwrite($fout, "acl all src " . $lansa . "/" . $lansn . "\n");
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
			
	/* allow access through proxy for custom admin port */
    $custom_port = $config['system']['webgui']['port'];
	if (isset($custom_port) && ($custom_port !== "")) {
		fwrite($fout, "acl pf_admin_port port	" . $custom_port . "\n");
	} else {
		$admin_protocol = $config['system']['webgui']['protocol'];
		switch ($admin_protocol) {
			case "http";
				fwrite($fout, "acl pf_admin_port port 80\n");
				break;
			case "https";
				fwrite($fout, "acl pf_admin_port port 443\n");
				break;
			default;
				fwrite($fout, "acl pf_admin_port port 80\n");
				break;
		}
	}
	
	/* define override hosts as specified in squid_nac.xml */
	if (isset($override_hosts) && ($override_hosts !== "")) {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
		
		$aclout = fopen($acldir . "/src_override_hosts.acl", "w");
 
		$override_hosts_array = split("; ", $override_hosts);
		foreach ($override_hosts_array as $ind_override_host) {
			fwrite($aclout, $ind_override_host . "\n");
		}
		
		fclose($aclout);
		
		fwrite($fout, 'acl override_hosts src "/usr/local/etc/squid/advanced/acls/src_override_hosts.acl"' . "\n");
	}	
			
	/* define subnets allowed to utilize proxy service */
	if (isset($allowed_subnets) && ($allowed_subnets !== "")) {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
	
		$aclout = fopen($acldir . "/src_subnets.acl","w");
				
		$allowed_subnets_array = split(";",$allowed_subnets);
		foreach ($allowed_subnets_array as $ind_allowed_subnets) {
			fwrite($aclout, $ind_allowed_subnets . "\n");
		}
				
		fclose($aclout);
				
		fwrite($fout, 'acl pf_networks src "/usr/local/etc/squid/advanced/acls/src_subnets.acl"' . "\n");
	} else {
		$aclout = fopen($acldir . "/src_subnets.acl","w");
		fwrite($aclout, $lansa . "/" . $lansn . "\n");
		fclose($aclout);
	}
		
			
	/* define ip addresses that have 'unrestricted' access */
	if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
		
		$aclout = fopen($acldir . "/src_unrestricted_ip.acl","w");
		
		$unrestricted_ip_array = split(";",$unrestricted_ip_addr);
		foreach ($unrestricted_ip_array as $ind_unrestricted_ip) {
			fwrite($aclout, $ind_unrestricted_ip . "\n");
		}
				
		fclose($aclout);
		
		fwrite($fout, 'acl pf_unrestricted_ip src "/usr/local/etc/squid/advanced/acls/src_unrestricted_ip.acl"' . "\n");
	}
			
	/* define mac addresses that have 'unrestricted' access */
	if (isset($unrestricted_mac_addr) && ($unrestricted_mac_addr !== "")) {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);

		$aclout = fopen($acldir . "/src_unrestricted_mac.acl","w");
			
		$unrestricted_mac_array = split(";",$unrestricted_mac_addr);
		foreach ($unrestricted_mac_array as $ind_unrestricted_mac) {
			fwrite($aclout, $ind_unrestricted_mac . "\n");
		}
			
		fclose($aclout);
		
		fwrite($fout, 'acl pf_unrestricted_mac src "/usr/local/etc/squid/advanced/acls/src_unrestricted_mac.acl"' . "\n");
	}
			
	/* define ip addresses that are banned from using the proxy service */
	if (isset($banned_ip_addr) && ($banned_ip_addr !== "")) {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);

		$aclout = fopen($acldir . "/src_banned_ip.acl","w");
			
		$banned_ip_array = split(";",$banned_ip_addr);
		foreach ($banned_ip_array as $ind_banned_ip) {
			fwrite($aclout, $ind_banned_ip . "\n");
		}
				
		fclose($aclout);
		
		fwrite($fout, 'acl pf_banned_ip src "/usr/local/etc/squid/advanced/acls/src_banned_ip.acl"' . "\n");
	}
			
	/* define mac addresses that are banned from using the proxy service */
	if (isset($banned_mac_addr) && ($banned_mac_addr !== "")) {
		if (!file_exists($acldir)) mwexec("/bin/mkdir -p " . $acldir);
	
			$aclout = fopen($acldir . "/src_banned_mac.acl","w");
			
			$banned_mac_array = split(";",$banned_mac_addr);
			foreach ($banned_mac_array as $ind_banned_mac) {
				fwrite($aclout, $ind_banned_mac . "\n");
			}
				
		fclose($aclout);
		
		fwrite($fout, 'acl pf_banned_mac src "/usr/local/etc/squid/advanced/acls/src_banned_mac.acl"' . "\n");
	}
	
	fwrite($fout, "acl pf_ips	dst " . $lanip . "\n");
	fwrite($fout, 'acl pf_networks	src "/usr/local/etc/squid/advanced/acls/src_subnets.acl"' . "\n");
    fwrite($fout, "acl CONNECT method CONNECT\n");
    fwrite($fout, "\n");
            
    fwrite($fout, "#access to squid; local machine; no restrictions\n");
    if (isset($auth_method) && ($auth_method == "none")) fwrite($fout, "http_access allow localnet\n");
    fwrite($fout, "http_access allow localhost\n");
	
	if (isset($override_hosts) && ($override_hosts !== "")) fwrite($fout, "http_access allow override_hosts\n");
	fwrite($fout, "\n");
	
	fwrite($fout, "#GUI admin to allow local connections\n");
	if ($config['system']['webgui']['protocol'] == "http")  fwrite($fout, "http_access allow pf_ips pf_networks pf_admin_port\n");
	if ($config['system']['webgui']['protocol'] == "https") fwrite($fout, "http_access allow CONNECT pf_ips pf_networks pf_admin_port\n");
       		
	fwrite($fout, "#Deny non web services\n");
    fwrite($fout, "http_access deny !Safe_ports\n");
    fwrite($fout, "http_access deny CONNECT !SSL_ports\n");
    fwrite($fout, "\n");
            
    fwrite($fout, "#Set custom configured ACLs\n");
    if (isset($auth_method) && ($auth_method != "none")) {
    	fwrite($fout, "http_access allow pf_networks for_inetusers within_timeframe\n");
    }
    
    fwrite($fout, "http_access deny all\n");
    fwrite($fout, "\n");
            
	if (isset($dl_overall) && ($dl_overall !== "") and isset($dl_per_host) && ($dl_per_host == "")) {
		fwrite($fout, "#Set throttle and bandwidth restrictions\n");
		
       	fwrite($fout, "delay_pools 1\n");
       	fwrite($fout, "delay_class 1 3\n");
       	
       	if ($dl_overall == "unlimited") {
       		fwrite($fout, "delay_parameters 1 -1/-1 -1/-1 " . ($dl_overall * 125) . "/" . ($dl_overall * 250) . "\n");
       	} else {
       		fwrite($fout, "delay_parameters 1 " . ($dl_overall * 125) . "/" . ($dl_overall * 250) . " -1/-1 -1/-1\n");
       	}
       	
   		/* if no unrestricted ip addresses are defined; this line is ignored */ 	      	
 		if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr == "")) fwrite($fout, "delay_access 1 deny pf_unrestricted_ip\n");
  	
       	fwrite($fout, "#delay_access 1 deny for_extended_users\n");
       	
   		/* this will define bandwidth delay restrictions for specified throttles */
   		if (isset($throttle_binary_files) && ($throttle_binary_files == "on")) {
   			fwrite($fout, "delay_access 1 allow all for_throttled_binary\n");
   		}
   		if (isset($throttle_cd_images) && ($throttle_cd_images == "on")) {
   			fwrite($fout, "delay_access 1 allow all for_throttled_cd\n");
   		}
   		if (isset($throttle_multimedia) && ($throttle_multimedia == "on")) {
   			fwrite($fout, "delay_access 1 allow all for_throttled_multimedia\n");
   		} else {
   			fwrite($fout, "delay_access 1 allow all\n");
   		}       	
   		fwrite($fout, "delay_initial_bucket_level 100%\n\n");
    }
            
    if (isset($dl_per_host) && ($dl_per_host !== "") and isset($dl_overall) && ($dl_overall == "")) {
		fwrite($fout, "#Set throttle and bandwidth restrictions\n");   
		
       	fwrite($fout, "delay_pools 1\n");
      	fwrite($fout, "delay_class 1 3\n");
      	
      	if ($dl_per_host == "unlimited") {
      		fwrite($fout, "delay_parameters 1 " . ($dl_per_host * 125) . "/" . ($dl_per_host * 250) . "-1/-1 -1/-1\n");
      	} else {
       		fwrite($fout, "delay_parameters 1 -1/-1 -1/-1 " . ($dl_per_host * 125) . "/" . ($dl_per_host * 250) . "\n");
       	}

   		/* if no unrestricted ip addresses are defined; this line is ignored */ 	
 		if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) fwrite($fout, "delay_access 1 deny pf_unrestricted_ip\n");
  		
       	fwrite($fout, "#delay_access 1 deny for_extended_users\n");
       	
   		/* this will define bandwidth delay restrictions for specified throttles */     	
   		if ($throttle_binary_files == "on") {
   			fwrite($fout, "delay_access 1 allow all for_throttled_binary\n");
   		}
   		if ($throttle_cd_images == "on") {
   			fwrite($fout, "delay_access 1 allow all for_throttled_cd\n");
   		}
   		if ($throttle_multimedia == "on") {
   			fwrite($fout, "delay_access 1 allow all for_throttled_multimedia\n");
   		} else {
   			fwrite($fout, "delay_access 1 allow all\n");
   		}
       	fwrite($fout, "delay_initial_bucket_level 100%\n\n");
		fwrite($fout, "\n");  	
    }
	        
    if (isset($dl_overall) && ($dl_overall !== "") and isset($dl_per_host) && ($dl_per_host !== "")) {
    	/* if no bandwidth restrictions are specified, then these parameters are not necessary */
    	if ($dl_overall !== "unlimited" and $dl_per_host !== "unlimited") { 	
			fwrite($fout, "#Set throttle and bandwidth restrictions\n");

   			if ((isset($dl_overall) && ($dl_overall == "unlimited")) and (isset($dl_per_host) && ($dl_per_host !== ""))) { 
   				fwrite($fout, "delay_pools 1\n");
   				fwrite($fout, "delay_class 1 3\n");
   				fwrite($fout, "delay_parameters 1 -1/-1 -1/-1 " . ($dl_per_host * 125) . "/" . ($dl_overall * 250) . "\n");
   			} elseif (isset($dl_overall) && ($dl_overall !== "") and isset($dl_per_host) && ($dl_per_host == "unlimited")) {
   				fwrite($fout, "delay_pools 1\n");
   				fwrite($fout, "delay_class 1 3\n");
   				fwrite($fout, "delay_parameters 1 " . ($dl_overall * 125) . "/" . ($dl_overall * 250) . " -1/-1 -1/-1\n");
   			}
   		}
   		
   		if ($dl_overall !== "unlimited" and $dl_per_host !== "unlimited") {
   		
   			/* if no unrestricted ip addresses are defined; this line is ignored */
   			if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) fwrite($fout, "delay_access 1 deny pf_unrestricted_ip\n");
 
   			fwrite($fout, "#delay_access 1 deny for_extended_users\n");
   		
   			/* this will define bandwidth delay restrictions for specified throttles */
   			if ($throttle_binary_files == "on") {
   				fwrite($fout, "delay_access 1 allow all for_throttled_binary\n");
   			}
   			if ($throttle_cd_images == "on") {
   				fwrite($fout, "delay_access 1 allow all for_throttled_cd\n");
   			}
   			if ($throttle_multimedia == "on") {
   				fwrite($fout, "delay_access 1 allow all for_throttled_multimedia\n");
   			} else {
   				fwrite($fout, "delay_access 1 allow all\n");
   			}
   			fwrite($fout, "delay_initial_bucket_level 100%\n\n");
   		}
    }

    fwrite($fout, "#Strip HTTP Header\n");
    fwrite($fout, "header_access X-Forwarded-For deny all\n");
    fwrite($fout, "header_access Via deny all\n");
    fwrite($fout, "\n");
            
	/* TODO: acl customization for snmp support */
	fwrite($fout, "\n");           

	if (isset($urlfilter_enable) && ($urlfilter_enable == "on")) {
		fwrite($fout, "redirect_program /usr/sbin/squidGuard");
		fwrite($fout, "redirect_children 5");
		fwrite($fout, "\n");
	}	
						
	if (isset($max_upload_size) && ($max_upload_size != "")) {
		fwrite($fout, "request_body_max_size " . $max_download_size . "KB\n");
	}
			
	if (isset($max_download_size) && ($max_download_size != "")) {
		if (isset($unrestricted_ip_addr) && ($unrestricted_ip_addr !== "")) fwrite($fout, "reply_body_max_size 0 allow pf_unrestricted_ip\n");
		fwrite($fout, "#reply_body_max_size 0 allow for_extended_users\n");
		fwrite($fout, "reply_body_max_size " . $max_download_size * 1024 . " allow all\n");
		fwrite($fout, "\n");
	}
			
	if ($maximum_object_size == "") $maximum_object_size="4096";
	if ($minimum_object_size == "") $minimum_object_size="0";
	fwrite($fout, "maximum_object_size " . $maximum_object_size . " KB\n");
	fwrite($fout, "minimum_object_size " . $minimum_object_size . " KB\n");
	fwrite($fout, "\n");
			
	if ($proxy_forwarding == "on") {
		fwrite($fout, "cache_peer " . $upstream_proxy . "parent " . $upstream_proxy_port . "3130 login=" . upstream_username . ":" . upstream_password . " default no-query\n");
		fwrite($fout, "never_direct allow all\n");
	}	
			
	if ($transparent_proxy == "on") {
       	fwrite($fout, "httpd_accel_host virtual\n");
      	fwrite($fout, "httpd_accel_port 80\n");
       	fwrite($fout, "httpd_accel_with_proxy on\n");
       	fwrite($fout, "httpd_accel_uses_host_header on\n");	
       	fwrite($fout, "\n");			
    }
    
	if (isset($visible_hostname) && ($visible_hostname !== "")) {
		fwrite($fout, "visible_hostname " . $visible_hostname . "\n");
	} 
			
	if (isset($cache_admin_email) && ($cache_admin_email !== "")) {
		fwrite($fout, "cache_mgr " . $cache_admin_email . "\n");
		fwrite($fout, "\n");
	}    
			
	fclose($fout);
			
	conf_mount_ro();
	config_unlock();
			
	touch($squidconfig);
} /* end function write_squid_config */

function mod_htpasswd() {
	conf_mount_rw();
	config_lock();
	global $config;
	
	if (!file_exists("/usr/local/etc/squid/advanced/ncsa")) mwexec("mkdir -p /usr/local/etc/squid/advanced/ncsa");
	
	$passfile = fopen("/usr/local/etc/squid/advanced/ncsa/passwd", "w+");

	if($config['installedpackages']['squidextlocalauth']['config'] != "") {
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

		
