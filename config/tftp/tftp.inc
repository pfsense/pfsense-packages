<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    tftp_inc.php
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


function guid()
{
    if (function_exists('com_create_guid')){
        return com_create_guid();
    }else{
        mt_srand((double)microtime()*10000);//optional for php 4.2.0 and up.
        $charid = strtoupper(md5(uniqid(rand(), true)));
        $hyphen = chr(45);// "-"
        $uuid = chr(123)// "{"
                .substr($charid, 0, 8).$hyphen
                .substr($charid, 8, 4).$hyphen
                .substr($charid,12, 4).$hyphen
                .substr($charid,16, 4).$hyphen
                .substr($charid,20,12)
                .chr(125);// "}"
        return $uuid;
    }
}
//echo guid();


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

function byte_convert( $bytes ) {
   
    if ($bytes<=0)
        return '0 Byte';
   
    $convention=1000; //[1000->10^x|1024->2^x]
    $s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
    $e=floor(log($bytes,$convention));
    return round($bytes/pow($convention,$e),2).' '.$s[$e];
}


function php_sync_package() 
{
  
  //global $config;   
  //sync_package_tftp_files();

  //if (pkg_is_service_running('inetd')) {  
  //    sync_package_tftp();
  //}

}
    

function php_install_command() 
{

	global $config;
	conf_mount_rw();
	config_lock();

	if (!is_dir('/tftpboot')) {
	   // Create the directory
	   exec("mkdir /tftpboot");
	   
	   //Set the directory permissions
	   exec("chmod -R 777 /tftpboot");

	}

	if (!is_dir('/usr/local/www/tftp')) {
	   // Create the directory
	   exec("mkdir /usr/local/www/tftp");
	}

	
	//rename PHP files from .tmp to .php	
	exec("cp /tmp/tftp_files.tmp /usr/local/www/tftp/tftp_files.php");
	unlink_if_exists("/tmp/tftp_files.tmp");
    		
	//prepare inetd.conf for tftp
		$filename = "/etc/inetd.conf";
		$handle = fopen($filename,"rb");
		$contents = fread($handle, filesize($filename));	
		fclose($handle);
		
		$handle = fopen($filename,"w");
		$contents = str_replace("#tftp", "tftp", $contents);
		fwrite($handle, $contents);		
		unset($contents); 	
		fclose($handle);
		unset($filename);


	// add a TFTP DHCP Option to the DHCP Server
		$filename = "/usr/local/www/services_dhcp.php";
		$fout = fopen($filename,"r");		
		$tmp = fread($fout, filesize($filename));
		fclose($fout);
		
		//tftp was not found in the string
		if (strpos($tmp, "tftp") === false) { 
		    $tmpsearch = "\$pconfig['netmask'] = \$config['dhcpd'][\$if]['netmask'];";
		    $tmpreplace = "\$pconfig['netmask'] = \$config['dhcpd'][\$if]['netmask'];\n";
		    $tmpreplace .= "\$pconfig['tftp'] = \$config['dhcpd'][\$if]['options']['tftp-server-name'];";
		    $tmp = str_replace($tmpsearch, $tmpreplace, $tmp);
		    unset($tmpsearch, $tmpreplace);
		    
		    $tmpsearch = "\$config['dhcpd'][\$if]['filename'] = \$_POST['filename'];";
		    $tmpreplace = "\$config['dhcpd'][\$if]['filename'] = \$_POST['filename'];\n";
		    $tmpreplace .= "		\$config['dhcpd'][\$if]['options']['tftp-server-name'] = \$_POST['tftp'];";
		    $tmp = str_replace($tmpsearch, $tmpreplace, $tmp);
		    unset($tmpsearch, $tmpreplace);
		    
		    $tmpsearch = "function show_netboot_config() {";
		    $tmpreplace = "function show_tftp_config() {\n";
		    $tmpreplace .= "	document.getElementById(\"showtftpbox\").innerHTML='';\n";
		    $tmpreplace .= "	aodiv = document.getElementById('showtftp');\n";
		    $tmpreplace .= "	aodiv.style.display = \"block\";\n";
		    $tmpreplace .= "}\n";
		    $tmpreplace .= "\n";
		    $tmpreplace .= "function show_netboot_config() {";
		    $tmp = str_replace($tmpsearch, $tmpreplace, $tmp);
		    unset($tmpsearch, $tmpreplace);
		    
		    $tmpsearch = "<td width=\"22%\" valign=\"top\" class=\"vncell\">Enable Network booting</td>";
		    $tmpreplace .= "";
		    //$tmpreplace = "		  <tr>\n";
		    $tmpreplace .= "<td width=\"22%\" valign=\"top\" class=\"vncell\">TFTP server</td>\n";
		    $tmpreplace .= "			<td width=\"78%\" class=\"vtable\">\n";
		    $tmpreplace .= "				<div id=\"showtftpbox\">\n";
		    $tmpreplace .= "					<input type=\"button\" onClick=\"show_tftp_config()\" value=\"Advanced\"></input> - Show TFTP configuration</a>\n";
		    $tmpreplace .= "				</div>\n";
		    $tmpreplace .= "				<div id=\"showtftp\" style=\"display:none\">\n";
		    $tmpreplace .= "					<input name=\"tftp\" type=\"text\" class=\"formfld\" id=\"tftp\" size=\"20\" value=\"<"."?=htmlspecialchars(\$pconfig['tftp']);?".">\"><br>\n";
		    $tmpreplace .= "				</div>\n";	      
		    $tmpreplace .= "			</td>\n";
		    $tmpreplace .= "			</tr>\n";
		    $tmpreplace .= "\n";
		    $tmpreplace .= "			<tr>\n";
		    $tmpreplace .= "			<td width=\"22%\" valign=\"top\" class=\"vncell\">Enable Network booting</td>";
		    $tmpreplace .= "\n";
		    $tmp = str_replace($tmpsearch, $tmpreplace, $tmp);
		    unset($tmpsearch, $tmpreplace);
		    
		    $fout = fopen($filename,"w");	
		    fwrite($fout, $tmp);
		    unset($tmp);
		    fclose($fout);
		} 
		unset($tmp, $filename);
				
				
				
		$filename = "/etc/inc/services.inc";
		$fout = fopen($filename,"r");		
		$tmp = fread($fout, filesize($filename));
		fclose($fout);
		
		//tftp was not found in the string
		if (strpos($tmp, "tftp") === false) {
		    $tmpsearch = "if (is_array(\$dhcpifconf['ntpserver']) && \$dhcpifconf['ntpserver'][0])";
		    $tmpreplace = "if (isset(\$dhcpifconf['options']['tftp-server-name'])) {\n";
		    $tmpreplace .= "		  	\$dhcpdconf .= \"	option tftp-server-name \\\"\".\$dhcpifconf['options']['tftp-server-name'].\"\\\";\".\"\\n\";\n";
		    $tmpreplace .= "		}\n";
		    $tmpreplace .= "		\n";
		    $tmpreplace .= "		if (is_array(\$dhcpifconf['ntpserver']) && \$dhcpifconf['ntpserver'][0])";
		    $tmp = str_replace($tmpsearch, $tmpreplace, $tmp);
		    unset($tmpsearch, $tmpreplace);
		    
		    $fout = fopen($filename,"w");	
		    fwrite($fout, $tmp);  
		    fclose($fout);
		} 
		unset($tmp, $filename);
			
  	
	// if backup file exists restore it
	    $filename = 'tftp.bak.tgz';
	    	   
	    //extract a specific directory to /usr/local/freeswitch
	    if (file_exists('/tmp/'.$filename)) {
	        system('cd /; tar xvpfz /tmp/tftp.bak.tgz');
	        system('chmod -R 744 /tftpboot/*');			
			unset($filename);
	    }
  	
	write_rcfile(array(
		"file" => "tftp.sh",
		"start" => "/usr/sbin/inetd",
		"stop" => "killall -9 inetd"
		)
	);
	
		  	       
	//php_sync_package();
	$handle = popen("/usr/sbin/inetd", "r");
	pclose($handle);
	
	//if (pkg_is_service_running('inetd')) {  
	//    temp_sync_package();
	//}
      
	//conf_mount_ro();
	//config_unlock();
  
}


function php_deinstall_command() 
{

	//prepare inetd.conf for tftp
		$filename = "/etc/inetd.conf";
		$handle = fopen($filename,"rb");
		$contents = fread($handle, filesize($filename));	
		fclose($handle);
		
		$handle = fopen($filename,"w");
		$contents = str_replace("tftp	dgram", "#tftp	dgram", $contents);
		fwrite($handle, $contents);		
		unset($contents); 	
		fclose($fout);
		unset($filename);
		
	exec("killall -9 inetd");
	
	unlink_if_exists("/usr/local/pkg/tftp.xml");
	unlink_if_exists("/usr/local/pkg/tftp.inc");
	unlink_if_exists("/usr/local/www/tftp/tftp_files.php");
	
	exec("rm -R /tftpboot");
	unlink_if_exists("/usr/local/etc/rc.d/tftp.sh");
	unlink_if_exists("/tmp/pkg_mgr_tftp.log");	
	
}

?>