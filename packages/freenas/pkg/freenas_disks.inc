<?php
/* $Id$ */
/* ========================================================================== */
/*
    freenas_disks.inc
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

/* include all configuration functions */
require_once("functions.inc");

$freenas_config =& $config['installedpackages']['freenas']['config'][0];

/* Mount all configured disks */
function disks_mount_all() {
  global $freenas_config, $g;

  if ($g['booting'])
    echo "Mounting Partitions... ";

  /* For each device configured: */
  if (is_array($freenas_config['mounts']['mount'])) {
    foreach ($freenas_config['mounts']['mount'] as $mountent) {
      /* Advanced Umount filesystem if not booting mode (mount edition) */
      if (!$g['booting'])
        disks_umount_adv($mountent);

      /* mount filesystem */
      disks_mount($mountent);
    } // end foreach
  } // end if

  if ($g['booting'])
    echo "done\n";

  return 0;
}

/* Mount using the configured mount given in parameter
 * Return 0 if sucessful, 1 if error
 */
function disks_mount($mount) {
  global $freenas_config, $g;

  /* Create one directory for each device under mnt */

  $mountname=escapeshellcmd($mount['sharename']);

  @mkdir ("/mnt/$mountname",0777);

  /* mount the filesystems */
  $devname=escapeshellcmd($mount['fullname']);

  /* check the fileystem only if there is a problem*/
  /* This part is too stupid: I must read the FreBSD 
   * start script for use the same intelligent method 
   * for checking hard drive
   */
  switch ($mount['fstype']) {
    case "ufs":
      if (mwexec("/sbin/mount -t ufs  -o acls $devname /mnt/$mountname") == 0) {
        /* Change this directory into 777 mode */
        mwexec("/bin/chmod 777 /mnt/$mountname");
        $result = 0;
      } else {
        /* If it's NOK, Check  filesystem and do a fsck, answer Yes to all question*/
        mwexec("/sbin/fsck -y -t ufs $devname");

        /* Re-try to mount the partition */
        if (mwexec("/sbin/mount -t ufs -o acls $devname /mnt/$mountname") == 0) {
          /* Change this directory into 777 mode */
          mwexec("/bin/chmod 777 /mnt/$mountname");
          $result = 0;
        } else {
          /* Not OK, remove the directory, prevent writing on RAM disk*/
          @rmdir ("/mnt/$mountname");
          $result = 1;
        } // end if
      } // end if
      break;
    case "msdosfs":
      if (mwexec("/sbin/mount_msdosfs -u ftp -g ftp -m 777 $devname /mnt/$mountname") == 0) {
        $result = 0;
      } else {
        exec("/sbin/fsck -y -t msdosfs {$devname}");
        
        if (mwexec("/sbin/mount_msdosfs -u ftp -g ftp -m 777 $devname /mnt/$mountname") == 0) {
          $result = 0;
        } else {
          /* Not OK, remove the directory, prevent writing on RAM disk*/
          @rmdir ("/mnt/$mountname");
          $result = 1;
        }
      }
      break;
    case "ntfs":
      if (mwexec("/sbin/mount_ntfs -u ftp -g ftp -m 777 $devname /mnt/$mountname") == 0) {
        $result = 0;
      } else {
        /* Not OK, remove the directory, prevent writing on RAM disk */
        @rmdir ("/mnt/$mountname");
        $result = 1;
      }
      break;
    case "ext2fs":
      if (mwexec("/sbin/mount_ext2fs $devname /mnt/$mountname") == 0) {
        /* Change this directory into 777 mode */
        mwexec("/bin/chmod 777 /mnt/$mountname");
        $result = 0;
      } else {
        exec("/usr/local/sbin/e2fsck -f -p {$devname}");
        
        if (mwexec("/sbin/mount_ext2fs $devname /mnt/$mountname") == 0) {
          /* Change this directory into 777 mode */
          mwexec("/bin/chmod 777 /mnt/$mountname");
          $result = 0;
        } else {
          /* Not OK, remove the directory, prevent writing on RAM */
          @rmdir ("/mnt/$mountname");
          $result= 1;
        }
      }
      break;
  } // end switch

  return $result;
}

