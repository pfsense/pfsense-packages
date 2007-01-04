<?php
/* $Id$ */
/* ========================================================================== */
/*
    freenas_system.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2006 Daniel S. Haischt <me@daniel.stefan.haischt.name>
    All rights reserved.

    Based on FreeNAS (http://www.freenas.org)
    Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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
/* ========================================================================== */

require_once("globals.inc");

$freenas_config =& $config['installedpackages']['freenas']['config'][0];

function system_do_extensions($early = false) {
  global $config, $g;

  if (!is_dir("{$g['etc_path']}/inc/ext")) { return; }

  $dh = @opendir("{$g['etc_path']}/inc/ext");
  
  if ($dh) {
    while (($extd = readdir($dh)) !== false) {
      if (($extd === ".") || ($extd === "..")) { continue; }
      
      $rcfile = "{$g['etc_path']}/inc/ext/" . $extd . "/" . ($early ? "rc.early" : "rc");

      if (file_exists($rcfile)) { passthru($rcfile); }
    } // end while
    
    closedir($dh);
  } // end if
}

function system_set_termcap() {
  global $config;

  if (isset($config['diag']['ipfstatentries'])) {
    $lines = $config['diag']['ipfstatentries'] + 6;
  } else {
    $lines = 306;
  }

  /* do not remove the tabs below or replace them using whitespaces */
  $termcap = <<<EOD
cons25w|ansiw|ansi80x25-raw:\
	:am:bs:NP:ms:pt:AX:eo:bw:ut:km:\
	:co#80:li#25:pa#64:Co#8:it#8:\
	:al=\E[L:cd=\E[J:ce=\E[K:cl=\E[H\E[J:cm=\E[%i%d;%dH:\
	:dc=\E[P:dl=\E[M:do=\E[B:bt=\E[Z:ho=\E[H:ic=\E[@:cb=\E[1K:\
	:nd=\E[C:rs=\Ec:so=\E[7m:se=\E[27m:up=\E[A:cr=^M:ta=^I:\
	:AF=\E[3%dm:AB=\E[4%dm:op=\E[39;49m:sc=\E7:rc=\E8:\
	:k1=\E[M:k2=\E[N:k3=\E[O:k4=\E[P:k5=\E[Q:k6=\E[R:k7=\E[S:k8=\E[T:\
	:k9=\E[U:k;=\E[V:F1=\E[W:F2=\E[X:K2=\E[E:nw=\E[E:ec=\E[%dX:\
	:kb=^H:kh=\E[H:ku=\E[A:kd=\E[B:kl=\E[D:kr=\E[C:le=^H:sf=\E[S:sr=\E[T:\
	:kN=\E[G:kP=\E[I:@7=\E[F:kI=\E[L:kD=\\177:kB=\E[Z:\
	:IC=\E[%d@:DC=\E[%dP:SF=\E[%dS:SR=\E[%dT:AL=\E[%dL:DL=\E[%dM:\
	:DO=\E[%dB:LE=\E[%dD:RI=\E[%dC:UP=\E[%dA:cv=\E[%i%dd:ch=\E[%i%d`:\
	:mb=\E[5m:md=\E[1m:mr=\E[7m:me=\E[m:bl=^G:\
	:ve=\E[=S:vi=\E[=1S:vs=\E[=2S:
cons25|ansis|ansi80x25:\
	:ac=l\\332m\\300k\\277j\\331u\\264t\\303v\\301w\\302q\\304x\\263n\\305`^Da\\260f\\370g\\361~\\371.^Y-^Xh\\261i^U0\\333y\\363z\\362:\
	:tc=cons25w:
dumb|su|unknown:\
	:am:co#132:li#$lines:do=^J:
xterm-noapp|xterm with cursor keys in normal mode:\
	:kl=\E[D:kd=\E[B:kr=\E[C:ku=\E[A:ks=\E=:ke=\E>:ti@:te@:tc=xterm:
xterm|xterm-color|X11 terminal emulator:\
	:ti@:te@:tc=xterm-xfree86:
xterm-xfree86|XFree86 xterm:\
	:k1=\EOP:k2=\EOQ:k3=\EOR:k4=\EOS:\
	:k5=\E[15~:k6=\E[17~:k7=\E[18~:k8=\E[19~:\
	:k9=\E[20~:k;=\E[21~:F1=\E[23~:F2=\E[24~:\
	:kH=\EOF:@7=\EOF:kI=\E[2~:\
	:kh=\EOH:*6=\EOF:kP=\E[5~:kN=\E[6~:\
	:ku=\EOA:kd=\EOB:kr=\EOC:kl=\EOD:Km=\E[M:tc=xterm-basic:
xterm-basic|xterm common (XFree86):\
	:li#24:co#80:am:kn#12:km:mi:ms:xn:bl=^G:\
	:is=\E[!p\E[?3;4l\E[4l\E>:rs=\E[!p\E[?3;4l\E[4l\E>:le=^H:\
	:AL=\E[%dL:DL=\E[%dM:DC=\E[%dP:al=\E[L:dc=\E[P:dl=\E[M:\
	:UP=\E[%dA:DO=\E[%dB:LE=\E[%dD:RI=\E[%dC:\
	:ho=\E[H:cd=\E[J:ce=\E[K:cl=\E[H\E[2J:cm=\E[%i%d;%dH:cs=\E[%i%d;%dr:\
	:im=\E[4h:ei=\E[4l:ks=\E[?1h\E=:ke=\E[?1l\E>:kD=\E[3~:kb=^H:\
	:sf=\n:sr=\EM:st=\EH:ct=\E[3g:sc=\E7:rc=\E8:\
	:eA=\E(B\E)0:as=^N:ae=^O:ml=\El:mu=\Em:up=\E[A:nd=\E[C:\
	:md=\E[1m:me=\E[m^O:mr=\E[7m:so=\E[7m:se=\E[27m:us=\E[4m:ue=\E[24m:\
	:ti=\E[?1049h:te=\E[?1049l:vi=\E[?25l:ve=\E[?25h:\
	:ut:Co#8:pa#64:op=\E[39;49m:AB=\E[4%dm:AF=\E[3%dm:\

EOD;

  if (!file_exists("/usr/share/misc")) { mkdir("/usr/share/misc"); }

  $fd = @fopen("/usr/share/misc/termcap", "w");
  
  if (!$fd) {
    printf("Error: cannot open termcap in system_set_termcap().\n");
    return 1;
  }
  
  chmod("/usr/share/misc/termcap", 0644);
  fwrite($fd, $termcap);
  fclose($fd);

  return 0;
}

/* TODO: Figure out whether this function can be removed cause
 *       implements its own user management framework.
 */
function system_users_create() {
  /* Create All the passwd file */
  global $config, $g;

  if ($g['booting']) { echo "Generating user database... "; }

  system_user_masterpasswd();
  system_user_group();
  system_user_pwdmkdb();

  if ($g['booting']) { echo "done\n"; }

  return 0;
}

/* TODO: Figure out whether this function can be removed cause
 *       implements its own user management framework.
 */
function system_user_masterpasswd() {
  /* Create the master.passwd file*/
  global $config, $g, $userindex, $groupindex;
  $root = getUNIXRoot();

  $masterpasswd = <<<EOD
root:{$root['password']}:0:0::0:0:Charlie &:/root:/bin/sh
toor:*:0:0::0:0:Bourne-again Superuser:/root:
daemon:*:1:1::0:0:Owner of many system processes:/root:/usr/sbin/nologin
operator:*:2:5::0:0:System &:/:/usr/sbin/nologin
bin:*:3:7::0:0:Binaries Commands and Source:/:/usr/sbin/nologin
tty:*:4:65533::0:0:Tty Sandbox:/:/usr/sbin/nologin
kmem:*:5:65533::0:0:KMem Sandbox:/:/usr/sbin/nologin
smmsp:*:25:25::0:0:Sendmail Submission User:/var/spool/clientmqueue:/usr/sbin/nologin
mailnull:*:26:26::0:0:Sendmail Default User:/var/spool/mqueue:/usr/sbin/nologin
proxy:*:62:62::0:0:Packet Filter pseudo-user:/nonexistent:/usr/sbin/nologin
_pflogd:*:64:64::0:0:pflogd privsep user:/var/empty:/usr/sbin/nologin
www:*:80:80::0:0:World Wide Web Owner:/nonexistent:/usr/sbin/nologin
nobody:*:65534:65534::0:0:Unprivileged user:/nonexistent:/usr/sbin/nologin
dhcpd:*:1002:1002::0:0:DHCP Daemon:/nonexistent:/sbin/nologin
_dhcp:*:65:65::0:0:dhcp programs:/var/empty:/usr/sbin/nologin
_isakmpd:*:68:68::0:0:isakmpd privsep:/var/empty:/sbin/nologin
ftp:*:21:50::0:0:FTP user:/mnt:/sbin/nologin
sshd:*:22:22::0:0:Secure Shell Daemon:/var/empty:/usr/sbin/nologin

EOD;

  if (is_array($config['system']['user'])) {
    foreach ($config['system']['user'] as $user) {
      $password= $user['password'];
      $groupname = $user['groupname'];
      $group =& $config['system']['group'][$groupindex[$groupname]];

      if (empty($user['uid'])) {
        $newuser = assignUID($user['name']);
        $newgroup = assignGID($groupname);
        if (! empty($newuser)) { $user = $newuser; }
        if (! empty($newgroup)) { $group = $newgroup; }
      }

      if (hasShellAccess($user['name'])) {
        $masterpasswd .= <<<EOD
{$user['name']}:{$password}:{$user['uid']}:{$group['gid']}::0:0:{$user['fullname']}:/mnt:/etc/rc.initial

EOD;
      } else {
        $masterpasswd .= <<<EOD
{$user['name']}:{$password}:{$user['uid']}:{$group['gid']}::0:0:{$user['fullname']}:/mnt:/usr/local/bin/scponly

EOD;
      } // end if
    } // end foreach
  } // end if

  $fd = fopen("/etc/master.passwd", "w");
  
  if (!$fd) {
    printf("Error: cannot open master.passwd in system_user_masterpasswd().\n");
    return 1;
  }

  fwrite($fd, $masterpasswd);
  fclose($fd);

return 0;
}

/* TODO: Figure out whether this function can be removed cause
 *       implements its own user management framework.
 */
function system_user_group() {
  /* Create the group file*/
  global $config, $g;

  $groupfile = <<<EOD
wheel:*:0:root
EOD;

  /* If user exist with full shell, put them on the wheel group */
  if (is_array($config['system']['user'])) {
    foreach ($config['system']['user'] as $user) {
      if (hasShellAccess($user['name']) && isSystemAdmin($user['name'])) {
        $groupfile .= <<<EOD
,{$user['name']}
EOD;
      } // end if
    } // end foreach
  } // end if

  $groupfile .= <<<EOD

daemon:*:1:
kmem:*:2:
sys:*:3:
tty:*:4:
operator:*:5:root
mail:*:6:
bin:*:7:
staff:*:20:
EOD;

  /* If user exist without full shell, put them on the staff group */
  if (is_array($config['system']['user'])) {
    foreach ($config['system']['user'] as $user) {
      if (hasShellAccess($user['name'])) {
        $groupfile .= <<<EOD
{$user['name']},
EOD;
      } // end if
    } // end foreach
  } // end if

  $groupfile .= <<<EOD

sshd:*:22:
smmsp:*:25:
mailnull:*:26:
guest:*:31:
proxy:*:62:
_pflogd:*:64:
_dhcp:*:65:
ftp:*:50:
authpf:*:63:
network:*:69:
www:*:80:
nogroup:*:65533:
nobody:*:65534:
admin:*:0:

EOD;

  if (is_array($config['system']['group'])) {
    foreach ($config['system']['group'] as $group) {
      if (empty($group['gid'])) {
        $newgroup = assignGID($group['name']);
        if (! empty($newgroup)) { $group = $newgroup; }
      }

      $groupfile .= <<<EOD
{$group['name']}:*:{$group['gid']}:

EOD;
    } // end foreach
  } // end if

  $fd = fopen("/etc/group", "w");
  
  if (!$fd) {
    printf("Error: cannot open group in system_user_group().\n");
    return 1;
  }

  fwrite($fd, $groupfile);
  fclose($fd);

  return 0;
}

function system_user_pwdmkdb() {
  /* Generate the db of password */
  global $config, $g;

  mwexec("/usr/sbin/pwd_mkdb -p -d /etc /etc/master.passwd");

  return 0;
}

function system_user_samba() {
  /* Generate the db of password */
  
  // TODO: MUST FIND A WAY OF USING UNIX CRYPTED PASSWORD IN THE PLACE OF CLEAR TEXT PASSWORD FOR GENERATING SAMBA DB!!!
  
  global $config, $g;
  
  if (is_array($config['system']['user'])) {
    foreach ($config['system']['user'] as $user) {
      /* TODO: the password in config.xml is already encrypted */
      $password = escapeshellcmd($user['password']);
      $login = escapeshellcmd($user['name']);
      mwexec("(/bin/echo {$password}; /bin/echo {$password}) | /usr/local/bin/smbpasswd -s -a {$login}");
      //mwexec("(/bin/echo {$password}; /bin/echo {$password}) | /usr/local/bin/pdbedit -tau {$login}");
    } // end foreach
  } // end if

  return 0;

}

function system_pam_configure() {
  /* Create the pam configuration files*/
  global $config, $g;

  if (!file_exists("{$g['varetc_path']}/pam.d")) {
    mkdir("{$g['varetc_path']}/pam.d", 0744);
  }

  $system = <<<EOD
# System-wide defaults
# auth
auth            sufficient      pam_opie.so             no_warn no_fake_prompts
auth            requisite       pam_opieaccess.so       no_warn allow_local

EOD;

  if (isset($config['ad']['enable'])) {
    $system .= <<<EOD
#auth	sufficient		/usr/local/lib/pam_winbind.so	debug try_first_pass

EOD;
  } 

  $system .= <<<EOD
auth    required		pam_unix.so             no_warn try_first_pass nullok

# account

EOD;

  if (isset($config['ad']['enable'])) {
    $system .= <<<EOD
#account sufficient /usr/local/lib/pam_winbind.so

EOD;
  } 

  $system .= <<<EOD
account         required        pam_login_access.so
account         required        pam_unix.so

# session
session         required        pam_lastlog.so          no_fail

# password

EOD;

  if (isset($config['ad']['enable'])) {
    $system .= <<<EOD
#password 	sufficient /usr/local/lib/pam_winbind.so	debug try_first_pass

EOD;
  }

  $system .= <<<EOD
password        required        pam_unix.so             no_warn try_first_pass

EOD;

  $fd = fopen("{$g['varetc_path']}/pam.d/system", "w");

  if (!$fd) {
    printf("Error: cannot open /pam.d/system in system_pam_configure().\n");
    return 1;
  }

  fwrite($fd, $system);
  fclose($fd);

  $sshd .= <<<EOD
# PAM configuration for the "sshd" service

# auth
auth            required        pam_nologin.so          no_warn
auth            sufficient      pam_opie.so             no_warn no_fake_prompts
auth            requisite       pam_opieaccess.so       no_warn allow_local

EOD;

  if (isset($config['ad']['enable'])) {
    $sshd .= <<<EOD
auth sufficient /usr/local/lib/pam_winbind.so	debug try_first_pass

EOD;
  }

  $sshd .= <<<EOD
auth            required        pam_unix.so             no_warn try_first_pass

# account

EOD;

  if (isset($config['ad']['enable'])) {
    $sshd .= <<<EOD
account			sufficient		/usr/local/lib/pam_winbind.so

EOD;
  }

  $sshd .= <<<EOD
account         required        pam_unix.so

# session
session         required        pam_permit.so

# password

EOD;

  if (isset($config['ad']['enable'])) {
    $sshd .= <<<EOD
password	sufficient	/usr/local/lib/pam_winbind.so	debug try_first_pass

EOD;
  }

  $sshd .= <<<EOD
password        required        pam_unix.so             no_warn try_first_pass

EOD;

  unset($fd);
  $fd = fopen("{$g['varetc_path']}/pam.d/sshd", "w");
  
  if (!$fd) {
    printf("Error: cannot open /pam.d/sshd in system_pam_configure().\n");
    return 1;
  }

  fwrite($fd, $sshd);
  fclose($fd);

  $ftp = <<<EOD
# PAM configuration for the "ftpd" service

# auth
auth            required        pam_nologin.so          no_warn
auth            sufficient      pam_opie.so             no_warn no_fake_prompts
auth            requisite       pam_opieaccess.so       no_warn allow_local

EOD;

  if (isset($config['ad']['enable'])) {
    $ftp .= <<<EOD
auth	sufficient	/usr/local/lib/pam_winbind.so	debug try_first_pass

EOD;
  } 

  $ftp .= <<<EOD
auth            required        pam_unix.so             no_warn try_first_pass

# account

EOD;

  if (isset($config['ad']['enable'])) {
    $ftp .= <<<EOD
account		sufficient	/usr/local/lib/pam_winbind.so

EOD;
  } 

  $ftp .= <<<EOD
account         required        pam_login_access.so
account         required        pam_unix.so

# session
session         required        pam_permit.so

EOD;

  unset($fd);
  $fd = fopen("{$g['varetc_path']}/pam.d/ftp", "w");
  
  if (!$fd) {
    printf("Error: cannot open /pam.d/ftp in system_pam_configure().\n");
    return 1;
  }

  fwrite($fd, $ftp);
  fclose($fd);

  $login = <<<EOD
# PAM configuration for the "login" service
#

# auth
auth            required        pam_nologin.so          no_warn

EOD;

  if (isset($config['ad']['enable'])) {
    $login .= <<<EOD
auth	sufficient	/usr/local/lib/pam_winbind.so	debug try_first_pass

EOD;
  } 

  $login .= <<<EOD
auth            sufficient      pam_self.so             no_warn
auth            include         system

# account

EOD;

  if (isset($config['ad']['enable'])) {
    $login .= <<<EOD
account		sufficient	/usr/local/lib/pam_winbind.so

EOD;
  } 

  $login .= <<<EOD
account         requisite       pam_securetty.so
account         include         system

# session
session         include         system

# password
password        include         system

EOD;

  unset($fd);
  $fd = fopen("{$g['varetc_path']}/pam.d/login", "w");

  if (!$fd) {
    printf("Error: cannot open /pam.d/login in system_pam_configure().\n");
    return 1;
  }

  fwrite($fd, $login);
  fclose($fd);

  unset($fd);

  /* Create the nsswitch.conf file*/
  if (isset($config['ad']['enable'])) {
    $nsswitch = <<<EOD
group: files winbind
group_compat: nis
hosts: files dns wins
networks: files
passwd: files winbind
passwd_compat: nis
shells: files

EOD;
  } else {
    $nsswitch = <<<EOD
group: compat
group_compat: nis
hosts: files dns
networks: files
passwd: compat
passwd_compat: nis
shells: files

EOD;
  } // end if
  
  $fd = fopen("{$g['varetc_path']}/nsswitch.conf", "w");

  if (!$fd) {
    printf("Error: cannot open /var/etc/nsswitch.conf in system_pam_configure().\n");
    return 1;
  }

  fwrite($fd, $nsswitch);
  fclose($fd);

  unset($fd);

  return 0;
}

function system_tuning() {
  /* fine tune kernel value */
  global $config, $g;
  
  if (isset($config['system']['tune'])) 	{
    if ($g['booting']) { echo 'Tuning the system... '; }

    mwexec('/sbin/sysctl net.inet.tcp.delayed_ack=0');
    mwexec('/sbin/sysctl net.inet.tcp.sendspace=65536');
    mwexec('/sbin/sysctl net.inet.tcp.recvspace=65536');
    mwexec('/sbin/sysctl net.inet.udp.recvspace=65536');
    mwexec('/sbin/sysctl net.inet.udp.maxdgram=57344');
    mwexec('/sbin/sysctl net.local.stream.recvspace=65535');
    mwexec('/sbin/sysctl net.local.stream.sendspace=65535');
    mwexec('/sbin/sysctl kern.ipc.maxsockbuf=2097152');	
    mwexec('/sbin/sysctl kern.ipc.somaxconn=8192');
    /* This variable can be set on the boot conf file only: */
    /* mwexec('/sbin/sysctl kern.ipc.maxsockets=16424'); */
    mwexec('/sbin/sysctl kern.ipc.nmbclusters=60000');
    mwexec('/sbin/sysctl kern.maxfiles=65536');
    mwexec('/sbin/sysctl kern.maxfilesperproc=32768');
    mwexec('/sbin/sysctl net.inet.tcp.inflight.enable=0');

    if ($g['booting']) { echo "done\n"; }
    return 0;
  } else {
    /* Set the default value (if previously changed) if not booting mode */
    if (!$g['booting']) {
      mwexec('/sbin/sysctl net.inet.tcp.delayed_ack=1');
      mwexec('/sbin/sysctl net.inet.tcp.sendspace=32768');
      mwexec('/sbin/sysctl net.inet.tcp.recvspace=65536');
      mwexec('/sbin/sysctl net.inet.udp.recvspace=42080');
      mwexec('/sbin/sysctl net.inet.udp.maxdgram=9216');
      mwexec('/sbin/sysctl net.local.stream.recvspace=8192');
      mwexec('/sbin/sysctl net.local.stream.sendspace=8192');
      mwexec('/sbin/sysctl kern.ipc.maxsockbuf=262144');	
      mwexec('/sbin/sysctl kern.ipc.somaxconn=128');
      /* mwexec('/sbin/sysctl kern.ipc.maxsockets=16424'); */
      mwexec('/sbin/sysctl kern.ipc.nmbclusters=3072');
      mwexec('/sbin/sysctl kern.maxfiles=1064');
      mwexec('/sbin/sysctl kern.maxfilesperproc=957');
    } // end if
    
    return 0;
  } // end if
}

/* TODO: Probably a function that is not needed */
function system_install_mount_cd($cdrom) {
  /* Part of install process: Mount the CDROM */
  global $config, $g;

  /* Creating tempo directory for cdrom*/
  echo "Creating Mount point for the CDROM:\n";
  if (mwexec("/bin/mkdir /mnt/cdrom_fr_0507")) { return 1; }

  /* Mounting the CDROM */
  echo "Mount CDROM:\n";
  if (mwexec("/sbin/mount_cd9660 /dev/$cdrom /mnt/cdrom_fr_0507")) { return 1; }

  return 0;
}

/* TODO: Probably a function that is not needed */
function system_install_init_fulldisk($harddrive) {
  /* Part of install process: Initialize the destination disk with one partition */
  global $config, $g;

  echo "Erasing partitions:\n";
  if (mwexec("/bin/dd if=/dev/zero of=/dev/$harddrive bs=1k count=20")) { return 1; }

  echo "Creating the primary partition and install simple MBR:\n";
  if (mwexec("/sbin/fdisk -BI -b /boot/mbr $harddrive")) { return 1; }

  /* Must wait that the /dev is upatded with the new information */
  /* There should be a more intelligent code here than this stupid timer... */
  echo "Waiting for system update..";
  $devtotest="/dev/$harddrive". "s1";
  $i=0;

  while (!file_exists($devtotest)) 	{
    sleep(1);
    echo ".";
    $i++;
    
    if ($i==20) { return 1; }
  }
  echo "\n";

  echo "Erasing primary partition:\n";
  if (mwexec("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($harddrive) . "s1 bs=32k count=16")) {
    return 1;
  }

  echo "Creating FreeBSD partition:\n";
  if (mwexec("/sbin/bsdlabel -B -w -b /boot/boot " . escapeshellarg($harddrive) ."s1 auto")) {
    return 1;
  }

  echo "Setting BSDlabel:\n";

  disks_bsdlabel($harddrive,"s1","4.2BSD");

  echo "Creating UFS filesystem:\n";
  if (mwexec("/sbin/newfs -U /dev/" . escapeshellarg($harddrive) . "s1")) { return 1; }

  return 0;
}

/* TODO: Probably a function that is not needed */
function system_install_init_halfdisk($harddrive) {
  /* Part of install process: Initialize the destination disk with 2 partitions */
  global $config, $g;

  /* Create the partitions */
  fdisk_hd_install($harddrive);

  if (mwexec("(/bin/echo y; /bin/echo y) | /sbin/fdisk -B -b /boot/mbr $harddrive")) { return 0; }

  /* Must wait that the /dev is upatded with the new information */
  echo "Waiting for system update..";

  $devtotest="/dev/$harddrive". "s2";
  $i=0;
  while (!file_exists($devtotest)) {
    sleep(1);
    echo ".";
    $i++;

    if ($i==20) { return 1; }
  }
  echo "\n";

  echo "Creating BSD Label:\n";
  if (mwexec("/sbin/bsdlabel -B -w -b /boot/boot " . escapeshellarg($harddrive) ."s1 auto")) {
    return 1;
  }
  if (mwexec("/sbin/bsdlabel -w " . escapeshellarg($harddrive) ."s2 auto")) {
    return 1;
  }

  echo "Modify BSD Label information:\n";

  disks_bsdlabel($harddrive,"s1","4.2BSD");
  disks_bsdlabel($harddrive,"s2","4.2BSD");

  echo "Creating filesystem:\n";
  if (mwexec("/sbin/newfs -U /dev/" . escapeshellarg($harddrive) . "s1")) {
    return 1;
  }
  if (mwexec("/sbin/newfs -U /dev/" . escapeshellarg($harddrive) . "s2")) {
    return 1;
  }

  return 0;
}

/* TODO: Probably a function that is not needed */
function system_install_mount_destdisk($harddrive) {
  /* Part of install process: Mount the destination disk */
  global $config, $g;

  echo "Creation tempory mouting point:\n";
  if (mwexec("/bin/mkdir /mnt/install_fr_0507")) { return 1; }

  echo "Mount destination disk:\n";
  if (mwexec("/sbin/mount /dev/" . escapeshellarg($harddrive) . "s1 /mnt/install_fr_0507")) {
    return 1;
  }

  return 0;
}

/* TODO: Probably a function that is not needed */
function system_install_unmount() {
  /* Part of install process: Unmounting the disk */
  global $config, $g;

  /* Unmounting disk */
  if (mwexec("/sbin/umount /mnt/install_fr_0507")) { return 1; }
  if (mwexec("/sbin/umount /mnt/cdrom_fr_0507")) { return 1; }
  
  /* using 'rm -rf' for deleting the temp directory is too dangerous here...  */
  /*
  if (mwexec("/bin/rm -rf /mnt/cdrom_fr_0507")) { return 1; }
  if (mwexec("/bin/rm -rf /mnt/install_fr_0507")) { return 1; }
 */
  
  /* Test: replace with PHP function rmdir */
  @rmdir ("/mnt/cdrom_fr_0507");
  @rmdir ("/mnt/install_fr_0507");

  return 0;
}

/* TODO: Probably a function that is not needed */
function system_install_installation($harddrive) {
  /* Part of install process: dd image file on the destination disk */
  global $config, $g;

  echo "Installation:\n";
  if (mwexec("/usr/bin/gunzip -S \"\" -c /mnt/cdrom_fr_0507/FreeNAS-generic-pc.gz | dd of=/dev/" . escapeshellarg($harddrive) . " bs=16k > /dev/null 2>&1")) {
    return 1;
  }

  return 0;
}

?>