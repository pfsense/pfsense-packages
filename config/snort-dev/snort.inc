<?php
/* $Id$ */
/*
	snort.inc
	Copyright (C) 2006 Scott Ullrich
	Copyright (C) 2009 Robert Zelaya
	part of pfSense
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

require_once("pfsense-utils.inc");

// Needed on 2.0 because of filter_get_vpns_list()
require_once("filter.inc");

/* find out if were in 1.2.3-RELEASE */

$pfsense_ver_chk = exec('/bin/cat /etc/version');
if ($pfsense_ver_chk == '1.2.3-RELEASE')
{
	$pfsense_stable = 'yes';
}else{
	$pfsense_stable = 'no';
}    

/* Get id and realinterfaces */

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];

$interface_fake = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
$if_real = convert_friendly_interface_to_real_interface_name($interface_fake);

/* get the real iface name of wan */
function convert_friendly_interface_to_real_interface_name2($interface)
{
        global $config;
		
        $lc_interface = strtolower($interface);
        if($lc_interface == "lan") return $config['interfaces']['lan']['if'];
        if($lc_interface == "wan") return $config['interfaces']['wan']['if'];
        $ifdescrs = array();
        for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++)
                $ifdescrs['opt' . $j] = "opt" . $j;
        foreach ($ifdescrs as $ifdescr => $ifname)
		{
                if(strtolower($ifname) == $lc_interface)
            return $config['interfaces'][$ifname]['if'];
                if(strtolower($config['interfaces'][$ifname]['descr']) == $lc_interface)
                        return $config['interfaces'][$ifname]['if'];
		}
   
   return $interface;
}

$if_real_wan = convert_friendly_interface_to_real_interface_name2($interface_fake);

/* Allow additional execution time 0 = no limit. */
ini_set('max_execution_time', '9999');
ini_set('max_input_time', '9999');

/* define oinkid */
if($config['installedpackages']['snortglobal'])
	$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];
	
