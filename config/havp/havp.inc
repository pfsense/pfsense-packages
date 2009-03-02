<?php
/*
        havp.inc
        Part of pfSense package
        Copyright (C) 2008 Serg Dvorianceev
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

/* ! ÍÀVP v.0.88 !*/

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');

# defines
define('HV_USER',          'havp');
define('HV_AV_USER',       'havp');

define('HV_WORKDIR',       '/usr/local/etc/havp');
define('HV_CONFIGFILE',    '/havp.config');
define('HV_WHITELISTFILE', '/whitelist');
define('HV_BLACKLISTFILE', '/blacklist');

define('HV_PIDFILE',       '/var/run/havp.pid');
define('HV_LOGDIR',        '/var/log/havp');
define('HV_AVLOGDIR',      '/var/log/clamav');
define('HV_ACCESSLOG',     '/access.log');
define('HV_LOG',           '/havp.log');
define('HV_TEMPDIR',       '/var/tmp');
define('HV_HAVPTEMPDIR',   '/var/tmp/havp');
define('HV_SCANTEMPFILE',  '/havp/havp-XXXXXX');
define('HV_DEFAULTPORT',   '3125');
define('HV_DEFAULTADDR',   '127.0.0.1');
define('HV_TEMPLATEPATH',  '/usr/local/share/examples/havp/templates');
define('HV_CRONNAME_AVUPD','havp_av_update');
define('HV_CRONCMD_AVUPD', '/usr/local/etc/rc.d/clamav-freshclam start');
define('HV_CRONKEY_AVUPD', '/clamav-freshclam');
define('HV_AVUPD_SCRIPT',  '/usr/local/etc/rc.d/havp_avupdate.sh');
define('HV_FRESHCLAM_CONFIGFILE', '/usr/local/etc/freshclam.conf');

# XML fields
define('XML_HAVPENABLE',        'enable');
define('XML_HAVPLANG',          'havplang');
define('XML_PARENTPROXY',       'parentproxy');
define('XML_WHITELIST',         'whitelist');
define('XML_BLACKLIST',         'blacklist');
define('XML_PROXYIFACE',        'proxyiface');
define('XML_PROXYPORT',         'proxyport');
define('XML_USEEXTIFACE',       'listenextinterface');
define('XML_XFORWARDEDFOR',     'xforwardedfor');
define('XML_FAILSCANERROR',     'failscanerror');
define('XML_LANGUAGE',          'lang');
define('XML_SCANIMG',           'scanimg');
define('XML_SCANARC',           'scanarc');
define('XML_SCANMAXSIZE',       'scanmaxsize');
define('XML_MAXDOWNLOADSIZE',   'maxdownloadsize');
define('XML_SYSLOG',            'syslog');
define('XML_HAVPUPDATE',        'havpavupdate');
# define('','');

function havp_install(){
    havp_check_system();
}

function havp_deinstall() {
    havp_setup_cron(HV_CRONNAME_AVUPD,"", "");
    mwexec("rm -rf " . HV_AVUPD_SCRIPT);
    mwexec("rm -rf " . HV_PIDFILE);
}

function havp_resync() {
    global $config;
    $pfconf = $config['installedpackages']['havp']['config'][0];

    havp_check_system();

    # whitelist and blacklist
    # also white-listed:
    $whitelist = havp_whitelist_def() . "\n" . str_replace(" ", "\n", base64_decode($pfconf[XML_WHITELIST]));
    $blacklist = str_replace(" ", "\n", base64_decode($pfconf[XML_BLACKLIST]));
    # stupid havp parser - error on 0x0D:
    $whitelist = str_replace("\r", "", $whitelist);
    $blacklist = str_replace("\r", "", $blacklist);
    file_put_contents(HV_WORKDIR . HV_WHITELISTFILE, $whitelist);
    file_put_contents(HV_WORKDIR . HV_BLACKLISTFILE, $blacklist);

    # config havp
    file_put_contents(HV_WORKDIR . HV_CONFIGFILE, havp_config());
    set_file_access(HV_WORKDIR, HV_USER, '0755');

    # config freshclam
    file_put_contents(HV_FRESHCLAM_CONFIGFILE, havp_config_freshclam());
    set_file_access(HV_FRESHCLAM_CONFIGFILE, HV_AV_USER, '0664');

    # cron task
    $on = false;
    $opt = array("0", "*", "*", "*", "*", "root", "/usr/bin/nice -n20 " . HV_AVUPD_SCRIPT);
    switch($pfconf['havpavupdate']) {
        case 'none':   $on = false; break;
        case 'hv_01h': $on = true;  $opt[1]= "*/1";  break;
        case 'hv_02h': $on = true;  $opt[1]= "*/2";  break;
        case 'hv_03h': $on = true;  $opt[1]= "*/3";  break;
        case 'hv_04h': $on = true;  $opt[1]= "*/4";  break;
        case 'hv_06h': $on = true;  $opt[1]= "*/6";  break;
        case 'hv_08h': $on = true;  $opt[1]= "*/8";  break;
        case 'hv_12h': $on = true;  $opt[1]= "*/12"; break;
        case 'hv_24h': $on = true;  $opt[1]= "0";    break;
        default:       break;
    }
    havp_setup_cron(HV_CRONNAME_AVUPD, $opt, $on);

    mwexec("killall havp");
    mwexec("killall havp");
    mwexec("/usr/local/sbin/havp -c /usr/local/etc/havp" . HV_CONFIGFILE);

mountRAMdisk();

}

