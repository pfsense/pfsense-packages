<?php
/*
	apache_mod_security.inc
	part of apache_mod_security package (http://www.pfSense.com)
	Copyright (C) 2009, 2010 Scott Ullrich
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

// Rules directory location
define("rules_directory", "/usr/local/apachemodsecurity/rules");

// Ensure NanoBSD can write. pkg_mgr will remount RO
conf_mount_rw();

// Needed mod_security directories
if(!is_dir("/usr/local/apachemodsecurity"))  	
	safe_mkdir("/usr/local/apachemodsecurity");
if(!is_dir("/usr/local/apachemodsecurity/rules")) 
	safe_mkdir("/usr/local/apachemodsecurity/rules");

// Startup function	
function apache_mod_security_start() {
	exec("/usr/local/sbin/httpd -k start");
}

// Shutdown function
function apache_mod_security_stop() {
	exec("/usr/local/sbin/httpd -k stop");
}

// Restart function
function apache_mod_security_restart() {
	if(is_process_running("httpd")) {
		exec("/usr/local/sbin/httpd -k graceful");
	} else {
		apache_mod_security_start();		
	}
}

// Install function
function apache_mod_security_install() {
	global $config, $g;

	// We might be reinstalling and a configuration
	// already exists.
	generate_apache_configuration();

	$filename = "apache_mod_security.sh";

	$start = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
	<?php
		require_once(\"functions.inc\");
		require_once(\"/usr/local/pkg/apache_mod_security.inc\");
		apache_mod_security_start();
	?>
ENDPHP\n";

	$stop = "/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDPHP
	<?php
		require_once(\"functions.inc\");
		require_once(\"/usr/local/pkg/apache_mod_security.inc\");
		apache_mod_security_stop();
	?>
ENDPHP\n";
	
	write_rcfile(array(
			"file" => $filename,
			"start" => $start,
			"stop" => $stop
		)
	);
}

// Deinstall package routines
function apache_mod_security_deinstall() {
	global $config, $g;
	apache_mod_security_stop();
	exec("/bin/rm -rf /usr/local/apachemodsecurity");
	exec("/bin/rm -f /usr/local/etc/rc.d/apache_mod_security.sh");
}

// Regenerate apache configuration and handle server restart
function apache_mod_security_resync() {
	global $config, $g;
	apache_mod_security_install();
	if(!file_exists(rules_directory . "/10_asl_rules.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/10_asl_rules.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/10_asl_rules.conf");
	if(!file_exists(rules_directory . "/a_exclude.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/a_exclude.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/a_exclude.conf");
	if(!file_exists(rules_directory . "/blacklist.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/blacklist.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/blacklist.conf");
	if(!file_exists(rules_directory . "/default.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/rules/default.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/default.conf");
	if(!file_exists(rules_directory . "/recons.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/recons.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/recons.conf");
	if(!file_exists(rules_directory . "/rootkits.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/rootkits.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/rootkits.conf");
	if(!file_exists(rules_directory . "/useragents.conf"))
		exec("/usr/bin/fetch -q -o " . rules_directory . "/useragents.conf https://packages.pfsense.org/packages/config/apache_mod_security/rules/useragents.conf");
	apache_mod_security_checkconfig();
	apache_mod_security_restart();
}

function apache_mod_security_checkconfig() {
	global $config, $g;
	$status = mwexec("/usr/local/sbin/httpd -t");
	if($status) {
		$input_error = "apache_mod_security_package: There was an error parsing the Apache configuration: {$status}";
		log_error("apache_mod_security_package: There was an error parsing the Apache configuration: {$status}");
	}
}

// Generate mod_proxy specific configuration
function generate_apache_configuration() {
	global $config, $g;
	$mod_proxy = "";

	// Set global site e-mail
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalsiteadminemail']) {
		$global_site_email = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalsiteadminemail'];
	} else {
                $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalsiteadminemail'] = "admin@admin.com";
                $global_site_email = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalsiteadminemail'];
                // update configuration with default value in this case
                write_config($pkg['addedit_string']);
		log_error("WARNING!  Global site Administrator E-Mail address has not been set.  Defaulting to bogus e-mail address.");
	}

	// Set ServerName
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['hostname']) {
		$servername = "ServerName {$config['installedpackages']['apachemodsecuritysettings']['config'][0]['hostname']}\n";
	} else {
		$servername = "ServerName " . `hostname` . "\n";
		$config['installedpackages']['apachemodsecuritysettings']['config'][0]['hostname'] = `hostname`;
                // update configuration with default value in this case
                write_config($pkg['addedit_string']);
	}

	// Set global listening directive and ensure nothing is listening on this port already
	$globalbind = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoipaddr'];
	$socksstat = split("\n", `/usr/bin/sockstat | awk '{ print $6 }' | grep ":{$globalbind}" | cut -d ":" -f2`);
	if(is_array($socksstat)) {
		foreach($socksstat as $ss) {
			if($ss == $globalbind) {
				$already_binded = true;
				$input_errors[] = "Sorry, there is a process already listening on port {$globalbind}";
			}
		}
	}
//	if(!$already_binded) {
		if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoipaddr']) {
			$global_listen  =  $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoipaddr'];
			if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoport'])
				$global_listen .= ":" . $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoport'];
			else 
				$global_listen .= ":80";
		} else {
 	               	$config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoipaddr'] = "";
        	        $global_listen  =  $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoipaddr'];
			// update configuration with default value in this case
			 write_config($pkg['addedit_string']);		
			if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoport'])
				$global_listen .= ":" . $config['installedpackages']['apachemodsecuritysettings']['config'][0]['globalbindtoport'];
			else 
				$global_listen .= ":80";
		}
//	} else {
//		log_error("Could not start mod_security + mod_proxy on port {$global_listen}.  Process is already bound to this port.");
//	}
	
	// Setup mem_cache
	if(file_exists("/usr/local/libexec/apache22/mod_mem_cache.so")) {
		if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['mod_mem_cache']) {
			if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['mod_mem_cache_size'])
				$mcachesize = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['mod_mem_cache_size'];
			else 
				$mcachesize = "100";
			//$mem_cache = "MCacheSize $mcachesize\n";
		}
	}

	// CacheRoot Directive
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['mod_disk_cache']) {
		safe_mkdir("/var/db/apachemodsecuritycache");
		$cache_root .= "CacheRoot /var/db/apachemodsecuritycache\n";
	}

	// SecRequestBodyInMemoryLimit Directive
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['secrequestbodyinmemorylimit']) 
		$secrequestbodyinmemorylimit = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['secrequestbodyinmemorylimit'];
	else 
		$secrequestbodyinmemorylimit = "131072";

	// SecRequestBodyLimit
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['secrequestbodylimit']) 
		$secrequestbodylimit = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['secrequestbodylimit'];
	else 
		$secrequestbodylimit = "10485760";

	// ErrorDocument
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['errordocument']) 
		$errordocument = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['errordocument'];
	else 
		$errordocument = "";

	// SecAuditEngine
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['secauditengine']) 
		$secauditengine = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['secauditengine'];
	else 
		$secauditengine = "RelevantOnly";

	// SecReadStateLimit
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['SecReadStateLimit']) 
		$secreadstatelimit = "SecReadStateLimit " . $config['installedpackages']['apachemodsecuritysettings']['config'][0]['SecReadStateLimit'] ."\n";
	else 
		$secreadstatelimit = "";

	$mod_proxy .= <<<EOF

# Off when using ProxyPass
ProxyRequests off

<Proxy *>
	Order deny,allow
	Allow from all
</Proxy>

EOF;

	/*  
		#####################################################
		# Format for the Proxy servers:
		# Please do not delete these from the source file
		# in any "cleanups" that you feel you are performing.
		# They are here for sanity checking the code.
		#----------------1 backend ip---------------------
		#<VirtualHost IP:port>
		#  ServerAdmin $serveradmin
		#  ServerName $primarysitehostname
		#  ServerAlias $additionalsitehostnames
		#  ProxyPass / $backendwebserverURL
		#  ProxyPassReverse / $backendwebserverURL
		#</VirtualHost>
		#where serveralias will be a space-separated list of "additional site hostnames"
		#i.e. if you have www1.example.com and www2.example.com there, it's "ServerAlias www1.example.com www2.example.com"
		#-------------------------------------------------
		#------------mutliple backend ips-----------------
		# Add:
		#<Proxy balancer://$sitename>
		#  BalancerMember $backend1
		#  BalancerMember $backend2
		#</Proxy>
		# Change:
		# ProxyPass / balancer://$sitename/
		# ProxyPassReverse / balancer://$sitename/
		#-------------------------------------------------
		#####################################################
	*/
	$mod_proxy .= "\n";

	$configuredaliases = array();
	// Read already configured addresses
        if($config['installedpackages']['apachemodsecuritysettings']['config']['0']) {
		foreach($config['installedpackages']['apachemodsecuritysettings']['config']['0']['row'] as $row) {
                	if ($row['ipaddress'] && $row['ipport']) {
				$configuredaliases[] = $row; 
                        }
               	}
	} 


	// clear list of bound addresses
        $config['installedpackages']['apachemodsecuritysettings']['config'][0]['row'] = array();

	// Process proxy sites
	// Configure NameVirtualHost directives
        $aliases = "";
	$processed = array();

	if($config['installedpackages']['apachemodsecurity']) {
		foreach($config['installedpackages']['apachemodsecurity']['config'] as $ams) {
			if($ams['ipaddress'] && $ams['port']) 
				$local_ip_port = "{$ams['ipaddress']}:{$ams['port']}";
			else 
				$local_ip_port = $global_listen;
			// Do not add entries twice.
			if(!in_array($local_ip_port, $processed)) {
				// explicit bind if not global ip:port
				if ($local_ip_port != $global_listen) {
					$aliases .= "Listen $local_ip_port\n";
					// Automatically add this to configuration
                                	$config['installedpackages']['apachemodsecuritysettings']['config'][0]['row'][] = array('ipaddress' => $ams['ipaddress'], 'ipport' => $ams['port']); 
				}
				$mod_proxy .= "NameVirtualHost $local_ip_port\n";
				$processed[] = $local_ip_port;
			}
		}
	}

