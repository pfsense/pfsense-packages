<?php
$GLOBALS['jailctl_tmp']['mounts'][]='bin';
$GLOBALS['jailctl_tmp']['mounts'][]='lib';
$GLOBALS['jailctl_tmp']['mounts'][]='libexec';
$GLOBALS['jailctl_tmp']['mounts'][]='sbin';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/bin';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/include';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/lib';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/libdata';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/libexec';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/sbin';
$GLOBALS['jailctl_tmp']['mounts'][]='usr/share';

$GLOBALS['jailctl_tmp']['unionmounts'][]='etc';
$GLOBALS['jailctl_tmp']['unionmounts'][]='var';
$GLOBALS['jailctl_tmp']['unionmounts'][]='usr/local/etc';

$GLOBALS['jailctl_tmp']['mount_unionfs']='mount -t unionfs -o below ';
$GLOBALS['jailctl_tmp']['mount_nullfs']='mount -t nullfs -o ro';

function jailctl_initialize_jail($jail, $jstatus, $jconfig) {
	$cmd='sed -I "" -e s/"^root:[^:]*:"/"root:'.addcslashes($jconfig['jpasswd'], '$/').':"/ '.$jstatus['jpath'].'/etc/master.passwd '."\n";
	$cmd.='sed -I "" -e s/"^.PermitRootLogin.*"/"PermitRootLogin yes"/ '.$jstatus['jpath'].'/etc/ssh/sshd_config '."\n";
	$cmd.="pwd_mkdb -d ".$jstatus['jpath']."/etc ".$jstatus['jpath']."/etc/master.passwd \n";
	$cmd.="echo '".$jail['jip']."    ".$jail['jname']." ".$jstatus['jhostname'][0]."' >> ".$jstatus['jpath']."/etc/hosts \n";
	$cmd.="echo 'hostname=\"".$jail['jname']."\"' > ".$jstatus['jpath']."/etc/rc.conf \n";

	if(is_array($jconfig['row'])) {
		foreach($jconfig['row'] as $_row) {
			$cmd.="echo '".$_row['rcconfline']."' >> ".$jstatus['jpath']."/etc/rc.conf \n";
			#$cmd.="echo '".str_replace(' ', "\n", $jconfig['jrcconf'])."' >> ".$jstatus['jpath']."/etc/rc.conf \n";
		}
	}
	$cmd.="echo 'domain ".$jstatus['jhostname'][1]."' > ".$jstatus['jpath']."/etc/resolv.conf \n";
	foreach(explode(' ', trim($jconfig['jdnsservers'])) as $_ns) {
		$cmd.="echo 'nameserver ".str_replace(' ', "\nnameserver ", $_ns)."' >> ".$jstatus['jpath']."/etc/resolv.conf \n";
	}
	return $cmd;
}

function jailctl_create_template($jail, $jstatus, $jconfig) {
	$mounts=$GLOBALS['jailctl_tmp']['mounts'];
	$unionmounts=$GLOBALS['jailctl_tmp']['unionmounts'];
	$mount_unionfs=$GLOBALS['jailctl_tmp']['mount_unionfs'];
	$mount_nullfs=$GLOBALS['jailctl_tmp']['mount_nullfs'];
	
	$_path=$jstatus['jpath'];
	$_name=$jail['name'];
	$_ip=$jail['jip'];
	$_hostname=$jstatus['jhostname'];
	$_dnsservers=$jconfig['jdnsservers'];
	$_passwd=$jconfig['jpasswd'];
	$_template=$jail['jtemplates'];
	
	exec('mkdir -p '.$_path.'/dev');
	exec('mkdir -p '.$_path.'/root');
	exec('ln -s /usr/home '.$_path.'/home');

	$jail['juse_unionfs']=='on' ? $mount=$mount_unionfs : $mount=$mount_nullfs ;
	
	foreach($mounts as $mp) {
		$cmd[]='mkdir -p '.$_path.'/'.$mp;
		$cmd[]=$mount.' '.$_template.'/'.$mp.' '.$_path.'/'.$mp;
	}
	
	foreach($unionmounts as $mp) {
		$cmd[]='mkdir -p '.$_path.'/'.$mp;
		$cmd[]=$mount_unionfs.' '.$_template.'/'.$mp.' '.$_path.'/'.$mp;
	}
	
	foreach($cmd as $_t) {
#		exec($_t.' 2>&1 ', $_mount);
		exec($_t.' 2>&1 ');
	}
#	foreach($_mount as $_out) {
#		echo $_out."<br>";
#	}
	
	$cmd=jailctl_initialize_jail($jail, $jstatus, $jconfig);
	$fd=fopen('/tmp/jcreate_'.$jail['jname'], 'w+');
	fwrite($fd, $cmd);
	fclose($fd);
	exec("sh /tmp/jcreate_".$jail['jname']." 2>&1 >/var/log/jcreate_exec &");
	
	foreach(array_reverse($mounts, true) as $mp) {
		exec('umount -f '.$_path.'/'.$mp);
	}
	
	foreach(array_reverse($unionmounts, true) as $mp) {
		exec('umount -f '.$_path.'/'.$mp);
	}
}