/* Mount using fullname (/dev/ad0s1) given in parameter*/
function disks_mount_fullname($fullname) {
  global $freenas_config;

  if (is_array($freenas_config['mounts']['mount'])) {
    /* Search the mount list for given disk and partition */
    foreach($freenas_config['mounts']['mount'] as $mountk => $mountv) {
      if($mountv['fullname'] == $fullname) {
        $mount = $mountv;
      }
    }
  }

  if($mount) {
    $result= disks_mount($mount);
  } else {
    $result=0;
  }

  return $result;
}

/* Umount the specified configured mount point
 * Return 0 is successfull, 1 if error
 */
function disks_umount($mount) {
  global $freenas_config, $g;

  /* Umout the specified mount point */
  /* The $mount variable is the all config table for the mount point*/
  $mountname=escapeshellcmd($mount['sharename']);

  if (mwexec("/sbin/umount $mountname") == 0) {
    if (@rmdir ("/mnt/$mountname")) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return 1;
  }
}

/* Advanced unmount the specified mount point without using the sharename value
 * Used when changing the 'sharename': Need to umount the old unknow sharename
 * Return 0 if successful, 1 if error 
 */
function disks_umount_fullname($fullname) {
  global $freenas_config;

  /* Search the mount list for given fullname */
  foreach($freenas_config['mounts']['mount'] as $mountk => $mountv) {
    if (strcmp($mountv['fullname'],$fullname) == 0) {
      $mount = $mountv;
    }
  }

  if($mount) {
    $result = disks_umount($mount);
  } else {
    $result= 1;
  }

  return $result;
}

/* Advanced unmount the specified mount point without using the sharename value
 * Used when changing the 'sharename': Need to umount the old unknow sharename
 * Return 0 if successful, 1 if error 
 */
function disks_umount_adv($mount)
{
  $fulname="{$mount['fullname']}";

  // get the mount list
  $detmount = get_mounts_list();

  //Look for the mount point in all mounted point
  foreach ($detmount as $detmountk => $detmountv) {
    // If we found the mount point on the device
    if (strcmp($detmountv['fullname'],$fullname) == 0) {
      $mountname="{$detmountv['mp']}";
    }
  }

  if ($mountname) {
    exec("/sbin/umount $mountname");
    @rmdir ("$mountname");
    return 0;
  } else {
    $result = 1;
  }

  return $result;
}

function disks_mount_status($mount) {
  // This option check if the mount are mounted
  global $freenas_config, $g;
  $detmount = get_mounts_list();
  $status="ERROR";

  // Recreate the full system name device+s+partition number
  /* mount the filesystems */
  $mountpart="{$mount['partition']}";

  if ((strcmp($mountpart,"gvinum") == 0) || (strcmp($mountpart,"gmirror") == 0)) {
    $complete = "{$mount['mdisk']}";
  } else {
    $complete = "{$mount['mdisk']}{$mount['partition']}";
  }

  //echo "debug, display complete: $complete <br>";

  foreach ($detmount as $detmountk => $detmountv) {
    //echo "debug, display detmountv[mdisk]: {$detmountv['mdisk']} <br>";
        
    if (strcmp($detmountv['mdisk'],$complete) == 0) {
      $status="OK";
      return $status;
    }        
  }

  return $status;
}

/* This option check if this fullname (/dev/ad0s1) is mounted
 * Return 0 if not, 1 if yes
 */
function disks_check_mount_fullname($fullname) {
  $detmount = get_mounts_list();
  $status=0;
  //print_r($detmount);
  
  foreach ($detmount as $detmountk => $detmountv) {
    if (strcmp($detmountv['fullname'],$fullname) == 0) {
      $status=1;
      break;
    }
  }

  return $status;
}

/* This option check if the configured mount is mounted. */
function disks_check_mount($mount)
{
  return disks_check_mount_fullname($mount['fullname']);
}

/* This function check if the disk is mounted
 * Return 0 if not, 1 if yes 
 */
function disks_check_mount_disk($disk) {
  $detmount = get_mounts_list();
  $status=0;

  foreach ($detmount as $detmountk => $detmountv) {
  /* Must found the $disk (ad0) in result $mdisk (ad0s1) */
  // strpos will return 0 (found at position 0) if found, must check the 'false' value
  if (strpos($detmountv['mdisk'], $disk) !== false ) {
    $status=1;
    break;
  }
}
return $status;
}