//** Uncomment to allow adding ip/ports not used by any site proxies
//** Otherwise unused addresses/ports will be automatically deleted from the configuration
//	foreach ($configuredaliases as $ams) {
//		$local_ip_port = "{$ams['ipaddress']}:{$ams['ipport']}";
//               if(!in_array($local_ip_port, $processed)) {
//                	// explicit bind if not global ip:port
//                        if ($local_ip_port != $global_listen) {
//                        	$aliases .= "Listen $local_ip_port\n";    
//                                // Automatically add this to configuration
//                                $config['installedpackages']['apachemodsecuritysettings']['config'][0]['row'][] = array('ipaddress' => $ams['ipaddress'], 'ipport' => $ams['ipport']);
//                        }
//		}
//	}

	// update configuration with actual ip bindings
	write_config($pkg['addedit_string']);


	// Setup mod_proxy entries $mod_proxy
	if($config['installedpackages']['apachemodsecurity']) {
		foreach($config['installedpackages']['apachemodsecurity']['config'] as $ams) {
			// Set rowhelper used variables
                        $additionalsitehostnames = "";
			foreach($ams['row'] as $row) {
				if ($row['additionalsitehostnames']) {
					$additionalsitehostnames .= "{$row['additionalsitehostnames']} ";
			}				
			}				
			$backend_sites = ""; 
			$sslproxyengine = ""; 
			$backend_sites_count = 0;
			$balancer_members = ""; // not technically needed.
			foreach($ams['row'] as $row) {
				if ($row['webserveripaddr']) {
					$normalised_ipaddr = "";
					if (substr(trim($row['webserveripaddr']), 0, strlen("https:")) == "https:") {
						// if backend is https, then enable SSLProxyEngine
						$sslproxyengine = "SSLProxyEngine on";
					} else if (substr(trim($row['webserveripaddr']), 0, strlen("http:")) != "http:") {
				// Ensure leading http(s)://
						$normalised_ipaddr .= "http://";
					}
					$normalised_ipaddr .=  trim($row['webserveripaddr']);				
					$balancer_members .= " BalancerMember " . $normalised_ipaddr . "\n";
				// Ensure trailing /
					if(substr($normalised_ipaddr,-1) != "/") {
						$normalised_ipaddr .= "/";
					}
					$backend_sites .= $normalised_ipaddr . " ";
                                        $backend_sites_count++;
				}
			}
			// Set general items
			if($ams['siteemail'])
				$serveradmin = $ams['siteemail'];
			else 
				$serveradmin = $global_site_email;
			if($ams['primarysitehostname'])
				$primarysitehostname = $ams['primarysitehostname'];
			$sitename = str_replace(" ", "", $ams['sitename']);			
			// Set local listening directive
			if($ams['ipaddress'] && $ams['port']) 
				$local_ip_port = "{$ams['ipaddress']}:{$ams['port']}";
			else 
				$local_ip_port = $global_listen;
			// Is this item a load balancer
			if($backend_sites_count>1) {
				$balancer = true;
				$mod_proxy .= "<Proxy balancer://{$sitename}>\n";
				$mod_proxy .= $balancer_members;
				$mod_proxy .= "</Proxy>\n";
				$backend_sites = " balancer://{$sitename}/";
				$sitename = "";			// we are not using sitename in this case
			}
			// Set SSL items
			if($ams['siteurl'])
				$siteurl = $ams['siteurl'];
			if($ams['certificatefile'])
				$certificatefile = $ams['certificatefile'];
			if($ams['certificatekeyfile'])
				$certificatekeyfile = $ams['certificatekeyfile'];
			if($ams['certificatechainfile'])
				$certificatechainfile = $ams['certificatechainfile'];
			// Begin VirtualHost
			$mod_proxy .= "\n<VirtualHost {$local_ip_port}>\n";
			if($siteurl == "HTTPS" && $certificatefile && $certificatekeyfile) {
				$mod_proxy .= "  SSLEngine on\n";
				if ($certificatefile)
					$mod_proxy .= "  SSLCertificateFile /usr/local/etc/apache22/$certificatefile\n";  
				if ($certificatekeyfile)
					$mod_proxy .= "  SSLCertificateKeyFile /usr/local/etc/apache22/$certificatekeyfile\n";
				if ($certificatechainfile)
					$mod_proxy .= "  SSLCertificateChainFile /usr/local/etc/apache22/$certificatechainfile\n";
			}
			if($sslproxyengine)
				$mod_proxy .= "  {$sslproxyengine}\n";
			if($additionalsitehostnames)
				$mod_proxy .= "  ServerAlias $additionalsitehostnames\n";
			if($serveradmin)
				$mod_proxy .= "  ServerAdmin $serveradmin\n";
			if($primarysitehostname)
				$mod_proxy .= "  ServerName $primarysitehostname \n";						
			if($backend_sites) {
				$mod_proxy .= "  ProxyPassReverse /{$sitename}  {$backend_sites}\n";
				$mod_proxy .= "  ProxyPass / {$backend_sites}\n";
			}
			if($ams['preserveproxyhostname']) 
				$mod_proxy .= "  ProxyPreserveHost on\n";
			$mod_proxy .= "</VirtualHost>\n\n";
			// End VirtualHost			
		}
	}

	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['modsecuritycustom'])  
		$mod_security_custom = $config['installedpackages']['apachemodsecuritysettings']['config'][0]['modsecuritycustom'];

	// Process and include rules
	if(is_dir(rules_directory)) {
		$mod_security_rules = "";
		$files = return_dir_as_array(rules_directory);
		foreach($files as $file) { 
			if(file_exists(rules_directory . "/" . $file)) {
				// XXX: TODO integrate snorts rule on / off thingie
				$file_txt = file_get_contents(rules_directory . "/" . $file);
				$mod_security_rules .= $file_txt . "\n";
			}
		}
	}

	// Mod_security enabled?
	if($config['installedpackages']['apachemodsecuritysettings']['config'][0]['enablemodsecurity'])  {
		$enable_mod_security = true;
		$mod_security = <<< EOF
<IfModule mod_security.c>

    # Turn the filtering engine On or Off
    SecFilterEngine On

	# XXX Add knobs for these
	SecRuleEngine On
	SecRequestBodyAccess On
	SecResponseBodyAccess On

	SecRequestBodyInMemoryLimit {$secrequestbodyinmemorylimit}
	SecRequestBodyLimit {$secrequestbodylimit}

	{$mod_security_custom}

	SecResponseBodyMimeTypesClear
	SecResponseBodyMimeType (null) text/plain text/html text/css text/xml

	# XXX Add knobs for these
	SecUploadDir /var/spool/apache/private
	SecUploadKeepFiles Off

    # The audit engine works independently and
    # can be turned On of Off on the per-server or
    # on the per-directory basis
    SecAuditEngine {$secauditengine}

	# XXX Add knobs for these
    # Make sure that URL encoding is valid
    SecFilterCheckURLEncoding On

	# XXX Add knobs for these
    # Unicode encoding check
    SecFilterCheckUnicodeEncoding On

	# XXX Add knobs for these
    # Only allow bytes from this range
    SecFilterForceByteRange 1 255

	# Help prevent the effects of a Slowloris-type of attack
	# $secreadstatelimit

    # Cookie format checks.
    SecFilterCheckCookieFormat On

    # The name of the audit log file
    SecAuditLog logs/audit_log

    # Should mod_security inspect POST payloads
    SecFilterScanPOST On

	# Include rules from rules/ directory
	{$mod_security_rules}

</IfModule>

EOF;

}

