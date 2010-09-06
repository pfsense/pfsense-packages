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
require_once("config.inc");
require_once("functions.inc");

// Needed on 2.0 because of filter_get_vpns_list()
require_once("filter.inc");

/* package version */
$snort_package_version = 'Snort 2.8.6.1 pkg v. 1.34';

/* find out if were in 1.2.3-RELEASE */
$pfsense_ver_chk = exec('/bin/cat /etc/version');
if ($pfsense_ver_chk == '1.2.3-RELEASE')
{
	$pfsense_stable = 'yes';
}else{
	$pfsense_stable = 'no';
}

/* find out what arch where in x86 , x64 */
/* TODO: should be more clear in this code */
$snort_arch_ck = '';
exec('/usr/bin/uname -m', $snort_arch_ck);
if($snort_arch_ck[0] == 'i386') {
	$snort_arch = 'x86';
}else{
	$snort_arch = 'x64';
}

/* tell me my theme */
$pfsense_theme_is = $config['theme'];

/* func builds custom white lists */
function find_whitelist_key($find_wlist_number) {
        global $config, $g;

	$whitelist_array = $config['installedpackages']['snortglobal']['whitelist']['item'];
	$w_key = -1;

	foreach ($whitelist_array as $value) {
	$w_key += 1;
		if ($config['installedpackages']['snortglobal']['whitelist']['item'][$w_key]['uuid'] == $find_wlist_number) {
			return $w_key;
		}
	}
}

/* func builds custom suppress lists */
function find_suppress_key($find_slist_number) {
        global $config, $g;

	$suppresslist_array = $config['installedpackages']['snortglobal']['suppress']['item'];
	$s_key = -1;

	foreach ($suppresslist_array as $value2) {
	$s_key += 1;
		if ($config['installedpackages']['snortglobal']['suppress']['item'][$s_key]['uuid'] == $find_slist_number) {
			return $s_key;
		}
	}
}

/* func builds custom whitelests */
function build_base_whitelist($build_netlist, $wanip, $wangw, $wandns, $vips, $vpns, $userwips) {
global $config, $g;

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

	if($wanip == 'yes') {
        /* add all WAN ips to the whitelist */
        $wan_if = get_real_wan_interface();
        $ip = find_interface_ip($wan_if);
        if($ip) 
                $home_net .= "{$ip} ";
	}

	if($wangw == 'yes') {
        /* Add Gateway on WAN interface to whitelist (For RRD graphs) */
        $int = convert_friendly_interface_to_real_interface_name2("WAN");
        $gw = get_interface_gateway($int);
        if($gw) 
                $home_net .= "{$gw} ";
	}

	if($wandns == 'yes') {
        /* Add DNS server for WAN interface to whitelist */
        $dns_servers = get_dns_servers();
        foreach($dns_servers as $dns) {
                if($dns) 
                        $home_net .= "{$dns} ";         
        }
	}
	
	if($vips == 'yes') {		
        /* iterate all vips and add to whitelist */        
        if($config['virtualip'])
                foreach($config['virtualip']['vip'] as $vip)
                        if($vip['subnet']) 
                                $home_net .= $vip['subnet'] . " ";
	}

		/* Add loopback to whitelist (ftphelper) */
		if($userwips > -1 && $build_netlist == 'netlist') {
        	$home_net .= "127.0.0.1 ";
		}elseif ($userwips > -1 &&  $build_netlist == 'whitelist') {
			$home_net .= "127.0.0.1 ";
		}else{
			$home_net .= "127.0.0.1";
		}
	
        /* grab a list of vpns and whitelist if user desires added by nestorfish 954 */
        if($vpns == 'yes')
        {
                if ($pfsense_stable == 'yes') // chk what pfsense version were on
                {
                        $vpns_list = get_vpns_list();
                }

                if ($pfsense_stable == 'no') // chk what pfsense version were on
                {
                        $vpns_list = filter_get_vpns_list();
                }
			if ($vpns_list != '') {
				$home_net .= "$vpns_list ";
			}
		 }

		 /* never ever compair numbers to words */
        if($userwips > -1)
        {
			if (!is_array($config['installedpackages']['snortglobal']['whitelist']['item']))
			$config['installedpackages']['snortglobal']['whitelist']['item'] = array();

			$home_net .= $config['installedpackages']['snortglobal']['whitelist']['item'][$userwips]['address'];
		}

			/* this foe whitelistfile, convert spaces to carriage returns */
			$whitelist_home_net = str_replace("  ", "\n", $home_net);
			$whitelist_home_net = str_replace(" ", "\n", $home_net);

			/* this is for snort.conf */
			$home_net = trim($home_net);
			$home_net = str_replace(" ", ",", $home_net);
			$home_net = "[{$home_net}]";

		if($build_netlist == 'netlist') {
			return $home_net;
		}

		if($build_netlist == 'whitelist') {
			return $whitelist_home_net;
		}		
} 


/* checks to see if snort is running yes/no and stop/start */
	function Running_Ck($snort_uuid, $if_real, $id) {
		global $config;
	
	$snort_up_ck = exec("/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep snort | /usr/bin/awk '{print \$2;}' | sed 1q");

	if(snort_up_ck == ''){
		$snort_up = 'no';
		return $snort_up;
	}

	if(snort_up_ck != ''){

	//$snort_up_pre = exec("/usr/bin/top -a -U snort -u | grep -v grep | grep \"R {$snort_uuid}_{$if_real}\" | awk '{print \$1;}'");
	//$snort_up_s = exec("/usr/bin/top -U snort -u | grep snort | grep {$snort_up_pre} | /usr/bin/awk '{print \$1;}'");
	//$snort_up_r = exec("/usr/bin/top -U root -u | grep snort | grep {$snort_up_pre} | /usr/bin/awk '{print \$1;}'");

	/* use ob_clean to clear output buffer, this code needs to be watched */
	ob_clean();
	$snort_up_prell = exec("/usr/bin/top -a -U snort -u | grep -v grep | grep \"R {$snort_uuid}\" | awk '{print \$1;}'", $retval);

	if ($snort_up_prell != "") {
		$snort_uph = 'yes';
	}else{
		$snort_uph = 'no';
		}
	}

	return $snort_uph;
	}