/* This option check if the configured disk is online (detected by the system)
 * Result: "MISSING", disk don't detected by OS
 * Result: "ONLINE", disk is online
 * Result: "CHANGED", disk have changed (be replaced ?)
 */
function disks_status($diskname) {
  // This option check if the configured disk is online
  global $freenas_config, $g;
  $detectedlist = get_physical_disks_list();
  $status="MISSING";
  
  foreach ($detectedlist as $detecteddisk => $detecteddiskv) {
    if ($detecteddisk == $diskname['name']) {
      $status="ONLINE";
      if (($detecteddiskv['size'] != $diskname['size']) || ($detecteddiskv['desc'] != $diskname['desc'])) {
        $status="CHANGED";
      }
      break;
    }
  }
  
  return $status;
}

function disks_addfstab($cfgdev,$cfgtype) {
  global $freenas_config, $g;

  /* Open or create fstab in RW */
  $fd = fopen("{$g['etc_path']}/fstab", "w");

  if ( $fd ) {
    /* check for the precence of dev */
    /* ADD (check if it's ADD line or replace) the line for the dev */
    $fstab = "/dev/$cfgdev /mnt/$cfgdev $cfgtype rw 1 1\n";
                
    /* write out an fstab */
    fwrite($fd, $fstab);

    /* close file */
    fclose($fd);
  } else {
    die( "fopen failed for {$g['etc_path']}/fstab" ) ;
  }
}

function disks_umount_all() {
  global $freenas_config, $g;

  /* Sync disks*/
  mwexec("/bin/sync");

  if (is_array($freenas_config['mounts']['mount'])) {
    foreach ($freenas_config['mounts']['mount'] as $mountent) {
      /* Umount filesystem */
      disks_umount($mountent);
    }
  }

  return 0;
}

/* Configure, create and start gvinum volume */
function disks_raid_gvinum_configure() {
  global $freenas_config, $g;

  /* Generate the raid.conf file */
  if ($freenas_config['gvinum']['vdisk']) {

    foreach ($freenas_config['gvinum']['vdisk'] as $a_raid_conf) {
      if (file_exists($g['varrun_path'] . "/raid.conf.dirty") && 
          !in_array("{$a_raid_conf['name']}\n", file($g['varrun_path'] . "/raid.conf.dirty"))) { continue; }
        
      /* generate raid.conf */
      $fd = fopen("{$g['varetc_path']}/raid-{$a_raid_conf['name']}.conf", "w");
      if (!$fd) {
        printf("Error: cannot open raid.conf in services_raid_configure().\n");
        return 1;
      }

      $raidconf="";
      foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
        $raidconf .= <<<EOD
drive disk_{$diskrv} device {$diskrv}

EOD;
      } // end foreach
                 
      $raidconf .= <<<EOD
volume {$a_raid_conf['name']}

EOD;

      switch ($a_raid_conf['type']) {
        case "0":
          $raidconf .= <<<EOD
plex org striped 256k

EOD;

          foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
            /* Get the disksize */
            $disksize=get_disks_size($diskrv);
            /* Remove the ending 'B' in 'MB' */
            $disksize=rtrim($disksize, 'B');
            /*
            $raidconf .= <<<EOD
sd length {$disksize} drive disk_{$diskrv}

EOD;
*/
                                                
            $raidconf .= <<<EOD
sd length 0 drive disk_{$diskrv}

EOD;
        
          } // end foreach
          break;
        case "1":
          foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
            $raidconf .= <<<EOD
plex org concat

EOD;
                                                
            /* Get the disksize */
            $disksize=get_disks_size($diskrv);
            /* Remove the ending 'B' in 'MB' */
            $disksize=rtrim($disksize, 'B');
            /*
            $raidconf .= <<<EOD
sd length {$disksize} drive disk_{$diskrv}

EOD;
*/
            $raidconf .= <<<EOD
sd length 0 drive disk_{$diskrv}

EOD;
        
          } // end foreach
          break;
        case "5":
          $raidconf .= <<<EOD
plex org raid5 256k

EOD;

          foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
            /* Get the disksize */
            $disksize=get_disks_size($diskrv);
            /* Remove the ending 'B' in 'MB' */
            $disksize=rtrim($disksize, 'B');
            /*
            $raidconf .= <<<EOD
sd length {$disksize} drive disk_{$diskrv}

EOD;
*/

            $raidconf .= <<<EOD
sd length 0 drive disk_{$diskrv}

EOD;
        
          } // end foreach
          break;
      } // end switch
    
      fwrite($fd, $raidconf);
      fclose($fd);

      /* Create each volume */        
      mwexec("/sbin/gvinum create {$g['varetc_path']}/raid-{$a_raid_conf['name']}.conf");        
    } // end foreach
                                        
        
    /* start each volume */
        
    foreach ($freenas_config['gvinum']['vdisk'] as $a_raid_conf) {
      exec("/sbin/gvinum lv $raidname",$rawdata);
    
      if (strpos($rawdata[0],"State: up")>=0) { continue; }

      mwexec("/sbin/gvinum start {$a_raid_conf['name']}");
    }        
  } // end if

  return 0;
}