if(file_exists("/usr/local/libexec/apache22/mod_mem_cache.so"))
	$mod_mem_cacheLoad = "Module mem_cache_module libexec/apache22/mod_mem_cache.so\n";

	$apache_config = <<<EOF
##################################################################################
# NOTE: This file was generated by the pfSense package management system.        #
#       Please do not edit this file by hand!   If you need to add functionality #
#       then edit /usr/local/pkg/apache_mod_security* files.                     #
#                                                                                #
#       And don't forget to submit your changes to coreteam@pfsense.org          #
###################################################################################
#
# This is the main Apache HTTP server configuration file.  It contains the
# configuration directives that give the server its instructions.
# See <URL:http://httpd.apache.org/docs/2.2> for detailed information.
# In particular, see 
# <URL:http://httpd.apache.org/docs/2.2/mod/directives.html>
# for a discussion of each configuration directive.
#
# Do NOT simply read the instructions in here without understanding
# what they do.  They're here only as hints or reminders.  If you are unsure
# consult the online docs. You have been warned.  
#
# Configuration and logfile names: If the filenames you specify for many
# of the server's control files begin with "/" (or "drive:/" for Win32), the
# server will use that explicit path.  If the filenames do *not* begin
# with "/", the value of ServerRoot is prepended -- so "/var/log/foo_log"
# with ServerRoot set to "/usr/local" will be interpreted by the
# server as "/usr/local//var/log/foo_log".

