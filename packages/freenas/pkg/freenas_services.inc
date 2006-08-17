<?php
/* $Id$ */
/*
	services.inc
	part of m0n0wall (http://m0n0.ch/wall)
	
	Copyright (C) 2003-2005 Manuel Kasper <mk@neon1.net>.
	All rights reserved.

	Modified for FreeNAS (http://freenas.org) by Olivier Cochard <cochard@gmail.com>
	
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

$freenas_config =& $config['installedpackages']['freenas']['config'][0];

function services_samba_configure() {
	global $freenas_config, $g;

	/* kill any running samba */
	
	killbyname("smbd");
	killbyname("winbindd");
	killbyname("nmbd");

	if (isset($freenas_config['samba']['enable']))
	{
	
		if ($g['booting'])
			echo "Starting Samba... ";

		/* generate smb.conf */
		$fd = fopen("{$g['varetc_path']}/smb.conf", "w");
		if (!$fd) {
			printf("Error: cannot open smb.conf in services_samba_configure().\n");
			return 1;
		}
		
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			return 0;

			
		$sambaconf = <<<EOD
[global]
encrypt passwords = yes
netbios name = {$freenas_config['samba']['netbiosname']}
workgroup = {$freenas_config['samba']['workgroup']}
server string = {$freenas_config['samba']['serverdesc']}
load printers = no
security = {$freenas_config['samba']['security']}
dns proxy = no
socket options = IPTOS_LOWDELAY TCP_NODELAY
EOD;

		if ($freenas_config['samba']['sndbuf']) 	{
			$sambaconf .= <<<EOD
 SO_SNDBUF={$freenas_config['samba']['sndbuf']}
EOD;
		}
		else 	{
			$sambaconf .= <<<EOD
 SO_SNDBUF=16384
EOD;
		}
		
		if ($freenas_config['samba']['rcvbuf']) 	{
			$sambaconf .= <<<EOD
 SO_RCVBUF={$freenas_config['samba']['rcvbuf']}
EOD;
		}
		else 	{
			$sambaconf .= <<<EOD
 SO_RCVBUF=16384
EOD;
		}
		
		if ($freenas_config['samba']['winssrv']) 	{
			$sambaconf .= <<<EOD

wins server = {$freenas_config['samba']['winssrv']}

EOD;
		}
		
		if (isset ($freenas_config['samba']['unixcharset'])) 		{
			$sambaconf .= <<<EOD

unix charset = {$freenas_config['samba']['unixcharset']}

EOD;
		}
		else {
			$sambaconf .= <<<EOD

unix charset = UTF-8

EOD;
		
		}
		
		
		
		$sambaconf .= <<<EOD

local master = {$freenas_config['samba']['localmaster']}
time server = {$freenas_config['samba']['timesrv']}
guest account = ftp
display charset = UTF-8
max log size = 100
#syslog only = yes
syslog = 3
load printers = no
printing = bsd
printcap name = /dev/null
disable spoolss = Yes
log level = {$freenas_config['samba']['loglevel']}
dos charset = {$freenas_config['samba']['doscharset']}
smb passwd file = {$g['varetc_path']}/private/smbpasswd
private dir = {$g['varetc_path']}/private

EOD;

	switch ($freenas_config['samba']['security']) 	{
	case "share":
		$sambaconf .= <<<EOD
create mask = 0666
directory mask = 0777
passdb backend = smbpasswd

EOD;
	
		$guestmode = <<<EOD
guest ok = yes

EOD;
		
		break;
	case "domain":
		$sambaconf .= <<<EOD
passdb backend = smbpasswd
allow trusted domains = No
dns proxy = No
ldap ssl = no
idmap backend = rid:{$freenas_config['samba']['workgroup']}=10000-20000
idmap uid = 10000-20000
idmap gid = 10000-20000
winbind enum groups = yes
winbind enum users = yes
winbind use default domain = Yes
template homedir = /mnt
template shell = /bin/sh

EOD;
	
		$guestmode = <<<EOD
guest ok = no

EOD;
		break;
	case "user":
		$sambaconf .= <<<EOD
passdb backend = smbpasswd

EOD;
		$guestmode = <<<EOD
guest ok = no

EOD;
		break;
	}
	
	
	
			foreach ($freenas_config['mounts']['mount'] as $mountent) {
				/* Unmount filesystem if not booting mode*/
				
				$sambaconf .= <<<EOD
			
[{$mountent['sharename']}]
comment = {$mountent['desc']}
path = /mnt/{$mountent['sharename']}
public = yes
writeable = yes
printable = no
veto files = /.snap/
{$guestmode}

EOD;

			if ( (is_array($freenas_config['samba']['hidemount']) && in_array($mountent['sharename'],$freenas_config['samba']['hidemount'])))
			
			$sambaconf .= <<<EOD
browseable = no

EOD;

			}
			
			if (isset($freenas_config['samba']['recyclebin'])) {
				$sambaconf .= <<<EOD
vfs object = recycle
recycle:repository = Recycle Bin
recycle:keeptree = Yes
recycle:versions = Yes

EOD;
			}

		fwrite($fd, $sambaconf);
		fclose($fd);

		/* run nmbd and nmbd */
		
		if (isset($freenas_config['ad']['enable'])) 	{
			mwexec("/usr/local/bin/net rpc join -S {$freenas_config['ad']['ad_srv_name']} -U {$freenas_config['ad']['admin_name']}%{$freenas_config['ad']['admin_pass']}");
		}

		mwexec("/usr/local/sbin/nmbd -D -s {$g['varetc_path']}/smb.conf");
		
		if (isset($freenas_config['ad']['enable'])) 	{
			mwexec("/usr/local/sbin/winbindd -s {$g['varetc_path']}/smb.conf");
			//mwexec("/usr/local/sbin/wbinfo --set-auth-user {$freenas_config['ad']['admin_name']}%{$freenas_config['ad']['admin_pass']}");
		}
		
		mwexec("/usr/local/sbin/smbd -D -s {$g['varetc_path']}/smb.conf");
		
		
		/* TEST: Wait for samba starting */
		sleep(2);

		/* Generate the samba password file */
		system_user_samba();

		if ($g['booting'])
			echo "done\n";
	}

	return 0;
}