function disks_raid_configure()
{
  global $freenas_config, $g;

  /* Generate the raid.conf file */
  if ($freenas_config['raid']['vdisk']) {        
    foreach ($freenas_config['raid']['vdisk'] as $a_raid_conf) {
      if (file_exists($g['varrun_path'] . "/raid.conf.dirty") && 
          ! in_array("{$a_raid_conf['name']}\n",file($g['varrun_path'] . "/raid.conf.dirty"))) { continue; }
        
      /* generate raid.conf */
      $fd = fopen("{$g['varetc_path']}/raid-{$a_raid_conf['name']}.conf", "w");
      
      if (!$fd) {
        printf("Error: cannot open raid.conf in services_raid_configure().\n");
        return 1;
      }

      $raidconf="";
      foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
        $raidconf .= <<<EOD
drive disk_{$diskrv} device /dev/{$diskrv}s1a

EOD;

      } // end foreach

      $raidconf .= <<<EOD
volume {$a_raid_conf['name']}

EOD;

      switch ($a_raid_conf['type']) {
        case "0":
          $raidconf .= <<<EOD
plex org striped 256k

EOD;

          foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
            /* Get the disksize */
            $disksize=get_disks_size($diskrv);
            /* Remove the ending 'B' in 'MB' */
            $disksize=rtrim($disksize, 'B');
            /*
            $raidconf .= <<<EOD
sd length {$disksize} drive disk_{$diskrv}

EOD;
*/

$raidconf .= <<<EOD
sd length 0 drive disk_{$diskrv}

EOD;

          } // end foreach
          break;
        case "1":
          foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
            $raidconf .= <<<EOD
plex org concat

EOD;

          /* Get the disksize */
          $disksize=get_disks_size($diskrv);
          /* Remove the ending 'B' in 'MB' */
          $disksize=rtrim($disksize, 'B');
          /*
          $raidconf .= <<<EOD
sd length {$disksize} drive disk_{$diskrv}

EOD;
*/

          $raidconf .= <<<EOD
sd length 0 drive disk_{$diskrv}

EOD;

          } // end foreach
          break;
        case "5":
          $raidconf .= <<<EOD
plex org raid5 256k

EOD;

          foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
            /* Get the disksize */
            $disksize=get_disks_size($diskrv);
            /* Remove the ending 'B' in 'MB' */
            $disksize=rtrim($disksize, 'B');
            /*
            $raidconf .= <<<EOD
sd length {$disksize} drive disk_{$diskrv}

EOD;
*/

            $raidconf .= <<<EOD
sd length 0 drive disk_{$diskrv}

EOD;

          } // end foreach
          break;
      } // end switch
      
      fwrite($fd, $raidconf);
      fclose($fd);

      /* Create each volume */
      mwexec("/sbin/gvinum create {$g['varetc_path']}/raid-{$a_raid_conf['name']}.conf");
    } // end foreach

    /* start each volume */
    foreach ($freenas_config['raid']['vdisk'] as $a_raid_conf) {
      exec("/sbin/gvinum lv $raidname",$rawdata);
      if (strpos($rawdata[0],"State: up")>=0) { continue; }
      mwexec("/sbin/gvinum start {$a_raid_conf['name']}");
    }
  } // end if
  
  return 0;
}