/* checks to see if barnyard2 is running yes/no */
	function Running_Ck_b($snort_uuid, $if_real, $id) {
		global $config;
	
	$snort_up_ck_b = exec("/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep barnyard2 | /usr/bin/awk '{print \$2;}' | sed 1q");

	if($snort_up_ck_b == ''){
		$snort_up_b = 'no';
		return $snort_up_b;
	}

	if(snort_up_ck_b != ''){
	
	//$snort_up_pre_b = exec("/usr/bin/top -a -U snort -u | grep -v grep | grep \"f snort_{$snort_uuid}_{$if_real}.u2\" | awk '{print \$1;}'");
	//$snort_up_s_b = exec("/usr/bin/top -U snort -u | grep barnyard2 | grep {$snort_up_pre_b} | /usr/bin/awk '{print \$1;}'");
	//$snort_up_r_b = exec("/usr/bin/top -U root -u | grep barnyard2 | grep {$snort_up_pre_b} | /usr/bin/awk '{print \$1;}'");

	/* use ob_clean to clear output buffer, this code needs to be watched */
	ob_clean();
	$snort_up_pre_b = exec("/usr/bin/top -a -U snort -u | grep -v grep | grep \"f snort_{$snort_uuid}_{$if_real}.u2\" | awk '{print \$1;}'");

	if ($snort_up_pre_b != '') {
		$snort_up_b = 'yes';
	}else{
		$snort_up_b = 'no';
		}
	}

	return $snort_up_b;
	}

	function Running_Stop($snort_uuid, $if_real, $id) {
		global $config;
		
		/* if snort.sh crashed this will remove the pid */
		exec('/bin/rm /tmp/snort.sh.pid');
		
	$start_up_pre = exec("/usr/bin/top -a -U snort -u | grep -v grep | grep \"R {$snort_uuid}\" | awk '{print \$1;}'");
	$start_up_s = exec("/usr/bin/top -U snort -u | grep snort | grep {$start_up_pre} | awk '{ print $1; }'");
	$start_up_r = exec("/usr/bin/top -U root -u | grep snort | grep {$start_up_pre} | awk '{ print $1; }'");
	
	$start2_upb_pre = exec("/usr/bin/top -a -U snort -u | grep -v grep | grep \"snort_{$snort_uuid}_{$if_real}.u2\" | awk '{print \$1;}'");
	$start2_upb_s = exec("/usr/bin/top -U snort -u | grep barnyard2 | grep {$start2_upb_pre} | awk '{ print $1; }'");
	$start2_upb_r = exec("/usr/bin/top -U root -u | grep barnyard2 | grep {$start2_upb_pre} | awk '{ print $1; }'");

		if ($start_up_s != "" || $start_up_r != "" || $start2_upb_s != "" || $start2_upb_r != "")
		{
			if ($start_up_s != "")
			{
				exec("/bin/kill {$start_up_s}");				
				exec("rm /var/log/snort/run/snort_{$if_real}{$snort_uuid}*");
				exec("rm /var/log/snort/snort_{$snort_uuid}_{$if_real}*");
			}
		
			if ($start2_upb_s != "")
			{
				exec("/bin/kill {$start2_upb_s}");
				exec("/bin/rm /var/run/barnyard2_{$snort_uuid}_{$if_real}*");
			}
		
			if ($start_up_r != "")
			{
				exec("/bin/kill {$start_up_r}");
				exec("rm /var/log/snort/run/snort_{$if_real}{$snort_uuid}*");
				exec("rm /var/log/snort/snort_{$snort_uuid}_{$if_real}*");
			}
		
			if ($start2_upb_r != "")
			{
				exec("/bin/kill {$start2_upb_r}");
				exec("/bin/rm /var/run/barnyard2_{$snort_uuid}_{$if_real}*");
			}
		
			/* Log Iface stop */
			exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'Interface Rule STOP for {$snort_uuid}_{$if_real}...'");
		}
	}

	
	function Running_Start($snort_uuid, $if_real, $id) {
		global $config;
		
		/* if snort.sh crashed this will remove the pid */
		exec('/bin/rm /tmp/snort.sh.pid');

		$snort_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['enable'];
		if ($snort_info_chk == 'on') {
		exec("/usr/local/bin/snort -u snort -g snort -R \"{$snort_uuid}\" -D -q -l /var/log/snort --pid-path /var/log/snort/run -G {$snort_uuid} -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}");
		}
		/* define snortbarnyardlog_chk */
		/* top will have trouble if the uuid is to far back */
		$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
		$snortbarnyardlog_mysql_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_mysql'];
		if ($snortbarnyardlog_info_chk == 'on' && $snortbarnyardlog_mysql_info_chk != '' && $snort_info_chk == 'on') {
		exec("/usr/local/bin/barnyard2 -f \"snort_{$snort_uuid}_{$if_real}.u2\" -u snort -g snort --pid-path /var/log/snort/run -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort -D -q");
		}
		
		/* Log Iface stop */
		exec("/usr/bin/logger -p daemon.info -i -t SnortStartup 'Interface Rule START for {$id}_{$snort_uuid}_{$if_real}...'");
	}

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

//$if_real_wan = convert_friendly_interface_to_real_interface_name2($interface_fake);

/* Allow additional execution time 0 = no limit. */
ini_set('max_execution_time', '9999');
ini_set('max_input_time', '9999');

/* define oinkid */
if($config['installedpackages']['snortglobal'])
	$oinkid = $config['installedpackages']['snortglobal']['oinkmastercode'];

	
/* 
this code block is for deleteing logs while keeping the newest file,
snort is linked to these files while running, do not take the easy way out
by touch and rm, snort will lose sync and not log.

this code needs to be watched.
*/

/* list dir files */
function snort_file_list($snort_log_dir, $snort_log_file)
{
        $dir = opendir ("$snort_log_dir");
        while (false !== ($file = readdir($dir))) {
                if (strpos($file, "$snort_log_file",1) ) {
                    $file_list[] = $file;
                }
        }
        return $file_list;
}

/* snort dir files */
function snort_file_sort($snort_file1, $snort_file2)
{
    if ($snort_file1 == $snort_file2) {
        return 0;
    }
    return ($snort_file1 < $snort_file2); // ? -1 : 1; // this flips the array
}

