<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
    freeswitch.inc
    Copyright (C) 2008 Mark J Crane
    All rights reserved.

    FreeSWITCH (TM)
    http://www.freeswitch.org/

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

function build_menu() {

	//$script_name_array = split ("/", $_SERVER["SCRIPT_NAME"]);
	//$script_name = $script_name_array[count($script_name_array)-1];
	//echo "script_name: ".$script_name."<br />";
	
	$tab_array = array();
	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/pkg_edit.php?xml=freeswitch.xml&amp;id=0") { $menu_selected = true; }
	$tab_array[] = array(gettext("Settings"), $menu_selected, "/pkg_edit.php?xml=freeswitch.xml&amp;id=0");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_dialplan_includes.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_dialplan.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_dialplan_includes_edit.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_dialplan_includes_details_edit.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Dialplan"), $menu_selected, "/packages/freeswitch/freeswitch_dialplan_includes.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_extensions.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_extensions_edit.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Extensions"), $menu_selected, "/packages/freeswitch/freeswitch_extensions.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_features.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_ivr.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_ivr_edit.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_ivr_options_edit.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_recordings.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_recordings_edit.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Features"), $menu_selected, "/packages/freeswitch/freeswitch_features.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_gateways.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_gateways_edit.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Gateways"), $menu_selected, "/packages/freeswitch/freeswitch_gateways.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_profiles.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_profile_edit.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Profiles"), $menu_selected, "/packages/freeswitch/freeswitch_profiles.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_public_includes.php") { $menu_selected = true; }
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_public.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Public"), $menu_selected, "/packages/freeswitch/freeswitch_public_includes.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_status.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Status"), $menu_selected, "/packages/freeswitch/freeswitch_status.php");
	unset($menu_selected);

	$menu_selected = false;
	if ($_SERVER["SCRIPT_NAME"] == "/packages/freeswitch/freeswitch_vars.php") { $menu_selected = true; }
	$tab_array[] = array(gettext("Vars"), $menu_selected, "/packages/freeswitch/freeswitch_vars.php");
	unset($menu_selected);
	
	return $tab_array;
}


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


function event_socket_create($host, $port, $password)
{
    //$host has been deprecated

    //build the interface list
    $i = 0; $ifdescrs = array('wan' => 'WAN', 'lan' => 'LAN');
		for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
			$ifdescrs['opt' . $j] = $config['interfaces']['opt' . $j]['descr'];
		}
		
		//get the interface ip addresses and try to connect to them
		foreach ($ifdescrs as $ifdescr => $ifname){
        $ifinfo = get_interface_info($ifdescr);
        $interface_ip_address = $ifinfo['ipaddr'];

        if (strlen($interface_ip_address) > 0) {

           $fp = fsockopen($interface_ip_address, $port, $errno, $errdesc, 3);
           socket_set_blocking($fp,false);

           if (!$fp) {
               //connection failed continue through the loop testing other addresses
               //invalid handle
           }
           else {
               //connected to the socket return the handle

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

        } //end if interface_ip_address
		} //end foreach
} //end function


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


function event_socket_request_cmd($cmd)
{
  global $config;
  $password = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'];
  $port = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'];
  $host = $config['interfaces']['lan']['ipaddr'];

  if (pkg_is_service_running('freeswitch')) {
    $fp = event_socket_create($host, $port, $password);
    $response = event_socket_request($fp, $cmd);
    fclose($fp);
  }
  unset($host, $port, $password);
  	
}

function byte_convert( $bytes ) {

    if ($bytes<=0)
        return '0 Byte';

    $convention=1000; //[1000->10^x|1024->2^x]
    $s=array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB');
    $e=floor(log($bytes,$convention));
    return round($bytes/pow($convention,$e),2).' '.$s[$e];
}

function lan_sip_profile()
{
	global $config;
	//create the LAN profile if it doesn't exist
	if (!file_exists('/usr/local/freeswitch/conf/sip_profiles/lan.xml')) {
		$lan_ip = $config['interfaces']['lan']['ipaddr'];
		if (strlen($lan_ip) > 0) {
			exec("cp /usr/local/freeswitch/conf/sip_profiles/internal.xml /usr/local/freeswitch/conf/sip_profiles/lan.xml");
			
			$filename = "/usr/local/freeswitch/conf/sip_profiles/lan.xml";
			$handle = fopen($filename,"rb");
			$contents = fread($handle, filesize($filename));	
			fclose($handle);

			$handle = fopen($filename,"w");
			$contents = str_replace("<profile name=\"internal\">", "<profile name=\"lan\">", $contents);
			$contents = str_replace("<alias name=\"default\"/>", "", $contents);
			$contents = str_replace("<X-PRE-PROCESS cmd=\"include\" data=\"internal/*.xml\"/>", "<X-PRE-PROCESS cmd=\"include\" data=\"lan/*.xml\"/>", $contents);
			$contents = str_replace("<param name=\"rtp-ip\" value=\"\$\${local_ip_v4}\"/>", "<param name=\"rtp-ip\" value=\"".$lan_ip."\"/>", $contents);
			$contents = str_replace("<param name=\"sip-ip\" value=\"\$\${local_ip_v4}\"/>", "<param name=\"sip-ip\" value=\"".$lan_ip."\"/>", $contents);
			fwrite($handle, $contents);		
			unset($contents); 	
			fclose($handle);
			unset($filename);
		}
	}
	
}

function ListFiles($dir) {

    if($dh = opendir($dir)) {

        $files = Array();
        $inner_files = Array();

        while($file = readdir($dh)) {
            if($file != "." && $file != ".." && $file[0] != '.') {
                if(is_dir($dir . "/" . $file)) {
                    //$inner_files = ListFiles($dir . "/" . $file); //recursive
                    if(is_array($inner_files)) $files = array_merge($files, $inner_files); 
                } else {
                    array_push($files, $file);
					//array_push($files, $dir . "/" . $file);
                }
            }
        }

        closedir($dh);
        return $files;
    }
}

