<?php
/* ========================================================================== */
/*
	varnish.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
    Copyright (C) 2011-2013 Marcello Coutinho
    Copyright (C) 2012 Marcio Carlos Antao
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
$shortcut_section = "varnish";

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('VARNISH_LOCALBASE', '/usr/pbi/varnish-' . php_uname("m"));
else
  define('VARNISH_LOCALBASE','/usr/local');


function varnish_settings_post_validate($post, $input_errors) {
	if( !is_numeric($post['storagesize'])) 
		$input_errors[] = "A valid number is required for the field 'Storage size'";	
	if($post['listeningport'] && !is_numeric($post['listeningport'])) 
		$input_errors[] = "A valid number is required for the field 'Listening port'";	
	if($post['minworkers'] && !is_numeric($post['minworkers'])) 
		$input_errors[] = "A valid number is required for the field 'Minimum worker threads'";	
	if($post['maxworkers'] && !is_numeric($post['maxworkers'])) 
		$input_errors[] = "A valid number is required for the field 'Maximum worker threads'";	
	if($post['timeoutworkers'] && !is_numeric($post['timeoutworkers'])) 
		$input_errors[] = "A valid number is required for the field 'Worker thread timeout'";
	if($post['managment']){
		$mgm= explode(":",$post['managment']);
		if(!is_ipaddr($mgm[0]) || !is_numeric($mgm[1]))
		$input_errors[] = "A valid ip:port is required for the field 'managment'";
		}
	if($post['grace'] &&  ! preg_match("/^\d+(h|m|s)$/",$post['grace']))
		$input_errors[] = "A valid number with a time reference is required for the field 'Fetch grace'";
	if($post['saint'] &&  ! preg_match("/^\d+(h|m|s)$/",$post['saint']))
		$input_errors[] = "A valid number with a time reference is required for the field 'Saint mode'";
		
}

function varnish_lb_directors_post_validate($post, $input_errors) {
	if (preg_match("/[^a-zA-Z0-9]/", $post['directorname'])){
		$input_errors[] = "The directorname name must only contain the characters a-Z or 0-9";
	}
	else{
		if(empty($post['failover']))
			$_POST['failover'] = $post['directorname'];
	}
	if(stristr($post['directorurl'], 'http'))
		$input_errors[] = "You do not need to include the http:// string in the director URL";
	if($post['grace'] && ! preg_match("/^\d+(h|m|s)$/",$post['grace']))
		$input_errors[] = "A valid number with a time reference is required for the field 'Req grace'";
}

function varnish_backends_post_validate($post, $input_errors) {
 	if (!$post['backendname'] || preg_match("/[^a-zA-Z0-9]/", $post['backendname']))
		$input_errors[] = "The backend name must only contain the characters a-Z or 0-9";
	if(!is_ipaddr($post['ipaddress'])) 
		$input_errors[] = "A valid IP address is required for the field 'IPAddress'";
	if($post['first_byte_timeout'] && !is_numeric($post['first_byte_timeout'])) 
		$input_errors[] = "A valid number is required for the field 'first byte timeout'";
	if($post['connect_timeout'] && !is_numeric($post['connect_timeout'])) 
		$input_errors[] = "A valid number is required for the field 'connect timeout'";
	if($post['probe_interval'] && !is_numeric($post['probe_interval'])) 
		$input_errors[] = "A valid number is required for the field 'probe interval'";
	if($post['probe_interval'] && !is_numeric($post['probe_interval'])) 
		$input_errors[] = "A valid number is required for the field 'probe interval'";
	if($post['probe_timeout'] && !is_numeric($post['probe_timeout'])) 
		$input_errors[] = "A valid number is required for the field 'probe timeout'";
	if($post['probe_window'] && !is_numeric($post['probe_window'])) 
		$input_errors[] = "A valid number is required for the field 'probe window'";
	if($post['probe_threshold'] && !is_numeric($post['probe_threshold'])) 
		$input_errors[] = "A valid number is required for the field 'probe threshold'";
	$x=0;
	while ($post['maptype'.$x] != ""){
	if($post['grace'.$x] && ! preg_match("/^\d+(h|m|s)$/",$post['grace'.$x])){
		$input_errors[] = "A valid number with a time reference is required for the field 'grace' in map ".($x +1);
		}
		$x++;
		
}

}

function varnish_install() {
	create_varnish_rcd_file();
}

function varnish_deinstall() {
	create_varnish_rcd_file();
}

function text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}
function varnish_start() {
	global $g, $config;
	if ($config['installedpackages']['varnishsettings']['config'][0]['enablevarnish']){
		exec("chmod +x /usr/local/etc/rc.d/varnish.sh");
		mwexec("/usr/local/etc/rc.d/varnish.sh");}
	else{
		exec("chmod -x /usr/local/etc/rc.d/varnish.sh");
		mwexec("/usr/bin/killall varnishd");}
}

/* Build the URL mappings logic VCL config txt */
function varnish_get_url_mappings_txt() {
	global $g, $config, $urlmappings,$backends_in_use;
	$catch_all = "unset";
	$isfirst = true;
	if ($config['installedpackages']['varnishlbdirectors']['config'] != "") {
		foreach($config['installedpackages']['varnishlbdirectors']['config'] as $url) {
			// check options
			$directo_grace_time = "";
			if ($url['customapping'])
				$directo_grace_time .= text_area_decode($url['customapping'])."\n\t\t";
			if ($url['grace'])
				$directo_grace_time .= ($url['grace'] == "0s" ? "return(pass);" : "set req.grace={$url['grace']};");
			$fieldtype = ($url['fieldtype'] ? $url['fieldtype'] : "==");
			$director_prefix = ($url['directorurl'] && $url['directorurl2'] ? "^http://" : "");
			// check url
			if ($url['directorurl'] || $url['directorurl2'] || $catch_all == "unset") {
				if ($url['directorurl'] == "" && $url['directorurl2'] == "") {
					// director with no host or url, so director for catch all traffic not specified in config
					$lasturlmappings = "\telse\t{\n\t\tset req.backend = {$url['directorname']};\n\t\t}\n";
					$catch_all = "set";
					$isfirst = false;
				} else {
					if (!$isfirst)
						$urlmappings .= "\telse ";
					if (!$url['directorurl'])
						$urlmappings .= "if (req.url $fieldtype ".'"^'.$url['directorurl2'].'") {'."\n";
					else if (!$url['directorurl2'])
						$urlmappings .= "if (req.http.host $fieldtype ".'"'.$url['directorurl'].'") {'."\n";
					else
				        $urlmappings .= "if (req.http.host $fieldtype ".'"'.$url['directorurl'].'"'." && req.url $fieldtype ".'"^'.$url['directorurl2'].'") {'."\n";
				
					$urlbackend = "\t\t\tset req.backend = ".$url['directorname'].";";
					// check rewrite options
					if ($url['rewritehost'])
						$urlmappings .= "\t\t\tset req.http.host = regsub(req.http.host, ".'"'.$url['directorurl'].'",'.'"'.$url['rewritehost'].'")'.";\n";
					if ($url['rewriteurl'])
						$urlmappings .= "\t\t\tset req.url = regsub(req.url, ".'"'.$url['directorurl2'].'",'.'"^'.$url['rewriteurl'].'")'.";\n";
					// check failover
					if ($url['failover'] && $url['failover'] != $url['directorname']) {
						$tabs = ($url['grace'] ? "\n\t\t\t" : "");
						$urlfailover = "\t\t\tset req.backend = ".$url['failover'].";";
						$urlmappings .= "\t\tif (req.restarts == 0) {\n".$urlbackend.$tabs.$directo_grace_time.$tabs."}";
						$urlmappings .= "\n\t\telse\t{\n".$urlfailover.$tabs.$directo_grace_time.$tabs."}\n\t\t}\n";
						$isfirst = false;
					} else {
						$tabs = ($url['grace'] ? "\n\t\t" : "");
						$urlmappings .= $urlbackend.$tabs.$directo_grace_time."\n\t\t}\n";
						$isfirst = false;
					}
				}
  			}
  		}
	}
	if ($config['installedpackages']['varnishbackends']['config']) {
		foreach($config['installedpackages']['varnishbackends']['config'] as $urlmapping) {
			if (isset($urlmapping['row'])) {
				foreach($urlmapping['row'] as $url) {
					$directo_grace_time = "";
					if ($url['grace'])
						$directo_grace_time = ($url['grace'] == "0s" ? "\n\t\t return(pass);" : "\n\t\tset req.grace={$url['grace']};");
					$req = ($url['maptype'] ? $url['maptype'] : "http.host");
					$fieldtype = ($url['fieldtype'] ? $url['fieldtype'] : "==");
					if ($url['urlmapping'] != "" || $catch_all == "unset") {
						if ($url['urlmapping'] == "") {
							$catch_all = "set";
							$lasturlmappings .= "set req.backend = {$urlmapping['backendname']}BACKEND;\n";
						} else {
							if (!$isfirst)
								$urlmappings .= "\telse ";
							$urlmappings .= <<<EOAU
if (req.{$req} {$fieldtype} "{$url['urlmapping']}") {
		set req.backend = {$urlmapping['backendname']}BACKEND;{$directo_grace_time}
	} 

EOAU;
							$isfirst = false;
						}
						$backends_in_use[$urlmapping['backendname']] .= ($url['directorurl'] == "" ? "catch_all " : "url_map ");
					}
				}
			}
		}
		if ($urlmappings != "")
			$lasturlmappings = "\telse {\n\t\t$lasturlmappings\t}\n";
		return $urlmappings.$lasturlmappings;
	}
}