#
# ServerRoot: The top of the directory tree under which the server's
# configuration, error, and log files are kept.
#
# Do not add a slash at the end of the directory path.  If you point
# ServerRoot at a non-local disk, be sure to point the LockFile directive
# at a local disk.  If you wish to share the same ServerRoot for multiple
# httpd daemons, you will need to change at least LockFile and PidFile.
#
ServerRoot "/usr/local"

#
# Listen: Allows you to bind Apache to specific IP addresses and/or
# ports, instead of the default. See also the <VirtualHost>
# directive.
#
# Change this to Listen on specific IP addresses as shown below to 
# prevent Apache from glomming onto all bound IP addresses.
#
Listen {$global_listen}
{$aliases}

#
# Dynamic Shared Object (DSO) Support
#
# To be able to use the functionality of a module which was built as a DSO you
# have to place corresponding `LoadModule' lines at this location so the
# directives contained in it are actually available _before_ they are used.
# Statically compiled modules (those listed by `httpd -l') do not need
# to be loaded here.
#
# Example:
# LoadModule foo_module modules/mod_foo.so
#
# have to place corresponding `LoadModule' lines at this location so the
# LoadModule foo_module modules/mod_foo.so
LoadModule authn_file_module libexec/apache22/mod_authn_file.so
LoadModule authn_dbm_module libexec/apache22/mod_authn_dbm.so
LoadModule authn_anon_module libexec/apache22/mod_authn_anon.so
LoadModule authn_default_module libexec/apache22/mod_authn_default.so
LoadModule authn_alias_module libexec/apache22/mod_authn_alias.so
LoadModule authz_host_module libexec/apache22/mod_authz_host.so
LoadModule authz_groupfile_module libexec/apache22/mod_authz_groupfile.so
LoadModule authz_user_module libexec/apache22/mod_authz_user.so
LoadModule authz_dbm_module libexec/apache22/mod_authz_dbm.so
LoadModule authz_owner_module libexec/apache22/mod_authz_owner.so
LoadModule authz_default_module libexec/apache22/mod_authz_default.so
LoadModule auth_basic_module libexec/apache22/mod_auth_basic.so
LoadModule auth_digest_module libexec/apache22/mod_auth_digest.so
LoadModule file_cache_module libexec/apache22/mod_file_cache.so
LoadModule cache_module libexec/apache22/mod_cache.so
LoadModule disk_cache_module libexec/apache22/mod_disk_cache.so
LoadModule dumpio_module libexec/apache22/mod_dumpio.so
LoadModule include_module libexec/apache22/mod_include.so
LoadModule filter_module libexec/apache22/mod_filter.so
LoadModule charset_lite_module libexec/apache22/mod_charset_lite.so
LoadModule deflate_module libexec/apache22/mod_deflate.so
LoadModule log_config_module libexec/apache22/mod_log_config.so
LoadModule logio_module libexec/apache22/mod_logio.so
LoadModule env_module libexec/apache22/mod_env.so
LoadModule mime_magic_module libexec/apache22/mod_mime_magic.so
LoadModule cern_meta_module libexec/apache22/mod_cern_meta.so
LoadModule expires_module libexec/apache22/mod_expires.so
LoadModule headers_module libexec/apache22/mod_headers.so
LoadModule usertrack_module libexec/apache22/mod_usertrack.so
LoadModule unique_id_module libexec/apache22/mod_unique_id.so
LoadModule setenvif_module libexec/apache22/mod_setenvif.so
LoadModule version_module libexec/apache22/mod_version.so
LoadModule proxy_module libexec/apache22/mod_proxy.so
LoadModule proxy_connect_module libexec/apache22/mod_proxy_connect.so
LoadModule proxy_ftp_module libexec/apache22/mod_proxy_ftp.so
LoadModule proxy_http_module libexec/apache22/mod_proxy_http.so
LoadModule proxy_ajp_module libexec/apache22/mod_proxy_ajp.so
LoadModule proxy_balancer_module libexec/apache22/mod_proxy_balancer.so
LoadModule ssl_module libexec/apache22/mod_ssl.so
LoadModule mime_module libexec/apache22/mod_mime.so
LoadModule status_module libexec/apache22/mod_status.so
LoadModule autoindex_module libexec/apache22/mod_autoindex.so
LoadModule asis_module libexec/apache22/mod_asis.so
LoadModule info_module libexec/apache22/mod_info.so
LoadModule cgi_module libexec/apache22/mod_cgi.so
LoadModule vhost_alias_module libexec/apache22/mod_vhost_alias.so
LoadModule negotiation_module libexec/apache22/mod_negotiation.so
LoadModule dir_module libexec/apache22/mod_dir.so
LoadModule imagemap_module libexec/apache22/mod_imagemap.so
LoadModule actions_module libexec/apache22/mod_actions.so
LoadModule speling_module libexec/apache22/mod_speling.so
LoadModule userdir_module libexec/apache22/mod_userdir.so
LoadModule alias_module libexec/apache22/mod_alias.so
LoadModule rewrite_module libexec/apache22/mod_rewrite.so
{$mod_mem_cache}

