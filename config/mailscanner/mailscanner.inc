<?php
/*
	postfix.inc
	part of the Postfix package for pfSense
	Copyright (C) 2011-2013 Marcello Coutinho
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code MUST retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form MUST reproduce the above copyright
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
$shortcut_section = "mailscanner";
require_once("util.inc");
require("globals.inc");
#require("guiconfig.inc");

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version > 2.0)
	define('MAILSCANNER_LOCALBASE', '/usr/pbi/mailscanner-' . php_uname("m"));
else
  define('MAILSCANNER_LOCALBASE','/usr/local');

$uname=posix_uname();
if ($uname['machine']=='amd64')
        ini_set('memory_limit', '250M');
        
function ms_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function sync_package_mailscanner($via_rpc=false) {
	global $config;

	# detect boot process
	if (is_array($_POST)){
		if (preg_match("/\w+/",$_POST['__csrf_magic']))
			unset($boot_process);
		else
			$boot_process="on";
	}
	exec('/bin/pgrep -f MailScanner',$pgrep_out);
	if (count($pgrep_out) > 0 && isset($boot_process) && $via_rpc==false)
		return;

	#check default config
	$load_samples=0;
	
	#assign xml arrays
	if (!is_array($config['installedpackages']['mailscanner'])){
		$config['installedpackages']['mailscanner']['config'][0]=array(	'max_children'=> '5',
																		'pim'=> 'ScanMessages',
																		'syslog_facility'=> 'mail',
																		'syslog'=>'LogSpamAssassinRuleActions',
																		'advanced'=> 'DeliverInBackground,AutomaticSyntaxCheck',
																		'deliver_method'=>'batch',
																		'minimum_code'=>'batch',
																		'spam_score_format'=>'%d',
																		'cache_timings'=> '1800,300,10800,172800,600' );
		$load_samples++;
	}
	$mailscanner=$config['installedpackages']['mailscanner']['config'][0];
	if (!is_array($config['installedpackages']['msattachments'])){
		$config['installedpackages']['msattachments']['config'][0]=array('features'=>'ExpandTNEF,FindArchiveByContent,UnpackMicrosoftDocuments',
																		'tnef_contents'=>'replace',
																		'max_sizes'=>'-1',
																		'archive_depth'=>'8',
																		'attachment_filename'=>'MessageAttachments.zip',
																		'attachment_extension_exclude'=>'0',
																		'attachment_max_per_archive'=>'0',
																		'attachment_max'=>'50k');
		$load_samples++;
	}
	$attachments=$config['installedpackages']['msattachments']['config'][0];
	if (!is_array($config['installedpackages']['msantivirus'])){
		$config['installedpackages']['msantivirus']['config'][0]=array(	'features'=>'VirusScanning,CheckFilenamesInPassword-ProtectedArchives',
																		'virus_scanner'=>'auto',
																		'timeout'=>'300',
																		'silent_virus'=>'HTML-Iframe,All-viruses');
		$load_samples++;
	}
	$antivirus=$config['installedpackages']['msantivirus']['config'][0];
	if (!is_array($config['installedpackages']['mscontent'])){
		$config['installedpackages']['mscontent']['config'][0]=array('checks'=>'DangerousContentScanning,UseStricterPhishingNet,HighlightPhishingFraud',
																	'iframe_tags'=>'disarm',
																	'form_tags'=>'disarm',
																	'script_tags'=>'disarm',
																	'web_bugs'=>'disarm',
																	'codebase_tags'=>'disarm');
		$load_samples++;
	}
	$content=$config['installedpackages']['mscontent']['config'][0];
	if (!is_array($config['installedpackages']['msreport'])){
		$config['installedpackages']['msreport']['config'][0]=array('features'=>'HideIncomingWorkDir,IncludeScannerNameInReports',
																	'notification'=>'NotifySendersofBlockedFilenamesorFiletypes',
																	'system'=>'NoticesIncludeFullHeaders',
																	'language'=>'en');
		$load_samples++;
	}
	$report=$config['installedpackages']['msreport']['config'][0];
	if (!is_array($config['installedpackages']['msantispam'])){
		$config['installedpackages']['msantispam']['config'][0]=array(	'rblfeatures'=>'spam_checks',
																		'safeatures'=>'use_sa,sa_auto_whitelist,check_sa_if_on_spam_list,spam_score,cache_spamassassin_results,use_razor,use_dcc,use_bayes,use_auto_learn_bayes',
																		'sa_score'=>'6',
																		'spam_actions'=>'deliver',
																		'hi_score'=>'20',
																		'hispam_actions'=>'deliver',
																		'rebuild_bayes'=>'86400',
																		'mcp_features'=>'detailled_mcp_report',
																		'mcp_score'=>'1');
		$load_samples++;
	}
	$antispam=$config['installedpackages']['msantispam']['config'][0];
	if (is_array($config['installedpackages']['msalerts']))
		$alert=$config['installedpackages']['msalerts']['config'][0];
		
	#General options
	$info  =($mailscanner['orgname']?'%org-name% = '.$mailscanner['orgname']."\n":'%org-name% = Pfsense'."\n");
	$info .=($mailscanner['longorgname']?'%org-long-name% = '.$mailscanner['longorgname']."\n":'%org-long-name% = Pfsense Inc.'."\n");
	$info .=($mailscanner['website']?'%web-site% = '.$mailscanner['website']."\n":'%web-site% = www.pfsense.org'."\n");
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
	$mailscanner_dir=MAILSCANNER_LOCALBASE ."/etc/MailScanner";
	
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
	
	//$report_dir=MAILSCANNER_LOCALBASE."/share/MailScanner/reports/".strtolower($report['language']);
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
			if (file_exists($report_dir.'/'.$file_r.'.sample')){
				$config['installedpackages']['msreport']['config'][0][$key_r]=base64_encode(file_get_contents($report_dir.'/'.$file_r.'.sample'));
				file_put_contents($report_dir.'/'.$file_r,ms_text_area_decode($config['installedpackages']['msreport']['config'][0][$key_r]),LOCK_EX);
				$load_samples++;
				}
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
	#get mailscanner version
	$msc_bin=MAILSCANNER_LOCALBASE. "/sbin/mailscanner";
	if (file_exists($msc_bin)){
		$msc_bin_file=file_get_contents($msc_bin);
		if (preg_match("/MailScannerVersion = '(\S+)'/",$msc_bin_file,$msv_matches))
			$mailscanner_version=$msv_matches[1];
		else
			$mailscanner_version='4.83.5';
	}
	#create MailScanner.conf
	$mlb=MAILSCANNER_LOCALBASE;

	include("mailscanner.conf.template");
	#write files
	conf_mount_rw();
	$msc_dirs=array("incoming", "incoming/Locks", "quarantine");
	foreach ($msc_dirs as $msc_dir){
		if (!is_dir("/var/spool/MailScanner/{$msc_dir}")){
			mkdir("/var/spool/MailScanner/{$msc_dir}", 0755,true);
			chown ("/var/spool/MailScanner/{$msc_dir}",'postfix');
			}
		}
	if (is_dir('/var/spool/postfix'))
		chown ('/var/spool/postfix','postfix');
	
	$mlang=strtolower($report['language']);
	$mfiles[]= MAILSCANNER_LOCALBASE. "/etc/MailScanner/virus.scanners.conf";
	$mfiles[]= MAILSCANNER_LOCALBASE. "/share/MailScanner/reports/{$mlang}/inline.spam.warning.txt";
	$mfiles[]= MAILSCANNER_LOCALBASE. "/share/MailScanner/reports/{$mlang}/languages.conf";
	
	foreach ($mfiles as $mfile)
		if (! file_exists ($mfile) && file_exists($mfile.".sample"))
			copy($mfile.".sample",$mfile);
			

	#update spam.assassin.prefs.conf
	$sa_temp=ms_text_area_decode($config['installedpackages']['msantispam']['config'][0]['sa_pref_file']);
	$pattern[]='/#ifplugin/';
	$pattern[]='/#dcc_path/';
	$pattern[]='/#endif/';
	
	$replacement[]="ifplugin";
	$replacement[]="dcc_path";
	$replacement[]="endif";
	if (preg_match('/use_razor/',$antispam['safeatures'])){
			$pattern[]='/\nuse_razor2\s+0/';
			$replacement[]="\n".'# use_razor2		0';
			}
	else{
		$pattern[]='/\n#\s+use_razor2\s+0/';
		$replacement[]="\n".'use_razor2		0';
		}
	if (preg_match('/use_dcc/',$antispam['safeatures'])){
		$pattern[]='/\nuse_dcc\s+0/';
		$replacement[]="\n".'# use_dcc		0';
		}
	else{
		$pattern[]='/\n#\s+use_dcc\s+0/';
		$replacement[]="\n".'use_dcc		0';
		}
	if (preg_match('/use_pyzor/',$antispam['safeatures'])){
		$pattern[]='/#pyzor_path/';
		$pattern[]="/\S+yzor_disabled/";
		$pattern[]='/usr.bin.pyzor/';
		$pattern[]='/use_pyzor/';
		$pattern[]="/\S+o_not_use_pyzor/";
		$replacement[]="pyzor_path";
		$replacement[]="pyzor_path";
		$replacement[]="usr/local/bin/pyzor";
		$replacement[]="use_pyzor";
		$replacement[]="use_pyzor";
		}
	else{
		$pattern[]='/use_pyzor/';
		$pattern[]='/pyzor_path/';
		$replacement[]="#do_not_use_pyzor";
		$replacement[]="#pyzor_disabled";
		}
	if (preg_match('/use_auto_learn_bayes/',$antispam['safeatures'])){
		$pattern[]='/\nbayes_auto_learn\s+0/';
		$replacement[]="\n".'# bayes_auto_learn	0';
		}
	else{
		$pattern[]='/\n#\s+bayes_auto_learn\s+0/';
		$replacement[]="\n".'bayes_auto_learn	0';
		}
	if (preg_match('/use_bayes/',$antispam['safeatures'])){
		$pattern[]='/\nuse_bayes\s+0/';
		$replacement[]="\n".'# use_bayes	0';
		}
	else{
		$pattern[]='/\n#\s+use_bayes\s+0/';
		$replacement[]="\n".'use_bayes	0';
		}
	if (preg_match('/sa_auto_whitelist/',$antispam['safeatures'])){
		$pattern[]='/\nuse_auto_whitelist\s+0/';
		$replacement[]="\n".'# use_auto_whitelist 0';
		}
	else{
		$pattern[]='/\n#\s*use_auto_whitelist 0/';
		$replacement[]="\n".'use_auto_whitelist	0';
		}
	if ($antispam['rblchecks']){
		$pattern[]='/\nskip_rbl_checks\s+1/';
		$replacement[]="\n".'# skip_rbl_checks	1';
		}
	else{
		$pattern[]='/\n#\s+skip_rbl_checks\s+\d/';
		$replacement[]="\n".'skip_rbl_checks	1';
		}
	$pattern[]='/bayes_ignore_header ([a-zA-Z0-9_.-]+)MailScanner/';
	$replacement[]="bayes_ignore_header ".($mailscanner['orgname']!=""?$mailscanner['orgname']:"Pfsense")."-MailScanner";
	$pattern[]='/envelope_sender_header X([a-zA-Z0-9_.-]+)MailScanner-From/';
	$replacement[]="envelope_sender_header X-".($mailscanner['orgname']!=""?$mailscanner['orgname']:"Pfsense")."-MailScanner-From";
	
	
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
	$libexec_dir=MAILSCANNER_LOCALBASE. "/libexec/MailScanner/";
	if ($virus_scanning == "yes"){
		if ($antivirus['virus_scanner'] =="none"){
			unlink_if_exists($libexec_dir.'clamav-autoupdate');
			unlink_if_exists($libexec_dir.'clamav-wrapper');
		}
		else{
			$av_dirs=array('run','log','db');
			foreach ($av_dirs as $av_dir){
				if (!is_dir("/var/$av_dir/clamav"))
					mkdir("/var/$av_dir/clamav",0774,true);
				chown("/var/$av_dir/clamav", 'postfix');
				chgrp("/var/$av_dir/clamav", 'wheel');
				}
			$av_files=array('/var/db/clamav/daily.cld','/var/db/clamav/main.cvd','/var/db/clamav/mirrors.dat',
							'/var/log/clamav/clamd.log','/var/log/clamav/freshclam.log','/var/db/clamav/bytecode.cld');
			foreach ($av_files as $av_file){
				if (file_exists($av_file))
					chown($av_file, 'postfix');
				}
			copy($libexec_dir.'clamav-autoupdate.sample',$libexec_dir.'clamav-autoupdate');
			chmod ($libexec_dir.'clamav-autoupdate',0755);
			copy($libexec_dir.'clamav-wrapper.sample',$libexec_dir.'clamav-wrapper');
			chmod ($libexec_dir.'clamav-autoupdate',0755);
							
			#clamav-wrapper file
			$cconf=$libexec_dir."clamav-wrapper";
			if (file_exists($cconf)){
				$cconf_file=file_get_contents($cconf);
				if (preg_match('/"clamav"/',$cconf_file)){
					$cconf_file=preg_replace('/"clamav"/','"postfix"',$cconf_file);
					file_put_contents($cconf, $cconf_file, LOCK_EX);
					}
				}
				
			#freshclam conf file
			$cconf=MAILSCANNER_LOCALBASE. "/etc/freshclam.conf";
			if (file_exists($cconf)){
				$cconf_file=file_get_contents($cconf);
				if (preg_match('/DatabaseOwner clamav/',$cconf_file)){
					$cconf_file=preg_replace("/DatabaseOwner clamav/","DatabaseOwner postfix",$cconf_file);
					file_put_contents($cconf, $cconf_file, LOCK_EX);
					}
				}
				
			#clamd conf file
			$cconf=MAILSCANNER_LOCALBASE. "/etc/clamd.conf";
			if (file_exists($cconf)){
				$cconf_file=file_get_contents($cconf);
				if (preg_match('/User clamav/',$cconf_file)){
					$cconf_file=preg_replace("/User clamav/","User postfix",$cconf_file);
					file_put_contents($cconf, $cconf_file, LOCK_EX);
					}
				}
			#clamd script file
			$script=MAILSCANNER_LOCALBASE. '/etc/rc.d/clamav-clamd';
			if (file_exists($script)){
				$script_file=file($script);
				foreach ($script_file as $script_line){
					if(preg_match("/command=/",$script_line)){
						$new_clamav_startup.= "/bin/mkdir -p /var/run/clamav\n";
						$new_clamav_startup.= "chown postfix /var/run/clamav\n";
						$new_clamav_startup.=$script_line;
						}
					elseif(!preg_match("/(mkdir|chown|sleep|mailscanner)/",$script_line)) {
						$new_clamav_startup.=preg_replace("/NO/","YES",$script_line);
						}
					}
				file_put_contents($script, $new_clamav_startup, LOCK_EX);
				mwexec("/usr/sbin/pw user show postfix || /usr/sbin/pw user add -n postfix -s /usr/sbin/nologin");
				chmod ($script,0755);
				if($config['installedpackages']['mailscanner']['config'][0]['enable']){
					if (is_process_running('clamd')){
						log_error("Restarting clamav-clamd daemon");
						mwexec("$script restart");
						}
					else{
						log_error("Starting clamav-clamd daemon");
						mwexec_bg("$script start");
						}	
				}
				else{
					if (is_process_running('clamd')){
						log_error("Restarting clamav-clamd daemon");
						mwexec("$script start");
						}
					}
				}
				
				#check clamav database
				if (!file_exists('/var/db/clamav/main.cvd')){
				log_error('No clamav database found, running freshclam in background.');
				mwexec_bg(MAILSCANNER_LOCALBASE. '/bin/freshclam --config-file='.MAILSCANNER_LOCALBASE.'/etc/freshclam.conf --user=root');
				}
				
		}
	}
	else{
		unlink_if_exists($libexec_dir.'clamav-autoupdate');
		unlink_if_exists($libexec_dir.'clamav-wrapper');
	}
	
	#check dcc config file
			$script=MAILSCANNER_LOCALBASE. '/dcc/dcc_conf';
			if (file_exists($script)){
				$script_file=file_get_contents($script);
				if (preg_match('/DCCIFD_ENABLE=off/',$script_file)){
					$script_file=preg_replace("/DCCIFD_ENABLE=off/","DCCIFD_ENABLE=on",$script_file);
					file_put_contents($script, $script_file, LOCK_EX);
					}
				}
	
	#check dcc startup script
			$script=MAILSCANNER_LOCALBASE. '/etc/rc.d/dccifd';
			if (file_exists($script)){
				$script_file=file_get_contents($script);
				if (preg_match('/NO/',$script_file)){
					$script_file=preg_replace("/NO/","YES",$script_file);
					file_put_contents($script, $script_file, LOCK_EX);
					chmod ($script,0755);
					}
				
				if($config['installedpackages']['mailscanner']['config'][0]['enable']){
					if(is_process_running('dccifd')){
						log_error("Restarting dccifd");
						mwexec("$script restart");	
						}
					else{
						log_error("Starting dccifd");
						mwexec("$script start");
						}	
					}
				else{
					if(is_process_running('dccifd')){
						log_error("Stopping dccifd");
						mwexec("$script stop");	
						}
					}
				}

	$script=MAILSCANNER_LOCALBASE. '/etc/rc.d/mailscanner';

	#fix MIME::ToolUtils deprecated function and usecure dependency calls in /usr/local/sbin/mailscanner
	$cconf=MAILSCANNER_LOCALBASE. "/sbin/mailscanner";
	if (file_exists($cconf)){
		
		$perl_bin="perl_mailscanner";
		if(file_exists(MAILSCANNER_LOCALBASE . '/bin/perl') && !file_exists(MAILSCANNER_LOCALBASE . "/bin/{$perl_bin}")){
		 	link(MAILSCANNER_LOCALBASE . '/bin/perl',MAILSCANNER_LOCALBASE . '/bin/perl_mailscanner');
		}
		if (file_exists(MAILSCANNER_LOCALBASE . "/bin/{$perl_bin}")){
			$cconf_file=file_get_contents($cconf);
			$pattern2[0]='@#!/usr\S+bin/perl.*I@';
			//$pattern2[1]='/\smy .current = config MIME::ToolUtils/';
			$replacement2[0]='#!'.MAILSCANNER_LOCALBASE. "/bin/{$perl_bin} -U -I";
			//$replacement2[1]=' #my $current = config MIME::ToolUtils';
			if (preg_match('@#!/usr\S+bin/perl.*I@',$cconf_file)){
				$cconf_file=preg_replace($pattern2,$replacement2,$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
		}
			
	}

	#check spam assassin rules
	$saupdate="/usr/local/bin/sa-update";
	if (file_exists($saupdate)){
		$rules_found=0;
		if (file_exists("/var/db/spamassassin")){
			foreach (glob("/var/db/spamassassin/*",GLOB_ONLYDIR) as $dirname)
				$rules_found++;
			}
 		if ($rules_found==0){
 			log_error("Mailscanner- No spamassassin rules found, forcing sa-update.");
 			mwexec($saupdate);
 		}
	}	

	if (file_exists($script)){
		$script_file=file_get_contents($script);
		if (preg_match('/NO/',$script_file)){
			$script_file=preg_replace("/NO/","YES",$script_file);
			file_put_contents($script, $script_file, LOCK_EX);
			chmod ($script,0755);
			}
		exec('/bin/pgrep -f MailScanner', $pgrep_out);
		if($config['installedpackages']['mailscanner']['config'][0]['enable']){
			chmod ($script,0755);
			if (count($pgrep_out) >  0 && file_exists($script)){
				log_error("Restarting MailScanner");
				mwexec_bg("$script restart");
				}
			else{
				log_error("Starting MailScanner");
				mwexec("$script start");
				}
			}
		else{
			if (count($pgrep_out) >  0 && file_exists($script)){
				log_error("Stopping MailScanner");
				mwexec("$script stop");
				chmod ($script,0444);
				}
			}
		}
	conf_mount_ro();

	#does not sync during boot process
	if (isset($boot_process))
		return;

	/* Uses XMLRPC to synchronize the changes to a remote node */
	if (is_array($config['installedpackages']['mailscannersync'])){
		$mailscanner_sync=$config['installedpackages']['mailscannersync']['config'][0];
		$synctimeout = $mailscanner_sync['synctimeout'];
		$synconchanges = $mailscanner_sync['synconchanges'];
		switch ($synconchanges){
			case "manual":
				if (is_array($mailscanner_sync[row])){
					$rs=$mailscanner_sync[row];
					}
				else{
					log_error("[Mailscanner] xmlrpc sync is enabled but there is no hosts to push mailscanner config.");
					return;
					}
				break;
			case "auto":
					if (is_array($config['installedpackages']['carpsettings']) && is_array($config['installedpackages']['carpsettings']['config'])){
						$system_carp=$config['installedpackages']['carpsettings']['config'][0];
						$rs[0]['ipaddress']=$system_carp['synchronizetoip'];
						$rs[0]['username']=$system_carp['username'];
						$rs[0]['password']=$system_carp['password'];
						$rs[0]['enabless']=true;
						if (! is_ipaddr($system_carp['synchronizetoip'])){
							log_error("[Mailscanner] xmlrpc sync is enabled but there is no system backup hosts to push mailscanner config.");
							return;
							}
					}
					else{
						log_error("[Mailscanner] xmlrpc sync is enabled but there is no system backup hosts to push mailscanner config.");
						return;
					}
				break;			
			default:
				return;
			break;
		}
		if (is_array($rs)){
			log_error("[Mailscanner] xmlrpc sync is starting.");
			foreach($rs as $sh){
				$sync_to_ip = $sh['ipaddress'];
				if($sh['username'])
					$username = $sh['username'];
				else
					$username = 'admin';
				if($sh['password'] && $sh['ipaddress'] && $sh['enabless'])
					mailscanner_do_xmlrpc_sync($sh['ipaddress'], $username, $sh['password'],$sh['sync_type'],$synctimeout);
				}
			log_error("[Mailscanner] xmlrpc sync is ending.");
			}
 		}
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
	exec('/bin/pgrep -f MailScanner',$pgrep_out);
	if (count($pgreg_out) > 0){
		mwexec(MAILSCANNER_LOCALBASE. "/etc/rc.d/mailscanner stop");
		sleep(1);
		conf_mount_rw();
		unlink_if_exists(MAILSCANNER_LOCALBASE. "/etc/rc.d/mailscanner");
		conf_mount_ro();
		}
}

