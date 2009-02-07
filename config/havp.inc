<?php
require_once('globals.inc');
require_once('config.inc');
require_once('service-utils.inc');
require_once('pkg-utils.inc');
require_once('pfsense-utils.inc');
require_once("notices.inc");

/*
	havp.inc
        part of the HAVP package for pfSense
	Copyright (C) 2006 Rajkumar S <raj@linuxense.com>
	All rights reserved.
        $Id$

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
function sync_package_havp() {
	conf_mount_rw();
	config_lock();
	global $config, $g;
	$fd = fopen("/etc/havp.config","w");
	fwrite($fd, "USER havp\n");
	fwrite($fd, "GROUP havp\n");
	fwrite($fd, "ACCESSLOG /var/log/havp/access.log\n");
	fwrite($fd, "ERRORLOG /var/log/havp/error.log\n");
	fwrite($fd, "DISPLAYINITIALMESSAGES false\n");
	fwrite($fd, "ENABLECLAMLIB true\n");
	fwrite($fd, "CLAMDBDIR /var/db/clamav/\n");
	fwrite($fd, "ENABLECLAMD false\n");
	fwrite($fd, "ENABLEFPROT false\n");
	fwrite($fd, "ENABLEAVG false\n");
	fwrite($fd, "ENABLEAVESERVER false\n");
	fwrite($fd, "ENABLESOPHIE false\n");
	fwrite($fd, "ENABLETROPHIE false\n");
	fwrite($fd, "ENABLENOD32 false\n");
	fwrite($fd, "ENABLEAVAST false\n");
	if($config['installedpackages']['havp']['config'] != "") {
	     foreach($config['installedpackages']['havp']['config'] as $tmp) {
		  if ($tmp['servernumber'] != "") fwrite($fd,"SERVERNUMBER " . $tmp['servernumber'] . "\n");
		  if ($tmp['maxservers'] != "") fwrite($fd,"MAXSERVERS " . $tmp['maxservers'] . "\n");
		  if ($tmp['port'] != "") fwrite($fd,"PORT " . $tmp['port'] . "\n");
		  if ($tmp['source_address'] != "") fwrite($fd,"SOURCE_ADDRESS " . $tmp['source_address'] . " \n");
		  if ($tmp['dbreload'] != "") fwrite($fd,"DBRELOAD " . $tmp['dbreload'] . " \n");
		  if ($tmp['parentproxy'] != "") fwrite($fd,"PARENTPROXY " . $tmp['parentproxy'] . " \n");
		  if ($tmp['parentport'] != "") fwrite($fd,"PARENTPORT " . $tmp['parentport'] . " \n");
		  if ($tmp['maxscansize'] != "") fwrite($fd,"MAXSCANSIZE " . $tmp['maxscansize'] . " \n");
		  if ($tmp['trickling'] != "") fwrite($fd,"TRICKLING " . $tmp['trickling'] . " \n");
		  if ($tmp['maxdownloadsize'] != "") fwrite($fd,"MAXDOWNLOADSIZE " . $tmp['maxdownloadsize'] . " \n");

		  /* If Transparent then, don't bother with bind address */
		  if ($tmp['transparent'] == "on"){
		       fwrite($fd,"TRANSPARENT true\n");
		       fwrite($fd,"BIND_ADDRESS 127.0.0.1\n");
		       setup_transparency();
		  }
		  else {
		       fwrite($fd,"TRANSPARENT false\n");
		       if ($tmp['bind_address'] != ""){
			    fwrite($fd,"BIND_ADDRESS " . $config['interfaces'][strtolower($tmp['bind_address'])]['ipaddr']  . " \n");
		       }else { /* just to be sure, default is lan */
			    fwrite($fd,"BIND_ADDRESS " . $config['interfaces']['lan']['ipaddr'] . " \n");
		       }
		       $havp_pf_result = mwexec ("pfctl -a rdr-package/havp -F nat");
		       $havp_pf_result = mwexec ("pfctl -t havp -T kill");
		       if($havp_pf_result <> 0) {
			    file_notice("HAVP", "There were error(s) clearing the transparency rules", "HAVP", "");
		       }
		  }

		  if ($tmp['range'] == "on"){
		       fwrite($fd,"RANGE true\n");
		  }
		  else {
		       fwrite($fd,"RANGE false\n");
		  }
		  if ($tmp['log_oks'] == "on"){
		       fwrite($fd,"LOG_OKS true\n");
		  }
		  else {
		       fwrite($fd,"LOG_OKS false\n");
		  }
	     }
	} else {
	     fwrite($fd,"SERVERNUMBER 10\n");
	     fwrite($fd,"MAXSERVERS 100\n");
	     fwrite($fd,"PORT 8080\n");
	     fwrite($fd,"TRANSPARENT false\n");
	     fwrite($fd,"RANGE true\n");
	     fwrite($fd,"LOG_OKS true\n");
	     fwrite($fd,"BIND_ADDRESS " . $config['interfaces']['lan']['ipaddr'] . " \n");	     
	}
	fclose($fd);
	conf_mount_ro();
	config_unlock();
	unlink_if_exists("/usr/local/etc/havp/havp.config");
	mwexec("ln -sf /etc/havp.config /usr/local/etc/havp/havp.config");
	mwexec("/usr/local/etc/rc.d/havp.sh stop");
	mwexec("/usr/local/etc/rc.d/havp.sh start");
}

