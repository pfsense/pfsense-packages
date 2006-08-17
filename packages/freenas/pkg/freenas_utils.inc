<?php
/* $Id$ */
/*
	utils.inc
	part of FreeNAS (http://www.freenas.org)
	Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
	All rights reserved.
	
	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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

/* returns true if $desc is a valid description (alphanum and space, _ , - , .)*/
function is_validdesc($desc)
{
	
	if (!is_string($desc))
		return false;
		
	if (preg_match("/^[A-Za-z0-9]([A-Za-z0-9_\-\.\s]*[A-Za-z0-9])*$/", $desc))
		return true;
	else
		return false;
}

/* returns true if $login is a valid login name (alphanum,dot, _ , -)*/
function is_validlogin($login)
{
	
	if (!is_string($login))
		return false;
		
	if (preg_match("/^[A-Za-z0-9]([A-Za-z0-9_\-\.\s]*[A-Za-z0-9])*$/", $login))
		return true;
	
	else
		return false;
}

/* returns true if $login is a valid share name (alphanum,dot, _ , -)*/
function is_validsharename($sharename)
{
	
	if (!is_string($sharename))
		return false;
		
	if (preg_match("/^[A-Za-z0-9]([A-Za-z0-9_\-\.\s]*[A-Za-z0-9])*$/", $sharename))
		return true;
	
	else
		return false;
}

function get_mounts_list()
{
	// Return list of mounted disk
	//example: Array
	//	[0] => Array
    //	    (
    //        [mp] => /mnt/ad0s1
    //        [mdisk] => ad0s1
    //      )

	
	global $g;
	
	exec("/sbin/mount",$rawdata);
	
	$mountlist = array();
	$i=0;
	
	foreach ($rawdata as $line)
	{
		$aline = explode(" ", $line);
		
		if ((chop($aline[0]) == "/dev/md0") || (chop($aline[0]) == "devfs") || (chop($aline[0]) == "/dev/fd0"))
			continue;
		$mountlist[$i]['mp']=chop($aline[2]);
		// Get the complete name /dev/devicename
		
		$complete=explode("/",chop($aline[0]));
		
		$devname=$complete[2];
		
		// Test if it's a gvinum or gmirror device
		if ((strcmp($devname,"gvinum") == 0) || (strcmp($devname,"mirror") == 0))
			$devname=$complete[3];
		
		$mountlist[$i]['mdisk']=$devname;
		$i++;
	}
	
	return $mountlist;
}

function get_sraid_disks_list()
{
	/* Return list of ALL software volume: gvinum, gmirror */
	$disklist = array_merge((array)get_gvinum_disks_list(),(array)get_gmirror_disks_list());
	return $disklist;

}

function get_gvinum_disks_list()
{
	/* Get information about gvinum volume */
	global $g;
	
	/* Return list of Software RAID disk */
	
    //[volumename] => Array
    //    (
    //        [type] => Software RAID - gvinum
    //        [desc] => UP
    //        [size] => 6149 MB
    //    )
    

	/* on envoie la commande d'affichage de la liste des volumes*/
	exec("/sbin/gvinum list",$rawdata);
	
	foreach ($rawdata as $line)
	{
		
				
		/* Separe la ligne par les espace */
		$aline = preg_split("/\s+/", $line);
	
		/* Récupère la ligne commencant par 'V' */
		if ($aline[0] != "V")
		{
			continue ;
		}
		
		$diskname = chop($aline[1]);
		
		$disklist[$diskname]=array();
					
		$disklist[$diskname]['type'] = "Software RAID - gvinum";
		
		$disklist[$diskname]['size'] = "$aline[7] $aline[8]" ;
		
		$disklist[$diskname]['desc'] = $aline[3];
		
	}
	
	
		return $disklist;
}