/* build files newest first array */
function snort_build_order($snort_list)
{
        foreach ($snort_list as $value_list) {
        $list_order[] = $value_list;
        }
        return $list_order;
}

/* keep the newest remove the rest */
function snort_remove_files($snort_list_rm, $snort_file_safe)
{
        foreach ($snort_list_rm as $value_list)
        {
                if ($value_list != $snort_file_safe) {
                       exec("/bin/rm /var/log/snort/$value_list");
                }else{
                       exec("/bin/echo '' > /var/log/snort/$snort_file_safe");
                }
        }
}

function post_delete_logs()
{
	global $config, $g;

	
	$snort_log_dir = '/var/log/snort';

	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule'])) 
	{


			$rule_array = $config['installedpackages']['snortglobal']['rule'];
			$id = -1;
			foreach ($rule_array as $value)
			{

				if ($id == '') {
				$id = 0;
				}

				$id += 1;

				$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
				$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);
				$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

				if ($if_real != '' && $snort_uuid != '')
				{
					if ($config['installedpackages']['snortglobal']['rule'][$id]['snortunifiedlog'] == 'on')
					{
						$snort_log_file_u2 = "{$snort_uuid}_{$if_real}.u2.";
						$snort_list_u2 = snort_file_list($snort_log_dir, $snort_log_file_u2);
						if (is_array($snort_list_u2)) {
							usort($snort_list_u2, "snort_file_sort");
							$snort_u2_rm_list = snort_build_order($snort_list_u2);
							snort_remove_files($snort_u2_rm_list, $snort_u2_rm_list[0]);
						}
					}else{
						exec("/bin/rm $snort_log_dir/snort_{$snort_uuid}_{$if_real}.u2*");
					}

					if ($config['installedpackages']['snortglobal']['rule'][$id]['tcpdumplog'] == 'on')
					{
						$snort_log_file_tcpd = "{$snort_uuid}_{$if_real}.tcpdump.";
						$snort_list_tcpd = snort_file_list($snort_log_dir, $snort_log_file_tcpd);
						if (is_array($snort_list_tcpd)) {
							usort($snort_list_tcpd, "snort_file_sort");
							$snort_tcpd_rm_list = snort_build_order($snort_list_tcpd);
							snort_remove_files($snort_tcpd_rm_list, $snort_tcpd_rm_list[0]);
						}
					}else{
						exec("/bin/rm $snort_log_dir/snort_{$snort_uuid}_{$if_real}.tcpdump*");
					}

					/* create barnyard2 configuration file */
					//if ($config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'] == 'on')
						//create_barnyard2_conf($id, $if_real, $snort_uuid);

					if ($config['installedpackages']['snortglobal']['rule'][$id]['perform_stat'] == on)
					{
						exec("/bin/echo '' > /var/log/snort/snort_{$snort_uuid}_{$if_real}.stats");
					}
				}
			}
	}
}
	