/* Configure, create and start gmirror volume */
function disks_raid_gmirror_configure() {
  global $freenas_config, $g;

  /* Create the gmirror device */
  if ($freenas_config['gmirror']['vdisk']) {        
    // Load gmirror
    mwexec("/sbin/gmirror load");
                        
    foreach ($freenas_config['gmirror']['vdisk'] as $a_raid_conf) {
      /* Create each volume */
      $cmd = "/sbin/gmirror label -b {$a_raid_conf['balance']} {$a_raid_conf['name']} ";
                
      foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
        $cmd .= "{$diskrv} ";
      }
        
      mwexec($cmd);
    }
  }

  return 0;
}

/* Configure, create and start gconcat volume */
function disks_raid_gconcat_configure() {
  global $freenas_config, $g;

  if ($freenas_config['gconcat']['vdisk']) {
    // Load gconcat
    mwexec("/sbin/gconcat load");
                        
    foreach ($freenas_config['gconcat']['vdisk'] as $a_raid_conf) {
      /* Create each volume */
      $cmd = "/sbin/gconcat label {$a_raid_conf['name']} ";
                
      foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
        $cmd .= "{$diskrv} ";
      }
        
      mwexec($cmd);
        
    }
  }

  return 0;
}

/* Configure, create and start gstripe volume */
function disks_raid_gstripe_configure() {
  global $freenas_config, $g;

  if ($freenas_config['gstripe']['vdisk']) {
    // Load gstripe
    mwexec("/sbin/gstripe load");

    foreach ($freenas_config['gstripe']['vdisk'] as $a_raid_conf) {
      /* Create each volume */
      $cmd = "/sbin/gstripe label {$a_raid_conf['name']} ";

      foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
        $cmd .= "{$diskrv} ";
      }

      mwexec($cmd);
    }
  }

  return 0;
}

/* Configure, create and start graid5 volume */

function disks_raid_graid5_configure()
{
  global $freenas_config, $g;

  if ($freenas_config['graid5']['vdisk']) {
    /* Load graid5 */
    mwexec("/sbin/graid5 load");

    foreach ($freenas_config['graid5']['vdisk'] as $a_raid_conf) {
      /* Create each volume */
      $cmd = "/sbin/graid5 label -s 131072 {$a_raid_conf['name']} ";

      foreach ($a_raid_conf['diskr'] as $diskrk => $diskrv) {
        $cmd .= "{$diskrv} ";
      }

      mwexec($cmd);
    }
  }

  return 0;
}

function disks_raid_start() {
  global $freenas_config, $g;

  /* WARNING:  Must change this code for advanced RAID configuration ex: RAID1+0
   * Geom RAID volume must be started in 'intelligent' sort, for RAID1+0, gmirror must
   * be started before gstripe, etc... 
   */
  disks_raid_gvinum_start();
  disks_raid_gmirror_start();
  disks_raid_gstripe_start();
  disks_raid_gconcat_start();
  disks_raid_graid5_start();
        
  return 0;
}


function disks_raid_gvinum_start() {
  global $freenas_config, $g;

  /* Generate the raid.conf file */
  if ($freenas_config['raid']['vdisk']) {
    if ($g['booting'])
      echo "Start gvinum raid... ";

    /* start each volume */
    foreach ($freenas_config['raid']['vdisk'] as $a_raid_conf) {
      mwexec("/sbin/gvinum start {$a_raid_conf['name']}");
    }
        
    if ($g['booting'])
      echo "done\n";
  }

  return 0;
}

function disks_raid_gmirror_start() {
  global $freenas_config, $g;

  /* Start Geom mirror */
  if ($freenas_config['gmirror']['vdisk']) {
    if ($g['booting'])
      echo "Start gmirror raid... ";

    // Load geom mirror module        
    mwexec("/sbin/gmirror load");
        
    if ($g['booting'])
      echo "done\n";
  }

  return 0;
}

/* Start geom concat volumes */
function disks_raid_gconcat_start() {
  global $freenas_config, $g;

/* Start Geom concat */
  if ($freenas_config['gconcat']['vdisk']) {
    if ($g['booting'])
      echo "Start gconcat raid... ";

    // Load geom concat module
    mwexec("/sbin/gconcat load");
        
    if ($g['booting'])
      echo "done\n";
  }

  return 0;
}