function services_rsyncd_configure() {
	global $freenas_config, $g;

	/* kill any running rsync daemon */

	sigkillbypid("{$g['varrun_path']}/rsyncd.pid", "TERM");

	if (isset($freenas_config['rsyncd']['enable'])) {
	
		if ($g['booting'])
			echo "Starting RSYNC daemon... ";
		
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			return 1;

		/* generate rsyncd.conf */
		$fd = fopen("{$g['varetc_path']}/rsyncd.conf", "w");
		if (!$fd) 	{
			printf("Error: cannot open rsyncd.conf in services_rsyncd_configure().\n");
			return 1;
		}
			
		$rsyncdconf = <<<EOD
read only = {$freenas_config['rsyncd']['readonly']}
syslog facility = local4
list = yes
port = {$freenas_config['rsyncd']['port']}
pid file = {$g['varrun_path']}/rsyncd.pid

EOD;

		if (!empty($freenas_config['rsyncd']['maxcon'])) {
		$rsyncdconf .= <<<EOD
max connections = {$freenas_config['rsyncd']['maxcon']}

EOD;
		}
		
		if (!empty($freenas_config['rsyncd']['motd'])) {
		$rsyncdconf .= <<<EOD
motd file = {$g['varetc_path']}/rsyncd.motd

EOD;
		}
		
		if (isset($freenas_config['rsyncd']['rsyncd_user'])) 	{
			$rsyncdconf .= <<<EOD
uid = {$freenas_config['rsyncd']['rsyncd_user']}

EOD;

		}
		else 	{
			$rsyncdconf .= <<<EOD
uid = ftp
gid = ftp

EOD;
		}


	
		foreach ($freenas_config['mounts']['mount'] as $mountent) 	{
			/* Unmount filesystem if not booting mode*/
				
			$rsyncdconf .= <<<EOD
			
[{$mountent['sharename']}]
comment = {$mountent['desc']}
path = /mnt/{$mountent['sharename']}

EOD;
		}

		fwrite($fd, $rsyncdconf);
		fclose($fd);
		
		// Generate MOTD file if configured
		
		if (!empty($freenas_config['rsyncd']['motd'])) {
			$fd = fopen("{$g['varetc_path']}/rsyncd.motd", "w");
			if (!$fd) 	{
				printf("Error: cannot open rsyncd.motd in services_rsyncd_configure().\n");
				return 1;
			}
			
			$motd  = <<<EOD
{$freenas_config['rsyncd']['motd']}

EOD;
				
			fwrite($fd, $motd);
			fclose($fd);				
		}

		/* run rsync Daemon */
		mwexec("/usr/local/bin/rsync --daemon --config={$g['varetc_path']}/rsyncd.conf");


		if ($g['booting'])
			echo "done\n";
	}

	return 0;
}

function services_nfs_configure() {
	global $freenas_config, $g;

	/* kill any running nfsd */
	
	killbyname("rpc.statd");
	killbyname("rpc.lockd");
	killbypid("{$g['varrun_path']}/mountd.pid");
	forcekillbyname("nfsd");
	killbyname("rpcbind");

	
	if (isset($freenas_config['nfs']['enable']))
	{
	
		if ($g['booting'])
			echo "Starting NFS... ";
		
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			return 1;

		/* generate exports */
		$fd = fopen("{$g['varetc_path']}/exports", "w");
		if (!$fd) {
			printf("Error: cannot open exports in services_nfs_configure().\n");
			return 1;
		}
		
		list($network,$subnet) = 
		explode('/', $freenas_config['nfs']['nfsnetwork']);
		
		$subnet=gen_subnet_mask($subnet);
		
		$a_mount = &$freenas_config['mounts']['mount'];
		
		foreach ($a_mount as $mount)
		{
			$nfsconf .= <<<EOD
/mnt/{$mount['sharename']} -alldirs -maproot=root 
EOD;

			if ($freenas_config['nfs']['mapall'] == "yes")
			{
			
			$nfsconf .= <<<EOD
-mapall=root 
EOD;
			
			}
			
			$nfsconf .= <<<EOD
-network {$network} -mask $subnet

EOD;
			
		}
		fwrite($fd, $nfsconf);
		fclose($fd);

		/* run rpcbind, nfsd and mountd */
		mwexec("/usr/sbin/rpcbind");
		mwexec("/usr/sbin/nfsd -u -t -n 4");
		mwexec("/usr/sbin/mountd -r {$g['varetc_path']}/exports");
		mwexec("/usr/sbin/rpc.lockd");
		mwexec("/usr/sbin/rpc.statd");


		if ($g['booting'])
			echo "done\n";
	}

	return 0;
	
}

function services_ftpd_configure() {
	global $freenas_config, $g;
	// services_vsftpd_configure();
	// services_pureftpd_configure();
        services_wzdftpd_configure();
}