function snort_postinstall()
{
	global $config;
	conf_mount_rw();
	
/* find out if were in 1.2.3-RELEASE */
$pfsense_ver_chk = exec('/bin/cat /etc/version');
if ($pfsense_ver_chk == '1.2.3-RELEASE')
{
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
	$bpfbufsize = $config['installedpackages']['snortglobal']['bpfbufsize'];
	$bpfmaxbufsize = $config['installedpackages']['snortglobal']['bpfmaxbufsize'];
	$bpfmaxinsns = $config['installedpackages']['snortglobal']['bpfmaxinsns'];

	/* cleanup default files */
	if(file_exists('/usr/local/etc/snort/snort.conf-sample')) 
	{
		exec('/bin/rm /usr/local/etc/snort/snort.conf-sample');
		exec('/bin/rm /usr/local/etc/snort/threshold.conf-sample');
		exec('/bin/rm /usr/local/etc/snort/sid-msg.map-sample');
		exec('/bin/rm /usr/local/etc/snort/unicode.map-sample');
		exec('/bin/rm /usr/local/etc/snort/classification.config-sample');
		exec('/bin/rm /usr/local/etc/snort/generators-sample');
		exec('/bin/rm /usr/local/etc/snort/reference.config-sample');
		exec('/bin/rm /usr/local/etc/snort/gen-msg.map-sample');
		exec('/bin/rm /usr/local/etc/snort/sid');
		exec('/bin/rm /usr/local/etc/rc.d/snort');
		exec('/bin/rm /usr/local/etc/rc.d/bardyard2');
	}
	
	/* remove example files */
	if(file_exists('/usr/local/lib/snort/dynamicrules/lib_sfdynamic_example_rule.so.0'))
	{
		exec('/bin/rm /usr/local/lib/snort/dynamicrules/lib_sfdynamic_example*');
	}
	
	if(file_exists('/usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example.so'))
	{
		exec('/bin/rm /usr/local/lib/snort/dynamicpreprocessor/lib_sfdynamic_preprocessor_example*');
	}
	

	/* add snort user and group note: 920 keep the numbers < 2000, above this is reserved in pfSense 2.0 */
	exec('/usr/sbin/pw groupadd snort -g 920');
	exec('/usr/sbin/pw useradd snort -u 920 -c "Snort User" -d /nonexistent -g snort -s /sbin/nologin');


	/* create a few directories and ensure the sample files are in place */	
	if(!file_exists('/usr/local/etc/snort'))
	{
		exec('/bin/mkdir -p /usr/local/etc/snort');
	}
	
	if(!file_exists('/usr/local/etc/snort/custom_rules'))
	{
		exec('/bin/mkdir -p /usr/local/etc/snort/custom_rules/');
	}
	
	if(!file_exists('/usr/local/etc/snort/whitelist'))
	{
		exec('/bin/mkdir -p /usr/local/etc/snort/whitelist/');
	}
	
	if(!file_exists('/var/log/snort/run'))
	{
		exec('/bin/mkdir -p /var/log/snort/run');
	}
	
	if(!file_exists('/var/log/snort/barnyard2'))
	{
		exec('/bin/mkdir -p /var/log/snort/barnyard2/');
	}

	if(!file_exists('/usr/local/lib/snort/dynamicrules/'))
	{
		exec('/bin/mkdir -p /usr/local/lib/snort/dynamicrules/');
	}
	
	if(!file_exists('/var/db/whitelist'))
	{
		touch('/var/db/whitelist');
	}

	/* if users have old log files delete them */
	if(!file_exists('/var/log/snort/alert')) {
		touch('/var/log/snort/alert');
	}else{
		exec('/bin/rm -rf /var/log/snort/*');
		touch('/var/log/snort/alert');
	}
	
	/* rm barnyard2 important */
	if(!file_exists('/usr/local/bin/barnyard2')) {
		exec('/bin/rm /usr/local/bin/barnyard2');
	}
	
	/* important  */
	exec('/usr/sbin/chown -R snort:snort /var/log/snort');
	exec('/usr/sbin/chown -R snort:snort /usr/local/etc/snort');
	exec('/usr/sbin/chown -R snort:snort /usr/local/lib/snort');
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
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/css/style.css');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/css/colorbox.css');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/css/new_tab_menu.css');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/css/sexybuttons.css');
	chdir ("/usr/local/www/snort/images/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/alert.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/down.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/down2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon-table-sort.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon-table-sort-asc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon-table-sort-desc.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/up.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/up2.gif');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/logo.jpg');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/icon_excli.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/arrow_down.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/awesome-overlay-sprite.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/controls.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/logo22.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/new_tab_menu.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/page_white_text.png');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/images/transparent.gif');
	chdir ("/usr/local/www/snort/javascript/");
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/javascript/jquery.colorbox.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/javascript/jquery-1.4.2.min.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/javascript/mootools.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/javascript/sortableTable.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/javascript/row_helper.js');
	exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/javascript/prototype.js');
	
	/* install barnyard2 for 2.0 x86 x64 and 1.2.3 x86 */
	chdir ("/usr/local/bin/");
	
	update_status(gettext("Installing Barnyard2 for $snort_arch..."));
	update_output_window(gettext("Please wait..."));
	if ($pfsense_stable == 'yes') {
		exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/bin/7.3.x86/barnyard2');
	}

	if ($pfsense_stable == 'no' && $snort_arch == 'x86') {
		exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/bin/8.1x86/barnyard2');
	}
	
	if ($pfsense_stable == 'no' && $snort_arch == 'x64') {
		exec('/usr/bin/fetch http://www.pfsense.com/packages/config/snort/bin/8.1x64/barnyard2');
	}
	update_output_window(gettext("Finnished Installing Barnyard2..."));
	
	exec('/bin/chmod 755 /usr/local/bin/barnyard2');
	
	/* back to default */
	chdir ("/root/");
	
	/* make sure snort-old is deinstalled */
	/* remove when snort-old is removed */
	unset($config['installedpackages']['snort']);
	unset($config['installedpackages']['snortdefservers']);
	unset($config['installedpackages']['snortwhitelist']);
	unset($config['installedpackages']['snortthreshold']);
	unset($config['installedpackages']['snortadvanced']);
	write_config();
	conf_mount_rw();
	
	/* remake saved settings */
	if($config['installedpackages']['snortglobal']['forcekeepsettings'] == 'on') {
		update_status(gettext("Saved settings detected..."));
		update_output_window(gettext("Please wait... rebuilding files..."));
		sync_snort_package_empty();
		update_output_window(gettext("Finnished Rebuilding files..."));
	}
	
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
                                $cron_item['command'] = "/usr/bin/nice -n20 /usr/local/bin/php -f /usr/local/pkg/snort/snort_check_for_rule_updates.php >> /tmp/snort_update.log";
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

function sync_snort_package_remove_old()
{

	global $config, $g;

$snort_dir_scan = '/usr/local/etc/snort';

// scan dirm might have to make this into a funtion
$dh_scan  = opendir($snort_dir_scan);
while (false !== ($dir_filename = readdir($dh_scan))) {
    $list_dir_files[] = $dir_filename;
}

// find  patern in a array, very cool code
class array_ereg {
    function array_ereg($pattern) { $this->pattern = $pattern; }
    function ereg($string) {
        return ereg($this->pattern, $string);
    }
}

                        $rule_array2 = $config['installedpackages']['snortglobal']['rule'];
                        $id2 = -1;
                        foreach ($rule_array2 as $value)
                        {

                        $id += 1;

                        $result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
                        $if_real = convert_friendly_interface_to_real_interface_name2($result_lan);

						$snort_rules_list[] = "snort_$id$if_real";

						}


$snort_dir_filter = array_filter($list_dir_files, array(new array_ereg("snort_"), 'ereg'));
$snort_dir_filter_search_result = array_diff($snort_dir_filter, $snort_rules_list);

                        foreach ($snort_dir_filter_search_result as $value)
                        {
							exec("rm -r /usr/local/etc/snort/$value");
                        }

}

/* make sure this func on writes to files and does not start snort */
function sync_snort_package()
{
	global $config, $g;
	conf_mount_rw();

	/* all new files are for the user snort nologin */
	if(!file_exists('/var/log/snort'))
	{
		exec('/bin/mkdir -p /var/log/snort');	
	}
	
	if(!file_exists('/var/log/snort/run'))
	{
		exec('/bin/mkdir -p /var/log/snort/run');
	}
	
	if(!file_exists('/var/log/snort/barnyard2'))
	{
		exec('/bin/mkdir -p /var/log/snort/barnyard2');
	}
	
	/* all new files are for the user snort nologin */
	if(!file_exists('/var/log/snort/alert'))
	{
		exec('/usr/bin/touch /var/log/snort/alert');
	}
	
	/* important  */
	exec('/usr/sbin/chown -R snort:snort /var/log/snort');
	exec('/usr/sbin/chown -R snort:snort /usr/local/etc/snort');
	exec('/usr/sbin/chown -R snort:snort /usr/local/lib/snort');
	exec('/usr/sbin/chown snort:snort /tmp/snort*');
	exec('/usr/sbin/chown snort:snort /var/db/whitelist');

	exec('/bin/chmod 770 /var/db/whitelist');
	exec('/bin/chmod 770 /var/run/snort*');
	exec('/bin/chmod 770 /tmp/snort*');
	exec('/bin/chmod -R 770 /var/log/snort');
	exec('/bin/chmod -R 770 /usr/local/lib/snort');
	exec('/bin/chmod -R 770 /usr/local/etc/snort/');

	conf_mount_ro();
}

/* only run when a single iface needs to sync */
function sync_snort_package_all($id, $if_real, $snort_uuid)
{
	//global $config, $g, $id, $if_real, $snort_uuid, $interface_fake;
	global $config, $g;

/* RedDevil suggested code */
/* TODO: more testing needs to be done */
/* may cause voip to fail */
//exec("/sbin/sysctl net.bpf.bufsize=8388608");
//exec("/sbin/sysctl net.bpf.maxbufsize=4194304");
//exec("/sbin/sysctl net.bpf.maxinsns=512");
//exec("/sbin/sysctl net.inet.tcp.rfc1323=1");

# Error checking
if ($id != '' && $if_real != '') //new
{
	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule'])) 
	{

		conf_mount_rw();

				$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];
				$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
				$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);
				
				/* create snort configuration file */
				create_snort_conf($id, $if_real, $snort_uuid);

				/* if rules exist cp rules to each iface */
				create_rules_iface($id, $if_real, $snort_uuid);
				
				/* only build whitelist when needed */
				if ($config['installedpackages']['snortglobal']['rule'][$id]['blockoffenders7'] == 'on'){
					create_snort_whitelist($id, $if_real);
				}
				
				/* only build threshold when needed */
				if ($config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'] != 'default'){
					create_snort_suppress($id, $if_real);
				}
				
				/* create snort bootup file snort.sh only create once */
				create_snort_sh();

				/* create barnyard2 configuration file */
				$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
				if ($snortbarnyardlog_info_chk == 'on')
					create_barnyard2_conf($id, $if_real, $snort_uuid);

				sync_snort_package();

		conf_mount_ro();
		}
	}
}

