<?php
/* $Id$ */
/* ========================================================================== */
/*
    zabbix2-proxy.inc
    part of the Zebedee package for pfSense
    Copyright (C) 2013 Danilo G. Baio
	Copyright (C) 2013 Marcello Coutinho

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
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("globals.inc");

function php_install_zabbix2(){
	sync_package_zabbix2();	
}

function php_deinstall_zabbix2_agent(){
   global $config, $g;

   conf_mount_rw();
   $pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
   if ($pfs_version > 2.0){
      define('ZABBIX_AGENT_BASE', '/usr/pbi/zabbix22-agent-' . php_uname("m"));
   } else {
      define('ZABBIX_AGENT_BASE', '/usr/local');
   }

   exec("/usr/bin/killall zabbix_agentd");
   unlink_if_exists(ZABBIX_AGENT_BASE . "/etc/rc.d/zabbix2_agentd.sh");
   unlink_if_exists(ZABBIX_AGENT_BASE . "/etc/zabbix22/zabbix_agentd.conf");
   unlink_if_exists("/var/log/zabbix2/zabbix2_agentd.log");
   unlink_if_exists("/var/run/zabbix2/zabbix2_agentd.pid");

   if (!is_array($config['installedpackages']['zabbixproxy'])){
      if (is_dir("/var/log/zabbix2"))
         exec("/bin/rm -r /var/log/zabbix2/");
      if (is_dir("/var/run/zabbix2"))
         exec("/bin/rm -r /var/run/zabbix2/");
   }

   conf_mount_ro();
}

function php_deinstall_zabbix2_proxy(){
   global $config, $g;

   conf_mount_rw();
   $pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
   if ($pfs_version > 2.0){
      define('ZABBIX_PROXY_BASE', '/usr/pbi/zabbix22-proxy-' . php_uname("m"));
   } else {
      define('ZABBIX_PROXY_BASE', '/usr/local');
   }

   exec("/usr/bin/killall zabbix_proxy");
   unlink_if_exists(ZABBIX_PROXY_BASE . "/etc/rc.d/zabbix2_proxy.sh");
   unlink_if_exists(ZABBIX_PROXY_BASE . "/etc/zabbix22/zabbix_proxy.conf");
   unlink_if_exists("/var/log/zabbix2/zabbix_proxy.log");
   unlink_if_exists("/var/run/zabbix2/zabbix2_proxy.pid");

   if (!is_array($config['installedpackages']['zabbixagent'])){
      if (is_dir("/var/log/zabbix2"))
         exec("/bin/rm -r /var/log/zabbix2/");
      if (is_dir("/var/run/zabbix2"))
         exec("/bin/rm -r /var/run/zabbix2/");
   }

   if (is_dir("/var/db/zabbix2"))
      exec("/bin/rm -r /var/db/zabbix2/");

   conf_mount_ro();
}

function validate_input_zabbix2($post,&$input_errors){

	if  (isset($post['proxyenabled'])){
		if (!is_numericint($post['serverport'])) {
      		$input_errors[]='Server Port is not numeric.'.$ServerPort;
   			}
	
		if (!is_numericint($post['configfrequency'])) {
			$input_errors[]='Config Frequency is not numeric.';
			}
		}
	if  (isset($post['agentenabled'])){
		if (!preg_match("/\w+/", $post['server'])) {
			$input_errors[]='Server field is required.';
			}
		
		if (!preg_match("/\w+/", $post['hostname'])) {
			$input_errors[]='Hostname field is required.';
			}
	
		if ($post['listenip'] != '') {
			if (!is_ipaddr_configured($post['listenip']) && !preg_match("/(127.0.0.1|0.0.0.0)/",$post['listenip'])) {
				$input_errors[]='Listen IP is not a configured IP address.';
			}
		}

		if ($post['listenport'] != '') {
			if (!preg_match("/^\d+$/", $post['listenport'])) {
				$input_errors[]='Listen Port is not numeric.';
			}	
		}

		if ($post['refreshactchecks'] != '') {
			if (!preg_match("/^\d+$/", $post['refreshactchecks'])) {
				$input_errors[]='Refresh Active Checks is not numeric.';
			} elseif ( $post['refreshactchecks'] < 60 || $post['refreshactchecks'] > 3600 ) {
				$input_errors[]='You must enter a valid value for \'Refresh Active Checks\'';
			}
		}

		if ($post['timeout'] != '') {
			if (!is_numericint($post['timeout'])) {
				$input_errors[]='Timeout is not numeric.';
			} elseif ( $post['timeout'] < 1 || $post['timeout'] > 30 ) {
				$input_errors[]='You must enter a valid value for \'Timeout\'';
			}
		}
	
		if ($post['buffersend'] != '') {
			if (!is_numericint($post['buffersend'])) {
				$input_errors[]='Buffer Send is not numeric.';
			} elseif ( $post['buffersend'] < 1 || $post['buffersend'] > 3600 ) {
				$input_errors[]='You must enter a valid value for \'Buffer Send\'';
			}
		}
		
		if ($post['buffersize'] != '') {
			if (!is_numericint($post['buffersize'])) {
				$input_errors[]='Bufer Size is not numeric.';
			} elseif ( $post['buffersize'] < 2 || $post['buffersize'] > 65535 ) {
				$input_errors[]='You must enter a valid value for \'Buffer Size\'';
			}
		}
		
		if ($post['startagents'] != '') {
			if (!is_numericint($post['startagents'])) {
				$input_errors[]='Start Agents is not numeric.';
			} elseif ( $post['startagents'] < 0 || $post['startagents'] > 100 ) {
				$input_errors[]='You must enter a valid value for \'Start Agents\'';
			}
		}
	}	
}

function sync_package_zabbix2(){
	global $config, $g;

	conf_mount_rw();
	#check pfsense version
	$pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
	if ($pfs_version > 2.0){
		define('ZABBIX_AGENT_BASE', '/usr/pbi/zabbix22-agent-' . php_uname("m"));
		define('ZABBIX_PROXY_BASE', '/usr/pbi/zabbix22-proxy-' . php_uname("m"));
		}
	else {
		define('ZABBIX_AGENT_BASE', '/usr/local');
		define('ZABBIX_PROXY_BASE', '/usr/local');
	}

	#check zabbix proxy config
	if (is_array($config['installedpackages']['zabbixproxy'])){
		$zbproxy_config = $config['installedpackages']['zabbixproxy']['config'][0];
		if ($zbproxy_config['proxyenabled']=="on"){
			$Mode=(is_numericint($zbproxy_config['proxymode'])?$zbproxy_config['proxymode'] : 0);
			$AdvancedParams=base64_decode($zbproxy_config['advancedparams']);
		
			$zbproxy_conf_file = <<< EOF
Server={$zbproxy_config['server']}
ServerPort={$zbproxy_config['serverport']}
Hostname={$zbproxy_config['hostname']}
PidFile=/var/run/zabbix2/zabbix2_proxy.pid
DBName=/var/db/zabbix2/proxy.db
LogFile=/var/log/zabbix2/zabbix_proxy.log
ConfigFrequency={$zbproxy_config['configfrequency']}
FpingLocation=/usr/local/sbin/fping
#there's currently no fping6 (IPv6) dependency in the package, but if there was, the binary would likely also be in /usr/local/sbin
Fping6Location=/usr/local/sbin/fping6
ProxyMode={$Mode}
{$AdvancedParams}

EOF;
			file_put_contents(ZABBIX_PROXY_BASE . "/etc/zabbix22/zabbix_proxy.conf", strtr($zbproxy_conf_file, array("\r" => "")));
		}
	}
	/* check zabbix agent settings*/
	if (is_array($config['installedpackages']['zabbixagent'])){
		$zbagent_config = $config['installedpackages']['zabbixagent']['config'][0];
		if ($zbagent_config['agentenabled']=="on"){
			$RefreshActChecks=(preg_match("/(\d+)/",$zbagent_config['refreshactchecks'],$matches)? $matches[1] : "120");
			$BufferSend=(preg_match("/(\d+)/",$zbagent_config['buffersend'],$matches)? $matches[1]  : "5"  );
			$BufferSize=(preg_match("/(\d+)/",$zbagent_config['buffersize'],$matches)? $matches[1]  : "100");
			$StartAgents=(preg_match("/(\d+)/",$zbagent_config['startagents'],$matches)? $matches[1] :"3" );
			$UserParams=base64_decode($zbagent_config['userparams']);
			$ListenIp=($zbagent_config['listenip'] != ''? $zbagent_config['listenip'] : "0.0.0.0");
			$ListenPort=($zbagent_config['listenport'] != ''? $zbagent_config['listenport'] : "10050");
			$TimeOut=($zbagent_config['timeout'] != ''? $zbagent_config['timeout'] : "3");
	
			$zbagent_conf_file = <<< EOF
Server={$zbagent_config['server']}
ServerActive={$zbagent_config['serveractive']}
Hostname={$zbagent_config['hostname']}
ListenIP={$ListenIp}
ListenPort={$ListenPort}
RefreshActiveChecks={$RefreshActChecks}
DebugLevel=3
PidFile=/var/run/zabbix2/zabbix2_agentd.pid
LogFile=/var/log/zabbix2/zabbix2_agentd.log
LogFileSize=1
Timeout={$TimeOut}
BufferSend={$BufferSend}
BufferSize={$BufferSize}
StartAgents={$StartAgents}
{$UserParams}

EOF;
		file_put_contents(ZABBIX_AGENT_BASE . "/etc/zabbix22/zabbix_agentd.conf",  strtr($zbagent_conf_file, array("\r" => "")));
		}
	}
	$want_sysctls = array(
		'kern.ipc.shmall' => '2097152',
		'kern.ipc.shmmax' => '2147483648',
		'kern.ipc.semmsl' => '250'
	);
	$sysctls = array();
	#check sysctl file values
	$sc_file="";
	if (file_exists("/etc/sysctl.conf")) {
		$sc = file("/etc/sysctl.conf");
		foreach ($sc as $line) {
			list($sysk, $sysv) = explode("=", $line, 2);
			if (preg_match("/\w/",$line) && !array_key_exists($sysk, $want_sysctls))
				$sc_file.=$line;
			}
	}
	foreach ($want_sysctls as $ws=> $wv) {
		$sc_file .= "{$ws}={$wv}\n";
		exec("/sbin/sysctl {$ws}={$wv}");
	}
	file_put_contents("/etc/sysctl.conf", $sc_file);

	#check bootloader values
	$lt_file="";
	$want_tunables = array(
		'kern.ipc.semopm' => '100',
		'kern.ipc.semmni' => '128',
		'kern.ipc.semmns' => '32000',
		'kern.ipc.shmmni' => '4096'
	);
	$tunables = array();
	if (file_exists("/boot/loader.conf")) {
		$lt = file("/boot/loader.conf");
		foreach ($lt as $line) {
			list($tunable, $val) = explode("=", $line, 2);
			if (preg_match("/\w/",$line) && !array_key_exists($tunable, $want_tunables))
				$lt_file.=$line;
		}
	}
	foreach ($want_tunables as $wt => $wv) {
		$lt_file.= "{$wt}={$wv}\n";
	}
	file_put_contents("/boot/loader.conf", $lt_file);

	/*check startup script files*/
	/* create a few directories and ensure the sample files are in place */
	if (!is_dir(ZABBIX_PROXY_BASE . "/etc/zabbix22"))
		exec("/bin/mkdir -p " . ZABBIX_PROXY_BASE . "/etc/zabbix22");

	$dir_checks  = <<< EOF
