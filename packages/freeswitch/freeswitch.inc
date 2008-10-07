<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    freeswitch.xml
    Copyright (C) 2008 Mark J Crane
    All rights reserved.
    
    FreeSWITCH (TM)
    http://www.freeswitch.org/
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


function event_socket_create($host, $port, $password) 
{
   $fp = fsockopen($host, $port, $errno, $errdesc) 
     or die("Connection to $host failed");
   socket_set_blocking($fp,false);
   
   if ($fp) {
       while (!feof($fp)) {
          $buffer = fgets($fp, 1024);
          usleep(100); //allow time for reponse
          if (trim($buffer) == "Content-Type: auth/request") {
             fputs($fp, "auth $password\n\n");
             break;
          }
       }
       return $fp;
   }
   else {
       return false;
   }           
}


function event_socket_request($fp, $cmd) 
{
   if ($fp) {    
       fputs($fp, $cmd."\n\n");    
       usleep(100); //allow time for reponse
       
       $response = "";
       $i = 0;
       $contentlength = 0;
       while (!feof($fp)) {
          $buffer = fgets($fp, 4096);
          if ($contentlength > 0) {
             $response .= $buffer;
          }
          
          if ($contentlength == 0) { //if contentlenght is already don't process again
              if (strlen(trim($buffer)) > 0) { //run only if buffer has content
                  $temparray = split(":", trim($buffer));
                  if ($temparray[0] == "Content-Length") {
                     $contentlength = trim($temparray[1]);
                  }
              }
          }
          
          usleep(100); //allow time for reponse
          
          //optional because of script timeout //don't let while loop become endless
          if ($i > 10000) { break; } 
          
          if ($contentlength > 0) { //is contentlength set
              //stop reading if all content has been read.
              if (strlen($response) >= $contentlength) {  
                 break;
              }
          }
          $i++;
       }
       
       return $response;
   }
   else {
     echo "no handle";
   }
}


function event_socket_request_reloadxml()
{
	global $config;

  $password = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'];
  $port = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'];  
  $host = $config['interfaces']['lan']['ipaddr'];

  if (pkg_is_service_running('freeswitch')) 
  {
    $fp = event_socket_create($host, $port, $password);
    $cmd = "api reloadxml";
    $response = event_socket_request($fp, $cmd);
    fclose($fp);
  }
  unset($host, $port, $password);
  	
}
	
 
 
function sync_package_freeswitch_settings()
{
	global $config;
  
  if($config['installedpackages']['freeswitchsettings']['config'] != "") {
  
      conf_mount_rw();
      config_unlock();
	
      foreach($config['installedpackages']['freeswitchsettings']['config'] as $rowhelper) {

      		$fout = fopen("/usr/local/freeswitch/conf/directory/default/default.xml","w");		
          $tmpxml = "<include>\n";
          $tmpxml .= "  <user id=\"default\"> <!--if id is numeric mailbox param is not necessary-->\n";
          $tmpxml .= "    <variables>\n";
          $tmpxml .= "      <!--all variables here will be set on all inbound calls that originate from this user -->\n";
          $tmpxml .= "      <!-- set these to take advantage of a dialplan localized to this user -->\n";
          $tmpxml .= "      <variable name=\"numbering_plan\" value=\"" . $rowhelper['numbering_plan'] . "\"/>\n";
          $tmpxml .= "      <variable name=\"default_gateway\" value=\"" . $rowhelper['default_gateway'] . "\"/>\n";
          $tmpxml .= "      <variable name=\"default_area_code\" value=\"" . $rowhelper['default_area_code'] . "\"/>\n";
          $tmpxml .= "    </variables>\n";
          $tmpxml .= "  </user>\n";
          $tmpxml .= "</include>\n";
          fwrite($fout, $tmpxml);
          unset($tmpxml);
          fclose($fout);
          
          $fout = fopen("/usr/local/freeswitch/conf/autoload_configs/event_socket.conf.xml","w");
        	$tmpxml = "<configuration name=\"event_socket.conf\" description=\"Socket Client\">\n";
        	$tmpxml .= "  <settings>\n";
        	$tmpxml .= "    <param name=\"listen-ip\" value=\"". $config['interfaces']['lan']['ipaddr'] ."\"/>\n";
        	$tmpxml .= "    <param name=\"listen-port\" value=\"" . $rowhelper['event_socket_port'] . "\"/>\n"; 
        	$tmpxml .= "    <param name=\"password\" value=\"" . $rowhelper['event_socket_password'] . "\"/>\n";
        	$tmpxml .= "    <!--<param name=\"apply-inbound-acl\" value=\"lan\"/>-->\n";
        	$tmpxml .= "  </settings>\n";
        	$tmpxml .= "</configuration>";
          fwrite($fout, $tmpxml);
          unset($tmpxml, $event_socket_password);
          fclose($fout);
          
          $fout = fopen("/usr/local/freeswitch/conf/autoload_configs/xml_rpc.conf","w");          
          $tmpxml = "<configuration name=\"xml_rpc.conf\" description=\"XML RPC\">\n";
          $tmpxml .= "  <settings>\n";
          $tmpxml .= "    <!-- The port where you want to run the http service (default 8080) -->\n";
          $tmpxml .= "    <param name=\"http-port\" value=\"" . $rowhelper['xml_rpc_http_port'] . "\"/>\n";
          $tmpxml .= "    <!-- if all 3 of the following params exist all http traffic will require auth -->\n";
          $tmpxml .= "    <param name=\"auth-realm\" value=\"" . $rowhelper['xml_rpc_auth_realm'] . "\"/>\n";
          $tmpxml .= "    <param name=\"auth-user\" value=\"" . $rowhelper['xml_rpc_auth_user'] . "\"/>\n";
          $tmpxml .= "    <param name=\"auth-pass\" value=\"" . $rowhelper['xml_rpc_auth_pass'] . "\"/>\n";
          $tmpxml .= "  </settings>\n";
          $tmpxml .= "</configuration>\n";
          fwrite($fout, $tmpxml);
          unset($tmpxml, $event_socket_password);
          fclose($fout);
          
      }
      
      conf_mount_ro();
      event_socket_request_reloadxml();
  }
}