/* only run when all ifaces needed to sync */
function sync_snort_package_empty()
{
	global $config, $g;
	conf_mount_rw();
	
/* RedDevil suggested code */
/* TODO: more testing needs to be done */
/* may cause voip to fail */
//exec("/sbin/sysctl net.bpf.bufsize=8388608");
//exec("/sbin/sysctl net.bpf.maxbufsize=4194304");
//exec("/sbin/sysctl net.bpf.maxinsns=512");
//exec("/sbin/sysctl net.inet.tcp.rfc1323=1");

	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule'])) 
	{
		if ($id == "") 
		{

			$rule_array = $config['installedpackages']['snortglobal']['rule'];
			$id = -1;
			foreach ($rule_array as $value)
			{

				if ($id == '') {
				$id = 0;
				}

				$id += 1;

				$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
				$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);
				$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

				if ($if_real != '' && $snort_uuid != '') {
					
				/* create snort configuration file */
				create_snort_conf($id, $if_real, $snort_uuid);

				/* if rules exist cp rules to each iface */
				create_rules_iface($id, $if_real, $snort_uuid);
				
				/* only build whitelist when needed */
				if ($config['installedpackages']['snortglobal']['rule'][$id]['blockoffenders7'] == 'on'){
					create_snort_whitelist($id, $if_real);
				}
				
				/* only build threshold when needed */
				if ($config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'] != 'default'){
					create_snort_suppress($id, $if_real);
				}

				/* create barnyard2 configuration file */
				$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
				if ($snortbarnyardlog_info_chk == 'on')
					create_barnyard2_conf($id, $if_real, $snort_uuid);
				}
			}

				/* create snort bootup file snort.sh only create once */
				create_snort_sh();

				sync_snort_package();
				
				conf_mount_ro();

		}
	}
}

/* only bootup and ip refresh */
function sync_snort_package_config()
{
	global $config, $g;
	conf_mount_rw();
	
/* RedDevil suggested code */
/* TODO: more testing needs to be done */
/* may cause voip to fail */
//exec("/sbin/sysctl net.bpf.bufsize=8388608");
//exec("/sbin/sysctl net.bpf.maxbufsize=4194304");
//exec("/sbin/sysctl net.bpf.maxinsns=512");
//exec("/sbin/sysctl net.inet.tcp.rfc1323=1");

	/* do not start config build if rules is empty */
	if (!empty($config['installedpackages']['snortglobal']['rule'])) 
	{
		if ($id == "") 
		{

			$rule_array = $config['installedpackages']['snortglobal']['rule'];
			$id = -1;
			foreach ($rule_array as $value)
			{

				if ($id == '') {
				$id = 0;
				}

				$id += 1;

				$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
				$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);
				$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];

				if ($if_real != '' && $snort_uuid != '') {
					
				/* create snort configuration file */
				create_snort_conf($id, $if_real, $snort_uuid);
				
				/* only build whitelist when needed */
				if ($config['installedpackages']['snortglobal']['rule'][$id]['blockoffenders7'] == 'on'){
					create_snort_whitelist($id, $if_real);
				}
				
				/* only build threshold when needed */
				if ($config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'] != 'default'){
					create_snort_suppress($id, $if_real);
				}
				
				/* create barnyard2 configuration file */
				$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
				if ($snortbarnyardlog_info_chk == 'on')
					create_barnyard2_conf($id, $if_real, $snort_uuid);
				}
			}

				sync_snort_package();
				
				conf_mount_ro();

		}
	}
}


/* Start of main config files */
/* Start of main config files */