/* Start geom stripe volumes */
function disks_raid_gstripe_start() {
  global $freenas_config, $g;

  /* Start Geom stripe */
  if ($freenas_config['gstripe']['vdisk']) {
    if ($g['booting'])
      echo "Start gstripe raid... ";

    // Load geom stripe module
    mwexec("/sbin/gstripe load");
        
    if ($g['booting'])
      echo "done\n";        
  }

  return 0;
}

/* Start geom raid5 volumes */
function disks_raid_graid5_start() {
  global $freenas_config, $g;

  /* Start Geom RAID5 */
  if ($freenas_config['graid5']['vdisk']) {
    if ($g['booting'])
      echo "Start graid5 raid... ";

    // Load geom raid5 module
    mwexec("/sbin/graid5 load");
        
    if ($g['booting'])
      echo "done\n";
  }

  return 0;
}

function disks_raid_stop() {
  /* WARNING:  Must change this code for advanced RAID configuration ex: RAID1+0
   * Geom RAID volume must be started in 'intelligent' sort, for RAID1+0, gmirror must
   * be started before gstripe, etc... 
   */
  disks_raid_gvinum_stop();
  disks_raid_graid5_stop();
  disks_raid_gstripe_stop();
  disks_raid_gconcat_stop();
  disks_raid_gmirror_stop();

  return 0;
}

function disks_raid_gvinum_stop() {
  global $freenas_config, $g;

  /* Generate the raid.conf file */
  if ($freenas_config['raid']['vdisk']) {
    /* stop each volume */
    foreach ($freenas_config['raid']['vdisk'] as $a_raid_conf) {
      mwexec("/sbin/gvinum stop {$a_raid_conf['name']}");
    }
  }

  return 0;
}

function disks_raid_gmirror_stop() {
  global $freenas_config, $g;

  /* Generate the raid.conf file */
  if ($freenas_config['gmirror']['vdisk']) {
    /* start each volume */
    foreach ($freenas_config['gmirror']['vdisk'] as $a_raid_conf) 	{
      mwexec("/sbin/gmirror stop {$a_raid_conf['name']}");
    }
  }

  return 0;
}

/* Stop all geom concat volumes */
function disks_raid_gconcat_stop() {
  global $freenas_config, $g;
  
  /* Stop geom concat */
  if ($freenas_config['gconcat']['vdisk']) {
    /* start each volume */
    foreach ($freenas_config['gconcat']['vdisk'] as $a_raid_conf) {
      mwexec("/sbin/gconcat stop {$a_raid_conf['name']}");   
    }        
  }

  return 0;
}

/* Stop all geom stripe volumes */
function disks_raid_gstripe_stop() {
  global $freenas_config, $g;

  /* Stop geom stripe */
  if ($freenas_config['gstripe']['vdisk']) {
    /* start each volume */
    foreach ($freenas_config['gstripe']['vdisk'] as $a_raid_conf) {
      mwexec("/sbin/gstripe stop {$a_raid_conf['name']}");
    }    
  }

  return 0;
}

/* Stop all geom raid5 volumes */
function disks_raid_graid5_stop() {
  global $freenas_config, $g;

  /* Generate the raid.conf file */
  if ($freenas_config['graid5']['vdisk']) {
    /* start each volume */
    foreach ($freenas_config['graid5']['vdisk'] as $a_raid_conf) {
      mwexec("/sbin/graid5 stop {$a_raid_conf['name']}");
    }   
  }

  return 0;
}


/* Delete geom gvinum volume given in parameter */
function disks_raid_gvinum_delete($raidname) {
  global $freenas_config, $g;

  exec("/sbin/gvinum lv $raidname",$rawdata);
  
  if (strpos($rawdata[0],"State: up") === false) {
    return 0;
  }

  mwexec("/sbin/gvinum rm -r $raidname");

  foreach ($freenas_config['gvinum']['vdisk'] as $a_raid) {
    if ($a_raid['name'] == $raidname) {
      foreach ($a_raid['diskr'] as $disk) {
        mwexec("/sbin/gvinum rm -r disk_{$disk}");
      }
    }
  }

  return 0;
}

