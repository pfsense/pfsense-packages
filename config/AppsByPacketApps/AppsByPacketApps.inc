<?php
function get_arch(){
		//on détermine si le système est en 32 bits ou 64 bits
	 if(file_exists("/etc/platform")) {
		$arch = php_uname("m");
	 }
	 else{
		return "error";
	 }
	 
	 switch( $arch ){
		 case "i386" : return "32";
		 break;
		 case "amd64" : return "64";
		 break;
		 case "x86_64" : return "64";
		 break;
		 default :
			$res = strpos(system("file /sbin/init"),"64-bit");
			if( $res == "0" ){
				$res = strpos(system("file /sbin/init"),"32-bit");
			}
			else{
				return "64";
			}
			if( $res == "0" ){
				return "error";
			}
			else{
				return "32";
			}
		break;
	} 
}

function get_php_api_version(){
		//on détermine la version de l'API PHP pour les extensions
		//on s'y intéresse pour l'installation de blenc
	$phpini = system("grep extension_dir= /usr/local/etc/php.ini");
	$phpini_explode = explode("=",$phpini);
	$phplib = $phpini_explode[1];
	return $phplib;
}

function register_device($dns, $url,$data){
		//on vient enregistrer les informations du device sur le server de packetapps
	if( strlen($dns.$url) > 2000 ){
		error_log("[App By PacketApps] - Device Registration - Too long Url");
	}else{
		$data_json = array('data'=>json_encode($data));
		$ch = curl_init();
		curl_setopt($ch,CURLOPT_URL,$dns.$url);
		curl_setopt($ch,CURLOPT_RETURNTRANSFER,true);
		curl_setopt($ch, CURLOPT_HEADER, false);
		curl_setopt($ch,CURLOPT_POST, count($data_json));
		curl_setopt($ch,CURLOPT_POSTFIELDS, $data_json);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		$result = curl_exec($ch);
		curl_close($ch);
		
		return $result;

	}
	
}	

function gen_device_id(){
		//génération d'un id de 6 caratères [a-zA-Z0-9] aléatoire
		$alpha = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9'];
		$res = "1";
		for($i=01;$i<6;$i++){
			$res .= $alpha[rand()%31];
		}
		
		return $res;
}

function get_device_name(){
		//on vient prendre le hostname comme nom pour le device en base
	$config=&$GLOBALS['config'];
	$hostname = $config['system']['hostname'];
	if( strlen($hostname) <= 0 ){
		update_output_window("Error : Hostname not found in configuration. Please set a Hostname in menu : System -> General Setup, and try installation again.");
		exit(1);
	}
	if( strlen($hostname) > 13 ){
		return substr($hostname,0,12);
	}
	return $hostname;
}

function get_device_platform(){
	return "pfsense";
}

function get_device_platform_bits(){
	return get_arch();
}

function get_device_platform_version(){
	return preg_match("/[0-9]{1,2}.[0-9]{1,2}/",trim(file_get_contents("/etc/version")));
}

function get_cpu_freq(){
	$out = shell_exec("sysctl -a | grep dev.cpu.0.freq");
	$out = explode(' ',$out);
	return preg_match('/[0-9]*/',trim(round($out[1]/1000)));
}

function get_ram_size(){
	$out = shell_exec("sysctl -a | grep hw.realmem");
	$out = explode(' ',$out);
	return preg_match('/[0-9]*/',trim(round($out[1]/1000000000)));
}

function get_hd_size(){
	$out = shell_exec("df -h");
	$out = explode("\n",$out);
        
	$index_line_disk = 0;
	$index_line_default = 0;
	foreach( $out as $line ){
		
		$line_expl = explode('   ',$line);
		if( strstr($line_expl[5],"/disk") ){
			$index_line_disk = array_search($line,$out);
		}
		
		if( strstr($line_expl[5],"/")){
			$index_line_default = array_search($line,$out);
		}
	}
	$resultat = 0;
	if( $index_line_disk != 0 ){
		$line = $out[$index_line_disk];
		$line_expl = explode('   ',$line);
		$resultat = $line_expl[1];
	}
	else{
		$line = $out[$index_line_default];
		$line_expl = explode('   ',$line);
		$resultat = $line_expl[1];
	}
	return preg_match('/[0-9]*/',trim($resultat));
}


