<?php

function tinc_save() {
	global $config;
	conf_mount_rw();
	config_lock();
	exec("/bin/mv -f /usr/local/etc/tinc /usr/local/etc/tinc.old");
	safe_mkdir("/usr/local/etc/tinc");
	safe_mkdir("/usr/local/etc/tinc/hosts");
	exec("touch /usr/local/etc/tinc/WARNING-ENTIRE_DIRECTORY_ERASED_ON_SAVE_FROM_GUI");
	$tincconf = &$config['installedpackages']['tinc']['config'][0];
	$fout = fopen("/usr/local/etc/tinc/tinc.conf","w");

	// No proper config, bail out.
	if (!isset($tincconf['name']) || empty($tincconf['name']))
		return;

	fwrite($fout, "name=".$tincconf['name']."\n");
	fwrite($fout, "AddressFamily=".$tincconf['addressfamily']."\n");
	if(!is_array($config['installedpackages']['tinchosts']['config'])) { $config['installedpackages']['tinchosts']['config']=Array(); }
	foreach($config['installedpackages']['tinchosts']['config'] as $host) {
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
			file_put_contents('/usr/local/etc/tinc/hosts/'.$host['name'].'-up',str_replace("\r", "", base64_decode($host['host_up']))."\n");
			chmod('/usr/local/etc/tinc/hosts/'.$host['name'].'-up', 0744);
		}
		if($host['host_down'])
		{
			file_put_contents('/usr/local/etc/tinc/hosts/'.$host['name'].'-down',str_replace("\r", "", base64_decode($host['host_down']))."\n");
			chmod('/usr/local/etc/tinc/hosts/'.$host['name'].'-down', 0744);
		}
	}
	fwrite($fout, base64_decode($tincconf['extra'])."\n");
	fclose($fout);

	// Check if we need to generate a new RSA key pair.
	if ($tincconf['gen_rsa'])
	{
		safe_mkdir("/usr/local/etc/tinc/tmp");
		exec("/usr/local/sbin/tincd -c /usr/local/etc/tinc/tmp -K");
		$tincconf['cert_pub'] = base64_encode(file_get_contents('/usr/local/etc/tinc/tmp/rsa_key.pub'));
		$tincconf['cert_key'] = base64_encode(file_get_contents('/usr/local/etc/tinc/tmp/rsa_key.priv'));
		$tincconf['gen_rsa'] = false;
		$config['installedpackages']['tinc']['config'][0]['cert_pub'] = $tincconf['cert_pub'];
		$config['installedpackages']['tinc']['config'][0]['cert_key'] = $tincconf['cert_key'];
		$config['installedpackages']['tinc']['config'][0]['gen_rsa'] = $tincconf['gen_rsa'];
		rmdir_recursive("/usr/local/etc/tinc/tmp");
		write_config();
	}

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
		$_output .= "ifconfig \$INTERFACE group tinc\n";
	}
	file_put_contents('/usr/local/etc/tinc/tinc-up',$_output);
	chmod("/usr/local/etc/tinc/tinc-up", 0744);
	if($tincconf['tinc_down'])
	{
		file_put_contents('/usr/local/etc/tinc/tinc-down',str_replace("\r", "", base64_decode($tincconf['tinc_down'])) . "\n");
		chmod("/usr/local/etc/tinc/tinc-down", 0744);
	}
	if($tincconf['host_up'])
	{
		file_put_contents('/usr/local/etc/tinc/host-up',str_replace("\r", "", base64_decode($tincconf['host_up'])) . "\n");
		chmod("/usr/local/etc/tinc/host-up", 0744);
	}
	if($tincconf['host_down'])
	{
		file_put_contents('/usr/local/etc/tinc/host-down',str_replace("\r", "", base64_decode($tincconf['host_down'])) . "\n");
		chmod("/usr/local/etc/tinc/host-down", 0744);
	}
	if($tincconf['subnet_up'])
	{
		file_put_contents('/usr/local/etc/tinc/subnet-up',str_replace("\r", "", base64_decode($tincconf['subnet_up'])) . "\n");
		chmod("/usr/local/etc/tinc/subnet-up", 0744);
	}
	if($tincconf['subnet_down'])
	{
		file_put_contents('/usr/local/etc/tinc/subnet-down',str_replace("\r", "", base64_decode($tincconf['subnet_down'])) . "\n");
		chmod("/usr/local/etc/tinc/subnet-down", 0744);
	}
	system("/usr/local/etc/rc.d/tinc.sh restart 2>/dev/null");
	rmdir_recursive("/usr/local/etc/tinc.old");

	conf_mount_ro();
	config_unlock();
}

function tinc_install() {
	global $config;
	safe_mkdir("/usr/local/etc/tinc");
	safe_mkdir("/usr/local/etc/tinc/hosts");
	$_rcfile['file']='tinc.sh';
	$_rcfile['start'].="/usr/local/sbin/tincd --config=/usr/local/etc/tinc\n\t";
	$_rcfile['stop'].="/usr/local/sbin/tincd --kill \n\t";
	write_rcfile($_rcfile);
	unlink_if_exists("/usr/local/etc/rc.d/tincd");
	clear_log_file("/var/log/tinc.log");
	
	conf_mount_rw();
	config_lock();

        /* Create Interface Group */
        if (!is_array($config['ifgroups']['ifgroupentry']))
        $config['ifgroups']['ifgroupentry'] = array();

        $a_ifgroups = &$config['ifgroups']['ifgroupentry'];
        $ifgroupentry = array();
        $ifgroupentry['members'] = '';
        $ifgroupentry['descr'] = 'tinc mesh VPN interface group';
        $ifgroupentry['ifname'] = 'tinc';
        $a_ifgroups[] = $ifgroupentry;

        /* XXX: Do not remove this. */
        mwexec("/bin/rm -f /tmp/config.cache");

        write_config();

	conf_mount_ro();
	config_unlock();
}

function tinc_deinstall() {
	global $config;
        /* Remove Interface Group */
        conf_mount_rw();
        config_lock();
        if (!is_array($config['ifgroups']['ifgroupentry']))
        $config['ifgroups']['ifgroupentry'] = array();

        $a_ifgroups = &$config['ifgroups']['ifgroupentry'];

        $myid=-1;
        $i = 0;
        foreach ($a_ifgroups as $ifgroupentry)
        {
                if($ifgroupentry['ifname']=='tinc')
                {
                        $myid=$i;
                        break;
                }
                $i++;
        }

        if ($myid >= 0 && $a_ifgroups[$myid])
        {
                $members = explode(" ", $a_ifgroups[$_GET['id']]['members']);
                foreach ($members as $ifs)
                {
                        $realif = get_real_interface($ifs);
                        if ($realif)
                                mwexec("/sbin/ifconfig  {$realif} -group " . escapeshellarg($a_ifgroups[$_GET['id']]['ifname']));
                }
                unset($a_ifgroups[$myid]);
        	mwexec("/bin/rm -f /tmp/config.cache");
        	write_config();
        }
	conf_mount_ro();
	config_unlock();

	rmdir_recursive("/var/tmp/tinc");
	rmdir_recursive("/usr/local/etc/tinc*");
	unlink_if_exists("/usr/local/etc/rc.d/tinc.sh");
}

?>
