<?php
/*
	postfix.inc
	part of the Postfix package for pfSense
	Copyright (C) 2011 Marcello Coutinho
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
require("globals.inc");
#require("guiconfig.inc");


function ms_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function sync_package_mailscanner() {
	global $config;

	#assign xml arrays
	if (is_array($config['installedpackages']['mailscanner']))
		$mailscanner=$config['installedpackages']['mailscanner']['config'][0];
	if (is_array($config['installedpackages']['msattachments']))
		$attachments=$config['installedpackages']['msattachments']['config'][0];
	if (is_array($config['installedpackages']['msantivirus']))
		$antivirus=$config['installedpackages']['msantivirus']['config'][0];
	if (is_array($config['installedpackages']['mscontent']))
		$content=$config['installedpackages']['mscontent']['config'][0];
	if (is_array($config['installedpackages']['msreport']))
		$report=$config['installedpackages']['msreport']['config'][0];
	if (is_array($config['installedpackages']['msantispam']))
		$antispam=$config['installedpackages']['msantispam']['config'][0];
	if (is_array($config['installedpackages']['msalerts']))
		$alert=$config['installedpackages']['msalerts']['config'][0];
		
	#General options
	$info  =($mailscanner['orgname']?'%org-name% = '.$mailscanner['orgname']."\n":'%org-name% = Pfsense'."\n");
	$info .=($mailscanner['longorgname']?'%org-long-name% = '.$mailscanner['longorgname']."\n":'%org-long-name% = Pfsense Inc.'."\n");
	$info .=($mailscanner['website']?'%web-site% = '.$mailscanner['website']."\n":'%web-site% = www.pfsense.com'."\n");
	$max_children =($mailscanner['max_children']?$mailscanner['max_children']:'5');
	$scan_messages=(preg_match('/ScanMessages/',$mailscanner['pim'])?"yes":"no");
	$reject_message=(preg_match('/RejectMessage/',$mailscanner['pim'])?"yes":"no");
	$default_rule_multiple=(preg_match('/UseDefaultRulesWithMultipleRecipients/',$mailscanner['advanced'])?"yes":"no");
	$read_ipaddress=(preg_match('/ReadIPAddressFromReceivedHeader/',$mailscanner['advanced'])?"yes":"no");
	$spam_score_format=($mailscanner['spam_score_format']?$mailscanner['spam_score_format']:'%d');
	$cache_timings=($mailscanner['cache_timings']?$mailscanner['cache_timings']:'1800,300,10800,172800,600');
	$debug_spam=(preg_match('/DebugSpamAssassin/',$mailscanner['advanced'])?"yes":"no");
	$debug=(preg_match('/DebugMailScanner/',$mailscanner['advanced'])?"yes":"no");
	$foreground=(preg_match('/RunInForeground/',$mailscanner['advanced'])?"yes":"no");
	$look_up_last=(preg_match('/AlwaysLookedUpLast,/',$mailscanner['advanced'])?"yes":"no");
	$look_up_last_batch=(preg_match('/AlwaysLookedUpLastAfterBatch/',$mailscanner['advanced'])?"yes":"no");
	$deliver_background=(preg_match('/DeliverInBackground/',$mailscanner['advanced'])?"yes":"no");
	$split_exim_spool=(preg_match('/Split Exim Spool/',$mailscanner['advanced'])?"yes":"no");
	$syntax_check=(preg_match('/AutomaticSyntaxCheck/',$mailscanner['advanced'])?"yes":"no");
	#logging
	$syslog_facility=($mailscanner['syslog_facility']?$mailscanner['syslog_facility']:'mail');
	$log_speed=(preg_match('/LogSpeed/',$mailscanner['syslog'])?"yes":"no");
	$log_spam=(preg_match('/LogSpam/',$mailscanner['syslog'])?"yes":"no");
	$log_non_spam=(preg_match('/LogNonSpam/',$mailscanner['syslog'])?"yes":"no");
	$log_delivery=(preg_match('/LogDeliveryAndNon-Delivery/',$mailscanner['syslog'])?"yes":"no");
	$log_filenames=(preg_match('/LogPermittedFilenames/',$mailscanner['syslog'])?"yes":"no");
	$log_filetypes=(preg_match('/LogPermittedFiletypes/',$mailscanner['syslog'])?"yes":"no");
	$log_mime=(preg_match('/LogPermittedFileMIME/',$mailscanner['syslog'])?"yes":"no");
	$log_silent=(preg_match('/LogSilent/',$mailscanner['syslog'])?"yes":"no");
	$log_dangerous=(preg_match('/LogDangerousHTML/',$mailscanner['syslog'])?"yes":"no");
	$log_sa_rule_action=(preg_match('/LogSpamAssassinRule/',$mailscanner['syslog'])?"yes":"no");

	#Attachment options
	$max_size=($attachments['max_sizes']?$attachments['max_sizes']:'-1');
	$archive_depth=($attachments['archive_depth']?$attachments['archive_depth']:'8');	
	$expand_tnef=(preg_match('/ExpandTNEF/',$attachments['features'])?"yes":"no");
	$deliver_tnef=(preg_match('/DeliverUnparsableTNEF/',$attachments['features'])?"yes":"no");
	$find_archive=(preg_match('/FindArchiveByContent/',$attachments['features'])?"yes":"no");
	$microsoft=(preg_match('/UnpackMicrosoftDocuments/',$attachments['features'])?"yes":"no");
	$zip_attachments=(preg_match('/ZipAttachments/',$attachments['features'])?"yes":"no");
	$zip_file=($attachments['attachment_filename']?$attachments['attachment_filename']:'MessageAttachments.zip');
	$zip_exclude=($attachments['attachment_extension_exclude']?$attachments['attachment_extension_exclude']:".zip .rar .gz .tgz .jpg .jpeg .mpg .mpe .mpeg .mp3 .rpm .htm .html .eml");
	$unzip_max_per_archive=($attachments['attachment_max_per_archive']?$attachments['attachment_max_per_archive']:"0");
	$unzip_max=($attachments['attachment_max']?$attachments['attachment_max']:"50k");

	#Antivirus
	$virus_scanning=(preg_match('/VirusScanning/',$antivirus['features'])?"yes":"no");
	$deliver_disinfected=(preg_match('/DeliverDisinfectedFiles/',$antivirus['features'])?"yes":"no");
	$antivirus_timeout=($antivirus['timeout']?$antivirus['timeout']:"300");
	$silent_viruses=preg_replace("/,/"," ",$antivirus['silent_virus']);
	$deliver_silent=(preg_match('/StillDeliverSilentViruses/',$antivirus['features'])?"yes":"no");
	$spam_virus_header=($antivirus['virus_header']?$antivirus['virus_header']:'X-%org-name%-MailScanner-SpamVirus-Report:');
	$block_encrypted=(preg_match('/BlockEncryptedMessages/',$antivirus['features'])?"yes":"no");
	$block_unencrypted=(preg_match('/BlockUnencryptedMessages/',$antivirus['features'])?"yes":"no");
	$allow_password=(preg_match('/AllowPassword-ProtectedArchives/',$antivirus['features'])?"yes":"no");
	$check_filenames=(preg_match('/CheckFilenamesInPassword-ProtectedArchives/',$antivirus['features'])?"yes":"no");
	$custom_antivirus_options=ms_text_area_decode($antivirus['custom']);
	
	#content
	$dangerous_content=(preg_match('/DangerousContentScanning/',$content['checks'])?"yes":"no");
	$partial_messages=(preg_match('/AllowPartialMessages/',$content['checks'])?"yes":"no");
	$external_bodies=(preg_match('/AllowExternalMessageBodies/',$content['checks'])?"yes":"no");
	$phishing_fraud=(preg_match('/FindPhishingFraud/',$content['checks'])?"yes":"no");
	$numeric_phishig=(preg_match('/AlsoFindNumericPhishing/',$content['checks'])?"yes":"no");
	$stricter_phishing_net=(preg_match('/UseStricterPhishingNet/',$content['checks'])?"yes":"no");
	$highlight_phishing=(preg_match('/HighlightPhishingFraud/',$content['checks'])?"yes":"no");
	$dangerous_html=(preg_match('/ConvertDangerousHTMLToText/',$content['checks'])?"yes":"no");
	$html_to_text=(preg_match('/ConvertHTMLToText/',$content['checks'])?"yes":"no");
	
	#reports and responses
	$include_scanner_name=(preg_match('/IncludeScannerNameInReports/',$report['features'])?"yes":"no");
	$hide_incoming_work_dir=(preg_match('/HideIncomingWorkDir/',$report['features'])?"yes":"no");
	$quarantine_whole_message_as_queue=(preg_match('/QuarantineWholeMessagesAsQueueFiles/',$report['features'])?"yes":"no");
	$quarantine_whole_message=(preg_match('/QuarantineWholeMessage,/',$report['features'])?"yes":"no");
	$quarantine_modified_body=(preg_match('/QuarantineModifiedBody/',$report['features'])?"yes":"no");
	$quarantine_silent_virus=(preg_match('/QuarantineSilentViruses/',$report['features'])?"yes":"no");
	$quarantine_infections=(preg_match('/QuarantineInfections/',$report['features'])?"yes":"no");
	$keep_spam_and_mcp=(preg_match('/KeepSpamAndMCPArchiveClean/',$report['features'])?"yes":"no");
	$report_language=strtolower($report['language']);

	#notifications
	$notify_sender=(preg_match('/NotifySenders,/',$report['notification'])?"yes":"no");
	$notify_sender_viruses=(preg_match('/NotifySendersOfViruses/',$report['notification'])?"yes":"no");
	$notify_sender_fileytypes=(preg_match('/NotifySendersOfFilenames/',$report['notification'])?"yes":"no");
	$notify_sender_attachments=(preg_match('/NotifySendersSize/',$report['notification'])?"yes":"no");
	$notify_sender_contents=(preg_match('/NotifySendersOfOtherBlockedContent/',$report['notification'])?"yes":"no");

	#notices do system admin
	$send_notices=(preg_match('/SendNotices/',$report['system'])?"yes":"no");
	$notices_include_header=(preg_match('/NoticesIncludeFullHeaders/',$report['system'])?"yes":"no");
	$hide_incoming_work_dir_notices=(preg_match('/HideIncomingWorkDirinNotices/',$report['system'])?"yes":"no");
	$notice_from=($report['notice_from']?$report['notice_from']:"MailScanner");
	$notice_to=($report['notice_to']?$report['notice_to']:"MailScanner");
	$notice_signature=($report['notice_signature']?$report['notice_signature']:'-- \nMailScanner\nEmail Virus Scanner\nwww.mailscanner.info');
	
	#antispa- spam assassin
	$use_sa=(preg_match('/use_sa/',$antispam['safeatures'])?"yes":"no");
	$sa_auto_whitelist=(preg_match('/sa_auto_whitelist/',$antispam['safeatures'])?"yes":"no");
	$check_sa_if_on_spam_list=(preg_match('/check_sa_if_on_spam_list/',$antispam['safeatures'])?"yes":"no");
	$include_sa_bin_attachments=(preg_match('/include_sa_bin_attachments/',$antispam['safeatures'])?"yes":"no");
	$spam_score=(preg_match('/spam_score/',$antispam['safeatures'])?"yes":"no");
	$cache_spamassassin_results=(preg_match('/cache_spamassassin_results/',$antispam['safeatures'])?"yes":"no");
	$wait_during_bayes_rebuild=(preg_match('/wait_during_bayes_rebuild/',$antispam['safeatures'])?"yes":"no");
	$sa_max=($antispam['sa_max']?$antispam['sa_max']:"40k");
	$sa_score=($antispam['sa_score']?$antispam['sa_score']:"6");
	$hi_score=($antispam['hi_score']?$antispam['hi_score']:"20");
	$rebuild_bayes=($antispam['rebuild_bayes']?$antispam['rebuild_bayes']:"86400");
	$spam_actions=($antispam['spam_actions']?preg_replace("/,/"," ",$antispam['spam_actions']):"deliver");
	$hispam_actions=($antispam['hispam_actions']?preg_replace("/,/"," ",$antispam['hispam_actions']):"delete");
	$use_sa=(preg_match('/use_sa/',$antispam['safeatures'])?"yes":"no");
	#antispam - mcp
	$mcp_checks=(preg_match('/mcp_checks/',$antispam['mcp_features'])?"yes":"no");
	$bounce_mcp=(preg_match('/bounce_mcp/',$antispam['mcp_features'])?"yes":"no");
	$is_mcp=(preg_match('/is_mcp/',$antispam['mcp_features'])?"yes":"no");
	$is_not_mcp=(preg_match('/is_not_mcp/',$antispam['mcp_features'])?"yes":"no");
	$mcp_is_high_score=(preg_match('/mcp_is_high_score/',$antispam['mcp_features'])?"yes":"no");
	$include_mcp_report=(preg_match('/include_mcp_report/',$antispam['mcp_features'])?"yes":"no");
	$detailled_mcp_report=(preg_match('/detailled_mcp_report/',$antispam['mcp_features'])?"yes":"no");				
	$score_mcp_report=(preg_match('/score_mcp_report/',$antispam['mcp_features'])?"yes":"no");
	$log_mcp=(preg_match('/log_mcp/',$antispam['mcp_features'])?"yes":"no");
	$mcp_score=($antispam['mcp_score']?$antispam['mcp_score']:"1");
	$hi_mcp_score=($antispam['hi_mcp_score']?$antispam['hi_mcp_score']:"10");
	$mcp_action=($antispam['mcp_action']?preg_replace("/,/"," ",$antispam['mcp_action']):"deliver");
	$mcp_hi_action=($antispam['mcp_hi_action']?preg_replace("/,/"," ",$antispam['mcp_hi_action']):"delete");
	$mcp_max=($antispam['mcp_max']?$antispam['mcp_max']:"200k");
		
	/*
Language Strings = %report-dir%/languages.conf
*/
	#check files
	$load_samples=0;
	$mailscanner_dir="/usr/local/etc/MailScanner";
	
	if($attachments['filename_rules'] == ""){
		$config['installedpackages']['msattachments']['config'][0]['filename_rules']=base64_encode(file_get_contents($mailscanner_dir.'/archives.filename.rules.conf.sample'));
		$load_samples++;
	}
	if($attachments['filetype_rules'] == ""){
		$config['installedpackages']['msattachments']['config'][0]['filetype_rules']=base64_encode(file_get_contents($mailscanner_dir.'/archives.filetype.rules.conf.sample'));
		$load_samples++;
	}
	if($content['phishing_safe'] == ""){
		$config['installedpackages']['mscontent']['config'][0]['phishing_safe']=base64_encode(file_get_contents($mailscanner_dir.'/phishing.safe.sites.conf.sample'));
		$load_samples++;
	}
	if($content['phishing_bad'] == ""){
		$config['installedpackages']['mscontent']['config'][0]['phishing_bad']=base64_encode(file_get_contents($mailscanner_dir.'/phishing.bad.sites.conf.sample'));
		$load_samples++;
	}	
	if($content['country_domains'] == ""){
		$config['installedpackages']['mscontent']['config'][0]['country_domains']=base64_encode(file_get_contents($mailscanner_dir.'/country.domains.conf.sample'));
		$load_samples++;
	}
	if($antispam['sa_pref_file'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['sa_pref_file']=base64_encode(file_get_contents($mailscanner_dir.'/spam.assassin.prefs.conf.sample'));
		$load_samples++;
	}
	if($antispam['rbl_file'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['rbl_file']=base64_encode(file_get_contents($mailscanner_dir.'/spam.lists.conf.sample'));
		$load_samples++;
	}
	if($antispam['mcp_pref_file'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['mcp_pref_file']=base64_encode(file_get_contents($mailscanner_dir.'/mcp/mcp.spam.assassin.prefs.conf.sample'));
		copy($mailscanner_dir.'/mcp/10_example.cf.sample',$mailscanner_dir.'/mcp/10_example.cf');
		copy($mailscanner_dir.'/mcp/v320.pre.sample',$mailscanner_dir.'/mcp/v320.pre');
		$load_samples++;
	}
	if($antispam['bounce'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['bounce']=base64_encode(file_get_contents($mailscanner_dir.'/rules/bounce.rules.sample'));
		$load_samples++;
	}
	if($antispam['spam_whitelist'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['spam_whitelist']=base64_encode(file_get_contents($mailscanner_dir.'/rules/spam.whitelist.rules.sample'));
		$load_samples++;
	}
	if($antispam['max_message_size'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['max_message_size']=base64_encode(file_get_contents($mailscanner_dir.'/rules/max.message.size.rules.sample'));
		$load_samples++;
	}
	
	$report_dir="/usr/local/share/MailScanner/reports/".strtolower($report['language']);
	#CHECK REPORT FILES
	$report_files= array('deletedbadcontent' => 'deleted.content.message.txt',
						'deletedbadfilename' => 'deleted.filename.message.txt',
						'deletedvirus' =>'deleted.virus.message.txt',
						'deletedsize' => 'deleted.size.message.txt',
						'storedbadcontent' => 'stored.content.message.txt',
						'storedbadfilename' => 'stored.filename.message.txt',
						'storedvirus' => 'stored.virus.message.txt',
						'storedsize' =>  'stored.size.message.txt',
						'disinfected' => 'disinfected.report.txt',
						'sendercontent' => 'sender.content.report.txt',
						'sendererror' => 'sender.error.report.txt',
						'senderbadfilename' => 'sender.filename.report.txt',
						'sendervirus' => 'sender.virus.report.txt',
						'sendersize' => 'sender.size.report.txt',
						'senderrbl' => 'sender.spam.rbl.report.txt',
						'sendersa' => 'sender.spam.sa.report.txt',
						'sendermcp' => 'sender.mcp.report.txt',
						'senderspam'=>'sender.spam.report.txt',
						'recipientmcp'=>'recipient.mcp.report.txt',
						'recipientspam'=>'recipient.spam.report.txt',
						'rejection' =>'rejection.report.txt');
	
	foreach ($report_files as $key_r => $file_r){
		if ($report[$key_r] == ""){
			#$input_errors[]= $key;
			$config['installedpackages']['msreport']['config'][0][$key_r]=base64_encode(file_get_contents($report_dir.'/'.$file_r.'.sample'));
			file_put_contents($report_dir.'/'.$file_r,ms_text_area_decode($config['installedpackages']['msreport']['config'][0][$key_r]),LOCK_EX);
			$load_samples++;
			}
		#print $key_r ."X $file_r X". base64_encode(file_get_contents($report_dir.'/'.$file_r.'.sample')) ."<br>";
		
		if ($alert['sig']){
			if($alert['sig_html'] == ""){
				$config['installedpackages']['msalerts']['config'][0]['sig_html']=base64_encode(file_get_contents($report_dir.'/inline.sig.html'));
				$load_samples++;
				}
			if($alert['sig_txt'] == ""){
				$config['installedpackages']['msalerts']['config'][0]['sig_txt']=base64_encode(file_get_contents($report_dir.'/inline.sig.txt'));
				$load_samples++;
				}
			}

		if ($alert['warning']){
			if($alert['warning_html'] == ""){
				$config['installedpackages']['msalerts']['config'][0]['warning_html']=base64_encode(file_get_contents($report_dir.'/inline.warning.html'));
				$load_samples++;
				}
			if($alert['warning_txt'] == ""){
				$config['installedpackages']['msalerts']['config'][0]['warning_txt']=base64_encode(file_get_contents($report_dir.'/inline.warning.txt'));
				$load_samples++;
				}
			}
			
			
	}
	#exit;
	if($load_samples > 0)
		write_config();
	/*
Phishing Safe Sites File = %etc-dir%/phishing.safe.sites.conf
Phishing Bad Sites File = %etc-dir%/phishing.bad.sites.conf
Country Sub-Domains List = %etc-dir%/country.domains.conf
*/
	
	#create MailScanner.conf$deliver_silent
	$mc=<<<EOF
{$info}
# Configuration directory containing this file
%etc-dir% = /usr/local/etc/MailScanner

# Set the directory containing all the reports in the required language
%report-dir% = /usr/local/share/MailScanner/reports/{$report_language}

# Rulesets directory containing your ".rules" files
%rules-dir% = /usr/local/etc/MailScanner/rules

# Configuration directory containing files related to MCP
# (Message Content Protection)
%mcp-dir% = /usr/local/etc/MailScanner/mcp

#
# System settings
# ---------------
#
Max Children = {$max_children}	
Run As User = postfix
Run As Group = postfix
Queue Scan Interval = 6
Incoming Queue Dir = /var/spool/postfix/hold
Outgoing Queue Dir = /var/spool/postfix/incoming
Incoming Work Dir = /var/spool/MailScanner/incoming
Quarantine Dir = /var/spool/MailScanner/quarantine
PID file = /var/run/MailScanner.pid
Restart Every = 14400
MTA = postfix
Sendmail = /usr/local/sbin/sendmail

#
# Incoming Work Dir Settings
# --------------------------
#
Incoming Work User = postix
Incoming Work Group = postix
Incoming Work Permissions = 0600

#
# Quarantine and Archive Settings
# -------------------------------
#
Quarantine User = postifx
Quarantine Group = postfix
Quarantine Permissions = 0600

#
# Processing Incoming Mail
# ------------------------
#
Max Unscanned Bytes Per Scan = 100m
Max Unsafe Bytes Per Scan = 50m
Max Unscanned Messages Per Scan = 30
Max Unsafe Messages Per Scan = 30
Max Normal Queue Size = 800
Scan Messages =	{$scan_messages}
Reject Message = {$reject_message}
Maximum Processing Attempts = 10
Processing Attempts Database = /var/spool/MailScanner/incoming/Processing.db
Maximum Attachments Per Message = 200
Expand TNEF = {$expand_tnef}
Deliver Unparsable TNEF = {$deliver_tnef}
Use TNEF Contents = {$attachments['tnef_contents']}
TNEF Expander = /usr/local/bin/tnef --maxsize=100000000
TNEF Timeout = 120
File Command = /usr/bin/file
File Timeout = 20
Gunzip Command = /usr/bin/gunzip
Gunzip Timeout = 50
Unrar Command = /usr/local/bin/unrar
Unrar Timeout = 50
Find UU-Encoded Files = no
Maximum Message Size = %rules-dir%/max.message.size.rules
Maximum Attachment Size ={$max_size}
Minimum Attachment Size = -1
Maximum Archive Depth = {$archive_depth}
Find Archives By Content ={$find_archive}
Unpack Microsoft Documents = {$microsoft}
Zip Attachments = {$zip_attachments}
Attachments Zip Filename = {$zip_file} 
Attachments Min Total Size To Zip = 100k
Attachment Extensions Not To Zip = {$zip_exclude}
Add Text Of Doc = no
Antiword = /usr/bin/antiword -f
Antiword Timeout = 50
Unzip Maximum Files Per Archive = {$unzip_max_per_archive}
Unzip Maximum File Size = {$unzip_max}
Unzip Filenames = *.txt *.ini *.log *.csv
Unzip MimeType = text/plain

#
# Virus Scanning and Vulnerability Testing
# ----------------------------------------
#
Virus Scanning = {$virus_scanning}
Virus Scanners = {$antivirus['virus_scanner']}
Virus Scanner Timeout = {$antivirus_timeout}
Deliver Disinfected Files = {$deliver_disinfected}
Silent Viruses = {$silent_viruses}
Still Deliver Silent Viruses = {$deliver_silent}
Non-Forging Viruses = Joke/ OF97/ WM97/ W97M/ eicar
Spam-Virus Header = {$spam_virus_header}
Virus Names Which Are Spam = Sane*UNOFFICIAL HTML/* *Phish*
Block Encrypted Messages = {$block_encrypted}
Block Unencrypted Messages = {$block_unencrypted}
Allow Password-Protected Archives = {$allow_password}
Check Filenames In Password-Protected Archives = {$check_filenames}
Monitors for ClamAV Updates = /var/db/clamav/*.cvd
ClamAVmodule Maximum Recursion Level = 8
ClamAVmodule Maximum Files = 1000
ClamAVmodule Maximum File Size = 10000000 # (10 Mbytes)
ClamAVmodule Maximum Compression Ratio = 25
Allowed Sophos Error Messages =
Sophos IDE Dir = /opt/sophos-av/lib/sav
Sophos Lib Dir = /opt/sophos-av/lib
Monitors For Sophos Updates = /opt/sophos-av/lib/sav/*.ide
Clamd Port = 3310
Clamd Socket = /var/run/clamav/clamd.sock
Clamd Lock File = # /var/lock/subsys/clamd
Clamd Use Threads = no
ClamAV Full Message Scan = yes
Fpscand Port = 10200
{$custom_antivirus_options}

#
# Removing/Logging dangerous or potentially offensive content
# -----------------------------------------------------------
#
Dangerous Content Scanning = {$dangerous_content}
Allow Partial Messages = {$partial_messages}
Allow External Message Bodies = {$external_bodies}
Find Phishing Fraud = {$phishing_fraud}
Also Find Numeric Phishing = {$numeric_phishig}
Use Stricter Phishing Net = ${stricter_phishing_net}
Highlight Phishing Fraud = ${highlight_phishing}
Phishing Safe Sites File = %etc-dir%/phishing.safe.sites.conf
Phishing Bad Sites File = %etc-dir%/phishing.bad.sites.conf
Country Sub-Domains List = %etc-dir%/country.domains.conf
Allow IFrame Tags = {$content['iframe_tags']}
Allow Form Tags = {$content['form_tags']}
Allow Script Tags = {$content['script_tags']}
Allow WebBugs = {$content['web_bugs']}
Ignored Web Bug Filenames = spacer pixel.gif pixel.png gap shim
Known Web Bug Servers = msgtag.com
Web Bug Replacement = http://www.mailscanner.tv/1x1spacer.gif
Allow Object Codebase Tags = {$content['codebase_tags']}
Convert Dangerous HTML To Text = {$dangerous_html}
Convert HTML To Text = {$html_to_text}

#
# Attachment Filename Checking
# ----------------------------
#
Archives Are = zip rar ole
Allow Filenames =
Deny Filenames =
Filename Rules = %etc-dir%/filename.rules.conf
Allow Filetypes =
Allow File MIME Types =
Deny Filetypes =
Deny File MIME Types =
Filetype Rules = %etc-dir%/filetype.rules.conf
Archives: Allow Filenames =
Archives: Deny Filenames =
Archives: Filename Rules = %etc-dir%/archives.filename.rules.conf
Archives: Allow Filetypes =
Archives: Allow File MIME Types =
Archives: Deny Filetypes =
Archives: Deny File MIME Types =
Archives: Filetype Rules = %etc-dir%/archives.filetype.rules.conf
Default Rename Pattern = __FILENAME__.disarmed

#
# Reports and Responses
# ---------------------
#
Quarantine Infections = {$quarantine_infections}
Quarantine Silent Viruses = {$quarantine_silent_virus}
Quarantine Modified Body = {$quarantine_modified_body}
Quarantine Whole Message = {$quarantine_whole_message}
Quarantine Whole Messages As Queue Files = {$quarantine_whole_message_as_queue}
Keep Spam And MCP Archive Clean = {$keep_spam_and_mcp}
Language Strings = %report-dir%/languages.conf
Rejection Report = %report-dir%/rejection.report.txt
Deleted Bad Content Message Report  = %report-dir%/deleted.content.message.txt
Deleted Bad Filename Message Report = %report-dir%/deleted.filename.message.txt
Deleted Virus Message Report        = %report-dir%/deleted.virus.message.txt
Deleted Size Message Report        = %report-dir%/deleted.size.message.txt
Stored Bad Content Message Report  = %report-dir%/stored.content.message.txt
Stored Bad Filename Message Report = %report-dir%/stored.filename.message.txt
Stored Virus Message Report        = %report-dir%/stored.virus.message.txt
Stored Size Message Report        = %report-dir%/stored.size.message.txt
Disinfected Report = %report-dir%/disinfected.report.txt
Inline HTML Signature = %report-dir%/inline.sig.html
Inline Text Signature = %report-dir%/inline.sig.txt
Signature Image Filename = %report-dir%/sig.jpg
Signature Image <img> Filename = signature.jpg
Inline HTML Warning = %report-dir%/inline.warning.html
Inline Text Warning = %report-dir%/inline.warning.txt
Sender Content Report        = %report-dir%/sender.content.report.txt
Sender Error Report        = %report-dir%/sender.error.report.txt
Sender Bad Filename Report = %report-dir%/sender.filename.report.txt
Sender Virus Report        = %report-dir%/sender.virus.report.txt
Sender Size Report         = %report-dir%/sender.size.report.txt
Hide Incoming Work Dir = {$hide_incoming_work_dir}
Include Scanner Name In Reports = {$include_scanner_name}
#
# Changes to Message Headers
# --------------------------
#
Mail Header = X-%org-name%-MailScanner:
Spam Header = X-%org-name%-MailScanner-SpamCheck:
Spam Score Header = X-%org-name%-MailScanner-SpamScore:
Information Header = X-%org-name%-MailScanner-Information:
Add Envelope From Header = yes
Add Envelope To Header = no
Envelope From Header = X-%org-name%-MailScanner-From:
Envelope To Header = X-%org-name%-MailScanner-To:
ID Header = X-%org-name%-MailScanner-ID:
IP Protocol Version Header = # X-%org-name%-MailScanner-IP-Protocol:
Spam Score Character = s
SpamScore Number Instead Of Stars = no
Minimum Stars If On Spam List = 0
Clean Header Value       = Found to be clean
Infected Header Value    = Found to be infected
Disinfected Header Value = Disinfected
Information Header Value = Please contact the ISP for more information
Detailed Spam Report = yes
Include Scores In SpamAssassin Report = yes
Always Include SpamAssassin Report = no
Multiple Headers = append
Place New Headers At Top Of Message = no
Hostname = the %org-name% ($HOSTNAME) MailScanner
Sign Messages Already Processed = no
Sign Clean Messages = yes
Attach Image To Signature = no
Attach Image To HTML Message Only = yes
Allow Multiple HTML Signatures = no
Dont Sign HTML If Headers Exist = # In-Reply-To: References:
Mark Infected Messages = yes
Mark Unscanned Messages = yes
Unscanned Header Value = Not scanned: please contact your Internet E-Mail Service Provider for details
Remove These Headers = X-Mozilla-Status: X-Mozilla-Status2:
Deliver Cleaned Messages = yes

#
# Notifications back to the senders of blocked messages
# -----------------------------------------------------
#
Notify Senders = {$notify_sender}
Notify Senders Of Viruses = {$notify_sender_viruses}
Notify Senders Of Blocked Filenames Or Filetypes = {$notify_sender_fileytypes}
Notify Senders Of Blocked Size Attachments = {$notify_sender_attachments}
Notify Senders Of Other Blocked Content = {$notify_sender_contents}
Never Notify Senders Of Precedence = list bulk

#
# Changes to the Subject: line
# ----------------------------
#
Scanned Modify Subject = no # end
Scanned Subject Text = [Scanned]
Virus Modify Subject = start
Virus Subject Text = [Virus?]
Filename Modify Subject = start
Filename Subject Text = [Filename?]
Content Modify Subject = start
Content Subject Text = [Dangerous Content?]
Size Modify Subject = start
Size Subject Text = [Size]
Disarmed Modify Subject = start
Disarmed Subject Text = [Disarmed]
Phishing Modify Subject = no
Phishing Subject Text = [Fraude?]
Spam Modify Subject = start
Spam Subject Text = [Spam?]
High Scoring Spam Modify Subject = start
High Scoring Spam Subject Text = [Spam?]

#
# Changes to the Message Body
# ---------------------------
#
Warning Is Attachment = yes
Attachment Warning Filename = %org-name%-Attachment-Warning.txt
Attachment Encoding Charset = ISO-8859-1

#
# Mail Archiving and Monitoring
# -----------------------------
#
Archive Mail =
Missing Mail Archive Is = directory

#
# Notices to System Administrators
# --------------------------------
#
Send Notices = {$send_notices}
Notices Include Full Headers = {$notices_include_header}
Hide Incoming Work Dir in Notices = {$hide_incoming_work_dir_notices}
Notice Signature = {$notice_signature}
Notices From = ${$notice_from}
Notices To = ${$notice_to}
Local Postmaster = postmaster

#
# Spam Detection and Virus Scanner Definitions
# --------------------------------------------
#
Spam List Definitions = %etc-dir%/spam.lists.conf
Virus Scanner Definitions = %etc-dir%/virus.scanners.conf

#
# Spam Detection and Spam Lists (DNS blocklists)
# ----------------------------------------------
#

Spam Checks = yes
Spam List = # spamhaus-ZEN # You can un-comment this to enable them
Spam Domain List =
Spam Lists To Be Spam = 1
Spam Lists To Reach High Score = 3
Spam List Timeout = 10
Max Spam List Timeouts = 7
Spam List Timeouts History = 10
Is Definitely Not Spam = %rules-dir%/spam.whitelist.rules
Is Definitely Spam = no
Definite Spam Is High Scoring = no
Ignore Spam Whitelist If Recipients Exceed = 20
Max Spam Check Size = 200k

#
# Watermarking
# ------------
#
Use Watermarking = no
Add Watermark = yes
Check Watermarks With No Sender = yes
Treat Invalid Watermarks With No Sender as Spam = nothing
Check Watermarks To Skip Spam Checks = yes
Watermark Secret = %org-name%-Secret
Watermark Lifetime = 604800
Watermark Header = X-%org-name%-MailScanner-Watermark:

#
# SpamAssassin
# ------------
#

Use SpamAssassin = {$use_sa}
Max SpamAssassin Size = {$sa_max}
Required SpamAssassin Score = {$sa_score}
High SpamAssassin Score = {$hi_score}
SpamAssassin Auto Whitelist = {$sa_auto_whitelist}
SpamAssassin Timeout = 75
Max SpamAssassin Timeouts = 10
SpamAssassin Timeouts History = 30
Check SpamAssassin If On Spam List = {$check_sa_if_on_spam_list}
Include Binary Attachments In SpamAssassin = {$include_sa_bin_attachments}
Spam Score = {$spam_score}
Cache SpamAssassin Results = {$cache_spamassassin_results}
SpamAssassin Cache Database File = /var/spool/MailScanner/incoming/SpamAssassin.cache.db
Rebuild Bayes Every = {$rebuild_bayes}
Wait During Bayes Rebuild = {$wait_during_bayes_rebuild}

#
# Custom Spam Scanner Plugin
# --------------------------
#
Use Custom Spam Scanner = no
Max Custom Spam Scanner Size = 20k
Custom Spam Scanner Timeout = 20
Max Custom Spam Scanner Timeouts = 10
Custom Spam Scanner Timeout History = 20

#
# What to do with spam
# --------------------
#

Spam Actions = {$spam_actions} header "X-Spam-Status: Yes"
High Scoring Spam Actions = {$hispam_actions} header "X-Spam-Status: Yes"
Non Spam Actions = deliver header "X-Spam-Status: No"
SpamAssassin Rule Actions =
Sender Spam Report         = %report-dir%/sender.spam.report.txt
Sender Spam List Report    = %report-dir%/sender.spam.rbl.report.txt
Sender SpamAssassin Report = %report-dir%/sender.spam.sa.report.txt
Inline Spam Warning = %report-dir%/inline.spam.warning.txt
Recipient Spam Report = %report-dir%/recipient.spam.report.txt
Enable Spam Bounce = %rules-dir%/bounce.rules
Bounce Spam As Attachment = no
#
# Logging
# -------
#
Syslog Facility = {$syslog_facility}
Log Speed = {$log_speed}
Log Spam = {$log_spam}
Log Non Spam = {$log_non_spam}
Log Delivery And Non-Delivery = {$log_delivery}
Log Permitted Filenames = {$log_filenames}
Log Permitted Filetypes = {$log_filetypes}
Log Permitted File MIME Types = {$log_mime}
Log Silent Viruses = {$log_silent}
Log Dangerous HTML Tags = {$log_dangerous}
Log SpamAssassin Rule Actions = {$log_sa_rule_action}

#
# Advanced SpamAssassin Settings
# ------------------------------
#
SpamAssassin Temporary Dir = /var/spool/MailScanner/incoming/SpamAssassin-Temp
SpamAssassin User State Dir =
SpamAssassin Install Prefix =
SpamAssassin Site Rules Dir = /usr/local/etc/mail/spamassassin
SpamAssassin Local Rules Dir =
SpamAssassin Local State Dir = # /var/lib/spamassassin
SpamAssassin Default Rules Dir =

#
# MCP (Message Content Protection)
# -----------------------------
#

MCP Checks = {$mcp_checks}
First Check = spam
MCP Required SpamAssassin Score = {$mcp_score}
MCP High SpamAssassin Score = {$hi_mcp_score}
MCP Error Score = 1
MCP Header = X-%org-name%-MailScanner-MCPCheck:
Non MCP Actions = deliver
MCP Actions = {$mcp_action}
High Scoring MCP Actions = {$mcp_hi_action}
Bounce MCP As Attachment = {$bounce_mcp}
MCP Modify Subject = start
MCP Subject Text = [MCP?]
High Scoring MCP Modify Subject = start
High Scoring MCP Subject Text = [MCP?]

Is Definitely MCP = {$is_mcp}
Is Definitely Not MCP = {$is_not_mcp}
Definite MCP Is High Scoring = {$mcp_is_high_score}
Always Include MCP Report = {$include_mcp_report}
Detailed MCP Report = {$detailled_mcp_report}
Include Scores In MCP Report = {$score_mcp_report}
Log MCP = {$log_mcp}

MCP Max SpamAssassin Timeouts = 20
MCP Max SpamAssassin Size = {$mcp_max}
MCP SpamAssassin Timeout = 10

MCP SpamAssassin Prefs File = %mcp-dir%/mcp.spam.assassin.prefs.conf
MCP SpamAssassin User State Dir =
MCP SpamAssassin Local Rules Dir = %mcp-dir%
MCP SpamAssassin Default Rules Dir = %mcp-dir%
MCP SpamAssassin Install Prefix = %mcp-dir%
Recipient MCP Report = %report-dir%/recipient.mcp.report.txt
Sender MCP Report = %report-dir%/sender.mcp.report.txt

#
# Advanced Settings
# -----------------
#
Use Default Rules With Multiple Recipients = {$default_rule_multiple}
Read IP Address From Received Header = {$read_ipaddress}
Spam Score Number Format = {$spam_score_format}
MailScanner Version Number = 4.83.5
SpamAssassin Cache Timings = {$cache_timings}
Debug = {$debug}
Debug SpamAssassin = {$debug_spam}
Run In Foreground = {$foreground}
Always Looked Up Last = {$look_up_last}
Always Looked Up Last After Batch = {$look_up_last_batch}
Deliver In Background = {$deliver_background}
Delivery Method = {$mailscanner['deliver_method']}
Split Exim Spool = {$split_exim_spool}
Lockfile Dir = /var/spool/MailScanner/incoming/Locks
Custom Functions Dir = /usr/local/lib/MailScanner/MailScanner/CustomFunctions
Lock Type =
Syslog Socket Type =
Automatic Syntax Check = {$syntax_check}
Minimum Code Status = {$mailscanner['minimum_code']}
include /usr/local/etc/MailScanner/conf.d/*



EOF;
	#write files
	conf_mount_rw();
	if (!is_dir("/var/spool/MailScanner/incoming")){
		mkdir("/var/spool/MailScanner/incoming", 0755,true);
		chown ('/var/spool/MailScanner/incoming','postfix');
		}
	if (!is_dir("/var/spool/MailScanner/quarantine")){
		mkdir("/var/spool/MailScanner/quarantine", 0755,true);
		chown ('/var/spool/MailScanner/quarantine','postfix');
		}
	chown ('/var/spool/postfix','postfix');
	
	$mlang=strtolower($report['language']);
	$mfiles[]="/usr/local/etc/MailScanner/virus.scanners.conf";
	$mfiles[]="/usr/local/share/MailScanner/reports/{$mlang}/inline.spam.warning.txt";
	$mfiles[]="/usr/local/share/MailScanner/reports/{$mlang}/languages.conf";
	
	foreach ($mfiles as $mfile)
		if (! file_exists ($mfile))
			copy($mfile.".sample",$mfile);
			

	#update spam.assassin.prefs.conf
	$sa_temp=ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['sa_pref_file']);
	$pattern[0]='/#ifplugin/';
	$pattern[1]='/#pyzor_path/';
	$pattern[2]='/usr.bin.pyzor/';
	$pattern[3]='/#dcc_path/';
	$pattern[4]='/#endif/';
	$replacement[0]="ifplugin";
	$replacement[1]="pyzor_path";
	$replacement[2]="usr/local/bin/pyzor";
	$replacement[3]="dcc_path";
	$replacement[4]="endif";
	if (preg_match('/use_razor/',$antispam['safeatures'])){
			$pattern[5]='/\nuse_razor2\s+0/';
			$replacement[5]="\n".'# use_razor2		0';
			}
	else{
		$pattern[5]='/\n#\s+use_razor2\s+0/';
		$replacement[5]="\n".'use_razor2		0';
		}
	if (preg_match('/use_dcc/',$antispam['safeatures'])){
		$pattern[6]='/\nuse_dcc\s+0/';
		$replacement[6]="\n".'# use_dcc		0';
		}
	else{
		$pattern[6]='/\n#\s+use_dcc\s+0/';
		$replacement[6]="\n".'use_dcc		0';
		}
	if (preg_match('/use_pyzor/',$antispam['safeatures'])){
		$pattern[7]='/\nuse_pyzor\s+0/';
		$replacement[7]="\n".'# use_pyzor	0';
		}
	else{
		$pattern[7]='/\n#\s+use_pyzor\s+0/';
		$replacement[7]="\n".'# use_pyzor		0';
		}
	if (preg_match('/use_auto_learn_bayes/',$antispam['safeatures'])){
		$pattern[8]='/\nbayes_auto_learn\s+0/';
		$replacement[8]="\n".'# bayes_auto_learn	0';
		}
	else{
		$pattern[8]='/\n#\s+bayes_auto_learn\s+0/';
		$replacement[8]="\n".'bayes_auto_learn	0';
		}
	if (preg_match('/use_bayes/',$antispam['safeatures'])){
		$pattern[9]='/\nuse_bayes\s+0/';
		$replacement[9]="\n".'# use_bayes	0';
		}
	else{
		$pattern[9]='/\n#\s+use_bayes\s+0/';
		$replacement[9]="\n".'use_bayes	0';
		}
	if (preg_match('/sa_auto_whitelist/',$antispam['safeatures'])){
		$pattern[10]='/\nuse_auto_whitelist\s+0/';
		$replacement[10]="\n".'# use_auto_whitelist 0';
		}
	else{
		$pattern[10]='/\n#\s*use_auto_whitelist 0/';
		$replacement[10]="\n".'use_auto_whitelist	0';
		}
	if ($antispam['rblchecks']){
		$pattern[11]='/\nskip_rbl_checks\s+1/';
		$replacement[11]="\n".'# skip_rbl_checks	1';
		}
	else{
		$pattern[11]='/\n#\s+skip_rbl_checks\s+\d/';
		$replacement[11]="\n".'skip_rbl_checks	1';
		}
	$pattern[12]='/bayes_ignore_header ([a-zA-Z0-9_.-]+)MailScanner/';
	$replacement[12]="bayes_ignore_header ".($mailscanner['orgname']!=""?$mailscanner['orgname']:"pfsense")."-MailScanner";
	$pattern[13]='/envelope_sender_header X([a-zA-Z0-9_.-]+)MailScanner-From/';
	$replacement[13]="envelope_sender_header X-".($mailscanner['orgname']!=""?$mailscanner['orgname']:"pfsense")."-MailScanner-From";
	
	
	$sa_temp=preg_replace($pattern,$replacement,$sa_temp);
	#rint "pre".$sa_temp;exit;
	$config['installedpackages']['msantispam']['config'][0]['sa_pref_file']=base64_encode($sa_temp);
	write_config();
	
	file_put_contents($mailscanner_dir."/MailScanner.conf", $mc, LOCK_EX);
	file_put_contents($mailscanner_dir."/filename.rules.conf",ms_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filename_rules']),LOCK_EX);
	file_put_contents($mailscanner_dir."/filetype.rules.conf",ms_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filetype_rules']),LOCK_EX);
	file_put_contents($mailscanner_dir."/archives.filename.rules.conf",ms_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filename_rules']),LOCK_EX);
	file_put_contents($mailscanner_dir."/archives.filetype.rules.conf",ms_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filetype_rules']),LOCK_EX);
	file_put_contents($mailscanner_dir."/phishing.safe.sites.conf",ms_text_area_decode($config['installedpackages']['mscontent']['config'][0]['phishing_safe']),LOCK_EX);
	file_put_contents($mailscanner_dir."/phishing.bad.sites.conf",ms_text_area_decode($config['installedpackages']['mscontent']['config'][0]['phishing_bad']),LOCK_EX);
	file_put_contents($mailscanner_dir."/country.domains.conf",ms_text_area_decode($config['installedpackages']['mscontent']['config'][0]['country_domains']),LOCK_EX);
	file_put_contents($mailscanner_dir.'/spam.assassin.prefs.conf',$sa_temp,LOCK_EX);
	file_put_contents($mailscanner_dir.'/spam.lists.conf',ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['rbl_file']),LOCK_EX);
	file_put_contents($mailscanner_dir.'/mcp/mcp.spam.assassin.prefs.conf',ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['mcp_pref_file']),LOCK_EX);
	file_put_contents($mailscanner_dir.'/rules/bounce.rules',ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['bounce']),LOCK_EX);
	file_put_contents($mailscanner_dir.'/rules/max.message.size.rules',ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['max_message_size']),LOCK_EX);
	file_put_contents($mailscanner_dir.'/rules/spam.whitelist.rules',ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['spam_whitelist']),LOCK_EX);
	
	foreach ($report_files as $key_r => $file_r)
		file_put_contents($report_dir.'/'.$file_r,ms_text_area_decode($config['installedpackages']['msreport']['config'][0][$key_r]),LOCK_EX);
		
	if ($alert['sig']){
		$sig_html=ms_text_area_decode($config['installedpackages']['msalerts']['config'][0]['sig_html']);
		$sig_txt=ms_text_area_decode($config['installedpackages']['msalerts']['config'][0]['sig_txt']);}
	else{
		$sig_html="";
		$sig_txt="";}
	file_put_contents($report_dir.'/inline.sig.txt',$sig_txt,LOCK_EX);
	file_put_contents($report_dir.'/inline.sig.html',$sig_html,LOCK_EX);

	if ($alert['warning']){
		$warning_html=ms_text_area_decode($config['installedpackages']['msalerts']['config'][0]['warning_html']);
		$warning_txt=ms_text_area_decode($config['installedpackages']['msalerts']['config'][0]['warning_txt']);}
	else{
		$warning_html="";
		$warning_txt="";}
	file_put_contents($report_dir.'/inline.warning.txt',$warning_txt,LOCK_EX);
	file_put_contents($report_dir.'/inline.warning.html',$warning_html,LOCK_EX);
	
	#check virus_scanner options
	$libexec_dir="/usr/local/libexec/MailScanner/";
	if ($virus_scanning == "yes"){
		if ($antivirus['virus_scanner'] =="none"){
			unlink_if_exists($libexec_dir.'clamav-autoupdate');
			unlink_if_exists($libexec_dir.'clamav-wrapper');
		}
		else{
			chown('/var/run/clamav/', 'postfix');
			chown('/var/log/clamav/', 'postfix');
			chown('/var/db/clamav/', 'postfix');
			if (file_exists('/var/db/clamav/bytecode.cld'))
				chown('/var/db/clamav/bytecode.cld', 'postfix');
			if (file_exists('/var/db/clamav/daily.cld'))
				chown('/var/db/clamav/daily.cld', 'postfix');
			if (file_exists('/var/db/clamav/main.cvd'))
				chown('/var/db/clamav/main.cvd', 'postfix');
			if (file_exists('/var/db/clamav/mirrors.dat'))
				chown('/var/db/clamav/mirrors.dat', 'postfix');
			if (file_exists('/var/log/clamav/clamd.log'))
				chown('/var/log/clamav/clamd.log', 'postfix');
			if (file_exists('/var/log/clamav/freshclam.log'))
				chown('/var/log/clamav/freshclam.log', 'postfix');
			
			copy($libexec_dir.'clamav-autoupdate.sample',$libexec_dir.'clamav-autoupdate');
			chmod ($libexec_dir.'clamav-autoupdate',0755);
			copy($libexec_dir.'clamav-wrapper.sample',$libexec_dir.'clamav-wrapper');
			chmod ($libexec_dir.'clamav-autoupdate',0755);
			if (!file_exists('/var/db/clamav/main.cvd')){
				log_error('No clamav database found, running freshclam in background.');
				mwexec_bg('/usr/local/bin/freshclam');
				}
			#clamav-wrapper file
			$cconf=$libexec_dir."clamav-wrapper";
			$cconf_file=file_get_contents($cconf);
			if (preg_match('/"clamav"/',$cconf_file)){
				$cconf_file=preg_replace('/"clamav"/','"postfix"',$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
				
			#freshclam conf file
			$cconf="/usr/local/etc/freshclam.conf";
			$cconf_file=file_get_contents($cconf);
			if (preg_match('/DatabaseOwner clamav/',$cconf_file)){
				$cconf_file=preg_replace("/DatabaseOwner clamav/","DatabaseOwner postfix",$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
				
			#clamd conf file
			$cconf="/usr/local/etc/clamd.conf";
			$cconf_file=file_get_contents($cconf);
			if (preg_match('/User clamav/',$cconf_file)){
				$cconf_file=preg_replace("/User clamav/","User postfix",$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
			#clamd script file
			$script='/usr/local/etc/rc.d/clamav-clamd';
			$script_file=file_get_contents($script);
			if (preg_match('/NO/',$script_file)){
				$script_file=preg_replace("/NO/","YES",$script_file);
				file_put_contents($script, $script_file, LOCK_EX);
				chmod ($script,0755);
				}
			mwexec_bg("$script fastrestart");
				
		}
	}
	else{
		unlink_if_exists($libexec_dir.'clamav-autoupdate');
		unlink_if_exists($libexec_dir.'clamav-wrapper');
	}
	
	#check dcc startup script
			$script='/usr/local/etc/rc.d/dccifd';
			$script_file=file_get_contents($script);
			if (preg_match('/NO/',$script_file)){
				$script_file=preg_replace("/NO/","YES",$script_file);
				file_put_contents($script, $script_file, LOCK_EX);
				chmod ($script,0755);
				}
			mwexec_bg("$script fastrestart");
	#check dcc config file
			$script='/usr/local/dcc/dcc_conf';
			$script_file=file_get_contents($script);
			if (preg_match('/DCCIFD_ENABLE=off/',$script_file)){
				$script_file=preg_replace("/DCCIFD_ENABLE=off/","DCCIFD_ENABLE=on",$script_file);
				file_put_contents($script, $script_file, LOCK_EX);
				}
			
	$script='/usr/local/etc/rc.d/mailscanner';

	#fix MIME::ToolUtils deprecated function and usecure dependency calls in /usr/local/sbin/mailscanner
	$cconf="/usr/local/sbin/mailscanner";
	$cconf_file=file_get_contents($cconf);
	$pattern2[0]='/perl\W+I/';
	$pattern2[1]='/\smy .current = config MIME::ToolUtils/';
	$replacement2[0]='perl -U -I';
	$replacement2[1]=' #my $current = config MIME::ToolUtils';
	if (preg_match('/perl\W+I/',$cconf_file)){
		$cconf_file=preg_replace($pattern2,$replacement2,$cconf_file);
		file_put_contents($cconf, $cconf_file, LOCK_EX);
		#force old process stop	
		mwexec("$script stop");
		}	
	
	$script_file=file_get_contents($script);
	if (preg_match('/NO/',$script_file)){
		$script_file=preg_replace("/NO/","YES",$script_file);
		file_put_contents($script, $script_file, LOCK_EX);
	}
	if($config['installedpackages']['mailscanner']['config'][0]['enable']){
		log_error("Reload mailscanner");
		chmod ($script,0755);
		mwexec_bg("$script fastrestart");
	}
	else{
		log_error("Stopping mailscanner if running");
		mwexec("$script stop");
		chmod ($script,0444);
	}
	conf_mount_ro();
	$synconchanges = $config['installedpackages']['mailscannersync']['config'][0]['synconchanges'];
	if(!$synconchanges && !$syncondbchanges) 
		return;
	log_error("[mailscanner] mailscanner_xmlrpc_sync.php is starting.");
	foreach ($config['installedpackages']['mailscannersync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		$sync_type = $sh['sync_type'];
		if($password && $sync_to_ip)
			mailscanner_do_xmlrpc_sync($sync_to_ip, $password,$sync_type);
		}
	}
	log_error("[postfix] postfix_xmlrpc_sync.php is ending.");

}

function mailscanner_validate_input($post, &$input_errors) {
	foreach ($post as $key => $value) {
		if (empty($value))
			continue;
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

function mailscanner_php_install_command() {
	sync_package_mailscanner();
}

function mailscanner_php_deinstall_command() {
	mwexec("/usr/local/etc/rc.d/mailscanner.sh stop");
	sleep(1);
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/mailscanner.sh");
	conf_mount_ro();
}

function mailscanner_do_xmlrpc_sync($sync_to_ip, $password,$sync_type) {
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
	$sync_xml=$config['installedpackages']['mailscannersync']['config'][0]['synconchanges'];
	if ($sync_xml){
		log_error("Include mailscanner config");
		$xml['mailscanner'] = $config['installedpackages']['mailscanner'];
		$xml['msreport'] = $config['installedpackages']['msreport'];
		$xml['mscontent'] = $config['installedpackages']['mscontent'];
		$xml['msantivirus'] = $config['installedpackages']['msantivirus'];
		$xml['msantispam'] = $config['installedpackages']['msantispam'];
		$xml['msalerts'] = $config['installedpackages']['msalerts'];
		}
	if (count($xml) > 0){
		/* assemble xmlrpc payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($xml)
		);
	
		/* set a few variables needed for sync code borrowed from filter.inc */
		$url = $synchronizetoip;
		log_error("Beginning mailscanner XMLRPC sync to {$url}:{$port}.");
		$method = 'pfsense.merge_installedpackages_section_xmlrpc';
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials('admin', $password);
		if($g['debug'])
			$cli->setDebug(1);
		/* send our XMLRPC message and timeout after 250 seconds */
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while attempting mailscanner XMLRPC sync with {$url}:{$port}.";
			log_error($error);
			file_notice("sync_settings", $error, "Mailscanner Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while attempting mailscanner XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "mailscanner Settings Sync", "");
		} else {
			log_error("mailscanner XMLRPC sync successfully completed with {$url}:{$port}.");
		}
		
		/* tell postfix to reload our settings on the destionation sync host. */
		$method = 'pfsense.exec_php';
		$execcmd  = "require_once('/usr/local/pkg/mailscanner.inc');\n";
		$execcmd .= "sync_package_mailscanner();";
		
		/* assemble xmlrpc payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($execcmd)
		);
	
		log_error("mailscanner XMLRPC reload data {$url}:{$port}.");
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials('admin', $password);
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while attempting mailscanner XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
			log_error($error);
			file_notice("sync_settings", $error, "mailscanner Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while attempting mailscanner XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "mailscanner Settings Sync", "");
		} else {
			log_error("mailscanner XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
		}
	}
}

?>