function check_result($res){
	$fields = json_decode($res,true);
	foreach( $fields as $field ){
		if( preg_match("/ERR:[.]*/",$field) ){
			return false;
		}
	}
	return true;
}
	

function app_install(){
   
   $update = false;
   $canContinue = true;
   
   //si on est dans le cas d'une update
   if( file_exists('/tmp/appup.tgz') ){
	  $static_output .= gettext("Putting back configuration files after Apps by PacketApps update.\n");
	  update_output_window($static_output);
	  exec('tar -C /usr/local/appsbypacketapps -zxvf /tmp/appup.tgz');
	  unlink('/tmp/appup.tgz');
	  exec('ls -a /usr/local/appsbypacketapps/',$files);
	  if( in_array('config.xml',$files) && in_array('.device',$files) && in_array('logs',$files) && in_array('backups',$files)){
		 $update = true;
	  }
   }
   
   if( !$update ){
	  //sinon c'est une installation normale
   
	 /*
	  * Enregistrement du device
	  */
	  $datas = array();
	  $dns = "https://www.packetapps.com/php";
	  
	  //génrération du device id
	  $url = "/app/register/device/";
	  
	  $i=0;
	  do{
		 
		  $datas = array( 'id' => gen_device_id(),
				'name' => get_device_name(),
				'platform' => get_device_platform(),
				'platform_bits' => get_device_platform_bits(),
				'platform_version' => get_device_platform_version(),
				'ram' => get_ram_size(),
				'cpu' => get_cpu_freq(),
				'hd' => get_hd_size()
			);
		  
		  $static_output .= gettext("Registration of the device pending.\n");
		update_output_window($static_output);
	       $res = register_device($dns,$url,$datas);
		   $i++;
	  }while( !check_result($res) && $i < 5 );
	  
	  if( $i == 5 ){
		 $canContinue = false;
	  }
		
	  if( $canContinue ){
			//on commence ensuite à mettre en place les dossiers e fichiers utiles
		  if( !file_exists("/usr/local/appsbypacketapps/") || !file_exists("/usr/local/appsbypacketapps/.device/") ){
			mkdir("/usr/local/appsbypacketapps/.device/",0750,true);
		  }
		  
		  
		  //enregistrement de l'id du device et son code secret
		  file_put_contents("/usr/local/appsbypacketapps/.device/id",$datas['id']);
		  $static_output .= gettext("Registration of the device done.\n");
			update_output_window($static_output);
			
			
		  $static_output .= gettext("Apps configuration.\n");
		  update_output_window($static_output);
		  //mise en place du fichier de configuration
		  file_put_contents("/usr/local/appsbypacketapps/config.xml","<?xml version='1.0'?><apps><packages></packages></apps>");
		  
		  $static_output .= gettext("Apps logs folder creation.\n");
		  update_output_window($static_output);
		  //mise en place du dossier de log pour les packages installés
		  mkdir("/usr/local/appsbypacketapps/logs/",0750,true);
		  
		  $static_output .= gettext("Apps backups folder creation.\n");
			update_output_window($static_output);
			//mise en place du dossier de backup des configurations
			mkdir("/usr/local/appsbypacketapps/backups/",0750,true);
	  }
   }
   
   
   if( $canContinue ){
	  $static_output .= gettext("PHP Module Blenc installation.\n");
	  update_output_window($static_output);
	  /*
	  * Installation de BLENC
	  */
	  $source_lib = "/usr/local/appsbypacketapps/blenc";
	  $dest_lib = get_php_api_version();
	  #1 - vérification de l'architecture du système
	  $arch = get_arch();
	  if( $arch != "error" ){
		 if( $arch == "32" ){
			 $dest = $dest_lib."blenc.so";
			 $source = $source_lib."_i386.so";
		 }
		 if( $arch == "64" ){
			 $dest = $dest_lib."blenc.so";
			 $source = $source_lib."_amd64.so";
		 }
		 copy($source,$dest);
		 if( !file_exists($dest) ){
				 $static_output .= gettext("[App By PacketApps] - blenc lib copy error\n");
				 update_output_window($static_output);
				 exit(0);
		 }
		 else{
			 chmod($dest,666);
			 touch("/etc/php_dynamodules/blenc");
			 exec("/etc/rc.php_ini_setup &");
		 }
		 
	  }
	 
	  $static_output .= gettext("PHP Module Blenc installation done.\n");
	  update_output_window($static_output);
	 
	 
	  $static_output .= gettext("Pkg FreeBSD package manager installation.\n");
	  update_output_window($static_output);
	  //installation de pkg
	  exec("env ASSUME_ALWAYS_YES=YES pkg bootstrap");
	  $static_output .= gettext("Pkg FreeBSD package manager installation done.\n");
		update_output_window($static_output);
	  
	  
	  //dossier qui va contenir les logos des apps
	  mkdir('/usr/local/www/logo/',0750,true);
	  
	  $static_output .= gettext("Apps interface.\n");
		update_output_window($static_output);
	  /*
	   * Liens symboliques des fichiers utiles
	   */
	  //interface de Apps
	   symlink("/usr/local/appsbypacketapps/main","/usr/local/www/system_apps_by_packetapps.php");
	   chmod("/usr/local/appsbypacketapps/main",0755);
	   //Requête d'installation d'un package grâce au numéro de licence du produit
	   symlink("/usr/local/appsbypacketapps/send_license","/usr/local/www/system_apps_by_packetapps_send_license.php");
	   chmod("/usr/local/appsbypacketapps/send_license",0755);
	   //librairie d'installation d'un package de packetapps
	   symlink("/usr/local/appsbypacketapps/install_pkg","/usr/local/www/system_apps_by_packetapps_install_pkg.php");
	   chmod("/usr/local/appsbypacketapps/install_pkg",0755);
	   //requête de désinstallation d'un produit via son identifiant
	   symlink("/usr/local/appsbypacketapps/uninstall","/usr/local/www/system_apps_by_packetapps_uninstall.php");
	   chmod("/usr/local/appsbypacketapps/uninstall",0755);
	   symlink("/usr/local/appsbypacketapps/bootstrap","/usr/local/www/css/bootstrap.css");
	   chmod("/usr/local/appsbypacketapps/bootstrap",0755);
	   symlink("/usr/local/appsbypacketapps/logo.png","/usr/local/www/logo/logo.png");
	   chmod("/usr/local/appsbypacketapps/logo.png",0755);
	   
	   $static_output .= gettext("End of custom installation of Apps by PacketApps\n");
	   update_output_window($static_output);
	   shell_exec("/usr/local/appsbypacketapps/finish_blenc.sh > /dev/null 2>&1 &");
	   $static_output .= gettext("PHP restarting\n");
	   update_output_window($static_output);
   }
   else{
	  $static_output .= gettext("Connection to master server impossible. Please check your internet connection\n");
	  update_output_window($static_output);
	  //peut être à revoir à terme
	  uninstall_package('Apps by Packetappscom');
   }
}

