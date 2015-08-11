<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    phpservice.inc
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
 
function phpservice_sync_package_php()
{

  global $config; 
  if($config['installedpackages']['phpservice']['config'] != "") {

    conf_mount_rw();
    
		$tmp = "<?php\n";
		$tmp .= "\n";
		$tmp .= "// Set time limit to indefinite execution\n";
		$tmp .= "set_time_limit (0);\n";
		$tmp .= "\n";
		$tmp .= "//run this program as long as the pid file exists\n";
		$tmp .= "\$filename = '/tmp/phpmonitor.pid';\n";
		$tmp .= "\$fp = fopen(\$filename, 'w');\n";
		$tmp .= "fwrite(\$fp, \"If this file is deleted then phpmonitor will stop.\");\n";
		$tmp .= "fclose(\$fp);\n";
		$tmp .= "unset(\$filename);\n";
		$tmp .= "\n";
		$tmp .= "//require_once(\"config.inc\");\n";
		$tmp .= "//global \$config;\n";
		$tmp .= "//\$syslogaddress = \$config['syslog']['remoteserver'];\n";
		$tmp .= "\$syslogaddress = \"127.0.0.1\";\n";
		$tmp .= "\$syslogport = 514;\n";
		$tmp .= "echo \"syslog server: \".\$syslogaddress.\"\\n\";\n";
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "function send_to_syslog(\$syslogaddress, \$syslogport, \$syslogmsg) {\n";
		$tmp .= "\n";
		$tmp .= "  \$fp = fsockopen(\"udp://\".\$syslogaddress, \$syslogport, \$errno, \$errstr);\n";
		$tmp .= "  if (!\$fp) {\n";
		$tmp .= "      //echo \"ERROR: \$errno - \$errstr<br />\\n\";\n";
		$tmp .= "  } else {\n";
		$tmp .= "      fwrite(\$fp, \$syslogmsg);\n";
		$tmp .= "      fclose(\$fp);\n";
		$tmp .= "  }\n";
		$tmp .= "\n";
		$tmp .= "}\n";
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "//\$msg = \"1.begin loop. \".date('r').\"\\n\";\n";
		$tmp .= "//\$fp = fopen('/tmp/phpmonitor.txt', 'a');\n";
		$tmp .= "//fwrite(\$fp, \$msg.\"\\n\");\n";
		$tmp .= "//fclose(\$fp);\n";
		$tmp .= "\n";
		$tmp .= "\$x = 0;\n";
		$tmp .= "while(\$x == 0) {\n";
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "  if(!file_exists('/tmp/phpmonitor.pid')) {\n";
		$tmp .= "    //if the file does not exist then close the program.\n";
		$tmp .= "    echo \"program closing\\n\";\n";
		$tmp .= "    return;\n";
		$tmp .= "  }\n";
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "\n";
		      
		foreach($config['installedpackages']['phpservice']['config'] as $rowhelper) {
			if ($rowhelper['enabled'] != "false") {
				$tmp_php = base64_decode($rowhelper['php']);
				if (strlen($tmp_php) > 0) {
					$tmp .= "// name: ".$rowhelper['name']." \n";
					$tmp .= "// description: ".$rowhelper['description']." \n\n";				      
					$tmp .= base64_decode($rowhelper['php']);
					$tmp .= "\n";
				}
			}

		}
      		
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "  //usleep(100000);  //micro seconds //2 seconds 2000000\n";
		$tmp .= "  sleep(1); //in seconds\n";
		$tmp .= "  //if (\$x > 60){ exit; } //exit after 60 seconds for testing\n";
		$tmp .= "} //emd while\n";
		$tmp .= "\n";
		$tmp .= "\n";
		$tmp .= "?>";
				
		$fout = fopen("/usr/local/pkg/phpservice.php","w");
		fwrite($fout, $tmp);
		unset($tmp);
		fclose($fout);
		          
		conf_mount_ro();

  }
}


function phpservice_sync_package()
{
  
	global $config;   
	phpservice_sync_package_php();

}
    

function phpservice_install_command()
{

	global $config;
	conf_mount_rw();

	if (!is_dir('/usr/local/www/packages/')) {
	   exec("mkdir /usr/local/www/packages/");
	}
	
	if (!is_dir('/usr/local/www/packages/phpservice/')) {
	   exec("mkdir /usr/local/www/packages/phpservice/");
	}

	
	//rename PHP files from .tmp to .php
	exec("cp /tmp/phpservice_php.tmp /usr/local/www/packages/phpservice/phpservice_php.php");
	unlink_if_exists("/tmp/phpservice_php.tmp");
	
	exec("cp /tmp/phpservice_php_edit.tmp /usr/local/www/packages/phpservice/phpservice_php_edit.php");
	unlink_if_exists("/tmp/phpservice_php_edit.tmp");
		
  //write_config();
	  	
	write_rcfile(array(
		"file" => "phpservice.sh",
		"start" => "/usr/local/bin/php /usr/local/pkg/phpservice.php >> /var/log/phpservice.log &",
		"stop" => "rm /tmp/phpmonitor.pid"
		)
	);
		  	       
	phpservice_sync_package();
	//$handle = popen("/usr/local/etc/rc.d/phpservice.sh start", "r");
	//pclose($handle);
	
	//if (pkg_is_service_running('phpservice')) {  
		//documentation purposes
	//}

	conf_mount_ro();

}


function phpservice_deinstall_command()
{

	conf_mount_rw();
	$handle = popen("/usr/local/etc/rc.d/phpservice.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/phpservice.xml");
	unlink_if_exists("/usr/local/pkg/phpservice.inc");
	unlink_if_exists("/usr/local/www/phpservice.inc");
	unlink_if_exists("/usr/local/etc/rc.d/phpservice.sh");	
	conf_mount_ro();

}

?>
