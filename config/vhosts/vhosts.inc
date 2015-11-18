<?php
/*
	vhosts.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
	Copyright (C) 2015 ESF, LLC
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

require_once('service-utils.inc');

//sort array
function sort_host($a, $b) {
	return strcmp($a["host"], $b["host"]);
}

function sort_port($a, $b) {
	return strcmp($a["port"], $b["port"]);
}

function sort_ip_address($a, $b) {
	return natsort($a["ipaddress"], $b["ipaddress"]);
}

// Check to see if the service is installed if it is return the id
function get_service_id ($service_array, $fieldname, $fieldvalue) {
	$x = 0;
	$id = '';
	foreach ($service_array as $rowhelper) {
		if ($rowhelper[$fieldname] == $fieldvalue) {
			$id = $x; //return the id
		}
		$x++;
	}
	if (strlen($id) > 0) {
		return ($id);
	} else {
		return false;
	}
}


function vhosts_sync_package() {
	global $config;

	if ($config['installedpackages']['vhosts']['config'] != "") {
		conf_mount_rw();

		//sort the vhosts array
		$vhostarray = $config['installedpackages']['vhosts']['config'];
		if (count(vhostarray) > 1) {
			usort($vhostarray, 'sort_ipaddress');
			usort($vhostarray, 'sort_host');
			usort($vhostarray, 'sort_port');
		}
		$vhostarray_http = '';
		$vhostarray_https = '';
		$x = 0;
		foreach ($vhostarray as $rowhelper) {
			if ($rowhelper['enabled'] != "false") {
				if (strlen($rowhelper['certificate']) > 0 && strlen($rowhelper['privatekey']) > 0) {
					$vhostarray_https[$x]['host'] = $rowhelper['host'];
					$vhostarray_https[$x]['ipaddress'] = $rowhelper['ipaddress'];
					$vhostarray_https[$x]['port'] = $rowhelper['port'];
					$vhostarray_https[$x]['directory'] = $rowhelper['directory'];
					$vhostarray_https[$x]['certificate'] = $rowhelper['certificate'];
					$vhostarray_https[$x]['privatekey'] = $rowhelper['privatekey'];
					$vhostarray_https[$x]['enabled'] = $rowhelper['enabled'];
					$vhostarray_https[$x]['description'] = $rowhelper['description'];
				} else {
					$vhostarray_http[$x]['host'] = $rowhelper['host'];
					$vhostarray_http[$x]['ipaddress'] = $rowhelper['ipaddress'];
					$vhostarray_http[$x]['port'] = $rowhelper['port'];
					$vhostarray_http[$x]['directory'] = $rowhelper['directory'];
					$vhostarray_http[$x]['certificate'] = '';
					$vhostarray_http[$x]['privatekey'] = '';
					$vhostarray_http[$x]['enabled'] = $rowhelper['enabled'];
					$vhostarray_http[$x]['description'] = $rowhelper['description'];
					$server_port = $rowhelper['port'];
				}
				$x++;
			}
		}
		unset($x);

		// HTTP configuration
		if (count($vhostarray_http) > 0) {

			$tmp = <<<EOF
#
# lighttpd configuration file
#
# use a it as base for lighttpd 1.0.0 and above
#
############ Options you really have to take care of ####################

## FreeBSD!
server.event-handler		= "freebsd-kqueue"
server.network-backend		= "writev"  ## Fixes 7.x upload issues

## modules to load
server.modules =   (
				  "mod_access", "mod_accesslog",
                  "mod_fastcgi", "mod_cgi","mod_rewrite"
				)

## Unused modules
#                               "mod_setenv",
#                               "mod_compress"
#                               "mod_redirect",
#                               "mod_rewrite",
#                               "mod_ssi",
#                               "mod_usertrack",
#                               "mod_expire",
#                               "mod_secdownload",
#                               "mod_rrdtool",
#                               "mod_auth",
#                               "mod_status",
#                               "mod_alias",
#                               "mod_proxy",
#                               "mod_simple_vhost",
#                               "mod_evhost",
#                               "mod_userdir",
#                               "mod_cgi",
#                               "mod_accesslog"

## a static document-root, for virtual-hosting take look at the
## server.virtual-* options
server.document-root        = "/usr/local/vhosts/"

# Maximum idle time with nothing being written (php downloading)
server.max-write-idle = 999

## where to send error-messages to
server.errorlog             = "/var/log/lighttpd.error.log"

# files to check for if .../ is requested
server.indexfiles           = ( "index.php", "index.html",
                                "index.htm", "default.htm" )

# mimetype mapping
mimetype.assign             = (
  ".pdf"          =>      "application/pdf",
  ".sig"          =>      "application/pgp-signature",
  ".spl"          =>      "application/futuresplash",
  ".class"        =>      "application/octet-stream",
  ".ps"           =>      "application/postscript",
  ".torrent"      =>      "application/x-bittorrent",
  ".dvi"          =>      "application/x-dvi",
  ".gz"           =>      "application/x-gzip",
  ".pac"          =>      "application/x-ns-proxy-autoconfig",
  ".swf"          =>      "application/x-shockwave-flash",
  ".tar.gz"       =>      "application/x-tgz",
  ".tgz"          =>      "application/x-tgz",
  ".tar"          =>      "application/x-tar",
  ".zip"          =>      "application/zip",
  ".mp3"          =>      "audio/mpeg",
  ".m3u"          =>      "audio/x-mpegurl",
  ".wma"          =>      "audio/x-ms-wma",
  ".wax"          =>      "audio/x-ms-wax",
  ".ogg"          =>      "audio/x-wav",
  ".wav"          =>      "audio/x-wav",
  ".gif"          =>      "image/gif",
  ".jpg"          =>      "image/jpeg",
  ".jpeg"         =>      "image/jpeg",
  ".png"          =>      "image/png",
  ".xbm"          =>      "image/x-xbitmap",
  ".xpm"          =>      "image/x-xpixmap",
  ".xwd"          =>      "image/x-xwindowdump",
  ".css"          =>      "text/css",
  ".html"         =>      "text/html",
  ".htm"          =>      "text/html",
  ".js"           =>      "text/javascript",
  ".asc"          =>      "text/plain",
  ".c"            =>      "text/plain",
  ".conf"         =>      "text/plain",
  ".text"         =>      "text/plain",
  ".txt"          =>      "text/plain",
  ".dtd"          =>      "text/xml",
  ".xml"          =>      "text/xml",
  ".mpeg"         =>      "video/mpeg",
  ".mpg"          =>      "video/mpeg",
  ".mov"          =>      "video/quicktime",
  ".qt"           =>      "video/quicktime",
  ".avi"          =>      "video/x-msvideo",
  ".asf"          =>      "video/x-ms-asf",
  ".asx"          =>      "video/x-ms-asf",
  ".wmv"          =>      "video/x-ms-wmv",
  ".bz2"          =>      "application/x-bzip",
  ".tbz"          =>      "application/x-bzip-compressed-tar",
  ".tar.bz2"      =>      "application/x-bzip-compressed-tar"
 )

# Use the "Content-Type" extended attribute to obtain mime type if possible
#mimetypes.use-xattr        = "enable"

#### accesslog module
#accesslog.filename          = "/dev/null"

## deny access the file-extensions
#
# ~    is for backupfiles from vi, emacs, joe, ...
# .inc is often used for code includes which should in general not be part
#      of the document-root
url.access-deny             = ( "~", ".db" )


######### Options that are good to be but not neccesary to be changed #######

## bind to port (default: 80)
server.port                = {$server_port}

EOF;

			$ipaddress_previous_value = '';
			$host_previous_value = '';
			$port_previous_value = '';
			$ipaddress_and_port_previous_value = '';
			$x = 1;

			foreach ($vhostarray_http as $rowhelper) {
				if ($rowhelper['enabled'] != "false") {
					$host = $rowhelper['host'];
					$ipaddress = $rowhelper['ipaddress'];
					$port = $rowhelper['port'];
					$directory = $rowhelper['directory'];
					if (strlen($rowhelper['certificate']) > 0) {
						$certificate = base64_decode($rowhelper['certificate']);
					}
					if (strlen($rowhelper['privatekey']) > 0) {
						$privatekey = base64_decode($rowhelper['privatekey']);
					}

					// Set directory default to the host
					if (strlen($directory) == 0) {
						$directory = $host;
					}

					// If the vhost directory doesn't exist then create it
					safe_mkdir("/usr/local/vhosts/{$directory}");

					if (!file_exists("/usr/local/vhosts/{$directory}/index.php")) {
						$index_file = "/usr/local/vhosts/{$directory}/index.php";
						$index_tmp = "<?php\n";
						$index_tmp .= "	echo phpinfo();\n";
						$index_tmp .= "?>\n";
						$fout = fopen($index_file, "w");
						fwrite($fout, $index_tmp);
						fclose($fout);
						unset($index_file);
					}

					// Set the default port
					if (strlen($port) == 0) {
						$port = '10081';
					}

					if ($ipaddress . ':' . $port != $ipaddress_and_port_previous_value) {
						if ($x > 1) {
							$tmp .= "}\n\n";
						}
						$tmp .= "\$SERVER[\"socket\"] == \"" . $ipaddress . ":" . $port . "\" {\n";
					}

					$tmp .= "	\$HTTP[\"host\"] == \"" . $host . "\" {\n";
					$tmp .= "		server.document-root        = \"/usr/local/vhosts/" . $directory . "\"\n";

					// Enable SSL if the cert and key were both provided
					if (strlen($rowhelper['certificate']) > 0 && strlen($rowhelper['privatekey']) > 0) {
						$pem_file = "/var/etc/cert-vhosts-{$ipaddress}-{$port}.pem";
						$fout = fopen($pem_file, "w");
						fwrite($fout, $certificate.PHP_EOL.$privatekey);
						fclose($fout);
						$tmp .= "		ssl.pemfile = \"" . $pem_file . "\"\n";
						$tmp .= "		ssl.engine = \"enable\"\n";
						unset($pem_file);
					}
					if (count($vhostarray_http) > 0) {
						$tmp .= "	}\n";
					}

					$ipaddress_previous_value = $ipaddress;
					$host_previous_value = $host;
					$port_previous_value = $port;
					$ipaddress_and_port_previous_value = $ipaddress.':'.$port;
					$x++;
				}
			}

			$tmp .= <<<EOF
}

## error-handler for status 404
#server.error-handler-404   = "/error-handler.html"
#server.error-handler-404   = "/error-handler.php"

## to help the rc.scripts
server.pid-file            = "/var/run/vhosts-http.pid"

## virtual directory listings
server.dir-listing         = "disable"

## enable debugging
debug.log-request-header   = "disable"
debug.log-response-header  = "disable"
debug.log-request-handling = "disable"
debug.log-file-not-found   = "disable"

#### compress module
#compress.cache-dir         = "/tmp/lighttpd/cache/compress/"
#compress.filetype          = ("text/plain", "text/html")

#server.network-backend = "writev"
server.upload-dirs = ( "/root/", "/tmp/", "/var/" )
server.max-request-size    = 2097152

#### fastcgi module
## read fastcgi.txt for more info
fastcgi.server = ( ".php" =>
        ( "localhost" =>
                (
                        "socket" => "/var/run/php-fpm.socket",
                        "broken-scriptfilename" => "enable"
                )
        )
)

#### CGI module
cgi.assign                 = ( ".cgi" => "" )

EOF;

			$fout = fopen("/var/etc/vhosts-http.conf", "w");
			fwrite($fout, $tmp);
			unset($tmp);
			fclose($fout);
		// END (if count(vhostarray_http) > 0)
		}

		// HTTPS configuration
		$ipaddress_previous_value = '';
		$host_previous_value = '';
		$port_previous_value = '';
		$ipaddress_and_port_previous_value = '';
		$x=1;
		if (count($vhostarray_https) > 0) {
			foreach($vhostarray_https as $rowhelper) {
				if ($rowhelper['enabled'] != "false") {
					$host = $rowhelper['host'];
					$ipaddress = $rowhelper['ipaddress'];
					$port = $rowhelper['port'];
					$directory = $rowhelper['directory'];
					$description = $rowhelper['description'];
					if (strlen($rowhelper['certificate']) > 0) {
						$certificate = base64_decode($rowhelper['certificate']);
					}
					if (strlen($rowhelper['privatekey']) > 0) {
						$privatekey = base64_decode($rowhelper['privatekey']);
					}

					// Set directory default to the host
					if (strlen($directory) == 0) {
						$directory = $host;
					}

					// If the vhost directory doesn't exist then create it
					safe_mkdir("/usr/local/vhosts/{$directory}");

					if (!file_exists("/usr/local/vhosts/{$directory}/index.php")) {
						$index_file = "/usr/local/vhosts/{$directory}/index.php";
						$index_tmp = "<?php\n";
						$index_tmp .= "	echo phpinfo();\n";
						$index_tmp .= "?>\n";
						$fout = fopen($index_file, "w");
						fwrite($fout, $index_tmp);
						fclose($fout);
						unset($index_file);
					}

					// Set the default port
					if (strlen($port) == 0) {
						$port = '443';
					}

					$tmp =<<<EOF
#
# lighttpd configuration file
#
# use a it as base for lighttpd 1.0.0 and above
#
############ Options you really have to take care of ####################

## FreeBSD!
server.event-handler		= "freebsd-kqueue"
server.network-backend		= "writev"  ## Fixes 7.x upload issues

## modules to load
server.modules =   (
				  "mod_access", "mod_accesslog",
                  "mod_fastcgi", "mod_cgi","mod_rewrite"
				)

## Unused modules
#                               "mod_setenv",
#                               "mod_compress"
#                               "mod_redirect",
#                               "mod_rewrite",
#                               "mod_ssi",
#                               "mod_usertrack",
#                               "mod_expire",
#                               "mod_secdownload",
#                               "mod_rrdtool",
#                               "mod_auth",
#                               "mod_status",
#                               "mod_alias",
#                               "mod_proxy",
#                               "mod_simple_vhost",
#                               "mod_evhost",
#                               "mod_userdir",
#                               "mod_cgi",
#                               "mod_accesslog"

## a static document-root, for virtual-hosting take look at the
## server.virtual-* options

##

server.document-root        = "/usr/local/vhosts/"

# Maximum idle time with nothing being written (php downloading)
server.max-write-idle = 999

## where to send error-messages to
server.errorlog             = "/var/log/lighttpd.error.log"

# files to check for if .../ is requested
server.indexfiles           = ( "index.php", "index.html",
                                "index.htm", "default.htm" )

# mimetype mapping
mimetype.assign             = (
  ".pdf"          =>      "application/pdf",
  ".sig"          =>      "application/pgp-signature",
  ".spl"          =>      "application/futuresplash",
  ".class"        =>      "application/octet-stream",
  ".ps"           =>      "application/postscript",
  ".torrent"      =>      "application/x-bittorrent",
  ".dvi"          =>      "application/x-dvi",
  ".gz"           =>      "application/x-gzip",
  ".pac"          =>      "application/x-ns-proxy-autoconfig",
  ".swf"          =>      "application/x-shockwave-flash",
  ".tar.gz"       =>      "application/x-tgz",
  ".tgz"          =>      "application/x-tgz",
  ".tar"          =>      "application/x-tar",
  ".zip"          =>      "application/zip",
  ".mp3"          =>      "audio/mpeg",
  ".m3u"          =>      "audio/x-mpegurl",
  ".wma"          =>      "audio/x-ms-wma",
  ".wax"          =>      "audio/x-ms-wax",
  ".ogg"          =>      "audio/x-wav",
  ".wav"          =>      "audio/x-wav",
  ".gif"          =>      "image/gif",
  ".jpg"          =>      "image/jpeg",
  ".jpeg"         =>      "image/jpeg",
  ".png"          =>      "image/png",
  ".xbm"          =>      "image/x-xbitmap",
  ".xpm"          =>      "image/x-xpixmap",
  ".xwd"          =>      "image/x-xwindowdump",
  ".css"          =>      "text/css",
  ".html"         =>      "text/html",
  ".htm"          =>      "text/html",
  ".js"           =>      "text/javascript",
  ".asc"          =>      "text/plain",
  ".c"            =>      "text/plain",
  ".conf"         =>      "text/plain",
  ".text"         =>      "text/plain",
  ".txt"          =>      "text/plain",
  ".dtd"          =>      "text/xml",
  ".xml"          =>      "text/xml",
  ".mpeg"         =>      "video/mpeg",
  ".mpg"          =>      "video/mpeg",
  ".mov"          =>      "video/quicktime",
  ".qt"           =>      "video/quicktime",
  ".avi"          =>      "video/x-msvideo",
  ".asf"          =>      "video/x-ms-asf",
  ".asx"          =>      "video/x-ms-asf",
  ".wmv"          =>      "video/x-ms-wmv",
  ".bz2"          =>      "application/x-bzip",
  ".tbz"          =>      "application/x-bzip-compressed-tar",
  ".tar.bz2"      =>      "application/x-bzip-compressed-tar"
 )

# Use the "Content-Type" extended attribute to obtain mime type if possible
#mimetypes.use-xattr        = "enable"

#### accesslog module
#accesslog.filename          = "/dev/null"

## deny access the file-extensions
#
# ~    is for backupfiles from vi, emacs, joe, ...
# .inc is often used for code includes which should in general not be part
#      of the document-root
url.access-deny             = ( "~", ".db" )


######### Options that are good to be but not neccesary to be changed #######

## bind to port (default: 80)
server.port                = {$port}

EOF;
					// Enable SSL if the cert and key were both provided
					$pem_file = "/var/etc/cert-vhosts-{$ipaddress}-{$port}.pem";
					$fout = fopen($pem_file, "w");
					fwrite($fout, $certificate.PHP_EOL.$privatekey);
					fclose($fout);
					$tmp .= "## ssl configuration\n";
					$tmp .= "ssl.pemfile = \"" . $pem_file . "\"\n";
					$tmp .= "ssl.engine = \"enable\"\n";
					unset($pem_file);


					$tmp .= <<<EOF

## error-handler for status 404
#server.error-handler-404   = "/error-handler.html"
#server.error-handler-404   = "/error-handler.php"

## to help the rc.scripts
server.pid-file            = "/var/run/vhosts-{$ipaddress}-{$port}-ssl.pid"

## virtual directory listings
server.dir-listing         = "disable"

## enable debugging
debug.log-request-header   = "disable"
debug.log-response-header  = "disable"
debug.log-request-handling = "disable"
debug.log-file-not-found   = "disable"

#### compress module
#compress.cache-dir         = "/tmp/lighttpd/cache/compress/"
#compress.filetype          = ("text/plain", "text/html")

#server.network-backend = "writev"
server.upload-dirs = ( "/root/", "/tmp/", "/var/" )
server.max-request-size    = 2097152

#### fastcgi module
## read fastcgi.txt for more info
fastcgi.server = ( ".php" =>
        ( "localhost" =>
                (
                        "socket" => "/var/run/php-fpm.socket",
                        "broken-scriptfilename" => "enable"
                )
        )
)

#### CGI module
cgi.assign                 = ( ".cgi" => "" )

EOF;

					$fout = fopen("/var/etc/vhosts-{$ipaddress}-{$port}-ssl.conf", "w");
					fwrite($fout, $tmp);
					unset($tmp);
					fclose($fout);

					write_rcfile(array(
						"file" => "vhosts-{$ipaddress}-{$port}-ssl.sh",
						"start" => "/usr/local/sbin/lighttpd -f /var/etc/vhosts-{$ipaddress}-{$port}-ssl.conf",
						"stop" => "kill `cat /var/run/vhosts-{$ipaddress}-{$port}-ssl.pid`"
						)
					);

					// Add or update a service
					$ent['name'] = "vhosts-ssl-{$x}";
					$ent['rcfile'] = "vhosts-{$ipaddress}-{$port}-ssl.sh";
					$ent['executable'] = "vhosts-{$ipaddress}-{$port}-ssl";
					$ent['description'] = "vHosts SSL, Host: {$host}, IP Address: {$ipaddress}, port: {$port}, desc: {$description}";
					$ent['custom_php_service_status_command'] = "\$vhost_output=''; exec('/bin/pgrep -anf '.".escapeshellarg($ent['executable']).", \$vhost_output, \$retval); \$rc=(intval(\$retval) == 0);";
					$a_service   = $config['installedpackages']['service'];
					$service_id = get_service_id ($a_service, 'name', "vhosts-ssl-{$x}");
					if (is_int($service_id)) {
						// Update
						$a_service[$service_id] = $ent;
					} else {
						// Add
						$a_service[] = $ent;
					}
				// END if enabled
				}
				$x++;
			// END foreach
			}
		 // END if array count
		}
		write_config();
		conf_mount_ro();
	}
}

function vhosts_install_command() {
	global $config;
	safe_mkdir("/usr/local/vhosts/");

	write_rcfile(array(
		"file" => "vhosts-http.sh",
		"start" => "/usr/local/sbin/lighttpd -f /var/etc/vhosts-http.conf",
		"stop" => "kill `cat /var/run/vhosts-http.pid`"
		)
	);

	vhosts_sync_package();
}


function vhosts_deinstall_command() {
	exec("/bin/rm -f /usr/local/etc/rc.d/vhosts*");
	exec("/bin/rm -f /var/etc/vhosts*");
	exec("/bin/rm -rf /usr/local/www/packages/vhosts");
}

?>
