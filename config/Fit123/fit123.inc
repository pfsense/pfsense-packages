<?php

function Fit123_install_deinstall() {
	conf_mount_rw();
	config_lock();
	exec("cd ..");
	exec("rm -d -R /usr/local/pkg/Fit123");
	conf_mount_ro();
	config_unlock();
}

function Fit123_install_config() {
	conf_mount_rw();
	config_lock();
	exec("cd ..");
//Adding new themes
//	exec("tar -zxovf /usr/local/pkg/code-red.tar.gz");
//	exec("mv code-red /usr/local/www/themes/code-red");
//Creating backup directory
	exec("mkdir /usr/local/pkg/Fit123");
	exec("mkdir /usr/local/pkg/Fit123/backup");
//Copy orignal files to backup dir
	//Date
	exec("cp /usr/local/www/index.php /usr/local/pkg/Fit123/backup/");
	//Captive Portal Add-On
//	exec("cp /etc/inc/filter.inc /usr/local/pkg/Fit123/backup/");
//	exec("cp /usr/local/www/services_captiveportal.php /usr/local/pkg/Fit123/backup/");
	//LTSP network boot Option
	exec("cp /etc/inc/services.inc /usr/local/pkg/Fit123/backup/");
	exec("cp /usr/local/www/services_dhcp.php /usr/local/pkg/Fit123/backup/");
	//DNS Server adds option for a 3th and 4th DNS Server
	exec("cp /usr/local/www/system.php /usr/local/pkg/Fit123/backup/");
//Greate directories and downloading files to them
	//Date
	exec("mkdir /usr/local/pkg/Fit123/date");
	exec("fetch -o /usr/local/pkg/Fit123/date/index.php https://packages.pfsense.org/packages/config/Fit123/bin/date/index.abc");
	//Captive Portal Add-On
	exec("mkdir /usr/local/pkg/Fit123/cpaddon");
	exec("fetch -o /usr/local/pkg/Fit123/cpaddon/filter.inc https://packages.pfsense.org/packages/config/Fit123/bin/cpaddon/filter.inc");
	exec("fetch -o /usr/local/pkg/Fit123/cpaddon/services_captiveportal.php https://packages.pfsense.org/packages/config/Fit123/bin/cpaddon/services_captiveportal.abc");
	//LTSP 3th network boot Option
	exec("mkdir /usr/local/pkg/Fit123/LTSP");
	exec("fetch -o /usr/local/pkg/Fit123/LTSP/ https://packages.pfsense.org/packages/config/Fit123/bin/ltsp/services.inc");
	exec("fetch -o /usr/local/pkg/Fit123/LTSP/services_dhcp.php https://packages.pfsense.org/packages/config/Fit123/bin/ltsp/services_dhcp.abc");
	//AFC Reset's states after filter change
	exec("mkdir /usr/local/pkg/Fit123/afc");
	exec("fetch -o /usr/local/pkg/Fit123/afc/reset_states.sh https://packages.pfsense.org/packages/config/Fit123/bin/afc/reset_states.sh");
	exec("chmod 744 /usr/local/pkg/Fit123/afc/reset_states.sh");
	//DDNS 
	exec("mkdir /usr/local/pkg/Fit123/ddns");
	//DNS Server adds option for a 3th and 4th DNS Server
	exec("mkdir /usr/local/pkg/Fit123/dnssrv");
	exec("fetch -o /usr/local/pkg/Fit123/dnssrv/system.php https://packages.pfsense.org/packages/config/Fit123/bin/dnssrv/system.abc");
	conf_mount_ro();
	config_unlock();
}