function snort_postinstall()
{
	global $config;
	conf_mount_rw();

	if(!file_exists("/var/log/snort/"))
	mwexec("mkdir -p /var/log/snort/");

	if(!file_exists("/var/log/snort/alert"))
		touch("/var/log/snort/alert");

	/* snort -> advanced features */
	$bpfbufsize = $config['installedpackages']['snortglobal']['bpfbufsize'];
	$bpfmaxbufsize = $config['installedpackages']['snortglobal']['bpfmaxbufsize'];
	$bpfmaxinsns = $config['installedpackages']['snortglobal']['bpfmaxinsns'];

	
	/* create a few directories and ensure the sample files are in place */
	exec("/bin/mkdir -p /usr/local/etc/snort");
	exec("/bin/mkdir -p /var/log/snort");
	exec("/bin/mkdir -p /usr/local/etc/snort/rules");

	if(file_exists("/usr/local/etc/snort/snort.conf-sample")) 
	{
		exec("/bin/rm /usr/local/etc/snort/snort.conf-sample");
		exec("/bin/rm /usr/local/etc/snort/threshold.conf-sample");
		exec("/bin/rm /usr/local/etc/snort/sid-msg.map-sample");
		exec("/bin/rm /usr/local/etc/snort/unicode.map-sample");
		exec("/bin/rm /usr/local/etc/snort/classification.config-sample");
		exec("/bin/rm /usr/local/etc/snort/generators-sample");
		exec("/bin/rm /usr/local/etc/snort/reference.config-sample");
		exec("/bin/rm /usr/local/etc/snort/gen-msg.map-sample");
		exec("/bin/rm /usr/local/etc/snort/sid");
		exec("/bin/rm /usr/local/etc/rc.d/snort");
		exec("/bin/rm /usr/local/etc/rc.d/bardyard2");
	}
	
	if(!file_exists("/usr/local/etc/snort/custom_rules"))
	{
		exec("/bin/mkdir -p /usr/local/etc/snort/custom_rules/");
	}
	
	exec("/usr/sbin/pw groupadd snort");
	exec('/usr/sbin/pw useradd snort -c "SNORT USER" -d /nonexistent -g snort -s /sbin/nologin');
	exec("/usr/sbin/chown -R snort:snort /var/log/snort");
	exec("/usr/sbin/chown -R snort:snort /usr/local/etc/snort");
	exec("/usr/sbin/chown -R snort:snort /usr/local/lib/snort");
	exec("/bin/chmod -R 755  /var/log/snort");
	exec("/bin/chmod -R 755  /usr/local/etc/snort");
	exec("/bin/chmod -R 755  /usr/local/lib/snort");
	
	
	/* remove example files */
	if(file_exists("/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so.0"))
	{
		exec('/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example*');
	}
	
	if(file_exists("/usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example.so"))
	{
		exec('/bin/rm /usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example*');
	}

	/* move files around, make it look clean */
	exec('/bin/mkdir -p /usr/local/www/snort/css');
	exec('/bin/mkdir -p /usr/local/www/snort/images');
	exec('/bin/mkdir -p /usr/local/www/snort/javascript');
	
	chdir ("/usr/local/www/snort/css/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/css/style.css');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/css/style2.css');
	chdir ("/usr/local/www/snort/images/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/alert.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/down.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/down2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon-table-sort.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon-table-sort-asc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/icon-table-sort-desc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/up.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/up2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/logo.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/images/footer.jpg');
	chdir ("/usr/local/www/snort/javascript/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/jquery.blockUI.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/jquery-1.3.2.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/mootools.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/sortableTable.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort-dev/javascript/tabs.js');
	
	/* back to default */
	chdir ("/root/");
	
	conf_mount_ro();
	
}	
	
function sync_package_snort_reinstall() 
{
	global $config;
	conf_mount_rw();
	
	if(!$config['installedpackages']['snortglobal'])
		return;
    
	/* create snort configuration file */
	create_snort_conf();
  
	/* start snort service */
	// start_service("snort"); // do not start, may be needed latter.
	
	conf_mount_ro();
}

/* func for updating cron */
function snort_rm_blocked_install_cron($should_install) 
{
        global $config, $g;

        if ($g['booting']==true)
                return;

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
		
	$snort_rm_blocked_info_ck = $config['installedpackages']['snortglobal']['rm_blocked'];
		if ($snort_rm_blocked_info_ck == "1h_b")
		{
                $snort_rm_blocked_min = "*/5";
                $snort_rm_blocked_hr = "*";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "3600";
		}
		if ($snort_rm_blocked_info_ck == "3h_b")
		{
                $snort_rm_blocked_min = "*/15";
                $snort_rm_blocked_hr = "*";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "10800";
		}
		if ($snort_rm_blocked_info_ck == "6h_b")
		{
                $snort_rm_blocked_min = "*/30";
                $snort_rm_blocked_hr = "*";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "21600";
		}
		if ($snort_rm_blocked_info_ck == "12h_b")
		{
                $snort_rm_blocked_min = "2";
                $snort_rm_blocked_hr = "*/1";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "43200";
		}
		if ($snort_rm_blocked_info_ck == "1d_b")
		{
                $snort_rm_blocked_min = "2";
                $snort_rm_blocked_hr = "*/2";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "86400";
		}
		if ($snort_rm_blocked_info_ck == "4d_b")
		{
                $snort_rm_blocked_min = "2";
                $snort_rm_blocked_hr = "*/8";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "345600";
		}
		if ($snort_rm_blocked_info_ck == "7d_b")
		{
                $snort_rm_blocked_min = "2";
                $snort_rm_blocked_hr = "*/14";
                $snort_rm_blocked_mday = "*";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "604800";
		}
		if ($snort_rm_blocked_info_ck == "28d_b")
		{
                $snort_rm_blocked_min = "2";
                $snort_rm_blocked_hr = "0";
                $snort_rm_blocked_mday = "*/2";
                $snort_rm_blocked_month = "*";
                $snort_rm_blocked_wday = "*";
				$snort_rm_blocked_expire = "2419200";
		}
        switch($should_install) 
		{
                case true:
                        if(!$is_installed) 
						{		
                                $cron_item = array();
                                $cron_item['minute'] = "$snort_rm_blocked_min";
                                $cron_item['hour'] = "$snort_rm_blocked_hr";
                                $cron_item['mday'] = "$snort_rm_blocked_mday";
                                $cron_item['month'] = "$snort_rm_blocked_month";
                                $cron_item['wday'] = "$snort_rm_blocked_wday";
                                $cron_item['who'] = "root";
                                $cron_item['command'] = "/usr/bin/nice -n20 /usr/local/sbin/expiretable -t $snort_rm_blocked_expire snort2c";
                                $config['cron']['item'][] = $cron_item;
                                write_config("Installed $snort_rm_blocked_info_ck minute filter reload for Time Based Rules");
                                configure_cron();
                        }
                break;
                case false:
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
                break;
		}
}

/* func to install snort update */
function snort_rules_up_install_cron($should_install) {
        global $config, $g;

        if ($g['booting']==true)
                return;

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
	$snort_rules_up_info_ck = $config['installedpackages']['snortglobal']['autorulesupdate7'];
		if ($snort_rules_up_info_ck == "6h_up") {
                $snort_rules_up_min = "3";
                $snort_rules_up_hr = "*/6";
                $snort_rules_up_mday = "*";
                $snort_rules_up_month = "*";
                $snort_rules_up_wday = "*";
		}
		if ($snort_rules_up_info_ck == "12h_up") {
                $snort_rules_up_min = "3";
                $snort_rules_up_hr = "*/12";
                $snort_rules_up_mday = "*";
                $snort_rules_up_month = "*";
                $snort_rules_up_wday = "*";
		}
		if ($snort_rules_up_info_ck == "1d_up") {
                $snort_rules_up_min = "3";
                $snort_rules_up_hr = "0";
                $snort_rules_up_mday = "*/1";
                $snort_rules_up_month = "*";
                $snort_rules_up_wday = "*";
		}
		if ($snort_rules_up_info_ck == "4d_up") {
                $snort_rules_up_min = "3";
                $snort_rules_up_hr = "0";
                $snort_rules_up_mday = "*/4";
                $snort_rules_up_month = "*";
                $snort_rules_up_wday = "*";
		}
		if ($snort_rules_up_info_ck == "7d_up") {
                $snort_rules_up_min = "3";
                $snort_rules_up_hr = "0";
                $snort_rules_up_mday = "*/7";
                $snort_rules_up_month = "*";
                $snort_rules_up_wday = "*";
		}
		if ($snort_rules_up_info_ck == "28d_up") {
                $snort_rules_up_min = "3";
                $snort_rules_up_hr = "0";
                $snort_rules_up_mday = "*/28";
                $snort_rules_up_month = "*";
                $snort_rules_up_wday = "*";
		}
        switch($should_install) {
                case true:
                        if(!$is_installed) {		
                                $cron_item = array();
                                $cron_item['minute'] = "$snort_rules_up_min";
                                $cron_item['hour'] = "$snort_rules_up_hr";
                                $cron_item['mday'] = "$snort_rules_up_mday";
                                $cron_item['month'] = "$snort_rules_up_month";
                                $cron_item['wday'] = "$snort_rules_up_wday";
                                $cron_item['who'] = "root";
                                $cron_item['command'] = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/snort/snort_check_for_rule_updates.php >> /usr/local/etc/snort/snort_update.log";
                                $config['cron']['item'][] = $cron_item;
                                write_config("Installed 15 minute filter reload for Time Based Rules");
                                configure_cron();
                        }
                break;
                case false:
                        if($is_installed == true) {
                                if($x > 0) {
                                        unset($config['cron']['item'][$x]);
                                        write_config();
										conf_mount_rw();
                                }
                                configure_cron();
                        }
                break;
		}
}

/* make sure this func on writes to files and does not start snort */
function sync_snort_package()
{
	global $config, $g;
	conf_mount_rw();



	/* all new files are for the user snort nologin */
	if(!file_exists("/var/log/snort"))
	{
		exec("/bin/mkdir -p /var/log/snort");
	}
	
	exec("/usr/sbin/chown -R snort:snort /var/log/snort");
	exec("/usr/sbin/chown -R snort:snort /usr/local/etc/snort");
	exec("/usr/sbin/chown -R snort:snort /usr/local/lib/snort");
	exec("/bin/chmod -R 755  /var/log/snort");
	exec("/bin/chmod -R 755  /usr/local/etc/snort");
	exec("/bin/chmod -R 755  /usr/local/lib/snort");	

	conf_mount_ro();
}

/* make sure this func on writes to files and does not start snort */
function sync_snort_package_all()
{
	global $config, $g, $id, $if_real, $interface_fake;
	conf_mount_rw();



/* RedDevil suggested code */
/* TODO: more testing needs to be done */
exec("/sbin/sysctl net.bpf.bufsize=8388608");
exec("/sbin/sysctl net.bpf.maxbufsize=4194304");
exec("/sbin/sysctl net.bpf.maxinsns=512");
exec("/sbin/sysctl net.inet.tcp.rfc1323=1");

if ($id != '' && $if_real != '')
{

	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule'])) 
	{
		if ($id == "") 
		{

			$rule_array = $config['installedpackages']['snortglobal']['rule'];
			$id = -1;
			foreach ($rule_array as $value)
			{

				$id += 1;

				$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
				$if_real = convert_friendly_interface_to_real_interface_name($result_lan);

				/* create snort configuration file */
				create_snort_conf();

				/* create snort bootup file snort.sh */
				create_snort_sh();

				/* if rules exist cp rules to each iface */
				create_rules_iface();

				/* create barnyard2 configuration file */
				$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
				if ($snortbarnyardlog_info_chk == 'on')
					create_barnyard2_conf();

			}

		}else{

			/* create snort configuration file */
			create_snort_conf();

			/* create snort bootup file snort.sh */
			create_snort_sh();

			/* if rules exist cp rules to each iface */
			create_rules_iface();

			/* create barnyard2 configuration file */
			$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
			if ($snortbarnyardlog_info_chk == on)
			{
				create_barnyard2_conf();
			}
		}
	}

	/* all new files are for the user snort nologin */
	if(!file_exists("/var/log/snort"))
	{
		exec("/bin/mkdir -p /var/log/snort");
	}
	
	exec("/usr/sbin/chown -R snort:snort /var/log/snort");
	exec("/usr/sbin/chown -R snort:snort /usr/local/etc/snort");
	exec("/usr/sbin/chown -R snort:snort /usr/local/lib/snort");
	exec("/bin/chmod -R 755  /var/log/snort");
	exec("/bin/chmod -R 755  /usr/local/etc/snort");
	exec("/bin/chmod -R 755  /usr/local/lib/snort");	

	/* Generate the snort instance list */
	$rc_snort = "";
	$i = 0;
	$rules = &$config['installedpackages']['snortglobal']['rule'];
	foreach($rules as $snort) {
		$name = "${i}${if_real}";
		$if_real = convert_friendly_interface_to_real_interface_name($snort['interface']);
		$rc_snort .= "snort_list=\"\${snort_list} ${name}\"\n";
		$rc_snort .= "snort_${name}_conf=\"/usr/local/etc/snort/snort_${name}/snort.conf\"\n";
		$rc_snort .= "snort_${name}_name=\"${name}\"\n";
		$rc_snort .= "snort_${name}_id=\"${i}\"\n";
		$rc_snort .= "snort_${name}_interface=\"${if_real}\"\n";

		$snortenable_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['enable'];
		if ($snortenable_info_chk == 'on')
			$rc_snort .= "snort_${name}_enable=\"YES\"\n";
		else
			$rc_snort .= "snort_${name}_enable=\"NO\"\n";

		$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
		if ($snortbarnyardlog_info_chk == 'on')
			$rc_snort .= "snort_${name}_barnyard=\"YES\"\n";
		$i++;
	}
	$rcconf = fopen("/var/etc/rc.snort", "w");
	if(!$rcconf) {
		log_error("Could not open /var/etc/rc.snort for writing.");
		exit;
	}
	fwrite($rcconf, $rc_snort);
	fclose($rcconf);


	conf_mount_ro();

}
}

/* Start of main config files */
/* Start of main config files */

////////////////////////// >>>>>>>>>>


/* open snort.sh for writing" */
function create_snort_sh()
{
		# Don not add $id or this will break

        global $config, $g, $if_real, $if_real_wan;
        conf_mount_rw();

	$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];

	/* define snortbarnyardlog_chk */
	if ($snortbarnyardlog_info_chk == on) {

	$start_barnyard2 = "sleep 4/n/usr/local/bin/barnyard2 -u snort -g snort -c /usr/local/etc/snort/snort_$id$if_real/barnyard2.conf -d /var/log/snort -f snort.u2_$id$if_real -w /usr/local/etc/snort/snort_$id$if_real/barnyard2.waldo -D -q";
	
}

	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule'])) 
	{
		if ($id == "") 
		{

			$rule_array = $config['installedpackages']['snortglobal']['rule'];
			$id = -1;
			foreach ($rule_array as $value)
			{

			$id += 1;
			
			$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
			$if_real = convert_friendly_interface_to_real_interface_name($result_lan);

	/* Get all interface startup commands ready */

$snort_sh_text2[] = <<<EOD
###### For Each Iface

		# If Snort proc is NOT running
		if [ "`/bin/ps -auwx | grep -v grep | grep "R $id$if_real" | awk '{print $2;}'`" = "" ]; then

		/bin/echo "snort.sh run" > /tmp/snort.sh.pid
		
		# Start snort and barnyard2
		/bin/rm /var/run/snort_$if_real$id$if_real.pid
		/bin/rm /var/run/snort_$if_real$id$if_real.pid.lck

		/usr/local/bin/snort -u snort -g snort -R $id$if_real -D -q -l /var/log/snort -G $id -c /usr/local/etc/snort/snort_$id$if_real/snort.conf -i $if_real
		$start_barnyard2

		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort HARD Reload For $id$if_real..."

		fi
EOD;

$snort_sh_text3[] = <<<EOE

###### For Each Iface

		#### Fake start only used on bootup and Pfsense IP changes
		#### Only try to restart if snort is running on Iface
		if [ "`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "R $id$if_real" | /usr/bin/awk '{print $2;}'`" != "" ]; then

		snort_pid="`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "R $id$if_real" | /usr/bin/awk '{print $2;}'`"
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort already running, soft restart"

		#### Remake the configs on boot Important!
		/usr/local/bin/php -f /usr/local/pkg/pf/snort_dynamic_ip_reload.php $id $if_real

		#### Restart Iface
		/bin/kill -HUP \${snort_pid}
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort Soft Reload For $id$if_real..."

		fi

EOE;

$snort_sh_text4[] = <<<EOF

		pid_s=`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "R $id$if_real" | /usr/bin/awk '{print \$2;}'`
		sleep 3
		pid_b=`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "snort.u2_$id$if_real" | /usr/bin/awk '{print \$2;}'`

		if [ \${pid_s} ] ; then
		
		/bin/echo "snort.sh run" > /tmp/snort.sh.pid
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort HARD STOP For $id$if_real..."

		/bin/kill \${pid_s}
		sleep 3
		/bin/kill \${pid_b}

		/bin/rm /var/run/snort_$if_real$id$if_real.pid.lck
		/bin/rm /var/run/snort_$if_real$id$if_real.pid

		fi

EOF;

			}
		}
	}

$start_snort_iface_start = implode("\n\n", $snort_sh_text2);

$start_snort_iface_restart = implode("\n\n", $snort_sh_text3);

$start_snort_iface_stop = implode("\n\n", $snort_sh_text4);

/* open snort.sh for writing" */
conf_mount_rw();

$snort_sh_text = <<<EOD
#!/bin/sh
# This file was automatically generated
# by the pfSense service handler.
# Code added to protect from double starts on pfSense bootup
######## Begining of Main snort.sh

rc_start() {

		#### Check for double starts, Pfsense has problems with that
		if /bin/ls /tmp/snort.sh.pid > /dev/null ; then

		/usr/bin/logger -p daemon.info -i -t SnortStartup "Error: snort.sh IS running"
		exit 0

		fi

		/bin/echo "snort.sh run" > /tmp/snort.sh.pid

$start_snort_iface_restart

		/bin/rm /tmp/snort.sh.pid

		#### If on Fake start snort is NOT running DO a real start.
		if [ "`/bin/ps -auwx | grep -v grep | grep "R $id$if_real" | awk '{print $2;}'`" = "" ]; then

		rc_start_real

		fi
}

rc_start_real() {

		#### Check for double starts, Pfsense has problems with that
		if /bin/ls /tmp/snort.sh.pid > /dev/null ; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Error: snort.sh IS running"
		exit 0
		fi

$start_snort_iface_start

		/bin/rm /tmp/snort.sh.pid

}

rc_stop() {

		#### Check for double starts, Pfsense has problems with that
		if /bin/ls /tmp/snort.sh.pid > /dev/null ; then
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Error: snort.sh IS running"
		exit 0
		fi

$start_snort_iface_stop

		/bin/rm /tmp/snort.sh.pid
		/bin/rm /var/run/snort*

}

case $1 in
	start)
		rc_start
		;;
	start_real)
		rc_start_real
		;;
	stop)
		rc_stop
		;;
	restart)
		rc_stop
		rc_start_real
		;;