function get_gmirror_disks_list()
{
	/* Get information about gmirror volume */
	global $g;
	
	
    //[volumename] => Array
    //    (
    //        [type] => Software RAID - gmirror
    //        [desc] => COMPLETE
    //        [size] => 6149M
    //    )


	/* Display all configured gmirror volume*/
	exec("/sbin/gmirror list",$rawdata);
	
	$foundname = 0 ;
	$founddesc = 0 ;
	$goodname = 0 ;
	
	foreach ($rawdata as $line) {

				
		/* Use space for break the line */
		$aline = preg_split("/\s+/", $line);
		
		// First Step: Getting the array name
		// look for this output:
		// Geom name: pouet
		
		if ( (strcmp($aline[0],"Geom") == 0) && (strcmp($aline[1],"name:") == 0) ) {
			$diskname = $aline[2];
			$disklist[$diskname]=array();
			$foundname = 1 ;
			continue ;
		}
		
		// Second Step: Getting the array status
		// look for this output:
		// State: COMPLETE
		
		if ( (strcmp($aline[0],"State:") == 0) && $foundname) {
		
			$desc = $aline[1];
			$disklist[$diskname]['desc'] = $desc;
			$founddesc=1;
			continue ;
		}

		// Third Step: Getting the array Size
		// look for this output:
		// Name: mirror/pouet
		// Mediasize: 107373568 (102M)
		
		if (preg_match("/Name: mirror\/(.*)$/", $line, $matches)) {
			if ($matches[1]=$diskname) {
				$goodname=1;
				continue ;
			}
		
		}
		
		if (($aline[0] = "Mediasize:") && $goodname) {
			// extract the size between the ( )
			preg_match("/.*\\(([^\)]*)\).*/",$aline[3],$match);
			$disklist[$diskname]['size'] = $match[1];
			$disklist[$diskname]['type'] = "Software RAID - gmirror";
			
			// init the check variable for the next RAID volume
			$foundname = 0 ;
			$founddesc = 0 ;
			$goodname = 0 ;
			
			continue ;
			
		}				
		
	}
	
	return $disklist;
		
}



function get_ata_disks_list()
{
	/* Return list of ATA disk */
	
    //[ad0] => Array
    //    (
    //        [type] => IDE
    //        [desc] => QUANTUM FIREBALL EX6.4A/A0A.0D00
    //        [size] => 6149MB
    //    )


	global $g;
	
	/* Recupere le dmesg */
	exec("/sbin/dmesg",$rawdmesg);
			
	$disklist = array();
	
	/******* Getting IDE disk informations *******/
	
	exec("/sbin/atacontrol list",$rawdata);
	
	foreach ($rawdata as $line) {
					
		/* Separe la ligne par les espace */
		$aline = preg_split("/\s+/", $line);
	
		/* Si ATA alors NEXT */
		if ($aline[0] == "ATA")
			continue ;
	
		$diskname = chop($aline[2]);
		
		/* Exlude CDROM (acdX) and Empty (no) */
		if (!preg_match("/^(acd)/", $diskname ) & $diskname != "no") {
			
			$disklist[$diskname]=array();
					
			$disklist[$diskname]['type'] = "IDE";
		
			/* Match the description witch is include between < and > */
		
			preg_match("/.*\<([^>]*)>.*/",$line,$match);
		
			$disklist[$diskname]['desc'] = $match[1];
			
			/* Looking for the disk size */
			foreach ($rawdmesg as $dmesgline) 	{
				
				/* Take only the first dmesg line */
				if (!$disklist[$diskname]['size']) 		{	
				
					/* Separe la ligne par les espace */
					$dmesgtab = explode(" ", $dmesgline);
					$dmesgtab[0] = rtrim($dmesgtab[0],":");
					if ($dmesgtab[0]!="" &&(strcasecmp($dmesgtab[0],$diskname) == 0))
						$disklist[$diskname]['size'] = $dmesgtab[1];
				}
			}
			
		}
		
	}
	return $disklist;
}


function get_scsi_disks_list()
{
	/* Recupere la liste des disques SCSI */
	
	//[ad0] => Array
    //    (
    //        [type] => IDE
    //        [desc] => QUANTUM FIREBALL EX6.4A/A0A.0D00
    //        [size] => 6149MB
    //    )


	global $g;
		
	/* Recupere le dmesg */
	exec("/sbin/dmesg",$rawdmesg);

	/* on envoie la commande d'affichage de la liste des disques*/
	exec("/sbin/camcontrol devlist",$rawdata);
		
	foreach ($rawdata as $line) {
	
		/* Get information include between parenthese: (pass0,da0) or (da0,pass0)*/
		
		preg_match("/.*\(([^>]*)\).*/",$line,$match);
	
		/* Sépare le resultat par la virgule  */
		$temp = preg_split("/,/", $match[1]);
		
		// Check if diskname is the first (da0,pass0) or the second (pass0,da0) arguement
		$diskname = $temp[1];
		if ($diskname[0] == "p")
			$diskname = $temp[0];
		
		/* On exlus les lecteurs cd */
		if (!preg_match("/^(cd)/", $diskname  )) 	{
			$disklist[$diskname]=array();
			$disklist[$diskname]['type'] = "SCSI";
			/* Recupère la description: ce qu'il y a entre < et > */
		
			preg_match("/.*\<([^>]*)>.*/",$line,$match);
		
			$disklist[$diskname]['desc'] = $match[1];

			/* Looking for the disk size */
			foreach ($rawdmesg as $dmesgline) 	{			
					/* Separe la ligne par les espace */
					$dmesgtab = explode(" ", $dmesgline);
					$dmesgtab[0] = rtrim($dmesgtab[0],":");
					if ($dmesgtab[0]!="" &&(strcasecmp($dmesgtab[0],$diskname) == 0))
						$disklist[$diskname]['size'] = $dmesgtab[1];
				
			}
		
		}
	}
	return $disklist;

}