function Fit123_config() {
global $config;

	$adddate = $config['installedpackages']['fit123']['config'][0]['adddate'];
	if($adddate)
	exec("cp /usr/local/pkg/Fit123/date/index.php /usr/local/www/index.php");
	else
	exec("cp /usr/local/pkg/Fit123/backup/index.php /usr/local/www/index.php");

	$cpaddon = $config['installedpackages']['fit123']['config'][0]['cpaddon'];
	if($cpaddon){
	exec("cp /usr/local/pkg/Fit123/cpaddon/filter.inc /etc/inc/filter.inc");
	exec("cp /usr/local/pkg/Fit123/cpaddon/services_captiveportal.php /usr/local/www/services_captiveportal.php");
	}
	else
	{
	exec("cp /usr/local/pkg/Fit123/backup/filter.inc /etc/inc/filter.inc");
	exec("cp /usr/local/pkg/Fit123/backup/services_captiveportal.php /usr/local/www/services_captiveportal.php");
	}

	$ltsp = $config['installedpackages']['fit123']['config'][0]['ltsp'];
	if($ltsp){
	exec("cp /usr/local/pkg/Fit123/LTSP/services.inc /etc/inc/services.inc");
	exec("cp /usr/local/pkg/Fit123/LTSP/services_dhcp.php /usr/local/www/services_dhcp.php");
	}	
	else
	{
	exec("cp /usr/local/pkg/Fit123/backup/services.inc /etc/inc/services.inc");
	exec("cp /usr/local/pkg/Fit123/backup/services_dhcp.php /usr/local/www/services_dhcp.php");
	}

	$afc = $config['installedpackages']['fit123']['config'][0]['afc'];
	if($afc)
	$config['system']['afterfilterchangeshellcmd'] = "/usr/local/pkg/Fit123/afc/reset_states.sh";
	else
	$config['system']['afterfilterchangeshellcmd'] = "";

	$dnssrv = $config['installedpackages']['fit123']['config'][0]['dnssrv'];
	if($dnssrv)
	exec("cp /usr/local/pkg/Fit123/dnssrv/system.php /usr/local/www/system.php");
	else
	exec("cp /usr/local/pkg/Fit123/backup/system.php /usr/local/www/system.php");
}

function cass_config(){
global $config;
// Is CASS enable or not
	$hoba2 = $config['installedpackages']['cass']['config'][0]['cass2'];
	if($hoba2){
	// Is CASS.sh already added as a conjob
        $task_key = '/usr/local/pkg/Fit123/CASS.sh';
        $x_name='';
        $x=0;
        foreach($config['cron']['item'] as $item) {
            if($item['command']==$task_key) {
               $x_name = $x;
            }
            $x++;
        }
        unset($x);
        if(!$x_name > 0)
	{
	// Add cron job CASS.sh to config.xml
        $cron_item = array();
        $cron_item['minute']    = "*/1";
        $cron_item['hour']      = "*";
        $cron_item['mday']      = "*";
        $cron_item['month']     = "*";
        $cron_item['wday']      = "*";
        $cron_item['who']       = "root";
        $cron_item['command']   = "/usr/local/pkg/Fit123/CASS.sh";
        $config['cron']['item'][] = $cron_item;
        write_config();
        configure_cron();
	}
	}
	else
	{
	// Remove CASS.sh cron entry from config.xml
        $task_key = '/usr/local/pkg/Fit123/CASS.sh';
        $x_name='';
        $x=0;
        foreach($config['cron']['item'] as $item) {
            if($item['command']==$task_key) {
               $x_name = $x;
            }
            $x++;
        }
        unset($x);
	if($x_name > 0)
        {
            unset($config['cron']['item'][$x_name]);
            write_config();
        }
        configure_cron();
}

// Create local shell script to watch for wan ip change
	$local_voip = $config['installedpackages']['cass']['config'][0]['ypia'];
	$voip_p = $config['installedpackages']['cass']['config'][0]['iaoyv'];

	//Open CASS.sh for writing	
	$hfd = fopen("/usr/local/pkg/Fit123/CASS.sh","w");
	if(!$hfd) {
	  log_error("could not open /usr/local/pkg/Fit123/CASS.sh for writing");
	exit;
	}
	// The start of the config file
	fwrite($hfd, '#!/bin/sh');
	fwrite($hfd, "\n");
	fwrite($hfd, 'lip="');
	fwrite($hfd, $local_voip);
	fwrite($hfd, '"');
	fwrite($hfd, "\n");
	fwrite($hfd, 'rip="');
	fwrite($hfd, $voip_p);
	fwrite($hfd, '"');
	fwrite($hfd, "\n");
	fwrite($hfd, 'dns_file="/usr/local/pkg/Fit123/CASS.ip"');
	fwrite($hfd, "\n");
	fwrite($hfd, 'EXIT_SUCCESS=0');
	fwrite($hfd, "\n");
	fwrite($hfd, 'EXIT_FAILURE=1');
	fwrite($hfd, "\n");
	fwrite($hfd, 'if [ `id -u` -ne 0 ]');
	fwrite($hfd, "\n");
	fwrite($hfd, 'then');
	fwrite($hfd, "\n");
	fwrite($hfd, '        echo "Only root may run this program."');
	fwrite($hfd, "\n");
	fwrite($hfd, '        exit $EXIT_FAILURE');
	fwrite($hfd, "\n");
	fwrite($hfd, 'fi');
	fwrite($hfd, "\n");
	fwrite($hfd, 'get_ip(){');
	fwrite($hfd, "\n");
	fwrite($hfd, '        if [ -f $dns_file ]');
	fwrite($hfd, "\n");
	fwrite($hfd, '        then');
	fwrite($hfd, "\n");
	fwrite($hfd, '                registered_ip=`cat ${dns_file}`');
	fwrite($hfd, "\n");
	fwrite($hfd, '        else');
	fwrite($hfd, "\n");
	fwrite($hfd, '                registered_ip=""');
	fwrite($hfd, "\n");
	fwrite($hfd, '        fi');
	fwrite($hfd, "\n");
	fwrite($hfd, '        current_ip=`fetch -qo- http://jackson.io/ip/`');
	fwrite($hfd, "\n");
	fwrite($hfd, '}');
	fwrite($hfd, "\n");
	fwrite($hfd, 'update_hosts(){');
	fwrite($hfd, "\n");
	fwrite($hfd, '        if [ "$registered_ip" != "$current_ip" ]');
	fwrite($hfd, "\n");
	fwrite($hfd, '        then');
	fwrite($hfd, "\n");
	fwrite($hfd, '                /sbin/pfctl -k $lip -k $rip');
	fwrite($hfd, "\n");
	fwrite($hfd, '                echo $current_ip > $dns_file');
	fwrite($hfd, "\n");
	fwrite($hfd, '                echo "WAN ip address changed, clearing states entries.." | logger');
	fwrite($hfd, "\n");
	fwrite($hfd, '        fi');
	fwrite($hfd, "\n");
	fwrite($hfd, '}');
	fwrite($hfd, "\n");
	fwrite($hfd, 'get_ip');
	fwrite($hfd, "\n");
	fwrite($hfd, 'update_hosts');
	fwrite($hfd, "\n");
	fwrite($hfd, 'exit $EXIT_SUCCESS');
	fwrite($hfd, "\n");
	fclose($hfd);

	exec("chmod 744 /usr/local/pkg/Fit123/CASS.sh");
}