<IfModule !mpm_netware_module>
<IfModule !mpm_winnt_module>
#
# If you wish httpd to run as a different user or group, you must run
# httpd as root initially and it will switch.  
#
# User/Group: The name (or #number) of the user/group to run httpd as.
# It is usually good practice to create a dedicated user and group for
# running httpd, as with most system services.
#
User www
Group www

</IfModule>
</IfModule>

# 'Main' server configuration
#
# The directives in this section set up the values used by the 'main'
# server, which responds to any requests that aren't handled by a
# <VirtualHost> definition.  These values also provide defaults for
# any <VirtualHost> containers you may define later in the file.
#
# All of these directives may appear inside <VirtualHost> containers,
# in which case these default settings will be overridden for the
# virtual host being defined.
#

#
# ServerAdmin: Your address, where problems with the server should be
# e-mailed.  This address appears on some server-generated pages, such
# as error documents.  e.g. admin@your-domain.com
#
ServerAdmin {$global_site_email}

#
# ServerName gives the name and port that the server uses to identify itself.
# This can often be determined automatically, but we recommend you specify
# it explicitly to prevent problems during startup.
#
# If your host doesn't have a registered DNS name, enter its IP address here.
#
{$servername}

#
# DocumentRoot: The directory out of which you will serve your
# documents. By default, all requests are taken from this directory, but
# symbolic links and aliases may be used to point to other locations.
#
DocumentRoot "/usr/local/apachemodsecurity"

