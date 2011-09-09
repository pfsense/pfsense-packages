<?php
/*
	postfix.inc
	part of the Postfix package for pfSense
	Copyright (C) 2010 Erik Fonnesbeck
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
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("globals.inc");

function px_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function px_get_real_interface_address($iface) {
	global $config;
	$iface = convert_friendly_interface_to_real_interface_name($iface);
	$line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
	list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);
	return array($ip, long2ip(hexdec($netmask)));
}

function sync_relay_recipients($via_cron="cron"){
	global $config;
	#relay recipients
	if ($config['installedpackages']['postfixrecipients']['config']) {
		$relay_recipients="";
		$relay_ldap_recipients="";
		$ad_export="/usr/local/etc/postfix/adexport.pl";
		foreach ($config['installedpackages']['postfixrecipients']['config'] as $postfix_recipients_config) {
			if($postfix_recipients_config['location'] && file_exists($postfix_recipients_config['location']))
				$relay_recipients .= file_get_contents($postfix_recipients_config['location']);
			if($postfix_recipients_config['custom_recipients'])
				$relay_recipients .= px_text_area_decode($postfix_recipients_config['custom_recipients']);
			if($postfix_recipients_config['enable_ldap']){
				#validate cront job
				if(preg_match("/(\d+)(\w)/",$postfix_recipients_config['freq'],$matches)){
					$cron_sufix="\t*\t*\troot\t/usr/local/bin/php /usr/local/www/postfix_recipientes.php";
					switch ($matches[2]){
						case m:
							$cron= "*/".$matches[1]."\t*\t*".$cron_sufix;
							break;
						case h:
							$cron= "0\t*/".$matches[1]."\t*".$cron_sufix;
							break;
						case d:
							$cron= "0\t0\t*/".$matches[1].$cron_sufix;
							break;
						default:
							$input_errors[] = "A valid number with a time reference is required for the field 'Frequency'";
					}
					#update cront job file
					$crontab = file('/etc/crontab');
					foreach ($crontab as $line)
						$new_cron.=(preg_match("/postfix_recipientes.php/",$line)?$cron."\n":$line);
					#include if conf does not exist in crontab
					$new_cron.=(!preg_match("/postfix_recipientes.php/",$new_cron)?"\n".$cron."\n\n":"");	
					file_put_contents("/etc/crontab",$new_cron, LOCK_EX);
					#check crontab changes
					$md5_new_file = trim(md5_file('/etc/crontab'));
					if(file_exists('/etc/crontab.md5'))
						$md5_old_file = trim(file_get_contents('/etc/crontab.md5'));
					if($md5_new_file <> $md5_old_file){
						mwexec('/usr/bin/killall -HUP cron');
						file_put_contents("/etc/crontab.md5",$md5_new_file, LOCK_EX);
					}
				}
				$relay_ldap_recipients="";
				if ($via_cron == "gui"){
					#running via pfsense gui, not time for ldap fetch.
					$ldap_recipients='/usr/local/etc/postfix/relay_ldap_recipients.txt';
					if (!file_exists($ldap_recipients))
						system('/usr/bin/touch '. $ldap_recipients);
					$relay_ldap_recipients=file_get_contents($ldap_recipients);
					}	
				else{				
					#running via crontab, time to get ldap content.
					$ldap_temp=array();
					foreach ($postfix_recipients_config['row'] as $postfix_ldap) {
						print "extracting from ".$postfix_ldap['dc']."...";
						$filename="/usr/local/etc/postfix/relay_ldap_recipients.".$postfix_ldap['dc'].".txt";
						exec($ad_export." ".$postfix_ldap['dc']." ".$postfix_ldap['cn']." ".$postfix_ldap['username']." ".$postfix_ldap['password'],$ldap_fetch,$status);
						if ($status == 0){
							#write backup conf for ldap server
							$fp=fopen($filename,"w+");
							foreach($ldap_fetch as $key => $value)
									fwrite($fp,$value."\n");
							fclose($fp);
							}
						else{
							if (file_exists($filename)) {
								#LDAP fetch failed...read backup file.
								print "Restoring backup file for ".$postfix_ldap['dc']."...";
								$ldap_fetch=file($filename);
								}
							else{
								#we never got any info from this server.
								print "There is no backup file for ".$postfix_ldap['dc']."...";
								$ldap_fetch=array();
								}
							}
						$ldap_all = array_merge($ldap_temp,$ldap_fetch);
						$ldap_temp=$ldap_all;
						print "(".count($ldap_fetch).")\n";
						$ldap_fetch=array();
						}
						$ldap_unique=array_unique($ldap_all);
						print "Total ldap recipients:".count($ldap_all)."\tunique:".count($ldap_unique)."\n";
						foreach($ldap_unique as $recipient)
							$relay_ldap_recipients.=($recipient != ""?$recipient." OK\n":"");
						
					#save ldap relay recipients
					file_put_contents("/usr/local/etc/postfix/relay_ldap_recipients.txt",$relay_ldap_recipients, LOCK_EX);
					}
				}
		}
	#save all relay recipients and reload postfix
	file_put_contents("/usr/local/etc/postfix/relay_recipientes",$relay_ldap_recipients."\n".$relay_recipients, LOCK_EX);
	exec("/usr/local/sbin/postmap /usr/local/etc/postfix/relay_recipientes");
	mwexec("/usr/local/sbin/postfix reload");
	}
	if($relay_recipients !="" || $relay_ldap_recipients!="")
		return("relay_recipient_maps = hash:/usr/local/etc/postfix/relay_recipientes\n");
}
function sync_package_postfix() {
	global $config;
	$relay_domains = "";
	$transport = "";
	$postfix_config=$config['installedpackages']['postfix']['config'][0];
	$message_size_limit=($postfix_config['message_size_limit']?$postfix_config['message_size_limit']:"10240000");
	$process_limit=($postfix_config['process_limit']?$postfix_config['process_limit']:"100");
	if (is_array($postfix_config['row'])) {
		foreach ($postfix_config['row'] as $postfix_row) {
			$relay_domains .= ' ' . $postfix_row['domain'];
			if (!empty($postfix_row['mailserverip']))
				$transport .= $postfix_row['domain'] . " smtp:[" . $postfix_row['mailserverip'] . "]\n";
				}
			}
	#check logging
	if ($postfix_config['log_to']){
			switch($postfix_config['log_to']){
				case 'maillog':
					system("/usr/bin/touch /var/log/maillog");
					$mail_syslog="mail.crit;";
				break;
				case 'none':
					$mail_syslog="mail.crit;";
				break;
				default:
					$mail_syslog='mail.*;';
				break;
			}
            #update /etc/inc/system.inc
            $sys_log_file='/etc/inc/system.inc';
            $sys_log = file($sys_log_file);
            $new_sys_log="";
            $found_mail=0;
            foreach ($sys_log as $line){
            	$new_line=preg_replace('/mail.(.|crit);/',$mail_syslog,$line);
            	#set syslog entry mail.* %/var/log/maillog when log_to = system
            	if (preg_match ('/mail.(.|crit);/',$line) && $postfix_config['log_to'] =="maillog")
            		$new_sys_log .= 'mail.*'."\t\t\t\t\t\t".'/var/log/maillog'."\n";
            	#remove syslog entry mail.* %/var/log/maillog when log_to != system
            	if (preg_match ("/^mail/",$line))
            		$new_sys_log .="";
            	else	
            		$new_sys_log .= $new_line;
            }
            if (!file_exists('/root/system.inc.backup')) {
            	copy ($sys_log_file,'/root/system.inc.backup');
            }
            file_put_contents($sys_log_file,$new_sys_log, LOCK_EX);
            #mwexec('/usr/local/bin/php -q /usr/local/www/postfix_syslog.php');
            #restart syslog daemon	
            system_syslogd_start();
		}
            
				/*
            	#insert new syslog definition
                if (preg_match("/.*mail.crit.(.*)/",$line,$matches)){
                	if ($postfix_config['log_to'] == "/var/log/system.log"){
                		system("/usr/bin/touch /var/log/maillog");
                    	$new_sys_log .= $postfix_log.$matches[1]."\n".$line;
                		}
                	else 
                	{$new_sys_log .= $postfix_log.$postfix_log_sufix."\n".$line;}
                }
                else{
                	#remove previous syslog definition
                    $new_sys_log .= (preg_match("/mail.(info|debug|log)/",$line)?"":$line);
                    }
                }
                file_put_contents($sys_log_file,$new_sys_log, LOCK_EX);
                
		}

                #update /var/etc/syslog.conf
		$sys_log_file="/var/etc/syslog.conf";
		$sys_log = file($sys_log_file);
                $postfix_log .= $postfix_log_sufix;
                $new_sys_log="";
		foreach ($sys_log as $line)
			$new_sys_log.=(preg_match("/mail.(info|debug|log)/",$line)?$postfix_log."\n":$line);
		#include if conf does not exist in crontab
		$new_sys_log.=(!preg_match("/mail.(info|debug|log)/",$new_sys_log)?"\n".$postfix_log."\n\n":"");	
		file_put_contents($sys_log_file,$new_sys_log, LOCK_EX);
		#check crontab changes
		$md5_new_file = trim(md5_file($sys_log_file));
		$md5_old_file = trim(file_get_contents($sys_log_file.'.md5'));
		if($md5_new_file <> $md5_old_file){
			mwexec('/usr/bin/killall -HUP syslogd');
			file_put_contents($sys_log_file.'.md5',$md5_new_file, LOCK_EX);
		}
		*/
	#}

	#check_debug
	if($postfix_config['debug_list'] && $postfix_config['debug_list']!=""){
	$check_debug ="\n#Debugging postfix\n";
	$check_debug.="debug_peer_list = ".px_text_area_decode($postfix_config['debug_list'])."\n";
	$check_debug.="debug_peer_level = ".$postfix_config['debug_level']."\n\n";
	}
	#check relay recipients
	$all_relay_recipients=sync_relay_recipients('gui');
	
	$copyright=<<<ABOUT
#Part of the Postfix package for pfSense
#Copyright (C) 2010 Erik Fonnesbeck
#Copyright (C) 2011 Marcello Coutinho
#All rights reserved.
#DO NOT EDIT THIS FILE


ABOUT;
	$postfix_main="#main.cf\n".$copyright;
	#Header Maps
	if ($config['installedpackages']['postfixacl']['config'][0]['header_maps']){
		$postfix_main .= "header_checks = regexp:/usr/local/etc/postfix/header_check\n";
		$header_check = px_text_area_decode($config['installedpackages']['postfixacl']['config'][0]['header_maps']);
		}
	#MIME Maps
	if ($config['installedpackages']['postfixacl']['config'][0]['mime_maps']){
		$postfix_main .= "mime_header_checks = regexp:/usr/local/etc/postfix/mime_check\n";
		$mime_check = px_text_area_decode($config['installedpackages']['postfixacl']['config'][0]['mime_maps']);
		}
	#Body Maps
	if ($config['installedpackages']['postfixacl']['config'][0]['body_maps']){
		$postfix_main .= "body_checks = regexp:/usr/local/etc/postfix/body_check\n";
		$body_check = px_text_area_decode($config['installedpackages']['postfixacl']['config'][0]['body_maps']);
		}
	#Client CIDR
	if ($config['installedpackages']['postfixacl']['config'][0]['cal_cidr']){
		if ($antispam['zombie_blocker']=='disabled')
			$cal_cidr = px_text_area_decode($config['installedpackages']['postfixacl']['config'][0]['cal_cidr']);
		else
			#formatar o arquivo retirando os 'oks'
			$cal_cidr_tmp = px_text_area_decode($config['installedpackages']['postfixacl']['config'][0]['cal_cidr']); 	
			$cal_cidr = preg_replace('/ ok/i'," permit",$cal_cidr_tmp);
		}
	#Client REGEXP
	if ($config['installedpackages']['postfixacl']['config'][0]['cal_regexp']){
		$cal_regexp = px_text_area_decode($config['installedpackages']['postfixacl']['config'][0]['cal_regexp']);
		}
	$postfix_main .= px_text_area_decode($postfix_config['maincf'])."\n". 
		"relay_domains ={$relay_domains}\n" .
		"transport_maps = hash:/usr/local/etc/postfix/transport\n" .
		"local_recipient_maps =\n" .
		$all_relay_recipients.
		"mydestination =\n" .
		"mynetworks_style = host\n" .
		"message_size_limit = {$message_size_limit}\n" .
		"default_process_limit = {$process_limit}\n";
	#assign antispam options
	$antispam=$config['installedpackages']['postfixantispam']['config'][0];
	
	if($antispam['antispam_enabled']){
		switch ($antispam['antispam_software']){
			case "mailscanner":
			$header_check .= (!preg_match("/^Received:/",$header_check)?"\n/^Received:/ HOLD\n":"");
			$postfix_main_antispam = "#Saving all mail after header/body/rbl/spf checks to mailscanner\n\n";
			break;
			case "policyd2":
			if ($antispam['antispam_location']){
			$postfix_main_antispam = <<<EOF
#using policyd v2
client_throttle = check_policy_service {$antispam['antispam_location']}
smtpd_client_restrictions = check_policy_service {$antispam['antispam_location']}
smtpd_restriction_classes =
		has_our_domain_as_sender
		client_throttle
smtpd_end_of_data_restrictions = check_policy_service {$antispam['antispam_location']}	


EOF;
			}
			else{
			$postfix_main_antispam = "Policyd v2 has no location set.\n\n";	
			}
			break;
		}	
	}
	
	if ($antispam['header_check'] == "strong")
	{
	$postfix_main .= <<<EOF
disable_vrfy_command = yes
strict_rfc821_envelopes = yes

#Just reject after helo,sender,client,recipient tests
smtpd_delay_reject = yes

# Don't talk to mail systems that don't know their own hostname.
smtpd_helo_required = yes
smtpd_helo_restrictions = reject_unknown_helo_hostname

smtpd_sender_restrictions = reject_non_fqdn_sender,
				reject_unknown_sender_domain,
				reject_unauth_pipelining,
				reject_multi_recipient_bounce,
				permit

# Allow connections from specified local clients and strong check everybody else.								
smtpd_client_restrictions = check_client_access regexp:/usr/local/etc/postfix/cal_regexp,
				check_client_access cidr:/usr/local/etc/postfix/cal_cidr,
				reject_unknown_client_hostname,
				reject_unauth_pipelining,
				reject_multi_recipient_bounce,
				permit							

smtpd_recipient_restrictions = reject_invalid_helo_hostname,
				reject_unknown_recipient_domain,
				reject_non_fqdn_helo_hostname,
				reject_non_fqdn_recipient,
				reject_unauth_destination,
				reject_unauth_pipelining,
				reject_multi_recipient_bounce,
				SPFSPFSPFRBLRBLRBL

EOF;
	}
else
	{
	#erro nas listas de bloqueio
	$postfix_main .= <<<EOF
#Just reject after helo,sender,client,recipient tests
smtpd_delay_reject = yes
	
# Don't talk to mail systems that don't know their own hostname.
smtpd_helo_required = yes
smtpd_helo_restrictions = reject_unknown_helo_hostname

smtpd_sender_restrictions = reject_unknown_sender_domain,
				RBLRBLRBL							

# Allow connections from specified local clients and rbl check everybody else if rbl check are set.
smtpd_client_restrictions = check_client_access regexp:/usr/local/etc/postfix/cal_regexp,
				check_client_access cidr:/usr/local/etc/postfix/cal_cidr,
				RBLRBLRBL

# Whitelisting: local clients may specify any destination domain.
smtpd_recipient_restrictions = reject_unauth_destination,
				SPFSPFSPFRBLRBLRBL

EOF;
	}
#check spf option
$spf=($antispam['postfix_spf']?"check_policy_service unix:private/spf,\n\t\t\t\t":"");
$postfix_main=preg_replace("/SPFSPFSPF/",$spf,$postfix_main);
$postfix_main .= $postfix_main_antispam.$check_debug;
switch ($antispam['zombie_blocker'])
		{
		case "enforce":
		case "drop":
		case "ignore":
		$postscreen=1;
		break;
		
		case "disabled":
		$postscreen=0;
		break;
		}
	if ($antispam['soft_bounce'] == "enabled")
	{
	$postfix_main.="soft_bounce = yes\n";	
	}
	
	if ($postscreen==1)	#Postscreen enabled
	{
	if(preg_match("/(\d+),(\d+)(s|m|h|w)/",$antispam['greet_time'],$greet)){
			$postfix_main.='postscreen_greet_wait = ${stress?'.$greet[1].'}${stress:'.$greet[2].'}'.$greet[3]."\n";
			}
		$ag=$antispam['after_greeting'];
		if(preg_match("/postscreen_disable_vrfy_command/",$antispam['after_greeting'])){
			$postfix_main.="postscreen_disable_vrfy_command = yes\n";
			}
		if(preg_match("/postscreen_non_smtp_command_enable/",$antispam['after_greeting'])){
			$postfix_main.="postscreen_non_smtp_command_enable = yes\n";
			$postfix_main.="postscreen_non_smtp_command_action = ".$antispam['zombie_blocker']."\n";
			}
		if(preg_match("/postscreen_pipelining_enable/",$antispam['after_greeting'])){
			$postfix_main.="postscreen_pipelining_enable = yes\n";
			$postfix_main.="postscreen_pipelining_action = ".$antispam['zombie_blocker']."\n";
			}
		if(preg_match("/postscreen_bare_newline_enable/",$antispam['after_greeting'])){
			$postfix_main.="postscreen_bare_newline_enable = yes\n";
			$postfix_main.="postscreen_bare_newline_action = ".$antispam['zombie_blocker']."\n";
			}
		if(preg_match("/postscreen_greet_check/",$antispam['after_greeting'])){
			$postfix_main.="postscreen_greet_action = ".$antispam['zombie_blocker']."\n";
			}
	
	$postfix_main.="postscreen_access_list = cidr:/usr/local/etc/postfix/cal_cidr\n";
	$postfix_main.="postscreen_dnsbl_action= ".$antispam['zombie_blocker']."\n";
	$postfix_main.="postscreen_blacklist_action= ".$antispam['zombie_blocker']."\n";

	#postscreen interface loop
	$ifaces = ($postfix_config['enabled_interface'] ? $postfix_config['enabled_interface'] : 'wan');
	$real_ifaces = array();
	$postfix_master="";
	foreach (explode(",", $ifaces) as $i => $iface) {
		$real_ifaces[] = px_get_real_interface_address($iface);
		if($real_ifaces[$i][0]) {
			$postfix_master .=$real_ifaces[$i][0].":25	inet  n       -       n       -       1       postscreen\n\t-o user=postfix\n";
			$postfix_master .=($antispam['soft_bounce'] == "postscreen"?"\t-o soft_bounce=yes\n":"");
		}
	}
	$postfix_master .= $postfix_inets.<<<MASTEREOF
smtpd     pass  -       -       n       -       -       smtpd
dnsblog   unix  -       -       n       -       0       dnsblog
tlsproxy  unix  -       -       n       -       0       tlsproxy

MASTEREOF;
	$rbl2="";
	if ($antispam['rbl_servers'] != "")
		{
		$postfix_main .= "postscreen_dnsbl_sites=" . $antispam['rbl_servers']."\n";
		$postfix_main .= "postscreen_dnsbl_threshold=" . $antispam['rbl_threshold']."\n";
		}
	}
	else
	{					#Postscreen disabled
	if ($antispam['rbl_servers'] != "")
		{
		$RBL = explode(",",$antispam['rbl_servers']);
		foreach ($RBL as $rbl)
			{
			$prefix=($rbl2 !=""?"\t\t\t\t":"");
			$rbl2.= $prefix."reject_rbl_client $rbl,\n";
			}
		}
	
	#interface loop
	$postfix_inets="";
	$ifaces = ($postfix_config['enabled_interface'] ? $postfix_config['enabled_interface'] : 'loopback');
	$real_ifaces = array();
	$postfix_master="";
	foreach (explode(",", $ifaces) as $i => $iface) {
		$real_ifaces[] = px_get_real_interface_address($iface);
		if($real_ifaces[$i][0]) {
			$postfix_master .=$real_ifaces[$i][0].":25	inet  n       -       n       -       1       smtpd\n\t-o user=postfix\n";
		}
	}
		
	}
	$rbl2.=($rbl2 !=""?"\t\t\t\tpermit\n":"permit\n");
	$postfix_main=preg_replace("/RBLRBLRBL/",$rbl2,$postfix_main);
	$postfix_master .= <<<MASTEREOF2
pickup    fifo  n       -       n       60      1       pickup
cleanup   unix  n       -       n       -       0       cleanup
qmgr      fifo  n       -       n       300     1       qmgr
tlsmgr    unix  -       -       n       1000?   1       tlsmgr
rewrite   unix  -       -       n       -       -       trivial-rewrite
bounce    unix  -       -       n       -       0       bounce
defer     unix  -       -       n       -       0       bounce
trace     unix  -       -       n       -       0       bounce
verify    unix  -       -       n       -       1       verify
flush     unix  n       -       n       1000?   0       flush
proxymap  unix  -       -       n       -       -       proxymap
proxywrite unix -       -       n       -       1       proxymap
smtp      unix  -       -       n       -       -       smtp
relay     unix  -       -       n       -       -       smtp
	-o smtp_fallback_relay=
showq     unix  n       -       n       -       -       showq
error     unix  -       -       n       -       -       error
retry     unix  -       -       n       -       -       error
discard   unix  -       -       n       -       -       discard
local     unix  -       n       n       -       -       local
virtual   unix  -       n       n       -       -       virtual
lmtp      unix  -       -       n       -       -       lmtp
anvil     unix  -       -       n       -       1       anvil
scache    unix  -       -       n       -       1       scache
spf    unix	-       n       n       -       -       spawn
	user=nobody argv=/usr/local/sbin/postfix-policyd-spf

MASTEREOF2;
	
	conf_mount_rw();

	log_error("Writing out configuration");
	file_put_contents("/usr/local/etc/postfix/main.cf", $postfix_main, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/master.cf", $postfix_master, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/transport", $transport, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/cal_cidr", $cal_cidr, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/cal_regexp", $cal_regexp, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/header_check", $header_check, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/mime_check", $mime_check, LOCK_EX);
	file_put_contents("/usr/local/etc/postfix/body_check", $body_check, LOCK_EX);
	$FILES=array("transport");
	foreach ($FILES as $file)
		{
		mwexec("/usr/local/sbin/postmap /usr/local/etc/postfix/".$file);
		}

	if (!is_dir("/etc/mail"))
		mkdir("/etc/mail", 0755);
	if (!file_exists("/etc/mail/aliases"))
		touch("/etc/mail/aliases");
	exec("/usr/local/bin/newaliases");
	postfix_start();
	postfix_sync_on_changes();
}
function postfix_start(){
	global $config;
	$start = "/usr/local/sbin/postfix start\n";
	$stop = "/usr/local/sbin/postfix stop\n";
	log_error("Writing rc_file");		
	write_rcfile(array("file" => "postfix.sh", "start" => $start, "stop" => $stop));

	conf_mount_ro();

	sleep(1);
	if ($config['installedpackages']['postfix']['config'][0]['enable_postfix']){
		log_error("Reloading/starting postfix");
		system('/bin/chmod +x /usr/local/etc/rc.d/postfix.sh');
		mwexec_bg("/usr/local/sbin/postfix reload || /usr/local/sbin/postfix start");
		log_error("Postfix setup completed");
		}
	else{
		log_error("Stopping postfix");
		mwexec("/usr/local/etc/rc.d/postfix.sh stop");
		system('/bin/chmod -x /usr/local/etc/rc.d/postfix.sh');
		}
}