function ddns_config(){
global $config;

// Is DDNS enable or not
	$hoba = $config['installedpackages']['ddns']['config'][0]['enable'];
	if($hoba){
	// Is DDNS.sh already added as a conjob
        $task_key = '/usr/local/pkg/Fit123/ddns/DDNS.sh';
        $x_name='';
        $x=0;
        foreach($config['cron']['item'] as $item) {
            if($item['command']==$task_key) {
               $x_name = $x;
            }
            $x++;
        }
        unset($x);
        if(!$x_name > 0)
	{
	// Add cron job DDNS.sh to config.xml
        $cron_item = array();
        $cron_item['minute']    = "*/5";
        $cron_item['hour']      = "*";
        $cron_item['mday']      = "*";
        $cron_item['month']     = "*";
        $cron_item['wday']      = "*";
        $cron_item['who']       = "root";
        $cron_item['command']   = "/usr/local/pkg/Fit123/ddns/DDNS.sh";
        $config['cron']['item'][] = $cron_item;
        write_config();
        configure_cron();
	}
	}
	else
	{
	// Remove DDNS.sh cron entry from config.xml
        $task_key = '/usr/local/pkg/Fit123/ddns/DDNS.sh';
        $x_name='';
        $x=0;
        foreach($config['cron']['item'] as $item) {
            if($item['command']==$task_key) {
               $x_name = $x;
            }
            $x++;
        }
        unset($x);
	if($x_name > 0)
        {
            unset($config['cron']['item'][$x_name]);
            write_config();
        }
        configure_cron();
	}

// Created DDNS updater file ddns.php

	//Get user entry from config.xml 
	$line1text = $config['installedpackages']['ddns']['config'][0]['line1'];
	$line2text = $config['installedpackages']['ddns']['config'][0]['line2'];
	$line3text = $config['installedpackages']['ddns']['config'][0]['line3'];
	
	// Open ddns.php for writing
	$hf = fopen("/usr/local/pkg/Fit123/ddns/ddns.php","w");
	if(!$hf) {
	  log_error("could not open /usr/local/pkg/Fit123/ddns/ddns.php for writing");
	exit;
	}

	// The start of the config file
	fwrite($hf, '<?php');
	fwrite($hf, "\n");
	fwrite($hf, '$ch = curl_init();');
	fwrite($hf, "\n");
	fwrite($hf, 'curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);');
	fwrite($hf, "\n");
	// add to config file
	if ($line1text != ""){
	fwrite($hf, 'curl_setopt($ch, CURLOPT_URL, "');
	fwrite($hf, $line1text);
	fwrite($hf, '");');
	fwrite($hf, "\n");
	fwrite($hf, '$result = curl_exec($ch);');
	fwrite($hf, "\n");
	}
	if ($line2text != ""){
	fwrite($hf, 'curl_setopt($ch, CURLOPT_URL, "');
	fwrite($hf, $line2text);
	fwrite($hf, '");');
	fwrite($hf, "\n");
	fwrite($hf, '$result = curl_exec($ch);');
	fwrite($hf, "\n");
	}
	if ($line3text != ""){
	fwrite($hf, 'curl_setopt($ch, CURLOPT_URL, "');
	fwrite($hf, $line3text);
	fwrite($hf, '");');
	fwrite($hf, "\n");
	fwrite($hf, '$result = curl_exec($ch);');
	fwrite($hf, "\n");
	}
	// The end of the config file
	fwrite($hf, 'curl_close ($ch);');
	fwrite($hf, "\n");
	fwrite($hf, '?>');
	fwrite($hf, "\n");
	fclose($hf);

// Create local shell script to watch for wan ip change

	// Get WAN nic name from config.xml 
	$wannic = $config['interfaces']['wan']['if'];
	
	//Open DDNS.sh for writing	
	$hfd = fopen("/usr/local/pkg/Fit123/ddns/DDNS.sh","w");
	if(!$hfd) {
	  log_error("could not open /usr/local/pkg/Fit123/ddns/DDNS.sh for writing");
	exit;
	}
	// The start of the config file
	fwrite($hfd, '#!/bin/sh');
	fwrite($hfd, "\n");
	fwrite($hfd, 'ext_if="');
	fwrite($hfd, $wannic);
	fwrite($hfd, '"');
	fwrite($hfd, "\n");
	fwrite($hfd, 'dns_file="/usr/local/pkg/Fit123/ddns/DDNS.ip"');
	fwrite($hfd, "\n");
	fwrite($hfd, 'update_file="/usr/local/bin/php /usr/local/pkg/Fit123/ddns/ddns.php"');
	fwrite($hfd, "\n");
	fwrite($hfd, 'EXIT_SUCCESS=0');
	fwrite($hfd, "\n");
	fwrite($hfd, 'EXIT_FAILURE=1');
	fwrite($hfd, "\n");
	fwrite($hfd, 'if [ `id -u` -ne 0 ]');
	fwrite($hfd, "\n");
	fwrite($hfd, 'then');
	fwrite($hfd, "\n");
	fwrite($hfd, '        echo "Only root may run this program."');
	fwrite($hfd, "\n");
	fwrite($hfd, '        exit $EXIT_FAILURE');
	fwrite($hfd, "\n");
	fwrite($hfd, 'fi');
	fwrite($hfd, "\n");
	fwrite($hfd, 'get_ip(){');
	fwrite($hfd, "\n");
	fwrite($hfd, '        if [ -f $dns_file ]');
	fwrite($hfd, "\n");
	fwrite($hfd, '        then');
	fwrite($hfd, "\n");
	fwrite($hfd, '                registered_ip=`cat ${dns_file}`');
	fwrite($hfd, "\n");
	fwrite($hfd, '        else');
	fwrite($hfd, "\n");
	fwrite($hfd, '                registered_ip=""');
	fwrite($hfd, "\n");
	fwrite($hfd, '        fi');
	fwrite($hfd, "\n");
	fwrite($hfd, '        current_ip=`ifconfig ${ext_if} | awk ');
	fwrite($hfd, "'");
	fwrite($hfd, '/inet / { print $2 }');
	fwrite($hfd, "'");
	fwrite($hfd, '`');
	fwrite($hfd, "\n");
	fwrite($hfd, '}');
	fwrite($hfd, "\n");
	fwrite($hfd, 'update_hosts(){');
	fwrite($hfd, "\n");
	fwrite($hfd, '        if [ "$registered_ip" != "$current_ip" ]');
	fwrite($hfd, "\n");
	fwrite($hfd, '        then');
	fwrite($hfd, "\n");
	fwrite($hfd, '                $update_file');
	fwrite($hfd, "\n");
	fwrite($hfd, '                echo $current_ip > $dns_file');
	fwrite($hfd, "\n");
	fwrite($hfd, '                echo "Your DDNS records have been updated" | logger');
	fwrite($hfd, "\n");
	fwrite($hfd, '        fi');
	fwrite($hfd, "\n");
	fwrite($hfd, '}');
	fwrite($hfd, "\n");
	fwrite($hfd, 'get_ip');
	fwrite($hfd, "\n");
	fwrite($hfd, 'update_hosts');
	fwrite($hfd, "\n");
	fwrite($hfd, 'exit $EXIT_SUCCESS');
	fwrite($hfd, "\n");
	fclose($hfd);

	exec("chmod 744 /usr/local/pkg/Fit123/ddns/DDNS.sh");
}
?>