function app_deinstall(){
   
   
   //d'abords on backup certaines info dans le cas ou on est en train d'update
   $static_output .= gettext("Saving configuration files\n");
   update_output_window($static_output);
   $app_files = array('.','..','install_pkg','main','check_associated','associate','uninstall','bootstrap.css','app.inc','app.xml','blenc_amd64.so','blenc_i386.so','finish_blenc.sh','logo.png','send_license','appup.tgz');
   //il va donc nous rester 'logs/' .device/ config.xml backups/ et les dossiers des packages installées 
   $files = scandir('/usr/local/appsbypacketapps/');
   chdir('/usr/local/appsbypacketapps/');
   $file_save = array();
   foreach( $files as $file ){
	  if( !in_array($file,$app_files)) {
		 $file_save[] = $file;
	  }
   }
   
   exec('tar -zcf /tmp/appup.tgz '.implode(' ',$file_save));
   
   
   //ensuite on peut démarrer la suppression des fichiers pour la désinstallation
   $static_output .= gettext("Deleting hardware informations of Apps by PacketApps\n");
   update_output_window($static_output);
	//suppression informations du device
	unlink("/usr/local/appsbypacketapps/.device/id");

	rmdir("/usr/local/appsbypacketapps/.device/");
	
	 $static_output .= gettext("Uninstall PacketApps packages of Apps by PacketApps\n");
	   update_output_window($static_output);
	//désinstallation de l'ensemble des fichiers des packages de PacketApps installés
	$config_app = simplexml_load_file("/usr/local/appsbypacketapps/config.xml");
	$pkgs = $config_app->packages;
	$pkgs = json_decode(json_encode($pkgs),true);
	include_once("/usr/local/appsbypacketapps/install_pkg");
	if( count($pkgs) > 0 ){
		foreach( $pkgs as $pkg ){
			deinstall_pkg($pkg['name']);
		}
	}
	
	$static_output .= gettext("Deleting configuration of Apps by PacketApps\n");
	   update_output_window($static_output);
	//suppression configuration de Apps
	unlink("/usr/local/appsbypacketapps/config.xml");
	
	$static_output .= gettext("Deleting Blenc PHP Module of Apps by PacketApps\n");
	   update_output_window($static_output);
	//suppression du module php blenc
	unlink("/etc/php_dynamodules/blenc");
	$arch = get_arch();
	$dest = get_php_api_version();
	unlink($dest."blenc.so");
	exec("/etc/rc.php_ini_setup &");
	
	$static_output .= gettext("Deleting web pages of Apps by PacketApps\n");
	   update_output_window($static_output);
	//suppression des liens symbolique des fichiers de Apps
	unlink("/usr/local/www/system_apps_by_packetapps.php");
	unlink("/usr/local/www/system_apps_by_packetapps_send_license.php");
	unlink("/usr/local/www/system_apps_by_packetapps_install_pkg.php");
	unlink("/usr/local/www/system_apps_by_packetapps_uninstall.php");
	unlink("/usr/local/www/css/bootstrap.css");
	unlink("/usr/local/www/logo/logo.png");
	//suppression des fichiers de Apps
	unlink("/usr/local/appsbypacketapps/main");
	unlink("/usr/local/appsbypacketapps/send_license");
	unlink("/usr/local/appsbypacketapps/install_pkg");
	unlink("/usr/local/appsbypacketapps/uninstall");
	unlink("/usr/local/appsbypacketapps/bootstrap.css");
	unlink("/usr/local/appsbypacketapps/logo.png");
	
	$static_output .= gettext("Deleting log files of Apps by PacketApps\n");
	   update_output_window($static_output);
	//suppression des logs
	$logs = scandir("/usr/local/appsbypacketapps/logs/");
	if( count($logs) > 0 ){
		foreach( $logs as $log ){
			if( $log !== '.' && $log !== '..' ){
				unlink($log);
			}
		}
	}
	rmdir("/usr/local/appsbypacketapps/logs/");
	
	$static_output .= gettext("Deleting backup files of Apps by PacketApps\n");
	   update_output_window($static_output);
	//suppression des fichiers de backup
	$bcks = scandir("/usr/local/appsbypacketapps/backups/");
	if( count($bcks) > 0 ){
		foreach( $bcks as $bck ){
		    if( $bck !== '.' && $bck !== '..' ){
				$bcks_pkg = scandir($bck);
				if( $bck !== '.' && $bck !== '..' ){
						foreach( $bcks_pkg as $pkg_bck ){
							unlink($pkg_bck);	
						}
						rmdir($bck);
				}
				
			}
		}
		
	}
	rmdir("/usr/local/appsbypacketapps/backups/");
	
	$static_output .= gettext("Deleting logo files of Apps by PacketApps\n");
	   update_output_window($static_output);
	//suppression des logos
	$logos = scandir("/usr/local/www/logo/");
	if( count($logos) > 0 ){
		foreach( $logos as $logo ){
			if( $logo !== '.' && $logo !== '..' ){
				unlink($logo);
			}
		}
	}
	rmdir("/usr/local/www/logo/");
	
	//restart php after finishing deinstall
	shell_exec("/usr/local/appsbypacketapps/finish_blenc.sh > /dev/null 2>&1 &"); 
	unlink('/usr/local/appsbypacketapps/finish_blenc.sh');
	$static_output .= gettext("PHP restarting\n");
	   update_output_window($static_output);
	   
	   
	//suppression du dossier racine de Apps
	exec("rm -R /usr/local/appsbypacketapps/");
	$static_output .= gettext("End of custom deinstallation of Apps by PacketApps\n");
	   update_output_window($static_output);
	   
    
	   
}
?>
