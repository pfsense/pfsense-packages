<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    vhosts.inc
    Copyright (C) 2008 Mark J Crane
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

//show errors
	ini_set('display_errors', '1');
	//error_reporting (E_ALL); // Report everything
	//error_reporting (E_ALL ^ E_NOTICE); // Report everything
	error_reporting(E_ALL ^ E_NOTICE ^ E_WARNING ); //hide notices and warnings

if (!function_exists("pkg_is_service_running")) {
	function pkg_is_service_running($servicename) 
	{
		exec("/bin/ps ax | awk '{ print $5 }'", $psout);
		array_shift($psout);
		foreach($psout as $line) {
			$ps[] = trim(array_pop(explode(' ', array_pop(explode('/', $line)))));
		}
		if(is_service_running($servicename, $ps) or is_process_running($servicename) ) {
			return true;
		}
		else {
			return false;
		}
	}
}

if (!function_exists("byte_convert")) {
	function byte_convert( $bytes ) {
		if ($bytes<=0)
			return '0 Byte';

		$convention=1000; //[1000->10^x|1024->2^x]
		$s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
		$e=floor(log($bytes,$convention));
		return round($bytes/pow($convention,$e),2).' '.$s[$e];
	}
}

//sort array
	function sort_host($a, $b){
		return strcmp($a["host"], $b["host"]);
	}

	function sort_port($a, $b){
		return strcmp($a["port"], $b["port"]);
	}

	function sort_ip_address($a, $b){
		return natsort($a["ipaddress"], $b["ipaddress"]);
	}


//check to see if the service is installed if it is return the id
	function get_service_id ($service_array, $fieldname, $fieldvalue) {
		$x = 0;
		$id = '';
		foreach($service_array as $rowhelper) {
			if ($rowhelper[$fieldname] == $fieldvalue) {
				$id = $x; //return the id
			}
			$x++;
		}
		if (strlen($id) > 0) {
			return ($id);
		}
		else {
			return false;
		}
	}


