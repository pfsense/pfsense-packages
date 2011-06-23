<?php
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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

// unset crsf checks
if(isset($_POST['__csrf_magic'])) {
  unset($_POST['__csrf_magic']);
}

require_once("pfsense-utils.inc");
require_once("config.inc");
require_once("functions.inc");

/* Allow additional execution time 0 = no limit. */
ini_set('max_execution_time', '9999');
ini_set('max_input_time', '9999');

function snort_postinstall()
{
	global $config;
	conf_mount_rw();

	/* find out if were in 1.2.3-RELEASE */
	$pfsense_ver_chk = exec('/bin/cat /etc/version');
	if ($pfsense_ver_chk == '1.2.3-RELEASE') {
		$pfsense_stable = 'yes';
	}else{
		$pfsense_stable = 'no';
	}

	/* find out what arch where in x86 , x64 */
	$snort_arch_ck = '';
	exec('/usr/bin/uname -m', $snort_arch_ck);
	if($snort_arch_ck[0] == 'i386') {
		$snort_arch = 'x86';
	}else{
		$snort_arch = 'x64';
	}

	/* snort -> advanced features */
	//$bpfbufsize = $config['installedpackages']['snortglobal']['bpfbufsize'];
	//$bpfmaxbufsize = $config['installedpackages']['snortglobal']['bpfmaxbufsize'];
	//$bpfmaxinsns = $config['installedpackages']['snortglobal']['bpfmaxinsns'];

	// create a few directories and ensure the sample files are in place
	if(!file_exists('/usr/local/etc/snort')) {
		exec('/bin/mkdir -p /usr/local/etc/snort');
	}

	if(!file_exists('/usr/local/etc/snort/whitelist')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/whitelist/');
	}

	if(!file_exists('/var/log/snort/run')) {
		exec('/bin/mkdir -p /var/log/snort/run');
	}

	if(!file_exists('/var/log/snort/barnyard2')) {
		exec('/bin/mkdir -p /var/log/snort/barnyard2/');
	}

	if(!file_exists('/usr/local/lib/snort/dynamicrules/')) {
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
	}

	// for snort2c, remove when snortsam is working
	if(!file_exists('/var/db/whitelist')) {
		touch('/var/db/whitelist');
	}
	
	if (!file_exists('/usr/local/etc/snort/etc')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/etc');
	}
	
	if (!file_exists('/usr/local/etc/snort/signatures')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/signatures');
	}	
	
	if (!file_exists('/usr/local/etc/snort/snort_download')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/snort_download');
	}
	
	if (!file_exists('/usr/local/etc/snort/snortDBrules/DB')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/snortDBrules/DB');
	}

	if (!file_exists('/usr/local/etc/snort/snortDBrules/custom_rules')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/snortDBrules/custom_rules');
	}

	if (!file_exists('/usr/local/etc/snort/snortDBrules/emerging_rules')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/snortDBrules/emerging_rules');
	}	

	if (!file_exists('/usr/local/etc/snort/snortDBrules/pfsense_rules')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/snortDBrules/pfsense_rules');
	}

	if (!file_exists('/usr/local/etc/snort/snortDBrules/snort_rules')) {
		exec('/bin/mkdir -p /usr/local/etc/snort/snortDBrules/snort_rules');
	}
	
	if (!file_exists('/var/snort/')) {
		exec('/bin/mkdir -p /var/snort/');
	}
	
	if (!file_exists('/usr/local/pkg/snort/snortDBtemp')) {
		exec('/bin/mv /var/snort/snortDBtemp');
	}	
	
	// cleanup default files
	if(file_exists('/usr/local/etc/snort/snort.conf-sample')) {
		exec('/bin/rm /usr/local/etc/snort/classification.config-sample');
		exec('/bin/mv /usr/local/etc/snort/classification.config /usr/local/etc/snort/etc/classification.config');				
		exec('/bin/rm /usr/local/etc/snort/gen-msg.map-sample');
		exec('/bin/mv /usr/local/etc/snort/gen-msg.map /usr/local/etc/snort/etc/gen-msg.map');		
		exec('/bin/rm /usr/local/etc/snort/reference.config-sample');
		exec('/bin/mv /usr/local/etc/snort/reference.config /usr/local/etc/snort/etc/reference.config');		
		exec('/bin/rm /usr/local/etc/snort/sid-msg.map-sample');
		exec('/bin/mv /usr/local/etc/snort/sid-msg.map /usr/local/etc/snort/etc/sid-msg.map');				
		exec('/bin/rm /usr/local/etc/snort/snort.conf-sample');
		exec('/bin/mv /usr/local/etc/snort/snort.conf /usr/local/etc/snort/etc/snort.conf');				
		exec('/bin/rm /usr/local/etc/snort/threshold.conf-sample');
		exec('/bin/mv /usr/local/etc/snort/threshold.conf /usr/local/etc/snort/etc/threshold.conf');
		exec('/bin/rm /usr/local/etc/snort/unicode.map-sample');
		exec('/bin/mv /usr/local/etc/snort/unicode.map /usr/local/etc/snort/etc/unicode.map');		
		exec('/bin/rm /usr/local/etc/snort/generators-sample');
		exec('/bin/mv /usr/local/etc/snort/generators /usr/local/etc/snort/etc/generators');
		exec('/bin/rm /usr/local/etc/snort/sid');
		exec('/bin/rm /usr/local/etc/rc.d/snort');
		exec('/bin/rm /usr/local/etc/rc.d/bardyard2');
	}

	// remove example files
	if(file_exists('/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so.0')) {
		exec('/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example*');
	}

	if(file_exists('/usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example.so')) {
		exec('/bin/rm /usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example*');
	}


	// add snort user and group note: 920 keep the numbers < 2000, above this is reserved in pfSense 2.0
	exec('/usr/sbin/pw groupadd snort -g 920');
	exec('/usr/sbin/pw useradd snort -u 920 -c "Snort User" -d /nonexistent -g snort -s /sbin/nologin');

	// if users have old log files delete them */
	if(!file_exists('/var/log/snort/alert')) {
		touch('/var/log/snort/alert');
	}else{
		exec('/bin/rm -rf /var/log/snort/*');
		touch('/var/log/snort/alert');
	}

	// rm barnyard2 important */
	if(!file_exists('/usr/local/bin/barnyard2')) {
		exec('/bin/rm /usr/local/bin/barnyard2');
	}

	/* important  */
	exec('/usr/sbin/chown -R snort:snort /var/log/snort');
	exec('/usr/sbin/chown -R snort:snort /usr/local/etc/snort');
	exec('/usr/sbin/chown -R snort:snort /usr/local/lib/snort');
	exec('/usr/sbin/chown -R snort:snort /var/snort');
	exec('/usr/sbin/chown snort:snort /tmp/snort*');
	exec('/usr/sbin/chown snort:snort /var/db/whitelist');
	exec('/bin/chmod 660 /var/log/snort/alert');
	exec('/bin/chmod 660 /var/db/whitelist');
	exec('/bin/chmod -R 660  /usr/local/etc/snort/*');
	exec('/bin/chmod -R 660  /tmp/snort*');
	exec('/bin/chmod -R 660  /var/run/snort*');
	exec('/bin/chmod -R 660  /var/snort/run/*');
	exec('/bin/chmod 770 /usr/local/lib/snort');
	exec('/bin/chmod 770 /usr/local/etc/snort');
	exec('/bin/chmod 770 /usr/local/etc/whitelist');
	exec('/bin/chmod 770 /var/log/snort');
	exec('/bin/chmod 770 /var/log/snort/run');
	exec('/bin/chmod 770 /var/log/snort/barnyard2');

	/* move files around, make it look clean */
	exec('/bin/mkdir -p /usr/local/www/snort/css');
	exec('/bin/mkdir -p /usr/local/www/snort/images');
	exec('/bin/mkdir -p /usr/local/www/snort/javascript');

	chdir ("/usr/local/www/snort/css/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/css/style_snort2.css');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/css/new_tab_menu.css');
	chdir ("/usr/local/www/snort/images/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/alert.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/arrow_down.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/awesome-overlay-sprite.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/controls.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/down.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/down2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/footer.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/footer2.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon-table-sort-asc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon-table-sort-desc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon-table-sort.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon_excli.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/loading.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/logo.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/logo22.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/page_white_text.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/transparent.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/up.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/up2.gif');
	chdir ("/usr/local/www/snort/javascript/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/jquery-1.6.1.min.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/jquery.form.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/snort_globalsend.js');

	/* back to default */
	chdir ('/root/');

	// make sure snort-old is deinstalled
	// remove when snort-old is removed
	unset($config['installedpackages']['snort']);
	unset($config['installedpackages']['snortdefservers']);
	unset($config['installedpackages']['snortwhitelist']);
	unset($config['installedpackages']['snortthreshold']);
	unset($config['installedpackages']['snortadvanced']);
	write_config();
	conf_mount_rw();

	// remake saved settings
	// TODO: make sre this works in final release
	/*
	if($config['installedpackages']['snortglobal']['forcekeepsettings'] == 'on') {
		update_status(gettext("Saved settings detected..."));
		update_output_window(gettext("Please wait... rebuilding files..."));
		sync_snort_package_empty();
		update_output_window(gettext("Finnished Rebuilding files..."));
	}
	*/

	conf_mount_ro();

}

function snort_deinstall()
{

	global $config, $g;
	conf_mount_rw();

	// remove custom sysctl //
	remove_text_from_file("/etc/sysctl.conf", "sysctl net.bpf.bufsize=20480");
	
	// decrease bpf buffers back to 4096, from 20480
	exec('/sbin/sysctl net.bpf.bufsize=4096');
	
	exec('/usr/usr/bin/killall snort');
	sleep(2);
	exec('/usr/usr/bin/killall -9 snort');
	sleep(2);
	exec('/usr/usr/bin/killall barnyard2');
	sleep(2);
	exec('/usr/usr/bin/killall -9 barnyard2');
	sleep(2);
	
	exec('/usr/sbin/pw userdel snort');
	exec('/usr/sbin/pw groupdel snort');
	exec('rm -rf /usr/local/etc/snort*');
	exec('rm -rf /usr/local/pkg/snort*');
	exec('rm -rf /usr/local/pkg/pf/snort*');

	exec("cd /var/db/pkg && pkg_delete `ls | grep snort`");
	exec("cd /var/db/pkg && pkg_delete `ls | grep perl-threaded`");
	exec("cd /var/db/pkg && pkg_delete `ls | grep mysql-client-5.1.50_1`");
	exec('rm -r /usr/local/bin/barnyard2');

	// TODO: figure out how to detect pfsense packages that use the same freebsd pkckages  and not deinstall
	//exec("cd /var/db/pkg && pkg_delete `ls | grep perl`");
	//exec("cd /var/db/pkg && pkg_delete `ls | grep barnyard2`");
	//exec("cd /var/db/pkg && pkg_delete `ls | grep pcre`"); // Never remove pcre or pfsense will break

	// Remove snort cron entries Ugly code needs smoothness 
	// TODO: redo code because its a mess
	function snort_rm_blocked_deinstall_cron($should_install)
	{
		global $config, $g;
		conf_mount_rw();

		$is_installed = false;

		if(!$config['cron']['item'])
		return;

		$x=0;
		foreach($config['cron']['item'] as $item)
		{
			if (strstr($item['command'], "snort2c"))
			{
				$is_installed = true;
				break;
			}

			$x++;
				
		}
		if($is_installed == true)
		{
			if($x > 0)
			{
				unset($config['cron']['item'][$x]);
				write_config();
				conf_mount_rw();
			}

			configure_cron();

		}
		conf_mount_ro();

	}

	function snort_rules_up_deinstall_cron($should_install)
	{
		global $config, $g;
		conf_mount_rw();

		$is_installed = false;

		if(!$config['cron']['item'])
		return;

		$x=0;
		foreach($config['cron']['item'] as $item) {
			if (strstr($item['command'], "snort_check_for_rule_updates.php")) {
				$is_installed = true;
				break;
			}
			$x++;
		}
		if($is_installed == true) {
			if($x > 0) {
				unset($config['cron']['item'][$x]);
				write_config();
				conf_mount_rw();
			}
			configure_cron();
		}
	}

	snort_rm_blocked_deinstall_cron("");
	snort_rules_up_deinstall_cron("");


	/* Unset snort registers in conf.xml IMPORTANT snort will not start with out this */
	/* Keep this as a last step */
	if($config['installedpackages']['snortglobal']['forcekeepsettings'] != 'on') {
		unset($config['installedpackages']['snortglobal']);
	}
	write_config();
	conf_mount_rw();

	exec('rm -rf /usr/local/www/snort');
	exec('rm -rf /usr/local/lib/snort/');
	exec('rm -rf /var/log/snort/');
	exec('rm -rf /usr/local/pkg/snort');
	exec('rm -rf /var/snort');	

	conf_mount_ro();

}

// make sure this func on writes to files and does not start snort */
function sync_snort_package()
{
	global $config, $g;
	conf_mount_rw();



	conf_mount_ro();
}

?>