function havp_install_command() {
	global $config, $g;
	mwexec ("mkdir -p /var/log/havp/");
	mwexec ("mkdir -p /var/tmp/havp/");
	mwexec ("mkdir -p /var/run/havp/");
	mwexec ("chown havp:havp /var/log/havp");
	mwexec ("chown havp:havp /var/tmp/havp");
	mwexec ("chown havp:havp /var/run/havp");
	mwexec ("cp /usr/local/pkg/havp.sh /usr/local/etc/rc.d/");
	mwexec ("chmod +x /usr/local/etc/rc.d/havp.sh");
	sync_package_havp();
}

function havp_deinstall_command() {
	global $config, $g;
	mwexec("/usr/local/etc/rc.d/havp.sh stop");
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/havp.sh");
	unlink_if_exists("/usr/local/etc/havp/havp.config");
	unlink_if_exists("/etc/havp.config");
	conf_mount_ro();
}
function sync_package_havp_whitelist() {
	global $config;
	conf_mount_rw();
	config_lock();
	$fd = fopen("/usr/local/etc/havp/whitelist","w");
	if($config['installedpackages']['havpwhitelist']['config'] != "")
	    foreach($config['installedpackages']['havpwhitelist']['config'] as $tmp) {
		fwrite($fd, $tmp['url'] . "\n");
	    }
	fclose($fd);
	/* signal a reload of all files */
	conf_mount_ro();
	config_unlock();
}

function sync_package_havp_blacklist() {
	global $config;
	conf_mount_rw();
	config_lock();
	$fd = fopen("/usr/local/etc/havp/blacklist","w");
	if($config['installedpackages']['havpblacklist']['config'] != "")
	    foreach($config['installedpackages']['havpblacklist']['config'] as $tmp) {
		fwrite($fd, $tmp['url'] . "\n");
	    }
	fclose($fd);
	/* signal a reload of all files */
	conf_mount_ro();
	config_unlock();
}

function add_trans_table(){
	global $config;
	conf_mount_rw();
	config_lock();

	# Flush all entries first, and then add them.
	$havp_pf_result = mwexec ('pfctl -a "rdr-package/havp" -t havp -T flush');
	if($havp_pf_result <> 0) {
		file_notice("HAVP", "There were error(s) flushing the exclude table", "HAVP", "");
	}
	if($config['installedpackages']['havptransexclude']['config'] != ""){
		foreach($config['installedpackages']['havptransexclude']['config'] as $tmp) {
			$havp_pf_result = mwexec ('pfctl -a "rdr-package/havp" -t havp -T add ' . $tmp['ip']);
			if($havp_pf_result <> 0) {
				file_notice("HAVP", "There were error(s) adding the ip " . $tmp['ip'], "HAVP", "");
			}
		}
	}
	/* signal a reload of all files */
	conf_mount_ro();
	config_unlock();
}

function setup_transparency(){
	global $config;
	$trans_file = fopen("/tmp/havp_pf.rules","w");
	fwrite($trans_file, "table <havp> persist\n");
	fwrite($trans_file, "rdr on " . $config['interfaces']['lan']['if'] .  " inet proto tcp from !<havp> to ! " . $config['interfaces']['lan']['ipaddr'] . " port = http -> 127.0.0.1 port 8080 \n");
	fclose($trans_file);
	$havp_pf_result = mwexec ('pfctl -a "rdr-package/havp" -f /tmp/havp_pf.rules');
	if($havp_pf_result <> 0) {
		file_notice("HAVP", "There were error(s) loading the transparency rules", "HAVP", "");
	}
	add_trans_table();
}

function transparency_init(){
	global $config;
	if($config['installedpackages']['havp']['config'] != "") {
		if($config['installedpackages']['havp']['config'][0]['transparent'] == "on") {
			setup_transparency();
		}
	}
}
	
?>
