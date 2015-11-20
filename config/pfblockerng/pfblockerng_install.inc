<?php
/*
	pfBlockerNG_install.inc

	pfBlockerNG
	Copyright (c) 2015 BBcan177@gmail.com
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

require_once('/usr/local/pkg/pfblockerng/pfblockerng.inc');
require_once('/usr/local/www/pfblockerng/pfblockerng.php');

global $config, $pfb, $static_output;
pfb_global();

function update_static_output($text) {
	global $static_output;

	$static_output .= "{$text}";
	update_output_window("{$static_output}");
	return;
}


// Set 'Install flag' to skip sync process during installations.
$g['pfblockerng_install'] = true;

// Remove previous ccdir location files if exist
$old_ccfiles = glob('/usr/pbi/pfblockerng-' . php_uname('m') . '/share/GeoIP/*_v?.txt');
if (!empty($old_ccfiles)) {
	foreach ($old_ccfiles as $oldfile) {
		unlink_if_exists("{$oldfile}");
	}
}

update_static_output("\nDownloading MaxMind Country databases.\n This may take a minute...");

// Only Download the MaxMind Database files (Skip Alexa)
unset($pfb['extras'][5]);

if (!pfblockerng_download_extras()) {
	update_static_output(" MaxMind download failed!\nFetching MaxMind archive from pfSense package repo...");
	// Fetch archived MaxMind database
	$url = 'https://packages.pfsense.org/packages/config/pfblockerng/countrycodes.tar.bz2';
	exec("/usr/bin/fetch -o /tmp/countrycodes.tar.bz2 {$url}");
	// Uncompress archived Country code file
	exec("/usr/bin/tar -jx -C {$pfb['ccdir']} -f /tmp/countrycodes.tar.bz2");
	update_static_output(" done.\n");
}
else {
	update_static_output(" done.\n");
}

update_static_output("Converting MaxMind Country databases for pfBlockerNG.\n This may take a few minutes...");
pfblockerng_uc_countries();
update_static_output(" done.\nCreating pfBlockerNG Continent XML files...");
pfblockerng_get_countries();

if ($pfb['keep'] == 'on' && isset($pfb['widgets']) && strpos($pfb['widgets'], 'pfblockerng-container') !== FALSE) {
	update_static_output(" done.\nRestoring previous pfBlockerNG Widget settings...");
	// Restore previous widget setting if 'keep' is enabled.
	$config['widgets']['sequence'] = $pfb['widgets'];
	write_config('pfBlockerNG: Save widget');
} else {
	update_static_output(" done.\nAdding pfBlockerNG Widget to the Dashboard...");
	$widgets = $config['widgets']['sequence'];
	if (strpos($widgets, 'pfblockerng-container') === FALSE) {
		if (empty($widgets)) {
			$config['widgets']['sequence'] = 'pfblockerng-container:col2:show';
		} else {
			$config['widgets']['sequence'] .= ',pfblockerng-container:col2:show';
		}
		write_config('pfBlockerNG: Save widget');
	}
}

update_static_output(" done.\nRemove any existing and create link for DNSBL lighttpd executable...");
unlink_if_exists('/usr/local/sbin/lighttpd_pfb');
link('/usr/local/sbin/lighttpd', '/usr/local/sbin/lighttpd_pfb');

update_static_output(" done.\nCreating DNSBL web server start-up script...");
$rc = array();
$rc['file'] = 'dnsbl.sh';
$rc['start'] = <<<EOF

	# Start DNSBL Lighttpd webserver
	if [ -f '{$pfb['dnsbl_conf']}' ]; then
		/usr/local/sbin/lighttpd_pfb -f '{$pfb['dnsbl_conf']}'
	fi

	# Terminate DNSBL HTTPS Daemon if found
	pidnum="$(/bin/ps -wax | /usr/bin/grep '[p]fblockerng.inc dnsbl' | /usr/bin/awk '{print $1}')"
	if [ ! -z "\${pidnum}" ]; then
		/bin/kill -9 "\${pidnum}"
		/bin/sleep 2
	fi

	# Start DNSBL HTTPS Daemon
	/usr/local/bin/php -f /usr/local/pkg/pfblockerng/pfblockerng.inc dnsbl &

EOF;

$rc['stop'] = <<<EOF

	# Terminate DNSBL Lighttpd webserver, if found.
	pidnum="$(/bin/pgrep lighttpd_pfb)"
	if [ ! -z "\${pidnum}" ]; then
		/usr/bin/killall lighttpd_pfb
	fi

	# Terminate DNSBL HTTPS Daemon, if found.
	pidnum="$(/bin/ps -wax | /usr/bin/grep '[p]fblockerng.inc dnsbl' | /usr/bin/awk '{print $1}')"
	if [ ! -z "\${pidnum}" ]; then
		/bin/kill -9 "\${pidnum}"
		/bin/sleep 2
	fi

EOF;

write_rcfile($rc);
update_static_output(" done.\n");

if ($pfb['enable'] == 'on' && $pfb['dnsbl'] == 'on' && !empty($pfb['dnsbl_port']) && !empty($pfb['dnsbl_port_ssl'])) {
	update_static_output("Creating DNSBL web server config ...");

	$pfb_conf = <<<EOF
#
#pfBlockerNG Lighttpd DNSBL configuration file
#
server.bind			= "0.0.0.0"
server.port			= "{$pfb['dnsbl_port']}"
server.event-handler		= "freebsd-kqueue"
server.network-backend		= "freebsd-sendfile"
server.dir-listing		= "disable"
server.document-root		= "/usr/local/www/pfblockerng/www/"
server.errorlog			= "/var/log/pfblockerng/dnsbl_error.log"
server.pid-file			= "/var/run/dnsbl.pid"
server.modules			= ( "mod_access", "mod_fastcgi", "mod_rewrite" )

server.indexfiles		= ( "index.php" )
mimetype.assign			= ( ".html" => "text/html", ".gif" => "image/gif" )
url.access-deny			= ( "~", ".inc" )
fastcgi.server			= ( ".php" => ( "localhost" => ( "socket" => "/var/run/php-fpm.socket", "broken-scriptfilename" => "enable" ) ) )

debug.log-condition-handling	= "enable"

\$HTTP["host"] =~ ".*" {
	url.rewrite-once = ( ".*" => "index.php" )
}

\$SERVER["socket"] == "0.0.0.0:{$pfb['dnsbl_port_ssl']}" {
	ssl.engine		= "enable"
	ssl.pemfile		= "{$pfb['dnsbl_cert']}"
	ssl.use-sslv2		= "disable"
	ssl.use-sslv3		= "disable"
	ssl.honor-cipher-order	= "enable"
	ssl.cipher-list		= "AES128+EECDH:AES256+EECDH:AES128+EDH:AES256+EDH:AES128-SHA:AES256-SHA:!aNULL:!eNULL:!DSS"

	\$HTTP["host"] =~ ".*" {
		url.rewrite-once = ( ".*" => "index.php" )
	}
}

EOF;

	@file_put_contents($pfb['dnsbl_conf'], $pfb_conf, LOCK_EX);
	unset($pfb_conf);
	update_static_output(" done.\n");

	update_static_output("Starting DNSBL Service...");
	start_service('dnsbl');
	update_static_output(" done.\n");
}

unset($g['pfblockerng_install']);	// Remove 'Install flag'
update_static_output("Custom commands completed ... ");
return TRUE;

?>