#
# Each directory to which Apache has access can be configured with respect
# to which services and features are allowed and/or disabled in that
# directory (and its subdirectories). 
#
# First, we configure the "default" to be a very restrictive set of 
# features.  
#
<Directory />
    AllowOverride None
    Order deny,allow
    Deny from all
</Directory>

#
# Note that from this point forward you must specifically allow
# particular features to be enabled - so if something's not working as
# you might expect, make sure that you have specifically enabled it
# below.
#

#
# This should be changed to whatever you set DocumentRoot to.
#
#<Directory "/usr/local/www/apachemodsecurity/">
#    #
#    # Possible values for the Options directive are "None", "All",
#    # or any combination of:
#    #   Indexes Includes FollowSymLinks SymLinksifOwnerMatch ExecCGI MultiViews
#    #
#    # Note that "MultiViews" must be named *explicitly* --- "Options All"
#    # doesn't give it to you.
#    #
#    # The Options directive is both complicated and important.  Please see
#    # http://httpd.apache.org/docs/2.2/mod/core.html#options
#    # for more information.
#    #
#    Options Indexes FollowSymLinks
#
#    #
#    # AllowOverride controls what directives may be placed in .htaccess files.
#    # It can be "All", "None", or any combination of the keywords:
#    #   Options FileInfo AuthConfig Limit
#    #
#    AllowOverride None
#
#    #
#    # Controls who can get stuff from this server.
#    #
#    Order allow,deny
#    Allow from all
#
#</Directory>
#
#
# DirectoryIndex: sets the file that Apache will serve if a directory
# is requested.
#
#<IfModule dir_module>
#    DirectoryIndex index.html
#</IfModule>
#
#
# The following lines prevent .htaccess and .htpasswd files from being 
# viewed by Web clients. 
#
#<FilesMatch "^\.ht">
#    Order allow,deny
#    Deny from all
#    Satisfy All
#</FilesMatch>
#
#
# ErrorLog: The location of the error log file.
# If you do not specify an ErrorLog directive within a <VirtualHost>
# container, error messages relating to that virtual host will be
# logged here.  If you *do* define an error logfile for a <VirtualHost>
# container, that host's errors will be logged there and not here.
#
ErrorLog "/var/log/httpd-error.log"