function services_wzdftpd_configure() {
	global $freenas_config, $g;
        
	/* kill any WZDFTPD */
	killbyname("wzdftpd");

	if (isset($freenas_config['ftp']['enable'])) {
	
		if ($g['booting'])
			echo "Starting FTP server... ";

                if (! file_exists("/var/log/wzdftpd"))
                        mkdir("/var/log/wzdftpd");
                        
                if (! file_exists("{$g['varetc_path']}/wzdftpd"))
                        mkdir("{$g['varetc_path']}/wzdftpd");

		/* generate wzd.cfg */
		$fd = fopen("{$g['varetc_path']}/wzdftpd/wzd.cfg", "w");
		if (!$fd) {
			printf("Error: cannot open vsftpd.conf in services_vsftpd_configure().\n");
			return 1;
		}
                
                $ftpconf = <<<EOD
[GLOBAL]
config version = 0.7.2
backup config = true
port = {$freenas_config['ftp']['port']}

EOD;

		if ($freenas_config['ftp']['pasv_min_port'] && $freenas_config['ftp']['pasv_max_port']) {
			$ftpconf .= <<<EOD
pasv_low_range={$freenas_config['ftp']['pasv_min_port']}
pasv_high_range={$freenas_config['ftp']['pasv_max_port']}

EOD;
		}
                
                $ftpconf .= <<<EOD

pasv_low_range = 2500
pasv_high_range  = 3000
pasv_ip = {$freenas_config['ftp']['pasv_address']}
dir_message = .message
logfile = /var/log/wzdftpd/wzd.log
max_threads = 32

backend = ${datarootdir}/wzdftpd/backends/libwzdplaintext.so > 124
backend = ${datarootdir}/wzdftpd/backends/libwzdpam.so

max_ul_speed = 0
max_dl_speed = 0
deny_access_files_uploaded = 1

tls_certificate = /usr/local/etc/wzdftpd/wzd.pem
sitefile_ginfo  = /usr/local/etc/wzdftpd/file_ginfo.txt
sitefile_group  = /usr/local/etc/wzdftpd/file_group.txt
sitefile_user   = /usr/local/etc/wzdftpd/file_user.txt

[pre_ip_check]
localhost = allow

[perms]
site_addip = +O +G
site_adduser = +O +G
site_backend = +O
site_chacl = +O
site_change = +O +G
site_changegrp = +O +G
site_checkperm = +O
site_chgrp = +O
site_chmod = +O
site_chown = +O
site_chpass = *
site_chratio = +O +G
site_close = +O
site_color = !=guest *
site_delip = +O +G
site_deluser = +O +G
site_flags = -admin
site_free = *
site_ginfo = +O +G
site_give = *
site_group = +O
site_groups = +O
site_grpadd = +O
site_grpaddip = +O
site_grpchange = +O
site_grpdel = +O
site_grpdelip = +O
site_grpkill = +O
site_grpratio = +O
site_grpren = +O
site_gsinfo = +O +G
site_help = *
site_idle = *
site_invite = !=guest *
site_kick = +O
site_kill = +O
site_link = +O
site_msg = +O
site_perm = +O
site_purge = +O +G
site_readd = +O +G
site_reload = +O
site_reopen = +O
site_rules = *
site_rusage = +O
site_savecfg = +O
site_sections = +O
site_shutdown = +O
site_su = +O
site_swho = +O
site_tagline = !=guest *
site_take = +O
site_unlock = +O
site_uptime = *
site_user = +O +G
site_users = -admin
site_utime = *
site_vars = +O
site_vars_group = +O
site_vars_user = +O
site_version = +O
site_who = !=guest *
site_wipe = +O
site_vfsls = +O
site_vfsadd = +O
site_vfsdel = +O

[modules]
${datarootdir}/wzdftpd/modules/libwzd_debug.so = deny
${datarootdir}/wzdftpd/modules/libwzd_test.so = deny
${datarootdir}/wzdftpd/modules/libwzd_sfv.so = allow
${datarootdir}/wzdftpd/modules/libwzd_tcl.so = deny
${datarootdir}/wzdftpd/modules/libwzd_perl.so = deny

EOD;

                if (empty($config['system']['zeroconf_disable'])) {
                        $ftpconf .= <<<EOD
${datarootdir}/wzdftpd/modules/libwzd_zeroconf.so = allow

[ZEROCONF]
zeroconf_port = {$freenas_config['ftp']['port']}
zeroconf_username = root
zeroconf_path = /

EOD;

                }
                
                $ftpconf .= <<<EOD
[sfv]
progressmeter = [WzD] - %3d%% Complete - [WzD]
del_progressmeter = \[.*] - ...% Complete - \[WzD]
incomplete_indicator = ../(incomplete)-%0
other_completebar = [WzD] - ( %.0mM %fF - COMPLETE ) - [WzD]

[plaintext]
param = /usr/local/etc/wzdftpd/users

[sections]
ALL = /* ^([]\[A-Za-z0-9_.'() \t+-])*$

[cron]

[events]

[custom_commands]
site_rules = !/usr/local/etc/wzdftpd/file_rules.txt

site_groups = !/usr/local/etc/wzdftpd/file_groups.txt
site_help = !/usr/local/etc/wzdftpd/file_help.txt
site_swho = !/usr/local/etc/wzdftpd/file_swho.txt
site_users = !/usr/local/etc/wzdftpd/file_users.txt
site_vfsls = !/usr/local/etc/wzdftpd/file_vfs.txt
site_who = !/usr/local/etc/wzdftpd/file_who.txt

[vfs]

[messages]

EOD;

                if (isset($freenas_config['ftp']['banner'])) 	{
                        $ftpconf .= "200 = {$freenas_config['ftp']['banner']}";
                }
                
		fwrite($fd, $ftpconf);
		fclose($fd);

		/* run vsftpd */
		mwexec("/usr/local/sbin/wzdftpd -f {$g['varetc_path']}/wzdftpd/wzd.cfg");


		if ($g['booting'])
			echo "done\n";
	}
        
        return 0;
}

