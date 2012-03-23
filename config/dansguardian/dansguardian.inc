<?php
/*
	dansguardian.inc
	part of the Dansguardian package for pfSense
	Copyright (C) 2012 Marcello Coutinho
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

require_once("util.inc");
require("globals.inc");
#require("guiconfig.inc");


function dg_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function dg_get_real_interface_address($iface) {
	global $config;
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	$postfix_enabled=$config['installedpackages']['postfix']['config'][0]['enable_postfix'];
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

function sync_package_dansguardian() {
	global $config,$g;

	#check if its booting
	if ($g['booting']){
		if (is_array($config['installedpackages']['dansguardian']))
			if (!$config['installedpackages']['dansguardian']['config'][0]['enable'])
				return;
		if (file_exists("/var/run/dansguardian.pid")){
			exec("/bin/ps -p `cat /var/run/dansguardian.pid` 2>&1",$return);
			if (preg_match("/dansguardian/",$return[1]))
				return;
		}
	}
	#assign xml arrays
	if (!is_array($config['installedpackages']['dansguardian']))
		$config['installedpackages']['dansguardian']['config'][0]=array('interface'=>'lo0',
																		'daemon_options'=>'softrestart');
	$dansguardian=$config['installedpackages']['dansguardian']['config'][0];
	if (!is_array($config['installedpackages']['dansguardianconfig']))
		$config['installedpackages']['dansguardianconfig']['config'][0]=array('auth_plugin'=>'',
																		'scan_options'=>'scancleancache,createlistcachefiles,deletedownloadedtempfiles',
																		'weightedphrasemode'=>'2',
																		'preservecase'=>'0',
																		'phrasefiltermode'=>'2',
																		'cron'=>'day');
	$dansguardian_config=$config['installedpackages']['dansguardianconfig']['config'][0];
	if (!is_array($config['installedpackages']['dansguardianlog']))
		$config['installedpackages']['dansguardianlog']['config'][0]=array('report_level'=>'3',
																		'report_language'=>'ukenglish',
																		'report_options'=>'showweightedfound,usecustombannedimage,nonstandarddelimiter',
																		'logging_options'=>'logconnectionhandlingerrors',
																		'loglevel'=>'2',
																		'logexceptionhits'=>'2',
																		'logfileformat'=>'1');
	$dansguardian_log=$config['installedpackages']['dansguardianlog']['config'][0];
	if (is_array($config['installedpackages']['dansguardianlimits']))
		$dansguardian_limits=$config['installedpackages']['dansguardianlimits']['config'][0];
	if (is_array($config['installedpackages']['dansguardianusers']))
		$dansguardian_users=$config['installedpackages']['dansguardianusers']['config'][0];
	if (is_array($config['installedpackages']['dansguardianblacklist']['config']))
		$dansguardian_blacklist=$config['installedpackages']['dansguardianblacklist']['config'][0];
		
	#daemon options
	$dansguardian_enabled=$dansguardian['enable_dg'];
	$filterport=($dansguardian['filterports']?$dansguardian['filterports']:"8080");
	$softrestart=(preg_match('/softrestart/',$dansguardian['daemon_options'])?"yes":"no");
	$nodaemon=(preg_match('/nodaemon/',$dansguardian['daemon_options'])?"yes":"off");
	if (preg_match("/\d+\/\d+/",$dansguardian['children']))
		list($minchildren,$maxchildren) = split ("/", $dansguardian['children'], 2);
	else
		list($minchildren,$maxchildren) = split ("/", "8/120", 2);
	if (preg_match("/\d+\/\d+/",$dansguardian['sparechildren']))
		list($minsparechildren,$maxsparechildren) = split ("/", $dansguardian['sparechildren'], 2);
	else
		list($minsparechildren,$maxsparechildren) = split ("/", "8/64", 2);	
	$maxagechildren=($dansguardian['maxagechildren']?$dansguardian['maxagechildren']:"500");
	$maxips=($dansguardian['maxips']?$dansguardian['maxips']:"0");
	$preforkchildren=($dansguardian['preforkchildren']?$dansguardian['preforkchildren']:"10");
	$proxyip=($dansguardian['proxyip']?$dansguardian['proxyip']:"127.0.0.1");
	$proxyport=($dansguardian['proxyport']?$dansguardian['proxyport']:"127.0.0.1");
	
	#general options
	$urlcachenumber=($dansguardian_config['urlcachenumber']?$dansguardian_config['urlcachenumber']:"1000");
	$urlcacheage=($dansguardian_config['urlcacheage']?$dansguardian_config['urlcacheage']:"900");
	$scancleancache=(preg_match('/scancleancache/',$dansguardian_config['scan_options'])?"on":"off");
	$hexdecodecontent=(preg_match('/hexdecodecontent/',$dansguardian_config['scan_options'])?"on":"off");
	$forcequicksearch=(preg_match('/forcequicksearch/',$dansguardian_config['scan_options'])?"on":"off");
	$reverseaddresslookups=(preg_match('/reverseaddresslookups/',$dansguardian_config['scan_options'])?"on":"off");
	$reverseclientiplookups=(preg_match('/reverseclientiplookups/',$dansguardian_config['scan_options'])?"on":"off");
	$logclienthostnames=(preg_match('/logclienthostnames/',$dansguardian_config['scan_options'])?"on":"off");
	$createlistcachefiles=(preg_match('/createlistcachefiles/',$dansguardian_config['scan_options'])?"on":"off");
	$prefercachedlists=(preg_match('/prefercachedlists/',$dansguardian_config['scan_options'])?"on":"off");
	$deletedownloadedtempfiles=(preg_match('/deletedownloadedtempfiles/',$dansguardian_config['scan_options'])?"on":"off");
	$weightedphrasemode=($dansguardian_config['weightedphrasemode']?$dansguardian_config['weightedphrasemode']:"2");
	$phrasefiltermode=($dansguardian_config['phrasefiltermode']?$dansguardian_config['phrasefiltermode']:"2");
	$preservecase=($dansguardian_config['preservecase']?$dansguardian_config['preservecase']:"0");
	$clamdscan=(preg_match('/clamdscan/',$dansguardian_config['content_scanners'])?"on":"off");
	$icapscan=(preg_match('/icapscan/',$dansguardian_config['content_scanners'])?"on":"off");
	$contentscannertimeout=($dansguardian_config['contentscannertimeout']?$dansguardian_config['contentscannertimeout']:"60");
	$contentscanexceptions=($dansguardian_config['contentscanexceptions']?"on":"off");
	$recheckreplacedurls=(preg_match('/recheckreplacedurls/',$dansguardian_config['misc_options'])?"on":"off");
	$forwardedfor=(preg_match('/forwardedfor/',$dansguardian_config['misc_options'])?"on":"off");
	$recheckreplacedurls=(preg_match('/icapscan/',$dansguardian_config['misc_options'])?"on":"off");
	$usexforwardedfor=(preg_match('/usexforwardedfor/',$dansguardian_config['misc_options'])?"on":"off");
	$authplugin=(preg_match('/usr/',$dansguardian_config['auth_plugin'])?"authplugin = '".$dansguardian_config['auth_plugin']."'":"");
	
	#limits
	$maxuploadsize=($dansguardian_limits['maxuploadsize']?$dansguardian_limits['maxuploadsize']:"-1");
	$maxcontentfiltersize=($dansguardian_limits['maxcontentfiltersize']?$dansguardian_limits['maxcontentfiltersize']:"256");
	$maxcontentramcachescansize=($dansguardian_limits['maxcontentramcachescansize']?$dansguardian_limits['maxcontentramcachescansize']:"1000");
	$maxcontentfilecachescansize=($dansguardian_limits['maxcontentfilecachescansize']?$dansguardian_limits['maxcontentfilecachescansize']:"2000");
	$initialtrickledelay=($dansguardian_limits['initialtrickledelay']?$dansguardian_limits['initialtrickledelay']:"20");
	$trickledelay=($dansguardian_limits['trickledelay']?$dansguardian_limits['trickledelay']:"20");
	
	#report and log
	$reportlevel=($dansguardian_log['report_level']?$dansguardian_log['report_level']:"3");
	$reportlanguage=($dansguardian_log['report_language']?$dansguardian_log['report_language']:"ukenglish");
	$showweightedfound=(preg_match('/showweightedfound/',$dansguardian_log['report_options'])?"on":"off");
	$usecustombannedflash=(preg_match('/usecustombannedflash/',$dansguardian_log['report_options'])?"on":"off");
	if (file_exists('/usr/local/share/dansguardian/blockedflash.swf'))
		$custombannedflashfile="custombannedflashfile = '/usr/local/share/dansguardian/blockedflash.swf'";
	$usecustombannedimage=(preg_match('/usecustombannedimage/',$dansguardian_log['report_options'])?"on":"off");
	$nonstandarddelimiter=(preg_match('/nonstandarddelimiter/',$dansguardian_log['report_options'])?"on":"off");
	
	$logchildprocesshandling=(preg_match('/logchildprocesshandling/',$dansguardian_log['logging_options'])?"on":"off");
	$logconnectionhandlingerrors=(preg_match('/logconnectionhandlingerrors/',$dansguardian_log['logging_options'])?"on":"off");
	$nologger=(preg_match('/nologger/',$dansguardian_log['logging_options'])?"on":"off");
	$logadblocks=(preg_match('/logadblocks/',$dansguardian_log['logging_options'])?"on":"off");
	$anonymizelogs=(preg_match('/anonymizelogs/',$dansguardian_log['logging_options'])?"on":"off");

	$loglevel=($dansguardian_log['loglevel']?$dansguardian_log['loglevel']:"2");
	$logexceptionhits=($dansguardian_log['logexceptionhits']?$dansguardian_log['logexceptionhits']:"2");
	$logfileformat=($dansguardian_log['logfileformat']?$dansguardian_log['logfileformat']:"1");
	
	#check files
	#create sample files
	$files = array(	"/dansguardianf1.conf",
					"/lists/filtergroupslist",
					"/lists/bannedphraselist",
					"/lists/exceptionphraselist",
					"/lists/weightedphraselist",
					"/lists/exceptionsitelist",
					"/lists/bannedsitelist",
					"/lists/greysitelist",
					"/lists/logsitelist",
					"/lists/bannedregexpurllist",
					"/lists/bannedurllist",
					"/lists/exceptionregexpurllist",
					"/lists/exceptionurllist",
					"/lists/greyurllist",
					"/lists/logregexpurllist",
					"/lists/logurllist",
					"/lists/urlregexplist",
					"/lists/exceptionfilesitelist",
					"/lists/exceptionfileurllist",
					"/lists/searchengineregexplist",
					"/lists/bannedsearchtermlist",
					"/lists/weightedsearchtermlist",
					"/lists/exceptionsearchtermlist",
					"/lists/contentregexplist",
					"/lists/exceptionextensionlist",
					"/lists/bannedextensionlist",
					"/lists/exceptionmimetypelist",
					"/lists/bannedmimetypelist",
					"/lists/headerregexplist",
					"/lists/bannedregexpheaderlist",
					"/lists/authplugins/ipgroups",
					"/lists/contentscanners/exceptionvirusextensionlist",
					"/lists/contentscanners/exceptionvirusmimetypelist",
					"/lists/contentscanners/exceptionvirussitelist",
					"/lists/contentscanners/exceptionvirusurllist",
					"/lists/pics");
	
	
	$dansguardian_dir="/usr/local/etc/dansguardian";
	foreach ($files as $file)
		if (! file_exists($dansguardian_dir.$file.'.sample')){
			$new_file="";
			$install_file=file($dansguardian_dir.$file);
			foreach ($install_file as $line)
				if (! preg_match("/Include/",$line))
					$new_file.= $line;
			file_put_contents($dansguardian_dir.$file.'.sample',$new_file,LOCK_EX);
			}
		
	$load_samples=0;
	
	#ssl men-in-the-middle feature
	$dirs=array("/var/log/dansguardian/stats","/etc/ssl/demoCA","/etc/ssl/demoCA/private","/etc/ssl/demoCA/crl","/etc/ssl/demoCA/certs",$dansguardian_dir."/ssl/generatedcerts",$dansguardian_dir."/ssl/generatedlinks");
	foreach ($dirs as $dir)
		if (!is_dir($dir))
			mkdir ($dir,0755,true);
	$ca_cert = lookup_ca($dansguardian_config["dca"]);
	if ($ca_cert != false) {
		if(base64_decode($ca_cert['prv'])) {
			file_put_contents("/etc/ssl/demoCA/private/cakey.pem",base64_decode($ca_cert['prv']));
			$ca_pk = "caprivatekeypath = '/etc/ssl/demoCA/private/cakey.pem'";
			}
		if(base64_decode($ca_cert['crt'])) {
			$cert_hash=array();
			file_put_contents("/etc/ssl/demoCA/cacert.pem",base64_decode($ca_cert['crt']));
			exec("/usr/bin/openssl x509 -hash -noout -in /etc/ssl/demoCA/cacert.pem",$cert_hash);
			file_put_contents("/usr/local/share/certs/".$cert_hash[0].".0",base64_decode($ca_cert['crt']));
			$ca_pem = "cacertificatepath = '/etc/ssl/demoCA/cacert.pem'";
		$generatedcertpath= "generatedcertpath = '/etc/ssl/demoCA/certs/'";
		#generatedcertpath = ".$dansguardian_dir . "/ssl/generatedcerts";
		$generatedlinkpath= "generatedlinkpath = '".$dansguardian_dir . "/ssl/generatedlinks'";
			}
		$svr_cert = lookup_cert($dansguardian_config["dcert"]);
		if ($svr_cert != false) {
			if(base64_decode($svr_cert['prv'])) {
				file_put_contents("/etc/ssl/demoCA/private/serverkey.pem",base64_decode($svr_cert['prv']));
				$cert_key = "certprivatekeypath = '/etc/ssl/demoCA/private/serverkey.pem' ";
			}
		}
	}
	
	#contentscanners preg_replace patterns
	$match[0]="/(conf)/";
	$match[1]="/(\/usr.local)/";
	$match[2]="/,/";
	$replace[0]="$1'";
	$replace[1]="contentscanner = '$1";
	$replace[2]="\n";
	
	$contentscanners=preg_replace($match,$replace,$dansguardian_config['content_scanners']);
	
	#includes preg_replace patterns
	$match[0]="/(.)$/";
	$match[1]="/\/usr.local/";
	$match[2]="/,/";
	$replace[0]="$1>\n";
	$replace[1]="\n.Include</usr/local";
	$replace[2]=">";

	#phrase ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardianphraseacl']['config'])){
		$banned_file=file("/usr/local/etc/dansguardian/lists/bannedphraselist");
		foreach($banned_file as $file_line)
			if (preg_match ("/^.Include<(\S+)>/",$file_line,$matches))
			 	$banned_includes .= $matches[1].",";
		
		$weighted_file=file("/usr/local/etc/dansguardian/lists/weightedphraselist");
		foreach($weighted_file as $file_line)
			if (preg_match ("/^.Include<(\S+)>/",$file_line,$matches))
			 	$weighted_includes .= $matches[1].",";
		$config['installedpackages']['dansguardianphraseacl']['config'][0]=array('name'=>'Default',
																			    'description'=>'Default Phrase access list setup',
																				'banned_enabled'=> "on",
																				'weighted_enabled'=> "on",
																				'exception_enabled'=> "on",
																				'banned_includes' => substr($banned_includes,0,-1),
																				'weighted_includes' => substr($weighted_includes,0,-1));
	}
	#loop on array
	$count=0;	
	if (is_array($config['installedpackages']['dansguardianphraseacl']['config']))
		foreach($config['installedpackages']['dansguardianphraseacl']['config'] as $dansguardian_phrase){
			#bannedphraselist
			if($dansguardian_phrase['banned_phraselist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedphraselist.sample')){
				$config['installedpackages']['dansguardianphraseacl']['config'][$count]['banned_phraselist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedphraselist.sample'));
				$load_samples++;
			}
			$includes=preg_replace($match,$replace,$dansguardian_phrase['banned_includes']);	
			file_put_contents($dansguardian_dir."/lists/bannedphraselist.".$dansguardian_phrase['name'],($dansguardian_phrase['banned_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianphraseacl']['config'][$count]['banned_phraselist']).$includes:""),LOCK_EX);
						
			#weightedphraselist
			if($dansguardian_phrase['weighted_phraselist'] == "" && file_exists ($dansguardian_dir.'/lists/weightedphraselist.sample')){
				$config['installedpackages']['dansguardianphraseacl']['config'][$count]['weighted_phraselist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/weightedphraselist.sample'));
				$load_samples++;
			}
			$includes=preg_replace($match,$replace,$dansguardian_phrase['weighted_includes']);	
			file_put_contents($dansguardian_dir."/lists/weightedphraselist.".$dansguardian_phrase['name'],($dansguardian_phrase['weighted_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianphraseacl']['config'][$count]['weighted_phraselist']).$includes:""),LOCK_EX);
				
			#exceptionphraselist
			if($dansguardian_phrase['exception_phraselist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionphraselist.sample')){
				$config['installedpackages']['dansguardianphraseacl']['config'][$count]['exception_phraselist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionphraselist.sample'));
				$load_samples++;
			}
			file_put_contents($dansguardian_dir."/lists/exceptionphraselist.".$dansguardian_phrase['name'],($dansguardian_phrase['exception_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianphraseacl']['config'][$count]['exception_phraselist']):""),LOCK_EX);
		$count++;	
		}
		
	#site ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardiansiteacl']['config']))
		$config['installedpackages']['dansguardiansiteacl']['config'][0]=array('name'=>'Default',
																			   'description'=>'Default Site access list setup',
																				'exceptionsite_enabled'=> "on",
																				'bannedsite_enabled'=> "on",
																				'greysite_enabled'=> "on",
																				'urlsite_enabled'=> "on");
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardiansiteacl']['config'] as $dansguardian_site){
		#exceptionsitelist
		if($dansguardian_site['exception_sitelist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionsitelist.sample')){
			$config['installedpackages']['dansguardiansiteacl']['config'][$count]['exception_sitelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionsitelist.sample'));
			$load_samples++;
		}
		$includes=preg_replace($match,$replace,$dansguardian_site['exception_includes']);
		file_put_contents($dansguardian_dir."/lists/exceptionsitelist.".$dansguardian_site['name'],($dansguardian_site['exceptionsite_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansiteacl']['config'][$count]['exception_sitelist']).$includes:""),LOCK_EX);	
			
		#exceptionfilesitelist
		if($dansguardian_site['exceptionfile_sitelist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionfilesitelist.sample')){
			$config['installedpackages']['dansguardiansiteacl']['config'][$count]['exceptionfile_sitelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionfilesitelist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/exceptionfilesitelist.".$dansguardian_site['name'],($dansguardian_site['exceptionsite_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansiteacl']['config'][$count]['exceptionfile_sitelist']):""),LOCK_EX);
			
		#bannedsitelist
		if($dansguardian_site['banned_sitelist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedsitelist.sample')){
			$config['installedpackages']['dansguardiansiteacl']['config'][$count]['banned_sitelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedsitelist.sample'));
			$load_samples++;
		}
		$includes=preg_replace($match,$replace,$dansguardian_site['banned_includes']);	
		file_put_contents($dansguardian_dir."/lists/bannedsitelist.".$dansguardian_site['name'],($dansguardian_site['bannedsite_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansiteacl']['config'][$count]['banned_sitelist']).$includes:""),LOCK_EX);
		
		#greysitelist
		if($dansguardian_site['grey_sitelist'] == "" && file_exists ($dansguardian_dir.'/lists/greysitelist.sample')){
			$config['installedpackages']['dansguardiansiteacl']['config'][$count]['grey_sitelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/greysitelist.sample'));
			$load_samples++;
		}	
		file_put_contents($dansguardian_dir."/lists/greysitelist.".$dansguardian_site['name'],($dansguardian_site['greysite_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansiteacl']['config'][$count]['grey_sitelist']):""),LOCK_EX);
			
		#logsitelist
		if($dansguardian_site['log_sitelist'] == "" && file_exists ($dansguardian_dir.'/lists/logsitelist.sample')){
			$config['installedpackages']['dansguardiansiteacl']['config'][$count]['log_sitelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/logsitelist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/logsitelist.".$dansguardian_site['name'],($dansguardian_site['urlsite_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansiteacl']['config'][$count]['log_sitelist']):""),LOCK_EX);	
		$count++;
	}	
	
	#URL ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardianurlacl']['config']))
		$config['installedpackages']['dansguardianurlacl']['config'][0]=array('name'=>'Default',
																			   'description'=>'Default Url access list setup',
																				'bannedurl_enabled'=> "on",
																				'exceptionurl_enabled'=> "on",
																				'contenturl_enabled'=> "on",
																				'greyurl_enabled'=> "on");
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardianurlacl']['config'] as $dansguardian_url){
		#bannedurllist
		if($dansguardian_url['banned_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['banned_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedurllist.sample'));
			$load_samples++;
		}
		$includes=preg_replace($match,$replace,$dansguardian_url['banned_includes']);
		file_put_contents($dansguardian_dir."/lists/bannedurllist.".$dansguardian_url['name'],($dansguardian_url['bannedurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['banned_urllist']).$includes:""),LOCK_EX);
			
		#bannedregexpurllist
		if($dansguardian_url['bannedregexp_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedregexpurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['bannedregexp_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedregexpurllist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/bannedregexpurllist.".$dansguardian_url['name'],($dansguardian_url['bannedurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['bannedregexp_urllist']):""),LOCK_EX);
			
		#greyurllist
		if($dansguardian_url['grey_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/greyurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['grey_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/greyurllist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/greyurllist.".$dansguardian_url['name'],($dansguardian_url['greyurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['grey_urllist']):""),LOCK_EX);
			
		#exceptionfileurllist
		if($dansguardian_url['exceptionfile_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionfileurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['exceptionfile_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionfileurllist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/exceptionfileurllist.".$dansguardian_url['name'],($dansguardian_url['exceptionurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['exceptionfile_urllist']):""),LOCK_EX);
		
		#exceptionregexpurllist
		if($dansguardian_url['exceptionregexp_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionregexpurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['exceptionregexp_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionregexpurllist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/exceptionregexpurllist.".$dansguardian_url['name'],($dansguardian_url['exceptionurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['exceptionregexp_urllist']):""),LOCK_EX);
		
		#exceptionurllist
		if($dansguardian_url['exception_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['exception_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionurllist.sample'));
			$load_samples++;
		}
		$includes=preg_replace($match,$replace,$dansguardian_url['exception_includes']);
		file_put_contents($dansguardian_dir."/lists/exceptionurllist.".$dansguardian_url['name'],($dansguardian_url['exceptionurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['exception_urllist']).$includes:""),LOCK_EX);
			
		#urlregexplist
		if($dansguardian_url['modify_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/urlregexplist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['modify_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/urlregexplist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/urlregexplist.".$dansguardian_url['name'],($dansguardian_url['contenturl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['modify_urllist']):""),LOCK_EX);
					
		#logurllist
		if($dansguardian_url['log_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/logurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['log_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/logurllist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/logurllist.".$dansguardian_url['name'],($dansguardian_url['logurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['log_urllist']):""),LOCK_EX);
			
		#logregexpurllist
		if($dansguardian_url['logregexp_urllist'] == "" && file_exists ($dansguardian_dir.'/lists/logregexpurllist.sample')){
			$config['installedpackages']['dansguardianurlacl']['config'][$count]['logregexp_urllist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/logregexpurllist.sample'));
			$load_samples++;
		}
		file_put_contents($dansguardian_dir."/lists/logregexpurllist.".$dansguardian_url['name'],($dansguardian_url['logurl_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianurlacl']['config'][$count]['logregexp_urllist']):""),LOCK_EX);
	$count++;
	}

	#Pics ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardianpicsacl']['config']))
		$config['installedpackages']['dansguardianpicsacl']['config'][0]=array('name'=>'Default',
																			   'description'=>'Default file access list setup');
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardianpicsacl']['config'] as $dansguardian_pics){
		#pics
		if($dansguardian_pics['pics'] == "" && file_exists ($dansguardian_dir.'/lists/pics.sample')){
				$config['installedpackages']['dansguardianpicsacl']['config'][$count]['pics']=base64_encode(file_get_contents($dansguardian_dir.'/lists/pics.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/pics.".$dansguardian_pics['name'],($dansguardian_pics['pics_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianpicsacl']['config'][$count]['pics']):""),LOCK_EX);
		$count++;
	}
		
	#Search ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardiansearchacl']['config']))
		$config['installedpackages']['dansguardiansearchacl']['config'][0]=array('name'=>'Default',
																			   'description'=>'Default search engine list setup');
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardiansearchacl']['config'] as $dansguardian_search){
		#searchengineregexplist
		if($dansguardian_search['searchengineregexplist'] == "" && file_exists ($dansguardian_dir.'/lists/searchengineregexplist.sample')){
				$config['installedpackages']['dansguardiansearchacl']['config'][$count]['searchengineregexplist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/searchengineregexplist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/searchengineregexplist.".$dansguardian_search['name'],($dansguardian_search['regexp_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansearchacl']['config'][$count]['searchengineregexplist']):""),LOCK_EX);

		#bannedsearchtermlist
		if($dansguardian_search['banned_searchtermlist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedsearchtermlist.sample')){
				$config['installedpackages']['dansguardiansearchacl']['config'][$count]['banned_searchtermlist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedsearchtermlist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/bannedsearchtermlist.".$dansguardian_search['name'],($dansguardian_search['banned_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansearchacl']['config'][$count]['banned_searchtermlist']):""),LOCK_EX);
			
		#weightedsearchtermlist
		if($dansguardian_search['weighted_searchtermlist'] == "" && file_exists ($dansguardian_dir.'/lists/weightedsearchtermlist.sample')){
				$config['installedpackages']['dansguardiansearchacl']['config'][$count]['weighted_searchtermlist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/weightedsearchtermlist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/weightedsearchtermlist.".$dansguardian_search['name'],($dansguardian_search['weighted_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansearchacl']['config'][$count]['weighted_searchtermlist']):""),LOCK_EX);
			
		#exceptionsearchtermlist
		if($dansguardian_search['exception_searchtermlist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionsearchtermlist.sample')){
				$config['installedpackages']['dansguardiansearchacl']['config'][$count]['exception_searchtermlist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionsearchtermlist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/exceptionsearchtermlist.".$dansguardian_search['name'],($dansguardian_search['exception_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiansearchacl']['config'][$count]['exception_searchtermlist']):""),LOCK_EX);
		$count++;
		}
		
	#File ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardianfileacl']['config']))
		$config['installedpackages']['dansguardianfileacl']['config'][0]=array('name'=>'Default',
																			   'description'=>'Default file access list setup',
																				'exception_enabled'=> "on",
																				'banned_enabled'=> "on");
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardianfileacl']['config'] as $dansguardian_file){
			#exceptionextensionlist
			if($dansguardian_file['exception_extensionlist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionextensionlist.sample')){
					$config['installedpackages']['dansguardianfileacl']['config'][$count]['exception_extensionlist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionextensionlist.sample'));
					$load_samples++;
				}
			file_put_contents($dansguardian_dir."/lists/exceptionextensionlist.".$dansguardian_file['name'],($dansguardian_file['exception_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianfileacl']['config'][$count]['exception_extensionlist']):""),LOCK_EX);

			#exceptionmimetypelist
			if($dansguardian_file['exception_mimetypelist'] == "" && file_exists ($dansguardian_dir.'/lists/exceptionmimetypelist.sample')){
					$config['installedpackages']['dansguardianfileacl']['config'][$count]['exception_mimetypelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/exceptionmimetypelist.sample'));
					$load_samples++;
				}
			file_put_contents($dansguardian_dir."/lists/exceptionmimetypelist.".$dansguardian_file['name'],($dansguardian_file['exception_enabled']?dg_text_area_decode($config['installedpackages']['exception_mimetypelist']):""),LOCK_EX);
			
			#bannedextensionlist
			if($dansguardian_file['banned_extensionlist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedextensionlist.sample')){
					$config['installedpackages']['dansguardianfileacl']['config'][$count]['banned_extensionlist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedextensionlist.sample'));
					$load_samples++;
				}
			file_put_contents($dansguardian_dir."/lists/bannedextensionlist.".$dansguardian_file['name'],($dansguardian_file['banned_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianfileacl']['config'][$count]['banned_extensionlist']):""),LOCK_EX);
			
			#bannedmimetypelist
			if($dansguardian_file['banned_mimetypelist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedmimetypelist.sample')){
					$config['installedpackages']['dansguardianfileacl']['config'][$count]['banned_mimetypelist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedmimetypelist.sample'));
					$load_samples++;
				}
			file_put_contents($dansguardian_dir."/lists/bannedmimetypelist.".$dansguardian_file['name'],($dansguardian_file['banned_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianfileacl']['config'][$count]['banned_mimetypelist']):""),LOCK_EX);
			$count++;
		}
		
	#header ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardianheaderacl']['config']))
		$config['installedpackages']['dansguardianheaderacl']['config'][0]=array('name'=>'Default',
																				  'description'=>'Default header access list setup');
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardianheaderacl']['config'] as $dansguardian_header){
		#headerregexplist
		if($dansguardian_header['header_regexplist'] == "" && file_exists ($dansguardian_dir.'/lists/headerregexplist.sample')){
				$config['installedpackages']['dansguardianheaderacl']['config'][$count]['header_regexplist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/headerregexplist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/headerregexplist.".$dansguardian_header['name'],($dansguardian_header['regexp_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianheaderacl']['config'][$count]['header_regexplist']):""),LOCK_EX);

		#bannedregexpheaderlist
		if($dansguardian_header['banned_regexpheaderlist'] == "" && file_exists ($dansguardian_dir.'/lists/bannedregexpheaderlist.sample')){
				$config['installedpackages']['dansguardianheaderacl']['config'][$count]['banned_regexpheaderlist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/bannedregexpheaderlist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/bannedregexpheaderlist.".$dansguardian_header['name'],($dansguardian_header['banned_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianheaderacl']['config'][$count]['banned_regexpheaderlist']):""),LOCK_EX);
			
		$count++;
		}
		
	#Content ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardiancontentacl']['config']))
	$config['installedpackages']['dansguardiancontentacl']['config'][0]=array('name'=>'Default',
																			  'description'=>'Default content setup');
	#loop on array
	$count=0;
	foreach($config['installedpackages']['dansguardiancontentacl']['config'] as $dansguardian_content){
		#content_regexplist
		if($dansguardian_content['content_regexplist'] == "" && file_exists ($dansguardian_dir.'/lists/contentregexplist.sample')){
				$config['installedpackages']['dansguardiancontentacl']['config'][$count]['content_regexplist']=base64_encode(file_get_contents($dansguardian_dir.'/lists/contentregexplist.sample'));
				$load_samples++;
			}
		file_put_contents($dansguardian_dir."/lists/contentregexplist.".$dansguardian_content['name'],($dansguardian_content['content_enabled']?dg_text_area_decode($config['installedpackages']['dansguardiancontentacl']['config'][$count]['content_regexplist']):""),LOCK_EX);
		$count++;
		}
		
	#Antivirus ACL
	#create a default setup if not exists
	if (!is_array($config['installedpackages']['dansguardianantivirusacl']['config']))
		$config['installedpackages']['dansguardianantivirusacl']['config'][0]=array();
	
	$dansguardian_antivirus=$config['installedpackages']['dansguardianantivirusacl']['config'][0];
	#exceptionvirusmimetypelist
	if($dansguardian_antivirus['mime_list'] == "" && file_exists ($dansguardian_dir.'/lists/contentscanners/exceptionvirusmimetypelist.sample')){
		$config['installedpackages']['dansguardianantivirusacl']['config'][0]['mime_list']=base64_encode(file_get_contents($dansguardian_dir.'/lists/contentscanners/exceptionvirusmimetypelist.sample'));
		$load_samples++;
	}
	file_put_contents($dansguardian_dir."/lists/contentscanners/exceptionvirusmimetypelist",($dansguardian_antivirus['mime_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianantivirusacl']['config'][0]['mime_list']):""),LOCK_EX);	
			
	#exceptionvirussitelist
	if($dansguardian_antivirus['site_list'] == "" && file_exists ($dansguardian_dir.'/lists/contentscanners/exceptionvirussitelist.sample')){
		$config['installedpackages']['dansguardianantivirusacl']['config'][0]['site_list']=base64_encode(file_get_contents($dansguardian_dir.'/lists/contentscanners/exceptionvirussitelist.sample'));
		$load_samples++;
	}
	file_put_contents($dansguardian_dir."/lists/contentscanners/exceptionvirussitelist",($dansguardian_antivirus['site_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianantivirusacl']['config'][0]['site_list']):""),LOCK_EX);
		
	#exceptionvirusurllist
	if($dansguardian_antivirus['url_list'] == "" && file_exists ($dansguardian_dir.'/lists/contentscanners/exceptionvirusurllist.sample')){
		$config['installedpackages']['dansguardianantivirusacl']['config'][0]['url_list']=base64_encode(file_get_contents($dansguardian_dir.'/lists/contentscanners/exceptionvirusurllist.sample'));
		$load_samples++;
	}
	file_put_contents($dansguardian_dir."/lists/contentscanners/exceptionvirusurllist",($dansguardian_antivirus['url_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianantivirusacl']['config'][0]['url_list']):""),LOCK_EX);
		
	#exceptionvirusextensionlist
	if($dansguardian_antivirus['extension_list'] == "" && file_exists ($dansguardian_dir.'/lists/contentscanners/exceptionvirusextensionlist.sample')){
		$config['installedpackages']['dansguardianantivirusacl']['config'][0]['extension_list']=base64_encode(file_get_contents($dansguardian_dir.'/lists/contentscanners/exceptionvirusextensionlist.sample'));
		$load_samples++;
	}	
	file_put_contents($dansguardian_dir."/lists/contentscanners/exceptionvirusextensionlist",($dansguardian_antivirus['extension_enabled']?dg_text_area_decode($config['installedpackages']['dansguardianantivirusacl']['config'][0]['extension_list']):""),LOCK_EX);
		
	#log report
	if ($dansguardian_log['report_file']=="" && file_exists("/usr/local/share/dansguardian/languages/".$reportlanguage."/template.html")){
		$report_file=file_get_contents("/usr/local/share/dansguardian/languages/".$reportlanguage."/template.html");
		$report_file=preg_replace('/<.*(html|head)>/','',$report_file);
		$config['installedpackages']['dansguardianlog']['config'][0]['report_file']=base64_encode($report_file);
		$dansguardian_log['report_file']=base64_encode($report_file);
		$load_samples++;
	}	

	if($load_samples > 0)
		write_config();

	#Filtergroups
	if (!is_array($config['installedpackages']['dansguardiangroups']['config']))
		$config['installedpackages']['dansguardiangroups']['config'][0]=array('name'=>'Default',
																			   'description'=>'Default dansguardian filtergroup',
																				'picsacl'=> "Default",
																				'phraseacl'=> "Default",
																				'siteacl'=> "Default",
																				'extensionacl'=> "Default",
																				'headeracl'=> "Default",
																				'contentacl'=> "Default",
																				'searchacl'=> "Default",
																				'urlacl'=> "Default",
																				'group_options' => "scancleancache,infectionbypasserrorsonly",
																				'reportinglevel'=>'3',
																				'mode'=> "1");

	$groups=array("scancleancache","hexdecodecontent","blockdownloads","enablepics","deepurlanalysis","infectionbypasserrorsonly","disablecontentscan","sslcertcheck","sslmitm");
	#loop on array
	$count=1;
	$user_xml="";
	$filtergroupslist="";
	foreach($config['installedpackages']['dansguardiangroups']['config'] as $dansguardian_groups){
		$dansguardian_group_name=strtolower($dansguardian_groups['name']);
		$dgfg[$count]=$dansguardian_group_name;
		$dansguardian_groups['blockdownloads']=($dansguardian_groups['blockdownloads']?$dansguardian_groups['blockdownloads']:"off");
		$dansguardian_groups['weightedphrasemode']=(preg_match("/\d/",$dansguardian_groups['weightedphrasemode'])?$dansguardian_groups['weightedphrasemode']:$dansguardian_config['weightedphrasemode']);
		$dansguardian_groups['naughtynesslimit']=($dansguardian_groups['naughtynesslimit']?$dansguardian_groups['naughtynesslimit']:"50");
		$dansguardian_groups['searchtermlimit']=($dansguardian_groups['searchtermlimit']?$dansguardian_groups['searchtermlimit']:"30");
		$dansguardian_groups['categorydisplaythreshold']=($dansguardian_groups['categorydisplaythreshold']?$dansguardian_groups['categorydisplaythreshold']:"0");
		$dansguardian_groups['embeddedurlweight']=($dansguardian_groups['embeddedurlweight']?$dansguardian_groups['embeddedurlweight']:"0");
		$dansguardian_groups['bypass']=($dansguardian_groups['bypass']?$dansguardian_groups['bypass']:"0");
		$dansguardian_groups['infectionbypass']=($dansguardian_groups['infectionbypass']?$dansguardian_groups['infectionbypass']:"0");
		$dansguardian_groups['mitmkey']=($dansguardian_groups['mitmkey']?$dansguardian_groups['mitmkey']:"dgs3dD3da");
		foreach ($groups as $group)
			$dansguardian_groups[$group]=(preg_match("/$group/",$dansguardian_groups['group_options'])?"on":"off");
		include("/usr/local/pkg/dansguardianfx.conf.template");
		file_put_contents($dansguardian_dir."/dansguardianf".$count.".conf", $dgf, LOCK_EX);
		
		if ($config['installedpackages']['dansguardianusers']['config'][0][$dansguardian_group_name]!=""){
			$import_users = explode("\n", base64_decode($config['installedpackages']['dansguardianusers']['config'][0][$dansguardian_group_name]));
			asort($import_users);
			$config['installedpackages']['dansguardianusers']['config'][0][$dansguardian_group_name]=base64_encode(implode("\n", $import_users));
			foreach ($import_users as $new_user){
				if (preg_match("/(\S+)\s+(\S+)/",$new_user,$matches))
				 $filtergroupslist.=$matches[1]."=filter".$count." #".$matches[2]."\n";
				elseif (preg_match("/(\S+)/",$new_user,$matches))
					$filtergroupslist.=$matches[1]."=filter".$count."\n";
				}
			}
		if ($config['installedpackages']['dansguardianips']['config'][0][$dansguardian_group_name]!=""){
			$import_ips = explode("\n", base64_decode($config['installedpackages']['dansguardianips']['config'][0][$dansguardian_group_name]));
			asort($import_ips);
			$config['installedpackages']['dansguardianips']['config'][0][$dansguardian_group_name]=base64_encode(implode("\n", $import_ips));
			foreach ($import_ips as $new_ip){
				if (preg_match("/(\S+)\s+(.*)/",$new_ip,$matches))
				 $filtergroupsiplist.=$matches[1]." = filter".$count." #".$matches[2]."\n";
				elseif (preg_match("/(\S+)/",$new_ip,$matches))
					$filtergroupsiplist.=$matches[1]." = filter".$count."\n";
				}
			}
		$filtergroup_count=count($import_users);
		$filtergroupip_count=count($import_ips);
		#Default group catch all unauth groups as well non listed users
		if($count > 1)
		$user_xml .=<<<EOF
		<field>
			<name>{$dansguardian_groups['description']} ({$filtergroup_count})</name>
			<type>listtopic</type>
		</field>
		<field>
			<fieldname>{$dansguardian_group_name}</fieldname>
			<fielddescr>{$dansguardian_groups['name']}</fielddescr>
			<description><![CDATA[Include users for this group one per line<br>Hint:PFSENSE\marcelloc #Marcello Coutinho]]></description>
			<type>textarea</type>
			
			<cols>80</cols><rows>12</rows>
			<encoding>base64</encoding>
		</field>		
EOF;
		$ips_xml .=<<<EOF
		<field>
			<name>{$dansguardian_groups['description']} ({$filtergroupip_count})</name>
			<type>listtopic</type>
		</field>
		<field>
			<fieldname>{$dansguardian_group_name}</fieldname>
			<fielddescr>{$dansguardian_groups['name']}</fielddescr>
			<description><![CDATA[Include ip addresses and or ipadresses/netmask for this group one per line<br>Hint:192.168.1.0/255.255.255.0<br>192.168.1.5]]></description>
			<type>textarea</type>
			
			<cols>80</cols><rows>12</rows>
			<encoding>base64</encoding>
		</field>		
EOF;
		
		$count++;
	}
	if ($user_xml==""){
		$user_xml .=<<<EOF
		<field>
			<name>Users</name>
			<type>listtopic</type>
		</field>
		<field>
			<fielddescr>Users</fielddescr>
			<fieldname>info_checkbox</fieldname>
			<type>checkbox</type>
			<description><![CDATA[Dansguardian users are required only when you have more then one group.<br>All unauthenticated users or unlisted uses will match first filter group.]]></description>
		</field>
EOF;
	}
	#Create/update filtergroupslist
	file_put_contents($dansguardian_dir."/lists/filtergroupslist",$filtergroupslist,LOCK_EX);
	#Create/update filtergroupsiplist
	file_put_contents($dansguardian_dir."/lists/authplugins/ipgroups",$filtergroupsiplist,LOCK_EX);
	#Create/update userlist xml file
	$ips_xml_header=file_get_contents("/usr/local/pkg/dansguardian_ips_header.xml");
	$user_xml_header=file_get_contents("/usr/local/pkg/dansguardian_users_header.xml");
	$user_xml_footer=file_get_contents("/usr/local/pkg/dansguardian_users_footer.xml");
	file_put_contents("/usr/local/pkg/dansguardian_users.xml",$user_xml_header.$user_xml.$user_xml_footer,LOCK_EX);
	file_put_contents("/usr/local/pkg/dansguardian_ips.xml",$ips_xml_header.$ips_xml.$user_xml_footer,LOCK_EX);
	
	#Create report template
	if (is_dir("/usr/local/share/dansguardian/languages/".$reportlanguage)) 
		file_put_contents("/usr/local/share/dansguardian/languages/".$reportlanguage."/template.html",dg_text_area_decode($dansguardian_log['report_file']),LOCK_EX);
	
	#check blacklist download files
	if ($dansguardian_blacklist['cron']=="force_download"){
		log_error("Blacklist udpate process started");
		file_notice("Dansguardian - Blacklist udpate process started","");
		file_put_contents("/root/dansguardian_custom.script",base64_decode($dansguardian_blacklist['custom_script']),LOCK_EX);
		if ($dansguardian_blacklist['enable_custom_script'] && $dansguardian_blacklist['custom_script'] != "")
			mwexec_bg("/root/dansguardian_custom.script");
		else
			mwexec_bg("/usr/local/bin/php /usr/local/www/dansguardian.php fetch_blacklist");
		}
	#update xml categories from downloaded file
	if ($dansguardian_blacklist['cron']=="force_update"){
		$config['installedpackages']['dansguardianblacklist']['config'][0]['cron']="never";
		mwexec_bg("/usr/local/bin/php /usr/local/www/dansguardian.php update_lists");
	}
	
	#Import default blacklists 
	if (!is_array($config['installedpackages']['dansguardianblacklistsurls']['config']))
		mwexec_bg("/usr/local/bin/php /usr/local/www/dansguardian.php update_lists");

	#get clamav user
	$cconf="/usr/local/etc/clamd.conf";
	$cconf_file=file_get_contents($cconf);
	if (preg_match("/User (\w+)/",$cconf_file,$matches)){
		$daemonuser = $matches[1];
		$daemongroup = 'nobody';
	}
	else{
		$daemonuser = 'nobody';
		$daemongroup = 'nobody';
	}
	$filtergroups=($count > 1?($count -1):1);	
	
	$filterip="";
	$filterports="";
	foreach (explode(",", $dansguardian['interface']) as $i => $iface) {
		$real_ifaces[] = dg_get_real_interface_address($iface);
		if($real_ifaces[$i][0])
			$filterip .="filterip = ".$real_ifaces[$i][0]."\n";
			$filterports.="filterports = ".$filterport."\n";
		}
	$filterip=($filterip==""?"filterip = ":$filterip);
	$filterports=($filterports==""?"filterports = $filterport":$filterports);
	include("/usr/local/pkg/dansguardian.conf.template");	

	#check cron_tab
	$new_cron=array();
	$cron_found=0;
	if (is_array($config['cron']['item']))
		foreach($config['cron']['item'] as $cron)
			if (!preg_match("/usr.local.(bin.freshclam|www.dansguardian)/",$cron["command"])){
				$cron_found++;
				$new_cron['item'][]=$cron;
			}
	$cron_cmd="/usr/local/bin/freshclam";
	if($dansguardian_config['cron'] && preg_match("/clamd/",$dansguardian_config['content_scanners']))
		switch ($dansguardian_config['cron']){
			case "day":
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"0",
											"mday" 	 =>	"*",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd);
				$config['cron']=$new_cron;
				$cron_found++;
				break;
			case "02days":
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"0",
											"mday" 	 =>	"*/2",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd);
				$config['cron']=$new_cron;
				$cron_found++;
				break;
			case "week":
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"0",
											"mday" 	 =>	"*/7",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd);
				$config['cron']=$new_cron;
				$cron_found++;
				break;
		}
	$cron_cmd="/usr/local/bin/php /usr/local/www/dansguardian.php fetch_blacklist";
	if($dansguardian_blacklist['cron'])
		switch ($dansguardian_blacklist['cron']){
			case "day":
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"0",
											"mday" 	 =>	"*",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd);
				$config['cron']=$new_cron;
				$cron_found++;
				break;
			case "02days":
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"0",
											"mday" 	 =>	"*/2",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd);
				$config['cron']=$new_cron;
				$cron_found++;
				break;
			case "week":
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"0",
											"mday" 	 =>	"*/7",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd);
				$config['cron']=$new_cron;
				$cron_found++;
				break;
		}

	$cron_cmd="/usr/local/bin/php /usr/local/www/dansguardian_ldap.php";
	if (is_array($config['installedpackages']['dansguardiangroups']['config']))
	 foreach ($config['installedpackages']['dansguardiangroups']['config'] as $dansguardian_groups){
	  if(preg_match('/(\d+)m/',$dansguardian_groups['freq'],$matches)){
				$new_cron['item'][]=array(	"minute" =>	"*/".$matches[1],
											"hour"	 =>	"*",
											"mday" 	 =>	"*",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd." ".$dansguardian_groups['name']);
				$config['cron']=$new_cron;
				$cron_found++;
		}
	  if(preg_match('/(\d+)h/',$dansguardian_groups['freq'],$matches)){
				$new_cron['item'][]=array(	"minute" =>	"0",
											"hour"	 =>	"*/".$matches[1],
											"mday" 	 =>	"*",
											"month"  =>	"*",
											"wday"	 =>	"*",
											"who"	 =>	"root",
											"command"=>	$cron_cmd." ".$dansguardian_groups['name']);
				$config['cron']=$new_cron;
				$cron_found++;
		}
	}
	#write files
	conf_mount_rw();

	write_config();
	
	#update cron
	if ($cron_found > 0){
		$config['cron']=$new_cron;
		configure_cron();
	}
	
	$dirs=array('/usr/local/etc/dansguardian/lists/bannedrooms/',
				'/var/log/dansguardian');

	foreach ($dirs as $dir)
		if (!is_dir($dir))
			mkdir ($dir,0755,true);
	
	#update file owner
	mwexec("chown -R $daemonuser:$daemongroup /usr/local/etc/dansguardian");
	mwexec("chown -R $daemonuser:$daemongroup /var/log/dansguardian");
	
	#create config files
	file_put_contents($dansguardian_dir."/dansguardian.conf", $dg, LOCK_EX);
	
	#check virus_scanner options
	$libexec_dir="/usr/local/libexec/dansguardian/";
	if (preg_match("/clamd/",$dansguardian_config['content_scanners'])){
			
		if (!(file_exists('/var/db/clamav/main.cvd')||file_exists('/var/db/clamav/main.cld'))){
			file_notice("Dansguardian - No antivirus database found for clamav, running freshclam in background.","");
			log_error('No antivirus database found for clamav, running freshclam in background.');
			mwexec_bg('/usr/local/bin/freshclam');
			}
				
			$match=array();
			$match[0]='/NO/';
			$replace=array();
			$replace[0]='YES';

			#clamdscan.conf dansguardian file	
			$cconf="/usr/local/etc/dansguardian/contentscanners/clamdscan.conf";
			$cconf_file=file_get_contents($cconf);
			if (preg_match('/#clamdudsfile/',$cconf_file)){
				$cconf_file=preg_replace('/#clamdudsfile/','clamdudsfile',$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
			
			#clamd conf file
			$cconf="/usr/local/etc/clamd.conf";
			$cconf_file=file_get_contents($cconf);
			if (preg_match("/User (\w+)/",$cconf_file,$matches)){
				#clamd script file
				$script='/usr/local/etc/rc.d/clamav-clamd';
				$script_file=file($script);
				foreach ($script_file as $script_line){
					if(preg_match("/command=/",$script_line)){
						$new_clamav_startup.= "/bin/mkdir /var/run/clamav\n";
						$new_clamav_startup.= "chown ".$matches[1]." /var/run/clamav\n";
						$new_clamav_startup.=$script_line;
						}
					elseif(!preg_match("/(mkdir|chown|sleep|mailscanner)/",$script_line)) {
						$new_clamav_startup.=preg_replace("/NO/","YES",$script_line);
						}
					}
				file_put_contents($script, $new_clamav_startup, LOCK_EX);
				chmod ($script,0755);
				if (file_exists('/var/run/dansguardian.pid'))
					mwexec("$script stop");
				unlink_if_exists("/tmp/.dguardianipc");
				unlink_if_exists("/tmp/.dguardianurlipc");
				mwexec_bg("$script start");
			}	
	}


	$script='/usr/local/etc/rc.d/dansguardian';
	if($config['installedpackages']['dansguardian']['config'][0]['enable']){
		$script_file=file_get_contents($script);
		if (preg_match('/NO/',$script_file)){
			$script_file=preg_replace("/NO/","YES",$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
			}
		chmod ($script,0755);
		mwexec("$script stop");			
		mwexec_bg("$script start");
	}
	else{
		mwexec("$script stop");
		chmod ($script,0444);	
	}
	
	if (!file_exists('/usr/local/etc/dansguardian/lists/phraselists/pornography/weighted_russian_utf8'))
		file_put_contents('/usr/local/etc/dansguardian/lists/phraselists/pornography/weighted_russian_utf8',"",LOCK_EX);
		
	conf_mount_ro();
	$synconchanges = $config['installedpackages']['dansguardiansync']['config'][0]['synconchanges'];
	if(!$synconchanges && !$syncondbchanges) 
		return;
	log_error("[dansguardian] dansguardian_xmlrpc_sync.php is starting.");
	foreach ($config['installedpackages']['dansguardiansync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		$sync_type = $sh['sync_type'];
		if($password && $sync_to_ip)
			dansguardian_do_xmlrpc_sync($sync_to_ip, $password,$sync_type);
		}
	}
	log_error("[dansguardian] dansguardian_xmlrpc_sync.php is ending.");

}

function dansguardian_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (preg_match("/^(name|description)$/",$key) && $value == "")
				$input_errors[] = "{$key} could not be empty.";
		else if ($key == "name" && $value=="sample")
				$input_errors[] = "{$value} cannot be used as name.";
		else if ($key == "name" && preg_match("/\W/",$value)) 
				$input_errors[] = "{$value} cannot be used as name. Use only a-z 0-9 characters";
		else if (empty($value))
				continue;
		else if($key == "freq" && (!preg_match("/^\d+(h|m|d)$/",$value) || $value == 0))
				$input_errors[] = "A valid number with a time reference is required for the field 'Update Frequency'";
	}
}