function havp_check_system() {

    # workdir permissions
    set_file_access(HV_WORKDIR, HV_USER, '');

    # tempdir
    if (!file_exists(HV_HAVPTEMPDIR)) mwexec("mkdir -p " . HV_HAVPTEMPDIR);
    set_file_access(HV_HAVPTEMPDIR, HV_USER, '');

    # template permissions
    set_file_access(HV_TEMPLATEPATH, HV_USER, '');

    # log files exists ?
    if (!file_exists(HV_LOGDIR . HV_ACCESSLOG)) file_put_contents(HV_LOGDIR . HV_ACCESSLOG, '');
    if (!file_exists(HV_LOGDIR . HV_LOG)) file_put_contents(HV_LOGDIR . HV_LOG, '');
    # log dir permissions
    set_file_access(HV_LOGDIR, HV_USER, '0764');

    # pid file
    if (!file_exists(HV_PIDFILE)) file_put_contents(HV_PIDFILE, '');
    set_file_access(HV_PIDFILE, HV_USER, '0664');

    # freshclam config permissions
    if (!file_exists(HV_FRESHCLAM_CONFIGFILE)) file_put_contents(HV_FRESHCLAM_CONFIGFILE, '');
    set_file_access(HV_FRESHCLAM_CONFIGFILE, HV_AV_USER, '0664');

    # log files exists ?
    if (!file_exists(HV_AVLOGDIR . '/clamd.log')) file_put_contents(HV_AVLOGDIR . '/clamd.log', '');
    if (!file_exists(HV_AVLOGDIR . '/freshclam.log')) file_put_contents(HV_AVLOGDIR . '/freshclam.log', '');
    # log dir permissions
    set_file_access(HV_AVLOGDIR, HV_USER, '0777');

    # checking dir's and permissions
    # "DatabaseDirectory /var/db/clamav";
    # "UpdateLogFile /var/log/clamav/freshclam.log";

    # AV update script
    file_put_contents(HV_AVUPD_SCRIPT, havp_AVupdate_script());
    set_file_access(HV_AVUPD_SCRIPT, HV_AV_USER, '0755');
}

function havp_validate_settings($post, $input_errors) {
    $submit = isset($_GET['submit']) ? $_GET['submit'] : $_POST['submit'];

    # manual update AV database
    if ($submit === 'Update_AV')
        havp_update_AV();
    else {
        $prxport = trim($post[XML_PROXYPORT]);
        if (!empty($prxport) && !is_port($prxport))
            $input_errors[] = 'You must enter a valid port number in the \'Proxy port\' field';

        # check whitelist
        $lst = explode("\n", str_replace(" ", "\n", $post[XML_WHITELIST]));
        foreach ($lst as $dm) {
            $dm = trim($dm);
            if ($dm && check_bw_domain($dm) === false)
                $input_errors[] = "Invalid whitelist element '$dm'.";
        }

        # check blacklist
        $lst = explode("\n", str_replace(" ", "\n", $post[XML_BLACKLIST]));
        foreach ($lst as $dm) {
            $dm = trim($dm);
            if ($dm && check_bw_domain($dm) === false)
                $input_errors[] = "Invalid blacklist element '$dm'.";
        }
    }

}