function postfix_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (empty($value))
			continue;
		if($key == "greet_time" && !preg_match("/(\d+),(\d+)(s|m|h|w)/",$value))
				$input_errors[] = "Wrong greet time sintax.";			
		if($key == "message_size_limit" && !is_numeric($value))
				$input_errors[] = "Message size limit must be numeric.";
		if($key == "process_limit" && !is_numeric($value))
				$input_errors[] = "Process limit must be numeric.";	
		if($key == "freq" && (!preg_match("/^\d+(h|m|d)$/",$value) || $value == 0))
				$input_errors[] = "A valid number with a time reference is required for the field 'Frequency'";
		if (substr($key, 0, 2) == "dc" && !is_hostname($value))
				$input_errors[] = "{$value} is not a valid host name.";
		if (substr($key, 0, 6) == "domain" && is_numeric(substr($key, 6))) {
			if (!is_domain($value))
				$input_errors[] = "{$value} is not a valid domain name.";
		} else if (substr($key, 0, 12) == "mailserverip" && is_numeric(substr($key, 12))) {
			if (empty($post['domain' . substr($key, 12)]))
				$input_errors[] = "Domain for {$value} cannot be blank.";
			if (!is_ipaddr($value) && !is_hostname($value))
				$input_errors[] = "{$value} is not a valid IP address or host name.";
		}
	}
}