function services_vsftpd_configure() {
	global $freenas_config, $g;

	/* kill any VSFTPD */
	killbyname("vsftpd");

	if (isset($freenas_config['ftp']['enable'])) {
	
		if ($g['booting'])
			echo "Starting FTP server... ";

		/* generate vfstpd.conf */
		$fd = fopen("{$g['varetc_path']}/vsftpd.conf", "w");
		if (!$fd) {
			printf("Error: cannot open vsftpd.conf in services_vsftpd_configure().\n");
			return 1;
		}
			
		$ftpconf = <<<EOD
# Standalone mode
listen=YES
background=YES
listen_port={$freenas_config['ftp']['port']}
max_clients={$freenas_config['ftp']['numberclients']}
max_per_ip={$freenas_config['ftp']['maxconperip']}
xferlog_enable=YES
#vsftpd_log_file=/var/log/ftp.log
syslog_enable=YES
# Access rights
anonymous_enable={$freenas_config['ftp']['anonymous']}
local_enable={$freenas_config['ftp']['localuser']}
write_enable=YES
hide_file=.snap
anon_other_write_enable=YES
anon_upload_enable=YES
anon_mkdir_write_enable=YES
chroot_local_user=YES
local_umask=066
anon_umask=066
chmod_enable=YES
pasv_address={$freenas_config['ftp']['pasv_address']}
# Features
ls_recurse_enable=YES
# Performance
idle_session_timeout={$freenas_config['ftp']['timeout']}

EOD;
		if (isset($freenas_config['ftp']['banner'])) 	{
			$ftpconf .= <<<EOD
ftpd_banner={$freenas_config['ftp']['banner']}

EOD;
		}
		else {
		$ftpconf .= <<<EOD
ftpd_banner=Welcome to FreeNAS FTP service

EOD;
		}
		
		if ($freenas_config['ftp']['pasv_min_port'] && $freenas_config['ftp']['pasv_max_port']) {
			$ftpconf .= <<<EOD
pasv_max_port={$freenas_config['ftp']['pasv_max_port']}
pasv_min_port={$freenas_config['ftp']['pasv_min_port']}

EOD;
		}
		
		if ($freenas_config['ftp']['pasv_address']) {
			$ftpconf .= <<<EOD
pasv_address={$freenas_config['ftp']['pasv_address']}

EOD;
		}
		
		fwrite($fd, $ftpconf);
		fclose($fd);

		/* run vsftpd */
		mwexec("/usr/local/sbin/vsftpd {$g['varetc_path']}/vsftpd.conf");


		if ($g['booting'])
			echo "done\n";
	}

	return 0;
}

function services_pureftpd_configure() {

	// Stop the pure-ftpd process and re-start it
	
	global $freenas_config, $g;

	/* kill any Pure-FTPD */
	killbyname("pure-ftpd");

	if (isset($freenas_config['ftp']['enable'])) {
	
		if ($g['booting'])
			echo "Starting FTP server... ";
			
		/* Generate the banner file */
			
		$fd = fopen("/mnt/.banner", "w");
		if (!$fd) {
			printf("Error: cannot open .banner in services_pureftpd_configure().\n");
			return 1;
		}
		
		if (empty($freenas_config['ftp']['banner'])) {
			$banner = <<<EOD
Welcome to FreeNAS FTP service
	
EOD;
		} else {
			$banner = <<<EOD
{$freenas_config['ftp']['banner']}
	
EOD;
		}
			
		fwrite($fd, $banner);
		fclose($fd);
			
			
		/* run pure-ftpd */
		
		$cmd = "/usr/local/sbin/pure-ftpd -S {$freenas_config['ftp']['port']} -c {$freenas_config['ftp']['numberclients']} -C {$freenas_config['ftp']['maxconperip']} -I {$freenas_config['ftp']['timeout']} -M U 077:077 -B -b ";
		
		if ( (isset($freenas_config['ftp']['anonymous'])) && (!isset($freenas_config['ftp']['localuser'])))
			$cmd .= "-e ";
			
		if ( (!isset($freenas_config['ftp']['anonymous'])) && (isset($freenas_config['ftp']['localuser'])))
			$cmd .= "-E ";
		
		if ($freenas_config['ftp']['pasv_min_port'] && $freenas_config['ftp']['pasv_max_port'])
			$cmd .= "-p {$freenas_config['ftp']['pasv_min_port']}:{$freenas_config['ftp']['pasv_max_port']} ";
			
		if ($freenas_config['ftp']['pasv_address'])
			$cmd .= "-P {$freenas_config['ftp']['pasv_address']} ";
			
		if ($freenas_config['ftp']['natmode'])
			$cmd .= "-N ";
			
		if ($freenas_config['ftp']['passiveip'])
			$cmd .= "-P {$freenas_config['ftp']['passiveip']} ";
			
		if ($freenas_config['ftp']['fxp'])
			$cmd .= "-W ";
		
		
		mwexec($cmd);


		if ($g['booting'])
			echo "done\n";
	}
			
		
	return 0;
}

