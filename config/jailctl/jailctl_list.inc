<?php
include('/usr/local/pkg/jailctl.inc');

$jconfig=$config['installedpackages']['pfjailctlsettings']['config'][0];
$jails=$config['installedpackages']['pfjailctl']['config'];
is_array($jails) ? $num_jails=count($jails) : $num_jails=0;


if($num_jails) {
	for ($i=0;$i<$num_jails;$i++) {
		$jail=$jails[$i];
		$jstatus=jailctl_jailstatus($config, $jail);
		$_path=$jstatus['jpath'];
		$_name=$jail['jname'];
		$_ip=$jail['jip'];
		$_hostname=$jstatus['jhostname'];
		$_dnsservers=$jconfig['jdnsservers'];
		$_passwd=$jconfig['jpasswd'];
		$_template=$jail['jtemplates'];
		$mounts=$GLOBALS['jailctl_tmp']['mounts'];
		$unionmounts=$GLOBALS['jailctl_tmp']['unionmounts'];
		$mount_unionfs=$GLOBALS['jailctl_tmp']['mount_unionfs'];
		$mount_nullfs=$GLOBALS['jailctl_tmp']['mount_nullfs'];

		if($_GET['jaction'] && $_GET['id']==$i) {
			switch ($_GET['jaction']) {
				case 'create':
#					if($jstate!='New') break;
					jailctl_create($config, $jail);
					if($jconfig['debug']) {
						echo("Create script:<br><pre>");
						echo(file_get_contents("/tmp/jcreate_".$_name));
						echo("</pre>");
					} else {
						header('Location: /pkg.php?xml=jailctl.xml');
					}
					flush();
					exit;
					
				case 'start':
#					if($jstate!='Stopped') break;
					exec('umount -f '.$_path.'/dev');
					if($_template) {
						$jail['juse_unionfs']=='on' ? $mount=$mount_unionfs : $mount=$mount_nullfs ;
						foreach($mounts as $mp) {
							exec($mount.' '.$_template.'/'.$mp.' '.$_path.'/'.$mp, $_cmd);
						}
						foreach($unionmounts as $mp) {
							exec($mount_unionfs.' '.$_template.'/'.$mp.' '.$_path.'/'.$mp, $_cmd);
						}
					}

					$cmd='jailctl start '.$_name;
					exec($cmd, $_cmd);
					if($jconfig['debug']) {
						echo("<script>alert('".addcslashes(implode("\\n", $_cmd), "'")."'); this.location.replace('/pkg.php?xml=jailctl.xml');</script>");
					} else {
						header('Location: /pkg.php?xml=jailctl.xml');
					}
					flush();
					exit;

				case 'stop':
#					if($jstate!='Running') break;
					$cmd='jailctl stop '.$_name;
					exec($cmd, $_cmd);
					if($_template) {
						foreach($mounts as $mp) {
							exec('umount -f '.$_template.'/'.$mp.' '.$_path.'/'.$mp, $_cmd);
						}
						foreach($unionmounts as $mp) {
							exec('umount -f '.$_template.'/'.$mp.' '.$_path.'/'.$mp, $_cmd);
						}
					}
					exec('umount -f '.$_path.'/dev');
					if($jconfig['debug']) {
						echo("<script>alert('".addcslashes(implode("\\n", $_cmd), "'")."'); this.location.replace('/pkg.php?xml=jailctl.xml');</script>");
					} else {
						header('Location: /pkg.php?xml=jailctl.xml');
					}
					flush();
					exit;
			}
		}

		$config['installedpackages']['pfjailctl']['config'][$i]['jstate']=$jstatus['jstate'];
		$config['installedpackages']['pfjailctl']['config'][$i]['jpath']=$jstatus['jpath'];
		$config['installedpackages']['pfjailctl']['config'][$i]['jifdesc']=
			$jstatus['jifdesc']." (".$jstatus['jrealif'].")";
		
		$jactions=Array();
		switch ($jstatus['jstate']) {
			case 'Building':
				$jactions[]="";
				break;
			case 'New':
				$jactions[]="<a href='/pkg.php?xml=jailctl.xml&jaction=create&id=".$i."'>Create</a>";
				break;
			case 'Stopped':
				$jactions[]="<a href='/pkg.php?xml=jailctl.xml&jaction=start&id=".$i."'>Start</a>";
				#$jactions[]="<a href='/pkg.php?xml=jailctl.xml&jaction=upgrade&id=".$i."'>Upgrade</a>";
				break;
			case 'Running':
				$jactions[]="<a href='/pkg.php?xml=jailctl.xml&jaction=stop&id=".$i."'>Stop</a>";
				break;
		}
		$config['installedpackages']['pfjailctl']['config'][$i]['jactions']=implode(' | ', $jactions);
	}
}


?>