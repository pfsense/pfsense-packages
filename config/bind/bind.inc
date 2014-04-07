<?PHP
/* $Id$ */
/*
	bind.inc
	part of the Bind package for pfSense
	Copyright (C) 2013 Juliano Oliveira/Adriano Brancher
	Copyright (C) 2013 Marcello Coutinho  
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
$shortcut_section = "bind";
require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('service-utils.inc');
if(!function_exists("filter_configure")) 
	require_once("filter.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('BIND_LOCALBASE', '/usr/pbi/bind-' . php_uname("m"));
else
	define('BIND_LOCALBASE','/usr/local');

define('CHROOT_LOCALBASE','/cf/named');

function bind_zone_validate($post, $input_errors){
	if (key_exists("mail",$_POST))
		$_POST['mail']=preg_replace("/@/",".",$post['mail']);
		
	switch ($_POST['type']){
		case 'slave':
			if( $_POST['slaveip'] == "")
				$input_errors[] = 'The field \'Master Zone IP\' is required for slave zones.';
		break;
		case 'forward':
			if( $_POST['forwarders'] == "")
			$input_errors[] = 'The field \'Forwarders\' is required for forward zones.';
		break;
		case 'redirect':
			$_POST['tll']=300;
			$_POST['refresh']=0;
			$_POST['serial']=0;
			$_POST['retry']=0;
			$_POST['expire']=0;
			$_POST['minimum']=0;
			if($_POST['mail']=='')
				$input_errors[] = "The field 'Mail Admin Zone' is required for {$_POST['type']} zones.";
			
		default:
			if($_POST['nameserver']=='')
				$input_errors[] = "The field 'Name server' is required for {$_POST['type']} zones.";
			for ($i=0;$i < count($_POST);$i++){
				if (key_exists("hostname$i",$_POST)){
					if ($_POST['reverso']=="on"){
						$_POST["hostvalue$i"]="";
						if (!preg_match("/(PTR|NS)/",$_POST["hosttype$i"]))
							$input_errors[] = 'On reverse zones, valid record types are NS or PTR';
						}
					if (preg_match("/(MX|NS)/",$_POST["hosttype$i"]))
						$_POST["hostname$i"]="";
					if (!preg_match("/(MX|NS)/",$_POST["hosttype$i"]) && $_POST["hostname$i"]=="")
						$input_errors[] = 'Record cannot be empty for '.$_POST["hosttype$i"].' type ';
					if ($_POST["hosttype$i"]=="MX" && $_POST["hostvalue$i"]=="")
						$_POST["hostvalue$i"]="10";
					if ($_POST["hosttype$i"]!="MX" && $_POST["hostvalue$i"]!="")
						$_POST["hostvalue$i"]="";
					if ($_POST["hostdst$i"]=="")
						$input_errors[] = 'Alias or IP address cannot be empty.';
				}
			}
		}
}

function bind_sync(){
	global $config;
	conf_mount_rw();
	//create rndc
	$rndc_confgen="/usr/local/sbin/rndc-confgen";
	if (!file_exists(BIND_LOCALBASE."/etc/rndc-confgen.pfsense") && file_exists($rndc_confgen)){
		exec("$rndc_confgen ",$rndc_conf);
		foreach($rndc_conf as $line)
			$confgen_file.="$line\n";
		file_put_contents(BIND_LOCALBASE."/etc/rndc-confgen.pfsense",$confgen_file);
		}
	if (file_exists(BIND_LOCALBASE."/etc/rndc-confgen.pfsense")){
		$rndc_conf=file(BIND_LOCALBASE."/etc/rndc-confgen.pfsense");
		$confgen="rndc.conf";
		$rndc_bindconf="";
		foreach ($rndc_conf as $line){
			if ($confgen =="rndc.conf"){
				if (!preg_match ("/^#/",$line))
					$rndc_file.=$line;
				}
			else{
				if (!preg_match ("/named.conf/",$line))
					$rndc_bindconf.=preg_replace('/#/',"",$line);
				}
			if (preg_match("/named.conf/",$line)){
				$confgen="named.conf";
				file_put_contents(BIND_LOCALBASE."/etc/rndc.conf",$rndc_file);
				}
		}
	}
	 
	$bind = $config["installedpackages"]["bind"]["config"][0];
	$bind_enable = $bind['enable_bind'];
	$bind_forwarder = $bind['bind_forwarder'];
	$forwarder_ips = $bind['bind_forwarder_ips'];
	$ram_limit = ($bind['bind_ram_limit']?$bind['bind_ram_limit']:"256M");
	$hide_version = $bind['bind_hide_version'];
	$bind_notify = $bind['bind_notify'];
	$custom_options = base64_decode($bind['bind_custom_options']);
	$bind_logging = $bind['bind_logging'];
	$bind_conf ="#Bind pfsense configuration\n";
	$bind_conf .="#Do not edit this file!!!\n\n";
	$bind_conf .= "$rndc_bindconf\n";
	$bind_conf .= <<<EOD

options {
	directory "/etc/namedb";
	pid-file "/var/run/named/pid";
	statistics-file "/var/log/named.stats";
	max-cache-size {$ram_limit};
	
EOD;
	// check response rate limit option
	//https://kb.isc.org/article/AA-01000/0/A-Quick-Introduction-to-Response-Rate-Limiting.html
	//http://ss.vix.su/~vjs/rl-arm.html
	if ($bind['rate_enabled']=="on"){
		$rate_limit=($bind['rate_limit']?$bind['rate_limit']:"15");
		$log_only=($bind['log_only']=="no"?"no":"yes");
		$bind_conf .= <<<EOD
		rate-limit {
			responses-per-second {$rate_limit};
			log-only {$log_only};
    		};
    	
EOD;
	}
	//check ips to listen on
	if (preg_match("/All/",$bind['listenon'])){
		$bind_listenonv6="any;";
		$bind_listenon="any;";
		}
	else{
		$bind_listenonv6="";
		$bind_listenon ="";
		foreach (explode(',',$bind['listenon']) as $listenon){
			if (is_ipaddrv6($listenon))
				$bind_listenonv6 .= $listenon."; ";
			elseif (is_ipaddr($listenon))
				$bind_listenon .= $listenon."; ";
			else{
				$listenon=(pfSense_get_interface_addresses(convert_friendly_interface_to_real_interface_name($listenon)));
				if (is_ipaddr($listenon['ipaddr']))
					$bind_listenon .= $listenon['ipaddr']."; ";
				if(is_ipaddrv6($listenon['ipaddr6']))
					$bind_listenonv6 .= $listenon['ipaddr6']."; ";
			}	
		}
	}
	$bind_listenonv6=($bind_listenonv6==""?"none;":$bind_listenonv6);
	$bind_listenon=($bind_listenon==""?"none;":$bind_listenon);
	//print "<PRE>$bind_listenonv6 $bind_listenon";
	if (key_exists("ipv6allow",$config['system'])){
		$bind_conf .="\t\tlisten-on-v6 { $bind_listenonv6 };\n";
		}
	$bind_conf .="\t\tlisten-on { $bind_listenon };\n";

	#forwarder config
	if ($bind_forwarder == on)
		$bind_conf .="\t\tforwarders { $forwarder_ips };\n";
	if ($bind_notify == on)
		$bind_conf .="\t\tnotify yes;\n"; 
	if ($hide_version == on)
		$bind_conf .="\t\tversion none;\n";

	$bind_conf .= preg_replace("/^/m","\t\t",$custom_options); 
	$bind_conf .= "\n\t};\n\n";
	
	if ($bind_logging == on){
		//check if bind is included on syslog
		$syslog_files=array("/etc/inc/system.inc","/var/etc/syslog.conf");
		$restart_syslog=0;
		foreach ($syslog_files as $syslog_file){
			$syslog_file_data=file_get_contents($syslog_file);
			if ( !preg_match("/dnsmasq,named,filterdns/",$syslog_file_data) || !preg_match("/'dnsmasq','named','filterdns'/",$syslog_file_data) ) {
				$syslog_file_data=preg_replace("/dnsmasq,filterdns/","dnsmasq,named,filterdns",$syslog_file_data);
				$syslog_file_data=preg_replace("/'dnsmasq','filterdns'/","'dnsmasq','named','filterdns'",$syslog_file_data);
				file_put_contents($syslog_file,$syslog_file_data);
				$restart_syslog++;
				}
			}
		if ($restart_syslog > 0){
			system("/usr/bin/killall -HUP syslogd");
		}
		$log_categories=explode(",",$bind['log_options']);
		$log_severity=($bind['log_severity']?$bind['log_severity']:'default');
		if (sizeof($log_categories) > 0 && $log_categories[0]!=""){
			$bind_conf .= <<<EOD
			
		logging {
			channel custom {
				syslog daemon;
				print-time no;
				print-severity yes;
				print-category yes;
				severity {$log_severity};
				};

EOD;
		foreach ($log_categories as $category)
			$bind_conf .="\t\t\tcategory $category\t{custom;};\n";
		$bind_conf .="\t\t};\n\n";				
				}
		 }
	else {
		$bind_conf .="\t\tlogging { category default { null; }; };\n\n";				
	}

	#Config Zone domain
	if(!is_array($config["installedpackages"]["bindacls"]) || !is_array($config["installedpackages"]["bindacls"]["config"])){
		$config["installedpackages"]["bindacls"]["config"][] =
			array("name"=>"none","description"=>"BIND Built-in ACL","row"=>array("value"=>"","description"=>""));
		$config["installedpackages"]["bindacls"]["config"][] =
			array("name"=>"any","description"=>"BIND Built-in ACL","row"=>array("value"=>"","description"=>""));
		$config["installedpackages"]["bindacls"]["config"][] =
			array("name"=>"localhost","description"=>"BIND Built-in ACL","row"=>array("value"=>"","description"=>""));
		$config["installedpackages"]["bindacls"]["config"][] =
			array("name"=>"localnets","description"=>"BIND Built-in ACL","row"=>array("value"=>"","description"=>""));
		write_config("Create BIND Built-in ACLs");
		}
	$bindacls = $config["installedpackages"]["bindacls"]["config"];
	for ($i=0; $i<sizeof($bindacls); $i++)
	{
		$aclname = $bindacls[$i]['name'];
		$aclhost = $bindacls[$i]['row'];
		if($aclname != "none" && $aclname != "any" && $aclname != "localhost" && $aclname != "localnets"){
			$bind_conf .= "acl \"$aclname\" {\n";	
			for ($u=0; $u<sizeof($aclhost); $u++)
			{
				$aclhostvalue = $aclhost[$u]['value'];
				$bind_conf .= "\t$aclhostvalue;\n";
			}
			$bind_conf .= "};\n\n";
		}	
	}
 
	if(is_array($config["installedpackages"]["bindviews"]))
		$bindview = $config["installedpackages"]["bindviews"]["config"];
	else
		$bindview =array();
		
	for ($i=0; $i<sizeof($bindview); $i++) 
	{
		$views = $config["installedpackages"]["bindviews"]["config"][$i];
		$viewname = $views['name'];
		$viewrecursion = $views['recursion']; 
		if($views['match-clients'] == '')
			$viewmatchclients = "none";
		else
			$viewmatchclients = str_replace(',','; ',$views['match-clients']); 	
		if($views['allow-recursion'] == '')
			$viewallowrecursion = "none";
		else
			$viewallowrecursion = str_replace(',','; ',$views['allow-recursion']); 	
		$viewcustomoptions = base64_decode($views['bind_custom_options']);
		
		$bind_conf .= "view \"$viewname\" { \n\n"; 
		$bind_conf .= "\trecursion $viewrecursion;\n"; 
		$bind_conf .= "\tmatch-clients { $viewmatchclients;};\n"; 
		$bind_conf .= "\tallow-recursion { $viewallowrecursion;};\n"; 
		$bind_conf .= "\t$viewcustomoptions\n\n"; 
			
		if(is_array($config["installedpackages"]["bindzone"])) 
			$bindzone = $config["installedpackages"]["bindzone"]["config"];
		else
			$bindzone =array();

		$write_config=0;
		for ($x=0; $x<sizeof($bindzone); $x++)
		{
			$zone = $bindzone[$x];
			if ($zone['disabled']=="on"){
				continue;
				}
			$zonename = $zone['name'];
			if ($zonename=="."){
				$custom_root_zone[$i]=true;
			}
			$zonetype = $zone['type'];
			$zoneview = $zone['view'];
			$zonecustom = base64_decode($zone['custom']);
			$zoneipslave = $zone['slaveip'];
			$zoneforwarders=$zone['forwarders'];
			$zonereverso = $zone['reverso'];
			
			if (!(is_dir(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview")))
            	mkdir(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview",0755,true);
            					
			if($zone['allowupdate'] == '')
				$zoneallowupdate = "none";
			else
				$zoneallowupdate = str_replace(',','; ',$zone['allowupdate']); 	
			if($zone['allowquery'] == '')
				$zoneallowquery = "none";
			else
				$zoneallowquery = str_replace(',','; ',$zone['allowquery']); 	
			if($zone['allowtransfer'] == '')
				$zoneallowtransfer = "none";
			else
				$zoneallowtransfer = str_replace(',','; ',$zone['allowtransfer']); 	

				if ($zoneview == $viewname){
					if($zonereverso == "on")
				 		$bind_conf .= "\tzone \"$zonename.in-addr.arpa\" {\n";
				  	else
						$bind_conf .= "\tzone \"$zonename\" {\n";
        		
					$bind_conf .= "\t\ttype $zonetype;\n";
					if ($zonetype != "forward")
        				$bind_conf .= "\t\tfile \"/etc/namedb/$zonetype/$zoneview/$zonename.DB\";\n";
        			switch ($zonetype){
        				case "slave":
        				$bind_conf .= "\t\tmasters { $zoneipslave; };\n";
        				$bind_conf .= "\t\tallow-transfer {none;};\n";
        				$bind_conf .= "\t\tnotify no;\n";
        				break;
        				case "forward":
        				$bind_conf .= "\t\tforward only;\n";
        				$bind_conf .= "\t\tforwarders { $zoneforwarders; };\n";
        				break;
        				case "redirect":
							$bind_conf .= "\t\t# While using redirect zones,NXDOMAIN Redirection will not override DNSSEC\n";
							$bind_conf .= "\t\t# If the client has requested DNSSEC records (DO=1) and the NXDOMAIN response is signed then no substitution will occur\n";
							$bind_conf .= "\t\t# https://kb.isc.org/article/AA-00376/192/BIND-9.9-redirect-zones-for-NXDOMAIN-redirection.html\n";
        				break;
        				default:
						$bind_conf .= "\t\tallow-update { $zoneallowupdate;};\n"; 
						$bind_conf .= "\t\tallow-query { $zoneallowquery;};\n"; 
						$bind_conf .= "\t\tallow-transfer { $zoneallowtransfer;};\n";
	        			if ($zone['dnssec']=="on"){
	        				//https://kb.isc.org/article/AA-00626/
	        				$bind_conf .="\n\t\t# look for dnssec keys here:\n";
	        				$bind_conf .="\t\tkey-directory \"/etc/namedb/keys\";\n\n";
	        				$bind_conf .="\t\t# publish and activate dnssec keys:\n";
	        				$bind_conf .="\t\tauto-dnssec maintain;\n\n";
	        				$bind_conf .="\t\t# use inline signing:\n";
	        				$bind_conf .="\t\tinline-signing yes;\n\n";
	        				}
	        			}
				 	if ($zonecustom != '')
        				$bind_conf .= "\t\t$zonecustom\n";
        				
					$bind_conf .= "\t};\n\n";
        
					switch($zonetype){
					  case "redirect":
					  case "master":
					  	//check/update slave dir permission
						chown(CHROOT_LOCALBASE."/etc/namedb/$zonetype","bind");
						chown(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview","bind");
						$zonetll = ($zone['tll']?$zone['tll']:"43200");
						$zonemail = ($zone['mail']?$zone['mail']:"zonemaster.{$zonename}");
						$zonemail = preg_replace("/@/",".",$zonemail);				
						$zoneserial = $zone['serial'];
						$zonerefresh = ($zone['refresh']?$zone['refresh']:"3600");		
						$zoneretry = ($zone['retry']?$zone['retry']:"600");
						$zoneexpire = ($zone['expire']?$zone['expire']:"86400");
						$zoneminimum = ($zone['minimum']?$zone['minimum']:"3600");
						$zonenameserver = $zone['nameserver'];
						$zoneipns = $zone['ipns'];
						$zonereverso = $zone['reverso'];
						if($zone['allowupdate'] == '')
							$zoneallowupdate = "none";
						else
							$zoneallowupdate = str_replace(',','; ',$zone['allowupdate']);
						if($zone['allowquery'] == '')
							$zoneallowquery = "none";
						else
							$zoneallowquery = str_replace(',','; ',$zone['allowquery']);
						if($zone['allowtransfer'] == '')
							$zoneallowtransfer = "none";
						else
							$zoneallowtransfer = str_replace(',','; ',$zone['allowtransfer']);
						$zone_conf = "\$TTL {$zonetll}\n;\n";
						if($zonereverso == "on")
							$zone_conf .= "\$ORIGIN {$zonename}.in-addr.arpa.\n\n";
						else
							$zone_conf .= "\$ORIGIN {$zonename}.\n\n";
						$zone_conf .= ";\tDatabase file {$zonename}.DB for {$zonename} zone.\n";
						$zone_conf .= ";\tDo not edit this file!!!\n";
						$zone_conf .= ";\tZone version {$zoneserial}\n;\n";
					 	if($zonereverso == "on" || $zonetype =="redirect")
							$zone_conf .= "@\t IN  SOA $zonenameserver. \t $zonemail. (\n";
						else
							$zone_conf .= "$zonename.\t IN  SOA $zonenameserver. \t $zonemail. (\n";

						$zone_conf .= "\t\t$zoneserial ; serial\n";
						$zone_conf .= "\t\t$zonerefresh ; refresh\n";
						$zone_conf .= "\t\t$zoneretry ; retry\n";
						$zone_conf .= "\t\t$zoneexpire ; expire\n";
						$zone_conf .= "\t\t$zoneminimum ; default_ttl\n\t\t)\n\n";
						$zone_conf .= ";\n; Zone Records\n;\n";

					 	if($zonereverso == "on")
							$zone_conf .= "\t IN NS \t$zonenameserver.\n";
						else{
							$zone_conf .= "@ \t IN NS \t$zonenameserver.\n";
							if ($zoneipns !="")
								$zone_conf .= "@ \t IN A \t$zoneipns\n";
						}
						for ($y=0; $y<sizeof($zone['row']); $y++)
						{
							$hostname = (preg_match("/(MX|NS)/",$zone['row'][$y]['hosttype'])?"@":$zone['row'][$y]['hostname']);
							$hosttype = $zone['row'][$y]['hosttype'];
							$hostdst = $zone['row'][$y]['hostdst'];
							if (preg_match("/[a-zA-Z]/",$hostdst) && !preg_match("/(TXT|SPF|AAAA)/",$hosttype))
								$hostdst .= ".";
							$hostvalue = $zone['row'][$y]['hostvalue'];
							
							$zone_conf .= "$hostname \t IN $hosttype $hostvalue \t$hostdst\n";
						}

						# Register DHCP static mappings
						if (($zone[regdhcpstatic] == 'on') && is_array($config['dhcpd'])) {
							$zoneparts = array_reverse(explode('.',$zonename));
							foreach ($config['dhcpd'] as $dhcpif => $dhcpifconf) {
								if (!isset($dhcpifconf['enable']) || !is_array($dhcpifconf['staticmap']))  {
									continue;
								}
								foreach ($dhcpifconf['staticmap'] as $host) {
									if (is_domain($host['domain'])) {
										$domain = $host['domain'];
									} elseif (is_domain($dhcpifconf['domain'])) {
										$domain = $dhcpifconf['domain'];
									} elseif (is_domain($config['system']['domain'])) {
										$domain = $config['system']['domain'];
									} else {
										continue;
									}
									if (!is_hostname($host['hostname']) || !is_ipaddr($host['ipaddr']))  {
										continue;
									}
									if ($zonereverso == "on") {
										$parts = explode('.',$host['ipaddr']);
										$intersect = array_intersect_assoc($parts,$zoneparts);
										if (count($zoneparts) == count($intersect)) {
											$diff = array_diff_assoc($parts,$zoneparts);
											$shortaddr = implode('.',array_reverse($diff));
											$zone_conf .= "{$shortaddr}\tIN PTR\t{$host['hostname']}.{$domain}.\n";
										}
									} else {
										$parts = array_reverse(explode('.',$domain));
										$diff = array_diff_assoc($parts,$zoneparts);
										if (count($diff) == 0) {
											$zone_conf .= "{$host['hostname']}\tIN A\t{$host['ipaddr']}\n";
										}
									}
								}
							}
						}

						if ($zone['customzonerecords']!=""){
							$zone_conf .= "\n\n;\n;custom zone records\n;\n".base64_decode($zone['customzonerecords'])."\n";
						}
						file_put_contents(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview/$zonename.DB", $zone_conf);
						$config["installedpackages"]["bindzone"]["config"][$x][resultconfig]=base64_encode($zone_conf);
						$write_config++;
						//check dnssec keys creation for master zones
        				if($zone['dnssec']=="on"){
	        				$zone_found=0;
	        				foreach (glob(CHROOT_LOCALBASE."/etc/namedb/keys/*{$zonename}*key",GLOB_NOSORT) as $filename){
								$zone_found++;
	        					}
							if ($zone_found==0){
								$key_restored=0;
								if(is_array($config['installedpackages']['dnsseckeys']) && is_array($config['installedpackages']['dnsseckeys']['config'])){
									foreach ($config['installedpackages']['dnsseckeys']['config']as $filer)
										if (preg_match ("/K$zonename\.+/",$filer['fullfile'])){
											file_put_contents($filer['fullfile'],base64_decode($filer['filedata']),LOCK_EX);
											chmod($filer['fullfile'],0700);
											chown($filer['fullfile'],"bind");
											$key_restored++;
											}
										}
								if ($key_restored > 0){
									log_error("[bind] {$key_restored} DNSSEC keys restored from XML backup for {$zonename} zone.");
									}
								$dnssec_bin="/usr/local/sbin/dnssec-keygen";
								if (file_exists($dnssec_bin) && $key_restored==0){
									exec("{$dnssec_bin} -K ".CHROOT_LOCALBASE."/etc/namedb/keys {$zonename}",$kout);
									exec("{$dnssec_bin} -K ".CHROOT_LOCALBASE."/etc/namedb/keys -fk {$zonename}",$kout);
									foreach($kout as $filename){
										chown(CHROOT_LOCALBASE."/etc/namedb/keys/{$filename}.key","bind");
										chown(CHROOT_LOCALBASE."/etc/namedb/keys/{$filename}.private","bind");
										}
									log_error("[bind] DNSSEC keys for {$zonename} created.");
									}
	        					}
	        				//get ds keys
	        				$dsfromkey="/usr/local/sbin/dnssec-dsfromkey";
	        				foreach (glob(CHROOT_LOCALBASE."/etc/namedb/keys/*{$zonename}*key",GLOB_NOSORT) as $filename) {
	        						$zone_key=file_get_contents($filename);
	        						if (preg_match("/IN DNSKEY 257 /",$zone_key) && file_exists($dsfromkey)){
	        							exec("$dsfromkey $filename",$dsset);
	        							$config["installedpackages"]["bindzone"]["config"][$x]['dsset']=base64_encode(array_pop($dsset)."\n".array_pop($dsset));
	        							$write_config++;
	        							}
								}
							//save dnssec keys to xml
							
							if($zone['backupkeys']=="on"){
								$dnssec_keys=0;
								foreach (glob(CHROOT_LOCALBASE."/etc/namedb/keys/*{$zonename}*",GLOB_NOSORT) as $filename){
									$file_found=0;
									if(is_array($config['installedpackages']['dnsseckeys']) && is_array($config['installedpackages']['dnsseckeys']['config'])){
										foreach ($config['installedpackages']['dnsseckeys']['config']as $filer){
											if ($filer['fullfile']==$filename)
												$file_found++;
											}
										}
									if ($file_found==0){
										$config['installedpackages']['dnsseckeys']['config'][]=array('fullfile'=> $filename,
																								'description'=> "bind {$zonename} DNSSEC backup file",
																								'filedata'=> base64_encode(file_get_contents($filename)));
										$write_config++;
										$dnssec_keys++;
										}
									}
									if($dnssec_keys>0){
										log_error("[bind] {$dnssec_keys} DNSSEC keys for {$zonename} zone saved on XML config.");
									}
								}
        					}
					break;
					case "slave":
					//check/update slave dir permission
					chown(CHROOT_LOCALBASE."/etc/namedb/$zonetype","bind");
					chown(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview","bind");
					//check if exists slave zone file
					$rsconfig="";
					if ($zone['dnssec']=="on"){
						if (file_exists(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview/$zonename.DB.signed"))
						exec("/usr/local/sbin/named-checkzone -D -f raw -o - {$zonename} ".CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview/$zonename.DB.signed",$slave_file);
						}
					else{
						if (file_exists(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview/$zonename.DB"))
							$slave_file=file(CHROOT_LOCALBASE."/etc/namedb/$zonetype/$zoneview/$zonename.DB");
						}
					if (is_array($slave_file)){
					foreach ($slave_file as $zfile)
						$rsconfig.= $zfile;
					$config["installedpackages"]["bindzone"]["config"][$x][resultconfig]=base64_encode($rsconfig);
					$write_config++;
					}
					break;
					}
				}
		}
		if (!$custom_root_zone[$i]){
			$bind_conf .="\tzone \".\" {\n";
			$bind_conf .="\t\ttype hint;\n";
			$bind_conf .="\t\tfile \"/etc/namedb/named.root\";\n";
			$bind_conf .= "\t};\n\n";
			}
		if($write_config > 0){
			write_config("save result config file for zone on xml");
		}
		$bind_conf .= "};\n";
	}
	$dirs=array("/etc/namedb/keys","/var/run/named","/var/dump","/var/log","/var/stats","/dev");
	foreach ($dirs as $dir){
		if (!is_dir(CHROOT_LOCALBASE .$dir))
			mkdir(CHROOT_LOCALBASE .$dir,0755,true);
		}
	//dev dirs for chroot
	$bind_dev_dir=CHROOT_LOCALBASE."/dev";
	if (!file_exists("$bind_dev_dir/random")){
		$dev_dirs=array("null","zero","random","urandom");
		exec("/sbin/mount -t devfs devfs {$bind_dev_dir}",$dout);
		exec("/sbin/devfs -m {$bind_dev_dir} ruleset 1",$dout);
		exec("/sbin/devfs -m {$bind_dev_dir} rule add hide",$dout);
		foreach ($dev_dirs as $dev_dir)
			exec("/sbin/devfs -m {$bind_dev_dir} rule add path $dev_dir unhide",$dout);
		exec("/sbin/devfs -m {$bind_dev_dir} rule applyset",$dout);
	}
	//http://www.unixwiz.net/techtips/bind9-chroot.html
    file_put_contents(CHROOT_LOCALBASE.'/etc/namedb/named.conf', $bind_conf);
    file_put_contents(CHROOT_LOCALBASE.'/etc/namedb/rndc.conf', $rndc_file);
    
	if (!file_exists(CHROOT_LOCALBASE."/etc/namedb/named.root")){
		//dig +tcp @a.root-servers.net > CHROOT_LOCALBASE."/etc/namedb/named.root"
		$named_root=file_get_contents("http://www.internic.net/domain/named.root");
		file_put_contents(CHROOT_LOCALBASE."/etc/namedb/named.root",$named_root,LOCK_EX);
	}
	if (!file_exists(CHROOT_LOCALBASE."/etc/localtime")){
		copy("/etc/localtime", CHROOT_LOCALBASE."/etc/localtime");
	}
	
	bind_write_rcfile();
	chown(CHROOT_LOCALBASE."/etc/namedb/keys","bind");
	chown(CHROOT_LOCALBASE."/etc/namedb","bind");
	chown(CHROOT_LOCALBASE."/var/log","bind");
	chown(CHROOT_LOCALBASE."/var/run/named","bind");
	chgrp(CHROOT_LOCALBASE."/var/log","bind");
	$bind_sh="/usr/local/etc/rc.d/named.sh";
 	if($bind_enable == "on"){
 		chmod ($bind_sh,0755);
 		mwexec("{$bind_sh} restart");
 		}
 	elseif (is_service_running('named')){
 		mwexec("{$bind_sh} stop");
		chmod ($bind_sh,0644); 		
 		}
 	//sync to backup servers
 	bind_sync_on_changes();
 	conf_mount_ro();
}

function bind_print_javascript_type_zone(){
?>
        <script language="JavaScript">
        <!--
        function on_type_zone_changed() {

		var field = document.iform.type;
       		var tipo = field.options[field.selectedIndex].value;
       			switch (tipo){
       			case 'master':
					document.iform.slaveip.disabled = 1;
					document.iform.tll.disabled = 0;
					document.iform.nameserver.disabled = 0;
					document.iform.reverso.disabled = 0;
					document.iform.forwarders.disabled = 1;
					document.iform.dnssec.disabled = 0;
					document.iform.backupkeys.disabled = 0;
					document.iform.regdhcpstatic.disabled = 0;
					document.iform.ipns.disabled = 0;
					document.iform.mail.disabled = 0;
					document.iform.serial.disabled = 0;
					document.iform.refresh.disabled = 0;
					document.iform.retry.disabled = 0;
					document.iform.expire.disabled = 0;
					document.iform.minimum.disabled = 0;
               	break;
               	case 'slave':
					document.iform.slaveip.disabled = 0;
					document.iform.tll.disabled = 1;
					document.iform.nameserver.disabled = 1;
					document.iform.reverso.disabled = 0;
					document.iform.forwarders.disabled = 1;
					document.iform.dnssec.disabled = 0;
					document.iform.backupkeys.disabled = 0;
					document.iform.regdhcpstatic.disabled = 0;
					document.iform.ipns.disabled = 1;
					document.iform.mail.disabled = 1;
					document.iform.serial.disabled = 1;
					document.iform.refresh.disabled = 1;
					document.iform.retry.disabled = 1;
					document.iform.expire.disabled = 1;
					document.iform.minimum.disabled = 1;
					break;
               	case 'forward':
					document.iform.slaveip.disabled = 1;
					document.iform.tll.disabled = 1;
					document.iform.nameserver.disabled = 1;
					document.iform.reverso.disabled = 1;
					document.iform.forwarders.disabled = 0;
					document.iform.dnssec.disabled = 1;
					document.iform.backupkeys.disabled = 1;
					document.iform.regdhcpstatic.disabled = 1;
					document.iform.ipns.disabled = 1;
					document.iform.mail.disabled = 1;
					document.iform.serial.disabled = 1;
					document.iform.refresh.disabled = 1;
					document.iform.retry.disabled = 1;
					document.iform.expire.disabled = 1;
					document.iform.minimum.disabled = 1;
				break;
               	case 'redirect':
					document.iform.slaveip.disabled = 1;
					document.iform.tll.disabled = 1;
					document.iform.nameserver.disabled = 0;
					document.iform.reverso.disabled = 1;
					document.iform.forwarders.disabled = 1;
					document.iform.dnssec.disabled = 1;
					document.iform.backupkeys.disabled = 1;
					document.iform.regdhcpstatic.disabled = 1;
					document.iform.ipns.disabled = 1;
					document.iform.mail.disabled = 0;
					document.iform.serial.disabled = 0;
					document.iform.refresh.disabled = 0;
					document.iform.retry.disabled = 0;
					document.iform.expire.disabled = 0;
					document.iform.minimum.disabled = 0;
				break;
       			}
        }
        -->
        </script>
<?php
}

function bind_print_javascript_type_zone2(){
        print("<script language=\"JavaScript\">on_type_zone_changed();document.iform.resultconfig.disabled = 1;document.iform.dsset.disabled = 1;</script>\n");
}

function bind_write_rcfile() {
        $rc = array();
        $BIND_LOCALBASE = "/usr/local";
        $rc['file'] = 'named.sh';
        $rc['start'] = <<<EOD
if [ -z "`ps auxw | grep "[n]amed -c /etc/namedb/named.conf"|awk '{print $2}'`" ];then
        {$BIND_LOCALBASE}/sbin/named -c /etc/namedb/named.conf -u bind -t /cf/named/
fi

EOD;
        $rc['stop'] = <<<EOD
killall -9 named 2>/dev/null
sleep 2
EOD;
        $rc['restart'] = <<<EOD
if [ -z "`ps auxw | grep "[n]amed -c /etc/namedb/named.conf"|awk '{print $2}'`" ];then
        	{$BIND_LOCALBASE}/sbin/named -c /etc/namedb/named.conf -u bind -t /cf/named/
        else
		killall -9 named 2>/dev/null
         	sleep 3	
        	{$BIND_LOCALBASE}/sbin/named -c /etc/namedb/named.conf -u bind -t /cf/named/
        fi

EOD;
        conf_mount_rw();
        write_rcfile($rc);
        conf_mount_ro();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function bind_sync_on_changes() {
	global $config, $g;
	if (is_array($config['installedpackages']['bindsync']['config'])){
		$bind_sync=$config['installedpackages']['bindsync']['config'][0];
		$synconchanges = $bind_sync['synconchanges'];
		$synctimeout = $bind_sync['synctimeout'];
		$master_zone_ip=$bind_sync['masterip'];
		switch ($synconchanges){
			case "manual":
				if (is_array($bind_sync[row])){
					$rs=$bind_sync[row];
					}
				else{
					log_error("[bind] xmlrpc sync is enabled but there is no hosts to push on bind config.");
					return;
					}
				break;
			case "auto":
					if (is_array($config['hasync'])){
						$hasync=$config['hasync'][0];
						$rs[0]['ipaddress']=$hasync['synchronizetoip'];
						$rs[0]['username']=$hasync['username'];
						$rs[0]['password']=$hasync['password'];
					}
					else{
						log_error("[bind] xmlrpc sync is enabled but there is no system backup hosts to push bind config.");
						return;
					}
				break;			
			default:
				return;
			break;
		}
		if (is_array($rs)){
			log_error("[bind] xmlrpc sync is starting.");
			foreach($rs as $sh){
				$sync_to_ip = $sh['ipaddress'];
				$password = $sh['password'];
				if($sh['username'])
					$username = $sh['username'];
				else
					$username = 'admin';
				if($password && $sync_to_ip)
					bind_do_xmlrpc_sync($sync_to_ip, $username, $password,$synctimeout,$master_zone_ip);
				}
			log_error("[bind] xmlrpc sync is ending.");
			}
 		}
}
/* Do the actual XMLRPC sync */
function bind_do_xmlrpc_sync($sync_to_ip, $username, $password, $synctimeout,$master_zone_ip) {
	global $config, $g;

	if(!$username)
		return;
		
	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

	if(!$synctimeout)
		$synctimeout=25;
		
		
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
	$xml['bind'] = $config['installedpackages']['bind'];
	$xml['bindacls'] = $config['installedpackages']['bindacls'];
	$xml['bindviews'] = $config['installedpackages']['bindviews'];
	$xml['bindzone'] = $config['installedpackages']['bindzone'];
	if (is_array($config['installedpackages']['dnsseckeys']))
		$xml['dnsseckeys']=$config['installedpackages']['dnsseckeys'];
	//change master zone to slave on backup servers
	if(is_array($xml['bindzone']["config"])) 
		for ($x=0; $x<sizeof($xml['bindzone']["config"]); $x++){
			if ($xml['bindzone']["config"][$x]['type']=="master"){
				$xml['bindzone']["config"][$x]['type']="slave";
				$xml['bindzone']["config"][$x]['slaveip']=$master_zone_ip;
			}
			
		}
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("[bind] Beginning bind XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after defined sync timeout value*/
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting bind XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "bind Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting bind XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "bind Settings Sync", "");
	} else {
		log_error("[bind] XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell bind to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/bind.inc');\n";
	$execcmd .= "bind_sync('yes');";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("[bind] XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting bind XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "Bind Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "[Bind] An error code was received while attempting bind XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "bind Settings Sync", "");
	} else {
		log_error("Bind XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
	
}
?>