esac

EOD;

        /* write out snort.sh */
        $bconf = fopen("/usr/local/etc/rc.d/snort.sh", "w");
        if(!$bconf) {
                log_error("Could not open /usr/local/etc/rc.d/snort.sh for writing.");
                exit;
        }
		/* write snort.sh */
        fwrite($bconf, $snort_sh_text);
        fclose($bconf);

}


///////////////////////// >>>>>>>>>>>>

/* if rules exist copy to new interfaces */
function create_rules_iface()
{

    global $config, $g, $id, $if_real;
    conf_mount_rw();
	
	if ($id != '' || $if_real != '')
	{
		$if_rule_dir = "/usr/local/etc/snort/snort_$id$if_real/rules";
		$folder_chk = (count(glob("$if_rule_dir/*")) === 0) ? 'empty' : 'full';

		if ($folder_chk == "empty")
		{
			exec("/bin/cp -R /usr/local/etc/snort/rules /usr/local/etc/snort/snort_$id$if_real");
			if (file_exists("/usr/local/etc/snort/custom_rules/local_$id$if_real.rules"))
			{
				exec("/bin/cp /usr/local/etc/snort/custom_rules/local_$id$if_real.rules /usr/local/etc/snort/snort_$id$if_real/rules/local_$id$if_real.rules");
			}
		}
	}
}

/* open barnyard2.conf for writing */
function create_barnyard2_conf() {
        global $bconfig, $bg, $id, $if_real;
        /* write out barnyard2_conf */

		if(!file_exists("/usr/local/etc/snort/snort_$id$if_real/barnyard2.conf"))
		{
			exec("/bin//usr/bin/touch /usr/local/etc/snort/snort_$id$if_real/barnyard2.conf");
		}
		
        $barnyard2_conf_text = generate_barnyard2_conf();
        $bconf = fopen("/usr/local/etc/snort/snort_$id$if_real/barnyard2.conf", "w");
        if(!$bconf) {
                log_error("Could not open /usr/local/etc/snort/snort_$id$if_real/barnyard2.conf for writing.");
                exit;
        }
        fwrite($bconf, $barnyard2_conf_text);
        fclose($bconf);
}

/* open barnyard2.conf for writing" */
function generate_barnyard2_conf() {

        global $config, $g, $id, $if_real;
        conf_mount_rw();

/* define snortbarnyardlog */
/* TODO add support for the other 5 output plugins */

$snortbarnyardlog_database_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_mysql'];
$snortbarnyardlog_hostname_info_chk = exec("/bin/hostname");

$snortbarnyardlog_interface_info_chk = $if_real;

$barnyard2_conf_text = <<<EOD

#	barnyard2.conf
#   barnyard2 can be found at http://www.securixlive.com/barnyard2/index.php
#
#	Copyright (C) 2006 Robert Zelaya
#	part of pfSense
#	All rights reserved.
#
#	1. Redistributions of source code must retain the above copyright notice,
#	   this list of conditions and the following disclaimer.
#
#	2. Redistributions in binary form must reproduce the above copyright
#	   notice, this list of conditions and the following disclaimer in the
#	   documentation and/or other materials provided with the distribution.
#
#	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
#	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
#	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#	POSSIBILITY OF SUCH DAMAGE.
#

# set the appropriate paths to the file(s) your Snort process is using
config reference-map:   /usr/local/etc/snort/reference.config
config class-map:       /usr/local/etc/snort/classification.config
config gen-msg-map:     /usr/local/etc/snort/gen-msg.map
config sid-msg-map:     /usr/local/etc/snort/sid-msg.map

config hostname:       $snortbarnyardlog_hostname_info_chk
config interface:      $id$snortbarnyardlog_interface_info_chk

# Step 2: setup the input plugins
input unified2

# database: log to a variety of databases
# output database: log, mysql, user=xxxx password=xxxxxx dbname=xxxx host=xxx.xxx.xxx.xxxx

$snortbarnyardlog_database_info_chk

EOD;

        return $barnyard2_conf_text;

}

