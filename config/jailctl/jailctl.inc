<?php
/* $Id$ */
include("jailctl-utils.inc");

if($_POST['xml']=='jailctl_settings.xml' && $_POST['jpasswd']) {
	$_POST['jpasswd']=crypt($_POST['jpasswd']);
} elseif($_POST['xml']=='jailctl_settings.xml' && !$_POST['jpasswd']) {
	$_POST['jpasswd']=$config['installedpackages']['pfjailctlsettings']['config'][0]['jpasswd'];
} 

if($_POST['xml']=='jailctl_settings.xml') {
	$_count=0;
	while($_count <= count(array_keys($_POST))) {
		if(isset($_POST['rcconfline'.$_count])) {
			$_POST['rcconfline'.$_count]=addcslashes($_POST['rcconfline'.$_count], '"');
		}
		$_count++;
	}	
} elseif($_GET['xml']=='jailctl_settings.xml') {
	$config['installedpackages']['pfjailctlsettings']['config'][0]['jpasswd']='';
} elseif($_POST['xml']=='jailctl.xml' && is_ipaddr($_POST['jif'])) {
	$_POST['jip']=$_POST['jif'];
} elseif($_POST['xml']=='jailctl.xml') {
	# Stuff goes here?
}


function jailctl_create($config, $jail) {
	$jstatus=jailctl_jailstatus($config, $jail);
	$jconfig=$config['installedpackages']['pfjailctlsettings']['config'][0];
	if($jail['jtemplates']) {
		jailctl_create_template($jail, $jstatus, $jconfig);
	} else {
		jailctl_create_sysinstall($jail, $jstatus, $jconfig);
	}
}

function jailctl_delete() {
	global $config, $jconfig, $jails;

	$jpath=$jconfig['jstorage']."/".$jails[$_GET['id']]['jname'];
	exec("rm -Rf '".$jpath."' >/dev/null 2>/dev/null &");
#	sync_package_jailctl();
}

function jailctl_deinstall() {
#	unset($GLOBALS['config']['installedpackages']['pfjailctlsettings']);
#	unset($GLOBALS['config']['installedpackages']['pfjailctl']);
}

function jailctl_install() {
	$jails=$GLOBALS['config']['installedpackages']['pfjailctl']['config'];
	for($i=0;$i<=count($jails);$i++) {
		if(is_array($jails[$i]) and ! $jails[$i]['jname'])
			unset($GLOBALS['config']['installedpackages']['pfjailctl']['config'][$i]);
	}

	if(!$GLOBALS['config']['installedpackages']['pfjailctlsettings']['config'][0]['jif']) {
		$settings['jif']='lan';
		$settings['jdnsservers']=$GLOBALS['config']['interfaces']['lan']['ipaddr'];
		$settings['jstorage']='/usr/local/jails';
		$settings['jbackup']=$settings['jstorage'];
		$settings['jbackupexcludes']='--exclude ./usr/ports/* --exclude ./tmp/* --exclude ./var/tmp/* --exclude ./usr/src/*';
		$settings['jrcconf']='sshd_enable="YES"';
		$settings['jpasswd']='$1$e4.8A4lV$oU.OQciTOnonltQkK12ff0';
		
		$GLOBALS['config']['installedpackages']['pfjailctlsettings']['config'][0]=$settings;
	}
}

