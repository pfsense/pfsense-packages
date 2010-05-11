<?php
/* ========================================================================== */
/*
	varnish.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2010 Scott Ullrich <sullrich@gmail.com>
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

function varnish_settings_post_validate($post, $input_errors) {
	if($post['storagesize'] && !is_numeric($post['storagesize'])) 
		$input_errors[] = "A valid number is required for the field 'Storage size'";	
	if($post['listeningport'] && !is_numeric($post['listeningport'])) 
		$input_errors[] = "A valid number is required for the field 'Listening port'";	
	if($post['minworkers'] && !is_numeric($post['minworkers'])) 
		$input_errors[] = "A valid number is required for the field 'Minimum worker threads'";	
	if($post['maxworkers'] && !is_numeric($post['maxworkers'])) 
		$input_errors[] = "A valid number is required for the field 'Maximum worker threads'";	
	if($post['timeoutworkers'] && !is_numeric($post['timeoutworkers'])) 
		$input_errors[] = "A valid number is required for the field 'Worker thread timeout'";
}

function varnish_lb_directors_post_validate($post, $input_errors) {
	if (preg_match("/[^a-zA-Z0-9]/", $post['directorname']))
		$input_errors[] = "The directorname name must only contain the characters a-Z or 0-9";
	if(stristr($post['directorurl'], 'http'))
		$input_errors[] = "You do not need to include the http:// string in the director URL";
}

function varnish_backends_post_validate($post, $input_errors) {
 	if (preg_match("/[^a-zA-Z0-9]/", $post['backendname']))
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
}

function varnish_install() {
	create_varnish_rcd_file();
}

function varnish_deinstall() {
	create_varnish_rcd_file();
}

function varnish_start() {
	mwexec("/usr/local/etc/rc.d/varnish.sh");
}

/* Return true if a backend is in use */
function is_varnish_backend_in_use($backendname) {
	global $config, $g;
	if($config['installedpackages']['varnishlbdirectors']['config'] != "") 
		foreach($config['installedpackages']['varnishlbdirectors']['config'] as $url) 
			if($url['row'])
				foreach($url['row'] as $urlmapping) 
					if($urlmapping['backendname'] == $backendname) 
						return true;
	if($config['installedpackages']['varnishbackends']['config'] != "") 
		foreach($config['installedpackages']['varnishbackends']['config'] as $backend) 
			if($backend['backendname'] == $backendname)
				if($backend['row'])	
					foreach($backend['row'] as $row)
						if($row['urlmapping'])
							return true;
	return false;
}

/* Build the URL mappings logic VCL config txt */
function varnish_get_url_mappings_txt() {
	global $g, $config, $urlmappings;
	$backends = "";
	$isfirst = true;
	if($config['installedpackages']['varnishlbdirectors']['config'] != "") {
  		foreach($config['installedpackages']['varnishlbdirectors']['config'] as $url) {
			if(!$isfirst) 
				$urlmappings .= "else ";
			$urlmappings .= <<<EOAU
if (req.http.host == "{$url['directorurl']}") {
		set req.backend = {$url['directorname']};
		set req.http.host = "{$url['directorurl']}";
	} 
EOAU;
			$isfirst = false;
		}
	}
	if($config['installedpackages']['varnishbackends']['config'])	 		
		foreach($config['installedpackages']['varnishbackends']['config'] as $urlmapping) {
			if($urlmapping['row'])
				foreach($urlmapping['row'] as $url) {
					if($url['fieldtype']) 
						$fieldtype = $url['fieldtype'];
					else 
						$fieldtype = "==";
					if(!$isfirst) 
						$urlmappings .= "else ";
					$urlmappings .= <<<EOAU
if (req.http.host {$fieldtype} "{$url['urlmapping']}") {
		set req.backend = {$urlmapping['backendname']}BACKEND;
		set req.http.host = "{$url['urlmapping']}";
	} 
EOAU;
					$isfirst = false;
		}
	}
	
	return $urlmappings;
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
sysctl kern.ipc.nmbclusters=65536
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
	require_once("varnish.inc");
	sync_package_varnish();
?>
ENDOFF
/usr/local/sbin/varnishd \
	{$listeningport} \
	-f /var/etc/default.vcl \
	{$storage_type} \
	-w {$minworkers},{$maxworkers},{$timeoutworkers}

EOF;
	fwrite($fd, $rc_file);
	fclose($fd);
	exec("chmod a+rx /usr/local/etc/rc.d/varnish.sh");
}

