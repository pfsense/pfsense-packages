<?php
/*
	set.php
	Copyright (C) 2015 H-T Reimers <reimers@mail.de>
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
	session_start();
	$encoding="ISO-8859-1";
	mb_internal_encoding("$encoding");

	require_once('surftool_class.inc');

	//Read configuration
	$surftool_config=parse_ini_file ("surftool.ini");
	
	$debug=1;
	if(isset($config["debuglevel"])) $debug=$config["debuglevel"];
	
	$switchtime=2;
	if(isset($config["refresh_time"])) $switchtime=$config["refresh_time"]+1;	
	
	$sgconf = new surftoolSquidGuardConf($surftool_config);

	//Prepare GUI functions
	$surftoolGUI = new surftoolGUI($sgconf);



	//Read changes from POST-data
	$change_msg=$surftoolGUI->changes();
	

	
	function print_header($surftoolGUI,$meta){
		echo "<head>\n";
		echo $surftoolGUI->get_java_script();
		echo $surftoolGUI->get_style();
		echo $meta;
		echo "\n</head>\n<body>\n";	
	}
 
 	if(!isset($_SESSION["login"])){
		$meta="<meta http-equiv='refresh' content='0; URL=index.php'>";
		print_header($surftoolGUI,$meta);
		echo "login first <a href='index.php'> weiter </a>";
		exit(0);
	}
	else if(isset($_POST["mode"])){
		$meta="<meta http-equiv='refresh' content='".$switchtime."' />";
		print_header($surftoolGUI,$meta);
	}
	else{
		$meta="";
		print_header($surftoolGUI,$meta);
	}
	


	//Print logout button
	echo "<form action='index.php' method='post'>
		<input type='submit' name='logout' value='logout'>
	</form> ";	


	if( $change_msg!=""){
		echo $change_msg;
		echo "Please wait ".$switchtime." seconds ...";
		exit(0);
	}
	
	

	//Get groups/acls
	$groupsAll=$sgconf->squidGuardConf;

	//=== Find Groups with similar ip.
	// ip adresses ar in $groupsAll["src"]["NAME"] and acls are in => $groupsAll["acl"]["NAME"]
	$ip = $_SERVER["REMOTE_ADDR"];
	$lastpoint=strripos($ip,'.');
	$firstpart=substr($ip,0,$lastpoint);
	$groupsNear=array();
	foreach($groupsAll["src"] AS $srcname => $src){
		if(isset($src["ip"])){
			foreach($src["ip"] AS $ip){
				$pos = strpos($ip, $firstpart);
				if( $pos !== false){
					if(isset($groupsAll["acl"]["$srcname"])){
						$groupsNear["acl"]["$srcname"]=$groupsAll["acl"]["$srcname"];
					}
					else{
						echo "Warning: no acl:'$srcname'<br>\n ";
					}
				}
			}
		}
		else echo "no src<br>\n";
	}



	echo "<pre>";
	//print_r($sgconf);
	echo "</pre>";
	
	echo "\n<form action='' method='post'>\n";
	echo "<form accept-charset='$encoding'> ";
	
	echo $surftoolGUI->print_switch_mode();
	
	echo "<h2>Vorschläge</h2>";
	$surftoolGUI->print_table($groupsNear);	
	echo "<h2>Alle Räume</h2>";
	$surftoolGUI->print_table($groupsAll);


	echo "\n</form>\n";
	


	//if($debug>1) preecho($data);

	echo"
	<form action='index.php' method='post'>
		<input type='submit' name='logout' value='logout'>
	</form> ";	



?>