function get_hraid_disks_list()
{
	/* Recupere la liste des disques RAID */
	
	$kerneldisks = explode(" ", trim(preg_replace("/kern.disks: /", "", exec("/sbin/sysctl kern.disks"))));
	
	/* Recupere la liste des disques ATA et SCSI */
	
	$diskdetected = array_merge((array)get_ata_disks_list(),(array)get_scsi_disks_list());
	
	/* Recupere le dmesg */
	exec("/sbin/dmesg",$rawdmesg);
	
	
	foreach ($kerneldisks as $diskname) {
		$allready=1;
		
		// Check of this entry is IDE or SCSI (allready detected)
		foreach ($diskdetected as $diskfoundk => $diskfoundv) 	{
			
			if (strcasecmp($diskfoundk,$diskname) == 0)
				$allready=0;
		}
		
		if ($allready) 	{

			/* If not an IDE and SCSI disk */
			$disklist[$diskname]=array();
		
			$disklist[$diskname]['type'] = "RAID";
			
			/* Looking for the disk size in the dmesg */
			foreach ($rawdmesg as $dmesgline) 		{
				/* Separe la ligne par les espace */
				$dmesgtab = explode(" ", $dmesgline);
				$dmesgtab[0] = rtrim($dmesgtab[0],":");
				// si la ligne commence par le nom du disque: attention il y a 2 lignes
				if ($dmesgtab[0]!="" &&(strcasecmp($dmesgtab[0],$diskname) == 0)) 	{
					// the first line as this example "aacd0: <RAID 5> on aac0"
					
					if (strcasecmp(substr($dmesgtab[1], 0, 1),"<") == 0) 	{
						/* Match the description witch is include between < and > */
						preg_match("/.*\<([^>]*)>.*/",$dmesgline,$match);
						$disklist[$diskname]['desc'] = $match[1];
					}
					else	{
						// si c'est la deuxieme ligne, elle ressemble a "aacd0: 138850MB (284365824 sectors)"
						$disklist[$diskname]['size'] = $dmesgtab[1];
					}
			
				}	
			}
		}
			
	}
		
		return $disklist;
}

function get_physical_disks_list()
{
	/* Return list of ALL disk: physical, hardware RAID and Software RAID disk */
	$disklist = array_merge((array)get_ata_disks_list(),(array)get_scsi_disks_list(),(array)get_hraid_disks_list());
	return $disklist;

}

function get_all_disks_list()
{
	/* Return list of ALL disk: physical, hardware RAID and Software RAID disk */
	$disklist = array_merge((array)get_ata_disks_list(),(array)get_scsi_disks_list(),(array)get_hraid_disks_list(), (array)get_sraid_disks_list());
	return $disklist;

}

function get_disks_size($diskname)
{
	/* Return size of the disk */
	$disklist=get_physical_disks_list();
	
	$disksize=0;
	
	foreach ($disklist as $diskk => $diskv) 	{
		if (strcmp($diskk,$diskname)==0)
			$disksize=$diskv['size'];	
	}
	return $disksize;
	
}
    
function get_cdrom_list()
{
	/* Return list of IDE AND SCSI CDROM */
	// exemple: Array
    // [acd0] => Array
    //    (
    //        [type] => IDE
    //        [desc] => TDK CDRW241040B/57S2
    //    )


	
	global $g;
	
	/* Recupere la liste des disques IDE */
	exec("/sbin/atacontrol list",$rawdata);
	
	/* Recupere le dmesg */
	exec("/sbin/dmesg",$rawdmesg);
			
	$disklist = array();
	
	/* Variable $i utilisé pour l'index du tableau */
	$i=0;
	foreach ($rawdata as $line) 	{
		
				
		/* Separe la ligne par les espace */
		$aline = preg_split("/\s+/", $line);
	
		/* Si ATA alors NEXT */
		if ($aline[0] == "ATA")
			continue ;
	
		$diskname = chop($aline[2]);
		
		/* Exlude disk (adX) and Empty (no) */
		if (!preg_match("/^(ad)/", $diskname ) & $diskname != "no") 	{
			
			$disklist[$diskname]=array();
			
			// $disklist[$i]['name'] = chop($aline[2]);
			
			
			$disklist[$diskname]['type'] = "IDE";
		
			/* Match the description witch is include between < and > */
		
			preg_match("/.*\<([^>]*)>.*/",$line,$match);
		
			$disklist[$diskname]['desc'] = $match[1];

			
			$i++;
			
		}
		
	}
	
	/* Get the SCSI disk list */
	
	/* Cleaning used variable and initialize array */
	unset($rawdata);
	unset($dmesgtab);
	reset($rawdmesg);
	
	/* Get the result of the command camcontrol*/
	exec("/sbin/camcontrol devlist",$rawdata);
		
	foreach ($rawdata as $line) 	{
	
		/* Get information include between parenthese: (pass0,da0) or (da0,pass0)*/
		
		preg_match("/.*\(([^>]*)\).*/",$line,$match);
	
		/* Sépare le resultat par la virgule  */
		$temp = preg_split("/,/", $match[1]);
		
		// Check if diskname is the first (da0,pass0) or the second (pass0,da0) arguement
		$diskname = $temp[1];
		if ($diskname[0] == "p")
			$diskname = $temp[0];
		
		/* On exlus les lecteurs disque dur da */
		if (!preg_match("/^(da)/", $diskname  )) 	{
			$disklist[$diskname]=array();
			$disklist[$diskname]['type'] = "SCSI";
			/* Recupère la description: ce qu'il y a entre < et > */
		
			preg_match("/.*\<([^>]*)>.*/",$line,$match);
		
			$disklist[$diskname]['desc'] = $match[1];
			
			$i++;
		
		}
		
	}
	
		return $disklist;
}