function get_backend_config_txt() {
	global $config, $g;
	$backends_used = array();
	if($config['installedpackages']['varnishbackends']['config'] != "") {
    	foreach($config['installedpackages']['varnishbackends']['config'] as $backend) {
			if($backend['connect_timeout'])
				$connect_timeout = $backend['connect_timeout'] . "s";
			else 
				$connect_timeout = "25s";
			if($backend['port'])
				$connect_timeout = $backend['port'] . "s";
			else 
				$connect_timeout = "80";
			if($backend['first_byte_timeout'])
				$first_byte_timeout = $backend['first_byte_timeout'] . "s";
			else 
				$first_byte_timeout = "300s";
			if($backend['probe_url'])
				$probe_url = $backend['probe_url'];
			else 
				$probe_url = "/";
			if($backend['probe_interval'])
				$probe_interval = $backend['probe_interval'] . "s";
			else 
				$probe_interval = "1s";
			if($backend['probe_timeout'])
				$probe_timeout = $backend['probe_timeout'] . "s";
			else 
				$probe_timeout = "1s";
			if($backend['probe_window'])
				$probe_window = $backend['probe_window'];
			else 
				$probe_window = "5";
			if($backend['probe_threshold'])
				$probe_threshold = $backend['probe_threshold'];
			else 
				$probe_threshold = "5";
			if(is_varnish_backend_in_use($backend['backendname'])) 
				if(!in_array($backend['backendname'], $backends_used))
					$backends .= <<<EOFA
backend {$backend['backendname']}BACKEND {
	.host = "{$backend['ipaddress']}";
	.port = "{$backend['port']}";
	.first_byte_timeout = {$first_byte_timeout};
	.connect_timeout = {$connect_timeout};
	.probe = {
		.url = "{$probe_url}";
		.interval = {$probe_interval};
		.timeout = {$probe_timeout};
		.window = {$probe_window};
		.threshold = {$probe_threshold};
	}
}


EOFA;
			$backends_used[] = $backend['backendname'];
		}
	}
	return $backends;
}

function get_lb_directors_config_txt() {
	global $config, $g;
	if($config['installedpackages']['varnishlbdirectors']['config'] != "") {
    	foreach($config['installedpackages']['varnishlbdirectors']['config'] as $backend) {
			$director = "";
			if($backend['row'])
				foreach($backend['row'] as $be) {
					if($be['weight']) 
						$weight = "\t\t\t.weight = {$be['weight']};\n";
					else 
						$weight = "";
					$director .=  "\t{\n\t\t.backend = {$be['backendname']}BACKEND;\n{$weight}\t}";
				}
			$backends .= <<<EOFA
director {$backend['directorname']} {$backend['directortype']} {
{$director}
}


EOFA;
		}
	}
	return $backends;
}

