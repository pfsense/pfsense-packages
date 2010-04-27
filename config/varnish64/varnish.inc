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

function sync_package_varnish() {
	global $config, $g;
	if($config['installedpackages']['varnishcustomvcl']['config'] != "") {
		foreach($config['installedpackages']['varnishcustomvcl']['config'] as $vcl) {
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
		}
		$fd = fopen("/usr/local/etc/rc.d/varnish.sh", "w");
		$rc_file = <<<EOF
#!/bin/sh
mkdir -p /var/varnish
rm /var/varnish/storage.bin 2>/dev/null
killall varnishd 2>/dev/null
sleep 1
/usr/local/sbin/varnishd \
	{$listeningport} \
	-f /var/etc/default.vcl \
	{$storage_type} \
	-w 200,4000,50

EOF;
		fwrite($fd, $rc_file);
		fclose($fd);
		exec("chmod a+rx /usr/local/etc/rc.d/varnish.sh");
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
<p>We are very sorry but an error occured during this request.</p>
</b>
<p>Please press refresh in your browser to try again.</p>
<p>Varnish Error "} obj.status " " obj.response {"</p>
<p>"} obj.response {"</p>
<h3>Guru Meditation:</h3>
<p>XID: "} req.xid {"</p>

</body>
</html>
"};
deliver;

EOF;

	$backends = "";
	$urlmappings = "";

	if($config['installedpackages']['varnishlbdirectors']['config'] != "") {
    	foreach($config['installedpackages']['varnishlbdirectors']['config'] as $backend) {
			$director = "";
			if($backend['row'])
				foreach($backend['row'] as $be) 
					$director .=  "\t{\n\t\t.backend = {$be['backendname']};\n\t}";
			$backends .= <<<EOFA
director {$backend['directorname']} {$backend['directortype']} {
{$director}
}


EOFA;
		}
	}

	
	if($config['installedpackages']['varnishbackends']['config'] != "") {
    	foreach($config['installedpackages']['varnishbackends']['config'] as $backend) {
			$isfirst = true;
			if($config['installedpackages']['varnish_lb_directors']['config'] != "") {
    			foreach($config['installedpackages']['varnish_lb_directors']['config']['row'] as $urlmapping) {
					if(!$isfirst) 
						$urlmappings .= "else ";
					$urlmappings .= <<<EOAU
if (req.http.host == "{$urlmapping['urlmapping']}") {
	set req.backend = "{$backend['backend']}";
	set req.http.host = "{$urlmapping['urlmapping']}";
}
 
EOAU;
				}
			}
			if($config['installedpackages']['varnishbackends']['config'])
   				foreach($config['installedpackages']['varnishbackends']['config'] as $urlmapping) {
					if($config['installedpackages']['varnishbackends']['config']['row'])
    					foreach($config['installedpackages']['varnishbackends']['config']['row'] as $urlmapping) {
							if(!$isfirst) 
								$urlmappings .= "else ";
							$urlmappings .= <<<EOAU
if (req.http.host == "{$urlmapping['urlmapping']}") {
	set req.backend = "{$backend['backend']}";
	set req.http.host = "{$urlmapping['url']}";
} 
EOAU;
				$isfirst = false;
				}
			}

			if($backend['connect_timeout'])
				$connect_timeout = $backend['connect_timeout'] . "s";
			else 
				$connect_timeout = "25s";
			if($backend['port'])
				$connect_timeout = $backend['port'];
			else 
				$connect_timeout = "80";
			if($backend['first_byte_timeout'])
				$first_byte_timeout = $backend['first_byte_timeout'];
			else 
				$first_byte_timeout = "300s";
			$backends .= <<<EOFA
backend {$backend['backendname']} {
	.host = "{$backend['ipaddress']}";
	.port = "{$backend['port']}";
	.first_byte_timeout = "{$first_byte_timeout}";
	.connect_timeout = "{$connect_timeout}";
}

EOFA;
		}
	}
	
if($vcl_fetch_early or $vcl_fetch_late) {
	$vcl_fetch = <<<FETCH
sub vcl_fetch {
	{$vcl_fetch_early}
	{$vcl_fetch_late}
	return(deliver);
}

FETCH;
}

$varnish_config_file = <<<EOF
# Varnish configuration file
# Automatically generated by the pfSense package system

sub vcl_error {
	{$errorvcl}
}

{$backends}

sub vcl_recv {
	{$vcl_recv_custom_early}
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
	{$vcl_recv_custom_late}
	if (req.request != "GET" && req.request != "HEAD") {
		pipe;
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
		exec("/usr/local/etc/rc.d/varnish.sh");
}

?>