function sync_package_freeswitch_dialplan()
{
	global $config;
	conf_mount_rw();
	config_unlock();
        
  if(strlen($config['installedpackages']['freeswitchdialplan']['config'][0]['dialplan_default_xml']) == 0) {
      /* dialplan not found in the pfsense config.xml get the default dialplan and save to config.xml. */
    	$filename = "/usr/local/freeswitch/conf/dialplan/default.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));  
      $config['installedpackages']['freeswitchdialplan']['config'][0]['dialplan_default_xml'] = base64_encode($tmpxml);
      unset($filename, $dialplan);
      fclose($fout);
  }
  else {
      /* found the dialplan in the pfsense config.xml save it to default.xml. */
    	$fout = fopen("/usr/local/freeswitch/conf/dialplan/default.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchdialplan']['config'][0]['dialplan_default_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

	conf_mount_ro();
  event_socket_request_reloadxml();

}


function sync_package_freeswitch_extensions()
{
	global $config;
    
  if($config['installedpackages']['freeswitchextensions']['config'] != "") {

    	conf_mount_rw();
    	config_unlock();
	
      /* delete all old extensions to prepare for new ones */ 
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/1*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/2*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/3*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/4*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/5*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/6*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/7*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/8*.xml");
    	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/9*.xml");
  	
      foreach($config['installedpackages']['freeswitchextensions']['config'] as $rowhelper) {

      		$fout = fopen("/usr/local/freeswitch/conf/directory/default/".$rowhelper['extension'].".xml","w");

          $tmpxml = "<include>\n";
          $tmpxml .= "  <user id=\"" . $rowhelper['extension'] . "\" mailbox=\"" . $rowhelper['mailbox'] . "\">\n";
          $tmpxml .= "    <params>\n";
          $tmpxml .= "      <param name=\"password\" value=\"" . $rowhelper['password'] . "\"/>\n";
          $tmpxml .= "      <param name=\"vm-password\" value=\"" . $rowhelper['vm-password'] . "\"/>\n";
          /* Disabled until further testing */
          /* if (strlen($rowhelper['vm-mailto']) > 0) { */
              /* $tmpxml .= "      <param name=\"vm-email-all-messages\" value=\"true\"/>\n"; */
              /* $tmpxml .= "      <param name=\"vm-attach-file\" value=\"true\"/>\n"; */
              /* $tmpxml .= "      <param name=\"vm-mailto\" value=\"" . $rowhelper['vm-mailto'] . "\"/>\n"; */
          /* } */
          /* <field> */
          /* 	<fielddescr>Voicemail Mail To</fielddescr> */
          /* 	<fieldname>vm-mailto</fieldname> */
          /* 	<description>Optional: Enter the email address to send voicemail to.</description> */
          /* 	<type>input</type> */
          /* </field> */		
          $tmpxml .= "    </params>>\n";
          $tmpxml .= "    <variables>\n";
          $tmpxml .= "      <variable name=\"toll_allow\" value=\"domestic,international,local\"/>\n";
          $tmpxml .= "      <variable name=\"accountcode\" value=\"" . $rowhelper['accountcode'] . "\"/>\n";
          $tmpxml .= "      <variable name=\"user_context\" value=\"" . $rowhelper['user_context'] . "\"/>\n";
          if (strlen($rowhelper['effective_caller_id_number']) > 0) {
            $tmpxml .= "      <variable name=\"effective_caller_id_name\" value=\"" . $rowhelper['effective_caller_id_name'] . "\"/>\n";
            $tmpxml .= "      <variable name=\"effective_caller_id_number\" value=\"" . $rowhelper['effective_caller_id_number'] . "\"/>\n";
          }
          if (strlen($rowhelper['outbound_caller_id_number']) > 0) {
            $tmpxml .= "      <variable name=\"outbound_caller_id_name\" value=\"" . $rowhelper['outbound_caller_id_name'] . "\"/>\n";
            $tmpxml .= "      <variable name=\"outbound_caller_id_number\" value=\"" . $rowhelper['outbound_caller_id_number'] . "\"/>\n";
          }
          $tmpxml .= "    </variables>\n";
          $tmpxml .= "  </user>\n";
          $tmpxml .= "</include>\n";
          fwrite($fout, $tmpxml);
          unset($tmpxml);
          fclose($fout);
      }
      
      conf_mount_ro();
      event_socket_request_reloadxml();

  }  
}


function sync_package_freeswitch_gateways()
{
	global $config;
  
  if($config['installedpackages']['freeswitchgateways']['config'] != "") {
  
      conf_mount_rw();
    	config_unlock();
    	
    	/* delete all old gateways to prepare for new ones */ 
    	unlink_if_exists("/usr/local/freeswitch/conf/sip_profiles/external/*.xml");
    	
      foreach($config['installedpackages']['freeswitchgateways']['config'] as $rowhelper) {
      		$fout = fopen("/usr/local/freeswitch/conf/sip_profiles/external/".$rowhelper['gateway'].".xml","w");

          $tmpxml .= "<include>\n";
          $tmpxml .= "      <gateway name=\"" . $rowhelper['gateway'] . "\">\n";
          $tmpxml .= "      <param name=\"username\" value=\"" . $rowhelper['username'] . "\"/>\n";
          $tmpxml .= "      <param name=\"password\" value=\"" . $rowhelper['password'] . "\"/>\n";
          $tmpxml .= "      <param name=\"from-user\" value=\"" . $rowhelper['from-user'] . "\"/>\n";
          $tmpxml .= "      <param name=\"from-domain\" value=\"" . $rowhelper['from-domain'] . "\"/>\n";
          $tmpxml .= "      <param name=\"proxy\" value=\"" . $rowhelper['proxy'] . "\"/>\n";
          $tmpxml .= "      <param name=\"expire-seconds\" value=\"" . $rowhelper['expire-seconds'] . "\"/>\n";
          $tmpxml .= "      <param name=\"register\" value=\"" . $rowhelper['register'] . "\"/>\n";
          $tmpxml .= "      <param name=\"retry_seconds\" value=\"" . $rowhelper['retry_seconds'] . "\"/>\n";
          $tmpxml .= "      <param name=\"extension\" value=\"" . $rowhelper['extension'] . "\"/>\n";
          $tmpxml .= "      <param name=\"context\" value=\"" . $rowhelper['context'] . "\"/>\n";
          $tmpxml .= "    </gateway>\n";
          $tmpxml .= "</include>";
          
          fwrite($fout, $tmpxml);
          unset($tmpxml);
          fclose($fout);
      }
      
      conf_mount_ro();
      event_socket_request_reloadxml();
      
  }
  
}


function sync_package_freeswitch_modules()
{
	global $config;
	conf_mount_rw();
	config_unlock();
        
  if(strlen($config['installedpackages']['freeswitchmodules']['config'][0]['modules_conf_xml']) == 0) {
      /* dialplan not found in the pfsense config.xml get the default dialplan and save to config.xml. */
    	$filename = "/usr/local/freeswitch/conf/autoload_configs/modules.conf.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));  
      $config['installedpackages']['freeswitchmodules']['config'][0]['modules_conf_xml'] = base64_encode($tmpxml);
      unset($filename, $dialplan);
      fclose($fout);
  }
  else {
      /* found the dialplan in the pfsense config.xml save it to default.xml. */
    	$fout = fopen("/usr/local/freeswitch/conf/autoload_configs/modules.conf.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchmodules']['config'][0]['modules_conf_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

	conf_mount_ro();
	event_socket_request_reloadxml();

}


function sync_package_freeswitch_public()
{
	global $config;
	conf_mount_rw();
	config_unlock();
        
  if(strlen($config['installedpackages']['freeswitchpublic']['config'][0]['public_xml']) == 0) {
      /* dialplan_public_xml not found in the pfsense config.xml get the default public.xml and save to config.xml. */
    	$filename = "/usr/local/freeswitch/conf/dialplan/public/00_inbound_did.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));  
      $config['installedpackages']['freeswitchpublic']['config'][0]['public_xml'] = base64_encode($tmpxml);
      unset($filename, $tmpxml);
      fclose($fout);
  }
  else {
      /* found dialplan_public_xml in the pfsense config.xml save it to public.xml. */
    	$fout = fopen("/usr/local/freeswitch/conf/dialplan/public/00_inbound_did.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchpublic']['config'][0]['public_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

	conf_mount_ro();
	event_socket_request_reloadxml();

}


function sync_package_freeswitch_vars()
{
	global $config;
	conf_mount_rw();
	config_unlock();
        
  if(strlen($config['installedpackages']['freeswitchvars']['config'][0]['vars_xml']) == 0) {
      /* dialplan not found in the pfsense config.xml get the default dialplan and save to config.xml. */
    	$filename = "/usr/local/freeswitch/conf/vars.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));  
      $config['installedpackages']['freeswitchvars']['config'][0]['vars_xml'] = base64_encode($tmpxml);
      unset($filename, $dialplan);
      fclose($fout);
  }
  else {
      /* found the dialplan in the pfsense config.xml save it to default.xml. */
    	$fout = fopen("/usr/local/freeswitch/conf/vars.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchvars']['config'][0]['vars_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

	conf_mount_ro();
	event_socket_request_reloadxml();

}

function sync_package_freeswitch_internal()
{
	global $config;
	conf_mount_rw();
	config_unlock();
        
  if(strlen($config['installedpackages']['freeswitchinternal']['config'][0]['internal_xml']) == 0) {
      /* internal_xml not found in the pfsense config.xml get the internal.xml and save to config.xml. */
    	$filename = "/usr/local/freeswitch/conf/sip_profiles/internal.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));  
      $config['installedpackages']['freeswitchinternal']['config'][0]['internal_xml'] = base64_encode($tmpxml);
      unset($filename, $dialplan);
      fclose($fout);
  }
  else {
      /* found the internal_xml in the pfsense config.xml save it to internal.xml. */
    	$fout = fopen("/usr/local/freeswitch/conf/sip_profiles/internal.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchinternal']['config'][0]['internal_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

	conf_mount_ro();
	event_socket_request_reloadxml();

}

function sync_package_freeswitch_external()
{
	global $config;
	conf_mount_rw();
	config_unlock();
        
  if(strlen($config['installedpackages']['freeswitchexternal']['config'][0]['external_xml']) == 0) {
      /* external_xml not found in the pfsense config.xml get the external.xml and save to config.xml. */
    	$filename = "/usr/local/freeswitch/conf/sip_profiles/external.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));  
      $config['installedpackages']['freeswitchexternal']['config'][0]['external_xml'] = base64_encode($tmpxml);
      unset($filename, $dialplan);
      fclose($fout);
  }
  else {
      /* found the external_xml in the pfsense config.xml save it to external.xml. */
    	$fout = fopen("/usr/local/freeswitch/conf/sip_profiles/external.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchexternal']['config'][0]['external_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

	conf_mount_ro();
	event_socket_request_reloadxml();

}
		
function sync_package_freeswitch() 
{
	global $config;   
  sync_package_freeswitch_settings();
  sync_package_freeswitch_dialplan();
	sync_package_freeswitch_extensions();
	sync_package_freeswitch_gateways();
  sync_package_freeswitch_modules();
  sync_package_freeswitch_public();
  sync_package_freeswitch_vars();  
  sync_package_freeswitch_internal();
  sync_package_freeswitch_external();
    
}
    

function freeswitch_php_install_command() 
{

	global $config;
	conf_mount_rw();
	config_lock();
  
  if (!is_dir('/usr/local/www/freeswitch/')) {
	   exec("mkdir /usr/local/www/freeswitch/");
	}
	exec("cp /tmp/freeswitch_status.tmp /usr/local/www/freeswitch/freeswitch_status.php");
	unlink_if_exists("/tmp/freeswitch_status.tmp");
	
	exec("tar zxvf /tmp/freeswitch.tgz -C /usr/local/");
	unlink_if_exists("/tmp/freeswitch.tgz");

	/* set default numbering_plan */
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['numbering_plan']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['numbering_plan'] = "US";
	}
	
	/* set default event_socket_password */
  if(strlen($config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'] = "ClueCon";
	}

	/* set default event_socket_port */
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'] = "8021";
	}
	
	/* set default xml_rpc_http_port */
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_http_port']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_http_port'] = "8787";
	}

	/* set default xml_rpc_auth_realm */
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_realm']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_realm'] = "freeswitch";
	}
  
	/* set default xml_rpc_auth_user */
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_user']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_user'] = "freeswitch";
	}
	
	/* set default xml_rpc_auth_pass */
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_pass']) == 0) {
	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_pass'] = "works";
	}
	
	$numbering_plan = $config['installedpackages']['freeswitchsettings']['config'][0]['numbering_plan'];
	$event_socket_password = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'];
	$event_socket_port = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'];
	$xml_rpc_http_port = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_http_port'];
	$xml_rpc_auth_realm = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_realm'];
	$xml_rpc_auth_user = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_user'];
	$xml_rpc_auth_pass = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_pass'];
        	
	$fout = fopen("/usr/local/freeswitch/conf/autoload_configs/event_socket.conf.xml","w");
	$tmpxml = "<configuration name=\"event_socket.conf\" description=\"Socket Client\">\n";
	$tmpxml .= "  <settings>\n";
	$tmpxml .= "    <param name=\"listen-ip\" value=\"". $config['interfaces']['lan']['ipaddr'] ."\"/>\n";
	$tmpxml .= "    <param name=\"listen-port\" value=\"". $event_socket_port ."\"/>\n";
	$tmpxml .= "    <param name=\"password\" value=\"". $event_socket_password ."\"/>\n";
	$tmpxml .= "    <!--<param name=\"apply-inbound-acl\" value=\"lan\"/>-->\n";
	$tmpxml .= "  </settings>\n";
	$tmpxml .= "</configuration>";
  fwrite($fout, $tmpxml);
  unset($tmpxml);
  fclose($fout);
      
	write_rcfile(array(
		"file" => "freeswitch.sh",
		"start" => "/usr/local/freeswitch/bin/./freeswitch -nc",
		"stop" => "/usr/local/freeswitch/bin/./freeswitch -stop"
		)
	);
	  	       
  sync_package_freeswitch();
  $handle = popen("/usr/local/etc/rc.d/freeswitch.sh start", "r");
  pclose($handle);

	$config['installedpackages']['freeswitchsettings']['config'][0]['freeswitch_version'] = "1.0.1 revision 9759.";
	$config['installedpackages']['freeswitchsettings']['config'][0]['freeswitch_package_version'] = "0.1";       
  conf_mount_ro();
	config_unlock();
  
}


function freeswitch_deinstall_command() 
{
	conf_mount_rw();
	config_lock();
	exec("killall -9 freeswitch");
	unlink_if_exists("/usr/local/pkg/freeswitch.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch.inc");
	unlink_if_exists("/usr/local/pkg/freeswitch_dialplan.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_extensions.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_external.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_gateways.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_internal.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_modules.xml");	
	unlink_if_exists("/usr/local/pkg/freeswitch_public.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_vars.xml");	
	unlink_if_exists("/usr/local/www/freeswitch/freeswitch_status.xml");	
	exec("rm -R /usr/local/freeswitch/");
	exec("rm -R /usr/local/www/freeswitch/");
	unlink_if_exists("/usr/local/etc/rc.d/freeswitch.sh");
	unlink_if_exists("/tmp/freeswitch.tar.gz");
	unlink_if_exists("/tmp/pkg_mgr_FreeSWITCH.log");	
	conf_mount_ro();
	config_unlock();
}




?>