function jailctl_create_sysinstall($jail, $jstatus, $jconfig) {
	exec('mkdir -p '.$jstatus['jpath']);
	$cmd="sysinstall ";
		$cmd.="_ftpPath=".$jconfig['jftpmirror']." ";
		$cmd.="nonInteractive=yes ";
		$cmd.="mediaSetFTP ";
		$cmd.="releaseName=7.1-RELEASE ";
		if($jail['jdists']) {
			$cmd.="dists=\"base ".str_replace(',', ' ', $jail['jdists'])."\" ";
			$cmd.="distSetCustom ";
		} else {
			$cmd.=$jail['jdistset']." ";
		}
		$cmd.="installRoot=".$jstatus['jpath']." ";
		$cmd.="installCommit ";
		$cmd.="> /var/log/jcreate.log 2>/var/log/jcreate.err \n";
		$cmd.="\n";

	$cmd.=jailctl_initialize_jail($jail, $jstatus, $jconfig);
	
	$fd=fopen('/tmp/jcreate_'.$jail['jname'], 'w+');
	fwrite($fd, $cmd);
	fclose($fd);
	
	exec("sh /tmp/jcreate_".$jail['jname']." 2>&1 >/var/log/jcreate_exec &");
}

function jailctl_find_if($ip) {
	exec("ifconfig | grep -C 1 '".addcslashes($ip, ".")."'", $_ifconfig);
	$if=explode(':', $_ifconfig[0]);
	
	return $if[0];
}

function jailctl_addinterfaces($config) {
	$vips=$config['virtualip']['vip'];
	?>
	<script type="text/javascript">
	function isValidIPAddress(ipaddr) {
		var re = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
		if (re.test(ipaddr)) {
			var parts = ipaddr.split(".");
			if (parseInt(parseFloat(parts[0])) == 0) { return false; }
			for (var i=0; i<parts.length; i++) {
				if (parseInt(parseFloat(parts[i])) > 255) { return false; }
			}
			return true;
		} else {
			return false;
		}
	}
	function addinterfaces() {
		<?php
	
		foreach($vips as $vip) {
			if($vip['mode']=='carp') {
				echo("document.forms['iform'].jif.appendChild(new Option('".$vip['descr']." (".$vip['subnet'].")', '".$vip['subnet']."'));");
			}
		}
		
		?>
	}
	addinterfaces();

	for (var i=0; i < document.forms['iform'].jif.length; i++) {
		<?php
		$jif=$config['installedpackages']['pfjailctl']['config'][$_GET['id']]['jif'];
		echo('if (document.forms["iform"].jif[i].value == "'.$jif.'") {');
		?>
			document.forms['iform'].jif[i].selected = true;
		} else {
			document.forms['iform'].jif[i].selected = false;
		}	
	}
	if (isValidIPAddress(document.forms['iform'].jif.value)) {
		document.forms['iform'].jip.disabled=true;
	}

	function setdisableip() {
		if (isValidIPAddress(document.forms['iform'].jif.value)) {
			document.forms['iform'].jip.value=document.forms['iform'].jif.value;
			document.forms['iform'].jip.disabled=true;
		} else {
			// document.forms['iform'].jip.value='';
			document.forms['iform'].jip.disabled=false;
		}
	}
	document.forms['iform'].jif.addEventListener('change', setdisableip, true);
	</script>
	<?php
}

function jailctl_addtemplates($config) {
	$templates=$config['installedpackages']['pfjailctltemplates']['config'][0]['templates'];
	?>
	<script type="text/javascript">
	function addtemplates() {
		<?php
		foreach($templates as $template) {
			echo("document.forms['iform'].jtemplates.appendChild(new Option('".$template['name']."', '".$template['path']."'));");
		}
		?>
	}
	addtemplates();
	</script>
	<?php
}


function jailctl_jailstatus($config, $jail) {
	$jconfig=$config['installedpackages']['pfjailctlsettings']['config'][0];
	#echo("<pre>"); print_r($jail); echo("</pre>");


	$jname=$jail['jname'];
	$jhostname=explode('.', $jname, 2);
	$jpath=$jconfig['jstorage']."/".$jname;
	$jip=$jail['jip'];
	$jif=$jail['jif'];

	if(is_ipaddr($jif)) {
		$jrealif=jailctl_find_if($jif);
		#$jrealif=guess_interface_from_ip($jif);
		#$jrealif=link_carp_interface_to_parent($jif);
	} else {
		$jrealif=$config['interfaces'][$jif]['if'];
	}
	
	if($config['interfaces'][$jif]['descr']) {
		$jifdesc=$config['interfaces'][$jif]['descr'];
	} elseif(is_ipaddr($jif)) {
		$vips=$config['virtualip']['vip'];
		foreach($vips as $vip) {
			if($vip['mode']=='carp' and $vip['subnet']==$jif) {
				$jifdesc='CARP on '.strtoupper($vip['interface']);
			}
		}
	} else {
		$jifdesc=strtoupper($jail['jif']);
	}
	
	$_out=Array();
	exec('ps axwww | grep "sysinstall.*'.$jname.'" | grep -v grep', $_out);
	if(count($_out)) {
		$jstate='Building';
	} else {
		$_out=Array();
		exec('/usr/local/sbin/jailctl status '.$jname, $_out);
		#echo("<pre>"); print_r($_out); echo("</pre>");
		if (substr($_out[0], 0, strlen($jname))==$jname) {
			$_tmp=Array();
			ereg("is (.*)\.", array_shift($_out), $_tmp);
			switch ($_tmp[1]) {
				case 'down':
					$jstate='Stopped';
					break;
				case 'up':
					$jstate='Running';
					break;
				default:
					$jstate='Undefined';
					break;
			}
		} else {
			$jstate='New';
		}
	}
	
	$jstatus['jstate']=$jstate;
	$jstatus['jpath']=$jpath;
	$jstatus['jrealif']=$jrealif;
	$jstatus['jifdesc']=$jifdesc;
	$jstatus['jhostname']=$jhostname;
	
	return $jstatus;
}


?>