function havp_config() {
    global $config;
    $pfconf = $config['installedpackages']['havp']['config'][0];
    $conf = array();

    $conf[] =
"# ============================================================
# HAVP config file
# This file generated automaticly with HAVP configurator (part of pfSense)
# (C)2008 Serg Dvoriancev
# email: dv_serg@mail.ru
# ============================================================
";

    $conf[] = "USER "  . HV_USER;
    $conf[] = "GROUP " . HV_USER;
    $conf[] = "DAEMON true";
    $conf[] = "PIDFILE " . HV_PIDFILE;
    $conf[] = "\n# For single user home use, 8 should be minimum.";
    $conf[] = "# For 500 users corporate use, start at 40.";
    $conf[] = "SERVERNUMBER 3";
    $conf[] = "MAXSERVERS 100";

    # log
    $conf[] = "\n# log ";
    $conf[] = "ACCESSLOG " . HV_LOGDIR . HV_ACCESSLOG;
    $conf[] = "ERRORLOG "  . HV_LOGDIR . HV_LOG;
    # syslog
    $syslog = ($pfconf[XML_SYSLOG] === 'on') ? 'true' : 'false';
    $conf[] = "\n# syslog";
    $conf[] = "USESYSLOG $syslog";  # use syslog?
    $conf[] = "SYSLOGNAME havp";
    $conf[] = "SYSLOGFACILITY daemon";
    $conf[] = "SYSLOGLEVEL info";
    #
    $conf[] = "\n# Level of HAVP logging\n#  0 = Only serious errors and information\n#  1 = Less interesting information is included";
    $conf[] = "LOG_OKS true";       # true - for debug, false - for work
    $conf[] = "LOGLEVEL 1";         # 0 - work level, 1 - debug level

    # temp
    $conf[] = "\n# temp ";
    $conf[] = "SCANTEMPFILE " . HV_TEMPDIR . HV_SCANTEMPFILE;
    $conf[] = "TEMPDIR " . HV_TEMPDIR;

    $conf[] = "\n#";
    $conf[] = "DBRELOAD 180";
    $conf[] = "TRANSPARENT false";
    $conf[] = "FORWARDED_IP false";

    # X-FORWARD
    $conf[] = "\n# X-FORWARD: proxy can include system's IP address or name in the HTTP requests it forwards";
    $v = ($pfconf[XML_XFORWARDEDFOR] === 'on') ? "true" : "false";
    $conf[] = "X_FORWARDED_FOR $v";

    # parent proxy = [proxy:port]
    if (!empty($pfconf[XML_PARENTPROXY])) {
        $prxy = str_replace(" ", ":", $pfconf[XML_PARENTPROXY]);
        $prxy = explode(":", $prxy);
        $conf[] = "\n# parent proxy ";
        $conf[] = "PARENTPROXY {$prxy[0]}";
        $conf[] = "PARENTPORT {$prxy[1]}";
    }

    # proxy listening on
    $conf[] = "\n# havp is listening on ";
    $pxyport = HV_DEFAULTPORT;
    $pxyaddr = HV_DEFAULTADDR;
    if (!empty($pfconf[XML_PROXYPORT])) $pxyport = $pfconf[XML_PROXYPORT];
    if ($pfconf[XML_USEEXTIFACE] === 'on' && !empty($pfconf[XML_PROXYIFACE])) {
        $pxyaddr = get_real_interface_address($pfconf[XML_PROXYIFACE]);
        $pxyaddr = $pxyaddr[0];
    }
    $conf[] = "PORT $pxyport";
    $conf[] = "BIND_ADDRESS $pxyaddr";

    # template files language
    $conf[] = "\n# Path to template files ";
    if (!empty($pfconf[XML_LANGUAGE]))
         $conf[] = "TEMPLATEPATH " . HV_TEMPLATEPATH . "/" . trim($pfconf[XML_LANGUAGE]);
    else $conf[] = "TEMPLATEPATH " . HV_TEMPLATEPATH . "/en";

    $conf[] = "\n# whitelist and blacklist";
    $conf[] = "WHITELISTFIRST true";
    $conf[] = "WHITELIST /usr/local/etc/havp" . HV_WHITELISTFILE;
    $conf[] = "BLACKLIST /usr/local/etc/havp" . HV_BLACKLISTFILE;

    # failscanerror - pass/block files if scanner error
    $conf[] = "\n# block file if error scanning";
    $v = ($pfconf[XML_FAILSCANERROR] === 'on') ? "true" : "false";
    $conf[] = "FAILSCANERROR $v";

    $conf[] = "\n# scanner ";
    $conf[] = "SCANNERTIMEOUT 10";
    $conf[] = "RANGE false";

    $conf[] = "\n# stream";
    $conf[] = "STREAMUSERAGENT Player Winamp iTunes QuickTime Audio RMA/ MAD/ Foobar2000 XMMS";
    $conf[] = "STREAMSCANSIZE 20000";

    # scan image
    $v = ($pfconf[XML_SCANIMG] === 'on') ? "true" : "false";
    $conf[] = "SCANIMAGES $v";

    $val = (!empty($pfconf[XML_SCANMAXSIZE]) && is_numeric($pfconf[XML_SCANMAXSIZE])) ? $pfconf[XML_SCANMAXSIZE] : 0;
    $conf[] = "MAXSCANSIZE $v";

    $conf[] = "# KEEPBACKBUFFER 200000";
    $conf[] = "# KEEPBACKTIME 5";

    $conf[] = "# After Trickling Time (seconds), some bytes are sent to browser to keep the connection alive";
    $conf[] = "TRICKLING 30";

    $conf[] = "# Downloads larger than MAXDOWNLOADSIZE will be blocked.";
    $val = (!empty($pfconf[XML_DOWNLOADMAXSIZE]) && is_numeric($pfconf[XML_DOWNLOADMAXSIZE])) ? $pfconf[XML_DOWNLOADMAXSIZE] : 0;
    $conf[] = "MAXDOWNLOADSIZE $val";

    $conf[] = "\n# ClamAV Library Scanner (libclamav) ";
    $conf[] = "ENABLECLAMLIB true";
    $conf[] = "# Should we block encrypted archives?";
    $conf[] = "# CLAMBLOCKENCRYPTED false";
    $conf[] = "# Should we block files that go over maximum archive limits?";
    $conf[] = "# CLAMBLOCKMAX false";
    $conf[] = "# Scanning limits inside archives (filesize = MB):";
    $conf[] = "# CLAMMAXFILES 1000";
    $conf[] = "# CLAMMAXFILESIZE 10";
    $conf[] = "# CLAMMAXRECURSION 8";
    $conf[] = "# CLAMMAXRATIO 250";

    $conf[] = "";
    return implode("\n", $conf);
}