function sync_package_varnish() {
	global $config, $g;
	if($config['installedpackages']['varnishcustomvcl']['config'] != "") {
		foreach($config['installedpackages']['varnishcustomvcl']['config'] as $vcl) {
			if($vcl['vcl_recv_early']) 
				$vcl_recv_early = $vcl['vcl_recv_early'];
			if($vcl['vcl_recv_late']) 
				$vcl_recv_late = $vcl['vcl_recv_late'];
			if($vcl['vcl_fetch_early']) 
				$vcl_fetch_early = $vcl['vcl_fetch_early'];
			if($vcl['vcl_fetch_late']) 
				$vcl_fetch_late = $vcl['vcl_fetch_late'];
			if($vcl['vcl_pipe_early']) 
				$vcl_pipe_early = $vcl['vcl_pipe_early'];
			if($vcl['vcl_pipe_late']) 
				$vcl_pipe_late = $vcl['vcl_pipe_late'];
		}
	}

	if(!$errorvcl) 
		$errorvcl = <<<EOF
set obj.http.Content-Type = "text/html; charset=utf-8";
synthetic {"
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>"} obj.status " " obj.response {"</title>
</head>
<body>
<b>
<p>We are very sorry but an error occurred during this request.</p>
</b>
<p>Please press refresh in your browser to try again.</p>
<p>Varnish Error "} obj.status " " obj.response {"</p>
<p>"} obj.response {"</p>
<h3>Guru Meditation:</h3>
<p>XID: "} req.xid {"</p>

</body>
</html>
"};
return(deliver);

EOF;

if($vcl_fetch_early or $vcl_fetch_late) {
	$vcl_fetch = <<<FETCH
sub vcl_fetch {
	{$vcl_fetch_early}
	{$vcl_fetch_late}
	return(deliver);
}

FETCH;
}

/* Grab configuration txt blocks */
$urlmappings = varnish_get_url_mappings_txt();
$backends = get_backend_config_txt();
$backends .= get_lb_directors_config_txt();

/* Start to build varnish default.vcl configurationf file */
$varnish_config_file = <<<EOF
# Varnish configuration file
# Automatically generated by the pfSense package system

# This file is located in /var/etc/default.vcl

sub vcl_error {
	{$errorvcl}
}

{$backends}

sub vcl_recv {
	{$vcl_recv_early}
	# If the client sent an X-Forwarded-For header, remove it. 
	# It cannot betrusted.
	unset req.http.X-Forwarded-For;
	# Note that we don't need to add the client ip to the X-Forwarded-For 
	# header, varnish will do that for us
	if (req.http.Accept-Encoding) {
		# Handle compression correctly. Varnish treats headers literally
		# not semantically. So it is very well possible that there are
		# cache misses because the headers sent by different browsers
		# aren't the same.
		# @see: http:// varnish.projects.linpro.no/wiki/FAQ/Compression
		if (req.http.Accept-Encoding ~ "gzip") {
			# if the browser supports it, we'll use gzip
			set req.http.Accept-Encoding = "gzip";
		} elsif (req.http.Accept-Encoding ~ "deflate") {
			# next, try deflate if it is supported
			set req.http.Accept-Encoding = "deflate";
		} else {
			# unknown algorithm. Probably junk, remove it
			unset req.http.Accept-Encoding;
		}
	}
	{$urlmappings}
	{$vcl_recv_late}
	if (req.request != "GET" && req.request != "HEAD") {
		return(pipe);
	}
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

{$vcl_fetch}

EOF;

		$fd = fopen("/var/etc/default.vcl", "w");
		fwrite($fd, $varnish_config_file);
		fclose($fd);

		varnish_sync_on_changes();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function varnish_sync_on_changes() {
	global $config, $g;
	log_error("[varnish] varnish_xmlrpc_sync.php is starting.");
	$synconchanges = $config['installedpackages']['varnishsync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	$sync_hosts = $config['installedpackages']['varnishsync']['config'];
	$previous_ip = "";
	$x=0;
	$sh = $config['installedpackages']['varnishsync']['config'][0];
	for($x=1; $x<isset($sh[$x]); $x++) {
		if($x > 1) 
			$counter = $x;
		else 
			$counter = "";
		$sync_to_ip = "";
		$password = "";
		if($sh['ipaddress' . $counter]) {
			$sync_to_ip = $sh['ipaddress' . $counter];
			$password   = $sh['password'  . $counter];
		}
		if($password && $sync_to_ip)
			varnish_do_xmlrpc_sync($sync_to_ip, $password);
	}
	log_error("[varnish] varnish_xmlrpc_sync.php is ending.");
}

/* Do the actual XMLRPC sync */
function varnish_do_xmlrpc_sync($sync_to_ip, $password) {
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
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting varnish XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
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
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting varnish XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting varnish XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "varnish Settings Sync", "");
	} else {
		log_error("varnish XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}

}

?>