if [ ! -d /var/log/zabbix2 ]
 then
 /bin/mkdir -p /var/log/zabbix2
 /usr/sbin/chmod 755 /var/log/zabbix2
 fi
/usr/sbin/chown -R zabbix:zabbix /var/log/zabbix2

if [ ! -d /var/run/zabbix2 ]
 then
 /bin/mkdir -p /var/run/zabbix2
 /usr/sbin/chmod 755 /var/run/zabbix2
 fi
/usr/sbin/chown -R zabbix:zabbix /var/run/zabbix2

if [ ! -d /var/db/zabbix2 ]
 then
 /bin/mkdir -p /var/db/zabbix2
 /usr/sbin/chmod 755 /var/db/zabbix2
 fi
/usr/sbin/chown -R zabbix:zabbix /var/db/zabbix2

EOF;
 
	$zproxy_rcfile="/usr/local/etc/rc.d/zabbix2_proxy.sh";
	if (is_array($zbproxy_config) && $zbproxy_config['proxyenabled']=="on"){
		$zproxy_start= strtr($dir_checks, array("\r" => "")). "\necho \"Starting Zabbix Proxy\"...\n";
		/* start zabbix proxy */
		$zproxy_start .= ZABBIX_PROXY_BASE . "/sbin/zabbix_proxy\n";
	
		$zproxy_stop  = "echo \"Stopping Zabbix Proxy\"\n";
		$zproxy_stop .= "/usr/bin/killall zabbix_proxy\n";
		$zproxy_stop .= "/bin/sleep 5\n";

		/* write out rc.d start/stop file */
		write_rcfile(array(
			"file" => "zabbix2_proxy.sh",
			"start" => $zproxy_start,
			"stop" => $zproxy_stop
			)
		);
		mwexec("{$zproxy_rcfile} restart");
	}else{
		if (file_exists($zproxy_rcfile)){
		mwexec("{$zproxy_rcfile} stop");
		unlink($zproxy_rcfile);
		}
	}
	
	$zagent_rcfile="/usr/local/etc/rc.d/zabbix2_agentd.sh";
	if (is_array($zbagent_config) && $zbagent_config['agentenabled']=="on"){
		$zagent_start .= strtr($dir_checks, array("\r" => "")). "\necho \"Starting Zabbix Agent...\"\n";
		$zagent_start .= ZABBIX_AGENT_BASE . "/sbin/zabbix_agentd\n";
	
		$zagent_stop  = "echo \"Stopping Zabbix Agent...\"\n";
		$zagent_stop .= "/usr/bin/killall zabbix_agentd\n";
		$zagent_stop .= "/bin/sleep 5\n";
	
		/* write out rc.d start/stop file */
		write_rcfile(array(
						"file" => "zabbix2_agentd.sh",
						"start" => "$zagent_start",
						"stop" => "$zagent_stop"
				)
		);
		mwexec("{$zagent_rcfile} restart");	
	}else{
		if (file_exists($zagent_rcfile)){
		mwexec("{$zagent_rcfile} stop");
		unlink($zagent_rcfile);
		}
	}
		
	conf_mount_ro();
}
?>