function dansguardian_php_install_command() {
	conf_mount_rw();
	#create ca-root hashes from ca-root-nss package
	print "Creating root certificate bundle hashes from the Mozilla Project\n";
	$cas=file('/usr/local/share/certs/ca-root-nss.crt');
	$cert=0;
	foreach ($cas as $ca){
    	if (preg_match("/--BEGIN CERTIFICATE--/",$ca))
			$cert=1;
		if ($cert == 1)
			$crt.=$ca;
		if (preg_match("/-END CERTIFICATE-/",$ca)){
			file_put_contents("/tmp/cert.pem",$crt, LOCK_EX);
			$cert_hash=array();
			exec("/usr/bin/openssl x509 -hash -noout -in /tmp/cert.pem",$cert_hash);
			file_put_contents("/usr/local/share/certs/".$cert_hash[0].".0",$crt,LOCK_EX);
			$crt="";
			$cert=0;
			}
		}
	sync_package_dansguardian();
}

function dansguardian_php_deinstall_command() {
	mwexec("/usr/local/etc/rc.d/dansguardian stop");
	sleep(1);
	conf_mount_rw();
	chmod ("/usr/local/etc/rc.d/dansguardian",0444);
	conf_mount_ro();
}

function dansguardian_do_xmlrpc_sync($sync_to_ip, $password,$sync_type) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$sync_xml=$config['installedpackages']['dansguardiansync']['config'][0]['synconchanges'];
	if ($sync_xml){
		log_error("Include dansguardian config");
		$xml['dansguardian'] = $config['installedpackages']['dansguardian'];
		$xml['dansguardianantivirusacl'] = $config['installedpackages']['dansguardianantivirusacl'];
		$xml['dansguardianconfig'] = $config['installedpackages']['dansguardianconfig'];
		$xml['dansguardianblacklist'] = $config['installedpackages']['dansguardianblacklist'];
		$xml['dansguardianldap'] = $config['installedpackages']['dansguardianldap'];
		$xml['dansguardiancontentacl'] = $config['installedpackages']['dansguardiancontentacl'];
		$xml['dansguardianfileacl'] = $config['installedpackages']['dansguardianfileacl'];
		$xml['dansguardiangroups'] = $config['installedpackages']['dansguardiangroups'];
		$xml['dansguardianheaderacl'] = $config['installedpackages']['dansguardianheaderacl'];
		$xml['dansguardianlimits'] = $config['installedpackages']['dansguardianlimits'];
		$xml['dansguardianlog'] = $config['installedpackages']['dansguardianlog'];
		$xml['dansguardianphraseacl'] = $config['installedpackages']['dansguardianphraseacl'];
		$xml['dansguardianpicsacl'] = $config['installedpackages']['dansguardianpicsacl'];
		$xml['dansguardiansearchacl'] = $config['installedpackages']['dansguardiansearchacl'];
		$xml['dansguardiansiteacl'] = $config['installedpackages']['dansguardiansiteacl'];
		$xml['dansguardianurlacl'] = $config['installedpackages']['dansguardianurlacl'];
		$xml['dansguardianusers'] = $config['installedpackages']['dansguardianusers'];
		
	}
	if (count($xml) > 0){
		/* assemble xmlrpc payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($xml)
		);
	
		/* set a few variables needed for sync code borrowed from filter.inc */
		$url = $synchronizetoip;
		log_error("Beginning dansguardian XMLRPC sync to {$url}:{$port}.");
		$method = 'pfsense.merge_installedpackages_section_xmlrpc';
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials('admin', $password);
		if($g['debug'])
			$cli->setDebug(1);
		/* send our XMLRPC message and timeout after 250 seconds */
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while attempting dansguardian XMLRPC sync with {$url}:{$port}.";
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while attempting dansguardian XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} else {
			log_error("dansguardian XMLRPC sync successfully completed with {$url}:{$port}.");
		}
		
		/* tell dansguardian to reload our settings on the destionation sync host. */
		$method = 'pfsense.exec_php';
		$execcmd  = "require_once('/usr/local/pkg/dansguardian.inc');\n";
		$execcmd .= "sync_package_dansguardian();";
		
		/* assemble xmlrpc payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($execcmd)
		);
	
		log_error("dansguardian XMLRPC reload data {$url}:{$port}.");
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials('admin', $password);
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while attempting dansguardian XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while attempting dansguardian XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} else {
			log_error("dansguardian XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
		}
	}
}

?>