function jailctl_servicestatus() {
	global $config;
	$jconfig=$config['installedpackages']['pfjailctlsettings']['config'][0];
	$jails=$config['installedpackages']['pfjailctl']['config'];
	is_array($jails) ? $num_jails=count($jails) : $num_jails=0;

	if($num_jails) {
		for ($i=0;$i<$num_jails;$i++) {
			$jname=$jails[$i]['jname'];
			$jstatus=Array();
			$_jcreate=Array();
			exec('/usr/local/sbin/jailctl status '.$jname, $jstatus);
	
			if (substr($jstatus[0], 0, strlen($jname))==$jname) {
				$_tmp=Array();
				ereg("is (.*)\.", array_shift($jstatus), $_tmp);
				switch ($_tmp[1]) {
					case 'up':
						break;
					default:
						return false;
				}
			} else {
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}

function jailctl_do_xmlrpc_sync($config) {
	$jails=$config['installedpackages']['pfjailctl']['config'][0];
	$jconfig=$config['installedpackages']['pfjailctlsettings']['config'][0];
	$sync=$config['installedpackages']['carpsettings']['config'][0];
	
	if(!$sync['synchronizetoip'] or !$sync['password'])
		return;

    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
	$synchronizetoip .= $sync['synchronizetoip'];

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['pfjailctl'] = $config['installedpackages']['pfjailctl'];
	$xml['pfjailctlsettings'] = $config['installedpackages']['pfjailctlsettings'];

	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($sync['password']),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning pfJailctl XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $sync['password']);
	/* send our XMLRPC message and timeout after 25 seconds */
	$resp = $cli->send($msg, "25");
	if(!$resp) {
		$error = "A communications error occured while attempting pfJailctl XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "pfJailctl Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting pfJailctl XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "pfJailctl Settings Sync", "");
	} else {
		log_error("pfJailctl XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	#echo "done.<br>";
}

function sync_package_jailctl() {
	global $config, $g;
	$jconfig=$config['installedpackages']['pfjailctlsettings']['config'][0];
	$jails=$config['installedpackages']['pfjailctl']['config'];
	is_array($jails) ? $num_jails=count($jails) : $num_jails=0;

	$jdefaultif=$config['interfaces']['lan']['if'];
	
	conf_mount_rw();
	config_lock();
	$fd = fopen("/usr/local/etc/jails.conf","w");
	
	fwrite($fd, 'IF="'.$jdefaultif.'"'."\n");
	fwrite($fd, 'JAIL_HOME="'.$jconfig['jstorage'].'/"'."\n");
	fwrite($fd, 'BACKUPDIR="'.$jconfig['jbackup'].'"'."\n");
	fwrite($fd, 'BACKUP_EXCLUDE="'.$jconfig['jbackupexcludes'].'"'."\n");
	fwrite($fd, 'INSTALLWORLD_FLAGS="'.$jconfig['jinstallflags'].'"'."\n");
	fwrite($fd, 'BATCH="'.$jconfig['jbatchcreate'].'"'."\n");
	fwrite($fd, 'ROOT_PW="'.$jconfig['jpasswd'].'"'."\n");
	fwrite($fd, 'NAMESERVERS="'.$jconfig['jdnsservers'].'"'."\n");
	fwrite($fd, 'RC_CONF=\''.$jconfig['jrcconf'].'\''."\n");

	fwrite($fd, "\nJAILS=''\n");

	$_rcfile['file']='jails.sh';
	$_rcfile['start']='';
	$_rcfile['stop']='';
	if($num_jails) {
		for ($i=0;$i<$num_jails;$i++) {
			$jname=$jails[$i]['jname'];
			$jpath=$jconfig['jstorage']."/".$jname."/";
			$jif=$jails[$i]['jif'];
			if(is_ipaddr($jif)) {
				$jip=$jif;
				#$jif=guess_interface_from_ip($jif);
				$jif=jailctl_find_if($jif);
			} else {
				$jif=$config['interfaces'][$jails[$i]['jif']]['if'];
				$jip=$jails[$i]['jip'];
			}
					
			$_cfg="JAILS=\"\$JAILS $jname:$jip;$jpath;$jif\"\n";
			fwrite($fd, $_cfg);
			
			if($jails[$i]['jautostart']=='on') {
				$_rcfile['start'].="/usr/local/sbin/jailctl start $jname \n\t";
				$_rcfile['stop'].="/usr/local/sbin/jailctl stop $jname \n\t";
			}
		}
	}
	
	write_rcfile($_rcfile);
	fclose($fd);
	jailctl_do_xmlrpc_sync($config);
}

?>