function havp_config_freshclam()
{
    global $config;
    $pfconf = $config['installedpackages']['havp']['config'][0];
    $conf = array();

    $conf[] =
"# ============================================================
# freshclam(HAVP) config file
# This file generated automaticly with HAVP configurator (part of pfSense)
# (C)2008 Serg Dvoriancev
# email: dv_serg@mail.ru
# ============================================================
";
    $conf[] = "DatabaseDirectory /var/db/clamav";
# --
# disable log to file while error not solved:
# "ERROR: Problem with internal logger (UpdateLogFile = /var/log/clamav/freshclam.log)."
# --
#    $conf[] = "UpdateLogFile /var/log/clamav/freshclam.log";

    if ($pfconf[XML_SYSLOG] === 'on') {
        $conf[] = "\n# syslog";
        $conf[] = "LogSyslog yes";
        $conf[] = "LogFacility LOG_LOCAL6"; # LOG_LOCAL6 | LOG_MAIL
    }

    $conf[] = "\n# pid";
    $conf[] = "PidFile /var/run/clamav/freshclam.pid";

    $conf[] = "\n# db";
    $conf[] = "DatabaseOwner clamav";
    $conf[] = "AllowSupplementaryGroups yes";
    $conf[] = "DNSDatabaseInfo current.cvd.clamav.net";

    $avsrv = $pfconf['avupdateserver'];
    $avsrv = explode(" ", trim($avsrv));

    foreach ($avsrv as $asr)
    	if (!empty($asr))
             $conf[] = "DatabaseMirror $asr";

	# regional mirror
    if (!empty($pfconf['dbregion'])) {
    	$conf[] = '# regional db';
		switch($pfconf['dbregion']) {
			case 'au': $conf[] = "DatabaseMirror clamav.mirror.ayudahosting.com.au"; break; # australia
			case 'ca': $conf[] = "DatabaseMirror clamav.mirror.rafal.ca"; break;  # canada
			case 'cn': $conf[] = "DatabaseMirror 4most2.clamav.ialfa.net"; break; # china
			case 'eu': $conf[] = "DatabaseMirror clamav.edpnet.net"; break;       # europe
			case 'id': $conf[] = "DatabaseMirror db.clamav.or.id"; break;         # indonesia
			case 'jp': $conf[] = "DatabaseMirror clamavdb2.ml-club.jp"; break;    # japan
			case 'kr': $conf[] = "DatabaseMirror clamav.hostway.co.kr"; break;    # korea
			case 'ml': $conf[] = "DatabaseMirror clamav.doubleukay.com"; break;   # malaysia
			case 'ru': $conf[] = "DatabaseMirror clamav.citrin.ru"; break;        # russia
			case 'sa': $conf[] = "DatabaseMirror clamav.dial-up.net"; break;      # south africa
			case 'tw': $conf[] = "DatabaseMirror clamav.cs.pu.edu.tw"; break;     # taiwan
			case 'uk': $conf[] = "DatabaseMirror clamav.oucs.ox.ac.uk"; break;    # united kingdom
			case 'us': $conf[] = "DatabaseMirror clamav.catt.com "; break;        # united states
			default: break;
		}
	}

#    $conf[] = "DatabaseMirror db.ru.clamav.net";
#    $conf[] = "DatabaseMirror db.us.clamav.net";

    $conf[] = "\n# DO NOT TOUCH the following line ";
    $conf[] = "DatabaseMirror database.clamav.net";

    $conf[] = "\n# Number of database checks per day. Default: 12 (every two hours)";
    $chks = 0;
    switch($pfconf['havpavupdate']) {
        case 'none':   $chks = 0;  break;
        case 'hv_01h': $chks = 24; break;
        case 'hv_02h': $chks = 12; break;
        case 'hv_03h': $chks = 8;  break;
        case 'hv_04h': $chks = 6;  break;
        case 'hv_06h': $chks = 4;  break;
        case 'hv_08h': $chks = 3;  break;
        case 'hv_12h': $chks = 2;  break;
        case 'hv_24h': $chks = 1;  break;
    }
    $conf[] = "Checks $chks";

#    $conf[] = "# Proxy settings";  # future
#HTTPProxyServer myproxy.com
#HTTPProxyPort 1234
#HTTPProxyUsername myusername
#HTTPProxyPassword mypass

# MAKE GUI Errors display
# Run command when database update process fails.
# Default: disabled
#OnErrorExecute command

# Run command when freshclam reports outdated version.
# In the command string %v will be replaced by the new version number.
# Default: disabled
#OnOutdatedExecute command

# Enable debug messages in libclamav.
# Default: disabled
#Debug

    $conf[] = "";
    return implode("\n", $conf);
}