#
# LogLevel: Control the number of messages logged to the error_log.
# Possible values include: debug, info, notice, warn, error, crit,
# alert, emerg.
#
LogLevel warn

<IfModule log_config_module>
    #
    # The following directives define some format nicknames for use with
    # a CustomLog directive (see below).
    #
    LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined
    LogFormat "%h %l %u %t \"%r\" %>s %b" common

    <IfModule logio_module>
      # You need to enable mod_logio.c to use %I and %O
      LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %I %O" combinedio
    </IfModule>

    #
    # The location and format of the access logfile (Common Logfile Format).
    # If you do not define any access logfiles within a <VirtualHost>
    # container, they will be logged here.  Contrariwise, if you *do*
    # define per-<VirtualHost> access logfiles, transactions will be
    # logged therein and *not* in this file.
    #
    #CustomLog "/var/log/httpd-access.log" common

    #
    # If you prefer a logfile with access, agent, and referer information
    # (Combined Logfile Format) you can use the following directive.
    #
    CustomLog "/var/log/httpd-access.log" combined
</IfModule>

#<IfModule alias_module>
#    #
#    # Redirect: Allows you to tell clients about documents that used to 
#    # exist in your server's namespace, but do not anymore. The client 
#    # will make a new request for the document at its new location.
#    # Example:
#    # Redirect permanent /foo http://www.example.com/bar
#
#    #
#    # Alias: Maps web paths into filesystem paths and is used to
#    # access content that does not live under the DocumentRoot.
#    # Example:
#    # Alias /webpath /full/filesystem/path
#    #
#    # If you include a trailing / on /webpath then the server will
#    # require it to be present in the URL.  You will also likely
#    # need to provide a <Directory> section to allow access to
#    # the filesystem path.
#
#    #
#    # ScriptAlias: This controls which directories contain server scripts. 
#    # ScriptAliases are essentially the same as Aliases, except that
#    # documents in the target directory are treated as applications and
#    # run by the server when requested rather than as documents sent to the
#    # client.  The same rules about trailing "/" apply to ScriptAlias
#    # directives as to Alias.
#    #
#    ScriptAlias /cgi-bin/ "/usr/local/www/apache22/cgi-bin/"
#
#</IfModule>

#<IfModule cgid_module>
#    #
#    # ScriptSock: On threaded servers, designate the path to the UNIX
#    # socket used to communicate with the CGI daemon of mod_cgid.
#    #
#    #Scriptsock /var/run/cgisock
#</IfModule>

#
# "/usr/local/www/apache22/cgi-bin" should be changed to whatever your ScriptAliased
# CGI directory exists, if you have that configured.
#
#<Directory "/usr/local/www/apache22/cgi-bin">
#    AllowOverride None
#    Options None
#    Order allow,deny
#    Allow from all
#</Directory>

#
# DefaultType: the default MIME type the server will use for a document
# if it cannot otherwise determine one, such as from filename extensions.
# If your server contains mostly text or HTML documents, "text/plain" is
# a good value.  If most of your content is binary, such as applications
# or images, you may want to use "application/octet-stream" instead to
# keep browsers from trying to display binary files as though they are
# text.
#
DefaultType text/plainm