function services_smart_configure() {
	global $config, $g;
	
	/* kill any running smartd */
	killbyname("smartd");

	if (isset($config['system']['smart'])) {
	
		if ($g['booting'])
			echo "Starting smartd... ";

		/* run smartd */
		mwexec("/usr/local/sbin/smartd --logfacility=local5");

		if ($g['booting'])
			echo "done\n";
	}

	return 0;
}

function services_cron_configure() {
	global $freenas_config, $g;
	
	/* kill any running cron */
	killbyname("cron");
			
	/* generate crontab */
	$fd = fopen("{$g['varetc_path']}/crontab", "w");
	
	$crontabconf = <<<EOD
SHELL=/bin/sh
PATH=/etc:/bin:/sbin:/usr/bin:/usr/sbin
HOME=/var/log
#
#minute hour    mday    month   wday    who     command
#

EOD;

	if (!$fd) 	{
		printf("Error: cannot open crontab in services_cron_configure().\n");
		return 1;
	}

	if (isset($freenas_config['rsyncclient']['enable'])) {
		
		if ($g['booting'])
			echo "Configuring cron for rsync client... ";
		
		if ($freenas_config['rsyncclient']['all_mins'] == "0") 	{ 
			foreach ($freenas_config['rsyncclient']['minute'] as $minutev) 	{
				if (strlen($cron_min) > 0)
					$cron_min = $cron_min . "," . $minutev;
				else
					$cron_min = $minutev;
			}
		}
		else
			$cron_min = "*";
	
	
		if ($freenas_config['rsyncclient']['all_hours'] == "0") { 
			foreach ($freenas_config['rsyncclient']['hour'] as $hourv) 	{
				if (strlen($cron_hour) > 0)
					 $cron_hour = $cron_hour . "," . $hourv;
				else
					 $cron_hour = $hourv;
			}
		}
		else
			$cron_hour = "*";
		
		if ($freenas_config['rsyncclient']['all_days'] == "0") 	{ 
			foreach ($freenas_config['rsyncclient']['day'] as $dayv) 	{
				if (strlen($cron_day) > 0)
					$cron_day = $cron_day . "," . $dayv;
				else
					$cron_day = $dayv;
			}
		}
		else
			$cron_day = "*";
		
		if ($freenas_config['rsyncclient']['all_months'] == "0") 	{
		  foreach ($freenas_config['rsyncclient']['month'] as $monthv) 	  {
			if (strlen($cron_month) > 0)
				$cron_month = $cron_month . "," . $monthv;
			else
				$cron_month = $monthv;
		  }
		}
		else
			$cron_month = "*";
		
		
		if ($freenas_config['rsyncclient']['all_weekday'] == "0") 	{
			foreach ($freenas_config['rsyncclient']['weekday'] as $weekdayv) 	{
				if (strlen($cron_weekday) > 0)
					 $cron_weekday = $cron_weekday . "," . $weekdayv;
				else
					 $cron_weekday = $weekdayv;
			}
		}
		else
			$cron_weekday = "*";		
		
		$crontabconf .= <<<EOD
{$cron_min}    {$cron_hour}        {$cron_day}        {$cron_month}        {$cron_weekday}       root    {$g['varrun_path']}/sync.sh

EOD;

		if ($g['booting'])
			echo "done\n";
	}
	
	// Erase all variable used previously
	unset ($cron_min, $cron_hour, $cron_day, $cron_month, $cron_weekday);
		
	if (isset($freenas_config['shutdown']['enable'])) {
		
		if ($g['booting'])
			echo "Configuring cron for shutdown... ";
		
		if ($freenas_config['shutdown']['all_mins'] == "0") { 
			foreach ($freenas_config['shutdown']['minute'] as $minutev) 	{
				if (strlen($cron_min) > 0)
					$cron_min = $cron_min . "," . $minutev;
				else
					$cron_min = $minutev;
			}
		}
		else
			$cron_min = "*";
	
	
		if ($freenas_config['shutdown']['all_hours'] == "0") 	{ 
			foreach ($freenas_config['shutdown']['hour'] as $hourv) 	{
				if (strlen($cron_hour) > 0)
					 $cron_hour = $cron_hour . "," . $hourv;
				else
					 $cron_hour = $hourv;
			}
		}
		else
			$cron_hour = "*";
		
		if ($freenas_config['shutdown']['all_days'] == "0") { 
			foreach ($freenas_config['shutdown']['day'] as $dayv) 	{
				if (strlen($cron_day) > 0)
					$cron_day = $cron_day . "," . $dayv;
				else
					$cron_day = $dayv;
			}
		}
		else
			$cron_day = "*";
		
		if ($freenas_config['shutdown']['all_months'] == "0") 	{
			foreach ($freenas_config['shutdown']['month'] as $monthv)   {
				if (strlen($cron_month) > 0)
					$cron_month = $cron_month . "," . $monthv;
				else
					$cron_month = $monthv;
			}
		}
		else
			$cron_month = "*";
		
		
		if ($freenas_config['shutdown']['all_weekday'] == "0") 	{
			foreach ($freenas_config['shutdown']['weekday'] as $weekdayv) 		{
				if (strlen($cron_weekday) > 0)
					 $cron_weekday = $cron_weekday . "," . $weekdayv;
				else
					 $cron_weekday = $weekdayv;
			}
		}
		else
			$cron_weekday = "*";		
		
		$crontabconf .= <<<EOD
{$cron_min}    {$cron_hour}        {$cron_day}        {$cron_month}        {$cron_weekday}       root    /etc/rc.shutdown

EOD;

		if ($g['booting'])
			echo "done\n";
	}
				
	fwrite($fd, $crontabconf);
	fclose($fd);

	/* run cron */
	mwexec("/usr/sbin/cron -s");
	
	return 0;
	
}