function mailscanner_do_xmlrpc_sync($sync_to_ip,$username,$password,$sync_type,$synctimeout) {
	global $config, $g;

	if(!$username)
		$username="admin";

	if(!$password)
		return;

	if(!$sync_to_ip)
		return;

	if(!$synctimeout)
		$synctimeout=120;
		
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
		$cli->setCredentials($username, $password);
		if($g['debug'])
			$cli->setDebug(1);
		/* send our XMLRPC message and timeout after $synctimeout seconds */
		$resp = $cli->send($msg, $synctimeout);
		if(!$resp) {
			$error = "A communications error occurred while attempting mailscanner XMLRPC sync with {$url}:{$port}.";
			log_error($error);
			file_notice("sync_settings", $error, "Mailscanner Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, $synctimeout);
			$error = "An error code was received while attempting mailscanner XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "mailscanner Settings Sync", "");
		} else {
			log_error("mailscanner XMLRPC sync successfully completed with {$url}:{$port}.");
		}
		
		/* tell postfix to reload our settings on the destionation sync host. */
		$method = 'pfsense.exec_php';
		$execcmd  = "require_once('/usr/local/pkg/mailscanner.inc');\n";
		$execcmd .= "sync_package_mailscanner(true);";
		
		/* assemble xmlrpc payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($execcmd)
		);
	
		log_error("mailscanner XMLRPC reload data {$url}:{$port}.");
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials($username, $password);
		$resp = $cli->send($msg, $synctimeout);
		if(!$resp) {
			$error = "A communications error occurred while attempting mailscanner XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
			log_error($error);
			file_notice("sync_settings", $error, "mailscanner Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, $synctimeout);
			$error = "An error code was received while attempting mailscanner XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "mailscanner Settings Sync", "");
		} else {
			log_error("mailscanner XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
		}
	}
}

?>