function create_snort_conf() {
	global $config, $g, $id, $if_real;
	/* write out snort.conf */
	$snort_conf_text = generate_snort_conf();
	conf_mount_rw();
	$conf = fopen("/usr/local/etc/snort/snort_$id$if_real/snort.conf", "w");
	if(!$conf) {
		log_error("Could not open /usr/local/etc/snort/$id$if_real/snort.conf for writing.");
		exit;
	}
	fwrite($conf, $snort_conf_text);
	fclose($conf);
	conf_mount_ro();
}

function snort_deinstall()
{

	global $config, $g, $id, $if_real;
	conf_mount_rw();

	/* remove custom sysctl */
	remove_text_from_file("/etc/sysctl.conf", "sysctl net.bpf.bufsize=20480");
	/* decrease bpf buffers back to 4096, from 20480 */
	exec("/sbin/sysctl net.bpf.bufsize=4096");
	exec("/usr/usr/bin/killall snort");
	sleep(2);
	exec("/usr/usr/bin/killall -9 snort");
	sleep(2);
	exec("/usr/usr/bin/killall barnyard2");
	sleep(2);
	exec("/usr/usr/bin/killall -9 barnyard2");
	sleep(2);
	exec("/usr/sbin/pw userdel snort");
	exec("/usr/sbin/pw groupdel snort");
	exec("rm -rf /usr/local/etc/snort*");
	exec("cd /var/db/pkg && pkg_delete `ls | grep barnyard2`");
	exec("cd /var/db/pkg && pkg_delete `ls | grep snort`");

	/* Remove snort cron entries Ugly code needs smoothness*/
	
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
		
		
	function snort_rules_up_deinstall_cron($should_install) {
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
	unset($config['installedpackages']['snortglobal']);
    write_config();
	conf_mount_rw();
	
	exec("rm -r /usr/local/www/snort");
	exec("rm -r /usr/local/pkg/snort");
	exec("rm -r /usr/local/lib/snort/");
	
	conf_mount_ro();
	
}

function generate_snort_conf()
{

	global $config, $g, $if_real, $id;
	conf_mount_rw();

if ($id != '' && $if_real != '')
{

	/* obtain external interface */
	/* XXX: make multi wan friendly */
	$snort_ext_int = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];

//	$snort_config_pass_thru = $config['installedpackages']['snortglobal']['rule'][$id]['configpassthru'];

	/* create basic files */
	if(!file_exists("/usr/local/etc/snort/snort/snort_$id$if_real"))
	{
		exec("/bin/mkdir -p /usr/local/etc/snort/snort_$id$if_real/");
		exec("/bin/mkdir -p /usr/local/etc/snort/snort_$id$if_real/rules");

		if(!file_exists("/usr/local/etc/snort/snort_$id$if_real/gen-msg.map"))
		{
			exec("/bin/cp /usr/local/etc/snort/classification.config /usr/local/etc/snort/snort_$id$if_real/classification.config");
			exec("/bin/cp /usr/local/etc/snort/gen-msg.map /usr/local/etc/snort/snort_$id$if_real/gen-msg.map");
			exec("/bin/cp /usr/local/etc/snort/reference.config /usr/local/etc/snort/snort_$id$if_real/reference.config");
			exec("/bin/cp /usr/local/etc/snort/sid-msg.map /usr/local/etc/snort/snort_$id$if_real/sid-msg.map");
			exec("/bin/cp /usr/local/etc/snort/unicode.map /usr/local/etc/snort/snort_$id$if_real/unicode.map");
			exec("/bin/cp /usr/local/etc/snort/threshold.conf /usr/local/etc/snort/snort_$id$if_real/threshold.conf");
			exec("/bin/cp /usr/local/etc/snort/snort.conf /usr/local/etc/snort/snort_$id$if_real/snort.conf");
			exec("/bin//usr/bin/touch /usr/local/etc/snort/snort_$id$if_real/barnyard2.conf");
			exec("/bin/mkdir -p /usr/local/etc/snort/snort_$id$if_real/rules");
		}
	}
}
	
/* define snortalertlogtype */
$snortalertlogtype = $config['installedpackages']['snortglobal']['snortalertlogtype'];
if ($snortalertlogtype == fast)
    $snortalertlogtype_type = "output alert_fast: alert";
else
	$snortalertlogtype_type = "output alert_full: alert"; 

/* define alertsystemlog */
$alertsystemlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['alertsystemlog'];
if ($alertsystemlog_info_chk == on)
    $alertsystemlog_type = "output alert_syslog: log_alert";

/* define tcpdumplog */
$tcpdumplog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['tcpdumplog'];
if ($tcpdumplog_info_chk == on)
    $tcpdumplog_type = "output log_tcpdump: snorttcpd.log";

/* define snortunifiedlog */
$snortunifiedlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['snortunifiedlog'];
if ($snortunifiedlog_info_chk == on)
    $snortunifiedlog_type = "output unified2: filename snort.u2_$id$if_real, limit 128";

/* define spoink (DISABLED)*/
$spoink_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['blockoffenders7'];
if ($spoink_info_chk == on)
    $spoink_type = "output alert_pf: /var/db/whitelist,snort2c";

	/* define servers and ports snortdefservers */

/* def DNS_SERVSERS */
$def_dns_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_dns_servers'];
if ($def_dns_servers_info_chk == "")
	$def_dns_servers_type = "\$HOME_NET";
else 
	$def_dns_servers_type = "$def_dns_servers_info_chk";

/* def DNS_PORTS */
$def_dns_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_dns_ports'];
if ($def_dns_ports_info_chk == "")
	$def_dns_ports_type = "53";
else 
	$def_dns_ports_type = "$def_dns_ports_info_chk";

/* def SMTP_SERVSERS */
$def_smtp_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_smtp_servers'];
if ($def_smtp_servers_info_chk == "")
	$def_smtp_servers_type = "\$HOME_NET";
else 
	$def_smtp_servers_type = "$def_smtp_servers_info_chk";

/* def SMTP_PORTS */
$def_smtp_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_smtp_ports'];
if ($def_smtp_ports_info_chk == "")
	$def_smtp_ports_type = "25";
else 
	$def_smtp_ports_type = "$def_smtp_ports_info_chk";

/* def MAIL_PORTS */
$def_mail_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_mail_ports'];
if ($def_mail_ports_info_chk == "")
	$def_mail_ports_type = "25,143,465,691";
else 
	$def_mail_ports_type = "$def_mail_ports_info_chk";
	
/* def HTTP_SERVSERS */
$def_http_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_http_servers'];
if ($def_http_servers_info_chk == "")
	$def_http_servers_type = "\$HOME_NET";
else 
	$def_http_servers_type = "$def_http_servers_info_chk";

/* def WWW_SERVSERS */
$def_www_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_www_servers'];
if ($def_www_servers_info_chk == "")
	$def_www_servers_type = "\$HOME_NET";
else 
	$def_www_servers_type = "$def_www_servers_info_chk";

/* def HTTP_PORTS */
$def_http_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_http_ports'];
if ($def_http_ports_info_chk == "")
	$def_http_ports_type = "80";
else 
	$def_http_ports_type = "$def_http_ports_info_chk";
	
/* def SQL_SERVSERS */
$def_sql_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_sql_servers'];
if ($def_sql_servers_info_chk == "")
	$def_sql_servers_type = "\$HOME_NET";
else 
	$def_sql_servers_type = "$def_sql_servers_info_chk";

/* def ORACLE_PORTS */
$def_oracle_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_oracle_ports'];
if ($def_oracle_ports_info_chk == "")
	$def_oracle_ports_type = "1521";
else 
	$def_oracle_ports_type = "$def_oracle_ports_info_chk";

/* def MSSQL_PORTS */
$def_mssql_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_mssql_ports'];
if ($def_mssql_ports_info_chk == "")
	$def_mssql_ports_type = "1433";
else 
	$def_mssql_ports_type = "$def_mssql_ports_info_chk";

/* def TELNET_SERVSERS */
$def_telnet_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_telnet_servers'];
if ($def_telnet_servers_info_chk == "")
	$def_telnet_servers_type = "\$HOME_NET";
else 
	$def_telnet_servers_type = "$def_telnet_servers_info_chk";
	
/* def TELNET_PORTS */
$def_telnet_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_telnet_ports'];
if ($def_telnet_ports_info_chk == "")
	$def_telnet_ports_type = "23";
else 
	$def_telnet_ports_type = "$def_telnet_ports_info_chk";
	
/* def SNMP_SERVSERS */
$def_snmp_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_snmp_servers'];
if ($def_snmp_servers_info_chk == "")
	$def_snmp_servers_type = "\$HOME_NET";
else 
	$def_snmp_servers_type = "$def_snmp_servers_info_chk";
	
/* def SNMP_PORTS */
$def_snmp_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_snmp_ports'];
if ($def_snmp_ports_info_chk == "")
	$def_snmp_ports_type = "161";
else 
	$def_snmp_ports_type = "$def_snmp_ports_info_chk";
	
/* def FTP_SERVSERS */
$def_ftp_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_ftp_servers'];
if ($def_ftp_servers_info_chk == "")
	$def_ftp_servers_type = "\$HOME_NET";
else 
	$def_ftp_servers_type = "$def_ftp_servers_info_chk";
	
/* def FTP_PORTS */
$def_ftp_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_ftp_ports'];
if ($def_ftp_ports_info_chk == "")
	$def_ftp_ports_type = "21";
else 
	$def_ftp_ports_type = "$def_ftp_ports_info_chk";
	
/* def SSH_SERVSERS */
$def_ssh_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_ssh_servers'];
if ($def_ssh_servers_info_chk == "")
	$def_ssh_servers_type = "\$HOME_NET";
else 
	$def_ssh_servers_type = "$def_ssh_servers_info_chk";
	
/* if user has defined a custom ssh port, use it */
if($config['system']['ssh']['port'])
	$ssh_port = $config['system']['ssh']['port'];
else
	$ssh_port = "22";
	
/* def SSH_PORTS */
$def_ssh_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_ssh_ports'];
if ($def_ssh_ports_info_chk == "")
	$def_ssh_ports_type = "{$ssh_port}";
else 
	$def_ssh_ports_type = "$def_ssh_ports_info_chk";
	
/* def POP_SERVSERS */
$def_pop_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_pop_servers'];
if ($def_pop_servers_info_chk == "")
	$def_pop_servers_type = "\$HOME_NET";
else 
	$def_pop_servers_type = "$def_pop_servers_info_chk";
	
/* def POP2_PORTS */
$def_pop2_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_pop2_ports'];
if ($def_pop2_ports_info_chk == "")
	$def_pop2_ports_type = "109";
else 
	$def_pop2_ports_type = "$def_pop2_ports_info_chk";
	
/* def POP3_PORTS */
$def_pop3_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_pop3_ports'];
if ($def_pop3_ports_info_chk == "")
	$def_pop3_ports_type = "110";
else 
	$def_pop3_ports_type = "$def_pop3_ports_info_chk";
	
/* def IMAP_SERVSERS */
$def_imap_servers_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_imap_servers'];
if ($def_imap_servers_info_chk == "")
	$def_imap_servers_type = "\$HOME_NET";
else 
	$def_imap_servers_type = "$def_imap_servers_info_chk";
	
/* def IMAP_PORTS */
$def_imap_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_imap_ports'];
if ($def_imap_ports_info_chk == "")
	$def_imap_ports_type = "143";
else 
	$def_imap_ports_type = "$def_imap_ports_info_chk";
	
/* def SIP_PROXY_IP */
$def_sip_proxy_ip_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_sip_proxy_ip'];
if ($def_sip_proxy_ip_info_chk == "")
	$def_sip_proxy_ip_type = "\$HOME_NET";
else 
	$def_sip_proxy_ip_type = "$def_sip_proxy_ip_info_chk";
	
/* def SIP_PROXY_PORTS */
$def_sip_proxy_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_sip_proxy_ports'];
if ($def_sip_proxy_ports_info_chk == "")
	$def_sip_proxy_ports_type = "5060:5090,16384:32768";
else 
	$def_sip_proxy_ports_type = "$def_sip_proxy_ports_info_chk";
	
/* def AUTH_PORTS */
$def_auth_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_auth_ports'];
if ($def_auth_ports_info_chk == "")
	$def_auth_ports_type = "113";
else 
	$def_auth_ports_type = "$def_auth_ports_info_chk";
	
/* def FINGER_PORTS */
$def_finger_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_finger_ports'];
if ($def_finger_ports_info_chk == "")
	$def_finger_ports_type = "79";
else 
	$def_finger_ports_type = "$def_finger_ports_info_chk";
	
/* def IRC_PORTS */
$def_irc_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_irc_ports'];
if ($def_irc_ports_info_chk == "")
	$def_irc_ports_type = "6665,6666,6667,6668,6669,7000";
else 
	$def_irc_ports_type = "$def_irc_ports_info_chk";
	
/* def NNTP_PORTS */
$def_nntp_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_nntp_ports'];
if ($def_nntp_ports_info_chk == "")
	$def_nntp_ports_type = "119";
else 
	$def_nntp_ports_type = "$def_nntp_ports_info_chk";
	
/* def RLOGIN_PORTS */
$def_rlogin_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_rlogin_ports'];
if ($def_rlogin_ports_info_chk == "")
	$def_rlogin_ports_type = "513";
else 
	$def_rlogin_ports_type = "$def_rlogin_ports_info_chk";
	
/* def RSH_PORTS */
$def_rsh_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_rsh_ports'];
if ($def_rsh_ports_info_chk == "")
	$def_rsh_ports_type = "514";
else 
	$def_rsh_ports_type = "$def_rsh_ports_info_chk";
	
/* def SSL_PORTS */
$def_ssl_ports_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_ssl_ports'];
if ($def_ssl_ports_info_chk == "")
	$def_ssl_ports_type = "443,465,563,636,989,990,992,993,994,995";
else 
	$def_ssl_ports_type = "$def_ssl_ports_info_chk";
	
	/* should we install a automatic update crontab entry? */
	$automaticrulesupdate = $config['installedpackages']['snortglobal']['automaticrulesupdate7'];

	/* if user is on pppoe, we really want to use ng0 interface */
	if($config['interfaces'][$snort_ext_int]['ipaddr'] == "pppoe")
		$snort_ext_int = "ng0";

	/* set the snort performance model */
	if($config['installedpackages']['snortglobal']['rule'][$id]['performance'])
		$snort_performance = $config['installedpackages']['snortglobal']['rule'][$id]['performance'];
	else
		$snort_performance = "lowmem";

	/* open snort's whitelist for writing */
	$whitelist = fopen("/var/db/whitelist", "w");
	if(!$whitelist) {
		log_error("Could not open /var/db/whitelist for writing.");
		return;
	}

	/* build an interface array list */
	$int_array = array('lan');
	for ($j = 1; isset ($config['interfaces']['opt' . $j]); $j++)
		if(isset($config['interfaces']['opt' . $j]['enable']))
			if(!$config['interfaces']['opt' . $j]['gateway'])
				$int_array[] = "opt{$j}";

	/*  iterate through interface list and write out whitelist items
	 *  and also compile a home_net list for snort.
	 */
	foreach($int_array as $int) {
		/* calculate interface subnet information */
		$ifcfg = &$config['interfaces'][$int];
		$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
		$subnetmask = gen_subnet_mask($ifcfg['subnet']);
		if($subnet == "pppoe" or $subnet == "dhcp") {
			$subnet = find_interface_ip("ng0");
			if($subnet)
				$home_net .= "{$subnet} ";
		} else {
			if ($subnet)
				if($ifcfg['subnet']) 
					$home_net .= "{$subnet}/{$ifcfg['subnet']} ";
		}
	}

	/* add all WAN ips to the whitelist */
	$wan_if = get_real_wan_interface();
	$ip = find_interface_ip($wan_if);
	if($ip) 
		$home_net .= "{$ip} ";
	
	/* Add Gateway on WAN interface to whitelist (For RRD graphs) */
	$int = convert_friendly_interface_to_real_interface_name("WAN");
	$gw = get_interface_gateway($int);
	if($gw) 
		$home_net .= "{$gw} ";
	
	/* Add DNS server for WAN interface to whitelist */
	$dns_servers = get_dns_servers();
	foreach($dns_servers as $dns) {
		if($dns) 
			$home_net .= "{$dns} ";		
	}
		
	/* Add loopback to whitelist (ftphelper) */
	$home_net .= "127.0.0.1 "; 	
	
	/* iterate all vips and add to whitelist */
	
	if($config['virtualip'])
		foreach($config['virtualip']['vip'] as $vip)
			if($vip['subnet']) 
				$home_net .= $vip['subnet'] . " ";

	if($config['installedpackages']['snortglobal']['whitelist'])
		foreach($config['installedpackages']['snortglobal']['whitelist'] as $snort)
			if($snort['ip']) 
				$home_net .= $snort['ip'] . " ";

	/* write out whitelist, convert spaces to carriage returns */
	$whitelist_home_net = str_replace("  ", " ", $home_net);
	$whitelist_home_net = str_replace(" ", "\n", $home_net);

	/* make $home_net presentable to snort */
	$home_net = trim($home_net);
	$home_net = str_replace(" ", ",", $home_net);
	$home_net = "[{$home_net}]";

	/* foreach through whitelist, writing out to file */
	$whitelist_split = split("\n", $whitelist_home_net);
	foreach($whitelist_split as $wl)
		if(trim($wl))
			fwrite($whitelist, trim($wl) . "\n");

	/* should we whitelist vpns? */
	$whitelistvpns = $config['installedpackages']['snortglobal']['whitelistvpns'];

	/* grab a list of vpns and whitelist if user desires added by nestorfish 954 */
	if($whitelistvpns) {
		if ($pfsense_stable == 'yes') // chk what pfsense version were on
		{
			$vpns_list = get_vpns_list();
		}
		if ($pfsense_stable == 'no') // chk what pfsense version were on
		{
			$vpns_list = filter_get_vpns_list();
		}

		$whitelist_vpns = split(" ", $vpns_list);
		foreach($whitelist_vpns as $wl)
			if(trim($wl))
				fwrite($whitelist, trim($wl) . "\n");
	}

	/* close file */
	fclose($whitelist);
	
	/* generate rule sections to load */
	$enabled_rulesets = $config['installedpackages']['snortglobal']['rule'][$id]['rulesets'];
	if($enabled_rulesets) {
		$selected_rules_sections = "";
		$enabled_rulesets_array = split("\|\|", $enabled_rulesets);
		foreach($enabled_rulesets_array as $enabled_item)
			$selected_rules_sections  .= "include \$RULE_PATH/{$enabled_item}\n";
	}

	conf_mount_ro();

/////////////////////////////
	
/* preprocessor code */
	
/* def perform_stat */
$snort_perform_stat = <<<EOD
##########################
                         #
# NEW                    #
# Performance Statistics #
                         #
##########################

preprocessor perfmonitor: time 300 file /var/log/snort/snort_$id$if_real.stats pktcnt 10000
	
EOD;

$def_perform_stat_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['perform_stat'];
if ($def_perform_stat_info_chk == "on")
	$def_perform_stat_type = "$snort_perform_stat";
else 
	$def_perform_stat_type = "";
	
/* def http_inspect */
$snort_http_inspect = <<<EOD
#################
                #
# HTTP Inspect  #
                #
#################

preprocessor http_inspect: global iis_unicode_map unicode.map 1252

preprocessor http_inspect_server: server default \
                        ports  { 80 8080 }  \
                        non_strict \
                        non_rfc_char  { 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 }  \
                        flow_depth 0  \
                        apache_whitespace no \
                        directory no \
                        iis_backslash no \
                        u_encode yes \
                        ascii no \
                        chunk_length 500000 \
                        bare_byte yes \
                        double_decode yes \
                        iis_unicode no \
                        iis_delimiter no \
                        multi_slash no
	
EOD;

$def_http_inspect_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['http_inspect'];
if ($def_http_inspect_info_chk == "on")
	$def_http_inspect_type = "$snort_http_inspect";
else 
	$def_http_inspect_type = "";
	
/* def other_preprocs */
$snort_other_preprocs = <<<EOD
##################
                 #
# Other preprocs #
                 #
##################

preprocessor rpc_decode: 111 32770 32771 32772 32773 32774 32775 32776 32777 32778 32779
preprocessor bo
	
EOD;

$def_other_preprocs_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['other_preprocs'];
if ($def_other_preprocs_info_chk == "on")
	$def_other_preprocs_type = "$snort_other_preprocs";
else 
	$def_other_preprocs_type = "";
	
/* def ftp_preprocessor */
$snort_ftp_preprocessor = <<<EOD
#####################
                    #
# ftp preprocessor  #
                    #
#####################

preprocessor ftp_telnet: global \
inspection_type stateless

preprocessor ftp_telnet_protocol: telnet \
   normalize \
   ayt_attack_thresh 200

preprocessor ftp_telnet_protocol: \
    ftp server default \
    def_max_param_len 100 \
    ports { 21 } \
    ftp_cmds { USER PASS ACCT CWD SDUP SMNT QUIT REIN PORT PASV TYPE STRU MODE } \
    ftp_cmds { RETR STOR STOU APPE ALLO REST RNFR RNTO ABOR DELE RMD MKD PWD } \
    ftp_cmds { LIST NLST SITE SYST STAT HELP NOOP } \
    ftp_cmds { AUTH ADAT PROT PBSZ CONF ENC } \
    ftp_cmds { FEAT CEL CMD MACB } \
    ftp_cmds { MDTM REST SIZE MLST MLSD } \
    ftp_cmds { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    alt_max_param_len 0 { CDUP QUIT REIN PASV STOU ABOR PWD SYST NOOP } \
    alt_max_param_len 100 { MDTM CEL XCWD SITE USER PASS REST DELE RMD SYST TEST STAT MACB EPSV CLNT LPRT } \
    alt_max_param_len 200 { XMKD NLST ALLO STOU APPE RETR STOR CMD RNFR HELP } \
    alt_max_param_len 256 { RNTO CWD } \
    alt_max_param_len 400 { PORT } \
    alt_max_param_len 512 { SIZE } \
    chk_str_fmt { USER PASS ACCT CWD SDUP SMNT PORT TYPE STRU MODE } \
    chk_str_fmt { RETR STOR STOU APPE ALLO REST RNFR RNTO DELE RMD MKD } \
    chk_str_fmt { LIST NLST SITE SYST STAT HELP } \
    chk_str_fmt { AUTH ADAT PROT PBSZ CONF ENC } \
    chk_str_fmt { FEAT CEL CMD } \
    chk_str_fmt { MDTM REST SIZE MLST MLSD } \
    chk_str_fmt { XPWD XCWD XCUP XMKD XRMD TEST CLNT } \
    cmd_validity MODE < char ASBCZ > \
    cmd_validity STRU < char FRP > \
    cmd_validity ALLO < int [ char R int ] > \
    cmd_validity TYPE < { char AE [ char NTC ] | char I | char L [ number ] } > \
    cmd_validity MDTM < [ date nnnnnnnnnnnnnn[.n[n[n]]] ] string > \
    cmd_validity PORT < host_port >

preprocessor ftp_telnet_protocol: ftp client default \
   max_resp_len 256 \
   bounce yes \
   telnet_cmds yes
	
EOD;

$def_ftp_preprocessor_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['ftp_preprocessor'];
if ($def_ftp_preprocessor_info_chk == "on")
	$def_ftp_preprocessor_type = "$snort_ftp_preprocessor";
else 
	$def_ftp_preprocessor_type = "";

/* def smtp_preprocessor */
$snort_smtp_preprocessor = <<<EOD
#####################
                    #
# SMTP preprocessor #
                    #
#####################

preprocessor SMTP: \
    ports { 25 465 691 } \
    inspection_type stateful \
    normalize cmds \
    valid_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN PIPELINING \
CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    normalize_cmds { MAIL RCPT HELP HELO ETRN EHLO EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET SEND SAML SOML AUTH TURN ETRN \
PIPELINING CHUNKING DATA DSN RSET QUIT ONEX QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    max_header_line_len 1000 \ 
    max_response_line_len 512 \
    alt_max_command_line_len 260 { MAIL } \
    alt_max_command_line_len 300 { RCPT } \
    alt_max_command_line_len 500 { HELP HELO ETRN EHLO } \
    alt_max_command_line_len 255 { EXPN VRFY ATRN SIZE BDAT DEBUG EMAL ESAM ESND ESOM EVFY IDENT NOOP RSET } \
    alt_max_command_line_len 246 { SEND SAML SOML AUTH TURN ETRN PIPELINING CHUNKING DATA DSN RSET QUIT ONEX } \
    alt_max_command_line_len 246 { QUEU STARTTLS TICK TIME TURNME VERB X-EXPS X-LINK2STATE XADR } \
    alt_max_command_line_len 246 { XAUTH XCIR XEXCH50 XGEN XLICENSE XQUEU XSTA XTRN XUSR } \
    xlink2state { enable }
	
EOD;

$def_smtp_preprocessor_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['smtp_preprocessor'];
if ($def_smtp_preprocessor_info_chk == "on")
	$def_smtp_preprocessor_type = "$snort_smtp_preprocessor";
else 
	$def_smtp_preprocessor_type = "";
	
/* def sf_portscan */
$snort_sf_portscan = <<<EOD
################
               #
# sf Portscan  #
               #
################

preprocessor sfportscan: scan_type { all } \
                         proto  { all } \
                         memcap { 10000000 } \
                         sense_level { medium } \
                         ignore_scanners { \$HOME_NET }
	
EOD;

$def_sf_portscan_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['sf_portscan'];
if ($def_sf_portscan_info_chk == "on")
	$def_sf_portscan_type = "$snort_sf_portscan";
else 
	$def_sf_portscan_type = "";
	
/* def dce_rpc_2 */
$snort_dce_rpc_2 = <<<EOD
###############
              #
# NEW         #
# DCE/RPC 2   #
              #
###############

preprocessor dcerpc2: memcap 102400, events [smb, co, cl]
preprocessor dcerpc2_server: default, policy WinXP, \
    detect [smb [139,445], tcp 135, udp 135, rpc-over-http-server 593], \
    autodetect [tcp 1025:, udp 1025:, rpc-over-http-server 1025:], \
    smb_max_chain 3
	
EOD;

$def_dce_rpc_2_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['dce_rpc_2'];
if ($def_dce_rpc_2_info_chk == "on")
	$def_dce_rpc_2_type = "$snort_dce_rpc_2";
else 
	$def_dce_rpc_2_type = "";
	
/* def dns_preprocessor */
$snort_dns_preprocessor = <<<EOD
####################
                   #
# DNS preprocessor #
                   #
####################

preprocessor dns: \
    ports { 53 } \
    enable_rdata_overflow
	
EOD;

$def_dns_preprocessor_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['dns_preprocessor'];
if ($def_dns_preprocessor_info_chk == "on")
	$def_dns_preprocessor_type = "$snort_dns_preprocessor";
else 
	$def_dns_preprocessor_type = "";
	
/* def SSL_PORTS IGNORE */
$def_ssl_ports_ignore_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['def_ssl_ports_ignore'];
if ($def_ssl_ports_ignore_info_chk == "")
	$def_ssl_ports_ignore_type = "443 465 563 636 989 990 992 993 994 995";
else 
	$def_ssl_ports_ignore_type = "$def_ssl_ports_ignore_info_chk";
	
//////////////////////////////////////////////////////////////////
	/* build snort configuration file */
        /* TODO; feed back from pfsense users to reduce false positives */
	$snort_conf_text = <<<EOD

# snort configuration file
# generated by the pfSense
# package manager system
# see /usr/local/pkg/snort.inc
# for more information
#	snort.conf
#   Snort can be found at http://www.snort.org/
#
#	Copyright (C) 2009 Robert Zelaya
#	part of pfSense
#	All rights reserved.
#
#	Redistribution and use in source and binary forms, with or without
#	modification, are permitted provided that the following conditions are met:
#
#	1. Redistributions of source code must retain the above copyright notice,
#	   this list of conditions and the following disclaimer.
#
#	2. Redistributions in binary form must reproduce the above copyright
#	   notice, this list of conditions and the following disclaimer in the
#	   documentation and/or other materials provided with the distribution.
#
#	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
#	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
#	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
#	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#	POSSIBILITY OF SUCH DAMAGE.

#########################
                        #
# Define Local Network  #
                        #
#########################

var HOME_NET {$home_net}
var EXTERNAL_NET !\$HOME_NET

###################
                  #
# Define Servers  #
                  #
###################

var DNS_SERVERS [{$def_dns_servers_type}]
var SMTP_SERVERS [{$def_smtp_servers_type}]
var HTTP_SERVERS [{$def_http_servers_type}]
var SQL_SERVERS [{$def_sql_servers_type}]
var TELNET_SERVERS [{$def_telnet_servers_type}]
var SNMP_SERVERS [{$def_snmp_servers_type}]
var FTP_SERVERS [{$def_ftp_servers_type}]
var SSH_SERVERS [{$def_ssh_servers_type}]
var POP_SERVERS [{$def_pop_servers_type}]
var IMAP_SERVERS [{$def_imap_servers_type}]
var RPC_SERVERS \$HOME_NET
var WWW_SERVERS [{$def_www_servers_type}]
var SIP_PROXY_IP [{$def_sip_proxy_ip_type}]
var AIM_SERVERS \
[64.12.24.0/23,64.12.28.0/23,64.12.161.0/24,64.12.163.0/24,64.12.200.0/24,205.188.3.0/24,205.188.5.0/24,205.188.7.0/24,205.188.9.0/24,205.188.153.0/24,205.188.179.0/24,205.188.248.0/24]

########################
                       #
# Define Server Ports  #
                       #
########################

portvar HTTP_PORTS [{$def_http_ports_type}]
portvar SHELLCODE_PORTS !80
portvar ORACLE_PORTS [{$def_oracle_ports_type}]
portvar AUTH_PORTS [{$def_auth_ports_type}]
portvar DNS_PORTS [{$def_dns_ports_type}]
portvar FINGER_PORTS [{$def_finger_ports_type}]
portvar FTP_PORTS [{$def_ftp_ports_type}]
portvar IMAP_PORTS [{$def_imap_ports_type}]
portvar IRC_PORTS [{$def_irc_ports_type}]
portvar MSSQL_PORTS [{$def_mssql_ports_type}]
portvar NNTP_PORTS [{$def_nntp_ports_type}]
portvar POP2_PORTS [{$def_pop2_ports_type}]
portvar POP3_PORTS [{$def_pop3_ports_type}]
portvar SUNRPC_PORTS [111,32770,32771,32772,32773,32774,32775,32776,32777,32778,32779]
portvar RLOGIN_PORTS [{$def_rlogin_ports_type}]
portvar RSH_PORTS [{$def_rsh_ports_type}]
portvar SMB_PORTS [139,445]
portvar SMTP_PORTS [{$def_smtp_ports_type}]
portvar SNMP_PORTS [{$def_snmp_ports_type}]
portvar SSH_PORTS [{$def_ssh_ports_type}]
portvar TELNET_PORTS [{$def_telnet_ports_type}]
portvar MAIL_PORTS [{$def_mail_ports_type}]
portvar SSL_PORTS [{$def_ssl_ports_type}]
portvar SIP_PROXY_PORTS [{$def_sip_proxy_ports_type}]

# DCERPC NCACN-IP-TCP
portvar DCERPC_NCACN_IP_TCP [139,445]
portvar DCERPC_NCADG_IP_UDP [138,1024:]
portvar DCERPC_NCACN_IP_LONG [135,139,445,593,1024:]
portvar DCERPC_NCACN_UDP_LONG [135,1024:]
portvar DCERPC_NCACN_UDP_SHORT [135,593,1024:]
portvar DCERPC_NCACN_TCP [2103,2105,2107]
portvar DCERPC_BRIGHTSTORE [6503,6504]

#####################
                    #
# Define Rule Paths #
                    #
#####################

var RULE_PATH /usr/local/etc/snort/snort_$id$if_real/rules
# var PREPROC_RULE_PATH ./preproc_rules

################################
                               #
# Configure the snort decoder  #
                               #
################################

config checksum_mode: all
config disable_decode_alerts
config disable_tcpopt_experimental_alerts
config disable_tcpopt_obsolete_alerts
config disable_ttcp_alerts
config disable_tcpopt_alerts
config disable_ipopt_alerts
config disable_decode_drops

###################################
                                  #
# Configure the detection engine  #
# Use lower memory models         #
                                  #
###################################

config detection: search-method {$snort_performance} max_queue_events 5
config event_queue: max_queue 8 log 3 order_events content_length

#Configure dynamic loaded libraries
dynamicpreprocessor directory /usr/local/lib/snort/dynamicpreprocessor/
dynamicengine /usr/local/lib/snort/dynamicengine/libsf_engine.so
dynamicdetection directory /usr/local/lib/snort/dynamicrules/

###################
                  #
# Flow and stream #
                  #
###################

preprocessor frag3_global: max_frags 8192
preprocessor frag3_engine: policy bsd detect_anomalies

preprocessor stream5_global: max_tcp 8192, track_tcp yes, \
track_udp yes, track_icmp yes
preprocessor stream5_tcp: policy BSD, ports both all, use_static_footprint_sizes
preprocessor stream5_udp:
preprocessor stream5_icmp:

{$def_perform_stat_type}

{$def_http_inspect_type}

{$def_other_preprocs_type}

{$def_ftp_preprocessor_type}

{$def_smtp_preprocessor_type}

{$def_sf_portscan_type}

############################
                           #
# OLD                      #
# preprocessor dcerpc: \   #
#    autodetect \          #
#    max_frag_size 3000 \  #
#    memcap 100000         #
                           #
############################

{$def_dce_rpc_2_type}

{$def_dns_preprocessor_type}

##############################
                             #
# NEW                        #
# Ignore SSL and Encryption  #
                             #
##############################

preprocessor ssl: ports { {$def_ssl_ports_ignore_type} }, trustservers, noinspect_encrypted

#####################
                    #
# Snort Output Logs #
                    #
#####################

$snortalertlogtype_type
$alertsystemlog_type
$tcpdumplog_type
$snortmysqllog_info_chk
$snortunifiedlog_type
$spoink_type
						
#################
                #
# Misc Includes #
                #
#################

include /usr/local/etc/snort/snort_$id$if_real/reference.config
include /usr/local/etc/snort/snort_$id$if_real/classification.config
include /usr/local/etc/snort/snort_$id$if_real/threshold.conf

# Snort user pass through configuration
{$snort_config_pass_thru}

###################
                  #
# Rules Selection #
                  #
###################

{$selected_rules_sections}

EOD;

	return $snort_conf_text;
}

/*   check downloaded text from snort.org to make sure that an error did not occur
 *   for example, if you are not a premium subscriber you can only download rules
 *   so often, etc. TO BE: Removed unneeded.
 */
function check_for_common_errors($filename) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	$contents = file_get_contents($filename);
	if(stristr($contents, "You don't have permission")) {
		if(!$console_mode) {
			update_all_status("An error occured while downloading {$filename}.");
			hide_progress_bar_status();
		} else {
			log_error("An error occured.  Scroll down to inspect it's contents.");
			echo "An error occured.  Scroll down to inspect it's contents.";
		}
		if(!$console_mode) {
			update_output_window(strip_tags("$contents"));
		} else {
			$contents = strip_tags($contents);
			log_error("Error downloading snort rules: {$contents}");
			echo "Error downloading snort rules: {$contents}";
		}
		scroll_down_to_bottom_of_page();
		exit;
	}
}

/* force browser to scroll all the way down */
function scroll_down_to_bottom_of_page() {
	global $snort_filename, $console_mode;
	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">parent.scrollTo(0,1500);\n</script>";
}

/* ensure downloaded file looks sane */
function verify_downloaded_file($filename) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(filesize($filename)<9500) {
		if(!$console_mode) {
			update_all_status("Checking {$filename}...");
			check_for_common_errors($filename);
		}
	}
	update_all_status("Verifying {$filename}...");
	if(!file_exists($filename)) {
		if(!$console_mode) {
			update_all_status("Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.");
			hide_progress_bar_status();
		} else {
			log_error("Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.");
			echo "Could not fetch snort rules ({$filename}).  Check oinkid key and dns and try again.";
		}
		exit;
	}
	update_all_status("Verified {$filename}.");
}