/* Delete geom mirror volume given in parameter */
function disks_raid_gmirror_delete($raidname) {
  global $freenas_config, $g;

  // Stop the volume
  mwexec("/sbin/gmirror stop $raidname");

  // Clear the gmirror information on the hard drive
  foreach ($freenas_config['gmirror']['vdisk'] as $a_raid) {
    if ($a_raid['name'] == $raidname) {
      foreach ($a_raid['diskr'] as $disk) {
        mwexec("/sbin/gmirror clear {$disk}");
      }
    }
  }

  return 0;
}

/* Delete geom concat volume given in parameter */
function disks_raid_gconcat_delete($raidname) {
  global $freenas_config, $g;

  // Stop the volume
  mwexec("/sbin/gconcat stop $raidname");

  // Clear the gconcat information on the hard drive
  foreach ($freenas_config['gconcat']['vdisk'] as $a_raid) {
    if ($a_raid['name'] == $raidname) {
      foreach ($a_raid['diskr'] as $disk) {
        mwexec("/sbin/gconcat clear {$disk}");
      }
      
      mwexec("/sbin/gconcat destroy $raidname");
    }
  }

  return 0;
}

/* Delete geom stripe volume given in parameter */
function disks_raid_gstripe_delete($raidname) {
  global $freenas_config, $g;

  // Stop the volume
  mwexec("/sbin/gstripe stop $raidname");

  // Clear the gconcat information on the hard drive
  foreach ($freenas_config['gstripe']['vdisk'] as $a_raid) {
    if ($a_raid['name'] == $raidname) {
      foreach ($a_raid['diskr'] as $disk) {
        mwexec("/sbin/gstripe clear {$disk}");
      }
      
      mwexec("/sbin/gstripe destroy $raidname");
    }
  }

  return 0;
}

/* Delete geom raid5 volume given in parameter */
function disks_raid_graid5_delete($raidname) {
  global $freenas_config, $g;

  // Stop the volume
  mwexec("/sbin/graid5 stop $raidname");

  // Clear the graid5 information on the hard drive
  foreach ($freenas_config['graid5']['vdisk'] as $a_raid) {
    if ($a_raid['name'] == $raidname) {
      foreach ($a_raid['diskr'] as $disk) {
        mwexec("/sbin/graid5 remove $raidname {$disk}");
      }
    
      mwexec("/sbin/graid5 destroy $raidname");
    }
  }

  return 0;
}

/* Initialise HARD DRIVE for installing FreeNAS (creating 2 partition) */
function fdisk_hd_install($harddrive) {
  global $freenas_config, $g;

  /* Initialise HARD DRIVE for installing FreeNAS (creating 2 partition) */

  /* getting disk information */
  $fdisk_info=fdisk_get_info($harddrive);

  /* setting FreeNAS partition size to 32Mb */

  $part_freenas_size=32;

  /* convert Mb to b */
  $part_freenas_size=$part_freenas_size * 1024 * 1024;

  $part1_size=$part_freenas_size / $fdisk_info['sec_size'];
  $part2_size=$fdisk_info['total'] - $part1_size;

  /* Create fdisk config file */

  /* generate fdisk.conf */
  $fd = fopen("{$g['varetc_path']}/fdisk.conf", "w");
  if (!$fd) {
    printf("Error: cannot open fdisk.conf in fdisk_hd_install().\n");
    return 1;
  }
        
$fdiskconf .= <<<EOD
g c{$fdisk_info['cyl']} h{$fdisk_info['head']} s{$fdisk_info['sect']}
p 1 165 1 $part1_size
p 2 165 $part1_size $part2_size
p 3 0 0 0
p 4 0 0 0
a 1

EOD;

  fwrite($fd, $fdiskconf);
  fclose($fd);

  /* Fdisk the disk */

  /* Warning: Ask two questions to the user */
  mwexec("/sbin/fdisk -f {$g['varetc_path']}/fdisk.conf /dev/$harddrive");

  return 0;
}