/* create threshold file */
/* TODO: other func should mirror this code */
function create_snort_suppress($id, $if_real) {
	
	    global $config, $g;
        conf_mount_rw();
        
        /* make sure dir is there */
        if (!file_exists('/usr/local/etc/snort/suppress/')) {
        	exec('/bin/mkdir -p /usr/local/etc/snort/suppress/');
        }
        
        if ($config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'] != 'default') {

        	preg_match('/([0-9]+)$/', $config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'], $slist_num_wrt);
        	
        	$whitelist_key_s = find_suppress_key($slist_num_wrt[0]);
        
        	/* file name */
        	$suppress_file_name = $config['installedpackages']['snortglobal']['suppress']['item'][$whitelist_key_s]['name'];
        	
        	/* Message */
			$s_data .= '# This file is auto generated by the snort package. Please do not edit this file by hand.' . "\n\n";
      
        	/* user added arguments */
			$s_data .= str_replace("\r", "", base64_decode($config['installedpackages']['snortglobal']['suppress']['item'][$whitelist_key_s]['suppresspassthru']));
        
        	/* open snort's whitelist for writing */
        	$suppresslist_w = fopen("/usr/local/etc/snort/suppress/$suppress_file_name", "w");
        	if(!$suppresslist_w) {
                log_error("Could not open /usr/local/etc/snort/suppress/$suppress_file_name for writing.");
                return;
        	}
        
        	fwrite($suppresslist_w, $s_data);
        	fclose($suppresslist_w);
        	conf_mount_ro();
        
        }
        
}

function create_snort_whitelist($id, $if_real) {
	
	    global $config, $g;
        conf_mount_rw();
        
        /* make sure dir is there */
        if (!file_exists('/usr/local/etc/snort/whitelist/')) {
        	exec('/bin/mkdir -p /usr/local/etc/snort/whitelist/');
        }
        
        if ($config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'] == 'default') {
        
        	/* open snort's whitelist for writing */
        	$whitelist_w = fopen("/usr/local/etc/snort/whitelist/defaultwlist", "w");
        	if(!$whitelist_w) {
                log_error("Could not open /usr/local/etc/snort/whitelist/defaultwlist for writing.");
                return;
        	}
        	
        	$w_data = build_base_whitelist('whitelist', 'yes', 'yes', 'yes', 'yes', 'yes', 'no');

        }else{

        	preg_match('/^([a-zA-z0-9]+)/', $config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'], $wlist_name_wrt);
        	preg_match('/([0-9]+)$/', $config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'], $wlist_num_wrt);
        	
        	$whitelist_key_w = find_whitelist_key($wlist_num_wrt[0]);
        
        	$build_netlist = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w]['snortlisttype'];
        	$wanip = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w]['wanips'];
        	$wangw = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w]['wangateips'];
        	$wandns = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w]['wandnsips'];
        	$vips = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w]['vips'];
        	$vpns = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_w]['vpnips'];
        
        	/* open snort's whitelist for writing */
        	$whitelist_w = fopen("/usr/local/etc/snort/whitelist/$wlist_name_wrt[0]", "w");
        	if(!$whitelist_w) {
                log_error("Could not open /usr/local/etc/snort/whitelist/$wlist_name_wrt[0] for writing.");
                return;
        	}

        	$w_data = build_base_whitelist($build_netlist, $wanip, $wangw, $wandns, $vips, $vpns, $whitelist_key_w);
        
        }
        
        fwrite($whitelist_w, $w_data);
        fclose($whitelist_w);
        conf_mount_ro();
        
}

function create_snort_homenet($id, $if_real) {
	
	    global $config, $g;
        conf_mount_rw();
        
        if ($config['installedpackages']['snortglobal']['rule'][$id]['homelistname'] == 'default' || $config['installedpackages']['snortglobal']['rule'][$id]['homelistname'] == '') {
        	return build_base_whitelist('netlist', 'yes', 'yes', 'yes', 'yes', 'yes', 'no');
        }else{
        	preg_match('/([0-9]+)$/', $config['installedpackages']['snortglobal']['rule'][$id]['homelistname'], $hlist_num_wrt);
        	
        	$whitelist_key_h = find_whitelist_key($hlist_num_wrt[0]);
        
        	$build_netlist_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['snortlisttype'];
        	$wanip_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['wanips'];
        	$wangw_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['wangateips'];
       		$wandns_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['wandnsips'];
        	$vips_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['vips'];
        	$vpns_h = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_h]['vpnips'];
        	
        	return build_base_whitelist($build_netlist_h, $wanip_h, $wangw_h, $wandns_h, $vips_h, $vpns_h, $whitelist_key_h);

        }
        
        conf_mount_ro();
        
}

function create_snort_externalnet($id, $if_real) {
	
	    global $config, $g;
        conf_mount_rw();

        	preg_match('/([0-9]+)$/', $config['installedpackages']['snortglobal']['rule'][$id]['externallistname'], $exlist_num_wrt);
        	
        	$whitelist_key_ex = find_whitelist_key($exlist_num_wrt[0]);
        
        	$build_netlist_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['snortlisttype'];
        	$wanip_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['wanips'];
        	$wangw_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['wangateips'];
       		$wandns_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['wandnsips'];
        	$vips_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['vips'];
        	$vpns_ex = $config['installedpackages']['snortglobal']['whitelist']['item'][$whitelist_key_ex]['vpnips'];
        	
        	return build_base_whitelist($build_netlist_ex, $wanip_ex, $wangw_ex, $wandns_ex, $vips_ex, $vpns_ex, $whitelist_key_ex);
        
        conf_mount_ro();
        
}

/* open snort.sh for writing" */
function create_snort_sh()
{
		# Don not add $id or this will break

        global $config, $g;
        conf_mount_rw();

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

			$snort_uuid = $config['installedpackages']['snortglobal']['rule'][$id]['uuid'];
			$result_lan = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];
			$if_real = convert_friendly_interface_to_real_interface_name2($result_lan);

	/* define snortbarnyardlog_chk */
	$snortbarnyardlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_enable'];
	$snortbarnyardlog_mysql_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_mysql'];
	
	if ($snortbarnyardlog_info_chk == 'on' && $snortbarnyardlog_mysql_info_chk != '') {
	$start_barnyard2 = "sleep 4;/usr/local/bin/barnyard2 -f snort_{$snort_uuid}_{$if_real}.u2 -u snort -g snort --pid-path /var/log/snort/run -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf -d /var/log/snort -D -q";
	}
			
/* Get all interface startup commands ready */

$snort_sh_text2[] = <<<EOD
###### For Each Iface

		# If Snort proc is NOT running
		if [ "`/bin/ps -auwx | grep -v grep | grep "R {$snort_uuid}{$if_real}" | awk '{print $2;}'`" = "" ]; then

		/bin/echo "snort.sh run" > /tmp/snort.sh.pid
		
		# Start snort and barnyard2
		/bin/rm /var/run/snort_{$snort_uuid}_{$if_real}.pid
		/bin/rm /var/run/snort_{$snort_uuid}_{$if_real}.pid.lck

		/usr/local/bin/snort -u snort -g snort -R {$snort_uuid} -D -q -l /var/log/snort --pid-path /var/log/snort/run -G {$snort_uuid} -c /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf -i {$if_real}
		$start_barnyard2

		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort HARD Reload For {$snort_uuid}_{$if_real}..."

		fi
EOD;

$snort_sh_text3[] = <<<EOE