function create_varnish_rcd_file() {
	global $config, $g;
	if($config['installedpackages']['varnishsettings']['config'] != "") {
		foreach($config['installedpackages']['varnishsettings']['config'] as $vs) {
			if($vs['storagetype'] == "malloc")
				$storage_type = "-s malloc,{$vs['storagesize']}MB";
			else 
				$storage_type = "-s file,/var/varnish/storage.bin,{$vs['storagesize']}MB";
			if($vs['listeningport'])
				$listeningport = "-a :{$vs['listeningport']}";
			else 
				$listeningport = "-a :80";
			if($vs['managment'])
				$advancedstartup = "-T {$vs['managment']} ";
			else 
				$advancedstartup = "";
			if($vs['advancedstartup'])
				$advancedstartup .= text_area_decode($vs['advancedstartup'])."\n";
			if($vs['minworkers'])
				$minworkers = "{$vs['minworkers']}";
			else 
				$minworkers = "200";
			if($vs['maxworkers'])
				$maxworkers = "{$vs['maxworkers']}";
			else 
				$maxworkers = "4000";
			if($vs['timeoutworkers'])
				$timeoutworkers = "{$vs['timeoutworkers']}";
			else 
				$timeoutworkers = "50";				
		}
	}
	$fd = fopen("/usr/local/etc/rc.d/varnish.sh", "w");
	$rc_file = <<<EOF
#!/bin/sh
mkdir -p /var/varnish
rm /var/varnish/storage.bin 2>/dev/null
killall varnishd 2>/dev/null
sleep 1
sysctl kern.ipc.somaxconn=16384
sysctl kern.maxfiles=131072
sysctl kern.maxfilesperproc=104856
sysctl kern.threads.max_threads_per_proc=4096
/usr/bin/env \
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
/usr/local/bin/php -q -d auto_prepend_file=config.inc <<ENDOFF
<?php
	require_once("globals.inc");		
	require_once("functions.inc");
	require_once("pkg-utils.inc");
	require_once("varnish.inc");
	sync_package_varnish();
	#varnish_start();

?>
ENDOFF

/usr/local/sbin/varnishd \
	{$listeningport} \
	-f /var/etc/default.vcl \
	{$storage_type} \
	-w {$minworkers},{$maxworkers},{$timeoutworkers} \
	{$advancedstartup} 

EOF;

	fwrite($fd, $rc_file);
	fclose($fd);
	exec("chmod a+rx /usr/local/etc/rc.d/varnish.sh");
}