function services_rsyncclient_configure()
{
	global $freenas_config, $g;

	// Generate a shell script that is used by cron
	// Generate crontab works
	
	if (isset($freenas_config['rsyncclient']['enable']))
	{	
	
		if ($g['booting'])
			echo "Generating synchronizing cron script... ";
		
		/* generate /var/run/sync.sh script */
		$fd = fopen("{$g['varrun_path']}/sync.sh", "w");
		if (!$fd)
		{
			printf("Error: cannot open /var/run/sync.sh in services_rsyncclient_configure().\n");
			return 1;
		}

	$syncscript = <<<EOD
#!/bin/sh

if [ -r /var/run/rsync-running ]; then
exit
fi
/usr/bin/touch /var/run/rsync-running

EOD;

		$opt_delete="";
		if (isset($freenas_config['rsyncclient']['opt_delete']))
			$opt_delete = "--delete --delete-after ";
	
		foreach ($freenas_config['rsyncclient']['sharetosync'] as $sharek => $sharev)
		{
			$syncscript .= <<<EOD
/usr/local/bin/rsync -rtz {$opt_delete}rsync://{$freenas_config['rsyncclient']['rsyncserverip']}/{$sharev} /mnt/{$sharev}

EOD;
		}
	
		$syncscript .= <<<EOD
/bin/rm -f /var/run/rsync-running

EOD;

		fwrite($fd,$syncscript);
		fclose($fd);
		
		/* Add exectutive right to the script */
		exec("/bin/chmod 770 {$g['varrun_path']}/sync.sh");
		
		if ($g['booting'])
			echo "done\n";
		return 0;
	}
	
	return 0;
}

function services_afpd_configure()
{
	global $freenas_config, $config, $g;

	/* kill any VSFTPD */
	killbyname("afpd");

	if (isset($freenas_config['afp']['enable'])) 	{
	
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			return 1;
			
		if ($g['booting'])
			echo "Starting AFP server... ";
			
		if (empty($freenas_config['afp']['afpname']))
			$afpname="-";
		else
			$afpname=$freenas_config['afp']['afpname'];
			
		/* generate afpd.conf */
		$fd = fopen("{$g['varetc_path']}/afpd.conf", "w");
		if (!$fd) 		{
			printf("Error: cannot open afpd.conf in services_afpd_configure().\n");
			return 1;
		}
                
		$afpconf = <<<EOD
{$afpname} -transall
EOD;

                if (empty($config['system']['zeroconf_disable'])) {
                        $afpconf .= <<<EOD
 -noslp -fqdn {$config['system']['hostname']} -uamlist
EOD;
                } else {
                        $afpconf .= <<<EOD
 -uamlist
EOD;
                }

		if (isset($freenas_config['afp']['guest']) && isset($freenas_config['afp']['local']) )
			$afpconf .= <<<EOD
 uams_guest.so,uams_clrtxt.so,uams_dhx.so
EOD;
		else if (isset($freenas_config['afp']['guest']))
			$afpconf .= <<<EOD
 uams_guest.so
EOD;
		else if (isset($freenas_config['afp']['local']))
			$afpconf .= <<<EOD
 uams_clrtxt.so,uams_dhx.so
EOD;
		
		$afpconf .= <<<EOD
 -nosavepassword -defaultvol {$g['varetc_path']}/AppleVolumes.default -systemvol {$g['varetc_path']}/AppleVolumes.system -uampath /etc/uams -guestname ftp

EOD;
		fwrite($fd, $afpconf);
		fclose($fd);
		
		/* generate AppleVolumes.default */
		$fd = fopen("{$g['varetc_path']}/AppleVolumes.default", "w");
		if (!$fd)
		{
			printf("Error: cannot open AppleVolumes.default in services_afpd_configure().\n");
			return 1;
		}
		
		$a_mount = &$freenas_config['mounts']['mount'];
		foreach ($a_mount as $mount)
		{
			$AppleVolumes .= <<<EOD
/mnt/{$mount['sharename']} "{$mount['sharename']}" 

EOD;
		
		}
		
		fwrite($fd, $AppleVolumes);
		fclose($fd);


		/* run afpdpd */
		mwexec("/usr/local/sbin/afpd -F {$g['varetc_path']}/afpd.conf");


		if ($g['booting'])
			echo "done\n";
	}

	return 0;
}

function services_nis_configure()
{
	global $freenas_config, $g;

	return 0;
}

function services_iscsi_configure()
{
	global $freenas_config, $g;
	
	if (isset($freenas_config['iscsi']['enable']))
	{
		if ($g['booting'])
			echo "Starting iSCSI driver... ";
			
		mwexec("/sbin/kldload kdload /boot/kernel/iscsi_initiator.ko");
		mwexec("/sbin/sysctl debug.iscsi=0");
		mwexec("/usr/local/sbin/iscontrol targetaddress={$freenas_config['iscsi']['targetaddress']} TargetName={$freenas_config['iscsi']['targetname']}");
		
		if ($g['booting'])
			echo "done\n";
	}
	
	return 0;
}

function service_zeroconf_configure() {
        //services_mdnsresponder_configure();
        //services_howl_configure();
        services_avahi_configure();
}