<IfModule mime_module>
    #
    # TypesConfig points to the file containing the list of mappings from
    # filename extension to MIME-type.
    #
    TypesConfig etc/apache22/mime.types

    #
    # AddType allows you to add to or override the MIME configuration
    # file specified in TypesConfig for specific file types.
    #
    #AddType application/x-gzip .tgz
    #
    # AddEncoding allows you to have certain browsers uncompress
    # information on the fly. Note: Not all browsers support this.
    #
    #AddEncoding x-compress .Z
    #AddEncoding x-gzip .gz .tgz
    #
    # If the AddEncoding directives above are commented-out, then you
    # probably should define those extensions to indicate media types:
    #
    AddType application/x-compress .Z
    AddType application/x-gzip .gz .tgz

    #
    # AddHandler allows you to map certain file extensions to "handlers":
    # actions unrelated to filetype. These can be either built into the server
    # or added with the Action directive (see below)
    #
    # To use CGI scripts outside of ScriptAliased directories:
    # (You will also need to add "ExecCGI" to the "Options" directive.)
    #
    #AddHandler cgi-script .cgi

    # For type maps (negotiated resources):
    #AddHandler type-map var

    #
    # Filters allow you to process content before it is sent to the client.
    #
    # To parse .shtml files for server-side includes (SSI):
    # (You will also need to add "Includes" to the "Options" directive.)
    #
    #AddType text/html .shtml
    #AddOutputFilter INCLUDES .shtml
</IfModule>

#
# The mod_mime_magic module allows the server to use various hints from the
# contents of the file itself to determine its type.  The MIMEMagicFile
# directive tells the module where the hint definitions are located.
#
#MIMEMagicFile etc/apache22/magic

#
# Customizable error responses come in three flavors:
# 1) plain text 2) local redirects 3) external redirects
#
# Some examples:

{$errordocument}

#ErrorDocument 500 "The server made a boo boo."
#ErrorDocument 404 /missing.html
#ErrorDocument 404 "/cgi-bin/missing_handler.pl"
#ErrorDocument 402 http://www.example.com/subscription_info.html
#

#
# EnableMMAP and EnableSendfile: On systems that support it, 
# memory-mapping or the sendfile syscall is used to deliver
# files.  This usually improves server performance, but must
# be turned off when serving from networked-mounted 
# filesystems or if support for these functions is otherwise
# broken on your system.
#
#EnableMMAP off
#EnableSendfile off

# Supplemental configuration
#
# The configuration files in the etc/apache22/extra/ directory can be 
# included to add extra features or to modify the default configuration of 
# the server, or you may simply copy their contents here and change as 
# necessary.

# Server-pool management (MPM specific)
#Include etc/apache22/extra/httpd-mpm.conf

# Multi-language error messages
#Include etc/apache22/extra/httpd-multilang-errordoc.conf

# Fancy directory listings
#Include etc/apache22/extra/httpd-autoindex.conf

# Language settings
#Include etc/apache22/extra/httpd-languages.conf

# User home directories
#Include etc/apache22/extra/httpd-userdir.conf

# Real-time info on requests and configuration
#Include etc/apache22/extra/httpd-info.conf

# Virtual hosts
#Include etc/apache22/extra/httpd-vhosts.conf

# Local access to the Apache HTTP Server Manual
#Include etc/apache22/extra/httpd-manual.conf

# Distributed authoring and versioning (WebDAV)
#Include etc/apache22/extra/httpd-dav.conf

# Various default settings
#Include etc/apache22/extra/httpd-default.conf

# Secure (SSL/TLS) connections
#Include etc/apache22/extra/httpd-ssl.conf
#
# Note: The following must must be present to support
#       starting without SSL on platforms with no /dev/random equivalent
#       but a statically compiled-in mod_ssl.
#
<IfModule ssl_module>
SSLRandomSeed startup builtin
SSLRandomSeed connect builtin
</IfModule>

# Cache settings
{$mem_cache}
{$cache_root}

# Mod security
{$mod_security}

# Proxysettings
{$mod_proxy}

# Include anything else
Include etc/apache22/Includes/*.conf

EOF;

	$fd = fopen("/usr/local/etc/apache22/httpd.conf", "w");
	if(!$fd) {
		$error_text = "Could not open httpd.conf for writing!";
		echo $error_text;
		log_error($error_text);
		exit;
	}
	fwrite($fd, $apache_config);
	fclose($fd);
}

?>