function vhosts_sync_package_php()
{
	global $config;

	if($config['installedpackages']['vhosts']['config'] != "") {

		conf_mount_rw();
		config_unlock();

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
			foreach($vhostarray as $rowhelper) {
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
					}
					else {
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

		//HTTP configuration
		if (count($vhostarray_http) > 0) {

			$tmp = "#\n";
			$tmp .= "# lighttpd configuration file\n";
			$tmp .= "#\n";
			$tmp .= "# use a it as base for lighttpd 1.0.0 and above\n";
			$tmp .= "#\n";
			$tmp .= "############ Options you really have to take care of ####################\n";
			$tmp .= "\n";
			$tmp .= "## FreeBSD!\n";
			$tmp .= "server.event-handler		= \"freebsd-kqueue\"\n";
			$tmp .= "server.network-backend		= \"writev\"  ## Fixes 7.x upload issues\n";
			$tmp .= "\n";
			$tmp .= "## modules to load\n";
			$tmp .= "server.modules =   (\n";
			$tmp .= "				  \"mod_access\", \"mod_accesslog\",\n";
			$tmp .= "                  \"mod_fastcgi\", \"mod_cgi\",\"mod_rewrite\"\n";
			$tmp .= "				)\n";
			$tmp .= "\n";
			$tmp .= "## Unused modules\n";
			$tmp .= "#                               \"mod_setenv\",\n";
			$tmp .= "#                               \"mod_compress\"\n";
			$tmp .= "#                               \"mod_redirect\",\n";
			$tmp .= "#                               \"mod_rewrite\",\n";
			$tmp .= "#                               \"mod_ssi\",\n";
			$tmp .= "#                               \"mod_usertrack\",\n";
			$tmp .= "#                               \"mod_expire\",\n";
			$tmp .= "#                               \"mod_secdownload\",\n";
			$tmp .= "#                               \"mod_rrdtool\",\n";
			$tmp .= "#                               \"mod_auth\",\n";
			$tmp .= "#                               \"mod_status\",\n";
			$tmp .= "#                               \"mod_alias\",\n";
			$tmp .= "#                               \"mod_proxy\",\n";
			$tmp .= "#                               \"mod_simple_vhost\",\n";
			$tmp .= "#                               \"mod_evhost\",\n";
			$tmp .= "#                               \"mod_userdir\",\n";
			$tmp .= "#                               \"mod_cgi\",\n";
			$tmp .= "#                                \"mod_accesslog\"\n";
			$tmp .= "\n";
			$tmp .= "## a static document-root, for virtual-hosting take look at the\n";
			$tmp .= "## server.virtual-* options\n";
			$tmp .= "server.document-root        = \"/usr/local/vhosts/\"\n";
			$tmp .= "\n";
			$tmp .= "# Maximum idle time with nothing being written (php downloading)\n";
			$tmp .= "server.max-write-idle = 999\n";
			$tmp .= "\n";
			$tmp .= "## where to send error-messages to\n";
			$tmp .= "server.errorlog             = \"/var/log/lighttpd.error.log\"\n";
			$tmp .= "\n";
			$tmp .= "# files to check for if .../ is requested\n";
			$tmp .= "server.indexfiles           = ( \"index.php\", \"index.html\",\n";
			$tmp .= "                                \"index.htm\", \"default.htm\" )\n";
			$tmp .= "\n";
			$tmp .= "# mimetype mapping\n";
			$tmp .= "mimetype.assign             = (\n";
			$tmp .= "  \".pdf\"          =>      \"application/pdf\",\n";
			$tmp .= "  \".sig\"          =>      \"application/pgp-signature\",\n";
			$tmp .= "  \".spl\"          =>      \"application/futuresplash\",\n";
			$tmp .= "  \".class\"        =>      \"application/octet-stream\",\n";
			$tmp .= "  \".ps\"           =>      \"application/postscript\",\n";
			$tmp .= "  \".torrent\"      =>      \"application/x-bittorrent\",\n";
			$tmp .= "  \".dvi\"          =>      \"application/x-dvi\",\n";
			$tmp .= "  \".gz\"           =>      \"application/x-gzip\",\n";
			$tmp .= "  \".pac\"          =>      \"application/x-ns-proxy-autoconfig\",\n";
			$tmp .= "  \".swf\"          =>      \"application/x-shockwave-flash\",\n";
			$tmp .= "  \".tar.gz\"       =>      \"application/x-tgz\",\n";
			$tmp .= "  \".tgz\"          =>      \"application/x-tgz\",\n";
			$tmp .= "  \".tar\"          =>      \"application/x-tar\",\n";
			$tmp .= "  \".zip\"          =>      \"application/zip\",\n";
			$tmp .= "  \".mp3\"          =>      \"audio/mpeg\",\n";
			$tmp .= "  \".m3u\"          =>      \"audio/x-mpegurl\",\n";
			$tmp .= "  \".wma\"          =>      \"audio/x-ms-wma\",\n";
			$tmp .= "  \".wax\"          =>      \"audio/x-ms-wax\",\n";
			$tmp .= "  \".ogg\"          =>      \"audio/x-wav\",\n";
			$tmp .= "  \".wav\"          =>      \"audio/x-wav\",\n";
			$tmp .= "  \".gif\"          =>      \"image/gif\",\n";
			$tmp .= "  \".jpg\"          =>      \"image/jpeg\",\n";
			$tmp .= "  \".jpeg\"         =>      \"image/jpeg\",\n";
			$tmp .= "  \".png\"          =>      \"image/png\",\n";
			$tmp .= "  \".xbm\"          =>      \"image/x-xbitmap\",\n";
			$tmp .= "  \".xpm\"          =>      \"image/x-xpixmap\",\n";
			$tmp .= "  \".xwd\"          =>      \"image/x-xwindowdump\",\n";
			$tmp .= "  \".css\"          =>      \"text/css\",\n";
			$tmp .= "  \".html\"         =>      \"text/html\",\n";
			$tmp .= "  \".htm\"          =>      \"text/html\",\n";
			$tmp .= "  \".js\"           =>      \"text/javascript\",\n";
			$tmp .= "  \".asc\"          =>      \"text/plain\",\n";
			$tmp .= "  \".c\"            =>      \"text/plain\",\n";
			$tmp .= "  \".conf\"         =>      \"text/plain\",\n";
			$tmp .= "  \".text\"         =>      \"text/plain\",\n";
			$tmp .= "  \".txt\"          =>      \"text/plain\",\n";
			$tmp .= "  \".dtd\"          =>      \"text/xml\",\n";
			$tmp .= "  \".xml\"          =>      \"text/xml\",\n";
			$tmp .= "  \".mpeg\"         =>      \"video/mpeg\",\n";
			$tmp .= "  \".mpg\"          =>      \"video/mpeg\",\n";
			$tmp .= "  \".mov\"          =>      \"video/quicktime\",\n";
			$tmp .= "  \".qt\"           =>      \"video/quicktime\",\n";
			$tmp .= "  \".avi\"          =>      \"video/x-msvideo\",\n";
			$tmp .= "  \".asf\"          =>      \"video/x-ms-asf\",\n";
			$tmp .= "  \".asx\"          =>      \"video/x-ms-asf\",\n";
			$tmp .= "  \".wmv\"          =>      \"video/x-ms-wmv\",\n";
			$tmp .= "  \".bz2\"          =>      \"application/x-bzip\",\n";
			$tmp .= "  \".tbz\"          =>      \"application/x-bzip-compressed-tar\",\n";
			$tmp .= "  \".tar.bz2\"      =>      \"application/x-bzip-compressed-tar\"\n";
			$tmp .= " )\n";
			$tmp .= "\n";
			$tmp .= "# Use the \"Content-Type\" extended attribute to obtain mime type if possible\n";
			$tmp .= "#mimetypes.use-xattr        = \"enable\"\n";
			$tmp .= "\n";
			$tmp .= "#### accesslog module\n";
			$tmp .= "#accesslog.filename          = \"/dev/null\"\n";
			$tmp .= "\n";
			$tmp .= "## deny access the file-extensions\n";
			$tmp .= "#\n";
			$tmp .= "# ~    is for backupfiles from vi, emacs, joe, ...\n";
			$tmp .= "# .inc is often used for code includes which should in general not be part\n";
			$tmp .= "#      of the document-root\n";
			$tmp .= "url.access-deny             = ( \"~\", \".db\" )\n";
			$tmp .= "\n";
			$tmp .= "\n";
			$tmp .= "######### Options that are good to be but not neccesary to be changed #######\n";
			$tmp .= "\n";
			$tmp .= "## bind to port (default: 80)\n";
			$tmp .= "server.port                = $server_port\n";
			$tmp .= "\n";
			$tmp .= "#the regex method isn't working\n";
			$tmp .= "#\$HTTP[\"host\"] =~ \"(^|\\.)host01\\.com\$\" {\n";
			$tmp .= "#	server.document-root        = \"/usr/local/vhosts/host01\"\n";
			$tmp .= "#}\n";
			$tmp .= "\n";

			$ipaddress_previous_value = '';
			$host_previous_value = '';
			$port_previous_value = '';
			$ipaddress_and_port_previous_value = '';
			$x=1;
			foreach($vhostarray_http as $rowhelper) {
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

					//set directory default to the host
						if (strlen($directory) == 0) { $directory = $host; }

					//if the vhost directory doesn't exist then create it
						//echo '/usr/local/vhosts/'.$directory.'<br />';
						if (!is_dir('/usr/local/vhosts/'.$directory)) {
							exec("mkdir /usr/local/vhosts/".$directory);
						}
						if (!file_exists('/usr/local/vhosts/'.$directory.'/index.php')) {
							$index_file = '/usr/local/vhosts/'.$directory.'/index.php';
							$index_tmp = "<?php\n";
							$index_tmp .= "	echo phpinfo();\n";
							$index_tmp .= "?>\n";
							$fout = fopen($index_file,"w");
							fwrite($fout, $index_tmp);
							fclose($fout);
							unset($index_file);
						}

					//set the default port
						if (strlen($port) == 0) { $port = '8001'; }

					if ($ipaddress.':'.$port != $ipaddress_and_port_previous_value) {
						if ($x > 1) { $tmp .= "}\n\n"; }
						$tmp .= "\$SERVER[\"socket\"] == \"".$ipaddress.":".$port."\" {\n";
					}

					$tmp .= "	\$HTTP[\"host\"] == \"".$host."\" {\n";
					$tmp .= "		server.document-root        = \"/usr/local/vhosts/".$directory."\"\n";

					//enable ssl if the cert and key were both provided
					if (strlen($rowhelper['certificate']) > 0 && strlen($rowhelper['privatekey']) > 0) {
						$pem_file = "/var/etc/cert-vhosts-".$ipaddress."-".$port.".pem";
						$fout = fopen($pem_file,"w");
						fwrite($fout, $certificate.$privatekey);
						fclose($fout);
						$tmp .= "		ssl.pemfile = \"".$pem_file."\"\n";
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
			$tmp .= "}\n";


			/*
			$tmp .= "\$SERVER[\"socket\"] == \"10.7.0.1:8001\" {\n";
			$tmp .= "	\$HTTP[\"host\"] == \"vhost01.com\" {\n";
			$tmp .= "		#ssl.pemfile = \"/var/www/certs/localhost.pem\"\n";
			$tmp .= "		#ssl.engine = \"enable\"\n";
			$tmp .= "		server.document-root        = \"/usr/local/vhosts/host01\"\n";
			$tmp .= "		#server.errorlog = \"/var/log/lighttpd/nixcraft/error.log\"\n";
			$tmp .= "		#accesslog.filename = \"/var/log/lighttpd/nixcraft/access.log\"\n";
			$tmp .= "		#server.error-handler-404 = \"/e404.php\"\n";
			$tmp .= "	}\n";
			$tmp .= "\n";
			$tmp .= "	\$HTTP[\"host\"] == \"vhost03.com\" {\n";
			$tmp .= "		#ssl.pemfile = \"/var/www/certs/localhost.pem\"\n";
			$tmp .= "		#ssl.engine = \"enable\"\n";
			$tmp .= "		server.document-root        = \"/usr/local/vhosts/host03\"\n";
			$tmp .= "		#server.errorlog = \"/var/log/lighttpd/nixcraft/error.log\"\n";
			$tmp .= "		#accesslog.filename = \"/var/log/lighttpd/nixcraft/access.log\"\n";
			$tmp .= "		#server.error-handler-404 = \"/e404.php\"\n";
			$tmp .= "	}\n";
			$tmp .= "}\n";
			$tmp .= "\n";
			$tmp .= "\$SERVER[\"socket\"] == \"10.7.0.1:8002\" {\n";
			$tmp .= "	\$HTTP[\"host\"] == \"vhost02.com\" {\n";
			$tmp .= "		#ssl.pemfile = \"/var/www/certs/localhost.pem\"\n";
			$tmp .= "		#ssl.engine = \"enable\"\n";
			$tmp .= "		server.document-root = \"/usr/local/vhosts/host02\"\n";
			$tmp .= "		#server.errorlog = \"/var/log/lighttpd/nixcraft/error.log\"\n";
			$tmp .= "		#accesslog.filename = \"/var/log/lighttpd/nixcraft/access.log\"\n";
			$tmp .= "		#server.error-handler-404 = \"/e404.php\"\n";
			$tmp .= "	}\n";
			$tmp .= "}\n";
			*/
			$tmp .= "\n";
			$tmp .= "## error-handler for status 404\n";
			$tmp .= "#server.error-handler-404   = \"/error-handler.html\"\n";
			$tmp .= "#server.error-handler-404   = \"/error-handler.php\"\n";
			$tmp .= "\n";
			$tmp .= "## to help the rc.scripts\n";
			$tmp .= "server.pid-file            = \"/var/run/vhosts-http.pid\"\n";
			$tmp .= "\n";
			$tmp .= "## virtual directory listings\n";
			$tmp .= "server.dir-listing         = \"disable\"\n";
			$tmp .= "\n";
			$tmp .= "## enable debugging\n";
			$tmp .= "debug.log-request-header   = \"disable\"\n";
			$tmp .= "debug.log-response-header  = \"disable\"\n";
			$tmp .= "debug.log-request-handling = \"disable\"\n";
			$tmp .= "debug.log-file-not-found   = \"disable\"\n";
			$tmp .= "\n";
			$tmp .= "#### compress module\n";
			$tmp .= "#compress.cache-dir         = \"/tmp/lighttpd/cache/compress/\"\n";
			$tmp .= "#compress.filetype          = (\"text/plain\", \"text/html\")\n";
			$tmp .= "\n";
			$tmp .= "#server.network-backend = \"writev\"\n";
			$tmp .= "server.upload-dirs = ( \"/root/\", \"/tmp/\", \"/var/\" )\n";
			$tmp .= "server.max-request-size    = 2097152\n";
			$tmp .= "\n";
			$tmp .= "#### fastcgi module\n";
			$tmp .= "## read fastcgi.txt for more info\n";
			$tmp .= "fastcgi.server = ( \".php\" =>\n";
			$tmp .= "	( \"localhost\" =>\n";
			$tmp .= "		(\n";
			$tmp .= "			\"socket\" => \"/tmp/php5-fastcgi.socket\",\n";
			$tmp .= "			\"min-procs\" => 1,\n";
			$tmp .= "			\"max-procs\" => 1,\n";
			$tmp .= "			\"idle-timeout\" => 0,\n";
			$tmp .= "			\"bin-environment\" => (\n";
			$tmp .= "				\"PHP_FCGI_MAX_REQUESTS\" => \"500\",\n";
			$tmp .= "				\"PHP_FCGI_CHILDREN\" => \"1\"\n";
			$tmp .= "			),\n";
			$tmp .= "			\"bin-path\" => \"/usr/local/bin/php\"\n";
			$tmp .= "		)\n";
			$tmp .= "	)\n";
			$tmp .= ")\n";
			$tmp .= "\n";
			$tmp .= "#### CGI module\n";
			$tmp .= "cgi.assign                 = ( \".cgi\" => \"\" )\n";

			$fout = fopen("/var/etc/vhosts-http.conf","w");
			fwrite($fout, $tmp);
			unset($tmp);
			fclose($fout);
		} //end (if count(vhostarray_http) > 0)

		//HTTPS configuration
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

					//set directory default to the host
						if (strlen($directory) == 0) { $directory = $host; }

					//if the vhost directory doesn't exist then create it
						//echo '/usr/local/vhosts/'.$directory.'<br />';
						if (!is_dir('/usr/local/vhosts/'.$directory)) {
							exec("mkdir /usr/local/vhosts/".$directory);
						}
						if (!file_exists('/usr/local/vhosts/'.$directory.'/index.php')) {
							$index_file = '/usr/local/vhosts/'.$directory.'/index.php';
							$index_tmp = "<?php\n";
							$index_tmp .= "	echo phpinfo();\n";
							$index_tmp .= "?>\n";
							$fout = fopen($index_file,"w");
							fwrite($fout, $index_tmp);
							fclose($fout);
							unset($index_file);
						}

					//set the default port
						if (strlen($port) == 0) { $port = '443'; }

					$tmp = "#\n";
					$tmp .= "# lighttpd configuration file\n";
					$tmp .= "#\n";
					$tmp .= "# use a it as base for lighttpd 1.0.0 and above\n";
					$tmp .= "#\n";
					$tmp .= "############ Options you really have to take care of ####################\n";
					$tmp .= "\n";
					$tmp .= "## FreeBSD!\n";
					$tmp .= "server.event-handler		= \"freebsd-kqueue\"\n";
					$tmp .= "server.network-backend		= \"writev\"  ## Fixes 7.x upload issues\n";
					$tmp .= "\n";
					$tmp .= "## modules to load\n";
					$tmp .= "server.modules =   (\n";
					$tmp .= "				  \"mod_accesslog\",\n";
					$tmp .= "				  \"mod_access\", \"mod_accesslog\",\n";
					$tmp .= "                  \"mod_fastcgi\", \"mod_cgi\",\"mod_rewrite\"\n";
					$tmp .= "				)\n";
					$tmp .= "\n";
					$tmp .= "## Unused modules\n";
					$tmp .= "#                               \"mod_setenv\",\n";
					$tmp .= "#                               \"mod_compress\"\n";
					$tmp .= "#                               \"mod_redirect\",\n";
					$tmp .= "#                               \"mod_rewrite\",\n";
					$tmp .= "#                               \"mod_ssi\",\n";
					$tmp .= "#                               \"mod_usertrack\",\n";
					$tmp .= "#                               \"mod_expire\",\n";
					$tmp .= "#                               \"mod_secdownload\",\n";
					$tmp .= "#                               \"mod_rrdtool\",\n";
					$tmp .= "#                               \"mod_auth\",\n";
					$tmp .= "#                               \"mod_status\",\n";
					$tmp .= "#                               \"mod_alias\",\n";
					$tmp .= "#                               \"mod_proxy\",\n";
					$tmp .= "#                               \"mod_simple_vhost\",\n";
					$tmp .= "#                               \"mod_evhost\",\n";
					$tmp .= "#                               \"mod_userdir\",\n";
					$tmp .= "#                               \"mod_cgi\",\n";
					$tmp .= "#                               \"mod_accesslog\"\n";
					$tmp .= "\n";
					$tmp .= "## a static document-root, for virtual-hosting take look at the\n";
					$tmp .= "## server.virtual-* options\n";
					$tmp .= "\n";
					$tmp .= "## ".$description." \n\n";
					$tmp .= "server.document-root        = \"/usr/local/vhosts/".$directory."\"\n";

					$tmp .= "\n";
					$tmp .= "# Maximum idle time with nothing being written (php downloading)\n";
					$tmp .= "server.max-write-idle = 999\n";
					$tmp .= "\n";
					$tmp .= "## where to send error-messages to\n";
					$tmp .= "server.errorlog             = \"/var/log/lighttpd.error.log\"\n";
					$tmp .= "\n";
					$tmp .= "# files to check for if .../ is requested\n";
					$tmp .= "server.indexfiles           = ( \"index.php\", \"index.html\",\n";
					$tmp .= "                                \"index.htm\", \"default.htm\" )\n";
					$tmp .= "\n";
					$tmp .= "# mimetype mapping\n";
					$tmp .= "mimetype.assign             = (\n";
					$tmp .= "  \".pdf\"          =>      \"application/pdf\",\n";
					$tmp .= "  \".sig\"          =>      \"application/pgp-signature\",\n";
					$tmp .= "  \".spl\"          =>      \"application/futuresplash\",\n";
					$tmp .= "  \".class\"        =>      \"application/octet-stream\",\n";
					$tmp .= "  \".ps\"           =>      \"application/postscript\",\n";
					$tmp .= "  \".torrent\"      =>      \"application/x-bittorrent\",\n";
					$tmp .= "  \".dvi\"          =>      \"application/x-dvi\",\n";
					$tmp .= "  \".gz\"           =>      \"application/x-gzip\",\n";
					$tmp .= "  \".pac\"          =>      \"application/x-ns-proxy-autoconfig\",\n";
					$tmp .= "  \".swf\"          =>      \"application/x-shockwave-flash\",\n";
					$tmp .= "  \".tar.gz\"       =>      \"application/x-tgz\",\n";
					$tmp .= "  \".tgz\"          =>      \"application/x-tgz\",\n";
					$tmp .= "  \".tar\"          =>      \"application/x-tar\",\n";
					$tmp .= "  \".zip\"          =>      \"application/zip\",\n";
					$tmp .= "  \".mp3\"          =>      \"audio/mpeg\",\n";
					$tmp .= "  \".m3u\"          =>      \"audio/x-mpegurl\",\n";
					$tmp .= "  \".wma\"          =>      \"audio/x-ms-wma\",\n";
					$tmp .= "  \".wax\"          =>      \"audio/x-ms-wax\",\n";
					$tmp .= "  \".ogg\"          =>      \"audio/x-wav\",\n";
					$tmp .= "  \".wav\"          =>      \"audio/x-wav\",\n";
					$tmp .= "  \".gif\"          =>      \"image/gif\",\n";
					$tmp .= "  \".jpg\"          =>      \"image/jpeg\",\n";
					$tmp .= "  \".jpeg\"         =>      \"image/jpeg\",\n";
					$tmp .= "  \".png\"          =>      \"image/png\",\n";
					$tmp .= "  \".xbm\"          =>      \"image/x-xbitmap\",\n";
					$tmp .= "  \".xpm\"          =>      \"image/x-xpixmap\",\n";
					$tmp .= "  \".xwd\"          =>      \"image/x-xwindowdump\",\n";
					$tmp .= "  \".css\"          =>      \"text/css\",\n";
					$tmp .= "  \".html\"         =>      \"text/html\",\n";
					$tmp .= "  \".htm\"          =>      \"text/html\",\n";
					$tmp .= "  \".js\"           =>      \"text/javascript\",\n";
					$tmp .= "  \".asc\"          =>      \"text/plain\",\n";
					$tmp .= "  \".c\"            =>      \"text/plain\",\n";
					$tmp .= "  \".conf\"         =>      \"text/plain\",\n";
					$tmp .= "  \".text\"         =>      \"text/plain\",\n";
					$tmp .= "  \".txt\"          =>      \"text/plain\",\n";
					$tmp .= "  \".dtd\"          =>      \"text/xml\",\n";
					$tmp .= "  \".xml\"          =>      \"text/xml\",\n";
					$tmp .= "  \".mpeg\"         =>      \"video/mpeg\",\n";
					$tmp .= "  \".mpg\"          =>      \"video/mpeg\",\n";
					$tmp .= "  \".mov\"          =>      \"video/quicktime\",\n";
					$tmp .= "  \".qt\"           =>      \"video/quicktime\",\n";
					$tmp .= "  \".avi\"          =>      \"video/x-msvideo\",\n";
					$tmp .= "  \".asf\"          =>      \"video/x-ms-asf\",\n";
					$tmp .= "  \".asx\"          =>      \"video/x-ms-asf\",\n";
					$tmp .= "  \".wmv\"          =>      \"video/x-ms-wmv\",\n";
					$tmp .= "  \".bz2\"          =>      \"application/x-bzip\",\n";
					$tmp .= "  \".tbz\"          =>      \"application/x-bzip-compressed-tar\",\n";
					$tmp .= "  \".tar.bz2\"      =>      \"application/x-bzip-compressed-tar\"\n";
					$tmp .= " )\n";
					$tmp .= "\n";
					$tmp .= "# Use the \"Content-Type\" extended attribute to obtain mime type if possible\n";
					$tmp .= "#mimetypes.use-xattr        = \"enable\"\n";
					$tmp .= "\n";
					$tmp .= "#### accesslog module\n";
					$tmp .= "#accesslog.filename          = \"/dev/null\"\n";
					$tmp .= "\n";
					$tmp .= "## deny access the file-extensions\n";
					$tmp .= "#\n";
					$tmp .= "# ~    is for backupfiles from vi, emacs, joe, ...\n";
					$tmp .= "# .inc is often used for code includes which should in general not be part\n";
					$tmp .= "#      of the document-root\n";
					$tmp .= "url.access-deny             = ( \"~\", \".db\" )\n";
					$tmp .= "\n";
					$tmp .= "\n";
					$tmp .= "######### Options that are good to be but not neccesary to be changed #######\n";
					$tmp .= "\n";
					$tmp .= "## bind to port (default: 80)\n";
					$tmp .= "server.port                = ".$port."\n";
					$tmp .= "\n";
					$tmp .= "#the regex method isn't working\n";
					$tmp .= "#\$HTTP[\"host\"] =~ \"(^|\\.)host01\\.com\$\" {\n";
					$tmp .= "#	server.document-root        = \"/usr/local/vhosts/host01\"\n";
					$tmp .= "#}\n";
					$tmp .= "\n";

					//enable ssl if the cert and key were both provided
						$pem_file = "/var/etc/cert-vhosts-".$ipaddress."-".$port.".pem";
						$fout = fopen($pem_file,"w");
						//echo $certificate; //exit;
						fwrite($fout, $certificate.$privatekey);
						fclose($fout);
						$tmp .= "## ssl configuration\n";
						$tmp .= "ssl.pemfile = \"".$pem_file."\"\n";
						$tmp .= "ssl.engine = \"enable\"\n";
						unset($pem_file);

					$tmp .= "\n";
					$tmp .= "## error-handler for status 404\n";
					$tmp .= "#server.error-handler-404   = \"/error-handler.html\"\n";
					$tmp .= "#server.error-handler-404   = \"/error-handler.php\"\n";
					$tmp .= "\n";
					$tmp .= "## to help the rc.scripts\n";
					$tmp .= "server.pid-file            = \"/var/run/vhosts-".$ipaddress."-".$port."-ssl.pid\"\n";
					$tmp .= "\n";
					$tmp .= "## virtual directory listings\n";
					$tmp .= "server.dir-listing         = \"disable\"\n";
					$tmp .= "\n";
					$tmp .= "## enable debugging\n";
					$tmp .= "debug.log-request-header   = \"disable\"\n";
					$tmp .= "debug.log-response-header  = \"disable\"\n";
					$tmp .= "debug.log-request-handling = \"disable\"\n";
					$tmp .= "debug.log-file-not-found   = \"disable\"\n";
					$tmp .= "\n";
					$tmp .= "#### compress module\n";
					$tmp .= "#compress.cache-dir         = \"/tmp/lighttpd/cache/compress/\"\n";
					$tmp .= "#compress.filetype          = (\"text/plain\", \"text/html\")\n";
					$tmp .= "\n";
					$tmp .= "#server.network-backend = \"writev\"\n";
					$tmp .= "server.upload-dirs = ( \"/root/\", \"/tmp/\", \"/var/\" )\n";
					$tmp .= "server.max-request-size    = 2097152\n";
					$tmp .= "\n";
					$tmp .= "#### fastcgi module\n";
					$tmp .= "## read fastcgi.txt for more info\n";
					$tmp .= "fastcgi.server = ( \".php\" =>\n";
					$tmp .= "	( \"localhost\" =>\n";
					$tmp .= "		(\n";
					$tmp .= "			\"socket\" => \"/tmp/php5-fastcgi.socket\",\n";
					$tmp .= "			\"min-procs\" => 1,\n";
					$tmp .= "			\"max-procs\" => 1,\n";
					$tmp .= "			\"idle-timeout\" => 0,\n";
					$tmp .= "			\"bin-environment\" => (\n";
					$tmp .= "				\"PHP_FCGI_MAX_REQUESTS\" => \"500\",\n";
					$tmp .= "				\"PHP_FCGI_CHILDREN\" => \"1\"\n";
					$tmp .= "			),\n";
					$tmp .= "			\"bin-path\" => \"/usr/local/php5/php-cgi\"\n";
					$tmp .= "		)\n";
					$tmp .= "	)\n";
					$tmp .= ")\n";
					$tmp .= "\n";
					$tmp .= "#### CGI module\n";
					$tmp .= "cgi.assign                 = ( \".cgi\" => \"\" )\n";

					$fout = fopen("/var/etc/vhosts-".$ipaddress."-".$port."-ssl.conf","w");
					fwrite($fout, $tmp);
					unset($tmp);
					fclose($fout);

					if (!function_exists('write_rcfile')) { require("/etc/inc/service-utils.inc"); }
					write_rcfile(array(
						"file" => "vhosts-".$ipaddress."-".$port."-ssl.sh",
						"start" => "/usr/local/sbin/lighttpd -f /var/etc/vhosts-".$ipaddress."-".$port."-ssl.conf",
						"stop" => "kill `cat /var/run/vhosts-".$ipaddress."-".$port."-ssl.pid`"
						)
					);

					//add or update a service
						$a_service   = &$config['installedpackages']['service'];
						$ent['name'] = "vhosts-ssl-$x";
						$ent['rcfile'] = "vhosts-".$ipaddress."-".$port."-ssl.sh";
						$ent['executable'] = "vhosts-".$ipaddress."-".$port."-ssl";
						$ent['description'] = "vHosts SSL, Host: $host, IP Address: ".$ipaddress.", port: ".$port." desc: ".$description;
						$a_service   = $config['installedpackages']['service'];
						$service_id = get_service_id ($a_service, 'name', "vhosts-ssl-$x");
						if (is_int($service_id)) {
							//update
							$a_service[$service_id] = $ent;
						}
						else {
							//add
							$a_service[] = $ent;
						}

				} //if enabled
				$x++;
			} //end for each
		} //end if array count
		write_config();
		conf_mount_ro();

	}
}