function  recording_js()
{

    global $config;
    $admin_pin = $config['installedpackages']['freeswitchsettings']['config'][0]['admin_pin'];

    $fout = fopen("/usr/local/freeswitch/scripts/recordings.js","w");
    $tmp = "  var pin = \"".$admin_pin."\";\n";
    $tmp .= "  //var pin = \"\"; //don't require a pin\n";
    $tmp .= "    //if you choose not to require a pin then then you may want to add a dialplan condition for a specific caller id\n";
    $tmp .= "\n";
    $tmp .= "  var digitmaxlength = 0;\n";
    $tmp .= "  var timeoutpin = 7500;\n";
    $tmp .= "  var timeouttransfer = 7500;\n";
    $tmp .= " var objdate = new Date();\n";
    $tmp .= "\n";
    $tmp .= " var adjusthours = 0; //Adjust Server time that is set to GMT 7 hours\n";
    $tmp .= " var adjustoperator = \"-\"; //+ or -\n";
    $tmp .= "\n";
    $tmp .= " if (adjustoperator == \"-\") {\n";
    $tmp .= "   var objdate2 = new Date(objdate.getFullYear(),objdate.getMonth(),objdate.getDate(),(objdate.getHours() - adjusthours),objdate.getMinutes(),objdate.getSeconds());\n";
    $tmp .= " }\n";
    $tmp .= " if (adjustoperator == \"+\") {\n";
    $tmp .= "   var objdate2 = new Date(objdate.getFullYear(),objdate.getMonth(),objdate.getDate(),(objdate.getHours() + adjusthours),objdate.getMinutes(),objdate.getSeconds());\n";
    $tmp .= " }\n";
    $tmp .= "\n";
    $tmp .= " var Hours = objdate2.getHours();\n";
    $tmp .= " var Mins = objdate2.getMinutes();\n";
    $tmp .= " var Seconds = objdate2.getSeconds();\n";
    $tmp .= " var Month = objdate2.getMonth() + 1;\n";
    $tmp .= " var Date = objdate2.getDate();\n";
    $tmp .= " var Year = objdate2.getYear()\n";
    $tmp .= " var Day = objdate2.getDay()+1;\n";
    $tmp .= " var exit = false;\n";
    $tmp .= "\n";
    $tmp .= "\n";
    $tmp .= "  function mycb( session, type, data, arg ) {\n";
    $tmp .= "     if ( type == \"dtmf\" ) {\n";
    $tmp .= "       //console_log( \"info\", \"digit: \"+data.digit+\"\\n\" );\n";
    $tmp .= "       if ( data.digit == \"#\" ) {\n";
    $tmp .= "         //console_log( \"info\", \"detected pound sign.\\n\" );\n";
    $tmp .= "         return( true );\n";
    $tmp .= "       }\n";
    $tmp .= "       dtmf.digits += data.digit;\n";
    $tmp .= "\n";
    $tmp .= "       if ( dtmf.digits.length < digitmaxlength ) {\n";
    $tmp .= "         return( true );\n";
    $tmp .= "       }\n";
    $tmp .= "     }\n";
    $tmp .= "     return( false );\n";
    $tmp .= "  }\n";
    $tmp .= "\n";
    $tmp .= "  //console_log( \"info\", \"Recording Request\\n\" );\n";
    $tmp .= "\n";
    $tmp .= "  var dtmf = new Object( );\n";
    $tmp .= "  dtmf.digits = \"\";\n";
    $tmp .= "\n";
    $tmp .= "  if ( session.ready( ) ) {\n";
    $tmp .= "      session.answer( );\n";
    $tmp .= "\n";

    $tmp .= "\n";
    $tmp .= "  if (pin.length > 0) {\n";
    //$tmp .= "      session.execute(\"set\", \"tts_engine=flite\");\n";
    //$tmp .= "      session.execute(\"set\", \"tts_voice=kal\");\n";
    //$tmp .= "      session.execute(\"speak\", \"Please enter your pin number now.\");\n";
    $tmp .= "      digitmaxlength = 6;\n";
    $tmp .= "      session.execute(\"set\", \"playback_terminators=#\");\n";
    $tmp .= "      session.streamFile( \"/usr/local/freeswitch/sounds/custom/8000/please_enter_the_pin_number.wav\", mycb, \"dtmf\");\n";
    $tmp .= "      session.collectInput( mycb, dtmf, timeoutpin );\n";
    $tmp .= "  }\n";
    $tmp .= "\n";
    $tmp .= "  if (dtmf.digits == pin || pin.length == 0) {\n";
    //$tmp .= "      //console_log( \"info\", \"Recordings pin is correct\\n\" );\n";
    //$tmp .= "      session.execute(\"set\", \"tts_engine=flite\");\n";
    //$tmp .= "      session.execute(\"set\", \"tts_voice=kal\");\n";
    //$tmp .= "      session.execute(\"speak\", \"Begin recording.\");\n";
    $tmp .= "      session.streamFile( \"/usr/local/freeswitch/sounds/custom/8000/begin_recording.wav\", mycb, \"dtmf\");\n";
    $tmp .= "      session.execute(\"set\", \"playback_terminators=#\");\n";
    $tmp .= "      session.execute(\"record\", \"/usr/local/freeswitch/recordings/temp\"+Year+Month+Day+Hours+Mins+Seconds+\".wav 180 200\");\n";
    $tmp .= "  }\n";
    $tmp .= "  else {\n";
    $tmp .= "      console_log( \"info\", \"Pin: \" + dtmf.digits + \" is incorrect\\n\" );\n";
    //$tmp .= "      session.execute(\"set\", \"tts_engine=flite\");\n";
    //$tmp .= "      session.execute(\"set\", \"tts_voice=kal\");\n";
    //$tmp .= "      session.execute(\"speak\", \"Your pin number is incorect, goodbye.\");\n";
    $tmp .= "      session.streamFile( \"/usr/local/freeswitch/sounds/custom/8000/your_pin_number_is_incorect_goodbye.wav\", mycb, \"dtmf\");\n";
    $tmp .= "  }\n";
    $tmp .= "  session.hangup();\n";
    $tmp .= "\n";
    $tmp .= " }";
    fwrite($fout, $tmp);
    unset($tmp);
    fclose($fout);

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
		$tmpxml .= "    <param name=\"listen-ip\" value=\"0.0.0.0\"/>\n";
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
		unset($tmpxml);
		fclose($fout);

		recording_js();

		//shout.conf.xml
		$fout = fopen("/usr/local/freeswitch/conf/autoload_configs/shout.conf.xml","w");
		$tmpxml = "<configuration name=\"shout.conf\" description=\"mod shout config\">\n";
		$tmpxml .= "  <settings>\n";
		$tmpxml .= "    <!-- Don't change these unless you are insane -->\n";
		$tmpxml .= "    <param name=\"decoder\" value=\"" . $rowhelper['mod_shout_decoder'] . "\"/>\n";
		$tmpxml .= "    <param name=\"volume\" value=\"" . $rowhelper['mod_shout_volume'] . "\"/>\n";
		$tmpxml .= "    <!--<param name=\"outscale\" value=\"8192\"/>-->\n";
		$tmpxml .= "  </settings>\n";
		$tmpxml .= "</configuration>";
		fwrite($fout, $tmpxml);
		unset($tmpxml);
		fclose($fout);
		
		//config.js
		$admin_pin = $config['installedpackages']['freeswitchsettings']['config'][0]['admin_pin'];
		$fout = fopen("/usr/local/freeswitch/scripts/config.js","w");
		$tmp = "//javascript include\n\n";
		$tmp .= "var admin_pin = \"" . $admin_pin . "\";\n";
		fwrite($fout, $tmp);
		unset($tmp);
		fclose($fout);

      }

      conf_mount_ro();
      $cmd = "api reloadxml";
      //event_socket_request_cmd($cmd);
      unset($cmd);
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
  $cmd = "api reloadxml";
  //event_socket_request_cmd($cmd);
  unset($cmd);

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
          if (strlen($rowhelper['cidr']) == 0) {
          	$tmpxml .= "  <user id=\"" . $rowhelper['extension'] . "\" mailbox=\"" . $rowhelper['mailbox'] . "\">\n";
          }
          else {
          	$tmpxml .= "  <user id=\"" . $rowhelper['extension'] . "\" mailbox=\"" . $rowhelper['mailbox'] . "\" cidr=\"" . $rowhelper['cidr'] . "\">\n";
          }
          $tmpxml .= "    <params>\n";
          $tmpxml .= "      <param name=\"password\" value=\"" . $rowhelper['password'] . "\"/>\n";
          $tmpxml .= "      <param name=\"vm-password\" value=\"" . $rowhelper['vm-password'] . "\"/>\n";
          if (strlen($rowhelper['vm-mailto']) > 0) {
              $tmpxml .= "      <param name=\"vm-email-all-messages\" value=\"true\"/>\n";

              switch ($rowhelper['vm-attach-file']) {
              case "true":
              		$tmpxml .= "      <param name=\"vm-attach-file\" value=\"true\"/>\n";
              		break;
              case "false":
              		$tmpxml .= "      <param name=\"vm-attach-file\" value=\"false\"/>\n";
              		break;
              default:
              		$tmpxml .= "      <param name=\"vm-attach-file\" value=\"true\"/>\n";
              }

              $tmpxml .= "      <param name=\"vm-mailto\" value=\"" . $rowhelper['vm-mailto'] . "\"/>\n";
          }
          if (strlen($rowhelper['auth-acl']) > 0) {
          	$tmpxml .= "      <param name=\"auth-acl\" value=\"" . $rowhelper['auth-acl'] . "\"/>\n";
          }          	
          $tmpxml .= "    </params>\n";
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
      $cmd = "api reloadxml";
      //event_socket_request_cmd($cmd);
      unset($cmd);

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

          if ($rowhelper['enabled'] != "false") {
		          $fout = fopen("/usr/local/freeswitch/conf/sip_profiles/external/".$rowhelper['gateway'].".xml","w");
		
		          $tmpxml .= "<include>\n";
		          $tmpxml .= "    <gateway name=\"" . $rowhelper['gateway'] . "\">\n";
		          if (strlen($rowhelper['username']) > 0) {
		            $tmpxml .= "      <param name=\"username\" value=\"" . $rowhelper['username'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['password']) > 0) {
		            $tmpxml .= "      <param name=\"password\" value=\"" . $rowhelper['password'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['realm']) > 0) {
		            $tmpxml .= "      <param name=\"realm\" value=\"" . $rowhelper['realm'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['from-user']) > 0) {
		            $tmpxml .= "      <param name=\"from-user\" value=\"" . $rowhelper['from-user'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['from-domain']) > 0) {
		            $tmpxml .= "      <param name=\"from-domain\" value=\"" . $rowhelper['from-domain'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['proxy']) > 0) {
		            $tmpxml .= "      <param name=\"proxy\" value=\"" . $rowhelper['proxy'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['expire-seconds']) > 0) {
		            $tmpxml .= "      <param name=\"expire-seconds\" value=\"" . $rowhelper['expire-seconds'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['register']) > 0) {
		            $tmpxml .= "      <param name=\"register\" value=\"" . $rowhelper['register'] . "\"/>\n";
		          }
				  
		          if (strlen($rowhelper['register-transport']) > 0) {
					switch ($rowhelper['register-transport']) {
					case "udp":
						$tmpxml .= "      <param name=\"register-transport\" value=\"udp\"/>\n";
						break;
					case "tcp":
						$tmpxml .= "      <param name=\"register-transport\" value=\"tcp\"/>\n";
						break;
					case "tls":
						$tmpxml .= "      <param name=\"register-transport\" value=\"tls\"/>\n";
						$tmpxml .= "      <param name=\"contact-params\" value=\"transport=tls\"/>\n";
						break;
					default:
						$tmpxml .= "      <param name=\"register-transport\" value=\"" . $rowhelper['register-transport'] . "\"/>\n";
					}
				  }

		          if (strlen($rowhelper['retry-seconds']) > 0) {
		            $tmpxml .= "      <param name=\"retry-seconds\" value=\"" . $rowhelper['retry-seconds'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['extension']) > 0) {
		            $tmpxml .= "      <param name=\"extension\" value=\"" . $rowhelper['extension'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['context']) > 0) {
		            $tmpxml .= "      <param name=\"context\" value=\"" . $rowhelper['context'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['caller-id-in-from']) > 0) {
		            $tmpxml .= "      <param name=\"context\" value=\"" . $rowhelper['context'] . "\"/>\n";
		          }
		          if (strlen($rowhelper['supress-cng']) > 0) {
		            $tmpxml .= "      <param name=\"context\" value=\"" . $rowhelper['context'] . "\"/>\n";
		          }
		
		          $tmpxml .= "    </gateway>\n";
		          $tmpxml .= "</include>";
		
		          fwrite($fout, $tmpxml);
		          unset($tmpxml);
		          fclose($fout);
          }		

      }

      conf_mount_ro();
      $cmd = "api sofia profile external restart reloadxml";
      //event_socket_request_cmd($cmd);
      unset($cmd);

  }

}


