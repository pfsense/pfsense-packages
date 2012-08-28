<?php

if(!isset($_GET['id']) and !isset($_POST['id'])) {
	if($GLOBALS['config']['installedpackages']['tinchosts']['savemsg']) {
		$savemsg=$GLOBALS['config']['installedpackages']['tinchosts']['savemsg'];
		unset($GLOBALS['config']['installedpackages']['tinchosts']['savemsg']);
		write_config();
	}
	
}

$hosts=$config['installedpackages']['tinchosts']['config'];
is_array($hosts) ? $num_hosts=count($hosts) : $num_hosts=0;
if(!isset($_GET['id']) and !isset($_POST['id']) and $num_hosts) {
	for ($i=0;$i<$num_hosts;$i++) {
		$host=$hosts[$i];
	}
}


function tinc_save() {
	$GLOBALS['config']['installedpackages']['tinchosts']['savemsg']='';
	conf_mount_rw();
	config_lock();
	exec("/bin/mv -f /usr/local/etc/tinc /usr/local/etc/tinc.old");
	safe_mkdir("/usr/local/etc/tinc");
	safe_mkdir("/usr/local/etc/tinc/hosts");
	exec("touch /usr/local/etc/tinc/WARNING-ENTIRE_DIRECTORY_ERASED_ON_SAVE_FROM_GUI");
	$tincconf = $GLOBALS['config']['installedpackages']['tinc']['config'][0];
	$fout = fopen("/usr/local/etc/tinc/tinc.conf","w");
	fwrite($fout, "name=".$tincconf['name']."\n");
	fwrite($fout, "AddressFamily=".$tincconf['addressfamily']."\n");
	if(!is_array($GLOBALS['config']['installedpackages']['tinchosts']['config'])) { $GLOBALS['config']['installedpackages']['tinchosts']['config']=Array(); }
	foreach($GLOBALS['config']['installedpackages']['tinchosts']['config'] as $host) {
		if($host['connect'])
		{
			fwrite($fout, "ConnectTo=" . $host['name'] . "\n");
		}
		
		$_output = "Address=".$host['address']."\n";
		$_output .= "Subnet=".$host['subnet']."\n";
		$_output .= base64_decode($host['extra'])."\n";
		$_output .= base64_decode($host['cert_pub'])."\n";
		file_put_contents('/usr/local/etc/tinc/hosts/'.$host['name'],$_output);
		if($host['host_up'])
		{
			file_put_contents('/usr/local/etc/tinc/hosts/'.$host['name'].'-up',base64_decode($host['host_up'])."\n");
			chmod('/usr/local/etc/tinc/hosts/'.$host['name'].'-up', 0744);
		}
		if($host['host_down'])
		{
			file_put_contents('/usr/local/etc/tinc/hosts/'.$host['name'].'-down',base64_decode($host['host_down'])."\n");
			chmod('/usr/local/etc/tinc/hosts/'.$host['name'].'-down', 0744);
		}
	}
	fwrite($fout, base64_decode($tincconf['extra'])."\n");
	fclose($fout);
	$_output = "Subnet=" . $tincconf['localsubnet'] . "\n";
	$_output .= base64_decode($tincconf['host_extra']) . "\n";
	$_output .= base64_decode($tincconf['cert_pub']) . "\n";
	file_put_contents('/usr/local/etc/tinc/hosts/' . $tincconf['name'],$_output);
	file_put_contents('/usr/local/etc/tinc/rsa_key.priv',base64_decode($tincconf['cert_key'])."\n");
	chmod("/usr/local/etc/tinc/rsa_key.priv", 0600);
	if($tincconf['tinc_up'])
	{
		$_output = base64_decode($tincconf['tinc_up']) . "\n";
	}
	else
	{
		$_output = "ifconfig \$INTERFACE " . $tincconf['localip'] . " netmask " . $tincconf['vpnnetmask'] . "\n";
	}
	file_put_contents('/usr/local/etc/tinc/tinc-up',$_output);
	chmod("/usr/local/etc/tinc/tinc-up", 0744);
	if($tincconf['tinc_down'])
	{
		file_put_contents('/usr/local/etc/tinc/tinc-down',base64_decode($tincconf['tinc_down']) . "\n");
		chmod("/usr/local/etc/tinc/tinc-down", 0744);
	}
	if($tincconf['host_up'])
	{
		file_put_contents('/usr/local/etc/tinc/host-up',base64_decode($tincconf['host_up']) . "\n");
		chmod("/usr/local/etc/tinc/host-up", 0744);
	}
	if($tincconf['host_down'])
	{
		file_put_contents('/usr/local/etc/tinc/host-down',base64_decode($tincconf['host_down']) . "\n");
		chmod("/usr/local/etc/tinc/host-down", 0744);
	}
	if($tincconf['subnet_up'])
	{
		file_put_contents('/usr/local/etc/tinc/subnet-up',base64_decode($tincconf['subnet_up']) . "\n");
		chmod("/usr/local/etc/tinc/subnet-up", 0744);
	}
	if($tincconf['subnet_down'])
	{
		file_put_contents('/usr/local/etc/tinc/subnet-down',base64_decode($tincconf['subnet_down']) . "\n");
		chmod("/usr/local/etc/tinc/subnet-down", 0744);
	}
	system("/usr/local/etc/rc.d/tinc.sh restart 2>/dev/null");
	rmdir_recursive("/usr/local/etc/tinc.old");
	conf_mount_ro();
	config_unlock();
}