/* extract rules */
function extract_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		$static_output = gettext("Extracting snort rules...");
		update_all_status($static_output);
	}
	if(!is_dir("/usr/local/etc/snort/rules/"))
		mkdir("/usr/local/etc/snort/rules/");
	$cmd = "/usr/bin/tar xzf {$tmpfname}/{$snort_filename} -C /usr/local/etc/snort/ rules/";
	$handle = popen("{$cmd} 2>&1", 'r');
	while(!feof($handle)) {
		$buffer = fgets($handle);
		update_output_window($buffer);
	}
	pclose($handle);	

	if(!$console_mode) {
		$static_output = gettext("Snort rules extracted.");
		update_all_status($static_output);
	} else {
		log_error("Snort rules extracted.");
		echo "Snort rules extracted.";
	}
}

/* verify MD5 against downloaded item */
function verify_snort_rules_md5($tmpfname) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		$static_output = gettext("Verifying md5 signature...");
		update_all_status($static_output);
	}

        $md555 = file_get_contents("{$tmpfname}/{$snort_filename_md5}");
        $md5 = `echo "{$md555}" | /usr/bin/awk '{ print $4 }'`;
	$file_md5_ondisk = `/sbin/md5 {$tmpfname}/{$snort_filename} | /usr/bin/awk '{ print $4 }'`;
	if($md5 == $file_md5_ondisk) {
		if(!$console_mode) {
			$static_output = gettext("snort rules: md5 signature of rules mismatch.");
			update_all_status($static_output);
			hide_progress_bar_status();
		} else {
			log_error("snort rules: md5 signature of rules mismatch.");
			echo "snort rules: md5 signature of rules mismatch.";
		}
		exit;
	}
}