###### For Each Iface

		#### Fake start only used on bootup and Pfsense IP changes
		#### Only try to restart if snort is running on Iface
		if [ "`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "R {$snort_uuid}{$if_real}" | /usr/bin/awk '{print $2;}'`" != "" ]; then

		snort_pid="`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "R {$snort_uuid}{$if_real}" | /usr/bin/awk '{print $2;}'`"
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort already running, soft restart"

		#### Restart Iface
		/bin/kill -HUP \${snort_pid}
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort Soft Reload For {$snort_uuid}_{$if_real}..."

		fi

EOE;

$snort_sh_text4[] = <<<EOF

		pid_s=`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "R {$snort_uuid}{$if_real}" | /usr/bin/awk '{print \$2;}'`
		sleep 3
		pid_b=`/bin/ps -auwx | /usr/bin/grep -v grep | /usr/bin/grep "snort_{$snort_uuid}_{$if_real}.u2" | /usr/bin/awk '{print \$2;}'`

		if [ \${pid_s} ] ; then
		
		/bin/echo "snort.sh run" > /tmp/snort.sh.pid
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort HARD STOP For {$snort_uuid}_{$if_real}..."

		/bin/kill \${pid_s}
		sleep 3
		/bin/kill \${pid_b}

		/bin/rm /var/run/snort_{$snort_uuid}_{$if_real}.pid.lck
		/bin/rm /var/run/snort_{$snort_uuid}_{$if_real}.pid

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
########
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

		#### Remake the configs on boot Important!
		/usr/local/bin/php -f /usr/local/pkg/pf/snort_dynamic_ip_reload.php &
		/usr/bin/logger -p daemon.info -i -t SnortStartup "Snort Startup files Sync..."

$start_snort_iface_restart

		/bin/rm /tmp/snort.sh.pid

		#### If on Fake start snort is NOT running DO a real start.
		if [ "`/bin/ps -auwx | grep -v grep | grep "R {$snort_uuid}{$if_real}" | awk '{print $2;}'`" = "" ]; then

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
function create_rules_iface($id, $if_real, $snort_uuid)
{

    global $config, $g;
    conf_mount_rw();

		$if_rule_dir = "/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules";
		$folder_chk = (count(glob("$if_rule_dir/*")) === 0) ? 'empty' : 'full';

		if ($folder_chk == "empty")
		{
			exec("/bin/cp -R /usr/local/etc/snort/rules /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}");
			if (file_exists("/usr/local/etc/snort/custom_rules/local_{$snort_uuid}_{$if_real}.rules"))
			{
				exec("/bin/cp /usr/local/etc/snort/custom_rules/local_{$snort_uuid}_{$if_real}.rules /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules/local_{$snort_uuid}_{$if_real}.rules");
			}
		}
	
}

/* open barnyard2.conf for writing */
function create_barnyard2_conf($id, $if_real, $snort_uuid) {
        global $bconfig, $g;
        /* write out barnyard2_conf */

		if(!file_exists("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf"))
		{
			exec("/usr/bin/touch /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf");
		}

		if(!file_exists("/var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo"))
		{
			exec("/usr/bin/touch /var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo");
			exec("/usr/sbin/chown snort:snort /var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo");
			exec("/bin/chmod 770 /var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo");
		}
		
        $barnyard2_conf_text = generate_barnyard2_conf($id, $if_real, $snort_uuid);
        $bconf = fopen("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf", "w");
        if(!$bconf) {
                log_error("Could not open /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf for writing.");
                exit;
        }
        fwrite($bconf, $barnyard2_conf_text);
        fclose($bconf);
}

/* open barnyard2.conf for writing" */
function generate_barnyard2_conf($id, $if_real, $snort_uuid) {

        global $config, $g;
        conf_mount_rw();

/* define snortbarnyardlog */
/* TODO: add support for the other 5 output plugins */

$snortbarnyardlog_database_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['barnyard_mysql'];
$snortbarnyardlog_hostname_info_chk = exec("/bin/hostname");
/* user add arguments */
$snortbarnyardlog_config_pass_thru = str_replace("\r", "", base64_decode($config['installedpackages']['snortglobal']['rule'][$id]['barnconfigpassthru']));

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

config reference_file:	    /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/reference.config
config classification_file: /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/classification.config
config gen_file:            /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/gen-msg.map
config sid_file:	        /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/sid-msg.map

config hostname:       $snortbarnyardlog_hostname_info_chk
config interface:      {$snort_uuid}_{$if_real}
config decode_data_link
config waldo_file: /var/log/snort/barnyard2/{$snort_uuid}_{$if_real}.waldo

## START user pass through ##

{$snortbarnyardlog_config_pass_thru}

## END user pass through ##

# Step 2: setup the input plugins
input unified2

config logdir: /var/log/snort

# database: log to a variety of databases
# output database: log, mysql, user=xxxx password=xxxxxx dbname=xxxx host=xxx.xxx.xxx.xxxx

$snortbarnyardlog_database_info_chk

EOD;

        return $barnyard2_conf_text;

}

function create_snort_conf($id, $if_real, $snort_uuid)
{
	global $config, $g;
	/* write out snort.conf */
	
	if ($if_real != '' && $snort_uuid != '') {
		
	if (!file_exists("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf")) {
		exec("/bin/mkdir /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/");
		exec("/usr/bin/touch /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf");
	}

	$snort_conf_text = generate_snort_conf($id, $if_real, $snort_uuid);
	conf_mount_rw();
	$conf = fopen("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf", "w");
	if(!$conf) {
		log_error("Could not open /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf for writing.");
		exit;
	}
	fwrite($conf, $snort_conf_text);
	fclose($conf);
	conf_mount_ro();
	}
}

function snort_deinstall()
{

	global $config, $g;
	conf_mount_rw();

	/* remove custom sysctl */
	remove_text_from_file("/etc/sysctl.conf", "sysctl net.bpf.bufsize=20480");
	/* decrease bpf buffers back to 4096, from 20480 */
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
	exec("cd /var/db/pkg && pkg_delete `ls | grep mysql-client-5.1.50_1`");
	exec('rm -r /usr/local/bin/barnyard2');
	
	/* TODO: figure out how to detect pfsense packages that use the same freebsd pkckages  and not deinstall */
	//exec("cd /var/db/pkg && pkg_delete `ls | grep perl`");
	//exec("cd /var/db/pkg && pkg_delete `ls | grep barnyard2`");	
	//exec("cd /var/db/pkg && pkg_delete `ls | grep pcre`"); // Never remove pcre or pfsense will break

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
	
	conf_mount_ro();

}