function sync_package_freeswitch_modules()
{

  global $config;
  conf_mount_rw();
  config_unlock();

  foreach($config['installedpackages']['freeswitchmodules']['config'] as $rowhelper) {
  		$fout = fopen("/usr/local/freeswitch/conf/autoload_configs/modules.conf.xml","w");

      $tmpxml ="";
      $tmpxml .= "<configuration name=\"modules.conf\" description=\"Modules\">\n";
      $tmpxml .= "  <modules>\n";
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Loggers (I'd load these first) -->\n";
      if ($rowhelper['mod_console'] == "enable"){   $tmpxml .= "    <load module=\"mod_console\"/>\n"; }
      if ($rowhelper['mod_logfile'] == "enable"){   $tmpxml .= "    <load module=\"mod_logfile\"/>\n"; }
      if ($rowhelper['mod_syslog'] == "enable"){    $tmpxml .= "    <load module=\"mod_syslog\"/>\n"; }
      $tmpxml .= "\n";
      if ($rowhelper['mod_yaml'] == "enable"){      $tmpxml .= "    <load module=\"mod_yaml\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Multi-Faceted -->\n";
      $tmpxml .= "    <!-- mod_enum is a dialplan interface, an application interface and an api command interface -->\n";
      if ($rowhelper['mod_enum'] == "enable"){      $tmpxml .= "    <load module=\"mod_enum\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- XML Interfaces -->\n";
      if ($rowhelper['mod_xml_rpc'] == "enable"){   $tmpxml .= "    <load module=\"mod_xml_rpc\"/>\n"; }
      if ($rowhelper['mod_xml_curl'] == "enable"){  $tmpxml .= "    <load module=\"mod_xml_curl\"/>\n"; }
      if ($rowhelper['mod_xml_cdr'] == "enable"){   $tmpxml .= "    <load module=\"mod_xml_cdr\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Event Handlers -->\n";
      if ($rowhelper['mod_cdr_csv'] == "enable"){   $tmpxml .= "    <load module=\"mod_cdr_csv\"/>\n"; }
      if ($rowhelper['mod_event_multicast'] == "enable"){ $tmpxml .= "    <load module=\"mod_event_multicast\"/>\n"; }
      if ($rowhelper['mod_event_socket'] == "enable"){ $tmpxml .= "    <load module=\"mod_event_socket\"/>\n"; }
      if ($rowhelper['mod_zeroconf'] == "enable"){  $tmpxml .= "    <load module=\"mod_zeroconf\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Directory Interfaces -->\n";
      if ($rowhelper['mod_ldap'] == "enable"){      $tmpxml .= "    <load module=\"mod_ldap\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Endpoints -->\n";
      if ($rowhelper['mod_dingaling'] == "enable"){ $tmpxml .= "    <load module=\"mod_dingaling\"/>\n"; }
      if ($rowhelper['mod_iax'] == "enable"){       $tmpxml .= "    <load module=\"mod_iax\"/>\n"; }
      if ($rowhelper['mod_portaudio'] == "enable"){ $tmpxml .= "    <load module=\"mod_portaudio\"/>\n"; }
      if ($rowhelper['mod_alsa'] == "enable"){      $tmpxml .= "    <load module=\"mod_alsa\"/>\n"; }
      if ($rowhelper['mod_sofia'] == "enable"){     $tmpxml .= "    <load module=\"mod_sofia\"/>\n"; }
      if ($rowhelper['mod_loopback'] == "enable"){  $tmpxml .= "    <load module=\"mod_loopback\"/>\n"; }
      if ($rowhelper['mod_wanpipe'] == "enable"){   $tmpxml .= "    <load module=\"mod_wanpipe\"/>\n"; }
      if ($rowhelper['mod_woomera'] == "enable"){   $tmpxml .= "    <load module=\"mod_woomera\"/>\n"; }
      if ($rowhelper['mod_openzap'] == "enable"){   $tmpxml .= "    <load module=\"mod_openzap\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Applications -->\n";
      if ($rowhelper['mod_commands'] == "enable"){  $tmpxml .= "    <load module=\"mod_commands\"/>\n"; }
      if ($rowhelper['mod_conference'] == "enable"){ $tmpxml .= "    <load module=\"mod_conference\"/>\n"; }
      if ($rowhelper['mod_dptools'] == "enable"){   $tmpxml .= "    <load module=\"mod_dptools\"/>\n"; }
      if ($rowhelper['mod_expr'] == "enable"){      $tmpxml .= "    <load module=\"mod_expr\"/>\n"; }
      if ($rowhelper['mod_fax'] == "enable"){      $tmpxml .= "    <load module=\"mod_fax\"/>\n"; }
      if ($rowhelper['mod_fifo'] == "enable"){      $tmpxml .= "    <load module=\"mod_fifo\"/>\n"; }
      if ($rowhelper['mod_voicemail'] == "enable"){ $tmpxml .= "    <load module=\"mod_voicemail\"/>\n"; }
      if ($rowhelper['mod_limit'] == "enable"){     $tmpxml .= "    <load module=\"mod_limit\"/>\n"; }
      if ($rowhelper['mod_esf'] == "enable"){       $tmpxml .= "    <load module=\"mod_esf\"/>\n"; }
      if ($rowhelper['mod_fsv'] == "enable"){       $tmpxml .= "    <load module=\"mod_fsv\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- SNOM Module -->\n";
      if ($rowhelper['mod_snom'] == "enable"){      $tmpxml .= "    <load module=\"mod_snom\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Dialplan Interfaces -->\n";
      if ($rowhelper['mod_dialplan_directory'] == "enable"){ $tmpxml .= "    <load module=\"mod_dialplan_directory\"/>\n"; }
      if ($rowhelper['mod_dialplan_xml'] == "enable"){ $tmpxml .= "    <load module=\"mod_dialplan_xml\"/>\n"; }
      if ($rowhelper['mod_dialplan_asterisk'] == "enable"){ $tmpxml .= "    <load module=\"mod_dialplan_asterisk\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Codec Interfaces -->\n";
      if ($rowhelper['mod_voipcodecs'] == "enable"){ $tmpxml .= "    <load module=\"mod_voipcodecs\"/>\n"; }
      if ($rowhelper['mod_g723_1'] == "enable"){    $tmpxml .= "    <load module=\"mod_g723_1\"/>\n"; }
      if ($rowhelper['mod_g729'] == "enable"){      $tmpxml .= "    <load module=\"mod_g729\"/>\n"; }
      if ($rowhelper['mod_amr'] == "enable"){       $tmpxml .= "    <load module=\"mod_amr\"/>\n"; }
      if ($rowhelper['mod_ilbc'] == "enable"){      $tmpxml .= "    <load module=\"mod_ilbc\"/>\n"; }
      if ($rowhelper['mod_speex'] == "enable"){     $tmpxml .= "    <load module=\"mod_speex\"/>\n"; }
      if ($rowhelper['mod_siren'] == "enable"){     $tmpxml .= "    <load module=\"mod_siren\"/>\n"; }
      if ($rowhelper['mod_celt'] == "enable"){     $tmpxml .= "    <load module=\"mod_celt\"/>\n"; }
      if ($rowhelper['mod_h26x'] == "enable"){      $tmpxml .= "    <load module=\"mod_h26x\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- File Format Interfaces -->\n";
      if ($rowhelper['mod_sndfile'] == "enable"){   $tmpxml .= "    <load module=\"mod_sndfile\"/>\n"; }
      if ($rowhelper['mod_native_file'] == "enable"){ $tmpxml .= "    <load module=\"mod_native_file\"/>\n"; }
      $tmpxml .= "    <!--For icecast/mp3 streams/files-->\n";
      if ($rowhelper['mod_shout'] == "enable"){     $tmpxml .= "    <load module=\"mod_shout\"/>\n"; }
      $tmpxml .= "    <!--For local streams (play all the files in a directory)-->\n";
      if ($rowhelper['mod_local_stream'] == "enable"){ $tmpxml .= "    <load module=\"mod_local_stream\"/>\n"; }
      if ($rowhelper['mod_tone_stream'] == "enable"){ $tmpxml .= "    <load module=\"mod_tone_stream\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Timers -->\n";
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Languages -->\n";
      if ($rowhelper['mod_spidermonkey'] == "enable"){ $tmpxml .= "    <load module=\"mod_spidermonkey\"/>\n"; }
      if ($rowhelper['mod_perl'] == "enable"){      $tmpxml .= "    <load module=\"mod_perl\"/>\n"; }
      if ($rowhelper['mod_python'] == "enable"){    $tmpxml .= "    <load module=\"mod_python\"/>\n"; }
      if ($rowhelper['mod_java'] == "enable"){      $tmpxml .= "    <load module=\"mod_java\"/>\n"; }
      if ($rowhelper['mod_lua'] == "enable"){       $tmpxml .= "    <load module=\"mod_lua\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- ASR /TTS -->\n";
      if ($rowhelper['mod_flite'] == "enable"){     $tmpxml .= "    <load module=\"mod_flite\"/>\n"; }
      if ($rowhelper['mod_pocketsphinx'] == "enable"){ $tmpxml .= "    <load module=\"mod_pocketsphinx\"/>\n"; }
      if ($rowhelper['mod_cepstral'] == "enable"){  $tmpxml .= "    <load module=\"mod_cepstral\"/>\n"; }
      if ($rowhelper['mod_openmrcp'] == "enable"){  $tmpxml .= "    <load module=\"mod_openmrcp\"/>\n"; }
      if ($rowhelper['mod_rss'] == "enable"){       $tmpxml .= "    <load module=\"mod_rss\"/>\n"; }
      $tmpxml .= "\n";
      $tmpxml .= "    <!-- Say -->\n";
      if ($rowhelper['mod_say_en'] == "enable"){    $tmpxml .= "    <load module=\"mod_say_en\"/>\n"; }
      if ($rowhelper['mod_say_zh'] == "enable"){    $tmpxml .= "    <load module=\"mod_say_zh\"/>\n"; }
      $tmpxml .= "  </modules>\n";
      $tmpxml .= "</configuration>";

      fwrite($fout, $tmpxml);
      unset($tmpxml);
      fclose($fout);
  }

  conf_mount_ro();
  $cmd = "api reloadxml";
  //event_socket_request_cmd($cmd);
  unset($cmd);

}


function sync_package_freeswitch_public()
{

  global $config;
  conf_mount_rw();
  config_unlock();

  if(strlen($config['installedpackages']['freeswitchpublic']['config'][0]['public_xml']) == 0) {
      /* dialplan_public_xml not found in the pfsense config.xml get the default public.xml and save to config.xml. */
      $filename = "/usr/local/freeswitch/conf/dialplan/public.xml";
      $fout = fopen($filename,"r");
      $tmpxml = fread($fout, filesize($filename));
      $tmpxml = str_replace("<anti-action application=\"export\" data=\"domain_name=\${sip_req_host}\"/>", "<!--<anti-action application=\"export\" data=\"domain_name=\${sip_req_host}\"/>-->", $tmpxml);
      $config['installedpackages']['freeswitchpublic']['config'][0]['public_xml'] = base64_encode($tmpxml);
      unset($filename, $tmpxml);
      fclose($fout);
  }
  else {
      /* found dialplan_public_xml in the pfsense config.xml save it to public.xml. */
      $fout = fopen("/usr/local/freeswitch/conf/dialplan/public.xml","w");
      $tmpxml = $config['installedpackages']['freeswitchpublic']['config'][0]['public_xml'];
      fwrite($fout, base64_decode($tmpxml));
      fclose($fout);
      unset($tmpxml);
  }

  conf_mount_ro();
  //$cmd = "api reloadxml";
  //event_socket_request_cmd($cmd);
  unset($cmd);

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
  $cmd = "api reloadxml";
  //event_socket_request_cmd($cmd);
  unset($cmd);

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
  $cmd = "api reloadxml";
  //event_socket_request_cmd($cmd);
  unset($cmd);

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
  $cmd = "api reloadxml";
  //event_socket_request_cmd($cmd);
  unset($cmd);

}


function get_recording_filename($id)
{
  global $config;
  $a_recordings = &$config['installedpackages']['freeswitchrecordings']['config'];
    if (count($a_recordings) > 0) {
      global $config;
      foreach($a_recordings as $rowhelper) {
        if ($rowhelper['recordingid'] == $id) {
            return $rowhelper['filename'];
        }
    }
  }
}


function sync_package_freeswitch_ivr()
{

  global $config;
  conf_mount_rw();
  config_lock();

  $a_ivr = &$config['installedpackages']['freeswitchivr']['config'];
  if (count($a_ivr) > 0) {
	  foreach($a_ivr as $rowhelper) {
	  	
	  	/*
	    $rowhelper['ivrid']
	    $rowhelper['ivrextension']
	    $rowhelper['ivrname']
	    $rowhelper['recordingid']
	    $rowhelper['ivrtimeout']
	    $rowhelper['ivrcontext']
	    $rowhelper['ivrconditionjs']
	    $rowhelper['ivrdescr']
	    */
	
	    $a_dialplan_includes          	= &$config['installedpackages']['freeswitchdialplanincludes']['config'];
	    $a_dialplan_include_details 	= &$config['installedpackages']['freeswitchdialplanincludedetails']['config'];

	
	    //add the IVR to the dialplan
	    if (strlen($rowhelper['ivrid']) > 0) {
	      $action = 'add'; //set default action to add
	      $i = 0;
	      if (count($a_dialplan_includes) > 0) {
		      foreach($a_dialplan_includes as $row) {
		
		          //$row['dialplanincludeid'];
		          //$row['extensionname'];
		          //$row['context'];
		          //$row['enabled'];
		          //echo "if (".$row['opt1name']." == \"ivrid\" && ".$row['opt1value']." == ".$rowhelper['ivrid'].") {\n";
		
		          if ($row['opt1name'] == "ivrid" && $row['opt1value'] == $rowhelper['ivrid']) {
		              //update
		              $action = 'update';
		              $dialplanincludeid = $row['dialplanincludeid'];
		              $extensionname = $row['extensionname'];
		              $order = $row['order'];
		              $context = $row['context'];
		              $enabled = $row['enabled'];
		              $descr = $row['descr'];
		              $opt1name = $row['opt1name'];
		              $opt1value = $row['opt1value'];
		              $id = $i;
		              //echo "update".$i."<br />\n";
		
		          }
		          $i++;
		
		      }
	      }
	
	
	      $ent = array();
	      if ($action == 'add') {
	
	          $dialplanincludeid = guid();
	          $ent['dialplanincludeid'] = $dialplanincludeid;
	      		$ent['extensionname'] = $rowhelper['ivrextension'];
	      		$ent['order'] = '9001'; //if update use the existing order number and extension name and desc
	      		$ent['context'] = $rowhelper['ivrcontext'];
	      		$ent['enabled'] = 'true';			
	      		$ent['descr'] = 'IVR';
	      		$ent['opt1name'] = 'ivrid';
	      		$ent['opt1value'] = $rowhelper['ivrid'];
	
	          //add to the config  		
	      		$a_dialplan_includes[] = $ent;
	      		unset($ent);      		
	      		
	          $ent = array();
	          $ent['dialplanincludeid'] = $dialplanincludeid;
	          $ent['tag'] = 'condition'; //condition, action, antiaction
	          $ent['fieldtype'] = 'destination_number';
	          $ent['fielddata'] = '^'.$rowhelper['ivrextension'].'$';
	          $a_dialplan_include_details[] = $ent;
	          unset($ent);
	      	
	      	  $ivrid = str_replace(array("{", "}"), "", $rowhelper['ivrid']);
	      	  	
	          $ent = array();
	          $ent['dialplanincludeid'] = $dialplanincludeid;
	          $ent['tag'] = 'action'; //condition, action, antiaction
	          $ent['fieldtype'] = 'javascript';
	          $ent['fielddata'] = 'ivr_'.$ivrid.'.js';
	          $a_dialplan_include_details[] = $ent;
	          unset($ent);
	      		
	      	  unset($ivrid);
	      		 		
	  		}
	      if ($action == 'update') {
	
	          $ent['dialplanincludeid'] = $dialplanincludeid;
	      		$ent['extensionname'] = $rowhelper['ivrextension'];
	      		$ent['order'] = $order;
	      		$ent['context'] = $context;
	      		$ent['enabled'] = $enabled;		
	      		$ent['descr'] = $descr;
	      		$ent['opt1name'] = $opt1name;
	      		$ent['opt1value'] = $opt1value;
	      		
	          //update the config
	          $a_dialplan_includes[$id] = $ent;
	
	      		unset($ent);	      		
	      		unset($extensionname);
	      		unset($order);
	      		unset($context);
	      		unset($enabled);
	      		unset($descr);
	      		unset($opt1name);
	      		unset($opt1value);
	      		unset($id);                    		
	  		}
	  		write_config();
	   		  		
	      sync_package_freeswitch_dialplan_includes();		
	      unset($dialplanincludeid);
	  		
	    } //end if strlen ivrid; add the IVR to the dialplan
	
	    // Build the IVR javascript
	    $recording_action_filename = get_recording_filename($rowhelper['recordingidaction']);
	    $recording_antiaction_filename = get_recording_filename($rowhelper['recordingidantiaction']);
	
	    $password = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'];
	    $port = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'];
	    $host = $config['interfaces']['lan']['ipaddr'];
		
		if (pkg_is_service_running('freeswitch')) {
		    $fp = event_socket_create($host, $port, $password);
		    $cmd = "api global_getvar domain";
		    $domain = trim(event_socket_request($fp, $cmd));
	    }
	
	
	    $tmp = ""; //make sure the variable starts with no value
	    $tmp .= "\n";
	    $tmp .= " var condition = true;\n";
	    $tmp .= "\n";
	    $tmp .= " var domain = \"".$domain."\"; //by default this is the ipv4 address of FreeSWITCH used for transfer to voicemail\n";
	    $tmp .= " var digitmaxlength = 0;\n";
	    $tmp .= " var objdate = new Date();\n";
	    $tmp .= "\n";
	    $tmp .= " var adjusthours = 0; //Adjust Server time that is set to GMT 7 hours\n";
	    $tmp .= " var adjustoperator = \"-\"; //+ or -\n";
	    $tmp .= "\n";
	    $tmp .= " if (adjustoperator == \"-\") {\n";
	    $tmp .= "   var objdate2 = new Date(objdate.getFullYear(),objdate.getMonth(),objdate.getDate(),(objdate.getHours() - adjusthours),objdate.getMinutes(),objdate.getSeconds());\n";
	    $tmp .= " }\n";
	    $tmp .= " if (adjustoperator == \"+\") {\n";
	    $tmp .= "   var objdate2 = new Date(objdate.getFullYear(),objdate.getMonth(),objdate.getDate(),(objdate.getHours() + adjusthours),objdate.getMinutes(),objdate.getSeconds());\n";
	    $tmp .= " }\n";
	    $tmp .= "\n";
	    $tmp .= " var Hours = objdate2.getHours();\n";
	    $tmp .= " var Mins = objdate2.getMinutes();\n";
	    $tmp .= " var Seconds = objdate2.getSeconds();\n";
	    $tmp .= " var Month = objdate2.getMonth() + 1;\n";
	    $tmp .= " var Date = objdate2.getDate();\n";
	    $tmp .= " var Year = objdate2.getYear()\n";
	    $tmp .= " var Day = objdate2.getDay()+1;\n";
	    $tmp .= " var exit = false;\n";
	    $tmp .= "\n";
	    $tmp .= "//console_log( \"info\", \"IVR Server Time is: \"+Hours+\":\"+Mins+\" \\n\" );\n";
	    $tmp .= "\n";
	    $tmp .= " function mycb( session, type, obj, arg ) {\n";
	    $tmp .= "    try {\n";
	    $tmp .= "        if ( type == \"dtmf\" ) {\n";
	    $tmp .= "          console_log( \"info\", \"digit: \"+obj.digit+\"\\n\" );\n";
	    $tmp .= "          if ( obj.digit == \"#\" ) {\n";
	    $tmp .= "            //console_log( \"info\", \"detected pound sign.\\n\" );\n";
	    $tmp .= "            exit = true;\n";
	    $tmp .= "            return( false );\n";
	    $tmp .= "          }\n";
	    $tmp .= "\n";
	    $tmp .= "          dtmf.digits += obj.digit;\n";
	    $tmp .= "\n";
	    $tmp .= "          if ( dtmf.digits.length >= digitmaxlength ) {\n";
	    $tmp .= "            exit = true;\n";
	    $tmp .= "            return( false );\n";
	    $tmp .= "          }\n";
	    $tmp .= "        }\n";
	    $tmp .= "    } catch (e) {\n";
	    $tmp .= "        console_log( \"err\", e+\"\\n\" );\n";
	    $tmp .= "    }\n";
	    $tmp .= "    return( true );\n";
	    $tmp .= " } //end function mycb\n";
	
	    $tmp .= "\n";
	    $tmp .= base64_decode($rowhelper['ivrconditionjs']);
	    $tmp .= "\n";
	    $tmp .= "\n";
	
	    //$tmp .= " //condition = true; //debugging\n";

	    $actiondirect = false;
	    $actioncount = 0;
	    foreach($config['installedpackages']['freeswitchivroptions']['config'] as $row) {
	        //find the correct IVR options with the correct action
	      	if ($row['ivrid'] == $rowhelper['ivrid']) {
	      	   if ($row['optionaction'] == "action") {
	      	        $actioncount++;
	      	        if (strtolower($row['optionnumber']) == "n") { //direct the call now don't wait for dtmf
	      	          $actiondirect = true;
                    $actiondirecttype = $row['optiontype'];
	      	          $actiondirectdest = $row['optiondest'];
	      	        }
	      	   }
	        }   	
	    }
	    //$tmp .= "action count: ".$actioncount."<br />\n";
	    if ($actioncount > 0) {
	      if ($actiondirect) {

      	    $tmp .= " if (condition) {\n";
      	    $tmp .= "    //direct\n";
      	    $tmp .= "    //console_log( \"info\", \"action direct\\n\" );\n";
            if ($actiondirecttype == "extension") {
                $tmp .= "    session.execute(\"transfer\", \"".$actiondirectdest." XML default\"); //".$actiondirectdest."\n";
            }
            if ($actiondirecttype == "voicemail") {
                //$tmp .= "    session.execute(\"voicemail\", \"".$actiondirectdest." XML default\");\n";
                $tmp .= "    session.execute(\"voicemail\", \"default \"+domain+\" ".$actiondirectdest."\");\n";
            }
      	    $tmp .= "}\n";

  	    }
  	    else {
      	    $tmp .= " if (condition) {\n";
      	    $tmp .= "    //action\n";
      	    $tmp .= "\n";
      	    $tmp .= "     //console_log( \"info\", \"action call now don't wait for dtmf\\n\" );\n";
      	    $tmp .= "      var dtmf = new Object( );\n";
      	    $tmp .= "     dtmf.digits = \"\";\n";
      	    $tmp .= "     if ( session.ready( ) ) {\n";
      	    $tmp .= "         session.answer( );\n";
      	    $tmp .= "\n";
      	    $tmp .= "         digitmaxlength = 1;\n";
      	    $tmp .= "         while (session.ready() && ! exit ) {\n";
      	    $tmp .= "           //session.streamFile( \"C:/Program Files/FreeSWITCH/sounds/".$recording_action_filename."\", mycb, \"dtmf ".$rowhelper['ivrtimeout']."\" );\n";
      	    $tmp .= "           session.streamFile( \"/usr/local/freeswitch/recordings/".$recording_action_filename."\", mycb, \"dtmf ".$rowhelper['ivrtimeout']."\" );\n";
      	    $tmp .= "           if (session.ready()) {\n";
      	    $tmp .= "           	if (dtmf.digits == 0) {\n";
      	    $tmp .= "           		dtmf.digits +=  session.getDigits(1, \"#\", ".($rowhelper['ivrtimeout']*1000)."); // ".$rowhelper['ivrtimeout']." seconds\n";
      	    $tmp .= "           		if (dtmf.digits == 0) {\n";
      	    //$tmp .= "           			console_log( "info", "time out option: " + dtmf.digits + "\n" );\n";

      	    $a_ivr_options = &$config['installedpackages']['freeswitchivroptions']['config'];	
      		//find the timeout IVR options with the correct action
      		    if (count($a_ivr_options) > 0) {
      			    foreach($a_ivr_options as $row) {		
      	
      			    	if ($row['ivrid'] == $rowhelper['ivrid']) {
      			
      			          if ($row['optionaction'] == "action") {
      		        	     if (strtolower($row['optionnumber']) == "t") {			
      			                  if ($row['optiontype'] == "extension") {
      			                      $tmp .= "                 session.execute(\"transfer\", \"".$row['optiondest']." XML default\"); //".$row['optiondescr']."\n";
      			                  }
      			                  if ($row['optiontype'] == "voicemail") {
      			                      //$tmp .= "                 session.execute(\"voicemail\", \"".$row['optiondest']." XML default\");\n";
      			                      $tmp .= "                 session.execute(\"voicemail\", \"default \"+domain+\" ".$row['optiondest']."\");\n";
      			                  }
      		        	     }
      			          } //end anti-action
      			
      			    	} //end ivrid
      			
      			    } //end for each
      		    } //if count
      		

      	    $tmp .= "           		}\n";
      	    $tmp .= "           		else {\n";
      	    $tmp .= "           			break; //dtmf found end the while loop\n";
      	    $tmp .= "           		}\n";                        	
      	    $tmp .= "           	}\n";	
      	    $tmp .= "           }\n";	
      	    $tmp .= "         }\n";
      	    $tmp .= "\n";
      	    $tmp .= "         //pickup the remaining digits\n";
      	    //$tmp .= "         //http://wiki.freeswitch.org/wiki/Session_getDigits\n";
      	    //$tmp .= "         //getDigits(length, terminators, timeout, digit_timeout, abs_timeout)\n";
      	    //$tmp .= "       //dtmf.digits +=  session.getDigits(2, \"#\", 3000); //allow up to 3 digits\n";
      	    $tmp .= "         dtmf.digits +=  session.getDigits(3, \"#\", 3000); //allow up to 4 digits\n";
      	    $tmp .= "\n";
      	    $tmp .= "\n";
      	    //$tmp .= "         console_log( \"info\", \"IVR Digit Pressed: \" + dtmf.digits + \"\\n\" );\n";
      	
      	
      	    //action
      	    $tmpaction = "";
      	
      	    $tmp .= "         if ( dtmf.digits.length > \"0\" ) {\n";
      	    $x = 0;
      	    $a_ivr_options = &$config['installedpackages']['freeswitchivroptions']['config'];
      	    if (count($a_ivr_options) > 0) {
      		    foreach($a_ivr_options as $row) {
      		
      		      /*
      		      $row['ivrid']
      		      $row['optionnumber']
      		      $row['optiontype']
      		      $row['optionaction']
      		      $row['optiondest']
      		      $row['optiondescr']
      		      */
      		
      		      $tmpactiondefault = "";
      		
      		      //find the correct IVR options with the correct action
      		    	if ($row['ivrid'] == $rowhelper['ivrid']){
      		
      		          if ($row['optionaction'] == "action") {
      		        	    //$tmpaction .= "\n";
      		        	
      		        	    switch ($row['optionnumber']) {
      		              //case "t":
      		
      		                  //if ($row['optiontype'] == "extension") {
      		                  //    $tmpactiondefault .= "                 session.execute(\"transfer\", \"".$row['optiondest']." XML default\"); //".$row['optiondescr']."\n";
      		                  //}
      		                  //if ($row['optiontype'] == "voicemail") {
      		                  //    //$tmpactiondefault .= "                 session.execute(\"voicemail\", \"".$row['optiondest']." XML default\");\n";
      		                  //    $tmpactiondefault .= "                 session.execute(\"voicemail\", \"default \"+domain+\" ".$row['optiondest']."\");\n";
      		                  //}
      		
      		                  //break;
      		              default:
      		                  //$tmpaction .= "             //console_log( \"info\", \"IVR Detected 1 digit \\n\" );\n";
      		                  if ($x == 0) {
      		                    $tmpaction .= "             if ( dtmf.digits == \"".$row['optionnumber']."\" ) { //".$row['optiondescr']."\n";
      		                  }
      		                  else {
      		                    $tmpaction .= "             else if ( dtmf.digits == \"".$row['optionnumber']."\" ) { //".$row['optiondescr']."\n";
      		                  }
      		                  if ($row['optiontype'] == "extension") {
      		                      $tmpaction .= "                 session.execute(\"transfer\", \"".$row['optiondest']." XML default\");\n";
      		                  }
      		                  if ($row['optiontype'] == "voicemail") {
      		                      //$tmpaction .= "                 session.execute(\"voicemail\", \"".$row['optiondest']." XML default\");\n";
      		                      $tmpaction .= "                 session.execute(\"voicemail\", \"default \"+domain+\" ".$row['optiondest']."\");\n";
      		                  }
      		                  $tmpaction .= "             }\n";
      		
      		
      		              }
      		
      		              $x++;
      		          } //end if action
      		
      		    	} //end ivrid
      		
      		
      		    } //end foreach
      	    } //end if count
      	
      	    $tmp .= $tmpaction;
      	    $tmp .= "             else {\n";
      	    $tmp .= "	                  session.execute(\"transfer\", dtmf.digits+\" XML default\");\n";
      	    //$tmp .= $tmpactiondefault;
      	    $tmp .= "             }\n";
      	    $tmp .= "\n";
      	    unset($tmpaction);
      	
      	
      	    $tmp .= "          } \n";
      	    //$tmp .= "          else if ( dtmf.digits.length == \"3\" ) {\n";
      	    //$tmp .= "	                  //Transfer to the extension the caller chose\n";
      	    //$tmp .= "	                  session.execute(\"transfer\", dtmf.digits+\" XML default\");\n";
      	    //$tmp .= "          } else {\n";
      	    //$tmp .= $tmpactiondefault;
      	    //$tmp .= "          }\n";
      	    $tmp .= "\n";
      	    $tmp .= "     } //end if session.ready\n";
      	    $tmp .= "\n";
      	    $tmp .= " }\n"; //end if condition
      	
  	       }	//if ($actiondirect) {
	    } //actioncount
	
	    $antiactiondirect = false;
	    $antiactioncount = 0;
	    foreach($config['installedpackages']['freeswitchivroptions']['config'] as $row) {
	        //find the correct IVR options with the correct action
	      	if ($row['ivrid'] == $rowhelper['ivrid']) {
	      	   if ($row['optionaction'] == "anti-action") {
	      	        $antiactioncount++;
	      	        if (strtolower($row['optionnumber']) == "n") { //direct the call now don't wait for dtmf
	      	          $antiactiondirect = true;
                    $antiactiondirecttype = $row['optiontype'];
	      	          $antiactiondirectdest = $row['optiondest'];
	      	        }
	      	   }
	        }   	
	    }
	    //$tmp .= "anti-action count: ".$antiactioncount."<br />\n";

	
	    if ($antiactioncount > 0) {
	      if ($antiactiondirect) {
      	    $tmp .= " else {\n";
      	    $tmp .= "     //anti-action direct\n";
      	    $tmp .= "     //console_log( \"info\", \"anti-action call now don't wait for dtmf\\n\" );\n";
            if ($antiactiondirecttype == "extension") {
                $tmp .= "    session.execute(\"transfer\", \"".$antiactiondirectdest." XML default\"); //".$antiactiondirectdest."\n";
            }
            if ($antiactiondirecttype == "voicemail") {
                //$tmp .= "    session.execute(\"voicemail\", \"".$antiactiondirectdest." XML default\");\n";
                $tmp .= "    session.execute(\"voicemail\", \"default \"+domain+\" ".$antiactiondirectdest."\");\n";
            }
      	    $tmp .= "}\n";
	      }
	      else {
      	    $tmp .= " else {\n";
      	    $tmp .= "     //anti-action\n";
      	    $tmp .= "     //console_log( \"info\", \"anti-action options\\n\" );\n";
      	    $tmp .= "\n";
      	    $tmp .= "     var dtmf = new Object( );\n";
      	    $tmp .= "     dtmf.digits = \"\";\n";
      	    $tmp .= "     if ( session.ready( ) ) {\n";
      	    $tmp .= "         session.answer( );\n";
      	    $tmp .= "\n";
      	    $tmp .= "         digitmaxlength = 1;\n";
      	    $tmp .= "         while (session.ready() && ! exit ) {\n";
      	    $tmp .= "           session.streamFile( \"/usr/local/freeswitch/recordings/".$recording_antiaction_filename."\", mycb, \"dtmf ".$rowhelper['ivrtimeout']."\" );\n";
      	    $tmp .= "           if (session.ready()) {\n";
      	    $tmp .= "           	if (dtmf.digits == 0) {\n";
      	    $tmp .= "           		dtmf.digits +=  session.getDigits(1, \"#\", ".($rowhelper['ivrtimeout']*1000)."); // ".$rowhelper['ivrtimeout']." seconds\n";
      	    $tmp .= "           		if (dtmf.digits == 0) {\n";
      	    //$tmp .= "           			console_log( "info", "time out option: " + dtmf.digits + "\n" );\n";


      		//find the timeout IVR options with the correct action
      		    if (count($a_ivr_options) > 0) {
      			    foreach($a_ivr_options as $row) {
      	
      			    	if ($row['ivrid'] == $rowhelper['ivrid']) {
      			
      			          if ($row['optionaction'] == "anti-action") {
      		        	     if (strtolower($row['optionnumber']) == "t") {			
      			        	    		
      			                  if ($row['optiontype'] == "extension") {
      			                      $tmp .= "                 session.execute(\"transfer\", \"".$row['optiondest']." XML default\"); //".$row['optiondescr']."\n";
      			                  }
      			                  if ($row['optiontype'] == "voicemail") {
      			                      //$tmp .= "                 session.execute(\"voicemail\", \"".$row['optiondest']." XML default\");\n";
      			                      $tmp .= "                 session.execute(\"voicemail\", \"default \"+domain+\" ".$row['optiondest']."\");\n";
      			                  }
      		        	     }

      			          } //end anti-action
      			
      			    	} //end ivrid
      			
      			    } //end for each
      		    } //if count


      	    $tmp .= "           		}\n";
       	    $tmp .= "           		else {\n";
       	    $tmp .= "           			break; //dtmf found end the while loop\n";
       	    $tmp .= "           		}\n";
                            	
      	    $tmp .= "           	}\n";	
      	    $tmp .= "           }\n";
      	    $tmp .= "         }\n";
      	    $tmp .= "\n";
      	    $tmp .= "         //pickup the remaining digits\n";
      	    $tmp .= "         //http://wiki.freeswitch.org/wiki/Session_getDigits\n";
      	    $tmp .= "         //getDigits(length, terminators, timeout, digit_timeout, abs_timeout)\n";
      	    $tmp .= "         dtmf.digits +=  session.getDigits(3, \"#\", 3000);\n";
      	    $tmp .= "\n";
      	    $tmp .= "         console_log( \"info\", \"IVR Digit Pressed: \" + dtmf.digits + \"\\n\" );\n";
      	    $tmp .= "\n";
      	
      	

      	
      	
      	    $tmpantiaction = "";
      	    $tmp .= "         if ( dtmf.digits.length > \"0\" ) {\n";
      	
      	    $x = 0;
      	    if (count($a_ivr_options) > 0) {
      		    foreach($a_ivr_options as $row) {
      		
      		      /*
      		      $row['ivrid']
      		      $row['optionnumber']
      		      $row['optiontype']
      		      $row['optionaction']
      		      $row['optiondest']
      		      $row['optiondescr']
      		      */
      		      //$tmpantiactiondefault = "";
      		
      		      //find the correct IVR options with the correct action
      		    	if ($row['ivrid'] == $rowhelper['ivrid']) {
      		
      		          if ($row['optionaction'] == "anti-action") {
      		        	
      		        	  switch ($row['optionnumber']) {
      		              //case "t":
      		
      		                  //if ($row['optiontype'] == "extension") {
      		                  //    $tmpantiactiondefault .= "                 session.execute(\"transfer\", \"".$row['optiondest']." XML default\"); //".$row['optiondescr']."\n";
      		                  //}
      		                  //if ($row['optiontype'] == "voicemail") {
      		                  //    //$tmpantiactiondefault .= "                 session.execute(\"voicemail\", \"".$row['optiondest']." XML default\");\n";
      		                  //    $tmpantiactiondefault .= "                 session.execute(\"voicemail\", \"default \"+domain+\" ".$row['optiondest']."\");\n";
      		                  //}
      		
      		                  //break;
      		              default:
      		                  //$tmpantiaction .= "             //console_log( \"info\", \"IVR Detected 1 digit \\n\" );\n";
      		
      		                  if ($x == 0) {
      		                    $tmpantiaction .= "             if ( dtmf.digits == \"".$row['optionnumber']."\" ) { //".$row['optiondescr']."\n";
      		                  }
      		                  else {
      		                    $tmpantiaction .= "             else if ( dtmf.digits == \"".$row['optionnumber']."\" ) { //".$row['optiondescr']."\n";
      		                  }
      		
      		                  if ($row['optiontype'] == "extension") {
      		                      $tmpantiaction .= "                 session.execute(\"transfer\", \"".$row['optiondest']." XML default\");\n";
      		                  }
      		                  if ($row['optiontype'] == "voicemail") {
      		                      //$tmpantiaction .= "                 session.execute(\"voicemail\", \"".$row['optiondest']." XML default\");\n";
      		                      $tmpantiaction .= "                 session.execute(\"voicemail\", \"default \"+domain+\" ".$row['optiondest']."\");\n";
      		                  }
      		                  $tmpantiaction .= "             }\n";
      		
      		
      		              } //end switch
      		
      		              $x++;
      		        	} //end anti-action
      		
      		    	} //end ivrid
      		    	
      		
      		    } //end for each
      	    } //if count
      	
      	    $tmp .= $tmpantiaction;
      	    $tmp .= "             else {\n";
      	    $tmp .= "	                  session.execute(\"transfer\", dtmf.digits+\" XML default\");\n";	
      	    //$tmp .= $tmpantiactiondefault;
      	    $tmp .= "             }\n";
      	    $tmp .= "\n";
      	    unset($tmpantiaction);
      	
      	    $tmp .= "          } \n";
      	    //$tmp .= "          else if ( dtmf.digits.length == \"3\" ) {\n";
      	    //$tmp .= "                //Transfer to the extension the caller chose\n";
      	    //$tmp .= "                session.execute(\"transfer\", dtmf.digits+\" XML default\"); \n";
      	    //$tmp .= "          }\n";
      	    //$tmp .= "          else {\n";
      	    //$tmp .= $tmpantiactiondefault;
      	    //$tmp .= "          }\n";
      	    $tmp .= "\n";
      	    $tmp .= "     } //end if session.ready\n";
      	    $tmp .= "\n";
      	    $tmp .=  " } //end if condition";
      	
	       }	//if ($antiactiondirect) {
	    } //antiactioncount
	    unset($tmpactiondefault);
	    unset($tmpantiactiondefault);
	
	    if (strlen($rowhelper['ivrid']) > 0) {
		    $ivrfilename = "ivr_".str_replace(array("{", "}"), "", $rowhelper['ivrid']).".js";
		  	$fout = fopen("/usr/local/freeswitch/scripts/".$ivrfilename,"w");
		    fwrite($fout, $tmp);
		    unset($ivrfilename);
		    fclose($fout);
	    }
	
	  } //end foreach
  } //end if count
  conf_mount_ro();
  config_unlock();

} //end function



function sync_package_freeswitch_dialplan_includes()
{

  global $config;
  conf_mount_rw();
  config_lock();

  $a_dialplan_includes          = &$config['installedpackages']['freeswitchdialplanincludes']['config'];
  $a_dialplan_include_details 	= &$config['installedpackages']['freeswitchdialplanincludedetails']['config'];

  if (count($a_dialplan_includes) > 0) {
      foreach($config['installedpackages']['freeswitchdialplanincludes']['config'] as $rowhelper) {
          $tmp = "";
          $tmp .= "\n";

          //$rowhelper['dialplanincludeid'];
          //$rowhelper['extensionname'];
          //$rowhelper['context'];
          //$rowhelper['enabled'];

        $tmp = "<extension name=\"".$rowhelper['extensionname']."\">\n";

        if (count($a_dialplan_include_details) > 0) {
		
			$conditioncount = 0;
			$i = 0;
          	foreach ($a_dialplan_include_details as $ent) {
			  if ($ent['tag'] == "condition" && $rowhelper['dialplanincludeid'] == $ent['dialplanincludeid']) {
			      $conditioncount++;
			      $i++;
			  }
			}
			
			$i = 1;
          	foreach ($a_dialplan_include_details as $ent) {
			  if ($ent['tag'] == "condition" && $rowhelper['dialplanincludeid'] == $ent['dialplanincludeid']) {
				    if ($conditioncount == 1) { //single condition
				    	//start tag
				    	$tmp .= "   <condition field=\"".$ent['fieldtype']."\" expression=\"".$ent['fielddata']."\">\n";
				    }
				    else { //more than one condition
						if ($i < $conditioncount) {
				              //all tags should be self-closing except the last one
				              $tmp .= "   <condition field=\"".$ent['fieldtype']."\" expression=\"".$ent['fielddata']."\"/>\n";
						}
						else {
				            //for the last tag use the start tag
				              $tmp .= "   <condition field=\"".$ent['fieldtype']."\" expression=\"".$ent['fielddata']."\">\n";
						}
				    }
				    $i++;
			  }
			} //end for each
		
		} //end if count

        if (count($a_dialplan_include_details) > 0) {
          $i = 0;
          foreach ($a_dialplan_include_details as $ent) {
            if ($ent['tag'] == "action" && $rowhelper['dialplanincludeid'] == $ent['dialplanincludeid']) {
                $tmp .= "       <action application=\"".$ent['fieldtype']."\" data=\"".$ent['fielddata']."\"/>\n";
            }
            $i++;
          }
        }

        if (count($a_dialplan_include_details) > 0) {
            $i = 0;
          foreach ($a_dialplan_include_details as $ent) {
            if ($ent['tag'] == "anti-action" && $rowhelper['dialplanincludeid'] == $ent['dialplanincludeid']) {
                $tmp .= "       <anti-action application=\"".$ent['fieldtype']."\" data=\"".$ent['fielddata']."\"/>\n";
            }
            $i++;
          }
        }

        //if (count($a_dialplan_include_details) > 0) {
        //foreach ($a_dialplan_include_details as $ent) {
        //    $i = 0;
        //    if ($ent['tag'] == "param" && $rowhelper['dialplanincludeid'] == $ent['dialplanincludeid']) {
                //$ent['tag']
                //$ent['fieldtype']
                //$ent['fielddata']
        //    }
        //    $i++;
        //  }
        //}

        if ($conditioncount > 0) {
            $tmp .= "   </condition>\n";
        }
        unset ($conditioncount);
        $tmp .= "</extension>\n";


        if ($rowhelper['enabled'] == "true") {
            $dialplanincludefilename = $rowhelper['order']."_".$rowhelper['extensionname'].".xml";
          	$fout = fopen("/usr/local/freeswitch/conf/dialplan/default/".$dialplanincludefilename,"w");
            fwrite($fout, $tmp);
            fclose($fout);
        }
        unset($dialplanincludefilename);
        unset($tmp);


      } //end foreach
  } //if array count

  conf_mount_ro();
  config_unlock();

}


function sync_package_freeswitch_public_includes()
{

  global $config;
  conf_mount_rw();
  config_lock();

  $a_public_includes          	= &$config['installedpackages']['freeswitchpublicincludes']['config'];
  $a_public_include_details 	= &$config['installedpackages']['freeswitchpublicincludedetails']['config'];

  if (count($a_public_includes) > 0) {
	  foreach($a_public_includes as $rowhelper) {
	      $tmp = "";
	      $tmp .= "\n";
	
	      //$rowhelper['publicincludeid'];
	      //$rowhelper['extensionname'];
	      //$rowhelper['context'];
	      //$rowhelper['enabled'];
	
	    $tmp = "<extension name=\"".$rowhelper['extensionname']."\">\n";
	
		if (count($a_public_include_details) > 0) {
		
			$conditioncount = 0;
			$i = 0;
			foreach ($a_public_include_details as $ent) {
			  if ($ent['tag'] == "condition" && $rowhelper['publicincludeid'] == $ent['publicincludeid']) {
			      $conditioncount++;
			      $i++;
			  }
			}
			
			$i = 1;
			foreach ($a_public_include_details as $ent) {
			  if ($ent['tag'] == "condition" && $rowhelper['publicincludeid'] == $ent['publicincludeid']) {
				    if ($conditioncount == 1) { //single condition
				    	//start tag
				    	$tmp .= "   <condition field=\"".$ent['fieldtype']."\" expression=\"".$ent['fielddata']."\">\n";
				    }
				    else { //more than one condition
						if ($i < $conditioncount) {
				              //all tags should be self-closing except the last one
				              $tmp .= "   <condition field=\"".$ent['fieldtype']."\" expression=\"".$ent['fielddata']."\"/>\n";
						}
						else {
				            //for the last tag use the start tag
				              $tmp .= "   <condition field=\"".$ent['fieldtype']."\" expression=\"".$ent['fielddata']."\">\n";
						}
				    }
				    $i++;
			  }
			} //end for each
		
		} //end if count

	
	    if (count($a_public_include_details) > 0) {
	      $i = 0;
	      foreach ($a_public_include_details as $ent) {
	        if ($ent['tag'] == "action" && $rowhelper['publicincludeid'] == $ent['publicincludeid']) {
	            $tmp .= "       <action application=\"".$ent['fieldtype']."\" data=\"".$ent['fielddata']."\"/>\n";
	        }
	        $i++;
	      }
	    }
	
	    if (count($a_public_include_details) > 0) {
	        $i = 0;
	      foreach ($a_public_include_details as $ent) {
	        if ($ent['tag'] == "anti-action" && $rowhelper['publicincludeid'] == $ent['publicincludeid']) {
	            $tmp .= "       <anti-action application=\"".$ent['fieldtype']."\" data=\"".$ent['fielddata']."\"/>\n";
	        }
	        $i++;
	      }
	    }
	
	    //if (count($a_public_include_details) > 0) {
	    //foreach ($a_public_include_details as $ent) {
	    //    $i = 0;
	    //    if ($ent['tag'] == "param" && $rowhelper['publicincludeid'] == $ent['publicincludeid']) {
	            //$ent['tag']
	            //$ent['fieldtype']
	            //$ent['fielddata']
	    //    }
	    //    $i++;
	    //  }
	    //}
	
	    if ($conditioncount > 0) {
	        $tmp .= "   </condition>\n";
	    }
	    unset ($conditioncount);
	    $tmp .= "</extension>\n";
	
	
	    if ($rowhelper['enabled'] == "true") {
	        $publicincludefilename = $rowhelper['order']."_".$rowhelper['extensionname'].".xml";
	      	$fout = fopen("/usr/local/freeswitch/conf/dialplan/public/".$publicincludefilename,"w");
	        fwrite($fout, $tmp);
	        fclose($fout);
	    }
	    unset($publicincludefilename);
	    unset($tmp);
	
	
	  } //end foreach
  } //end if count
  conf_mount_ro();
  config_unlock();

}


function sync_package_freeswitch()
{

  global $config;
  sync_package_freeswitch_settings();
  sync_package_freeswitch_dialplan();
  sync_package_freeswitch_dialplan_includes();
  sync_package_freeswitch_extensions();
  sync_package_freeswitch_gateways();
  sync_package_freeswitch_modules();
  sync_package_freeswitch_public();
  sync_package_freeswitch_public_includes();
  sync_package_freeswitch_vars();
  sync_package_freeswitch_internal();
  sync_package_freeswitch_external();
  //sync_package_freeswitch_recordings();
  if (pkg_is_service_running('freeswitch')) {
      sync_package_freeswitch_ivr();
  }

}


function freeswitch_php_install_command()
{
	global $config;
	$freeswitch_package_version = "0.8.7.3";
	$freeswitch_build_version = "1.0.4 pre 6";
	$freeswitch_build_revision = "13238";
	
	//set script execution time limit to 24 hours
	set_time_limit (86400);
	ini_set(max_execution_time,86400);
	
	//hide errors
	ini_set('display_errors', '0'); 

	conf_mount_rw();
	config_lock();

	if (!is_dir('/usr/local/www/packages/')) {
	   exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/freeswitch/')) {
	   exec("mkdir /usr/local/www/packages/freeswitch/");
	}
	//$struname = exec('uname -v');
	//if (stristr($struname, 'FreeBSD 7.0')) {
	//	$freebsd_version = "7.0";
	//}

	
	//exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch.tgz"); //handled by freeswitch.xml
	exec("tar zxvf /tmp/freeswitch.tgz -C /usr/local/");
	unlink_if_exists("/tmp/freeswitch.tgz");


	//make a backup copy of the default config
	exec("cp -R /usr/local/freeswitch/conf /usr/local/freeswitch/conf.orig");
	
	//remove some default config files that are not needed
	unlink_if_exists("/usr/local/freeswitch/conf/dialplan/default/01_example.com.xml");
	unlink_if_exists("/usr/local/freeswitch/conf/dialplan/public/00_inbound_did.xml");	

	if (!is_dir('/usr/local/freeswitch/sounds/custom/')) {
	   exec("mkdir /usr/local/freeswitch/sounds/custom/");
	}

	if (!is_dir('/usr/local/freeswitch/sounds/custom/8000/')) {
	   exec("mkdir /usr/local/freeswitch/sounds/custom/8000/");
	}

	//copy audio files
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/please_enter_your_pin_number.wav");
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/please_enter_the_pin_number.wav");	
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/please_enter_the_extension_number.wav");
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/please_enter_the_phone_number.wav");
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/call_forward_has_been_set.wav");
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/call_forward_has_been_deleted.wav");	
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/begin_recording.wav");
	exec("cd /usr/local/freeswitch/sounds/custom/8000/;fetch http://www.pfsense.com/packages/config/freeswitch/your_pin_number_is_incorect_goodbye.wav");

	
	//download lib files
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libtinfo.so.5.6");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libncurses.so.5.6");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libogg.so.5.3");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libvorbis.so.4");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libcurl.so.5");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libspandsp.so.1");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libodbc.so.1");
	exec("cd /usr/local/lib/;fetch http://www.pfsense.com/packages/config/freeswitch/libiconv.so.3");
	
	
	//download xml package files
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch.inc");
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch.xml");
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_dialplan.xml");
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_external.xml");
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_internal.xml");
	exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_modules.xml");
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_public.xml");
	//exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_vars.xml");
	
	//misc files
	if (!is_dir('/usr/local/www/edit_area/')) {
	   exec("cd /usr/local/pkg/;fetch http://www.pfsense.com/packages/config/freeswitch/edit_area.tgz");
	   system('cd /usr/local/www; tar xvpfz /tmp/edit_area.tgz edit_area');
	   unlink_if_exists("/tmp/edit_area.tgz");
	}

	
	//rename PHP files from .tmp to .php
	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/class.smtp.tmp");
	exec("cp /tmp/class.smtp.tmp /usr/local/www/packages/freeswitch/class.smtp.php");
	unlink_if_exists("/tmp/class.smtp.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/class.phpmailer.tmp");
	exec("cp /tmp/class.phpmailer.tmp /usr/local/www/packages/freeswitch/class.phpmailer.php");
	unlink_if_exists("/tmp/class.phpmailer.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_cmd.tmp");	
	exec("cp /tmp/freeswitch_cmd.tmp /usr/local/www/packages/freeswitch/freeswitch_cmd.php");
	unlink_if_exists("/tmp/freeswitch_cmd.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_dialplan.tmp");	
	exec("cp /tmp/freeswitch_dialplan.tmp /usr/local/www/packages/freeswitch/freeswitch_dialplan.php");
	unlink_if_exists("/tmp/freeswitch_dialplan.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_dialplan_includes_details.tmp");	
	exec("cp /tmp/freeswitch_dialplan_includes_details.tmp /usr/local/www/packages/freeswitch/freeswitch_dialplan_includes_details.php");
	unlink_if_exists("/tmp/freeswitch_dialplan_includes_details.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_dialplan_includes_details_edit.tmp");
	exec("cp /tmp/freeswitch_dialplan_includes_details_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_dialplan_includes_details_edit.php");
	unlink_if_exists("/tmp/freeswitch_dialplan_includes_details_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_dialplan_includes.tmp");
	exec("cp /tmp/freeswitch_dialplan_includes.tmp /usr/local/www/packages/freeswitch/freeswitch_dialplan_includes.php");
	unlink_if_exists("/tmp/freeswitch_dialplan_includes.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_dialplan_includes_edit.tmp");	
	exec("cp /tmp/freeswitch_dialplan_includes_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_dialplan_includes_edit.php");
	unlink_if_exists("/tmp/freeswitch_dialplan_includes_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_extensions.tmp");
	exec("cp /tmp/freeswitch_extensions.tmp /usr/local/www/packages/freeswitch/freeswitch_extensions.php");
	unlink_if_exists("/tmp/freeswitch_extensions.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_extensions_edit.tmp");
	exec("cp /tmp/freeswitch_extensions_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_extensions_edit.php");
	unlink_if_exists("/tmp/freeswitch_extensions_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_features.tmp");
	exec("cp /tmp/freeswitch_features.tmp /usr/local/www/packages/freeswitch/freeswitch_features.php");
	unlink_if_exists("/tmp/freeswitch_features.tmp");	

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_gateways.tmp");
	exec("cp /tmp/freeswitch_gateways.tmp /usr/local/www/packages/freeswitch/freeswitch_gateways.php");
	unlink_if_exists("/tmp/freeswitch_gateways.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_gateways_edit.tmp");
	exec("cp /tmp/freeswitch_gateways_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_gateways_edit.php");
	unlink_if_exists("/tmp/freeswitch_gateways_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_ivr.tmp");	
	exec("cp /tmp/freeswitch_ivr.tmp /usr/local/www/packages/freeswitch/freeswitch_ivr.php");
	unlink_if_exists("/tmp/freeswitch_ivr.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_ivr_edit.tmp");
	exec("cp /tmp/freeswitch_ivr_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_ivr_edit.php");
	unlink_if_exists("/tmp/freeswitch_ivr_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_ivr_options.tmp");
	exec("cp /tmp/freeswitch_ivr_options.tmp /usr/local/www/packages/freeswitch/freeswitch_ivr_options.php");
	unlink_if_exists("/tmp/freeswitch_ivr_options.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_ivr_options_edit.tmp");
	exec("cp /tmp/freeswitch_ivr_options_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_ivr_options_edit.php");
	unlink_if_exists("/tmp/freeswitch_ivr_options_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_profiles.tmp");
	exec("cp /tmp/freeswitch_profiles.tmp /usr/local/www/packages/freeswitch/freeswitch_profiles.php");
	unlink_if_exists("/tmp/freeswitch_profiles.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_profile_edit.tmp");
	exec("cp /tmp/freeswitch_profile_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_profile_edit.php");
	unlink_if_exists("/tmp/freeswitch_profile_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_public.tmp");
	exec("cp /tmp/freeswitch_public.tmp /usr/local/www/packages/freeswitch/freeswitch_public.php");
	unlink_if_exists("/tmp/freeswitch_public.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_public_includes.tmp");
	exec("cp /tmp/freeswitch_public_includes.tmp /usr/local/www/packages/freeswitch/freeswitch_public_includes.php");
	unlink_if_exists("/tmp/freeswitch_public_includes.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_public_includes_edit.tmp");    	
	exec("cp /tmp/freeswitch_public_includes_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_public_includes_edit.php");
	unlink_if_exists("/tmp/freeswitch_public_includes_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_public_includes_details.tmp");
	exec("cp /tmp/freeswitch_public_includes_details.tmp /usr/local/www/packages/freeswitch/freeswitch_public_includes_details.php");
	unlink_if_exists("/tmp/freeswitch_public_includes_details.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_public_includes_details_edit.tmp");
	exec("cp /tmp/freeswitch_public_includes_details_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_public_includes_details_edit.php");
	unlink_if_exists("/tmp/freeswitch_public_includes_details_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_mailto.tmp");
	exec("cp /tmp/freeswitch_mailto.tmp /usr/local/www/packages/freeswitch/freeswitch_mailto.php");
	unlink_if_exists("/tmp/freeswitch_mailto.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_recordings.tmp");
	exec("cp /tmp/freeswitch_recordings.tmp /usr/local/www/packages/freeswitch/freeswitch_recordings.php");
	unlink_if_exists("/tmp/freeswitch_recordings.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_recordings_edit.tmp");	
	exec("cp /tmp/freeswitch_recordings_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_recordings_edit.php");
	unlink_if_exists("/tmp/freeswitch_recordings_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_recordings_play.tmp");
	exec("cp /tmp/freeswitch_recordings_play.tmp /usr/local/www/packages/freeswitch/freeswitch_recordings_play.php");
	unlink_if_exists("/tmp/freeswitch_recordings_play.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_status.tmp");
	exec("cp /tmp/freeswitch_status.tmp /usr/local/www/packages/freeswitch/freeswitch_status.php");
	unlink_if_exists("/tmp/freeswitch_status.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_time_conditions.tmp");
	exec("cp /tmp/freeswitch_time_conditions.tmp /usr/local/www/packages/freeswitch/freeswitch_time_conditions.php");
	unlink_if_exists("/tmp/freeswitch_time_conditions.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_time_conditions_edit.tmp");
	exec("cp /tmp/freeswitch_time_conditions_edit.tmp /usr/local/www/packages/freeswitch/freeswitch_time_conditions_edit.php");
	unlink_if_exists("/tmp/freeswitch_time_conditions_edit.tmp");

	exec("cd /tmp/;fetch http://www.pfsense.com/packages/config/freeswitch/freeswitch_vars.tmp");
	exec("cp /tmp/freeswitch_vars.tmp /usr/local/www/packages/freeswitch/freeswitch_vars.php");
	unlink_if_exists("/tmp/freeswitch_vars.tmp");

	exec("cd /usr/local/freeswitch/scripts/;fetch http://www.pfsense.com/packages/config/freeswitch/disa.js");
	exec("cp /usr/local/freeswitch/htdocs/slim.swf /usr/local/www/packages/freeswitch/slim.swf");

  /* freeswitch settings defaults */
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['numbering_plan']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['numbering_plan'] = "US";
  	}
    if(strlen($config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'] = "ClueCon";
  	}
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'] = "8021";
  	}
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_http_port']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_http_port'] = "8787";
  	}
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_realm']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_realm'] = "freeswitch";
  	}
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_user']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_user'] = "freeswitch";
  	}
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_pass']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_pass'] = "works";
  	}
  	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['admin_pin']) == 0) {
  	   $config['installedpackages']['freeswitchsettings']['config'][0]['admin_pin'] = "7575";
  	}	
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['mod_shout_decoder']) == 0) {
		$config['installedpackages']['freeswitchsettings']['config'][0]['mod_shout_decoder'] = "i386";
	}	
	if (strlen($config['installedpackages']['freeswitchsettings']['config'][0]['mod_shout_volume']) == 0) {
		$config['installedpackages']['freeswitchsettings']['config'][0]['mod_shout_volume'] = "0.3";
	}
    		
	$numbering_plan = $config['installedpackages']['freeswitchsettings']['config'][0]['numbering_plan'];
	$event_socket_password = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_password'];
	$event_socket_port = $config['installedpackages']['freeswitchsettings']['config'][0]['event_socket_port'];
	$xml_rpc_http_port = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_http_port'];
	$xml_rpc_auth_realm = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_realm'];
	$xml_rpc_auth_user = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_user'];
	$xml_rpc_auth_pass = $config['installedpackages']['freeswitchsettings']['config'][0]['xml_rpc_auth_pass'];
	$admin_pin = $config['installedpackages']['freeswitchsettings']['config'][0]['admin_pin'];

  //write the recording.js script
    recording_js();

  //add recording.js to the dialplan
    $a_dialplan_includes          	= &$config['installedpackages']['freeswitchdialplanincludes']['config'];
    $a_dialplan_include_details 	= &$config['installedpackages']['freeswitchdialplanincludedetails']['config'];

	//delete dialplan recording from the previous install
	    if (count($a_dialplan_includes) > 0) {
	    	$i = 0;
	    	foreach ($a_dialplan_includes as $ent) {
	        	if ($ent['extensionname'] == "Recordings") {
	            	unset($a_dialplan_includes[$i]);
	        	}
	        	$i++;
	    	}
	    }

	//delete the recording dialplan details		
	    if (count($a_dialplan_include_details) > 0) {
	    	$i = 0;
	    	foreach ($a_dialplan_include_details as $ent) {
	        	if ($ent['fielddata'] == "^732673$") {
	            	unset($a_dialplan_include_details[$i]);
	        	}
	        	if ($ent['fielddata'] == "recordings.js") {
	            	unset($a_dialplan_include_details[$i]);	            	
	        	}
	        	$i++;
	      	}
	    }
	
	//add recording to the dialplan
		$dialplanincludeid = guid();

		$ent = array();
		$ent['dialplanincludeid'] = $dialplanincludeid;
		$ent['extensionname'] = 'Recordings';
		$ent['order'] = '9000';
		$ent['context'] = 'default';
		$ent['enabled'] = 'true';			
		$ent['descr'] = '*732673 Default system recordings tool';
		$a_dialplan_includes[] = $ent;
		unset($ent);
			
		$ent = array();
		$ent['dialplanincludeid'] = $dialplanincludeid;
		$ent['tag'] = 'condition'; //condition, action, antiaction
		$ent['fieldtype'] = 'destination_number';
		$ent['fielddata'] = '^\*(732673)$';
		$a_dialplan_include_details[] = $ent;
		unset($ent);
			
		$ent = array();
		$ent['dialplanincludeid'] = $dialplanincludeid;
		$ent['tag'] = 'action'; //condition, action, antiaction
		$ent['fieldtype'] = 'javascript';
		$ent['fielddata'] = 'recordings.js';
		$a_dialplan_include_details[] = $ent;
		unset($ent);


	//delete dialplan DISA from the previous install
		$disa_enabled = 'false';
	    if (count($a_dialplan_includes) > 0) {
	    	$i = 0;
	    	foreach ($a_dialplan_includes as $ent) {
	        	if ($ent['extensionname'] == "DISA") {
					$disa_enabled = $ent['enabled'];
					unset($a_dialplan_includes[$i]);
	        	}
	        	$i++;
	    	}
	    }
		
	//add the DISA to the dialplan
		$dialplanincludeid = guid();

		$ent = array();
		$ent['dialplanincludeid'] = $dialplanincludeid;
		$ent['extensionname'] = 'DISA';
		$ent['order'] = '000';
		$ent['context'] = 'default';
		$ent['enabled'] = $disa_enabled;
		$ent['descr'] = '*3472 Direct Inward System Access';
		$a_dialplan_includes[] = $ent;
		unset($ent);
			
		$ent = array();
		$ent['dialplanincludeid'] = $dialplanincludeid;
		$ent['tag'] = 'condition'; //condition, action, antiaction, set
		$ent['fieldtype'] = 'destination_number';
		$ent['fielddata'] = '^\*(3472)$';
		$a_dialplan_include_details[] = $ent;
		unset($ent);
			
		$ent = array();
		$ent['dialplanincludeid'] = $dialplanincludeid;
		$ent['tag'] = 'action'; //condition, action, antiaction, set
		$ent['fieldtype'] = 'javascript';
		$ent['fielddata'] = 'disa.js';
		$a_dialplan_include_details[] = $ent;
		unset($ent);

	write_config();	
    		
	//prepare switch.conf.xml for voicemail to email
		$filename = "/usr/local/freeswitch/conf/autoload_configs/switch.conf.xml";
		$handle = fopen($filename,"rb");
		$contents = fread($handle, filesize($filename));	
		fclose($handle);
		
		$handle = fopen($filename,"w");
		$contents = str_replace("<param name=\"mailer-app\" value=\"sendmail\"/>", "<param name=\"mailer-app\" value=\"/usr/local/bin/php\"/>", $contents);
		$contents = str_replace("<param name=\"mailer-app-args\" value=\"-t\"/>", "<param name=\"mailer-app-args\" value=\"/usr/local/www/packages/freeswitch/freeswitch_mailto.php\"/>", $contents);
		fwrite($handle, $contents);		
		unset($contents); 	
		fclose($handle);
		unset($filename);
	
	//prepare shout.conf.xml for mod_shout	
		$fout = fopen("/usr/local/freeswitch/conf/autoload_configs/shout.conf.xml","w");
		$tmpxml = "<configuration name=\"shout.conf\" description=\"mod shout config\">\n";
		$tmpxml .= "  <settings>\n";
		$tmpxml .= "    <!-- Don't change these unless you are insane -->\n";
		$tmpxml .= "    <param name=\"decoder\" value=\"i586\"/>\n";
		$tmpxml .= "    <!--<param name=\"volume\" value=\".1\"/>-->\n";
		$tmpxml .= "    <!--<param name=\"outscale\" value=\"8192\"/>-->\n";
		$tmpxml .= "  </settings>\n";
		$tmpxml .= "</configuration>";
		fwrite($fout, $tmpxml);
		unset($tmpxml);
		fclose($fout);
	
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

	/* freeswitch modules defaults */

  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_console']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_console'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_logfile']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_logfile'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_syslog']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_syslog'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_yaml']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_yaml'] = "disable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_enum']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_enum'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_xml_rpc']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_xml_rpc'] = "disable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_xml_curl']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_xml_curl'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_xml_cdr']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_xml_cdr'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_cdr_csv']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_cdr_csv'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_event_multicast']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_event_multicast'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_event_socket']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_event_socket'] = "enable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_zeroconf']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_zeroconf'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_ldap']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_ldap'] = "disable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_dingaling']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_dingaling'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_iax']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_iax'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_portaudio']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_portaudio'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_alsa']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_alsa'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_sofia']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_sofia'] = "enable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_loopback']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_loopback'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_wanpipe']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_wanpipe'] = "disable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_woomera']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_woomera'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_openzap']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_openzap'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_commands']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_commands'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_conference']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_conference'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_dptools']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_dptools'] = "enable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_expr']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_expr'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_fax']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_fax'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_fifo']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_fifo'] = "enable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_voicemail']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_voicemail'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_limit']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_limit'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_esf']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_esf'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_fsv']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_fsv'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_snom']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_snom'] = "disable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_dialplan_directory']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_dialplan_directory'] = "disable";
  	}  	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_dialplan_xml']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_dialplan_xml'] = "enable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_dialplan_asterisk']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_dialplan_asterisk'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_voipcodecs']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_voipcodecs'] = "enable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_g723_1']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_g723_1'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_g729']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_g729'] = "enable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_amr']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_amr'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_ilbc']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_ilbc'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_speex']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_speex'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_siren']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_siren'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_celt']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_celt'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_h26x']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_h26x'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_sndfile']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_sndfile'] = "enable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_native_file']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_native_file'] = "enable";
  	}  	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_shout']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_shout'] = "disable";
  	}
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_local_stream']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_local_stream'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_tone_stream']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_tone_stream'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_spidermonkey']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_spidermonkey'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_perl']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_perl'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_python']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_python'] = "disable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_java']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_java'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_lua']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_lua'] = "enable";
  	} 	
    if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_flite']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_flite'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_pocketsphinx']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_pocketsphinx'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_cepstral']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_cepstral'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_openmrcp']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_openmrcp'] = "disable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_rss']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_rss'] = "disable";
  	}          	
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_say_en']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_say_en'] = "enable";
  	}
  	if (strlen($config['installedpackages']['freeswitchmodules']['config'][0]['mod_say_zh']) == 0) {
  	   $config['installedpackages']['freeswitchmodules']['config'][0]['mod_say_zh'] = "disable";
  	}
  	
	// if backup file exists restore it		
		if (!is_dir('/root/backup/')) {
			exec("mkdir /root/backup/");
		}
	
	//extract a specific directory to /usr/local/freeswitch
		$filename = 'freeswitch.bak.tgz';
		if (file_exists('/root/backup/'.$filename)) {
			//echo "The file $filename exists";
	
			exec("rm -R /usr/local/freeswitch/conf/sip_profiles/");
			exec("rm -R /usr/local/freeswitch/sounds/music/");
			
			//Recommended
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/db/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/log/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/recordings/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/scripts/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/storage/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/sounds/custom/8000/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/sounds/music/8000/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/conf/ssl/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/conf/sip_profiles/');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/conf/vars.xml');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/conf/dialplan/default.xml');
			system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/conf/dialplan/public.xml');

			//Optional
			//system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/conf/');
			//system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/grammar/');
			//system('cd /usr/local; tar xvpfz /root/backup/'.$filename.' freeswitch/htdocs/');

			unset($filename);
		}


	exec("rm -R /freeswitch");
	exec("cp /usr/local/freeswitch/conf/directory/default/brian.xml /usr/local/freeswitch/conf/directory/default/brian.xml.noload");
	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/brian.xml");
	unlink_if_exists("/usr/local/freeswitch/conf/directory/default/example.com.xml");
	unlink_if_exists("/usr/local/freeswitch/conf/dialplan/default/99999_enum.xml");
		
	write_rcfile(array(
		"file" => "freeswitch.sh",
		"start" => "/usr/local/freeswitch/bin/./freeswitch -nc",
		"stop" => "/usr/local/freeswitch/bin/./freeswitch -stop"
		)
	);
	
	lan_sip_profile();

	sync_package_freeswitch();
	$handle = popen("/usr/local/etc/rc.d/freeswitch.sh start", "r");
	pclose($handle);

	if (pkg_is_service_running('freeswitch')) {
		sync_package_freeswitch_ivr();
	}

	$config['installedpackages']['freeswitchsettings']['config'][0]['freeswitch_version'] = $freeswitch_build_version." revision ".$freeswitch_build_revision.".";
	$config['installedpackages']['freeswitchsettings']['config'][0]['freeswitch_package_version'] = $freeswitch_package_version;
	
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
	unlink_if_exists("/usr/local/pkg/freeswitch_internal.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_modules.xml");	
	unlink_if_exists("/usr/local/pkg/freeswitch_public.xml");
	unlink_if_exists("/usr/local/pkg/freeswitch_vars.xml");	

	unlink_if_exists("/usr/local/www/packages/freeswitch/class.smtp.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/class.phpmailer.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_cmd.php");	
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_dialplan_includes_details.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_dialplan_includes_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_dialplan_includes.php");	
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_dialplan_includes_details_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_extensions.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_extensions_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_ivr.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_ivr_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_ivr_options_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_ivr_options.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_gateways.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_gateways_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_mailto.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_public_includes_details.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_public_includes_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_public_includes.php");	
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_public_includes_details_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_recordings.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_recordings_edit.php");	
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_recordings_play.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_time_conditions.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_time_conditions_edit.php");
	unlink_if_exists("/usr/local/www/packages/freeswitch/freeswitch_status.php");	
	unlink_if_exists("/usr/local/www/packages/freeswitch/slim.swf");

	exec("rm -R /usr/local/freeswitch/");
	exec("rm -R /usr/local/www/packages/freeswitch/");
	unlink_if_exists("/usr/local/etc/rc.d/freeswitch.sh");
	unlink_if_exists("/tmp/freeswitch.tar.gz");
	unlink_if_exists("/tmp/pkg_mgr_FreeSWITCH.log");	
	conf_mount_ro();
	config_unlock();

}

?>