/* hide progress bar */
function hide_progress_bar_status() {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode)
		echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='hidden';\n</script>";
}

/* unhide progress bar */
function unhide_progress_bar_status() {
        global $snort_filename, $snort_filename_md5, $console_mode;
        ob_flush();
        if(!$console_mode)
                echo "\n<script type=\"text/javascript\">document.progressbar.style.visibility='visible';\n</script>";
}

/* update both top and bottom text box during an operation */
function update_all_status($status) {
	global $snort_filename, $snort_filename_md5, $console_mode;
	ob_flush();
	if(!$console_mode) {
		update_status($status);
		update_output_window($status);
	}
}

/* obtain alert description for an ip address */
function get_snort_alert($ip) {
	global $snort_alert_file_split, $snort_config;
	if(!file_exists("/var/log/snort/alert"))
		return;
	if(!$snort_config)
		$snort_config = read_snort_config_cache();
	if($snort_config[$ip])
		return $snort_config[$ip];
	if(!$snort_alert_file_split)
		$snort_alert_file_split = split("\n", file_get_contents("/var/log/snort/alert"));
	foreach($snort_alert_file_split as $fileline) {
		if (preg_match("/\[\*\*\] (\[.*\]) (.*) (\[\*\*\])/", $fileline, $matches))
			$alert_title = $matches[2];
		if (preg_match("/(\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b)/", $fileline, $matches))
			$alert_ip = $matches[$id];
		if($alert_ip == $ip) {
			if(!$snort_config[$ip])
				$snort_config[$ip] = $alert_title;
			return $alert_title;
		}
	}
	return "n/a";
}