function generate_snort_conf($id, $if_real, $snort_uuid)
{
	global $config, $g;
	
	conf_mount_rw();
	
	/* custom home nets */
	$home_net = create_snort_homenet($id, $if_real);
	
	if ($config['installedpackages']['snortglobal']['rule'][$id]['externallistname'] == 'default'){
		$external_net = '!$HOME_NET';
	}else{
		$external_net = create_snort_externalnet($id, $if_real);
	}
	
	/* obtain external interface */
	/* XXX: make multi wan friendly */
	$snort_ext_int = $config['installedpackages']['snortglobal']['rule'][$id]['interface'];

	/* user added arguments */
	$snort_config_pass_thru = str_replace("\r", "", base64_decode($config['installedpackages']['snortglobal']['rule'][$id]['configpassthru']));
	
	/* create basic files */
	if(!file_exists("/usr/local/etc/snort/snort/snort_{$snort_uuid}_{$if_real}"))
	{
		exec("/bin/mkdir -p /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/");
		exec("/bin/mkdir -p /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules");

		if(!file_exists("/usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/gen-msg.map"))
		{
			exec("/bin/cp /usr/local/etc/snort/classification.config /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/classification.config");
			exec("/bin/cp /usr/local/etc/snort/gen-msg.map /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/gen-msg.map");
			exec("/bin/cp /usr/local/etc/snort/reference.config /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/reference.config");
			exec("/bin/cp /usr/local/etc/snort/sid-msg.map /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/sid-msg.map");
			exec("/bin/cp /usr/local/etc/snort/unicode.map /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/unicode.map");
			exec("/bin/cp /usr/local/etc/snort/threshold.conf /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/threshold.conf");
			exec("/bin/cp /usr/local/etc/snort/snort.conf /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/snort.conf");
			exec("/bin/cp/usr/bin/touch /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/barnyard2.conf");
			exec("/bin/mkdir -p /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules");
		}
	}


/* define basic log filename */
$snortunifiedlogbasic_type = "output unified: filename snort_{$snort_uuid}_{$if_real}.log, limit 128"; 
	
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
    $tcpdumplog_type = "output log_tcpdump: snort_{$snort_uuid}_{$if_real}.tcpdump";

/* define snortunifiedlog */
$snortunifiedlog_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['snortunifiedlog'];
if ($snortunifiedlog_info_chk == on)
    $snortunifiedlog_type = "output unified2: filename snort_{$snort_uuid}_{$if_real}.u2, limit 128";   

/* define spoink */
$spoink_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['blockoffenders7'];
if ($spoink_info_chk == on) {
	
	preg_match('/^([a-zA-z0-9]+)/', $config['installedpackages']['snortglobal']['rule'][$id]['whitelistname'], $wlist_name_file);

	if ($wlist_name_file[0] == 'default') {
		$spoink_whitelist_name = 'defaultwlist';
	}else{
		$spoink_whitelist_name = $wlist_name_file[0];
	}
	
    $spoink_type = "output alert_pf: /usr/local/etc/snort/whitelist/$spoink_whitelist_name,snort2c";
    
}

/* define threshold file */
$threshold_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'];
if ($threshold_info_chk != 'default') {
	
	preg_match('/^([a-zA-z0-9]+)/', $config['installedpackages']['snortglobal']['rule'][$id]['suppresslistname'], $slist_name_file2);

	$threshold_name = $slist_name_file2[0];
	
    $threshold_file_name = "include /usr/local/etc/snort/suppress/$threshold_name";
    
}
    
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
		$snort_performance = "ac-bnfa";


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

preprocessor perfmonitor: time 300 file /var/log/snort/snort_{$snort_uuid}_{$if_real}.stats pktcnt 10000
	
EOD;

$def_perform_stat_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['perform_stat'];
if ($def_perform_stat_info_chk == "on")
	$def_perform_stat_type = "$snort_perform_stat";
else 
	$def_perform_stat_type = "";
	
$def_flow_depth_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['flow_depth'];
	if ($def_flow_depth_info_chk == '')
	{
		$def_flow_depth_type = '0';
	}else{ 
		$def_flow_depth_type = $config['installedpackages']['snortglobal']['rule'][$id]['flow_depth'];
	}

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
                        flow_depth {$def_flow_depth_type}  \
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
	
/* stream5 queued settings */


$def_max_queued_bytes_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['max_queued_bytes'];
	if ($def_max_queued_bytes_info_chk == '')
	{
		$def_max_queued_bytes_type = '';
	}else{ 
		$def_max_queued_bytes_type = ' max_queued_bytes ' . $config['installedpackages']['snortglobal']['rule'][$id]['max_queued_bytes'] . ',';
	}
	
$def_max_queued_segs_info_chk = $config['installedpackages']['snortglobal']['rule'][$id]['max_queued_segs'];
	if ($def_max_queued_segs_info_chk == '')
	{
		$def_max_queued_segs_type = '';
	}else{ 
		$def_max_queued_segs_type = ' max_queued_segs ' . $config['installedpackages']['snortglobal']['rule'][$id]['max_queued_segs'] . ',';
	}
	
	
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
#	Copyright (C) 2009-2010 Robert Zelaya
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
var EXTERNAL_NET {$external_net}

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

var RULE_PATH /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/rules
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
preprocessor stream5_tcp: policy BSD, ports both all,{$def_max_queued_bytes_type}{$def_max_queued_segs_type} use_static_footprint_sizes
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

$snortunifiedlogbasic_type
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

include /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/reference.config
include /usr/local/etc/snort/snort_{$snort_uuid}_{$if_real}/classification.config
$threshold_file_name

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

//	ob_flush();
	$contents = file_get_contents($filename);
	if(stristr($contents, "You don't have permission")) {
		if(!$console_mode) {
			update_all_status("An error occured while downloading {$filename}.");
			hide_progress_bar_status();
		} else {
			log_error("An error occured.  Scroll down to inspect it's contents.");
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