function services_avahi_configure() {
	global $freenas_config, $config, $g;
        
        $service_template = <<<EOD
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>

  <name replace-wildcards="yes">@SERVICE_DESC@ on %h</name>

  <service>
    <type>@TYPE@</type>
    <port>@PORT@</port>
    @TXT_RECORDS@
  </service>

</service-group>

EOD;
	
	/* kill any running dbus daemon */
        sigkillbypid("{$g['varrun_path']}/dbus/dbus.pid", "TERM");
        
	/* kill any running avahi daemons */
        mwexec("/usr/local/sbin/avahi-daemon -k");


	if (isset($config['system']['zeroconf_disable']))
		return 0;
	
        if (! file_exists("/usr/local/etc/avahi/services"))
                mwexec("mkdir -p /usr/local/etc/avahi/services");
	
        if (isset($config['system']['webgui']['port'])) {
                $webservice = str_replace("@PORT@",
                                          $config['system']['webgui']['port'],
                                          $service_template);
                $webservice = str_replace("@TYPE@",
                                          "_http._tcp",
                                          $webservice);
                $webservice = str_replace("@SERVICE_DESC@",
                                          "pfSense webGUI",
                                          $webservice);
                $webservice = str_replace("@TXT_RECORDS@",
                                          "",
                                          $webservice);
                                          
                file_put_contents("/usr/local/etc/avahi/services/http.service", $webservice);
        } else {
                $webservice = str_replace("@PORT@",
                                          "80",
                                          $service_template);
                $webservice = str_replace("@TYPE@",
                                          "_http._tcp",
                                          $webservice);
                $webservice = str_replace("@SERVICE_DESC@",
                                          "pfSense webGUI",
                                          $webservice);
                $webservice = str_replace("@TXT_RECORDS@",
                                          "",
                                          $webservice);
                                          
                file_put_contents("/usr/local/etc/avahi/services/http.service", $webservice);
        }
        

	
	if (isset($freenas_config['afp']['enable'])) {
                /* NOP */
	}

	if (isset($freenas_config['rsyncd']['enable'])) 	{				
                $rsyncservice = str_replace("@PORT@",
                                          $config['rsyncd']['port'],
                                          $service_template);
                $rsyncservice = str_replace("@TYPE@",
                                          "_rsync._tcp",
                                          $rsyncservice);
                $rsyncservice = str_replace("@SERVICE_DESC@",
                                          "Remote Sync Daemon",
                                          $rsyncservice);
                $rsyncservice = str_replace("@TXT_RECORDS@",
                                          "",
                                          $rsyncservice);
                                          
                file_put_contents("/usr/local/etc/avahi/services/rsync.service", $rsyncservice);
	}

	if (isset($config['sshd']['enable'])) 	{				
                $sshservice = str_replace("@PORT@",
                                          "22",
                                          $service_template);
                $sshservice = str_replace("@TYPE@",
                                          "_ssh._tcp",
                                          $sshservice);
                $sshservice = str_replace("@SERVICE_DESC@",
                                          "Remote Terminal",
                                          $sshservice);
                $sshservice = str_replace("@TXT_RECORDS@",
                                          "",
                                          $sshservice);
                                          
                file_put_contents("/usr/local/etc/avahi/services/ssh.service", $sshservice);
	}
	
	if (isset($freenas_config['ftp']['enable'])) {
                /* NOP */
	}
	
	if (isset($freenas_config['samba']['enable'])) 	{				
                $cifsservice = str_replace("@PORT@",
                                          "139",
                                          $service_template);
                $cifsservice = str_replace("@TYPE@",
                                          "_smb._tcp",
                                          $cifsservice);
                $cifsservice = str_replace("@SERVICE_DESC@",
                                          "Samba Server",
                                          $cifsservice);
                $cifsservice = str_replace("@TXT_RECORDS@",
                                          "",
                                          $cifsservice);
                                          
                file_put_contents("/usr/local/etc/avahi/services/cifs.service", $cifsservice);
	}
	
	if (isset($freenas_config['nfs']['enable'])) {				
		
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			break;
		
		$a_mount = &$freenas_config['mounts']['mount'];
		
		foreach ($a_mount as $mount) {
                        $sharename = str_replace(" ", "", $mount['sharename']);
                        $sharename = strtolower($sharename);
                        
                        $nfsservice = str_replace("@PORT@",
                                                  "2049",
                                                  $service_template);
                        $nfsservice = str_replace("@TYPE@",
                                                  "_nfs._tcp",
                                                  $nfsservice);
                        $nfsservice = str_replace("@SERVICE_DESC@",
                                                  "NFS Mount: {$mount['sharename']}",
                                                  $nfsservice);
                        $nfsservice = str_replace("@TXT_RECORDS@",
                                                  "path=/mnt/{$mount['sharename']}",
                                                  $nfsservice);
                                                  
                        file_put_contents("/usr/local/etc/avahi/services/{$sharename}_nfs.service", $nfsservice);
		}
	}


	/* run dbus */
	mwexec("/usr/local/bin/dbus-daemon --system");
        
        /* run avahi */
        mwexec("/usr/local/sbin/avahi-daemon -D");

	if ($g['booting'])
		echo "done\n";
	
	return 0;
}

