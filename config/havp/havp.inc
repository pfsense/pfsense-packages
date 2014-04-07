<?php
/*
        havp.inc
        Part of pfSense package
        Copyright (C) 2009 Serg Dvorianceev
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

/* ! for HAVP v.0.88 ! */
/* ! Real virus collection for tests http://www.nvkz.kuzbass.net/as/ ! */

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('system.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('service-utils.inc');

if(!function_exists("filter_configure"))
	require_once("filter.inc");

# ------------------------------------------------------------------------------
# globals
# ------------------------------------------------------------------------------
# Debug / uncomment next for debug /
define('HV_DEBUG',             'false');

# use Clamd daemon (another - use libclam)
define('HV_USE_CLAMD',         'true');
define('HV_CLAMD_TCPSOCKET',   'true');

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# RAM Disk - use as 'tmp' dir for more quick work
#    note: this options allow RAM Disk allocation
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# set 'true' for enable RAM Disk
define('HV_USE_TMPRAMDISK', 'true');
# set 'false' for disable RAM Disk on VM (if you have troubles on VM)
define('HV_VM_TMPRAMDISK',  'true');

# ------------------------------------------------------------------------------
# forms
# ------------------------------------------------------------------------------
define('HVFORM_HAVP',         'havp');
define('HVFORM_FSCAN',        'havpfscan');
define('HVFORM_AVSET',        'havpavset');

# ------------------------------------------------------------------------------
# defines
# ------------------------------------------------------------------------------
# havp
define('HVDEF_ADDR',                    '127.0.0.1');
define('HVDEF_PROXYPORT',	            '8080');
define('HVDEF_MAXSCANSIZE',             '5000000'); # [bytes] ! do not enter 0 or big size !
define('HVDEF_MAXARCSCANSIZE',          '5000000'); # [bytes] ! do not enter 0 or big size !
define('HVDEF_PID_FILE',                '/var/run/havp.pid');
define('HVDEF_WORK_DIR',                '/usr/local/etc/havp');

$pfSversion = str_replace("\s", "", file_get_contents("/etc/version"));
if(preg_match("/^2./",$pfSversion))
	define('HVDEF_LOG_DIR',			'/var/log/havp');
else
	define('HVDEF_LOG_DIR',			'/var/log');

define('HVDEF_TEMP_DIR',                '/var/tmp');
define('HVDEF_HAVPTEMP_DIR',            HVDEF_TEMP_DIR.'/havp');
define('HVDEF_RAMTEMP_DIR',             HVDEF_TEMP_DIR.'/havpRAM');
define('HVDEF_SCANTEMPFILE',            '/havp-XXXXXX');
define('HVDEF_TEMPLATES',               '/usr/local/share/examples/havp/templates');
define('HVDEF_TEMPLATES_EX',            HVDEF_TEMPLATES . '_ex');
define('HVDEF_FILTER_RULES',            '/tmp/rules.havp');
define('HVDEF_HAVP_CONFIG',             HVDEF_WORK_DIR.'/havp.config');
define('HVDEF_HAVP_XMLCONF',            HVDEF_WORK_DIR.'/havp_conf.xml');
define('HVDEF_HAVP_WHITELIST',          HVDEF_WORK_DIR.'/whitelist');
define('HVDEF_HAVP_BLACKLIST',          HVDEF_WORK_DIR.'/blacklist');
define('HVDEF_HAVP_ACCESSLOG',          HVDEF_LOG_DIR .'/access.log');
define('HVDEF_HAVP_ERRORLOG',           HVDEF_LOG_DIR .'/havp.log');
define('HVDEF_HAVP_MINSRV',             '3');
define('HVDEF_HAVP_MAXSRV',             '100');

# Clam
#define('HVDEF_CLAM_RUNDIR',             '/var/run/clamav');
define('HVDEF_CLAM_RUNDIR',             '/var/run');
define('HVDEF_CLAM_DBDIR',              '/var/db/clamav');
if(preg_match("/^2./",$pfSversion))
	define('HVDEF_AVLOG_DIR',               '/var/log/clamav');
else
	define('HVDEF_AVLOG_DIR',               '/var/log');

define('HVDEF_CLAM_SOCKET',             HVDEF_CLAM_RUNDIR.'/clamd.sock');
define('HVDEF_CLAM_PID',                HVDEF_CLAM_RUNDIR.'/clamd.pid');
define('HVDEF_CLAM_LOG',                HVDEF_AVLOG_DIR . '/clamd.log');
define('HVDEF_CLAM_WORKDIR',            '/usr/local/etc');
define('HVDEF_CLAM_CONFIG',             '/usr/local/etc/clamd.conf');
define('HVDEF_CLAM_TCPSOCKET',          '3310');
define('HVDEF_FRESHCLAM_CONF',          '/usr/local/etc/freshclam.conf');
define('HVDEF_FRESHCLAM_LOG',           HVDEF_AVLOG_DIR . '/freshclam.log');
define('HVDEF_CLAMSCAN_LOG',            '/var/log/clamscan.log');
define('HVDEF_STATUS_FILE',             '/var/tmp/havp.status');

# script's
define('HVDEF_SCRIPT_DIR',              '/usr/local/etc/rc.d');
define('HVDEF_AVCRON_SCRIPT',           '/clamav-freshclam');
define('HVDEF_FILTER_RESYNC_SCRIPT',    '/usr/local/pkg/pf/havp_filter_resync.sh');
define('HVDEF_HAVP_STARTUP_SCRIPT',     HVDEF_SCRIPT_DIR . '/havp.sh');
define('HVDEF_CLAM_STARTUP_SCRIPT',     HVDEF_SCRIPT_DIR . '/clamd');
define('HVDEF_AVUPD_SCRIPT',            HVDEF_SCRIPT_DIR . '/havp_avupdate');

# status
define('HVDEF_HAVP_STATUS_FILE',        '/tmp/havp.status');
define('HVDEF_CLAM_STATUS_FILE',        '/tmp/clam.status');
define('HVDEF_UPD_STATUS_FILE',         '/tmp/havp.update.status');
define('HVDEF_FRESHCLAM_STATUS_FILE',   '/tmp/havp.freshclam.status');

# cron
define('HVDEF_CLAM_UPD_CRONNAME',       'havp_clam_update');
define('HVDEF_CLAM_UPD_CRONCMD',        HVDEF_SCRIPT_DIR . HVDEF_AVCRON_SCRIPT . " start");
define('HVDEF_CLAM_UPD_CRONKEY',        HVDEF_AVCRON_SCRIPT);

# user
define('HVDEF_USER',                    'havp');
define('HVDEF_GROUP',                   'havp');
define('HVDEF_AVUSER',                  HVDEF_USER);

# fields
define('HV_SCANTEMPFILE',	 'hv_scan_tempfile');

# ------------------------------------------------------------------------------
# XML fields
# ------------------------------------------------------------------------------
define('F_ENABLE',              'enable');
define('F_PROXYMODE',           'proxymode');
define('F_PROXYINTERFACE',      'proxyinterface');
define('F_PROXYBINDIFACE',      'proxybindiface');  # internal var
define('F_PROXYPORT',           'proxyport');
define('F_PARENTPROXY',         'parentproxy');
define('F_LANGUAGE',            'lang');
define('F_MAXDOWNLOADSIZE',     'maxdownloadsize');
define('F_RANGE',               'range');
define('F_WHITELIST',           'whitelist');
define('F_BLACKLIST',           'blacklist');
define('F_ENABLEFORWARDEDIP',   'enableforwardedip');
define('F_ENABLEXFORWARDEDFOR', 'enablexforwardedfor');
define('F_ENABLERAMDISK',       'enableramdisk');
# scanner
define('F_FAILSCANERROR',       'failscanerror');
define('F_SCANMAXSIZE',         'scanmaxsize');
define('F_SCANIMG',             'scanimg');
define('F_SCANARC',             'scanarc');
define('F_SCANSTREAM',          'scanstream');
define('F_SCANARCMAXSIZE',      'scanarcmaxsize');
define('F_SCANBROKENEXE',       'scanbrokenexe');
# antivirus options
define('F_HAVPUPDATE',        'havpavupdate');
define('F_DBREGION',          'dbregion');
define('F_AVUPDATESERVER',    'avupdateserver');
# log
define('F_SYSLOG',            'syslog');
define('F_LOG',               'log');
define('F_AVSETSYSLOG',       'avsetsyslog');
define('F_AVSETLOG',          'avsetlog');
#
define('F_TEMPLATEPATH',      'templatepath');    # internal var
# file scanner [HVFORM_FSCAN]
define('F_SCANFILEPATH',      'scanfilepath');
# ���� ��������� ��� ��� � ��� � ��� ������
define('F_DISABLEXFORWARD',   'disablexforward'); # + forwarded ip
define('F_FORWARDEDIP',       'forwardedip');

# ------------------------------------------------------------------------------
# global config
# ------------------------------------------------------------------------------
$havp_config = array();
$havp_config[HV_SCANTEMPFILE] = HVDEF_HAVPTEMP_DIR . HVDEF_SCANTEMPFILE;

# ------------------------------------------------------------------------------
# Initialization
# ------------------------------------------------------------------------------
havp_convert_pfxml_xml();

# ==============================================================================
# Installation and config
# ==============================================================================
function havp_install()
{
    update_status("HAVP check system..\n");
    havp_check_system();
    havp_fix();

    havp_avset_resync();
    havp_update_AV();


    update_status("Start update Antivirus bases. Wait 5-20 min before use ..");
}
# ------------------------------------------------------------------------------
function havp_deinstall()
{
    havp_setup_cron(HVDEF_AVUPD_SCRIPT,"", "");
    mwexec("killall -9 havp");
    mwexec("rm -rf " . HVDEF_HAVP_STARTUP_SCRIPT);
    mwexec("rm -rf " . HVDEF_FILTER_RESYNC_SCRIPT);
    mwexec("rm -rf " . HVDEF_PID_FILE);
#    mwexec("rm -rf " . HVDEF_CLAM_STARTUP_SCRIPT);
#    mwexec("rm -rf " . HVDEF_AVUPD_SCRIPT);
#    mwexec("rm -rf " . HVDEF_CLAM_PID);
#    mwexec("rm -rf " . HVDEF_CLAM_SOCKET);
    umountRAMDisk();
}
# ==============================================================================
# Events
# ==============================================================================
# before form
# ------------------------------------------------------------------------------
function havp_before_form($pkg)
{
}
# ------------------------------------------------------------------------------
function havp_fscan_before_form($pkg)
{
    if(is_array($pkg['fields']['field'])) {
        foreach($pkg['fields']['field'] as $key => $field) {
        	if ($field['fieldname'] === F_SCANFILEPATH) {
        		$pkg['fields']['field'][$key]['description'] .= havp_fscan_html();
                break;
        	}
        }
    }
}
# ------------------------------------------------------------------------------
# validation
# ------------------------------------------------------------------------------
function havp_validate_settings($post, $input_errors)
{
    $submit = isset($_GET['submit']) ? $_GET['submit'] : $_POST['submit'];

    # manual update AV database
    if ($submit === 'Update_AV') {
        havp_update_AV();
    }
    # Scan file or dir
    elseif($submit === 'Start_scan') {
        if (file_exists($post[F_SCANFILEPATH]))
            start_antivirus_scanner($post[F_SCANFILEPATH]);
        else $input_errors[] = "File or path not exists '{$post[F_SCANFILEPATH]}'.";
    }
    else {
        # ifaces
        if (!isset($post[F_PROXYINTERFACE]) || empty($post[F_PROXYINTERFACE])) {
            $post[F_PROXYINTERFACE] = "lan";
        }

        # port validate
        $prxport = trim($post[F_PROXYPORT]);
        if (!empty($prxport) && !is_port($prxport))
            $input_errors[] = 'You must enter a valid port number in the \'Proxy port\' field';

        # parent proxy validate
        $parent = trim($post[F_PARENT]);

        # max download size validate
        $maxval = trim($post[F_MAXDOWNLOADSIZE]);
        if (!empty($maxval) && !is_numericint($maxval)) # is_port - validate value
            $input_errors[] = 'You must enter a valid numeric value in \'Max download size\' field.';

        # scan max file size validate
        $maxval = trim($post[F_SCANMAXSIZE]);
        if (!empty($maxval) && !is_numericint($maxval)) # is_port - validate value
            $input_errors[] = 'You must enter a valid numeric value in \'Scan max file size\' field.';

        # whitelist validate
        $lst = str_replace(array(" ", ";"), "\n", $post[F_WHITELIST]);
        $lst = explode("\n", $lst);
        foreach ($lst as $dm) {
            $dm = trim($dm);
            if ($dm && check_bw_domain($dm) === false)
                $input_errors[] = "Invalid whitelist element '$dm'. Example: '*domain.com, domain.com/*path*'.";
        }

        # blacklist validate
        $lst = str_replace(array(" ", ";"), "\n", $post[F_BLACKLIST]);
        $lst = explode("\n", $lst);
        foreach ($lst as $dm) {
            $dm = trim($dm);
            if ($dm && check_bw_domain($dm) === false)
                $input_errors[] = "Invalid blacklist element '$dm'. Example: '*domain.com, domain.com/*path*'.";
        }
    }

}
# ------------------------------------------------------------------------------
# resync
# ------------------------------------------------------------------------------
function havp_resync()
{
    global $havp_config;

    havp_convert_pfxml_xml();
    havp_check_system();

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # whitelist and blacklist
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # also white-listed by default:
    $whitelist = havp_whitelist_def() . "\n" . str_replace(";", "\n", $havp_config[F_WHITELIST]);
    $blacklist = str_replace(";", "\n", $havp_config[F_BLACKLIST]);
    # fix: stupid havp parser - error on 0x0D:
    $whitelist = str_replace("\r", "", $whitelist);
    $blacklist = str_replace("\r", "", $blacklist);
    file_put_contents(HVDEF_HAVP_WHITELIST, $whitelist);
    file_put_contents(HVDEF_HAVP_BLACKLIST, $blacklist);

    # reconfigure clamd
    havp_reconfigure_clamd();

    # config havp
    file_put_contents   (HVDEF_HAVP_CONFIG, havp_config_havp());
    havp_set_file_access(HVDEF_WORK_DIR, HVDEF_USER, '0755');

    if ($havp_config[F_ENABLE] === 'true') {
        mwexec_bg(HVDEF_HAVP_STARTUP_SCRIPT . " restart");
        log_error("Starting HAVP");
    }
    else {
        mwexec_bg(HVDEF_HAVP_STARTUP_SCRIPT . " stop");
        log_error("Stopping HAVP");
    }

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # reconfigure squid
    havp_configure_squid();

    # reconfigure AV parts
    havp_reconfigure_freshclam();
    havp_reconfigure_cron();

    # configure system filter for 2.xx
    if (pfsense_version_() !== '1') filter_configure();

}
# ------------------------------------------------------------------------------
function havp_avset_resync()
{
    havp_convert_pfxml_xml();
    havp_check_system();
    # reconfigure
    havp_reconfigure_clamd();
    havp_reconfigure_freshclam();
    havp_reconfigure_cron();
}
# ==============================================================================
# check system
# ==============================================================================
function havp_check_system()
{
    global $havp_config;

    # check user group
    $grp = exec('pw group show ' . HVDEF_GROUP);
    if (strpos($grp, HVDEF_GROUP) !== 0) {
        exec('pw group add ' . HVDEF_GROUP);
    	 log_error("Antivirus: Group '" . HVDEF_GROUP . "' was added.");
    }
    $usr = exec('pw usershow -n ' . HVDEF_USER);
    if (strpos($usr, HVDEF_USER) !== 0) {
        exec('pw useradd ' . HVDEF_USER . ' -g ' . HVDEF_GROUP . ' -h - -s "/sbin/nologin" -d "/nonexistent" -c "havp daemon"');
    	 log_error("Antivirus: User '" . HVDEF_USER . "' was added.");
    }

    # workdir permissions
    havp_set_file_access(HVDEF_WORK_DIR, HVDEF_USER, '');

    # havp tempdir
    if (!file_exists(HVDEF_HAVPTEMP_DIR))
    	mwexec("mkdir -p " . HVDEF_HAVPTEMP_DIR);
    havp_set_file_access(HVDEF_HAVPTEMP_DIR, HVDEF_USER, '');

    # clamav dbdir
    if (!file_exists(HVDEF_CLAM_DBDIR))
	mwexec("mkdir -p " . HVDEF_CLAM_DBDIR);
    havp_set_file_access(HVDEF_CLAM_DBDIR, HVDEF_AVUSER, '');

    # RAM tempdir
    if (!file_exists(HVDEF_RAMTEMP_DIR))
    	mwexec("mkdir -p " . HVDEF_RAMTEMP_DIR);
    havp_set_file_access(HVDEF_RAMTEMP_DIR, HVDEF_USER, '');

    # template permissions
    if (!file_exists(HVDEF_TEMPLATES_EX))
        mwexec("mkdir -p " . HVDEF_TEMPLATES_EX);
    havp_set_file_access(HVDEF_TEMPLATES,    HVDEF_USER, '');
    havp_set_file_access(HVDEF_TEMPLATES_EX, HVDEF_USER, '');

    # havp log dir
    if (!file_exists(HVDEF_LOG_DIR))
    	mwexec("mkdir -p " . HVDEF_LOG_DIR);
    havp_set_file_access(HVDEF_LOG_DIR, HVDEF_USER, '');
    # log files exists ?
    if (!file_exists(HVDEF_HAVP_ACCESSLOG)) file_put_contents(HVDEF_HAVP_ACCESSLOG, '');
    if (!file_exists(HVDEF_HAVP_ERRORLOG))  file_put_contents(HVDEF_HAVP_ERRORLOG,  '');
    # log dir permissions
    havp_set_file_access(HVDEF_LOG_DIR, HVDEF_USER, '0764');

    # pid file
    if (!file_exists(HVDEF_PID_FILE)) file_put_contents(HVDEF_PID_FILE, '');
    havp_set_file_access(HVDEF_PID_FILE, HVDEF_USER, '0664');

    # freshclam config permissions
    if (!file_exists(HVDEF_FRESHCLAM_CONF)) file_put_contents(HVDEF_FRESHCLAM_CONF, '');
    havp_set_file_access(HVDEF_FRESHCLAM_CONF, HVDEF_AVUSER, '0664');

    # clam log dir
    if (!file_exists(HVDEF_AVLOG_DIR))
    	mwexec("mkdir -p " . HVDEF_AVLOG_DIR);
    havp_set_file_access(HVDEF_AVLOG_DIR, HVDEF_USER, '');
    # log files exists ?
    if (!file_exists(HVDEF_CLAM_LOG))      file_put_contents(HVDEF_CLAM_LOG, '');
    if (!file_exists(HVDEF_FRESHCLAM_LOG)) file_put_contents(HVDEF_FRESHCLAM_LOG, '');
    # log dir permissions
    # if (!file_exists(HVDEF_AVLOG_DIR))
    #    mwexec("mkdir -p " . HVDEF_AVLOG_DIR);
    havp_set_file_access(HVDEF_AVLOG_DIR, HVDEF_USER, '0777');

    # =-= ClamAV =-=
    # catalog for Pid and Socket files
#    if (!file_exists(HVDEF_CLAM_RUNDIR))
#        mwexec("mkdir -p " . HVDEF_CLAM_RUNDIR);
#    havp_set_file_access(HVDEF_CLAM_RUNDIR, HVDEF_USER, '0774');

    # AV update script
    file_put_contents(HVDEF_AVUPD_SCRIPT, havp_AVupdate_script());
    havp_set_file_access(HVDEF_AVUPD_SCRIPT, HVDEF_AVUSER, '0755');

    # AV update notification script
#    file_put_contents(HVDEF_ON_AVUPD_SCRIPT,    havp_on_avupd_script());
#    havp_set_file_access(HVDEF_ON_AVUPD_SCRIPT, HVDEF_AVUSER, '0755');

    # startup script's (havp and clamd)
    havp_startup_script();
    hv_clamd_startup_script();

    # havp filter script
    if (pfsense_version_() == '1') {
        # script exists only for 1.2.x
        file_put_contents(HVDEF_FILTER_RESYNC_SCRIPT, havp_filter_resync_script());
        havp_set_file_access(HVDEF_FILTER_RESYNC_SCRIPT, HVDEF_AVUSER, '0755');
    } else {
        # delete script if exists
        if (file_exists(HVDEF_FILTER_RESYNC_SCRIPT))
            mwexec("rm -f " . HVDEF_FILTER_RESYNC_SCRIPT);
    }

    # mount RAMDisk
    mountRAMdisk(true);
}
# ==============================================================================
# Reconfigure package parts
# ==============================================================================
function havp_reconfigure_clamd()
{
    file_put_contents   (HVDEF_CLAM_CONFIG, havp_config_clam());
    havp_set_file_access(HVDEF_CLAM_CONFIG, HVDEF_USER, '0664');
}
# ------------------------------------------------------------------------------
function havp_reconfigure_freshclam()
{
    # config freshclam
    file_put_contents   (HVDEF_FRESHCLAM_CONF, havp_config_freshclam());
    havp_set_file_access(HVDEF_FRESHCLAM_CONF, HVDEF_USER, '0664');
}
# ------------------------------------------------------------------------------
function havp_reconfigure_cron()
{
    global $havp_config;

    # cron task
    $on     = false;
    $optval = array("", "*/1", "*/2", "*/3", "*/4", "*/6", "*/8", "*/12", "0");
    $opt    = array("0", "*", "*", "*", "*", "root", "/usr/bin/nice -n20 " . HVDEF_AVUPD_SCRIPT);
    $opt[1] = $optval[$havp_config[F_HAVPUPDATE]];
    $on     = ($opt[1] !== "");

    havp_setup_cron(HVDEF_AVUPD_SCRIPT, $opt, $on);
}
# ------------------------------------------------------------------------------
# Convert conf to XML
# ------------------------------------------------------------------------------
function havp_convert_pfxml_xml()
{
    global $config, $havp_config;

    $pfconf = $config['installedpackages'][HVFORM_HAVP]['config'][0];

    # === GUI Fields ===
    $havp_config[F_ENABLE]          = ( $pfconf[F_ENABLE] === 'on' ? 'true' : 'false' );
    # proxy
    $havp_config[F_PROXYMODE]       = ( !empty($pfconf[F_PROXYMODE]) ? $pfconf[F_PROXYMODE] : 'standard' );
# ToDo: add check squid transparent
    $havp_config[F_PROXYINTERFACE]  = $pfconf[F_PROXYINTERFACE];
    $havp_config[F_PROXYPORT]       = ( !empty($pfconf[F_PROXYPORT]) ? $pfconf[F_PROXYPORT] : HVDEF_PROXYPORT );
# ToDo: add check squid proxy port
    # parent proxy
    # [F_PARENTPROXY] = "proxy_ip:port"
    $pfconf[F_PARENTPROXY]          = trim($pfconf[F_PARENTPROXY]);
    if (!empty($pfconf[F_PARENTPROXY])) {
    	$parent = explode(":", trim($pfconf[F_PARENTPROXY]));
    	$havp_config[F_PARENTPROXY] = array( 'ip' => $parent[0], 'port' => $parent[1] );
	}
	else $havp_config[F_PARENTPROXY] = '';
    # language
    $havp_config[F_LANGUAGE]            = trim($pfconf[F_LANGUAGE]);
    # proxy settings
    $havp_config[F_ENABLEFORWARDEDIP]   = ( $pfconf[F_ENABLEFORWARDEDIP]   === 'on' ? 'true' : 'false' );
    $havp_config[F_ENABLEXFORWARDEDFOR] = ( $pfconf[F_ENABLEXFORWARDEDFOR] === 'on' ? 'true' : 'false' );
    $havp_config[F_MAXDOWNLOADSIZE]     = ( is_numeric($pfconf[F_MAXDOWNLOADSIZE]) ? $pfconf[F_MAXDOWNLOADSIZE] : 0 );
    $havp_config[F_RANGE]               = ( $pfconf[F_RANGE]               === 'on' ? 'true' : 'false' );
    $havp_config[F_ENABLERAMDISK]       = ( $pfconf[F_ENABLERAMDISK]       === 'on' ? 'true' : 'false' );
    # whitelist
    $havp_config[F_WHITELIST]           = base64_decode($pfconf[F_WHITELIST]);
    $havp_config[F_WHITELIST]           = str_replace(";", "\n", $havp_config[F_WHITELIST]);
    $havp_config[F_WHITELIST]           = str_replace(";", " ",  $havp_config[F_WHITELIST]);
    # blacklist
    $havp_config[F_BLACKLIST]           = base64_decode($pfconf[F_BLACKLIST]);
    $havp_config[F_BLACKLIST]           = str_replace(";", "\n", $havp_config[F_BLACKLIST]);
    $havp_config[F_BLACKLIST]           = str_replace(";", " ",  $havp_config[F_BLACKLIST]);

    # =-= Temp RAMDisk =-=
    # use RAMDisk if only capacity > calculated [MAXSCANSIZE * 50 connections]
    # =-=
    # before config manage Temp Dir = RAMDisk|Hard Disk
    $havp_config[HV_SCANTEMPFILE]  = HVDEF_HAVPTEMP_DIR . HVDEF_SCANTEMPFILE;
    if ($havp_config[F_ENABLERAMDISK] === 'true') {
        $sys_capacity = get_memory();
        $mem_capacity = intval($sys_capacity[0]) / 4;                     # [Mb]
        $calculated   = 50 * $havp_config[F_SCANMAXSIZE] / (1024 * 1024); # [Mb]

        # this is restriction need for balancing between pfSense and HAVP work speed
        # we can not allocate memory at the expense of other services of the pfSense
        if ($mem_capacity > $calculated) {
    	    # re-define temp file to RAM Disk
            $havp_config[HV_SCANTEMPFILE]  = HVDEF_RAMTEMP_DIR . HVDEF_SCANTEMPFILE;
        }
        else
            log_error("havp: RAMDisk not used. Diagnostic: system {$sys_capacity[0]}Mb, avialable {$mem_capacity}Mb, calculated {$calculated}Mb. Try reducing 'MAXSCANSIZE' value.");
    }
    # scanner
    $havp_config[F_FAILSCANERROR]   = ( $pfconf[F_FAILSCANERROR] === 'on' ? 'true' : 'false' );
    $havp_config[F_SCANMAXSIZE]     = ( is_numeric($pfconf[F_SCANMAXSIZE])  ? $pfconf[F_SCANMAXSIZE] : HVDEF_MAXSCANSIZE ) * 1024; # KB -> Byte
    $havp_config[F_SCANIMG]         = ( $pfconf[F_SCANIMG]    === 'on' ? 'true' : 'false' );
    $havp_config[F_SCANARC]         = ( $pfconf[F_SCANARC]    === 'on' ? 'true' : 'false' );
    $havp_config[F_SCANSTREAM]      = ( $pfconf[F_SCANSTREAM] === 'on' ? 'true' : 'false' );
	$havp_config[F_SCANBROKENEXE]   = ( $pfconf[F_SCANBROKENEXE] === 'on' ? 'true' : 'false' );
    $havp_config[F_SCANARCMAXSIZE]  = ( is_numeric($pfconf[F_SCANARCMAXSIZE]) ? $pfconf[F_SCANARCMAXSIZE] : HVDEF_MAXARCSCANSIZE );
    # log
    $havp_config[F_SYSLOG]          = ( $pfconf[F_SYSLOG]      === 'on' ? 'true' : 'false' );
    $havp_config[F_LOG]             = ( $pfconf[F_LOG]         === 'on' ? 'true' : 'false' );
    #
    # =-= Internal variables =-=
    # proxy
    $havp_config[F_PROXYBINDIFACE] = 'localhost';
    # language template files path
    $lng = $havp_config[F_LANGUAGE] ? $havp_config[F_LANGUAGE] : "en";
    $havp_config[F_TEMPLATEPATH]    = ( file_exists(HVDEF_TEMPLATES_EX  . "/$lng") ? HVDEF_TEMPLATES_EX : HVDEF_TEMPLATES );
    $havp_config[F_TEMPLATEPATH]   .= "/$lng";
    #
    # =-= HVFORM_AVSET =-=
    # av settings
    $pf_avset_conf = $config['installedpackages'][HVFORM_AVSET]['config'][0];
    $havp_config[F_HAVPUPDATE]      = $pf_avset_conf[F_HAVPUPDATE];
    $havp_config[F_DBREGION]        = $pf_avset_conf[F_DBREGION];
    $havp_config[F_AVUPDATESERVER]  = $pf_avset_conf[F_AVUPDATESERVER];
    # avlog
    $havp_config[F_AVSETSYSLOG]     = $pf_avset_conf[F_AVSETSYSLOG] === 'on' ? 'true' : 'false';
    $havp_config[F_AVSETLOG]        = $pf_avset_conf[F_AVSETLOG]    === 'on' ? 'true' : 'false';

    #
    # store havp config cache
    $cfg_xml = dump_xml_config($havp_config, 'havp');
    file_put_contents(HVDEF_HAVP_XMLCONF, $cfg_xml);

    return $havp_config;
}
# ------------------------------------------------------------------------------
# config
# ------------------------------------------------------------------------------
# HAVP config
function havp_config_havp()
{
    global $havp_config;

    $conf = array();
    $conf[] =
"# ============================================================
# HAVP config file
# This file generated automaticly with HAVP configurator (part of pfSense)
# (C)2008 Serg Dvoriancev
# email: dv_serg@mail.ru
# ============================================================
";
    $conf[] = "USER           " . HVDEF_USER;
    $conf[] = "GROUP          " . HVDEF_GROUP;
    $conf[] = "DAEMON         true";
    $conf[] = "PIDFILE        " . HVDEF_PID_FILE;
    $conf[] = "\n# For small home use, 8 should be minimum.";
    $conf[] = "# For 500 users corporate use, start at 40.";
    $conf[] = "SERVERNUMBER   " . HVDEF_HAVP_MINSRV;
    $conf[] = "MAXSERVERS     " . HVDEF_HAVP_MAXSRV;
    # log
    $conf[] = "\n# log ";
    $conf[] = "ACCESSLOG      " . HVDEF_HAVP_ACCESSLOG;
    $conf[] = "ERRORLOG       " . HVDEF_HAVP_ERRORLOG;
    # syslog
    $conf[] = "\n# syslog";
    $conf[] = "USESYSLOG      {$havp_config[F_SYSLOG]}";
    $conf[] = "SYSLOGNAME     havp";
    $conf[] = "SYSLOGFACILITY daemon";
    $conf[] = "SYSLOGLEVEL    " . (HV_DEBUG === 'true' ? "debug" : "info");     # err | warning | info | debug
    #
    $conf[] = "\n# Level of HAVP logging\n#  0 = Only serious errors and information\n#  1 = Less interesting information is included";
    $conf[] = "LOG_OKS        false";                                       # false - access_log requests viruses only, true - access_log all requests
    $conf[] = "LOGLEVEL       " . ( HV_DEBUG === 'true' ? "1"    : "0" );   # 0 - work level, 1 - debug level
    # temp
    $conf[] = "\n# temp ";
    $conf[] = "SCANTEMPFILE   " . $havp_config[HV_SCANTEMPFILE];
    $conf[] = "TEMPDIR        " . HVDEF_TEMP_DIR;
    #
    $conf[] = "\n#";
    $conf[] = "DBRELOAD       180";
    $conf[] = "TRANSPARENT    " . ( $havp_config[F_PROXYMODE] === 'transparent' ? "true" : "false" );
    # X-FORWARD, X-FORWARDED-FOR options
    $conf[] = "\n# if HAVP is used as parent proxy by some other proxy, this allows to write the real users IP to log, instead of proxy IP.";
    $conf[] = "FORWARDED_IP    " . $havp_config[F_ENABLEFORWARDEDIP];
    $conf[] = "X_FORWARDED_FOR " . $havp_config[F_ENABLEXFORWARDEDFOR];
    # parent proxy = [proxy:port]
    if (!empty($havp_config[F_PARENTPROXY])) {
        $conf[] = "\n# parent proxy ";
        $conf[] = "PARENTPROXY     {$havp_config[F_PARENTPROXY]['ip']}";
        $conf[] = "PARENTPORT      {$havp_config[F_PARENTPROXY]['port']}";
    }
    # proxy listening on
    $conf[] = "\n# havp is listening on ";
    $conf[] = "PORT           {$havp_config[F_PROXYPORT]}";
    # bind to ip address
    $bind_iface = get_real_interface_address($havp_config[F_PROXYBINDIFACE]);
	$conf[] = "BIND_ADDRESS   {$bind_iface[0]}";
    # template files language
    $conf[] = "\n# Path to template files ";
    $conf[] = "TEMPLATEPATH   {$havp_config[F_TEMPLATEPATH]}";
    #
    $conf[] = "\n# whitelist and blacklist";
    $conf[] = "WHITELISTFIRST true";
    $conf[] = "WHITELIST      " . HVDEF_HAVP_WHITELIST;
    $conf[] = "BLACKLIST      " . HVDEF_HAVP_BLACKLIST;
    # failscanerror - pass/block files if scanner error
    $conf[] = "\n# block file if error scanning";
    $conf[] = "FAILSCANERROR  {$havp_config[F_FAILSCANERROR]}";
    #
    $conf[] = "\n# scanner ";
    $conf[] = "SCANNERTIMEOUT 10";
    #
    if ($havp_config[F_SCANSTREAM] === 'true') {
        #
        $conf[] = "\n# always allow range, if stream scan enabled";
        $conf[] = "RANGE           true";
        $conf[] = "\n# stream scan enabled";
        $conf[] = "STREAMUSERAGENT Player Winamp iTunes QuickTime Audio RMA/ MAD/ Foobar2000 XMMS";
        $conf[] = "STREAMSCANSIZE  2000";
    }
    else {
    	# renew downloads ?
        $conf[] = "RANGE           {$havp_config[F_RANGE]}";
        $conf[] = "\n# stream scan disabled";
        $conf[] = "STREAMSCANSIZE  0";
    }

    # scan options
    $conf[] = "SCANIMAGES      {$havp_config[F_SCANIMG]}";
    $conf[] = "MAXSCANSIZE     {$havp_config[F_SCANMAXSIZE]}";
    #
    $conf[] = "KEEPBACKBUFFER  200000";
    $conf[] = "KEEPBACKTIME    5";
    #
    $conf[] = "# After Trickling Time (seconds), some bytes are sent to browser to keep the connection alive";
    $conf[] = "TRICKLING       10";
    $conf[] = "TRICKLINGBYTES  1";
    #
    $conf[] = "# Downloads larger than MAXDOWNLOADSIZE will be blocked.";
    $conf[] = "MAXDOWNLOADSIZE {$havp_config[F_MAXDOWNLOADSIZE]}";
    #
    $conf[] = "\n# ClamAV Library Scanner (libclamav) ";
    $conf[] = "ENABLECLAMLIB        " . (HV_USE_CLAMD !== 'true' ? "true" : "false");
    # use clamd, if configured
    if (HV_USE_CLAMD === 'true') {
        $conf[] = "\n# Clamd scanner (Clam daemon)";
        $conf[] = "ENABLECLAMD          true";
        # clamd socket
        if (HV_CLAMD_TCPSOCKET === 'true') {
             $conf[] = "CLAMDSERVER          127.0.0.1";
             $conf[] = "CLAMDPORT            " . HVDEF_CLAM_TCPSOCKET;
        }
        else $conf[] = "CLAMDSOCKET          " . HVDEF_CLAM_SOCKET;
    }
    $conf[] = "";
    return implode("\n", $conf);
}
# ------------------------------------------------------------------------------
# Clamd config
# ------------------------------------------------------------------------------
function havp_config_clam()
{
    global $havp_config;

    $conf   = array();
    $conf[] =
"# ==============================================================================
# CLAMD config file
# This file generated automaticly with HAVP configurator (part of pfSense)
# (C)2008 Serg Dvoriancev
# email: dv_serg@mail.ru
# ==============================================================================
";
    $conf[] = "# log";
    $conf[] = "LogFileUnlock             yes";
    $conf[] = "LogFileMaxSize            2M";
    $conf[] = "LogTime                   yes";
    $conf[] = "LogClean                  no";
    $conf[] = "LogFacility               LOG_LOCAL6";
    $conf[] = "LogVerbose                " . ( HV_DEBUG === "true" ? "yes" : "no" );

    # Syslog
    $islog    = $havp_config[F_AVSETLOG]    === 'true';
    $issyslog = $havp_config[F_AVSETSYSLOG] === 'true';
    $conf[] = "LogSyslog                 " . ($islog && $issyslog ? 'yes' : 'no');
    if ($islog && !$issyslog)
        $conf[] = "LogFile                   " . HVDEF_CLAM_LOG;
    #
    $conf[] = "\n# sysdirs";
    $conf[] = "PidFile                   " . HVDEF_CLAM_PID;
    $conf[] = "TemporaryDirectory        " . HVDEF_TEMP_DIR;
    $conf[] = "DatabaseDirectory         /var/db/clamav";
    #
    $conf[] = "\n# socket";
    $conf[] = "LocalSocket               " . HVDEF_CLAM_SOCKET;
    $conf[] = "FixStaleSocket            yes";
    #
    if (HV_CLAMD_TCPSOCKET === 'true') {
        $conf[] = "TCPAddr                   127.0.0.1";
        $conf[] = "TCPSocket                 " . HVDEF_CLAM_TCPSOCKET;
    }
    $conf[] = "MaxConnectionQueueLength  30";
    #
    $conf[] = "\n# daemon";
    $conf[] = "MaxThreads                100";
    #
    $conf[] = "\n# scanner";
    $conf[] = "MaxDirectoryRecursion     255";
    $conf[] = "FollowDirectorySymlinks   no";    # not need scan symbol links dirs
    $conf[] = "FollowFileSymlinks        yes";
    $conf[] = "# perform a database check.(sec) [3600 sec = 60 min]";
    $conf[] = "SelfCheck                 3600";
    $conf[] = "# detect possibly unwanted applications.";
    $conf[] = "DetectPUA                 no";    # possible unwanted applications
    $conf[] = "AlgorithmicDetection      yes";
    $conf[] = "# executable";
	if ($havp_config[F_SCANBROKENEXE] === 'true')
	{$conf[] = "DetectBrokenExecutables   yes";}
	else
	{$conf[] = "DetectBrokenExecutables   no";}	
	#
    $conf[] = "ScanPE                    yes";
    $conf[] = "ScanELF                   yes";
    $conf[] = "# documents";
    $conf[] = "ScanOLE2                  yes";
    $conf[] = "ScanPDF                   yes";
    $conf[] = "# email";
    $conf[] = "ScanMail                  yes";
    $conf[] = "MailFollowURLs            no";
    $conf[] = "PhishingSignatures        yes";
    $conf[] = "PhishingScanURLs          yes";
    $conf[] = "PhishingAlwaysBlockSSLMismatch no";
    $conf[] = "PhishingAlwaysBlockCloak  no";
    $conf[] = "# html";
    $conf[] = "ScanHTML                  yes";
    $conf[] = "# archives";
    $conf[] = "ScanArchive               yes";
#    $conf[] = "ArchiveLimitMemoryUsage   no"; # deprecated on 0.95
    $conf[] = "ArchiveBlockEncrypted     no";
    $conf[] = "# limits";
    $conf[] = "MaxScanSize               50M";
    $conf[] = "MaxFileSize               30M";
    $conf[] = "MaxRecursion              255";
    $conf[] = "MaxFiles                  10000";
    #
    $conf[] = "\n# system";
    $conf[] = "User                      root"; # . HVDEF_USER; # mast have full access to files for scan
    $conf[] = "AllowSupplementaryGroups  yes";
    $conf[] = "Debug                     " . (HV_DEBUG === 'true' ? "yes" : "no");
    #
    $conf[] = "";
    return implode("\n", $conf);
}
# ------------------------------------------------------------------------------
# FreshClamAV config
# ------------------------------------------------------------------------------
function havp_config_freshclam()
{
    global $havp_config;
    $pfconf = $havp_config;
    $conf = array();

    $conf[] =
"# ==============================================================================
# freshclam(HAVP) config file
# This file generated automaticly with HAVP configurator (part of pfSense)
# (C)2008 Serg Dvoriancev
# email: dv_serg@mail.ru
# ==============================================================================
";
    $conf[] = "DatabaseDirectory /var/db/clamav";

    # log

    $conf[] = "LogFileMaxSize           2M";
    $conf[] = "LogTime                  yes";
    $conf[] = "LogVerbose               " . ( HV_DEBUG === "true" ? "yes" : "no" );
    $conf[] = "LogFacility              LOG_LOCAL6"; # LOG_LOCAL6 | LOG_MAIL
    $conf[] = "\n# syslog";

    # Syslog
    $is_syslog = ($pfconf[F_AVSETLOG] === 'true') && ($pfconf[F_AVSETSYSLOG] === 'true');
    $conf[] = "LogSyslog                " . ( $is_syslog ? 'yes' : 'no');
    unset ($is_syslog);

    # log
    # freshclam for 1.2.x have a bug with logfile permissions; now disable logfile for 1.2.x - only syslog
    $is_log = (pfsense_version_() != "1") && ($pfconf[F_AVSETLOG] === 'true');
    if ($is_log) {
        $conf[] = "UpdateLogFile            " . HVDEF_FRESHCLAM_LOG;
    } else {
        $conf[] = "# for pfsense 1.2.x Log disabled - permission bug exists!";
    }
    unset ($is_log);

    $conf[] = "\n# pid";
    $conf[] = "PidFile /var/run/clamav/freshclam.pid";

    $conf[] = "\n# db";
    $conf[] = "DatabaseOwner            havp";
    $conf[] = "AllowSupplementaryGroups yes";
    $conf[] = "DNSDatabaseInfo          current.cvd.clamav.net";

    $avsrv = $pfconf[F_AVUPDATESERVER];
    $avsrv = explode(" ", trim($avsrv));

    foreach ($avsrv as $asr)
    	if (!empty($asr))
             $conf[] = "DatabaseMirror  $asr";

	# regional mirror
    if (!empty($pfconf[F_DBREGION])) {
    	$conf[] = '# regional db';
		switch($pfconf[F_DBREGION]) {
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
			case 'us': $conf[] = "DatabaseMirror db.us.clamav.net "; break;        # united states
			default: break;
		}
	}

    $conf[] = "DatabaseMirror db.at.clamav.net";
    $conf[] = "DatabaseMirror db.au.clamav.net";
    $conf[] = "DatabaseMirror db.ba.clamav.net";
    $conf[] = "DatabaseMirror db.be.clamav.net";
    $conf[] = "DatabaseMirror db.ca.clamav.net";
    $conf[] = "DatabaseMirror db.ch.clamav.net";
    $conf[] = "DatabaseMirror db.cn.clamav.net";
    $conf[] = "DatabaseMirror db.cr.clamav.net";
    $conf[] = "DatabaseMirror db.cy.clamav.net";
    $conf[] = "DatabaseMirror db.cz.clamav.net";
    $conf[] = "DatabaseMirror db.de.clamav.net";
    $conf[] = "DatabaseMirror db.dk.clamav.net";
    $conf[] = "DatabaseMirror db.ec.clamav.net";
    $conf[] = "DatabaseMirror db.ee.clamav.net";
    $conf[] = "DatabaseMirror db.es.clamav.net";
    $conf[] = "DatabaseMirror db.fi.clamav.net";
    $conf[] = "DatabaseMirror db.fr.clamav.net";
    $conf[] = "DatabaseMirror db.gr.clamav.net";
    $conf[] = "DatabaseMirror db.hk.clamav.net";
    $conf[] = "DatabaseMirror db.hu.clamav.net";
    $conf[] = "DatabaseMirror db.id.clamav.net";
    $conf[] = "DatabaseMirror db.ie.clamav.net";
    $conf[] = "DatabaseMirror db.it.clamav.net";
    $conf[] = "DatabaseMirror db.jp.clamav.net";
    $conf[] = "DatabaseMirror db.kr.clamav.net";
    $conf[] = "DatabaseMirror db.li.clamav.net";
    $conf[] = "DatabaseMirror db.lt.clamav.net";
    $conf[] = "DatabaseMirror db.lv.clamav.net";
    $conf[] = "DatabaseMirror db.mt.clamav.net";
    $conf[] = "DatabaseMirror db.my.clamav.net";
    $conf[] = "DatabaseMirror db.ml.clamav.net";
    $conf[] = "DatabaseMirror db.no.clamav.net";
    $conf[] = "DatabaseMirror db.pl.clamav.net";
    $conf[] = "DatabaseMirror db.pt.clamav.net";
    $conf[] = "DatabaseMirror db.ro.clamav.net";
    $conf[] = "DatabaseMirror db.ru.clamav.net";
    $conf[] = "DatabaseMirror db.se.clamav.net";
    $conf[] = "DatabaseMirror db.sk.clamav.net";
    $conf[] = "DatabaseMirror db.th.clamav.net";
    $conf[] = "DatabaseMirror db.tr.clamav.net";
    $conf[] = "DatabaseMirror db.tw.clamav.net";
    $conf[] = "DatabaseMirror db.ua.clamav.net";
    $conf[] = "DatabaseMirror db.uk.clamav.net";
    $conf[] = "DatabaseMirror db.za.clamav.net";

    $conf[] = "\n# DO NOT TOUCH the following line ";
    $conf[] = "DatabaseMirror database.clamav.net";

    $conf[] = "\n# Number of database checks per day. Default: 12 (every two hours)";
    $chks = 0;
    $conf[] = "Checks $chks";

    $conf[] = "# notification";
    $conf[] = "OnUpdateExecute  date \"+%Y.%m.%d %H:%M:%S Antivirus update success\" > " . HVDEF_FRESHCLAM_STATUS_FILE;
    $conf[] = "OnErrorExecute   date \"+%Y.%m.%d %H:%M:%S Antivirus update error\" > "   . HVDEF_FRESHCLAM_STATUS_FILE;

    $conf[] = "Debug  " . (HV_DEBUG === 'true' ? "yes" : "no");

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
#Debug yes

    # use google safesearch AV database
    $conf[] = "SafeBrowsing     yes";

    $conf[] = "";
    return implode("\n", $conf);
}
# ------------------------------------------------------------------------------
# configure squid
function havp_configure_squid()
{
	global $config, $havp_config;
	$new_opt = array();
    $on_configure = ($havp_config[F_PROXYMODE] === 'squid' ? true : false);

    if (!isset($config['installedpackages']['squid']['config'][0]['custom_options'])) return;

    if ($on_configure === true) {
        $new_opt[] = "never_direct allow all";
        $new_opt[] = "cache_peer 127.0.0.1 parent {$havp_config[F_PROXYPORT]} 0 name=havp no-query no-digest no-netdb-exchange default";
    }

    # copy options, but not 'cache_peer' option
    $cust_opt = explode(";", $config['installedpackages']['squid']['config'][0]['custom_options']);
    foreach($cust_opt as $key => $val) {
        if (strpos($val, "never_direct") !== false) continue;
        if (strpos($val, "cache_peer 127.0.0.1 parent") !== false) continue;
        $new_opt[] = $val;
    }

    $new_opt = implode(";", $new_opt);
    if (/*is_package_installed('squid') &&  */file_exists('/usr/local/pkg/squid.inc')) {
        # squid config update
        $config['installedpackages']['squid']['config'][0]['custom_options'] = $new_opt;

        # disable upstream proxy
        if ($on_configure === true)
            $config['installedpackages']['squidupstream']['config'][0]['proxy_forwarding'] = '';

        write_config('Update redirector options to squid config.');
	    require_once('squid.inc');
        squid_resync();
	 }
}
# ------------------------------------------------------------------------------
function havp_whitelist_def()
{
    $whitelist  = array();

    $whitelist[] = "*sourceforge.net/*clamav-*";
    $whitelist[] = "*pfsense.com/*";
    $whitelist[] = "*.microsoft.com/*";
    $whitelist[] = "*.windowsupdate.com/*";         # M$ & M$ update
    # media and image extensions
    $whitelist[] = "*/*.gif\n*/*.swf\n*/*.png\n*/*.jpg\n*/*.jpeg\n*/*.mov\n*/*.avi\n*/*.flv\n*/*.bmp\n*/*.ico\n*/*.pdf\n*/*.mp3\n*/*.wma\n*/*.wmv\n*/*.ogg";

    return implode("\n", $whitelist);
}
# ==============================================================================
# Utils
# ==============================================================================
function havp_set_file_access($dir, $owner, $mod)
{
    if ( file_exists($dir) ) {
        mwexec("chgrp -R -v $owner $dir");
        mwexec("chown -R -v $owner $dir");
        if (!empty($mod)) {
             mwexec( "chmod -R -v $mod $dir");
        }
    }
}
# ------------------------------------------------------------------------------
# Src from squid.inc Copyright (C) 2006 Scott Ullrich, Fernando Lemos
function get_real_interface_address($iface)
{
        global $config;

        if ($iface === 'localhost') return array('127.0.0.1', '');

        $iface = convert_friendly_interface_to_real_interface_name($iface);
        $line = trim(shell_exec("ifconfig $iface | grep inet | grep -v inet6"));
        list($dummy, $ip, $dummy2, $netmask) = explode(" ", $line);

        return array($ip, long2ip(hexdec($netmask)));
}
#-------------------------------------------------------------------------------
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
#-------------------------------------------------------------------------------
function check_bw_domain($_dm)
{
    $domain = "";
    $path = "";

    if (!is_string($_dm)) return false;

    $pos = strpos($_dm, "/");
    if ($pos === false) {
        $domain = $_dm;
        $path = "";
    }
    else {
        $domain = substr($_dm, 0, $pos);
        $path   = substr($_dm, $pos+1);
    }

    # Domains can have a wildcard at begin '*domain.xx' - *my.domain.com
    # Path can have a wildcard(*) at begin and end '*xxx*'
    # Regex:  * - {0,}; + - {1,}; ? = {0,1}
    $df     = "[a-zA-Z0-9\-]";
    $dm_fmt = "^((\*)|(\*\.))?($df+\.)+$df{2,}$";        # d.com *d.com *.d.com
    $ph_fmt = "^((\*)|((\*)?([^\*]+)(\*)?))$";           # *path*

    if (empty($path)) {
    	# d.com *d.com *.d.com
        return eregi($dm_fmt, $domain);
    }
    else {
        if (!empty($domain)) {
            return (($domain === '*') || eregi($dm_fmt, $domain)) && eregi($ph_fmt, $path);
        }
    }

    return false;
}

# ------------------------------------------------------------------------------
# cron
# ------------------------------------------------------------------------------
# $options: [0]='minute', [1]='hour', [2]='mday', [3]='month', [4]='wday', [5]='who', [6]='command'
#
function havp_setup_cron($task_key, $options, $on_off)
{
    global $config;
	$cron_item = array();

    # $on_off = TRUE/FALSE - install/deinstall cron task:
    # prepare new cron item
	if (is_array($options)) {
		$cron_item['minute']  =  $options[0];
        $cron_item['hour']    =  $options[1];
        $cron_item['mday']    =  $options[2];
        $cron_item['month']   =  $options[3];
        $cron_item['wday']    =  $options[4];
        $cron_item['who']     = ($options[5]) ? $options[5] : 'nobody';
        $cron_item['command'] =  $options[6];
    }

    # unset old cron task with $task_key
    if (!empty($task_key)) {
        $flag_cron_upd = false;
        # delete old cron task if exists
        if (is_array($config['cron']['item'])) {
            foreach($config['cron']['item'] as $key => $val) {
                if (strpos($config['cron']['item'][$key]['command'], $task_key) !== false) {
                    unset($config['cron']['item'][$key]);
                    $flag_cron_upd = true;
                    break;
                }
            }
        }

        # set new cron task
        if (($on_off === true) and !empty($cron_item)) {
            $config['cron']['item'][] = $cron_item;
            $flag_cron_upd = true;
        }

        # write config and configure cron only if cron task modified
        if ($flag_cron_upd === true) {
            write_config("Installed cron task '$task_key' for 'havp' package");
            configure_cron();
        }
    }
    else {
        # ! error $name !
        return;
    }
}
# ------------------------------------------------------------------------------
# filter rules
# ------------------------------------------------------------------------------
function havp_generate_rules($type = 'filter')
{
    # pfSense v.2.x - welcome !

    # 'nat' 'filter'
    global $config, $havp_config;
    $rules = array();

    # no rules if havp disabled
    if ($havp_config[F_ENABLE] !== 'true') {
        return '';
    }

    $proxymode = $havp_config[F_PROXYMODE];
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # =-= HAVP always listen 127.0.0.1:port =-=
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Proxy mode:
    # Standard         - Filter: Rdr ifaces:port => 127.0.0.1:port
    # Parent for Squid - Filter: No
    # Transparent      - Filter: Rdr ifaces:port => 127.0.0.1:port;
    #                    Rdr Any Http => 127.0.0.1:port + Allow Http traffic via iface
    #                    If Squid transparent, then as Standard.
    # Internal         - Filter: No
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    $proxybindiface = 'lo0'; # 127.0.0.1
    $ifaces    = array_map('convert_friendly_interface_to_real_interface_name', explode(',', $havp_config[F_PROXYINTERFACE]));
    $proxyport = ( $havp_config[F_PROXYPORT] ? $havp_config[F_PROXYPORT] : HVDEF_PROXYPORT );

    # squid already transparent
    $squid_transparent_proxy = ($config['installedpackages']['squid']['config'][0]['transparent_proxy'] == 'on');
    if (($proxymode === 'transparent') && $squid_transparent_proxy) {
        $proxymode = 'standard';
        log_error("Havp: Squid is already configured as transparent proxy. Use 'Standard' proxy mode.");
    }

    # nat
    if ($type == 'nat') {
        $rules[] = "";
        $rules[] = "# havp proxy ifaces redirect";
        foreach($ifaces as $iface) {
            switch($proxymode) {
                case 'transparent':
                    # rdr any http => localhost:port
                    $rules[] = "rdr on $iface proto tcp from any to !($iface) port 80 -> $proxybindiface port $proxyport";
                case 'standard':
                case 'squid':
                    # rdr iface:port => localhost:port
                    $rules[] = "rdr on $iface proto tcp from any to ($iface) port $proxyport -> $proxybindiface port $proxyport";
                    break;
                    # no more rdr
                case 'internal':
                    default: break;
            }
        }
        $rules[] = "";
    }

    # filter
    if ($type == 'filter' || $type == 'rule') {
        $rules[] = "";
        $rules[] = "# havp proxy ifaces rules";
        foreach($ifaces as $iface) {
            switch($proxymode) {
                case 'transparent':
                    # pass http on iface
                    $rules[] = "pass in quick on $iface proto tcp from any to !($iface) port 80 flags S/SA keep state";
                    break;
                # no more rules
                case 'standard':
                case 'squid':
                case 'internal':
                    default: break;
            }
        }
        $rules[] = "";
    }

    if ($type == 'pfearly') {

    }

    if ($type == 'pflate') {

    }

    # test
    # file_put_contents("/tmp/havp_".$type, "state: $proxymode\n" . implode("\n", $rules));

    return implode("\n", $rules);
}
# ------------------------------------------------------------------------------
function havp_filter_update_3()
{
    # for 1.x only
    if (pfsense_version_() != '1') return;

	$rules_file = '/tmp/rules.debug';
    if (file_exists($rules_file)) {
        $newrules = array();
        $rules = file_get_contents($rules_file);
        $rules = explode("\n", $rules);

        foreach($rules as $val) {
        	$newrules[] = $val;
            # rdr
        	if (trim($val) === "rdr-anchor \"miniupnpd\"") {
        	    $newrules[] = "# havp rdr";
        	    $newrules[] = havp_generate_rules('nat');
        	    $newrules[] = "";
        	}
        	# rules
        	elseif(trim($val) === "anchor \"miniupnpd\"") {
        	    $newrules[] = "# havp rules";
        	    $newrules[] = havp_generate_rules('filter');
        	    $newrules[] = "";
        	}
        	$rules = implode("\n", $newrules);
        }
        file_put_contents($rules_file, $rules);
        mwexec("pfctl -f $rules_file");
    }
}

# ------------------------------------------------------------------------------
function havp_update_AV()
{
    # AV update script
    file_put_contents(HVDEF_AVUPD_SCRIPT, havp_AVupdate_script());
    havp_set_file_access(HVDEF_AVUPD_SCRIPT, HVDEF_AVUSER, '0755');
    mwexec_bg(HVDEF_AVUPD_SCRIPT); # run update background
}
# ==============================================================================
# Scripts
# ==============================================================================
# AV update script
function havp_AVupdate_script()
{
$f = HVDEF_UPD_STATUS_FILE;
$u = HVDEF_FRESHCLAM_STATUS_FILE;
return <<<EOD
#!/bin/sh
date +"%Y.%m.%d %H:%M:%S Antivirus update started."  > $f
date +"%Y.%m.%d %H:%M:%S Antivirus database already is updated." > $u
/usr/local/bin/freshclam
wait
cat $u >> $f
/usr/local/bin/sigtool --unpack-current daily.cvd
/usr/local/bin/sigtool --unpack-current main.cvd
wait
date +"%Y.%m.%d %H:%M:%S Antivirus update end." >> $f
EOD;

}
# ------------------------------------------------------------------------------
# HAVP service startup script
function havp_startup_script()
{
    global $havp_config;
    $pid = HVDEF_PID_FILE;
    $havpchk  = "ps auxw | grep \"[h]avp -c\"|awk '{print $2}'";
    $clamdchk = "ps auxw | grep \"[c]lamd -c\"|awk '{print $2}'";

    # rc script
    $rc = array();
    $rc['file'] = basename(HVDEF_HAVP_STARTUP_SCRIPT);

    $s[] = "\t# init";
    $s[] = "\techo 'Starting ..' > " . HVDEF_HAVP_STATUS_FILE;

    $s[] = "\t# start";
    $s[] = "\tif [ -z \"`{$havpchk}`\" ];then";
    if (HV_USE_CLAMD === 'true') {
        $clampid_dir = HVDEF_CLAM_RUNDIR;
        $s[] = "\t\t# start clamd before (to be sure)";
        $s[] = "\t\t" . HVDEF_CLAM_STARTUP_SCRIPT . " start";
        $s[] = "\t\tsleep 2";
        $s[] = "";

        $s[] = "\t\t# if clamd started";
        $s[] = "\t\tif [ -n \"`{$clamdchk}`\" ];then";
        $s[] = "\t\t\t# Waiting CLAMD";
        $s[] = "\t\t\techo -n \"Waiting CLAMD \"";
        $s[] = "\t\t\techo 'Waiting CLAMD' > " . HVDEF_HAVP_STATUS_FILE;
        $s[] = "\t\t\twhile [ \"`{$clamdchk}`\" != \"`/bin/cat {$clampid_dir}/clamd.pid`\" ];do";
        $s[] = "\t\t\t\techo -n '.'";
        $s[] = "\t\t\t\tsleep 1";
        $s[] = "\t\t\tdone";
        $s[] = "\t\t\techo";
        $s[] = "\t\tfi";
        $s[] = "";
    }
    $s[] = "\t\t/usr/local/sbin/havp -c " . HVDEF_HAVP_CONFIG . " 2>/dev/null";
    $s[] = "\t\twait";
    $s[] = "\tfi";

    $s[] = "\t# Status";
    $s[] = "\tif [ -z \"`{$havpchk}`\" ];then";
    $s[] = "\t\techo 'Stopped' > " . HVDEF_HAVP_STATUS_FILE;
    $s[] = "\telse";
    $s[] = "\t\techo 'Started' > " . HVDEF_HAVP_STATUS_FILE;
    $s[] = "\tfi";

    $s[] = "";
    $rc['start'] = implode("\n", $s);
    unset($s);

    $s[] = "# stop";
    $s[] = "\t killall havp 2>/dev/null";
    $s[] = "\t sleep 2";
    $s[] = "\t killall -9 havp 2>/dev/null";
    $s[] = "\t wait";
    $s[] = "\t echo 'Stopped' > " . HVDEF_HAVP_STATUS_FILE;
    $s[] = "";
    $rc['stop'] = implode("\n", $s);
    unset($s);

    # we don't use start if package disabled
    if ($havp_config[F_ENABLE] !== 'true') {
        $rc['start'] = "\t echo 'Disabled' > " . HVDEF_HAVP_STATUS_FILE;
    }

    write_rcfile($rc);
}
# ------------------------------------------------------------------------------
# clamd service startup script
function hv_clamd_startup_script()
{
    global $havp_config;
    $pid = HVDEF_CLAM_PID;
    $clamdchk = "ps auxw | grep \"[c]lamd -c\"|awk '{print $2}'";

    # rc script
    $rc = array();
    $rc['file'] = basename(HVDEF_CLAM_STARTUP_SCRIPT);
    $s[] = "\t\techo 'Starting..' > " . HVDEF_CLAM_STATUS_FILE;

    $s[] = "# start";
    $s[] = "\tif [ -z \"`{$clamdchk}`\" ];then";
    $s[] = "\t\t/usr/local/sbin/clamd -c " . HVDEF_CLAM_CONFIG . " 2>/dev/null";
    $s[] = "\t\twait";
    $s[] = "\tfi";
    $s[] = "\techo 'Started' > " . HVDEF_CLAM_STATUS_FILE;
    $s[] = "";
    $rc['start'] = implode("\n", $s);
    unset($s);

    $s[] = "#stop";
    $s[] = "\t killall clamd 2>/dev/null";
    $s[] = "\t sleep 2";
    $s[] = "\t killall -9 clamd 2>/dev/null";
    $s[] = "\t wait";
    $s[] = "\t\techo 'Stopped' > " . HVDEF_CLAM_STATUS_FILE;
    $s[] = "";
    $rc['stop'] = implode("\n", $s);
    unset($s);

    write_rcfile($rc);
}

# ------------------------------------------------------------------------------
# HAVP filter resync script
function havp_filter_resync_script()
{

return <<<EOD
#!/usr/local/bin/php -f
<?php
# havp filter hook
if (file_exists('/usr/local/pkg/havp.inc')) {
	require_once('havp.inc');
	havp_filter_update_3();
}
?>
EOD;

}

# ==============================================================================
# RAM Disk
# ==============================================================================
function mountRAMdisk($free_and_mount = true)
{
    global $havp_config;
    $mnt_point = HVDEF_RAMTEMP_DIR;
    $mnt_flag_file = "$mnt_point/.mnt";

    # RAM Disk disabled
    if (HV_USE_TMPRAMDISK !== 'true') {
        umountRAMDisk();
        return;
    }

    # RAM Disk on VM disabled
    if ((HV_VM_TMPRAMDISK  !== 'true') && VMWare_detect()) {
        umountRAMDisk();
   	    log_error("havp: RAMDisk on VM disabled.");
        return;
    }

    # free RAMDisk only
    if ($free_and_mount !== true) {
        umountRAMDisk();
        return;
    }

    # =-= Temp RAMDisk =-=
    # note: use 1/4 of system memory capacity
    $ramdisk_capacity = get_memory();
    $ramdisk_capacity = intval(intval($ramdisk_capacity[0]) / 4); # [Mb]

    # RAMDisk already exists?
    if (file_exists("/dev/md10")) return;
    # umount old RAMDisk
#    umountRAMDisk();

    # create and mount a swap backed file system on /var/tmp/havp by /dev/md10:
# SWAP
#    mwexec("mdconfig -a -t swap -s {$ramdisk_capacity}M -u 10");
#    mwexec("newfs -U /dev/md10");
#    mwexec("mount /dev/md10 $mnt_point");
# RAM - more quickly, used physical RAM
    mwexec("/sbin/mdmfs -s {$ramdisk_capacity}M md10 {$mnt_point}");
    mwexec("chmod 1777 {$mnt_point}");

    # create flag file
    file_put_contents($mnt_flag_file, "{$ramdisk_capacity}");
    # syslog
    if (HV_DEBUG === 'true')
        log_error("havp: Create RAMDisk {$ramdisk_capacity}Mb.");
}
# ------------------------------------------------------------------------------
function umountRAMDisk()
{
    global $havp_config;

    # detach and free all resources used by /dev/md10:
    mwexec("umount -f " . HVDEF_RAMTEMP_DIR);
    mwexec("mdconfig -d -u 10");
}

# ==============================================================================
# Utilites
# ==============================================================================
function VMWare_detect()
{
    global $g;
    $fc = '';

    if (file_exists("{$g['varlog_path']}/dmesg.boot") !== false)
        $fc = file_get_contents("{$g['varlog_path']}/dmesg.boot");

    return (strpos($fc, "<VMware Virtual") !== false);
}

function pfsense_version_()
{
    $ver = '1';

    if (file_exists('/etc/version')) {
        $s = file_get_contents('/etc/version');
        $s = str_replace('-', '.', $s); # '2.0-Beta' > '2.0.Beta'
        $s = explode(".", $s);
        $ver = $s ? $s[0] : '1';
    }

    return intval($ver);
}

# ------------------------------------------------------------------------------
function start_antivirus_scanner($filename)
{
    $param = array();
#   $param[] = "-v";                                                            # verbose
    if (HV_DEBUG === 'true')
         $param[] = "--debug";                                                  # debug option
    else $param[] = "--quiet";                                                  # output only errors
    $param[] = "--stdout";                                                      # Write to stdout instead of stderr
#   $param[] = "--no-summary";                                                  # Disable summary at end of scanning
    $param[] = "-i";                                                            # Only print infected files
    $param[] = "--tempdir=" . HVDEF_TEMP_DIR;                                   # Create temporary files in DIRECTORY
#   $param[] = "-d FILE/DIR";                                                   # Load virus database from FILE or load all .cvd and .db[2] files from DIR
    $param[] = "-l " . HVDEF_CLAMSCAN_LOG;                                      # Save scan report to FILE
    $param[] = "-r";                                                            # Scan subdirectories recursively
    $param[] = "--remove";                                                      # Remove infected files. Be careful!
    $param[] = "--detect-broken";                                               # Try to detect broken executable files
    $param[] = "--max-filesize=10000000";                                       # Files larger than this will be skipped and assumed clean
    $param[] = "--max-scansize=5000000";                                        # The maximum amount of data to scan for each container file (*)
    $param[] = "--max-files=10000";                                             # The maximum number of files to scan for each container file (*)
    $param[] = "--max-recursion=255";                                           # Maximum archive recursion level for container file (*)
    $param[] = "--max-dir-recursion=255";                                       # Maximum directory recursion level

    $param = implode(" ", $param);
    if (HV_USE_CLAMD === 'true')
         $param = "clamdscan $param $filename"; # use clamd daemon (more quickly)
    else $param = "clamscan  $param $filename";

    # debug clamscan cmd
    if (HV_DEBUG === 'true') file_put_contents("/tmp/clamscan.cmd", $param);

    if (file_exists($filename)) {
    	 log_error("Antivirus: Starting file '$filename' scanner. Log file is '" . HVDEF_CLAMSCAN_LOG . "'. Wait 5-10 minutes.");

    	 # put to log scanning file
    	 $cont="Starting scan file {$filename}\n";
        file_put_contents(HVDEF_CLAMSCAN_LOG, $cont);

        mwexec_bg("$param");
        exec("date +\"%Y.%m.%d %H:%M:%S Starting scan file '$filename'.\" > " . HVDEF_CLAMSCAN_LOG);
    }
    else log_error("Antivirus: Can't starting file scanner. File '$filename' not exists.");
}

# ------------------------------------------------------------------------------
# HTML
# ------------------------------------------------------------------------------
function havp_fscan_html()
{
    global $g;
    $clamscan_log = HVDEF_CLAMSCAN_LOG;

    return <<<EOD
<hr>
<span onClick="document.getElementById('scanfilepath').value = '/var/squid';" style="cursor: pointer;">
    <img src='./themes/{$g['theme']}/images/icons/icon_pass.gif' title='Click here'>
        <font size='-1'><u>&nbsp;Squid cache path (scan your squid cache now).</u></font>
    </img>
</span>
<br>
<span onClick="document.getElementById('scanfilepath').value = '/var/db';" style="cursor: pointer;">
    <img src='./themes/{$g['theme']}/images/icons/icon_pass.gif' title='Click here'>
        <font size='-1'><u>&nbsp;Common DB path.</u></font>
    </img>
</span>
<br>
<span onClick="document.getElementById('scanfilepath').value = '/tmp';" style="cursor: pointer;">
    <img src='./themes/{$g['theme']}/images/icons/icon_pass.gif' title='Click here'>
        <font size='-1'><u>&nbsp;Temp path.</u></font>
    </img>
</span>
<hr>
<input name='submit' type='submit' value='Start_scan'><br>
Press button for start antivirus scanner now. After 5-10 minutes look log file '{$clamscan_log}'.<br>
(Diagnostics: Execute Shell command: <b>'cat {$clamscan_log}'</b>)
EOD;

}

/* Future - in next time */
# blacklist, dns, down, error, invalid, maxsize, request, scanner, virus
function havp_html_notification_page($type, $title, $notify, $message)
{
    $class = '';
    switch($type) {
        case 'blacklist': $class = 'notify-warn';     break;
        case 'dns':       $class = 'notify-standart'; break;
        case 'down':      $class = 'notify-standart'; break;
        case 'error':     $class = 'notify-standart'; break;
        case 'invalid':   $class = 'notify-standart'; break;
        case 'maxsize':   $class = 'notify-warn';     break;
        case 'request':   $class = 'notify-standart'; break;
        case 'scanner':   $class = 'notify-warn';     break;
        case 'virus':     $class = 'notify-danger';   break;
   }

    return <<<EOD
<html>
 <head>
  <meta http-equiv="content-type" content="text/html; ">
  <style type="text/css">
   <!--
    .havp_scheme			{width: 100%; border: 0px; color: black; vertical-align: bottom; text-align: center; font-family: arial,helvetica; padding-bottom: 3%}
    .havp_scheme.header 		{font-size: 10pt; font-weight: bold; background-color: #FFFFFF; color: #446699;}
    .havp_scheme.notify		{font-size: 14pt; font-weight: bold; background-color: #E9E9E9; color: #446699;}
    .havp_scheme.notify-standart	{font-size: 14pt; font-weight: bold; background-color: #E9E9E9; color: #446699;}
    .havp_scheme.notify-strong  	{font-size: 14pt; font-weight: bold; background-color: #E9E9E9; color: #446699;}
    .havp_scheme.notify-danger  	{font-size: 14pt; font-weight: bold; background-color: #FFEFEF; color: #FF6666;}
    .havp_scheme.notify-warn    	{font-size: 14pt; font-weight: bold; background-color: #FFEFDF; color: #FF9966;}
    .havp_scheme.message  		{font-size: 10pt; background-color: #FFFFFF; color: #000066;}
    .havp_scheme.footer		{font-size: 10pt; background-color: #DDDDDD; color: #000066;}
   -->
  </style
  <title>HTTP AntiVirus Proxy: $type</title>
 </head>
 <body>
  <table class='havp_scheme' cellpadding='2' cellspacing='0' align='center'>
   <tr class='header'><td>$title<br>HTTP AntiVirus Proxy: $type</td></tr>
   <tr class='$class'><td>$notify</td></tr>
   <tr class='message'><td>$message<br><!--message--></td></tr>
   <tr class='footer' ><td>Powered by havp.</td></tr>
  </table>
 </body>
</html>
EOD;

}

# ==============================================================================
# Status, widgets
# ==============================================================================
function havp_get_scan_log()
{
    $s = '';
    $clamscanlog = "/var/log/clamscan.log";
    if (file_exists($clamscanlog)) {
        $s = file_get_contents("/var/log/clamscan.log");
    }
    if (empty($s)) $s = "Not found.";
    return $s;
}

function havp_get_filescanlist()
{
    $slist = array();
    $slist[0]['descr'] = 'Squid cache path (scan you squid cache now).';
    $slist[0]['path']  = '/var/squid';
    $slist[1]['descr'] = 'Common DB path.';
    $slist[1]['path']  = '/var/db';
    $slist[2]['descr'] = 'Temp path.';
    $slist[2]['path']  = '/tmp';
    return $slist;
}

function havp_get_av_viruslog()
{
    $s = array();
    if (file_exists(HVDEF_HAVP_ACCESSLOG)) {
        $log   = file_get_contents(HVDEF_HAVP_ACCESSLOG);

        $log   = explode("\n", $log);
        $count = 0;
        foreach($log as $ln) {
            if (substr_count(strtolower($ln), "virus clamd:"))
                $s[] = $ln;
        }
    }

    return $s;
}

function havp_get_av_statistic()
{
    $s = "Unknown.";
    if (file_exists(HVDEF_HAVP_ACCESSLOG)) {
        $log   = file_get_contents(HVDEF_HAVP_ACCESSLOG);

        $count = substr_count(strtolower($log), "virus clamd:");
        $s     = "Found $count viruses (total).";
    }

    return $s;
}

# ------------------------------------------------------------------------------
# Fix
# ------------------------------------------------------------------------------
function havp_fix()
{
    # remove old named scripts
    # now must exists 'havp.sh'/'clamd'
    mwexec(HVDEF_SCRIPT_DIR . "/havp");
    mwexec(HVDEF_SCRIPT_DIR . "/clamd.sh");
}

?>