function get_backend_config_txt() {
	global $config, $g, $backends_in_use;
	$backends = "";
	if ($config['installedpackages']['varnishbackends']['config'] != "") {
    	foreach($config['installedpackages']['varnishbackends']['config'] as $backend) {
			if ($backend['connect_timeout'])
				$connect_timeout = $backend['connect_timeout'] . "s";
			else 
				$connect_timeout = "25s";
			if ($backend['port'])
				$connect_port = $backend['port'];
			else 
				$connect_port = "80";
			if ($backend['first_byte_timeout'])
				$first_byte_timeout = $backend['first_byte_timeout'] . "s";
			else 
				$first_byte_timeout = "300s";
			if ($backend['probe_url'])
				if (preg_match("@^(http)://([a-zA-Z0-9.-]*)/(.*)$@",$backend['probe_url'],$matches)){
					$probe_url = ".request =\n";
					$probe_url .= "\t\t\t".'"GET /'.$matches[3].' HTTP/1.1"'."\n";
                                        $probe_url.="\t\t\t".'"Accept: text/*"'."\n";
                                        $probe_url.="\t\t\t".'"User-Agent: Varnish"'."\n";
					$probe_url .= "\t\t\t".'"Host: '.$matches[2].'"'."\n";
					$probe_url .= "\t\t\t".'"Connection: Close";';
				} else
					$probe_url = '.url = "'.$backend['probe_url'].'";';
			else
				$probe_url = '.url = "/";';
			if ($backend['probe_interval'])
				$probe_interval = $backend['probe_interval'] . "s";
			else
				$probe_interval = "1s";
			if ($backend['probe_timeout'])
				$probe_timeout = $backend['probe_timeout'] . "s";
			else
				$probe_timeout = "1s";
			if ($backend['probe_window'])
				$probe_window = $backend['probe_window'];
			else
				$probe_window = "5";
			if ($backend['probe_threshold'])
				$probe_threshold = $backend['probe_threshold'];
			else
				$probe_threshold = "5";

			if ($backend['probe_disable'])
				$probe = "";
			else {
				$probe = <<<EOFPROBE
	.probe = {
		 {$probe_url}
		.interval = {$probe_interval};
		.timeout = {$probe_timeout};
		.window = {$probe_window};
		.threshold = {$probe_threshold};
	}
EOFPROBE;
			}

			if (isset($probe_threshold)) {
				// last parameter set ,so write conf if backend is in use
				if ($backends_in_use[$backend['backendname']] != "") {
					$backends .= <<<EOFA

backend {$backend['backendname']}BACKEND {
	# used in {$backends_in_use[$backend['backendname']]}
	.host = "{$backend['ipaddress']}";
	.port = "{$backend['port']}";
	.first_byte_timeout = {$first_byte_timeout};
	.connect_timeout = {$connect_timeout};
{$probe}
}


EOFA;
				} else
				 $backends .= "\n# backend {$backend['backendname']} not in use.\n";
			}
    	}
	}
	return $backends;
}