function make_clickable($buffer) {
    global $config, $g;
    /* if clickable urls is disabled, simply return buffer back to caller */
    $clickablalerteurls = $config['installedpackages']['snort']['config'][$id]['oinkmastercode'];
    if(!$clickablalerteurls)
    	return $buffer;
    $buffer = eregi_replace("(^|[ \n\r\t])((http(s?)://)(www\.)?([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"\\2\" target=\"_blank\">\\2</a>", $buffer);
    $buffer = eregi_replace("(^|[ \n\r\t])((ftp://)(www\.)?([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"\\2\" target=\"_blank\">\\2</a>", $buffer);
    $buffer = eregi_replace("([a-z_-][a-z0-9\._-]*@[a-z0-9_-]+(\.[a-z0-9_-]+)+)","<a href=\"mailto:\\1\">\\1</a>", $buffer);
    $buffer = eregi_replace("(^|[ \n\r\t])(www\.([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"http://\\2\" target=\"_blank\">\\2</a>", $buffer);
    $buffer = eregi_replace("(^|[ \n\r\t])(ftp\.([a-z0-9_-]+(\.[a-z0-9_-]+)+)(/[^/ \n\r]*)*)","\\1<a href=\"ftp://\\2\" target=\"_blank\">\\2</a>", $buffer);

    return $buffer;
}

function read_snort_config_cache() {
	global $g, $config, $snort_config;
	if($snort_config)
		return $snort_config;
	if(file_exists($g['tmp_path'] . '/snort_config.cache')) {
		$snort_config = unserialize(file_get_contents($g['tmp_path'] . '/snort_config.cache'));
		return $snort_config;
	}
	return;
}

function write_snort_config_cache($snort_config) {
	global $g, $config;
	conf_mount_rw();
	$configcache = fopen($g['tmp_path'] . '/snort_config.cache', "w");
	if(!$configcache) {
		log_error("Could not open {$g['tmp_path']}/snort_config.cache  for writing.");
		return false;
	}
	fwrite($configcache, serialize($snort_config));
	fclose($configcache);
	conf_mount_ro();
	return true;
}

function snort_advanced() {
	global $g, $config;
	sync_package_snort();
}

function snort_define_servers() {
	global $g, $config;
	sync_package_snort();
}

?>