function havp_whitelist_def() {
    $whitelist  = array();

    $whitelist[] = "*sourceforge.net/*clamav-*";
    $whitelist[] = "*pfsense.com/*";
    $whitelist[] = "*.microsoft.com/*";
    $whitelist[] = "*.windowsupdate.com/*";         # M$ & M$ update
    # media and image extensions
    $whitelist[] = "*/*.gif\n*/*.swf\n*/*.png\n*/*.jpg\n*/*.jpeg\n*/*.mov\n*/*.avi\n*/*.flv\n*/*.bmp\n*/*.ico\n*/*.pdf\n*/*.mp3\n*/*.wma\n*/*.wmv\n*/*.ogg";

    return implode("\n", $whitelist);
}

# RAM disk
#  Mem -  RAM
# 128M -  16M
# 256M -  32M
# 512M -  64M
#   1G - 128M
#
function mountRAMdisk()
{
# disabled for VMware
# return;

    # detach and free all resources used by /dev/md10:
    mwexec("umount /var/tmp/havp");
    mwexec("mdconfig -d -u 10");

    # create and mount a 8MByte swap backed file system on /var/tmp/havp by /dev/md10:
    mwexec("mdconfig -a -t swap -s 1M -u 10");
    mwexec("newfs -U /dev/md10");
    mwexec("mount /dev/md10 /var/tmp/havp");
    mwexec("chmod 1777 /var/tmp/havp");
}

# ------------------------------------------------------------------------------
function set_file_access($dir, $owner, $mod) {
    mwexec("chgrp -R -v $owner $dir");
    mwexec("chown -R -v $owner $dir");
    if (!empty($mod)) {
         mwexec( "chmod -R -v $mod $dir");
    }
}

# Src from squid.inc
# Copyright (C) 2006 Scott Ullrich
# Copyright (C) 2006 Fernando Lemos
function get_real_interface_address($iface) {
        global $config;

        $iface = convert_friendly_interface_to_real_interface_name($iface);
        $line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
        list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);

        return array($ip, long2ip(hexdec($netmask)));
}
#-------------------------------------------------------------------------------