function postfix_php_install_command() {
	#small freebsd packages for full functional ldap and spf options
	system('/usr/sbin/pkg_add -r postfix-policyd-spf');
	#system('/usr/sbin/pkg_add -r p5-perl-ldap');
	sync_package_postfix();
}

function postfix_php_deinstall_command() {
	mwexec("/usr/local/etc/rc.d/postfix.sh stop");
	sleep(1);
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/postfix.sh");
	conf_mount_ro();
}

/* Uses XMLRPC to synchronize the changes to a remote node */
function postfix_sync_on_changes() {
	global $config, $g;
	log_error("[postfix] postfix_xmlrpc_sync.php is starting.");
	$synconchanges = $config['installedpackages']['postfixsync']['config'][0]['synconchanges'];	
	if(!$synconchanges) 
		return;
	foreach ($config['installedpackages']['postfixsync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		if($password && $sync_to_ip)
			postfix_do_xmlrpc_sync($sync_to_ip, $password);
		}
	}
	log_error("[postfix] postfix_xmlrpc_sync.php is ending.");
}

/* Do the actual XMLRPC sync */
function postfix_do_xmlrpc_sync($sync_to_ip, $password) {
	global $config, $g;

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

	$xmlrpc_sync_neighbor = $sync_to_ip;
    if($config['system']['webgui']['protocol'] != "") {
		$synchronizetoip = $config['system']['webgui']['protocol'];
		$synchronizetoip .= "://";
    }
    $port = $config['system']['webgui']['port'];
    /* if port is empty lets rely on the protocol selection */
    if($port == "") {
		if($config['system']['webgui']['protocol'] == "http") 
			$port = "80";
		else 
			$port = "443";
    }
	$synchronizetoip .= $sync_to_ip;

	/* xml will hold the sections to sync */
	$xml = array();
	$xml['postfix'] = $config['installedpackages']['postfix'];
	$xml['postfixacl'] = $config['installedpackages']['postfixacl'];
	$xml['postfixrecipients'] = $config['installedpackages']['postfixrecipients'];
	$xml['postfixantispam'] = $config['installedpackages']['postfixantispam'];
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($xml)
	);

	/* set a few variables needed for sync code borrowed from filter.inc */
	$url = $synchronizetoip;
	log_error("Beginning Postfix XMLRPC sync to {$url}:{$port}.");
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	if($g['debug'])
		$cli->setDebug(1);
	/* send our XMLRPC message and timeout after 250 seconds */
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting postfix XMLRPC sync with {$url}:{$port}.";
		log_error($error);
		file_notice("sync_settings", $error, "Postfix Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting postfix XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "Postfix Settings Sync", "");
	} else {
		log_error("Postfix XMLRPC sync successfully completed with {$url}:{$port}.");
	}
	
	/* tell postfix to reload our settings on the destionation sync host. */
	$method = 'pfsense.exec_php';
	$execcmd  = "require_once('/usr/local/pkg/postfix.inc');\n";
	$execcmd .= "sync_package_postfix();";
	
	/* assemble xmlrpc payload */
	$params = array(
		XML_RPC_encode($password),
		XML_RPC_encode($execcmd)
	);

	log_error("postfix XMLRPC reload data {$url}:{$port}.");
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials('admin', $password);
	$resp = $cli->send($msg, "250");
	if(!$resp) {
		$error = "A communications error occurred while attempting postfix XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error($error);
		file_notice("sync_settings", $error, "postfix Settings Sync", "");
	} elseif($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, "250");
		$error = "An error code was received while attempting postfix XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error($error);
		file_notice("sync_settings", $error, "postfix Settings Sync", "");
	} else {
		log_error("postfix XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}

}

?>