function get_lb_directors_config_txt() {
	global $config, $g, $backends_in_use;
	$backends="";
	if($config['installedpackages']['varnishlbdirectors']['config'] != "") {
    	foreach($config['installedpackages']['varnishlbdirectors']['config'] as $backend) {
			$director = "";
			if($backend['directortype'] == "round-robin" ){$weight=" ";}
			if($backend['row']){
				foreach($backend['row'] as $be) {
					if($be['weight'] && $weight != " ")
						$weight = "\t\t.weight = {$be['weight']};\n";
					elseif($weight != " ")
						$weight = "\t\t.weight = 100;\n";
				
			$director .=  "\t{\n\t\t.backend = {$be['backendname']}BACKEND;\n{$weight}\t}";
			$backends_in_use[$be['backendname']].= $backend['directorname']." ";
			}
			$backends .= <<<EOFA
director {$backend['directorname']} {$backend['directortype']} {
{$director}
}


EOFA;
				
			}
		}
	}
	return $backends;
}

function sync_package_varnish() {
	global $config, $g;
	if (is_array($config['installedpackages']['varnishcustomvcl']['config'])) {
		foreach($config['installedpackages']['varnishcustomvcl']['config'] as $vcl) {
			if ($vcl['vcl_recv_early'])
				$vcl_recv_early = text_area_decode($vcl['vcl_recv_early']);
			if ($vcl['vcl_recv_late'])
				$vcl_recv_late = text_area_decode($vcl['vcl_recv_late']);
			if ($vcl['vcl_fetch_early'])
				$vcl_fetch_early = text_area_decode($vcl['vcl_fetch_early']);
			if ($vcl['vcl_fetch_late'])
				$vcl_fetch_late = text_area_decode($vcl['vcl_fetch_late']);
			if ($vcl['vcl_pipe_early'])
				$vcl_pipe_early = text_area_decode($vcl['vcl_pipe_early']);
			if ($vcl['vcl_pipe_late'])
				$vcl_pipe_late = text_area_decode($vcl['vcl_pipe_late']);
		}
	}
		$vcl_recv_set_basic='#BASIC VCL RULES SETTING'."\n";
		$vcl_recv_action_basic='#BASIC VCL RULES ACTIONS'."\n";
		#$plataform=posix_uname();
		if (is_array($config['installedpackages']['varnishsettings']['config']))
		  foreach($config['installedpackages']['varnishsettings']['config'] as $vcl) {
			if ($vcl['streaming'])
				$vcl_fetch_stream="set beresp.do_stream = true;\n";
			if ($vcl['fixgzip']) {
				$vcl_recv_set_basic.="\t#Fix gzip compression\n";
				$vcl_recv_set_basic.="\t".'if (req.http.Accept-Encoding) {'."\n";
				$vcl_recv_set_basic.="\t".'if (req.url ~ "\.(gif|jpg|jpeg|bmp|png|ico|img|tga|wmf|gz|tgz|bz2|tbz|mp3|ogg)$") {'."\n\t\tunset req.http.Accept-Encoding;\n\t\t}\n";
				$vcl_recv_set_basic.="\t".'else if (req.http.Accept-Encoding ~ "gzip") {'."\n\t\tset req.http.Accept-Encoding = ".'"gzip"'.";\n\t\t}\n";
				$vcl_recv_set_basic.="\t".'else if (req.http.Accept-Encoding ~ "deflate") {'."\n\t\tset req.http.Accept-Encoding = ".'"deflate"'.";\n\t\t}\n";
				$vcl_recv_set_basic.="\telse\t{\n\t\tunset req.http.Accept-Encoding;\n\t\t}\n\t}\n";
			}
			$vcl_recv_set_basic.="\t#set client balance identity\n";
			switch ($vcl['clientbalance']){
				case 'url':
						$vcl_recv_set_basic.="\t".'set client.identity = req.url;'."\n\n";
					break;
				case 'ip':
					$vcl_recv_set_basic.="\t".'set client.identity = client.ip;'."\n\n";
					break;
				case 'agent':
					$vcl_recv_set_basic.="\t".'set client.identity = req.http.user-agent;'."\n\n";
					break;
				}
			if ($vcl['grace'])
				$vcl_grace_time="set beresp.grace = ".$vcl['grace'].";\n\t\t";
			if ($vcl['saint'])
				$vcl_saint_mode="set beresp.saintmode = ".$vcl['saint'].";\n\t\t";
			if ($vcl['xforward']) {
				$vcl_recv_set_basic.="\t#set X-forward\n";
				switch ($vcl['xforward']) {
					case 'set':
						$vcl_recv_set_basic .= "\tset req.http.X-Forwarded-For = client.ip;\n\n";
					break;
					case 'append':
						$vcl_recv_set_basic .= "\tset req.http.X-Forwarded-For = req.http.X-Forwarded-For + \",\" + client.ip;\n\n";
					break;
					case 'create':
						$vcl_recv_set_basic .= "\tset req.http.X-Forwarded-Varnish = client.ip;\n\n";
					break;
					case 'unset':
						$vcl_recv_set_basic .= "\tunset req.http.X-Forwarded-For;\n\n";
					break;
				}
			}
			if ($vcl['postcache']) {
				$vcl_recv_action_basic .= "\t#Disable post cache\n";
				$vcl_recv_action_basic .= "\t".'if (req.request == "POST") {'."\n\t\treturn(pass);\n\t\t}\n";
			}

			$vcl_fetch_session = "#Disable cache when backend is starting a session\n";
			$vcl_fetch_session .= "\t".'if (beresp.http.Set-Cookie && beresp.http.Set-Cookie ~ "(PHPSESSID|phpsessid)") {'."\n\t\treturn(hit_for_pass);\n\t\t}\n";
			$vcl_fetch_session .= "\t".'if (beresp.http.Set-Cookie && beresp.http.Set-Cookie ~ "(JSESSION|jsession)") {'."\n\t\treturn(hit_for_pass);\n\t\t}\n";
					
			if ($vcl['sessioncache'] == "never") {
				$vcl_recv_session = "\t#Disable session cache\n";
				$vcl_recv_session .= "\t".'if (req.http.Cookie && req.http.Cookie ~ "(PHPSESSID|phpsessid)") {'."\n\t\treturn(pass);\n\t\t}\n";
				$vcl_recv_session .= "\t".'if (req.http.Cookie && req.http.Cookie ~ "(JSESSION|jsession)") {'."\n\t\treturn(pass);\n\t\t}\n";
				$vcl_recv_session .= "\t".'if (req.http.X-Requested-With == "XMLHttpRequest" || req.url ~ "nocache") {'."\n\t\treturn(pass);\n\t\t}\n";
				$vcl_fetch_session .= "\t".'if (beresp.http.X-Requested-With == "XMLHttpRequest" || req.url ~ "nocache") {'."\n\t\treturn(hit_for_pass);\n\t\t}\n";
				$vcl_recv_static_prefix = ($vcl['staticache'] == "no" ? "" : "\n\t\tunset req.http.cookie;");
			} else {
				$vcl_hash = "#Enable Per user session cache.\n";
				$vcl_hash .= "sub vcl_hash {\n\thash_data(req.http.cookie);\n}\n";
			}
			// set static content var
			$vcl_recv_static_sufix = ($vcl['staticache'] == 'no' ? "pass" : "lookup");
			$vcl_recv_static = "\t#Enable static cache\n";
			$vcl_recv_static .= "\t".'if (req.request=="GET" && req.url ~ "\.(css|js|txt|zip|pdf|rtf|flv|swf|html|htm)$") {'.$vcl_recv_static_prefix."\n\t\treturn($vcl_recv_static_sufix);\n\t\t}\n";
			$vcl_recv_static .= "\t".'if (req.request=="GET" && req.url ~ "\.(gif|jpg|jpeg|bmp|png|ico|img|tga|wmf|mp3|ogg)$") {'.$vcl_recv_static_prefix."\n\t\treturn($vcl_recv_static_sufix);\n\t\t}\n";
			$vcl_fetch_static = "#Enable static cache\n";
			$vcl_fetch_static .= 'if (req.url ~ "\.(css|js|txt|zip|pdf|rtf|flv|swf|html|htm)$") {'."\n\tunset beresp.http.set-cookie;\n\t}\n";
			$vcl_fetch_static .= 'if (req.url ~ "\.(gif|jpg|jpeg|bmp|png|ico|img|tga|wmf|mp3|ogg)$") {'."\n\tunset beresp.http.set-cookie;\n\t}\n";
			
			switch ($vcl['staticache']) {
				case "all":
					// cache all static content, unseting cookie when present
					$vcl_recv_action_basic .= ($vcl['sessioncache'] == "never" ? $vcl_recv_static.$vcl_recv_session : $vcl_recv_static);
					$vcl_fetch_action = ($vcl['sessioncache'] == "never" ? $vcl_fetch_static.$vcl_fetch_session : $vcl_fetch_static);
					break;
				case "yes":
					// cache only object without cookie set
					$vcl_recv_action_basic .= ($vcl['sessioncache'] == "never" ? $vcl_recv_session.$vcl_recv_static : $vcl_recv_static);
					$vcl_fetch_action = $vcl_fetch_session;
					break;
				default:
					// no static cache at all
					$vcl_recv_action_basic .= $vcl_recv_static.$vcl_recv_session;
					$vcl_fetch_action = $vcl_fetch_session;
			}
				
			if ($vcl['rfc2616']) {
				$vcl_recv_action_basic .= "\t#Be rfc2616 compliant\n";
				$vcl_recv_action_basic .= "\t".'if (req.request ~ "^(GET|HEAD|PUT|POST|TRACE|OPTIONS|DELETE)$") {'."\n\t\treturn(lookup);\n\t\t}\n\telse\t{\n\t\treturn(pipe);\n\t\t}\n";
				//$vcl_recv_action_basic.="\t".'if (req.request != "GET" && req.request != "HEAD" && req.request != "PUT" && reqa.request != "POST" &&'."\n";
				//$vcl_recv_action_basic.="\t".'    req.request != "TRACE" && req.request != "OPTIONS" && req.request != "DELETE") {return(pipe);}'."\n\n";
			} else
				$vcl_recv_action_basic .= "\tif (req.request != \"GET\" && req.request != \"HEAD\") {return(pipe);}\n";
			if ($vcl['restarts'])
				$vcl_restarts = $vcl['restarts'];
			if ($vcl['htmlerror'])
				$errorvcl = text_area_decode($vcl['htmlerror']);
		}

	if (!$errorvcl)
		$errorvcl = <<<EOF
<html>
<head>
<title>"} + obj.status + " " + obj.response + {"</title>
</head>
<body>
<h1>Error "} + obj.status + " " + obj.response + {"</h1>
<p>"} + obj.response + {"</p>
<h3>Guru Meditation:</h3>
<p>XID: "} + req.xid + {"</p>
<hr>
<p>Varnish cache server</p>
</body>
138	</html>
EOF;

/* Grab configuration txt blocks */
/* Please keep this sequence to determine witch backends are in use */
$backends_in_use = array();
$lb_config= get_lb_directors_config_txt();
$urlmappings = varnish_get_url_mappings_txt();
$backends = get_backend_config_txt() . $lb_config ;
#$backends .= get_lb_directors_config_txt();


/* Start to build varnish default.vcl configurationf file */
$varnish_config_file = <<<EOF
# Varnish configuration file
# Automatically generated by the pfSense package system

# This file is located in /var/etc/default.vcl

sub vcl_error {
		if (obj.status == 503 && req.restarts < {$vcl_restarts}) {
			return(restart);
		}
	
	set obj.http.Content-Type = "text/html; charset=utf-8";
	synthetic {"<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
{$errorvcl}"};
	return(deliver);
	
}
        
{$backends}
{$vcl_hash}
sub vcl_recv {
	{$vcl_recv_early}
	{$vcl_recv_set_basic}
	{$urlmappings}
	{$vcl_recv_late}
	#respect client wish to refresh the page
	if (req.http.Pragma ~ "no-cache")
		{
		return(pass);
		}
	
	{$vcl_recv_action_basic}
	return(lookup);
}

sub vcl_pipe {
	{$vcl_pipe_early}
	# If we don't set the Connection: close header, any following
	# requests from the client will also be piped through and
	# left untouched by varnish. We don't want that.
	set req.http.connection = "close";
	# Note: no "pipe" action here - we'll fall back to the default
	# pipe method so that when any changes are made there, we
	# still inherit them.
	{$vcl_pipe_late}
}
sub vcl_hit {
	return (deliver);
}

sub vcl_miss {
	return (fetch);
}

sub vcl_fetch {
	{$vcl_fetch_stream}{$vcl_fetch_early}
	{$vcl_fetch_action}
	# Varnish respects the wishes of the backend application.
	if (beresp.http.Pragma ~ "no-cache" || beresp.http.Cache-Control ~ "(no-cache|no-store|private)") {
	return(hit_for_pass);
	}
    ## If the request to the backend returns a code other than 200, restart the loop
    ## If the number of restarts reaches the value of the parameter max_restarts,
    ## the request will be error'ed.  max_restarts defaults to 4.  This prevents
    ## an eternal loop in the event that, e.g., the object does not exist at all.
    if (beresp.status != 200 && beresp.status != 403 && beresp.status != 404 && 
    	beresp.status != 303 && beresp.status != 302 && beresp.status != 301 && beresp.status != 401 ) {
		{$vcl_saint_mode}return(restart);
        }

    
	{$vcl_fetch_late}
	{$vcl_grace_time}return(deliver);
}

sub vcl_deliver {
	##set resp.http.X-Served-By = server.hostname;
  	if (obj.hits > 0) {
    	set resp.http.X-Cache = "HIT";	
    	set resp.http.X-Cache-Hits = obj.hits;
  	} else {
    	set resp.http.X-Cache = "MISS";	
  	}
	return(deliver);
}

sub vcl_init {
	return (ok);
}
	
sub vcl_fini {
	return (ok);
}	

EOF;
		file_put_contents("/var/etc/default.vcl",$varnish_config_file,LOCK_EX);
		$cc_file="/usr/local/bin/cc";
		foreach (glob(VARNISH_LOCALBASE."/bin/gcc*") as $bin_file) {
			$gcc_file=$bin_file;
		}
		if (!file_exists($cc_file) && file_exists($gcc_file)){
			symlink($gcc_file,$cc_file);
		}
		
		$fd = fopen("/var/etc/default.vcl", "w");
		fwrite($fd, $varnish_config_file);
		fclose($fd);

		varnish_sync_on_changes();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function varnish_sync_on_changes() {
	global $config, $g;
	if (is_array($config['installedpackages']['varnishsync']['config'])){
		$varnish_sync=$config['installedpackages']['varnishsync']['config'][0];
		$synconchanges = $varnish_sync['synconchanges'];
		$synctimeout = $varnish_sync['synctimeout'];
		switch ($synconchanges){
			case "manual":
				if (is_array($varnish_sync[row])){
					$rs=$varnish_sync[row];
					}
				else{
					log_error("[varnish] xmlrpc sync is enabled but there is no hosts to push on varnish config.");
					return;
					}
				break;
			case "auto":
					if (is_array($config['hasync'])){
						$hasync=$config['hasync'];
						$rs[0]['ipaddress']=$hasync['synchronizetoip'];
						$rs[0]['username']=$hasync['username'];
						$rs[0]['password']=$hasync['password'];
					}
					else{
						log_error("[varnish] xmlrpc sync is enabled but there is no system backup hosts to push varnish config.");
						return;
					}
				break;			
			default:
				return;
			break;
		}
		if (is_array($rs)){
			log_error("[varnish] xmlrpc sync is starting.");
			foreach($rs as $sh){
				$sync_to_ip = $sh['ipaddress'];
				$password = $sh['password'];
				if($sh['username'])
					$username = $sh['username'];
				else
					$username = 'admin';
				if($password && $sync_to_ip)
					varnish_do_xmlrpc_sync($sync_to_ip, $username, $password,$synctimeout);
				}
			log_error("[varnish] xmlrpc sync is ending.");
			}
 		}
}
/* Do the actual XMLRPC sync */
function varnish_do_xmlrpc_sync($sync_to_ip, $username, $password,$synctimeout) {
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
	$xml['varnishcustomvcl'] = $config['installedpackages']['varnishcustomvcl'];
	$xml['varnishbackends'] = $config['installedpackages']['varnishbackends'];
	$xml['varnishlbdirectors'] = $config['installedpackages']['varnishlbdirectors'];
	$xml['varnishsettings'] = $config['installedpackages']['varnishsettings'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning Varnish XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after $synctimeout seconds */
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting varnish XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting varnish XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} else {
		log_error("varnish XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell varnish to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/varnish.inc');\n";
	$execcmd .= "sync_package_varnish();\nvarnish_start();";
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);
	
	log_error("varnish XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if(!$resp) {
		$error = "A communications error occurred while attempting varnish XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting varnish XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} else {
		log_error("varnish XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
	
}

?>