function havp_update_AV() {
    # AV update script
    if (file_exists(HV_AVUPD_SCRIPT)) {
        file_put_contents(HV_AVUPD_SCRIPT, havp_AVupdate_script());
        set_file_access(HV_AVUPD_SCRIPT, HV_AV_USER, '0755');
    }
    mwexec(HV_AVUPD_SCRIPT);
}

# *** check black/white list domain ***
# Lines can hold URLs with wildcards with following rules:
# Line must cointain Domain/Path
# Domains can have a wildcard at begin.
# Pages can hav a wildcard at begin and end.
# URLs without wildcards are exact
# Examples:
# (1) www.server-side.de    (Only this URL is whitelisted)
# (2) www.server-side.de/*  (Domain is completely whitelisted)
# (3) *server-side.de/index.html
# (4) */*.gif               (All .gif are whitelisted)
# (5) www.server-side.de/novirus*
# (6) www.server-side.de/*novirus*

function check_bw_domain($_dm) {
    $domain = "";
    $path = "";
    $pos = strpos($_dm, "/");

    if ($pos === false) {
        $domain = $_dm;
        $path = "";
    }
    else {
        $domain = substr($_dm, 0, $pos);
        $path   = substr($_dm, $pos+1);
    }

    $fmt  = "[a-zA-Z0-9_-]";

    # Domains can have a wildcard at begin '*xxx.xxx' - *my.domain.com
    if (!eregi("^(\*)|((\*){0,1}($fmt\.){0,}$fmt{1,})$", $domain)) return false;

    # Path can have a wildcard at begin and end '*xxx*'
    if ($path && !eregi("^(\*){0,1}(.[^\*][^=]){0,}(\*){0,1}$", $path)) return false;


    return true;
}

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function havp_AVupdate_script() {

# *** AV update script ***

$scr = <<<EOD
#!/bin/sh
# AV update script
# This file was automatically generated
# by the pfSense service handler.
/usr/local/bin/freshclam
wait
/usr/local/bin/sigtool --unpack-current daily.cvd
/usr/local/bin/sigtool --unpack-current main.cvd
wait
EOD;

# --- AV update script ---

    return $scr;
}
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# === UNDER CONSTRUCTION ===

# $day: 1, 2, .., 31, *, mon, tue, wed, thu, fri, sat, sun ; every day : [$day]
# $mon: 1, 2, .., 12, * ; every mon: [$mon]      [15]-[01]-[2001]
# $time = 'hh:mm': '12:00' - at 12:00; '*:*' - at any time; '/hh:/mm' - every hh every /mm
# havp_crontask('my_task', '15', '/1', '15:30', 'root' 'start_cmd', true);
function havp_crontask($task_name, $day, $mon, $time, $who, $cmd, $enabled)
{

}

# / === UNDER CONSTRUCTION ===

# $options: [0]='minute', [1]='hour', [2]='mday', [3]='month', [4]='wday', [5]='who', [6]='command'
#
function havp_setup_cron($task_name, $options, $on_off) {
        global $config;
	    $cron_item = array();

        # $on_off = TRUE/FALSE - install/deinstall cron task:
        # prepare new cron item
        if (is_array($options)) {
            $cron_item['task_name'] = $task_name;
            $cron_item['minute']  =  $options[0];
            $cron_item['hour']    =  $options[1];
            $cron_item['mday']    =  $options[2];
            $cron_item['month']   =  $options[3];
            $cron_item['wday']    =  $options[4];
            $cron_item['who']     = ($options[5]) ? $options[5] : 'nobody';
            $cron_item['command'] =  $options[6];
        }

    	# unset old cron task with $task_name
        if ($task_name !== "") {
            $flag_cron_upd = false;

            # delete old cron task if exists
            foreach($config['cron']['item'] as $key => $val) {
        	    if ($config['cron']['item'][$key]['task_name'] === $task_name) {
                    unset($config['cron']['item'][$key]);
                    $flag_cron_upd = true;

                    # log ! cron task deleted !
                    break;
                }
            }

            # set new cron task
            if (($on_off === true) and !empty($cron_item)) {
                $config['cron']['item'][] = $cron_item;
                $flag_cron_upd = true;

                # log ! cron task installed !
            }

            # write config and configure cron only if cron task modified
            if ($flag_cron_upd === true) {
                write_config("Installed cron task '$task_name' for 'havp' package");
                configure_cron();
                # log ! cron stored !
            }
        }
        else {
            # ! error $name !

            # if error - break function
        	return;
        }
}
?>
