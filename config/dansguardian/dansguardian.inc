<?php
/*
	dansguardian.inc
	part of the Dansguardian package for pfSense
	Copyright (C) 2012 Marcello Coutinho
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


function dg_text_area_decode($text){
	return preg_replace('/\r\n/', "\n",base64_decode($text));	
}

function sync_package_dansguardian() {
	global $config;

	#assign xml arrays
	if (is_array($config['installedpackages']['dansguardian']))
		$dansguardian=$config['installedpackages']['dansguardian']['config'][0];
	if (is_array($config['installedpackages']['dansguardianconfig']))
		$dansguardian_config=$config['installedpackages']['dansguardianconfig']['config'][0];
	if (is_array($config['installedpackages']['dansguardianlimits']))
		$dansguardian_limits=$config['installedpackages']['dansguardianlimits']['config'][0];
	if (is_array($config['installedpackages']['dansguardianlog']))
		$dansguardian_log=$config['installedpackages']['dansguardianlog']['config'][0];
	
	#daemon options
	$dansguardian_enabled=$dansguardian['enable_dg'];
	$filterports=($dansguardian['filterports']?$dansguardian['filterports']:"8080");
	$softrestart=(preg_match('/softrestart/',$dansguardian['daemon_options'])?"yes":"no");
	$nodaemon=(preg_match('/nodaemon/',$dansguardian['daemon_options'])?"yes":"no");
	if ($dansguardian['children'])
		list($min_children,$max_children) = split ("/", $dansguardian['children'], 2);
	else
		list($min_children,$max_children) = split ("/", "8/120", 2);
	if ($dansguardian['sparechildren'])
		list($min_spare_children,$max_spare_children) = split ("/", $dansguardian['sparechildren'], 2);
	else
		list($min_spare_children,$max_spare_children) = split ("/", "8/64", 2);	
	$maxagechildren=($dansguardian['maxagechildren']?$dansguardian['maxagechildren']:"500");
	$maxips=($dansguardian['maxips']?$dansguardian['maxips']:"0");

	
	#general options
	$urlcachenumber=($dansguardian_config['urlcachenumber']?$dansguardian_config['urlcachenumber']:"1000");
	$urlcacheage=($dansguardian_config['urlcacheage']?$dansguardian_config['urlcacheage']:"900");
	$scancleancache=(preg_match('/scancleancache/',$dansguardian_config['scan_options'])?"yes":"no");
	$hexdecodecontent=(preg_match('/hexdecodecontent/',$dansguardian_config['scan_options'])?"yes":"no");
	$forcequicksearch=(preg_match('/forcequicksearch/',$dansguardian_config['scan_options'])?"yes":"no");
	$reverseaddresslookups=(preg_match('/reverseaddresslookups/',$dansguardian_config['scan_options'])?"yes":"no");
	$reverseclientiplookups=(preg_match('/reverseclientiplookups/',$dansguardian_config['scan_options'])?"yes":"no");
	$logclienthostnames=(preg_match('/logclienthostnames/',$dansguardian_config['scan_options'])?"yes":"no");
	$createlistcachefiles=(preg_match('/createlistcachefiles/',$dansguardian_config['scan_options'])?"yes":"no");
	$prefercachedlists=(preg_match('/prefercachedlists/',$dansguardian_config['scan_options'])?"yes":"no");
	$deletedownloadedtempfiles=(preg_match('/deletedownloadedtempfiles/',$dansguardian_config['scan_options'])?"yes":"no");
	$weightedphrasemode=($dansguardian_config['weightedphrasemode']?$dansguardian_config['weightedphrasemode']:"2");
	$phrasefiltermode=($dansguardian_config['phrasefiltermode']?$dansguardian_config['phrasefiltermode']:"2");
	$preservecase=($dansguardian_config['preservecase']?$dansguardian_config['preservecase']:"0");
	$clamdscan=(preg_match('/clamdscan/',$dansguardian_config['content_scanners'])?"yes":"no");
	$icapscan=(preg_match('/icapscan/',$dansguardian_config['content_scanners'])?"yes":"no");
	$contentscannertimeout=($dansguardian_config['contentscannertimeout']?$dansguardian_config['contentscannertimeout']:"60");
	$contentscanexceptions=($dansguardian_config['contentscanexceptions']?"on":"off");
	$recheckreplacedurls=(preg_match('/recheckreplacedurls/',$dansguardian_config['misc_options'])?"yes":"no");
	$forwardedfor=(preg_match('/forwardedfor/',$dansguardian_config['misc_options'])?"yes":"no");
	$recheckreplacedurls=(preg_match('/icapscan/',$dansguardian_config['misc_options'])?"yes":"no");
	$usexforwardedfor=(preg_match('/usexforwardedfor/',$dansguardian_config['misc_options'])?"yes":"no");
	
	#limits
	$maxuploadsize=($dansguardian_limits['maxuploadsize']?$dansguardian_limits['maxuploadsize']:"-1");
	$maxcontentfiltersize=($dansguardian_limits['maxcontentfiltersize']?$dansguardian_limits['maxcontentfiltersize']:"256");
	$maxcontentramcachescansize=($dansguardian_limits['maxcontentramcachescansize']?$dansguardian_limits['maxcontentramcachescansize']:"2000");
	$maxcontentfilecachescansize=($dansguardian_limits['maxcontentfilecachescansize']?$dansguardian_limits['maxcontentfilecachescansize']:"1000");
	$initialtrickledelay=($dansguardian_limits['initialtrickledelay']?$dansguardian_limits['initialtrickledelay']:"20");
	$trickledelay=($dansguardian_limits['trickledelay']?$dansguardian_limits['trickledelay']:"20");
	
	#report and log
	$reportlevel=($dansguardian_log['report_level']?$dansguardian_log['report_level']:"3");
	$reportlanguage=($dansguardian_log['report_language']?$dansguardian_log['report_language']:"ukenglish");
	$showweightedfound=(preg_match('/showweightedfound/',$dansguardian_log['report_options'])?"on":"off");
	$usecustombannedflash=(preg_match('/usecustombannedflash/',$dansguardian_log['report_options'])?"on":"off");
	$usecustombannedimage=(preg_match('/usecustombannedimage/',$dansguardian_log['report_options'])?"on":"off");
	$nonstandarddelimiter=(preg_match('/nonstandarddelimiter/',$dansguardian_log['report_options'])?"on":"off");
	
	$logchildprocesshandling=(preg_match('/logchildprocesshandling/',$dansguardian_log['logging_options'])?"on":"off");
	$logconnectionhandlingerrors=(preg_match('/logconnectionhandlingerrors/',$dansguardian_log['logging_options'])?"on":"off");
	$nologger=(preg_match('/nologger/',$dansguardian_log['logging_options'])?"on":"off");
	$logadblocks=(preg_match('/logadblocks/',$dansguardian_log['logging_options'])?"on":"off");
	$anonymizelogs=(preg_match('/anonymizelogs/',$dansguardian_log['logging_options'])?"on":"off");

	$loglevel=($dansguardian_log['loglevel']?$dansguardian_log['loglevel']:"2");
	$logexceptionhits=($dansguardian_log['logexceptionhits']?$dansguardian_log['logexceptionhits']:"2");
	$logfileformat=($dansguardian_log['logfileformat']?$dansguardian_log['logfileformat']:"1");
	
	/*
Language Strings = %report-dir%/languages.conf
*/
	#check files
	$load_samples=0;
	$dansguardian_dir="/usr/local/etc/dansguardian";
	
	if($attachments['filename_rules'] == ""){
		$config['installedpackages']['msattachments']['config'][0]['filename_rules']=base64_encode(file_get_contents($dansguardian_dir.'/archives.filename.rules.conf.sample'));
		$load_samples++;
	}
	if($attachments['filetype_rules'] == ""){
		$config['installedpackages']['msattachments']['config'][0]['filetype_rules']=base64_encode(file_get_contents($dansguardian_dir.'/archives.filetype.rules.conf.sample'));
		$load_samples++;
	}
	if($content['phishing_safe'] == ""){
		$config['installedpackages']['mscontent']['config'][0]['phishing_safe']=base64_encode(file_get_contents($dansguardian_dir.'/phishing.safe.sites.conf.sample'));
		$load_samples++;
	}
	if($content['phishing_bad'] == ""){
		$config['installedpackages']['mscontent']['config'][0]['phishing_bad']=base64_encode(file_get_contents($dansguardian_dir.'/phishing.bad.sites.conf.sample'));
		$load_samples++;
	}	
	if($content['country_domains'] == ""){
		$config['installedpackages']['mscontent']['config'][0]['country_domains']=base64_encode(file_get_contents($dansguardian_dir.'/country.domains.conf.sample'));
		$load_samples++;
	}
	if($antispam['sa_pref_file'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['sa_pref_file']=base64_encode(file_get_contents($dansguardian_dir.'/spam.assassin.prefs.conf.sample'));
		$load_samples++;
	}
	if($antispam['rbl_file'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['rbl_file']=base64_encode(file_get_contents($dansguardian_dir.'/spam.lists.conf.sample'));
		$load_samples++;
	}
	if($antispam['mcp_pref_file'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['mcp_pref_file']=base64_encode(file_get_contents($dansguardian_dir.'/mcp/mcp.spam.assassin.prefs.conf.sample'));
		copy($dansguardian_dir.'/mcp/10_example.cf.sample',$dansguardian_dir.'/mcp/10_example.cf');
		copy($dansguardian_dir.'/mcp/v320.pre.sample',$dansguardian_dir.'/mcp/v320.pre');
		$load_samples++;
	}
	if($antispam['bounce'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['bounce']=base64_encode(file_get_contents($dansguardian_dir.'/rules/bounce.rules.sample'));
		$load_samples++;
	}
	if($antispam['spam_whitelist'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['spam_whitelist']=base64_encode(file_get_contents($dansguardian_dir.'/rules/spam.whitelist.rules.sample'));
		$load_samples++;
	}
	if($antispam['max_message_size'] == ""){
		$config['installedpackages']['msantispam']['config'][0]['max_message_size']=base64_encode(file_get_contents($dansguardian_dir.'/rules/max.message.size.rules.sample'));
		$load_samples++;
	}
	
	$report_dir="/usr/local/share/dansguardian/reports/".strtolower($report['language']);
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
			file_put_contents($report_dir.'/'.$file_r,dg_text_area_decode($config['installedpackages']['msreport']['config'][0][$key_r]),LOCK_EX);
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

		
#create dansguardian.conf
	$dg=<<<EOF
# DansGuardian config file for version 2.12.0.0

# **NOTE** as of version 2.7.5 most of the list files are now in dansguardianf1.conf


# Web Access Denied Reporting (does not affect logging)
#
# -1 = log, but do not block - Stealth mode
#  0 = just say 'Access Denied'
#  1 = report why but not what denied phrase
#  2 = report fully
#  3 = use HTML template file (accessdeniedaddress ignored) - recommended
#
reportinglevel = {$reportlevel}

# Language dir where languages are stored for internationalisation.
# The HTML template within this dir is only used when reportinglevel
# is set to 3. When used, DansGuardian will display the HTML file instead of
# using the perl cgi script.  This option is faster, cleaner
# and easier to customise the access denied page.
# The language file is used no matter what setting however.
#
languagedir = '/usr/local/share/dansguardian/languages'

# language to use from languagedir.
language = '{$reportlanguage}'

# Logging Settings
#
# 0 = none  1 = just denied  2 = all text based  3 = all requests
loglevel = {$loglevel}

# Log Exception Hits
# Log if an exception (user, ip, URL, phrase) is matched and so
# the page gets let through.  Can be useful for diagnosing
# why a site gets through the filter.
# 0 = never log exceptions
# 1 = log exceptions, but do not explicitly mark them as such
# 2 = always log & mark exceptions (default)
logexceptionhits = {$logexceptionhits}

# Log File Format
# 1 = DansGuardian format (space delimited)
# 2 = CSV-style format
# 3 = Squid Log File Format
# 4 = Tab delimited
logfileformat = {$logfileformat}

# truncate large items in log lines
# 0 = no truncating (default)
#maxlogitemlength = 0

# anonymize logs (blank out usernames & IPs)
anonymizelogs = {$anonymizelogs}


# Syslog logging
#
# Use syslog for access logging instead of logging to the file
# at the defined or built-in "loglocation"
#logsyslog = off

# Log file location
# 
# Defines the log directory and filename.
#loglocation = '/var/log/access.log'


# Statistics log file location
#
# Defines the stat file directory and filename.
# Only used in conjunction with maxips > 0
# Once every 3 minutes, the current number of IPs in the cache, and the most
# that have been in the cache since the daemon was started, are written to this
# file. IPs persist in the cache for 7 days.
#statlocation = '/var/log/stats'


# Network Settings
# 
# the IP that DansGuardian listens on.  If left blank DansGuardian will
# listen on all IPs.  That would include all NICs, loopback, modem, etc.
# Normally you would have your firewall protecting this, but if you want
# you can limit it to a certain IP. To bind to multiple interfaces,
# specify each IP on an individual filterip line.
# You can have the same IP twice so long as it has a different port.
filterip = {$filterip}

# the ports that DansGuardian listens to.  Specify one line per filterip
# line.  You can specify different authentication mechanisms per port but
# only if the mechanisms can co-exist (e.g. basic/proxy auth can't)
filterports = 8080
#filterports = 8081
{$filterports}

# the ip of the proxy (default is the loopback - i.e. this server)
proxyip = 127.0.0.1

# the port DansGuardian connects to proxy on
proxyport = 3128

# Whether to retrieve the original destination IP in transparent proxy
# setups and check it against the domain pulled from the HTTP headers.
#
# Be aware that when visiting sites which use a certain type of round-robin
# DNS for load balancing, DG may mark requests as invalid unless DG gets
# exactly the same answers to its DNS requests as clients.  The chances of
# this happening can be increased if all clients and servers on the same LAN
# make use of a local, caching DNS server instead of using upstream DNS
# directly.
#
# See http://www.kb.cert.org/vuls/id/435052
# on (default) | off
#!! Not compiled !! originalip = on

# accessdeniedaddress is the address of your web server to which the cgi
# dansguardian reporting script was copied. Only used in reporting levels 1 and 2.
#
# This webserver must be either:
#  1. Non-proxied. Either a machine on the local network, or listed as an exception
#     in your browser's proxy configuration.
#  2. Added to the exceptionsitelist. Option 1 is preferable; this option is
#     only for users using both transparent proxying and a non-local server
#     to host this script.
#
# Individual filter groups can override this setting in their own configuration.
#
accessdeniedaddress = 'http://YOURSERVER.YOURDOMAIN/cgi-bin/dansguardian.pl'

# Non standard delimiter (only used with accessdeniedaddress)
# To help preserve the full banned URL, including parameters, the variables
# passed into the access denied CGI are separated using non-standard
# delimiters. This can be useful to ensure correct operation of the filter
# bypass modes. Parameters are split using "::" in place of "&", and "==" in
# place of "=".
# Default is enabled, but to go back to the standard mode, disable it.
nonstandarddelimiter = {$nonstandarddelimiter}



# Banned image replacement
# Images that are banned due to domain/url/etc reasons including those
# in the adverts blacklists can be replaced by an image.  This will,
# for example, hide images from advert sites and remove broken image
# icons from banned domains.
# on (default) | off
usecustombannedimage = {$usecustombannedimage}
custombannedimagefile = '/usr/local/share/dansguardian/transparent1x1.gif'


#Banned flash replacement
usecustombannedflash = {$usecustombannedflash}
custombannedflashfile = '/usr/local/share/dansguardian/blockedflash.swf'



# Filter groups options
# filtergroups sets the number of filter groups. A filter group is a set of content
# filtering options you can apply to a group of users.  The value must be 1 or more.
# DansGuardian will automatically look for dansguardianfN.conf where N is the filter
# group.  To assign users to groups use the filtergroupslist option.  All users default
# to filter group 1.  You must have some sort of authentication to be able to map users
# to a group.  The more filter groups the more copies of the lists will be in RAM so
# use as few as possible.
filtergroups = 1
filtergroupslist = '/usr/local/etc/dansguardian/lists/filtergroupslist'



# Authentication files location
bannediplist = '/usr/local/etc/dansguardian/lists/bannediplist'
exceptioniplist = '/usr/local/etc/dansguardian/lists/exceptioniplist'

# Per-Room blocking definition directory
# A directory containing text files containing the room's name followed by IPs or ranges
# Think of it as bannediplist on crack
perroomblockingdirectory = '/usr/local/etc/dansguardian/lists/bannedrooms/'

# Show weighted phrases found
# If enabled then the phrases found that made up the total which excedes
# the naughtyness limit will be logged and, if the reporting level is
# high enough, reported. on | off
showweightedfound = {$showweightedfound}

# Weighted phrase mode
# There are 3 possible modes of operation:
# 0 = off = do not use the weighted phrase feature.
# 1 = on, normal = normal weighted phrase operation.
# 2 = on, singular = each weighted phrase found only counts once on a page.
# 
# IMPORTANT: Note that setting this to "0" turns off all features which
# extract phrases from page content, including banned & exception
# phrases (not just weighted), search term filtering, and scanning for
# links to banned URLs.
#
weightedphrasemode = {$weightedphrasemode}



# Positive (clean) result caching for URLs
# Caches good pages so they don't need to be scanned again.
# It also works with AV plugins.
# 0 = off (recommended for ISPs with users with disimilar browsing)
# 1000 = recommended for most users
# 5000 = suggested max upper limit
# If you're using an AV plugin then use at least 5000.
urlcachenumber = {$urlcachenumber}
#
# Age before they are stale and should be ignored in seconds
# 0 = never
# 900 = recommended = 15 mins
urlcacheage ={$urlcacheage} 



# Cache for content (AV) scan results as 'clean'
# By default, to save CPU, files scanned and found to be
# clean are inserted into the clean cache and NOT scanned
# again for a while.  If you don't like this then choose
# to disable it.
# on = cache results; do not re-scan
# off = do not cache; always re-scan
# (on|off) default = on.
scancleancache = {$scancleancache}



# Smart, Raw and Meta/Title phrase content filtering options
# Smart is where the multiple spaces and HTML are removed before phrase filtering
# Raw is where the raw HTML including meta tags are phrase filtered
# Meta/Title is where only meta and title tags are phrase filtered (v. quick)
# CPU usage can be effectively halved by using setting 0 or 1 compared to 2
# 0 = raw only
# 1 = smart only
# 2 = both of the above (default)
# 3 = meta/title
phrasefiltermode = {$phrasefiltermode}

# Lower casing options
# When a document is scanned the uppercase letters are converted to lower case
# in order to compare them with the phrases.  However this can break Big5 and
# other 16-bit texts.  If needed preserve the case.  As of version 2.7.0 accented
# characters are supported.
# 0 = force lower case (default)
# 1 = do not change case
# 2 = scan first in lower case, then in original case
preservecase = {$preservecase}

# Note:
# If phrasefiltermode and preserve case are both 2, this equates to 4 phrase
# filtering passes. If you have a large enough userbase for this to be a
# worry, and need to filter pages in exotic character encodings, it may be
# better to run two instances on separate servers: one with preservecase 1
# (and possibly forcequicksearch 1) and non ASCII/UTF-8 phrase lists, and one
# with preservecase 0 and ASCII/UTF-8 lists.



# Hex decoding options
# When a document is scanned it can optionally convert %XX to chars.
# If you find documents are getting past the phrase filtering due to encoding
# then enable.  However this can break Big5 and other 16-bit texts.
# off = disabled (default)
# on = enabled
hexdecodecontent = {$hexdecodecontent}



# Force Quick Search rather than DFA search algorithm
# The current DFA implementation is not totally 16-bit character compatible
# but is used by default as it handles large phrase lists much faster.
# If you wish to use a large number of 16-bit character phrases then
# enable this option.
# off (default) | on (Big5 compatible)
forcequicksearch = {$forcequicksearch}



# Reverse lookups for banned site and URLs.
# If set to on, DansGuardian will look up the forward DNS for an IP URL
# address and search for both in the banned site and URL lists.  This would
# prevent a user from simply entering the IP for a banned address.
# It will reduce searching speed somewhat so unless you have a local caching
# DNS server, leave it off and use the Blanket IP Block option in the
# bannedsitelist file instead.
reverseaddresslookups = {$reverseaddresslookups}



# Reverse lookups for banned and exception IP lists.
# If set to on, DansGuardian will look up the forward DNS for the IP
# of the connecting computer.  This means you can put in hostnames in
# the exceptioniplist and bannediplist.
# If a client computer is matched against an IP given in the lists, then the
# IP will be recorded in any log entries; if forward DNS is successful and a
# match occurs against a hostname, the hostname will be logged instead.
# It will reduce searching speed somewhat so unless you have a local DNS server, 
# leave it off.
reverseclientiplookups = {$reverseclientiplookups}


# Perform reverse lookups on client IPs for successful requests.
# If set to on, DansGuardian will look up the forward DNS for the IP
# of the connecting computer, and log host names (where available) rather than
# IPs against requests.
# This is not dependent on reverseclientiplookups being enabled; however, if it
# is, enabling this option does not incur any additional forward DNS requests.
logclienthostnames = {$logclienthostnames}


# Build bannedsitelist and bannedurllist cache files.
# This will compare the date stamp of the list file with the date stamp of
# the cache file and will recreate as needed.
# If a .processed file exists for an item (e.g. domain/URL) list, then that
# will be used instead, if it is up to date (i.e. newer than the unprocessed
# list file).
# This can increase process start speed on slow computers.
# Fast computers do not need this option.
# on | off, default = on
createlistcachefiles = {$createlistcachefiles}


# Prefer cached list files
# If enabled, DansGuardian will always prefer to load ".processed" versions of
# list files, regardless of their time stamps relative to the original
# unprocessed lists.  This is not generally useful unless you have a specific
# list update process which results in - for example - up-to-date, pre-sorted
# ".processed" list files with dummy unprocessed files.
# on | off, default = off
prefercachedlists = {$prefercachedlists}



# POST protection (web upload and forms)
# does not block forms without any file upload, i.e. this is just for
# blocking or limiting uploads
# measured in kibibytes after MIME encoding and header bumph
# use 0 for a complete block
# use higher (e.g. 512 = 512Kbytes) for limiting
# use -1 for no blocking
#maxuploadsize = 512
#maxuploadsize = 0
maxuploadsize = {$maxuploadsize}



# Max content filter size
# Sometimes web servers label binary files as text which can be very
# large which causes a huge drain on memory and cpu resources.
# To counter this, you can limit the size of the document to be
# filtered and get it to just pass it straight through.
# This setting also applies to content regular expression modification.
# The value must not be higher than maxcontentramcachescansize
# The size is in Kibibytes - eg 2048 = 2Mb
# use 0 to set it to maxcontentramcachescansize
maxcontentfiltersize = {$maxcontentfiltersize}



# Max content ram cache scan size
# This is only used if you use a content scanner plugin such as AV
# This is the max size of file that DG will download and cache
# in RAM.  After this limit is reached it will cache to disk
# This value must be less than or equal to maxcontentfilecachescansize.
# The size is in Kibibytes - eg 10240 = 10Mb
# use 0 to set it to maxcontentfilecachescansize
# This option may be ignored by the configured download manager.
maxcontentramcachescansize = {$maxcontentramcachescansize}



# Max content file cache scan size
# This is only used if you use a content scanner plugin such as AV
# This is the max size file that DG will download
# so that it can be scanned or virus checked.
# This value must be greater or equal to maxcontentramcachescansize.
# The size is in Kibibytes - eg 10240 = 10Mb
maxcontentfilecachescansize = {$maxcontentfilecachescansize}



# File cache dir
# Where DG will download files to be scanned if too large for the
# RAM cache.
filecachedir = '/tmp'



# Delete file cache after user completes download
# When a file gets save to temp it stays there until it is deleted.
# You can choose to have the file deleted when the user makes a sucessful
# download.  This will mean if they click on the link to download from
# the temp store a second time it will give a 404 error.
# You should configure something to delete old files in temp to stop it filling up.
# on|off (defaults to on)
deletedownloadedtempfiles = {$deletedownloadedtempfiles}



# Initial Trickle delay
# This is the number of seconds a browser connection is left waiting
# before first being sent *something* to keep it alive.  The
# *something* depends on the download manager chosen.
# Do not choose a value too low or normal web pages will be affected.
# A value between 20 and 110 would be sensible
# This may be ignored by the configured download manager.
initialtrickledelay = {$initialtrickledelay}



# Trickle delay
# This is the number of seconds a browser connection is left waiting
# before being sent more *something* to keep it alive.  The
# *something* depends on the download manager chosen.
# This may be ignored by the configured download manager.
trickledelay = {$trickledelay}



# Download Managers
# These handle downloads of files to be filtered and scanned.
# They differ in the method they deal with large downloads.
# Files usually need to be downloaded 100% before they can be
# filtered and scanned before being sent on to the browser.
# Normally the browser can just wait, but with content scanning,
# for example to AV, the browser may timeout or the user may get
# confused so the download manager has to do some sort of
# 'keep alive'.
#
# There are various methods possible but not all are included.
# The author does not have the time to write them all so I have
# included a plugin systam.  Also, not all methods work with all
# browsers and clients.  Specifically some fancy methods don't
# work with software that downloads updates.  To solve this,
# each plugin can support a regular expression for matching
# the client's user-agent string, and lists of the mime types
# and extensions it should manage.
#
# Note that these are the matching methods provided by the base plugin
# code, and individual plugins may override or add to them.
# See the individual plugin conf files for supported options.
#
# The plugins are matched in the order you specify and the last
# one is forced to match as the default, regardless of user agent
# and other matching mechanisms.
#
downloadmanager = '/usr/local/etc/dansguardian/downloadmanagers/fancy.conf'
##!! Not compiled !! downloadmanager = '/usr/local/etc/dansguardian/downloadmanagers/trickle.conf'
downloadmanager = '/usr/local/etc/dansguardian/downloadmanagers/default.conf'



# Content Scanners (Also known as AV scanners)
# These are plugins that scan the content of all files your browser fetches
# for example to AV scan.  The options are limitless.  Eventually all of
# DansGuardian will be plugin based.  You can have more than one content
# scanner. The plugins are run in the order you specify.
# This is one of the few places you can have multiple options of the same name.
#
# Some of the scanner(s) require 3rd party software and libraries eg clamav.
# See the individual plugin conf file for more options (if any).
#
#contentscanner = '/usr/local/etc/dansguardian/contentscanners/clamdscan.conf'
#!! Not compiled !! contentscanner = '/usr/local/etc/dansguardian/contentscanners/avastdscan.conf'
#!! Not compiled !! contentscanner = '/usr/local/etc/dansguardian/contentscanners/kavdscan.conf'
#contentscanner = '/usr/local/etc/dansguardian/contentscanners/icapscan.conf'
#!! Not compiled !! contentscanner = '/usr/local/etc/dansguardian/contentscanners/commandlinescan.conf'



# Content scanner timeout
# Some of the content scanners support using a timeout value to stop
# processing (eg AV scanning) the file if it takes too long.
# If supported this will be used.
# The default of 60 seconds is probably reasonable.
contentscannertimeout = {$contentscannertimeout}



# Content scan exceptions
# If 'on' exception sites, urls, users etc will be scanned
# This is probably not desirable behavour as exceptions are
# supposed to be trusted and will increase load.
# Correct use of grey lists are a better idea.
# (on|off) default = off
contentscanexceptions = {$contentscanexceptions}



# Auth plugins
# These replace the usernameidmethod* options in previous versions. They
# handle the extraction of client usernames from various sources, such as
# Proxy-Authorisation headers and ident servers, enabling requests to be
# handled according to the settings of the user's filter group.
# Multiple plugins can be specified, and will be used per port in the order
# filterports are listed.
#
# If you do not use multiple filter groups, you need not specify this option.
#
#authplugin = '/usr/local/etc/dansguardian/authplugins/proxy-basic.conf'
#authplugin = '/usr/local/etc/dansguardian/authplugins/proxy-digest.conf'
#authplugin = '/usr/local/etc/dansguardian/authplugins/proxy-ntlm.conf'
#authplugin = '/usr/local/etc/dansguardian/authplugins/ident.conf'
#authplugin = '/usr/local/etc/dansguardian/authplugins/ip.conf'



# Re-check replaced URLs
# As a matter of course, URLs undergo regular expression search/replace (urlregexplist)
# *after* checking the exception site/URL/regexpURL lists, but *before* checking against
# the banned site/URL lists, allowing certain requests that would be matched against the
# latter in their original state to effectively be converted into grey requests.
# With this option enabled, the exception site/URL/regexpURL lists are also re-checked
# after replacement, making it possible for URL replacement to trigger exceptions based
# on them.
# Defaults to off.
recheckreplacedurls = {$recheckreplacedurls}



# Misc settings

# if on it adds an X-Forwarded-For: <clientip> to the HTTP request
# header.  This may help solve some problem sites that need to know the
# source ip. on | off
forwardedfor = {$forwardedfor}


# if on it uses the X-Forwarded-For: <clientip> to determine the client
# IP. This is for when you have squid between the clients and DansGuardian.
# Warning - headers are easily spoofed. on | off
usexforwardedfor = {usexforwardedfor}


# if on it logs some debug info regarding fork()ing and accept()ing which
# can usually be ignored.  These are logged by syslog.  It is safe to leave
# it on or off
logconnectionhandlingerrors = {$logconnectionhandlingerrors}



# Fork pool options

# If on, this causes DG to write to the log file whenever child processes are
# created or destroyed (other than by crashes). This information can help in
# understanding and tuning the following parameters, but is not generally
# useful in production.
logchildprocesshandling = {$logchildprocesshandling}

# sets the maximum number of processes to spawn to handle the incoming
# connections.  Max value usually 250 depending on OS.
# On large sites you might want to try 180.
maxchildren = {$maxchildren}


# sets the minimum number of processes to spawn to handle the incoming connections.
# On large sites you might want to try 32.
minchildren = {$minchildren}


# sets the minimum number of processes to be kept ready to handle connections.
# On large sites you might want to try 8.
minsparechildren = {$minsparechildren}


# sets the minimum number of processes to spawn when it runs out
# On large sites you might want to try 10.
preforkchildren = {$preforkchildren}


# sets the maximum number of processes to have doing nothing.
# When this many are spare it will cull some of them.
# On large sites you might want to try 64.
maxsparechildren = {$maxsparechildren}


# sets the maximum age of a child process before it croaks it.
# This is the number of connections they handle before exiting.
# On large sites you might want to try 10000.
maxagechildren = {$maxagechildren}


# Sets the maximum number client IP addresses allowed to connect at once.
# Use this to set a hard limit on the number of users allowed to concurrently
# browse the web. Set to 0 for no limit, and to disable the IP cache process.
maxips = {$maxips}



# Process options
# (Change these only if you really know what you are doing).
# These options allow you to run multiple instances of DansGuardian on a single machine.
# Remember to edit the log file path above also if that is your intention.

# IPC filename
# 
# Defines IPC server directory and filename used to communicate with the log process.
ipcfilename = '/tmp/.dguardianipc'

# URL list IPC filename
# 
# Defines URL list IPC server directory and filename used to communicate with the URL
# cache process.
urlipcfilename = '/tmp/.dguardianurlipc'

# IP list IPC filename
#
# Defines IP list IPC server directory and filename, for communicating with the client
# IP cache process.
ipipcfilename = '/tmp/.dguardianipipc'

# PID filename
# 
# Defines process id directory and filename.
#pidfilename = '/var/run/dansguardian.pid'

# Disable daemoning
# If enabled the process will not fork into the background.
# It is not usually advantageous to do this.
# on|off (defaults to off)
nodaemon = {$nodaemon}

# Disable logging process
# on|off (defaults to off)
nologger = {$nologger}

# Enable logging of "ADs" category blocks
# on|off (defaults to off)
logadblocks = {$logadblocks}

# Enable logging of client User-Agent
# Some browsers will cause a *lot* of extra information on each line!
# on|off (defaults to off)
loguseragent = {$loguseragent}

# Daemon runas user and group
# This is the user that DansGuardian runs as.  Normally the user/group nobody.
# Uncomment to use.  Defaults to the user set at compile time.
# Temp files created during virus scanning are given owner and group read
# permissions; to use content scanners based on external processes, such as
# clamdscan, the two processes must run with either the same group or user ID.
#daemonuser = 'nobody'
#daemongroup = 'nobody'

# Soft restart
# When on this disables the forced killing off all processes in the process group.
# This is not to be confused with the -g run time option - they are not related.
# on|off (defaults to off)
softrestart = {softrestart}

# Mail program
# Path (sendmail-compatible) email program, with options.
# Not used if usesmtp is disabled (filtergroup specific).
#!! Not compiled !!mailer = '/usr/sbin/sendmail -t'

#SSL certificate checking path
#Path to CA certificates used to validate the certificates of https sites.
#sslcertificatepath = '/etc/ssl/certs/'

#SSL man in the middle
#CA certificate path
#Path to the CA certificate to use as a signing certificate for 
#generated certificates.
#cacertificatepath = '/home/stephen/dginstall/ca.pem'

#CA private key path
#path to the private key that matches the public key in the CA certificate.
#caprivatekeypath = '/home/stephen/dginstall/ca.key'

#Cert private key path
#The public / private key pair used by all generated certificates
#certprivatekeypath = '/home/stephen/dginstall/cert.key'

#Generated cert path
#The location where generated certificates will be saved for future use.
#(must be writable by the dg user)
#generatedcertpath = '/home/stephen/dginstall/generatedcerts/'

#Generated link path = ''
#The location where symlinks to certificates will be created.
#(must be writable by the dg user)
#generatedlinkpath = '/home/stephen/dginstall/generatedlinks/'
	
EOF;
	#write files
	conf_mount_rw();

	$mlang=strtolower($report['language']);
	$mfiles[]="/usr/local/etc/dansguardian/virus.scanners.conf";
	$mfiles[]="/usr/local/share/dansguardian/reports/{$mlang}/inline.spam.warning.txt";
	$mfiles[]="/usr/local/share/dansguardian/reports/{$mlang}/languages.conf";
	
	foreach ($mfiles as $mfile)
		if (! file_exists ($mfile))
			copy($mfile.".sample",$mfile);
			

	write_config();
	
	file_put_contents($dansguardian_dir."/dansguardian.conf", $mc, LOCK_EX);
	file_put_contents($dansguardian_dir."/filename.rules.conf",dg_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filename_rules']),LOCK_EX);
	file_put_contents($dansguardian_dir."/filetype.rules.conf",dg_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filetype_rules']),LOCK_EX);
	file_put_contents($dansguardian_dir."/archives.filename.rules.conf",dg_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filename_rules']),LOCK_EX);
	file_put_contents($dansguardian_dir."/archives.filetype.rules.conf",dg_text_area_decode($config['installedpackages']['msattachments']['config'][0]['filetype_rules']),LOCK_EX);
	file_put_contents($dansguardian_dir."/phishing.safe.sites.conf",dg_text_area_decode($config['installedpackages']['mscontent']['config'][0]['phishing_safe']),LOCK_EX);
	file_put_contents($dansguardian_dir."/phishing.bad.sites.conf",dg_text_area_decode($config['installedpackages']['mscontent']['config'][0]['phishing_bad']),LOCK_EX);
	file_put_contents($dansguardian_dir."/country.domains.conf",dg_text_area_decode($config['installedpackages']['mscontent']['config'][0]['country_domains']),LOCK_EX);
	file_put_contents($dansguardian_dir.'/spam.assassin.prefs.conf',$sa_temp,LOCK_EX);
	file_put_contents($dansguardian_dir.'/spam.lists.conf',dg_text_area_decode($config['installedpackages']['msantispam']['config'][0]['rbl_file']),LOCK_EX);
	file_put_contents($dansguardian_dir.'/mcp/mcp.spam.assassin.prefs.conf',dg_text_area_decode($config['installedpackages']['msantispam']['config'][0]['mcp_pref_file']),LOCK_EX);
	file_put_contents($dansguardian_dir.'/rules/bounce.rules',dg_text_area_decode($config['installedpackages']['msantispam']['config'][0]['bounce']),LOCK_EX);
	file_put_contents($dansguardian_dir.'/rules/max.message.size.rules',dg_text_area_decode($config['installedpackages']['msantispam']['config'][0]['max_message_size']),LOCK_EX);
	file_put_contents($dansguardian_dir.'/rules/spam.whitelist.rules',dg_text_area_decode($config['installedpackages']['msantispam']['config'][0]['spam_whitelist']),LOCK_EX);
	
	foreach ($report_files as $key_r => $file_r)
		file_put_contents($report_dir.'/'.$file_r,dg_text_area_decode($config['installedpackages']['msreport']['config'][0][$key_r]),LOCK_EX);
		
	if ($alert['sig']){
		$sig_html=dg_text_area_decode($config['installedpackages']['msalerts']['config'][0]['sig_html']);
		$sig_txt=dg_text_area_decode($config['installedpackages']['msalerts']['config'][0]['sig_txt']);}
	else{
		$sig_html="";
		$sig_txt="";}
	file_put_contents($report_dir.'/inline.sig.txt',$sig_txt,LOCK_EX);
	file_put_contents($report_dir.'/inline.sig.html',$sig_html,LOCK_EX);

	if ($alert['warning']){
		$warning_html=dg_text_area_decode($config['installedpackages']['msalerts']['config'][0]['warning_html']);
		$warning_txt=dg_text_area_decode($config['installedpackages']['msalerts']['config'][0]['warning_txt']);}
	else{
		$warning_html="";
		$warning_txt="";}
	file_put_contents($report_dir.'/inline.warning.txt',$warning_txt,LOCK_EX);
	file_put_contents($report_dir.'/inline.warning.html',$warning_html,LOCK_EX);
	
	#check virus_scanner options
	$libexec_dir="/usr/local/libexec/dansguardian/";
	if ($virus_scanning == "yes"){
		if ($antivirus['virus_scanner'] =="none"){
			unlink_if_exists($libexec_dir.'clamav-autoupdate');
			unlink_if_exists($libexec_dir.'clamav-wrapper');
		}
		else{
			if (file_exists('/var/run/clamav/'))
				chown('/var/run/clamav/', 'dansguardian');
			if (file_exists('/var/log/clamav/'))
				chown('/var/log/clamav/', 'dansguardian');
			if (file_exists('/var/db/clamav/'))
				chown('/var/db/clamav/', 'dansguardian');
			if (file_exists('/var/db/clamav/bytecode.cld'))
				chown('/var/db/clamav/bytecode.cld', 'dansguardian');
			if (file_exists('/var/db/clamav/daily.cld'))
				chown('/var/db/clamav/daily.cld', 'dansguardian');
			if (file_exists('/var/db/clamav/main.cvd'))
				chown('/var/db/clamav/main.cvd', 'dansguardian');
			if (file_exists('/var/db/clamav/mirrors.dat'))
				chown('/var/db/clamav/mirrors.dat', 'dansguardian');
			if (file_exists('/var/log/clamav/clamd.log'))
				chown('/var/log/clamav/clamd.log', 'dansguardian');
			if (file_exists('/var/log/clamav/freshclam.log'))
				chown('/var/log/clamav/freshclam.log', 'dansguardian');
			
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
				$cconf_file=preg_replace('/"clamav"/','"dansguardian"',$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
				
			#freshclam conf file
			$cconf="/usr/local/etc/freshclam.conf";
			$cconf_file=file_get_contents($cconf);
			if (preg_match('/DatabaseOwner clamav/',$cconf_file)){
				$cconf_file=preg_replace("/DatabaseOwner clamav/","DatabaseOwner dansguardian",$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
				
			#clamd conf file
			$cconf="/usr/local/etc/clamd.conf";
			$cconf_file=file_get_contents($cconf);
			if (preg_match('/User clamav/',$cconf_file)){
				$cconf_file=preg_replace("/User clamav/","User dansguardian",$cconf_file);
				file_put_contents($cconf, $cconf_file, LOCK_EX);
				}
			#clamd script file
			$script='/usr/local/etc/rc.d/clamav-clamd';
			$script_file=file($script);
			foreach ($script_file as $script_line){
				if(preg_match("/command=/",$script_line)){
					$new_clamav_startup.= "/bin/mkdir /var/run/clamav\n";
					$new_clamav_startup.= "chown dansguardian /var/run/clamav\n";
					$new_clamav_startup.=$script_line;
					}
				elseif(!preg_match("/(mkdir|chown|sleep|dansguardian)/",$script_line)) {
					$new_clamav_startup.=preg_replace("/NO/","YES",$script_line);
					}
				}
			file_put_contents($script, $new_clamav_startup, LOCK_EX);
			chmod ($script,0755);
			mwexec("$script stop");
			mwexec_bg("$script start");
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
	#check dcc config file
			$script='/usr/local/dcc/dcc_conf';
			$script_file=file_get_contents($script);
			if (preg_match('/DCCIFD_ENABLE=off/',$script_file)){
				$script_file=preg_replace("/DCCIFD_ENABLE=off/","DCCIFD_ENABLE=on",$script_file);
				file_put_contents($script, $script_file, LOCK_EX);
				}
			mwexec("$script stop");
			mwexec_bg("$script start");
				
	$script='/usr/local/etc/rc.d/dansguardian';

	#fix MIME::ToolUtils deprecated function and usecure dependency calls in /usr/local/sbin/dansguardian
	$cconf="/usr/local/sbin/dansguardian";
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
				chmod ($script,0755);
				}
	if($config['installedpackages']['dansguardian']['config'][0]['enable']){
		log_error("Reload dansguardian");
		chmod ($script,0755);
		mwexec("$script stop");
        sleep(2);
        mwexec_bg("$script start");
	}
	else{
		log_error("Stopping dansguardian if running");
		mwexec("$script stop");
		chmod ($script,0444);
	}
	conf_mount_ro();
	$synconchanges = $config['installedpackages']['dansguardiansync']['config'][0]['synconchanges'];
	if(!$synconchanges && !$syncondbchanges) 
		return;
	log_error("[dansguardian] dansguardian_xmlrpc_sync.php is starting.");
	foreach ($config['installedpackages']['dansguardiansync']['config'] as $rs ){
		foreach($rs['row'] as $sh){
		$sync_to_ip = $sh['ipaddress'];
		$password   = $sh['password'];
		$sync_type = $sh['sync_type'];
		if($password && $sync_to_ip)
			dansguardian_do_xmlrpc_sync($sync_to_ip, $password,$sync_type);
		}
	}
	log_error("[dansguardian] dansguardian_xmlrpc_sync.php is ending.");

}

function dansguardian_validate_input($post, &$input_errors) {
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

function dansguardian_php_install_command() {
	sync_package_dansguardian();
}

function dansguardian_php_deinstall_command() {
	mwexec("/usr/local/etc/rc.d/dansguardian.sh stop");
	sleep(1);
	conf_mount_rw();
	unlink_if_exists("/usr/local/etc/rc.d/dansguardian.sh");
	conf_mount_ro();
}

function dansguardian_do_xmlrpc_sync($sync_to_ip, $password,$sync_type) {
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
	$sync_xml=$config['installedpackages']['dansguardiansync']['config'][0]['synconchanges'];
	if ($sync_xml){
		log_error("Include dansguardian config");
		$xml['dansguardian'] = $config['installedpackages']['dansguardian'];
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
		log_error("Beginning dansguardian XMLRPC sync to {$url}:{$port}.");
		$method = 'pfsense.merge_installedpackages_section_xmlrpc';
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials('admin', $password);
		if($g['debug'])
			$cli->setDebug(1);
		/* send our XMLRPC message and timeout after 250 seconds */
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while attempting dansguardian XMLRPC sync with {$url}:{$port}.";
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while attempting dansguardian XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} else {
			log_error("dansguardian XMLRPC sync successfully completed with {$url}:{$port}.");
		}
		
		/* tell dansguardian to reload our settings on the destionation sync host. */
		$method = 'pfsense.exec_php';
		$execcmd  = "require_once('/usr/local/pkg/dansguardian.inc');\n";
		$execcmd .= "sync_package_dansguardian();";
		
		/* assemble xmlrpc payload */
		$params = array(
			XML_RPC_encode($password),
			XML_RPC_encode($execcmd)
		);
	
		log_error("dansguardian XMLRPC reload data {$url}:{$port}.");
		$msg = new XML_RPC_Message($method, $params);
		$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
		$cli->setCredentials('admin', $password);
		$resp = $cli->send($msg, "250");
		if(!$resp) {
			$error = "A communications error occurred while attempting dansguardian XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} elseif($resp->faultCode()) {
			$cli->setDebug(1);
			$resp = $cli->send($msg, "250");
			$error = "An error code was received while attempting dansguardian XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
			log_error($error);
			file_notice("sync_settings", $error, "dansguardian Settings Sync", "");
		} else {
			log_error("dansguardian XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
		}
	}
}

?>