function vhosts_sync_package() {
	global $config;   
	vhosts_sync_package_php();
}


function vhosts_install_command() {

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/vhosts/')) {
		exec("mkdir /usr/local/www/packages/vhosts/");
	}

	if (!is_dir('/usr/local/vhosts/')) {
		exec("mkdir /usr/local/vhosts/");
	}

	if(stristr(php_uname('r'), '7.2') == TRUE) {
		if (!file_exists('/usr/local/php5')) {
			chdir('/usr/local/');
			exec ("fetch https://files.pfsense.org/packages/7/vhosts/php5.tar.gz");
			exec("tar zxvf /usr/local/php5.tar.gz -C /usr/local/");
			exec("rm /usr/local/php5.tar.gz");
		}
		if (!file_exists('/usr/local/lib/libxml2.so.5')) {
			chdir('/usr/local/lib/');
			exec ("fetch https://files.pfsense.org/packages/7/vhosts/usr.local.lib/libxml2.so.5");
		}
		if (!file_exists('/usr/local/lib/libxml2.so')) {
			chdir('/usr/local/lib/');
			exec ("fetch https://files.pfsense.org/packages/7/vhosts/usr.local.lib/libxml2.so");
		}
		if (!file_exists('/usr/local/lib/libxml2.la')) {
			chdir('/usr/local/lib/');
			exec ("fetch https://files.pfsense.org/packages/7/vhosts/usr.local.lib/libxml2.la");
		}
		if (!file_exists('/usr/local/lib/libxml2.a')) {
			chdir('/usr/local/lib/');
			exec ("fetch https://files.pfsense.org/packages/7/vhosts/usr.local.lib/lib/libxml2.a");
		}
	}
	if(stristr(php_uname('r'), '8.1') == TRUE) {
		if (!file_exists('/usr/local/php5')) {
			chdir('/usr/local/');
			exec ("fetch https://files.pfsense.org/packages/8/vhosts/php5.tar.gz");
			exec("tar zxvf /usr/local/php5.tar.gz -C /usr/local/");
			exec("rm /usr/local/php5.tar.gz");
		}
	}

	//rename PHP files from .tmp to .php
	exec("cp /tmp/vhosts_php.tmp /usr/local/www/packages/vhosts/vhosts_php.php");
	unlink_if_exists("/tmp/vhosts_php.tmp");

	exec("cp /tmp/vhosts_php_edit.tmp /usr/local/www/packages/vhosts/vhosts_php_edit.php");
	unlink_if_exists("/tmp/vhosts_php_edit.tmp");

	chdir('/tmp/');
	exec ("fetch https://packages.pfsense.org/packages/config/vhosts/system_advanced_create_certs.tmp");
	exec("cp /tmp/system_advanced_create_certs.tmp /usr/local/www/packages/vhosts/system_advanced_create_certs.php");
	unlink_if_exists("/tmp/system_advanced_create_certs.tmp");

	//write_config();

	if (!function_exists('write_rcfile')) { require("/etc/inc/service-utils.inc"); }
	write_rcfile(array(
		"file" => "vhosts-http.sh",
		"start" => "/usr/local/sbin/lighttpd -f /var/etc/vhosts-http.conf",
		"stop" => "kill `cat /var/run/vhosts-http.pid`"
		)
	);

	vhosts_sync_package();

	//if (pkg_is_service_running('vhosts')) {  
		//documentation purposes
	//}

	conf_mount_ro();
	config_unlock();
  
}


function vhosts_deinstall_command()
{
	conf_mount_rw();
	config_lock();
	$handle = popen("/usr/local/etc/rc.d/vhosts.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/vhosts.xml");
	unlink_if_exists("/usr/local/pkg/vhosts.inc");
	unlink_if_exists("/usr/local/www/vhosts.inc");
	unlink_if_exists("/usr/local/etc/rc.d/vhosts-http.sh");
	exec ("rm /usr/local/etc/rc.d/vhosts*");
	exec ("rm /var/etc/vhosts*");
	exec("rm -R /usr/local/www/packages/vhosts");
	exec("rm -R /usr/local/php5");
	conf_mount_ro();
	config_unlock();
}

?>