function freenas_install($src,$dst)
{
	
	// Creating source directory
	passthru("/bin/mkdir /mnt/src");
	
	// Mount the source function is_cdrom must BE CREATED!!!!
	
	if (is_cdrom($src))
	{
		echo "Mount CDROM:\n";
		passthru("/sbin/mount_cd9660 /dev/$src /mnt/src");
	}
	else
	{
		echo "Mount disk:\n";
		passthru("/sbin/mount -t ufs /dev/$src /mnt/src");
	}
	
	
	echo "Erasing primary partitions:\n";
	passthru("/bin/dd if=/dev/zero of=/dev/$dst bs=1k count=20");
	
	echo "Creation 1 booting primary parition:\n";
	passthru("/sbin/fdisk -BI -b /boot/boot0 $dst");
	
	/* echo "Installing bootloader:\n";
	passthru("/usr/sbin/boot0cfg -B -b /boot/boot0 $dst"); */
	
	echo "Erasing FreeBSD partition on the primary:\n";
	passthru("/bin/dd if=/dev/zero of=/dev/" . escapeshellarg($dst) . "s1 bs=32k count=16");
	
	echo "Creating FreeBSD partition:\n";
	passthru("/sbin/bsdlabel -B -w -b /boot/boot " . escapeshellarg($dst) ."s1 auto");
	
	echo "Modify partition information:\n";
	
	/* Generation de la table des partitions dans un fichier temp */

	passthru("/sbin/bsdlabel " . escapeshellarg($dst) ."s1 > label.tmp");

	/* copie de ce fichier  dans un tableau*/
	$tableau = file("label.tmp");

	// Ouverture du fichier en mode ajout
	$handle = fopen("label.tmp", 'a');
	
	while(list(,$val) = each($tableau))
	{
		// Si la ligne contient le mot "unused"

		if (ereg ("unused",$val))
		{
			// On remplace c: par a:
			$val = ereg_replace ("c:","a:", $val);
			// On remplace unused par 4.2BSD
			$val = ereg_replace ("unused","4.2BSD", $val);
			// On ajoute cette ligne à la fin du fichier
			fwrite($handle, $val);
		}
	}
	// Fermeture du fichier
	fclose($handle);
	
	// On injecte la nouvelle table des paritions 
	passthru("/sbin/bsdlabel -R -B " . escapeshellarg($dst) ."s1 label.tmp");
	
	echo "Creating filesystem:\n";
	passthru("/sbin/newfs /dev/" . escapeshellarg($dst) . "s1");
	
	echo "Creation temp mouting point:\n";
	passthru("/bin/mkdir /mnt/src");
	
	echo "Mount disk:\n";
	passthru("/sbin/mount /dev/" . escapeshellarg($dst) . "s1 /mnt/dst");
	
	echo "Installation:\n";
	
	passthru("/bin/cp -pr /mnt/src/boot /mnt/dst");
	passthru("/bin/cp /mnt/src/cf.gz /mnt/dst/mfsroot.gz");
	
	if ($src == CDROM)
		passthru("/bin/rm -f /mnt/dst/boot/cdloader");
	
	passthru("mkdir /mnt/dst/conf");
	passthru("/bin/cp /conf/config.xml /mnt/dst/conf/");
	
	
	/* demontage des disques */
	passthru("/sbin/umount /mnt/dst"); 
	passthru("/sbin/umount /mnt/src");
	passthru("/bin/rm -rf /mnt/src");
	passthru("/bin/rm -rf /mnt/dst");

}

/* Force kill a process by name */
function forcekillbyname($procname) {
	return mwexec("/usr/bin/killall -9 " . escapeshellarg($procname));
}
?>