function fdisk_get_info($harddrive) {
  /* Return information about an harddrive

  $result['total'] : size
  $result['cyl'] : cylinders
  $result['head'] : heads
  $result['sect'] :  sectors/track
  $result['sec_size'] : Media sector size

  */

  global $freenas_config, $g;

  exec("/sbin/fdisk /dev/$harddrive",$rawdata);

  $result=array();

  foreach ($rawdata as $line) {
    /* separe the line by space or egal*/
    $aline= preg_split("/[\s,]+|=/", $line);
    $first_word = chop($aline[0]);
        
    if ($aline[0] == "Media") {
      $result['sec_size']=chop($aline[4]);
      continue ;
    }
        
    if ($aline[0] == "cylinders") {
      $result['cyl']=chop($aline[1]);
      $result['head']=chop($aline[3]);
      $result['sect']=chop($aline[5]);
      continue ;
    }
  }

  $result['total'] = $result['cyl'] * $result['head'] * $result['sect'] ;

  return $result;
}

/* Is this function still used ??? */
function disks_bsdlabel($harddrive,$partition,$type) {
  global $freenas_config, $g;

  // Generating BSD Label table
  passthru("/sbin/bsdlabel " . escapeshellarg($harddrive) ."$partition > {$g['tmp_path']}/label.tmp");

  // put this file on a array
  $tableau = file("{$g['tmp_path']}/label.tmp");

  // Open this file in add mode 
  $handle = fopen("{$g['tmp_path']}/label.tmp", 'a');

  while(list(,$val) = each($tableau)) {
    // If the line contain the word "unused"
    if (ereg ("unused",$val)) {
      // Replacing c: by a:
      $val = ereg_replace ("c:","a:", $val);
      // Peplacing unused by $type
      $val = ereg_replace ("unused",$type, $val);
      // Adding this line add the end of the file
      fwrite($handle, $val);
    }
  }
  
  // Closing file
  fclose($handle);

  // Injecting this new partition table 
  passthru("/sbin/bsdlabel -R -B " . escapeshellarg($harddrive) ."$partition {$g['tmp_path']}/label.tmp");
}


function disks_set_ataidle() {
  global $g, $freenas_config;

  if (is_array($freenas_config['disks']['disk'])) {
    foreach ($freenas_config['disks']['disk'] as $disk) {        
      if ($disk['type']=="IDE") {
        /* If UDMA mode forced, launch atacontrol */
        if (isset($disk['udma']) && ($disk['udma'] != "auto")) {
          mwexec("/sbin/atacontrol mode {$disk['name']}  {$disk['udma']}");
        }
                        
        /* Don't use ataidle if all is disabled */
        if (($disk['harddiskstandby'] == 0) && ($disk['apm'] == 0) && ($disk['acoustic'] == 0)) { continue; }
                                
        /* Found the channel and device number from the /dev name */
        /* Divise the number by 2, the interger is the channel number, the rest is the device */
                
        $value=trim($disk['name'],'ad');
                        
        $value=intval($value);
        $channel = $value/2;
        $device=$value % 2;
        $channel=intval($channel);
        $time=$disk['harddiskstandby'];
        $apm=$disk['apm'];
        $ac=$disk['acoustic'];
        
        /* mwexec("/usr/local/sbin/ataidle -A $ac -P $apm -S $time $channel $device"); */
        
        $cmd = "/usr/local/sbin/ataidle ";
        
        if ($disk['acoustic'] != 0) { $cmd .= "-A $ac  "; }
        if ($disk['apm'] != 0) { $cmd .= "-P $apm "; }
        if ($disk['harddiskstandby'] !=0) { $cmd .= "-S $time "; }
        
        $cmd .= "$channel $device";                
        mwexec($cmd);        
      } // end if
    } // end foreach
    
    return 1;
  } // end if
                
  return 0;
}

/* Is this function still used ???
Get list of partition information from disk.
Result is in the form:
[1] => Array
(
  [start] => 31
  [size]  => 409169
  [type]  => 0xa5
  [flags] => 0x80
)
*/
function disks_get_partition_info($disk) {  
  exec("/sbin/fdisk -s /dev/{$disk}", $rawdata);
  array_shift($rawdata);
  array_shift($rawdata);

  $result = array();

  foreach($rawdata as $partinfo) {
    $apartinfo = preg_split("/\s+/", $partinfo);
    $partid = chop($apartinfo[1],":");

    $result[$partid] = array();
    $result[$partid]['start'] = chop($apartinfo[2]);
    $result[$partid]['size']  = chop($apartinfo[3]);
    $result[$partid]['type']  = chop($apartinfo[4]);
    $result[$partid]['flags'] = chop($apartinfo[5]);
  }

  return $result;
}

?>