function tinc_status_1() {
	list($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$sizeb,$atime,$mtimeb,$ctime,$blksize,$blocks)=stat("/var/log/tinc.log");
	exec("/usr/local/sbin/tincd --config=/usr/local/etc/tinc -kUSR1");
	exec("tail -c +" . $sizeb . " /var/log/tinc.log | sed -e 's/.*tinc\[.*\]: //'",$result);
	$output="";
	foreach($result as $line)
	{
		$output .= $line . "\n";
	}
	return $output;
}

function tinc_status_2() {
	list($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$sizeb,$atime,$mtimeb,$ctime,$blksize,$blocks)=stat("/var/log/tinc.log");
	exec("/usr/local/sbin/tincd --config=/usr/local/etc/tinc -kUSR2");
	exec("tail -c +" . $sizeb . " /var/log/tinc.log | sed -e 's/.*tinc\[.*\]: //'",$result);
	$output="";
	foreach($result as $line)
	{
		$output .= $line . "\n";
	}
	return $output;
}

function tinc_install() {
	safe_mkdir("/usr/local/etc/tinc");
	safe_mkdir("/usr/local/etc/tinc/hosts");
#	make_dirs("/var/tmp/tinc/var/tmp/run/tinc");
#	system("/usr/sbin/chown -R tinc:tinc /var/tmp/tinc");
	$_rcfile['file']='tinc.sh';
	$_rcfile['start'].="/usr/local/sbin/tincd --config=/usr/local/etc/tinc\n\t";
	$_rcfile['stop'].="/usr/local/sbin/tincd --kill \n\t";
	write_rcfile($_rcfile);
	unlink_if_exists("/usr/local/etc/rc.d/tincd");
	
	conf_mount_rw();
	config_lock();
/*
       # find next free optional interface number 
                for ($i = 1; $i <= count($GLOBALS['config']['interfaces']); $i++) {
                        if (!$GLOBALS['config']['interfaces']["opt{$i}"])
                                break;
                }
                $newifname = 'opt' . $i;
                $descr = "TINC";
                $GLOBALS['config']['interfaces'][$newifname] = array();
                $GLOBALS['config']['interfaces'][$newifname]['descr'] = $descr;
                                                
        uksort($GLOBALS['config']['interfaces'], "compare_interface_friendly_names");
                                         
        $GLOBALS['config']['interfaces'][$newifname]['if'] = "tun0";
*/

        /* XXX: Do not remove this. */
        mwexec("/bin/rm -f /tmp/config.cache");

        write_config();

	conf_mount_ro();
	config_unlock();
}

function tinc_deinstall() {
	rmdir_recursive("/var/tmp/tinc");
	rmdir_recursive("/usr/local/etc/tinc*");
	unlink_if_exists("/usr/local/etc/rc.d/tinc.sh");
}

?>
