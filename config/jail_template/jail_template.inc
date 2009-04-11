<?php
function jail_template_install() {
	exec('mkdir /usr/jail_template');
	## exec('tar -C /usr/jail_template -yxf /tmp/jail_template.tar.bz2 2&gt;&amp;1 &amp;');
	$_rcfile['file']='000.jail_template.sh';
	$_rcfile['start']='';
	$_rcfile['stop']='';
	$_rcfile['start'].='mount -o ro /dev/`mdconfig -af /usr/jail_template.img.uzip`.uzip /usr/jail_template '."\n\t";
	$_rcfile['stop'].='umount -f /usr/jail_template '."\n\t";
	$_rcfile['stop'].='mdconfig -d -u `expr "$(mdconfig -l -v | grep jail_template | grep -v grep)" : "md\([0-9]*\)"` '."\n\t";
	write_rcfile($_rcfile);
	exec('mount -o ro /dev/`mdconfig -af /usr/jail_template.img.uzip`.uzip /usr/jail_template');
	
	$GLOBALS['config']['installedpackages']['pfjailctltemplates']['config'][0]['templates']['jail_template']['name']=
		'pfSense default jail template';
	$GLOBALS['config']['installedpackages']['pfjailctltemplates']['config'][0]['templates']['jail_template']['path']=
		'/usr/jail_template';
}

function jail_template_deinstall() {
	exec('umount -f /usr/jail_template');
	exec('mdconfig -d -u `expr "$(mdconfig -l -v | grep jail_template | grep -v grep)" : "md\([0-9]*\)');
	unset($GLOBALS['config']['installedpackages']['pfjailctltemplates']['config'][0]['templates']['jail_template']);
}

function jail_template_resync() {
	$GLOBALS['config']['installedpackages']['pfjailctltemplates']['config'][0]['templates']['jail_template']['name']=
		'pfSense default jail template';
	$GLOBALS['config']['installedpackages']['pfjailctltemplates']['config'][0]['templates']['jail_template']['path']=
		'/usr/jail_template';
}
?>