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
            
    /* squid_ng.xml values */
    $active_interface = $config['installedpackages']['squidng']['config'][0]['active_interface'];
    $transparent_proxy = $config['installedpackages']['squidng']['config'][0]['transparent_proxy'];
    $log_enabled = $config['installedpackages']['squidng']['config'][0]['log_enabled'];
    $urlfilter_enable = $config['installedpackages']['squidng']['config'][0]['urlfilter_enable'];
    $log_query_terms = $config['installedpackages']['squidng']['config'][0]['log_query_terms'];
    $log_user_agents = $config['installedpackages']['squidng']['config'][0]['log_user_agents'];
    $proxy_port = $config['installedpackages']['squidng']['config'][0]['proxy_port'];
    $visible_hostname = $config['installedpackages']['squidng']['config'][0]['visible_hostname'];
    $cache_admin_email = $config['installedpackages']['squidng']['config'][0]['cache_admin_email'];
    $error_language = $config['installedpackages']['squidng']['config'][0]['error_language'];
            
    /* squid_upstream.xml values */
    $proxy_forwarding = $config['installedpackages']['squidupstream']['config'][0]['proxy_forwarding'];
    $client_ip_forwarding = $config['installedpackages']['squidupstream']['config'][0]['client_ip_forwarding'];
    $user_forwarding = $config['installedpackages']['squidupstream']['config'][0]['user_forwarding'];
    $upstream_proxy = $config['installedpackages']['squidupstream']['config'][0]['upstream_proxy'];
    $upstream_proxy_port = $config['installedpackages']['squidupstream']['config'][0]['upstream_proxy_port'];
    $upstream_username = $config['installedpackages']['squidupstream']['config'][0]['upstream_username'];
    $upstream_password = $config['installedpackages']['squidupstream']['config'][0]['upstream_psasword'];
            
    /* squidcache.xml values */
    $memory_cache_size = $config['installedpackages']['squidcache']['config'][0]['memory_cache_size'];
    $harddisk_cache_size = $config['installedpackages']['squidcache']['config'][0]['harddisk_cache_size'];
    $minimum_object_size = $config['installedpackages']['squidcache']['config'][0]['minimum_object_size'];
    $maximum_object_size = $config['installedpackages']['squidcache']['config'][0]['maximum_object_size'];
    $level_subdirs = $config['installedpackages']['squidcache']['config'][0]['level_subdirs'];
    $memory_replacement = $config['installedpackages']['squidcache']['config'][0]['memory_replacement'];
    $cache_replacement = $config['installedpackages']['squidcache']['config'][0]['cache_replacement'];
    $domain = $config['installedpackages']['squidcache']['config'][0]['domain'];
    $enable_offline = $config['installedpackages']['squidcache']['config'][0]['enable_offline'];
            
    /* squidnac.xml values */
    $allowed_subnets = $config['installedpackages']['squidnac']['config'][0]['allowed_subnets'];
    $unrestricted_ip_address = $config['installedpackages']['squidnac']['config'][0]['unrestricted_ip_address'];
    $unrestricted_mac_addresses = $config['installedpackages']['squidnac']['config'][0]['unrestricted_mac_addresses'];
    $banned_ip_addresses = $config['installedpackages']['squidnac']['config'][0]['banned_ip_addresses'];
    $banned_mac_addresses = $config['installedpackages']['squidnac']['config'][0]['banned_mac_addresses'];
           
    /* squidtraffic.xml values */
    $max_download_size = $config['installedpackages']['squidtraffic']['config'][0]['max_download_size'];
    $max_upload_size = $config['installedpackages']['squidtraffic']['config'][0]['max_upload_size'];
    $dl_overall = $config['installedpackages']['squidtraffic']['config'][0]['dl_overall'];
    $dl_per_host = $config['installedpackages']['squidtraffic']['config'][0]['dl_per_host'];
    $throttle_binary_files = $config['installedpackages']['squidtraffic']['config'][0]['throttle_binary_files'];
    $throttle_cd_images = $config['installedpackages']['squidtraffic']['config'][0]['throttle_cd_images'];
    $throttle_multimedia = $config['installedpackages']['squidtraffic']['config'][0]['throttle_multimedia'];
                             
    /* squidauth.xml values (placeholder for now) */
	$no_auth = $config['installedpackages']['squidtraffic']['config'][0]['no_auth'];
	$local_auth = $config['installedpackages']['squidtraffic']['config'][0]['local_auth'];
	$ldap_auth = $config['installedpackages']['squidtraffic']['config'][0]['ldap_auth'];
	$windows_auth = $config['installedpackages']['squidtraffic']['config'][0]['windows_auth'];
	$radius_auth = $config['installedpackages']['squidtraffic']['config'][0]['radius_auth'];
	$auth_processes = $config['installedpackages']['squidtraffic']['config'][0]['auth_processes'];
	$auth_cache_ttl = $config['installedpackages']['squidtraffic']['config'][0]['auth_cache_ttl'];
	$limit_ip_addr = $config['installedpackages']['squidtraffic']['config'][0]['limit_ip_addr'];
	$user_ip_cache_ttl = $config['installedpackages']['squidtraffic']['config'][0]['user_ip_cache_ttl'];
	$req_unrestricted_auth = $config['installedpackages']['squidtraffic']['config'][0]['req_unrestricted_auth'];
	$auth_realm_prompt = $config['installedpackages']['squidtraffic']['config'][0]['auth_realm_prompt'];
	$no_domain_auth = $config['installedpackages']['squidtraffic']['config'][0]['no_domain_auth'];
	$min_pass_length = $config['installedpackages']['squidtraffic']['config'][0]['min_pass_length'];
	$bypass_extended = $config['installedpackages']['squidtraffic']['config'][0]['bypass_extended'];
			
    $fout = fopen($squidconfig,"w");
            
    /* option: shutdown_lifetime */
    fwrite($fout, "shutdown_lifetime 5 seconds\n");
    fwrite($fout, "\n");

	/* option: icp_port */	
	if($icp_port == "") $icp_port="3130"; 
	fwrite($fout, "icp_port " . $icp_port . "\n");
		
	/* option: http_port */
	if($http_port == "") $http_port="3128";
	if($config['installedpackages']['squidng']['config'][0]['active_interface'] == "LAN") {
		$listen_ip = find_interface_ip($config['interfaces']['lan']['if']);
	} elseif($int == "WAN") {
		$listen_ip = find_interface_ip($config['interfaces']['wan']['if']);
	} else {
		$int = convert_friendly_interface_to_real_interface_name($config['installedpackages']['squidng']['config'][0]['active_interface']);
		$listen_ip = find_interface_ip($int);
	}
	fwrite($fout, "http_port " . $listen_ip . ":" . $http_port . "\n");
	fwrite($fout, "\n");
			
	fwrite($fout, "acl QUERY urlpath_regex cgi-bin \?\n");
	fwrite($fout, "no_cache deny QUERY\n");		
			
	if ($domain !== "") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
			$aclout = fopen("/usr/local/etc/squid/acls/dst_nocache.acl","w");
			
			$domain_array = split(";",$domain);
			foreach ($domain_array as $no_cache_domain) {
			fwrite($aclout, $no_cache_domain . "\n");
		}
				
		fclose($aclout);
		
		fwrite($fout, 'acl no_cache_domains dstdomain "/usr/local/etc/squid/acls/dst_nocache.acl"' . "\n");
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
	fwrite($fout, "cache_dir aufs /var/squid/cache " . $harddisk_cache_size . " " . $level_subdirs . " 256\n");
	fwrite($fout, "\n");
			
	if ($error_language == "") $error_language="English";
	fwrite($fout, "error_directory /usr/local/etc/squid/errors/" . $error_language . "\n");
	fwrite($fout, "\n");
			
	if ($offline_mode == "on") {
		fwrite($fout, "offline_mode on\n");
		fwrite($fout, "\n");
	}
			
	if ($memory_replacement == "") $memory_replacement="heap GDSF";
	fwrite($fout, "memory_replacement_policy " . $memory_replacement . "\n");
	if ($cache_replacement == "") $cache_replacement="heap GDSF";
	fwrite($fout, "cache_replacement_policy " . $cache_replacement . "\n");
	fwrite($fout, "\n");
			
	if ($log_enabled == "on" ) {
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
		fwrite($fout, "useragent_log /var/squid/logs/useragent.log\n");
	} 
	fwrite($fout, "\n");
			
	fwrite($fout, "log_mime_hdrs off\n");
	fwrite($fout, "emulate_httpd_log on\n");
	if ($client_ip_forwarding !== "on") { 
		fwrite($fout, "forwarded_for off\n");
	} elseif ($user_forwarding !== "on") {
		fwrite($fout, "forwarded_for off\n");
	} else {
		fwrite($fout, "forwarded_for on\n");
	} 
	fwrite($fout, "\n");
	
	if ($no_auth == "on") {
		fwrite($fout, "\n");
	}
			
	if ($local_auth == "on") {
		fwrite($fout, "auth_param basic program /usr/local/libexec/squid/ncsa_auth /usr/local/etc/squid/advanced/ncsa/passwd\n");
		fwrite($fout, "auth_param basic children 5\n");
		fwrite($fout, "auth_param basic realm pfSense Advanced Proxy Service\n");
		fwrite($fout, "auth_param basic credentialsttl 60 minutes\n");
		fwrite($fout, "\n");
	}
		
	/* TODO: placeholder for local user management */
		
	if ($throttle_binary_files == "on") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
			$binary_out = "\.bin$\n\.cab$\n\.gz$\n\.rar$\n\.sea$\n\.tar$\n\.tgz$\n\.zip$\n";	

			$throttle_out = fopen("/usr/local/etc/squid/acls/dst_throttle_binary.acl","w");
			fwrite($throttle_out, $binary_out);
			fwrite($fout, 'acl for_throttled_binary url_regex -i "/usr/local/etc/squid/acls/dst_throttle_binary.acl"' . "\n");
			fclose($throttle_out);
		} else {
			if (file_exists("/usr/local/etc/squid/acls/dst_throttle_binary.acl")) unlink("/usr/local/etc/squid/acls/dst_throttle_binary.acl");
	}
			
	if ($throttle_cd_images == "on") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
				
		$cd_out = "\.b5t$\n\.bin$\n\.bwt$\n\.cdi$\n\.cue$\n\.gho$\n\.img$\n\.iso$\n\.mds$\n\.nrg$\n\.pqi$\n";
				
		$throttle_out = fopen("/usr/local/etc/squid/acls/dst_throttle_cd.acl","w");
		fwrite($throttle_out, $cd_out);
		fwrite($fout, 'acl for_throttled_cd url_regex -i "/usr/local/etc/squid/acls/dst_throttle_cd.acl"' . "\n");
		fclose($throttle_out);
	} else {
		if (file_exists("/usr/local/etc/squid/acls/dst_throttle_cd.acl")) unlink("/usr/local/etc/squid/acls/dst_throttle_cd.acl");
	}
			
	if ($throttle_multimedia == "on") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
			
		$multimedia_out = "\.aiff?$\n\.asf$\n\.avi$\n\.divx$\n\.mov$\n\.mp3$\n\.mpe?g$\n\.qt$\n\.ra?m$\n";			
			
		$throttle_out = fopen("/usr/local/etc/squid/acls/dst_throttle_multimedia.acl","w");
		fwrite($throttle_out, $multimedia_out);
		fwrite($fout, 'acl for_throttled_multimedia url_regex -i "/usr/local/etc/squid/acls/dst_throttle_multimedia.acl"' . "\n");
		fclose($throttle_out);
	} else {
		if (file_exists("/usr/local/etc/squid/acls/dst_throttle_multimedia.acl")) unlink("/usr/local/etc/squid/acls/dst_throttle_multimedia.acl");			
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
	fwrite($fout, "acl Safe_ports port 80 # http\n");
	fwrite($fout, "acl Safe_ports port 21 # ftp\n");
	fwrite($fout, "acl Safe_ports port 443 563 # https, snews\n");
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
	if ($custom_port !== "") {
		fwrite($fout, "acl pf_admin_port port " . $custom_port . "\n");
	}
			
	/* define subnets allowed to utilize proxy service */
	if ($allowed_subnets !== "") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
	
		$aclout = fopen("/usr/local/etc/squid/acls/src_subnets.acl","w");
				
		$allowed_subnets_array = split(";",$allowed_subnets);
		foreach ($allowed_subnets_array as $ind_allowed_subnets) {
			fwrite($aclout, $ind_allowed_subnets . "\n");
		}
				
		fclose($aclout);
				
		fwrite($fout, 'acl pf_networks src "/usr/local/etc/squid/acls/src_subnets.acl"' . "\n");
	}
			
	/* define ip addresses that have 'unrestricted' access */
	if ($unrestricted_ip_address !== "") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
			$aclout = fopen("/usr/local/etc/squid/acls/src_unrestricted_ip.acl","w");
		
			$unrestricted_ip_array = split(";",$unrestricted_ip_address);
			foreach ($unrestricted_ip_array as $ind_unrestricted_ip) {
				fwrite($aclout, $ind_unrestricted_ip . "\n");
		}
				
		fclose($aclout);
		
		fwrite($fout, 'acl pf_unrestricted_ip src "/usr/local/etc/squid/acls/src_unrestricted_ip.acl"' . "\n");
	}
			
	/* define mac addresses that have 'unrestricted' access */
	if ($unrestricted_mac_addresses !== "") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");

		$aclout = fopen("/usr/local/etc/squid/acls/src_unrestricted_mac.acl","w");
			
		$unrestricted_mac_array = split(";",$unrestricted_mac_addresses);
		foreach ($unrestricted_mac_array as $ind_unrestricted_mac) {
			fwrite($aclout, $ind_unrestricted_mac . "\n");
		}
			
		fclose($aclout);
		
		fwrite($fout, 'acl pf_unrestricted_mac src "/usr/local/etc/squid/acls/src_unrestricted_mac.acl"' . "\n");
	}
			
	/* define ip addresses that are banned from using the proxy service */
	if ($banned_ip_addresses !== "") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");

			$aclout = fopen("/usr/local/etc/squid/acls/src_banned_ip.acl","w");
			
			$banned_ip_array = split(";",$banned_ip_addresses);
			foreach ($banned_ip_array as $ind_banned_ip) {
				fwrite($aclout, $ind_banned_ip . "\n");
		}
				
			fclose($aclout);
		
		fwrite($fout, 'acl pf_banned_ip src "/usr/local/etc/squid/acls/src_banned_ip.acl"' . "\n");
	}
			
	/* define mac addresses that are banned from using the proxy service */
	if ($banned_mac_addresses !== "") {
		if (!file_exists("/usr/local/etc/squid/acls")) mwexec("/bin/mkdir -p /usr/local/etc/squid/acls");
	
			$aclout = fopen("/usr/local/etc/squid/acls/src_banned_mac.acl","w");
			
			$banned_mac_array = split(";",$banned_mac_addresses);
			foreach ($banned_mac_array as $ind_banned_mac) {
				fwrite($aclout, $ind_banned_mac . "\n");
			}
				
		fclose($aclout);
		
		fwrite($fout, 'acl pf_banned_mac src "/usr/local/etc/squid/acls/src_banned_mac.acl"' . "\n");
	}
			
    fwrite($fout, "acl CONNECT method CONNECT\n");
    fwrite($fout, "\n");
            
    fwrite($fout, "#access to squid; local machine; no restrictions\n");
    fwrite($fout, "http_access allow localnet\n");
    fwrite($fout, "http_access allow localhost\n");
	fwrite($fout, "\n");
       		
	fwrite($fout, "#Deny non web services\n");
    fwrite($fout, "http_access deny !Safe_ports\n");
    fwrite($fout, "http_access deny CONNECT\n");
    fwrite($fout, "\n");
            
    fwrite($fout, "#Set custom configured ACLs\n");
    fwrite($fout, "http_access deny all\n");
    fwrite($fout, "\n");
            
	if ($dl_overall !== "" and $dl_per_host == "") {
		fwrite($fout, "#Set throttle and bandwidth restrictions\n");
		
       	fwrite($fout, "delay_pools 1\n");
       	fwrite($fout, "delay_class 1 3\n");
       	
       	if ($dl_overall == "unlimited") {
       		fwrite($fout, "delay_parameters 1 -1/-1 -1/-1 " . ($dl_overall * 125) . "/" . ($dl_overall * 250) . "\n");
       	} else {
       		fwrite($fout, "delay_parameters 1 " . ($dl_overall * 125) . "/" . ($dl_overall * 250) . " -1/-1 -1/-1\n");
       	}
       	
   		/* if no unrestricted ip addresses are defined; this line is ignored */ 	      	
 		if ($unrestricted_ip_address == "") fwrite($fout, "delay_access 1 deny pf_unrestricted_ip\n");
  	
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
            
    if ($dl_per_host !== "" and $dl_overall == "") {
		fwrite($fout, "#Set throttle and bandwidth restrictions\n");   
		
       	fwrite($fout, "delay_pools 1\n");
      	fwrite($fout, "delay_class 1 3\n");
      	
      	if ($dl_per_host == "unlimited") {
      		fwrite($fout, "delay_parameters 1 " . ($dl_per_host * 125) . "/" . ($dl_per_host * 250) . "-1/-1 -1/-1\n");
      	} else {
       		fwrite($fout, "delay_parameters 1 -1/-1 -1/-1 " . ($dl_per_host * 125) . "/" . ($dl_per_host * 250) . "\n");
       	}

   		/* if no unrestricted ip addresses are defined; this line is ignored */ 	
 		if ($unrestricted_ip_address !== "") fwrite($fout, "delay_access 1 deny pf_unrestricted_ip\n");
  		
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
	        
    if ($dl_overall !== "" and $dl_per_host !== "") {    
    	/* if no bandwidth restrictions are specified, then these parameters are not necessary */
    	if ($dl_overall !== "unlimited" and $dl_per_host !== "unlimited") { 	
			fwrite($fout, "#Set throttle and bandwidth restrictions\n");

   			if ($dl_overall == "unlimited" and $dl_per_host !== "") { 
   				fwrite($fout, "delay_pools 1\n");
   				fwrite($fout, "delay_class 1 3\n");
   				fwrite($fout, "delay_parameters 1 -1/-1 -1/-1 " . ($dl_per_host * 125) . "/" . ($dl_overall * 250) . "\n");
   			} elseif ($dl_overall !== "" and $dl_per_host == "unlimited") {
   				fwrite($fout, "delay_pools 1\n");
   				fwrite($fout, "delay_class 1 3\n");
   				fwrite($fout, "delay_parameters 1 " . ($dl_overall * 125) . "/" . ($dl_overall * 250) . " -1/-1 -1/-1\n");
   			}
   		}
   		
   		if ($dl_overall !== "unlimited" and $dl_per_host !== "unlimited") {
   		
   			/* if no unrestricted ip addresses are defined; this line is ignored */
   			if ($unrestricted_ip_address !== "") fwrite($fout, "delay_access 1 deny pf_unrestricted_ip\n");
 
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
	fwrite($fout, "snmp_access deny all\n");
	fwrite($fout, "\n");           

	if ($urlfilter_enable == "on") {
		fwrite($fout, "redirect_program /usr/sbin/squidGuard");
		fwrite($fout, "redirect_children 5");
		fwrite($fout, "\n");
	}	
						
	if ($max_upload_size != "") {
		fwrite($fout, "request_body_max_size " . $max_download_size . "KB\n");
	}
			
	if ($max_download_size != "") {
		if ($unrestricted_ip_addresses !== "") fwrite($fout, "reply_body_max_size 0 allow pf_unrestricted_ip\n");
		fwrite($fout, "#reply_body_max_size 0 allow for_extended_users\n");
		fwrite($fout, "reply_body_max_size " . $max_download_size * 1024 . " allow all\n");
		fwrite($fout, "\n");
	}
			
	if ($visible_hostname !== "") {
		fwrite($fout, "visible_hostname " . $visible_hostname . "\n");
	} 
			
	if ($cache_admin_email !== "") {
		fwrite($fout, "cache_mgr " . $cache_admin_email . "\n");
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
			
	fclose($fout);
			
	conf_mount_ro();
	config_unlock();
			
	touch($squidconfig);
} /* end function write_squid_config */
		