function services_howl_configure()
{
	global $freenas_config, $config, $g;
	
	
	/* kill any mDNSResponder */
	killbyname("mDNSResponder");
	
	if (isset($config['system']['zeroconf_disable']))
		return 0;
	
	$fd = fopen("{$g['varetc_path']}/mDNSResponder.conf", "w");
	if (!$fd)
	{
		printf("Error: cannot open mDNSResponder.conf in services_howl_configure().\n");
		return 1;
	}
	
	if ($config['system']['webgui']['port'])
	{
	$mDNSResponder = <<<EOD
"{$config['system']['hostname']} Web Admin"	_http._tcp	local.	{$config['system']['webgui']['port']}

EOD;
	}
	else
	{
	$mDNSResponder = <<<EOD
"{$config['system']['hostname']} Web Admin"	_http._tcp	local.	80

EOD;
	}
	
	
	if (isset($freenas_config['afp']['enable']))
	{				
		$mDNSResponder .= <<<EOD
"{$config['system']['hostname']} AFP Server"	_afpovertcp._tcp	local.	548

EOD;
	}
	
	if (isset($freenas_config['rsyncd']['enable']))
	{				
		$mDNSResponder .= <<<EOD
"{$config['system']['hostname']} RSYNC Server"	_rsync._tcp.	local.	{$freenas_config['rsyncd']['port']}

EOD;
	}
	
	if (isset($config['sshd']['enable']))
	{				
		$mDNSResponder .= <<<EOD
"{$config['system']['hostname']} SSH Server"	_ssh._tcp	local.	{$config['sshd']['port']}

EOD;
	}
	
	if (isset($freenas_config['ftp']['enable']))
	{				
		$mDNSResponder .= <<<EOD
"{$config['system']['hostname']} FTP Server"	_ftp._tcp	local.	{$freenas_config['ftp']['port']}

EOD;
	}
	
	if (isset($freenas_config['samba']['enable']))
	{				
		$mDNSResponder .= <<<EOD
"{$config['system']['hostname']} Samba Server"	_smb._tcp	local.	139

EOD;
	}
	
	if (isset($freenas_config['nfs']['enable']))
	{				
		
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			break;
		
		$a_mount = &$freenas_config['mounts']['mount'];
		
		foreach ($a_mount as $mount)
		{
		$mDNSResponder .= <<<EOD
"{$config['system']['hostname']} NFS: {$mount['sharename']}"	_nfs._tcp	local.	2049 path=/mnt/{$mount['sharename']}

EOD;
		}
	}
	
	fwrite($fd, $mDNSResponder);
	fclose($fd);


	/* run mDNSResponder */
	mwexec("/usr/local/sbin/mDNSResponder -f {$g['varetc_path']}/mDNSResponder.conf");

	if ($g['booting'])
		echo "done\n";
	
	return 0;
}

function services_mdnsresponder_configure()
{
	global $freenas_config, $config, $g;
	
	
	/* kill any running snmpd */
	sigkillbypid("{$g['varrun_path']}/mDNSResponder.pid", "TERM");

	
	if (isset($config['system']['zeroconf_disable']))
		return 0;
	
	$fd = fopen("{$g['varetc_path']}/mDNSResponder.conf", "w");
	if (!$fd) 	{
		printf("Error: cannot open mDNSResponder.conf in services_bonjour_configure().\n");
		return 1;
	}
	
	if ($config['system']['webgui']['port']) 	{
	$mDNSResponder = <<<EOD
"{$config['system']['hostname']} Web Admin"
_http._tcp local.
{$config['system']['webgui']['port']}

EOD;
	}
	else 	{
	$mDNSResponder = <<<EOD
"{$config['system']['hostname']} Web Admin"
_http._tcp local.
80

EOD;
	}
	
	
	if (isset($freenas_config['afp']['enable'])) 	{				
		$mDNSResponder .= <<<EOD

"{$config['system']['hostname']} AFP Server"
_afpovertcp._tcp local.
548

EOD;
	}
	
	if (isset($freenas_config['rsyncd']['enable'])) 	{				
		$mDNSResponder .= <<<EOD

"{$config['system']['hostname']} RSYNC Server"
_rsync._tcp. local.
{$freenas_config['rsyncd']['port']}

EOD;
	}
	
	if (isset($config['sshd']['enable'])) 	{				
		$mDNSResponder .= <<<EOD

"{$freenas_config['system']['hostname']} SSH Server"
_ssh._tcp local.
{$config['sshd']['port']}

EOD;
	}
	
	if (isset($freenas_config['ftp']['enable'])) 	{				
		$mDNSResponder .= <<<EOD

"{$config['system']['hostname']} FTP Server"
_ftp._tcp local.
{$freenas_config['ftp']['port']}

EOD;
	}
	
	if (isset($freenas_config['samba']['enable'])) 	{				
		$mDNSResponder .= <<<EOD

"{$config['system']['hostname']} Samba Server"
_smb._tcp local.	
139

EOD;
	}
	
	if (isset($freenas_config['nfs']['enable'])) 	{				
		
		/* If no share configured, exit */
		if (!is_array($freenas_config['mounts']['mount']))
			break;
		
		$a_mount = &$freenas_config['mounts']['mount'];
		
		foreach ($a_mount as $mount) 		{
		$mDNSResponder .= <<<EOD

"{$config['system']['hostname']} NFS: {$mount['sharename']}"
_nfs._tcp local.
2049 path=/mnt/{$mount['sharename']}

EOD;
		}
	}
	
	fwrite($fd, $mDNSResponder);
	fclose($fd);


	/* run mDNSResponder */
	mwexec("/usr/local/sbin/mDNSResponderPosix -b -f {$g['varetc_path']}/mDNSResponder.conf");

	if ($g['booting'])
		echo "done\n";
	
	return 0;
}
?>