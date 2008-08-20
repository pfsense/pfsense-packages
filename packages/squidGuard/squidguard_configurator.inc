<?php
# ------------------------------------------------------------------------------
/*  squidguard_configurator.inc
    (C)2006-2008 Serg Dvoriancev
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
# ------------------------------------------------------------------------------
# SquidGuard Configurator
# email: dv_serg@mail.ru
# ------------------------------------------------------------------------------
# squidGuard inline options:
# squidGuard -C all          - update database
# squidGuard -c <configfile> - create squidGuard with specified config file
# ------------------------------------------------------------------------------
# Notes:
#   for work squidGuard need present ALL destinations;
#   if dest not present in config - then this item will ignored in operations
#   (in rebuild DB for example)
# ------------------------------------------------------------------------------
# Directories:
# work path - $workdir
# log path  - $workdir + $logdir
# ------------------------------------------------------------------------------

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');
require_once('squid.inc');

/* Allow additional execution time 0 = no limit. */
ini_set('max_execution_time', '3600');
ini_set('max_input_time', '3600');

# ------------------------------------------------------------------------------
# files header
# ------------------------------------------------------------------------------
define('FILES_DB_HEADER', '
# ------------------------------------------------------------------------------
# File created by squidGuard package GUI
# (C)2006-2008 Serg Dvoriancev
# ------------------------------------------------------------------------------
');

define('CONFIG_SG_HEADER', "
# ============================================================
# SquidGuard configuration file
# This file generated automaticly with SquidGuard configurator
# (C)2006 Serg Dvoriancev
# email: dv_serg@mail.ru
# ============================================================
");

define('ACL_WARNING_ABSENSE_PASS', "!WARNING! Absence PASS 'all' or 'none' added as 'none'");

# ------------------------------------------------------------------------------
# squid config options
# ------------------------------------------------------------------------------
define('REDIRECTOR_OPTIONS_REM', '# squidGuard options');
define('REDIRECTOR_PROGRAM_OPT', 'redirect_program');
define('REDIRECT_BYPASS_OPT',    'redirector_bypass');
define('REDIRECT_CHILDREN_OPT',  'redirect_children');

# ------------------------------------------------------------------------------
# setup count redirector processes will started
# * for big count users service increase this option, but you need use this on powerful system
# ------------------------------------------------------------------------------
define('REDIRECTOR_PROCESS_COUNT', '3');

# ------------------------------------------------------------------------------
# squidguard config options
# ------------------------------------------------------------------------------
# define default redirection url (redirector get this url for all blocked url's)
# * !ATTENTION! this url must be exists; IF url not exist, redirector will't block
#               (returned to squid some url, what blocked)
# ------------------------------------------------------------------------------
define('REDIRECT_BASE_URL', '/sgerror.php');
define('REDIRECT_URL_ARGS', '&a=%a&n=%n&i=%i&s=%s&t=%t&u=%u');

# ------------------------------------------------------------------------------
# squidguard system defines
# ------------------------------------------------------------------------------
define('SQUID_CONFIGFILE',             '/usr/local/etc/squid/squid.conf');
define('TMP_DIR',                      '/var/tmp');

define('SQUIDGUARD_CONFIGFILE',        '/squidGuard.conf');
define('SQUIDGUARDCONF_LOGFILE',       '/sg_configurator.log');
define('SQUIDGUARD_ACCESSBLOCK_FILE',  'block.log');
define('SQUIDGUARD_CONFBASE_DEF',      '/usr/local/etc/squid');
define('SQUIDGUARD_LOGDIR_DEF',        '/tmp');
define('SQUIDGUARD_WORKDIR_DEF',       '/usr/local/etc/squidGuard');
define('SQUIDGUARD_BINPATH_DEF',       '/usr/local/bin');
define('SQUIDGUARD_TMP',               '/var/tmp/squidGuard');                  # SG temp
define('SQUIDGUARD_VAR',               '/var/squidGuard');                      # SG variables
define('SQUIDGUARD_STATE',             '/squidGuard.state');
define('SQUIDGUARD_REBUILD',           '/squidGuard.rebuild');

# DB home catalog contains 'Blacklist' and 'User' sub-catalogs
define('SQUIDGUARD_DBHOME_DEF',        '/var/db/squidGuard');
define('SQUIDGUARD_DB_BLACKLIST',      '/bl');
define('SQUIDGUARD_DB_USER',           '/usr');
define('SQUIDGUARD_BL_UNPACK',         '/unpack');
define('SQUIDGUARD_BL_DB',             '/db');

# DB/Blacklist defines
define('SQUIDGUARD_BLK_ENTRIES',       '/blacklist.files');
define('BLACKLIST_ARCHIVE',            '/blacklists.tar');
define('BLK_LOCALFILE',                '/tmp/sg_blacklists.tar');
define('DB_REBUILD_SH',                '/tmp/squidGuard_db_rebuild.sh');
define('DB_REBUILD_CONF',              '/tmp/squidGuard_db_rebuild.conf');
define('DB_REBUILD_BLK_CONF',          '/squidGuard_blk_rebuild.conf');
define('BLK_TEMP',                     '/tmp/sg_blk');
define('SG_BLK_ARC',                   '/arcdb');                               # blk db archive
define('SG_INFO_FILE',                 '/var/squidGuard/sg_db_upd.inf');

# error_res
define('SG_ERR0', "Error! Check squidGuard configuration data.");

# redirect mode
define('RMOD_NONE',          'rmod_none');
define('RMOD_INT_ERRORPAGE', 'rmod_int');
define('RMOD_INT_BLANKPAGE', 'rmod_int_bpg');
define('RMOD_INT_BLANKIMG',  'rmod_int_bim');
define('RMOD_INT_SIZELIMIT', 'rmod_int_szl');
define('RMOD_EXT_ERR',       'rmod_ext_err');
define('RMOD_EXT_RDR',       'rmod_ext_rdr');
define('RMOD_EXT_MOVED',     'rmod_ext_mov');
define('RMOD_EXT_FOUND',     'rmod_ext_fnd');

# debug options
define('DEBUG_UPDATE_SQUID_CONF',        'true');
define('DEBUG_UPDATE_SQUIDGUARD_DB',     'true');
define('DEBUG_MAKE_SQUIDGUARD_CONFIG',   'true');

# GUI options
define('SQUIDGUARD_LOG_MAXCOUNT',        500);                                 # max log lines

# 0-all, 1-medium; 2-low
define('SQUIDGUARD_LOG_LEVEL',           0);

#
define('FLT_DEFAULT_ALL', 'all');
define('FLT_NOTALLOWIP',  '!in-addr');

# owner user name (squid system user - need for define rights access)
define('OWNER_NAME', 'proxy');

# Debug
define('DEBUG_ON', 'true');

# ==============================================================================
# black list
# ==============================================================================
# known black list standard names
# ------------------------------------------------------------------------------
define('FLT_AD',         'ads');
define('FLT_AGGRESSIVE', 'aggressive');
define('FLT_AUDIOVIDEO', 'audio-video');
define('FLT_DRUGGS',     'druggs');
define('FLT_GAMBLING',   'gambling');
define('FLT_HACKING',    'hacking');
define('FLT_MAIL',       'mail');
define('FLT_PORN',       'porn');
define('FLT_PROXY',      'proxy');
define('FLT_VIOLENCE',   'viol');
define('FLT_WAREZ',      'warez');

# ==============================================================================
# SquidGuard Configurator
# ==============================================================================
$squidguard_config = array(); # squidGuard config array

# call default init
sg_init();

# ------------------------------------------------------------------------------
# squidguard system fields
# ------------------------------------------------------------------------------
define('FLD_SQUIDGUARD',          'squidGuard');
define('FLD_LOGDIR',              'logdir');
define('FLD_DBHOME',              'dbhome');
define('FLD_WORKDIR',             'workdir');
define('FLD_BINPATH',             'binpath');
define('FLD_PROCCESSCOUNT',       'process_count');
define('FLD_SQUIDCONFIGFILE',     'squid_configfile');
define('FLD_ENABLED',             'enabled');
define('FLD_BLACKLISTENABLED',    'blacklist_enabled');
define('FLD_SGCONF_XML',          'sgxml_file');

# other fields
define('FLD_ITEM',                'item');
define('FLD_TIMES',               'times');
define('FLD_SOURCES',             'sources');
define('FLD_DESTINATIONS',        'destinations');
define('FLD_REWRITES',            'rewrites');
define('FLD_ACLS',                'acls');
define('FLD_DEFAULT',             'default');
define('FLD_NAME',                'name');
define('FLD_DESCRIPTION',         'description');
define('FLD_IP',                  'ip');
define('FLD_URLS',                'urls');
define('FLD_DOMAINS',             'domains');
define('FLD_EXPRESSIONS',         'expressions');
define('FLD_REDIRECT',            'redirect');
define('FLD_TARGETURL',           'targeturl');
define('FLD_REPLACETO',           'replaceto');
define('FLD_LOG',                 'log');
define('FLD_ITEM',                'item');
define('FLD_DISABLED',            'disabled');
define('FLD_TIMENAME',            'timename');
define('FLD_DESTINATIONNAME',     'destname');
define('FLD_REDIRECT',            'redirect');
define('FLD_REWRITE',             'rewrite');
define('FLD_REWRITENAME',         'rewritename');
define('FLD_OVERDESTINATIONNAME', 'overdestname');
define('FLD_OVERREDIRECT',        'overredirect');
define('FLD_OVERREWRITE',         'overrewrite');
define('FLD_OVERREWRITENAME',     'overrewritename');
define('FLD_TIMETYPE',            'timetype');
define('FLD_TIMEDAYS',            'timedays');
define('FLD_DATRANGE',            'daterange');
define('FLD_TIMERANGE',           'sg_timerange');
define('FLD_RMOD',                'redirect_mode');                     # [redirect_mode] = rmod_int <base- use sgerror.php>; rmod_301; rmod_302;
define('FLD_NOTALLOWINGIP',       'notallowingip');                     # not allowing ip in URL
define('FLD_USERNAME',            'username');

# transparent mode
define('FLD_SQUID_TRANSPARENT_MODE',   'squid_transparent_mode');
define('FLD_CURRENT_LAN_IP',           'current_lan_ip');
define('FLD_CURRENT_GUI_PORT',         'current_gui_port');
define('FLD_CURRENT_GUI_PROTO',        'current_gui_protocol');

# ------------------------------------------------------------------------------
# sg_init - initialize config array
# ------------------------------------------------------------------------------
function sg_init($init = '')
{
    global $squidguard_config;

    $squidguard_config = array();
    if(empty($init) or !is_array($init) ) {
        # default init (for generate minimal config)
        $squidguard_config[FLD_LOGDIR]          = SQUIDGUARD_LOGDIR_DEF;
        $squidguard_config[FLD_DBHOME]          = SQUIDGUARD_DBHOME_DEF;
        $squidguard_config[FLD_WORKDIR]         = SQUIDGUARD_WORKDIR_DEF;
        $squidguard_config[FLD_BINPATH]         = SQUIDGUARD_BINPATH_DEF;
        $squidguard_config[FLD_SQUIDCONFIGFILE] = SQUID_CONFIGFILE;
        $squidguard_config[FLD_PROCCESSCOUNT]   = REDIRECTOR_PROCESS_COUNT;
    } else {
        $squidguard_config = $init;
    }

    return $squidguard_config;
}

# ------------------------------------------------------------------------------
# sg_loadconfig_xml
# ------------------------------------------------------------------------------
function sg_load_configxml($filename)
{
    global $squidguard_config;

    sg_init();
    if (file_exists($filename)) {
        $xmlconf = file_get_contents($filename);

        if (!empty($xmlconf)) {
            $squidguard_config = $xmlconf[FLD_SQUIDGUARD];
            sg_addlog("sg_load_configxml: Success update from '$filename'.", 1);
        } else
            sg_addlog("sg_load_configxml: Error, file '$filename' is empty.", 2);
    } else
        sg_addlog("sg_load_configxml: Error, file '$filename' does not exists.", 2);
}

# ------------------------------------------------------------------------------
# sg_saveconfig_xml
# ------------------------------------------------------------------------------
function sg_save_configxml($filename)
{
    global $squidguard_config;

    file_put_contents($filename, dump_xml_config($squidguard_config, FLD_SQUIDGUARD));
}

# ------------------------------------------------------------------------------
# sg_reconfigure - squidguard reconfiguration
# ------------------------------------------------------------------------------
function sg_reconfigure()
{
    global $squidguard_config;
    $conf_file = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARD_CONFIGFILE;

    # 1. check system
    sg_check_system();

    # 2. reconfigure user db
    sg_reconfigure_user_db();

    # 3. generate squidGuard config
    $conf = sg_create_config();
    if ($conf) {
        if ($squidguard_config[FLD_WORKDIR])
            $conf_file = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_CONFIGFILE;
        file_put_contents($conf_file, $conf);
        file_put_contents('/usr/local/etc/squid' . SQUIDGUARD_CONFIGFILE, $conf); # << squidGuard want config '/usr/local/etc/squid' by default
        set_file_access($squidguard_config[FLD_WORKDIR], OWNER_NAME, 0755);
        sg_addlog("sg_reconfigure: save squidGuard config to '$conf_file'.", 1);
    } else
        sg_addlog("sg_reconfigure: error make squidGuard config.", 2);

    # 4. reconfigure squid
    squid_reconfigure();
}

# ------------------------------------------------------------------------------
# squid_reconfigure
# Insert in '/usr/local/squid/etc/squid.conf' options:
#     redirector_bypass on
#     redirect_program /usr/local/squidGuard/bin/squidGuard -c /path_to_config_file
#     redirect_children 1
# ------------------------------------------------------------------------------

function squid_reconfigure($remove_only = '')
{
    global $config;
    global $squidguard_config;
    $conf = '';
    $cust_opt = $config['installedpackages']['squid']['config'][0]['custom_options'];

    # remove old options
    if (!empty($cust_opt)) {
        $conf = explode(";", $cust_opt);
        foreach ($conf as $key => $c_opt) {
            $t_opt = ltrim($c_opt);
            if ((strpos($t_opt, REDIRECTOR_PROGRAM_OPT) === 0) or
                (strpos($t_opt, REDIRECT_BYPASS_OPT)    === 0) or
                (strpos($t_opt, REDIRECT_CHILDREN_OPT)  === 0))
                 unset($conf[$key]);
        }
        sg_addlog("squid_reconfigure: Remove old redirector options from Squid config.", 1);
    }

    # add new options - if squidGuard enabled
    if (empty($remove_only) && ($squidguard_config[FLD_ENABLED] === 'on')) {
        $redirector_path = $squidguard_config[FLD_BINPATH] . '/squidGuard';
        $redirector_conf = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_CONFIGFILE;

        $conf[] = REDIRECTOR_PROGRAM_OPT . " $redirector_path -c $redirector_conf";
        $conf[] = REDIRECT_BYPASS_OPT    . " on";
        $conf[] = REDIRECT_CHILDREN_OPT  . " " . REDIRECTOR_PROCESS_COUNT;

        sg_addlog("squid_reconfigure: Add new redirector options to Squid config.", 1);
    }

    # update config
    if (is_array($conf)) $conf = implode(";", $conf);

    $config['installedpackages']['squid']['config'][0]['custom_options'] = $conf;
    write_config('Update redirector options to squid config.');

    squid_resync();
}

# ------------------------------------------------------------------------------
# sg_check_system - check squidguard catalog's and access right's
# ------------------------------------------------------------------------------
function sg_check_system()
{
    global $squidguard_config;

    # check work_dir & create if not exists
    $work_dir = $squidguard_config[FLD_WORKDIR];
    if (!empty($work_dir)) {
        # check dir's
        if (!file_exists($work_dir)) {
            mwexec("mkdir -p $work_dir");
            set_file_access($work_dir, OWNER_NAME, 0755);
            sg_addlog("sg_check_system: Create work dir '$work_dir'.", 1);
        }
    }
    unset($work_dir);

    # check log_dir & create if not exists
    $log_dir = $squidguard_config[FLD_LOGDIR];
    if (!empty($log_dir)) {
        if (!file_exists($log_dir)) {
            mwexec("mkdir -p $log_dir");
            sg_addlog("sg_check_system: Create log dir '$log_dir'.", 1);
        }
        # set access right - need start any time;
        # (SG possible start from console and log file will have only root access)
        set_file_access($log_dir, OWNER_NAME, 0755);
    }
    unset($log_dir);

    # check db dir
    $db_dir = $squidguard_config[FLD_DBHOME];
    if (!empty($db_dir)) {
        if (!file_exists($db_dir)) {
            mwexec("mkdir -p $db_dir");
            sg_addlog("sg_check_system: Create db dir '$db_dir'.", 1);
        }
        # set access right
        set_file_access($db_dir, OWNER_NAME, 0755);
    }
    unset($db_dir);
}
# ==============================================================================
# squidGuard DB
# ==============================================================================
# sg_reconfigure_user_db - reconfigure(update) db user entries
# ------------------------------------------------------------------------------
function sg_reconfigure_user_db()
{
    global $squidguard_config;
    $dbhome = $squidguard_config[FLD_DBHOME];

    sg_addlog("sg_reconfigure_user_db: Begin with '$dbhome'", 1);

    # create user DB catalog, if not extsts
    if (!file_exists($dbhome)) {
        if (!mkdir($dbhome, 0755)) {
            sg_addlog("sg_reconfigure_user_db: Error create user DB directory '$dbhome'.", 2);
            return;
        }
        set_file_access($dbhome, OWNER_NAME, 0755);
        sg_addlog("sg_reconfigure_user_db: Create user DB directory '$dbhome'.", 1);
    }

    # update destinations to db
    $dests = $squidguard_config[FLD_DESTINATIONS];
    if(!empty($dests)){
        $dst_names = Array();
        $dst_list  = Array();

        sg_addlog("sg_reconfigure_user_db: Add user entries", 1);
        foreach($dests[FLD_ITEM] as $dst) {
            $path = "$dbhome/" . $dst[FLD_NAME];
            $dst_names[] = $path;
            $dst_list["usr_{$dst[FLD_NAME]}"] = $dst[FLD_NAME];

            # 1. check destination catalog and create them, if need
            if (!file_exists($path)) {
                if (!mkdir ($path, 0755)) {
                    sg_addlog("sg_reconfigure_user_db: Error create dir '$path'.", 2);
                    return;
                }
                sg_addlog("sg_reconfigure_user_db: Create dir '$path'.", 1);
            }

            # 2. build domains file
            $domains = $dst[FLD_DOMAINS];
            if (!empty($domains)) {
                $content = trim(str_replace(" ", "\n", $domains));
                file_put_contents($path . '/domains', $content);
                sg_addlog("sg_reconfigure_user_db: -- add {$dst[FLD_NAME]} domains '$domains';", 1);
            }
            unset($domains);

            # 3. build urls file
            $urls = $dst[FLD_URLS];
            if (!empty($urls)) {
                $content = trim(str_replace(" ", "\n", $urls));
                file_put_contents($path . '/urls', $content);
                    sg_addlog("sg_reconfigure_user_db: -- add {$dst[FLD_NAME]} urls '$content';", 1);
            }
            unset($urls);

            # 4. build expression file
            $expr = $dst[FLD_EXPRESSIONS];
            if (!empty($expr)) {
                $content = trim(str_replace("|", " ", $expr)); # delete first and last unnecessary '|' symbol
                $content = str_replace(" ", "|", $content);
                file_put_contents($path . '/expressions', $content);
                sg_addlog("sg_reconfigure_user_db: -- add {$dst[FLD_NAME]} expressions '$content';", 1);
            }
            unset($expr);
        }

        # 5. recursive set files access
        set_file_access($dbhome, OWNER_NAME, 0755);

        # 6. rebuild user db ('/var/db/squidGuard')
        sg_rebuild_db("_usrdb", $dbhome, $dst_list);
    } else
        sg_addlog("sg_reconfigure_user_db: Nothing. User destinations list empty.", 2);

    # 7. remove unused db entries
    sg_remove_unused_db_entries();
}

# ------------------------------------------------------------------------------
# sg_remove_unused_db_entries
# ------------------------------------------------------------------------------
function sg_remove_unused_db_entries()
{
    global $squidguard_config;
    $db_entries = array();
    $file_list = '';
    $dbhome  = $squidguard_config[FLD_DBHOME];
    $workdir = $squidguard_config[FLD_WORKDIR];

    # black list entries
    # * worked only with 'blacklist entries list file - else may be deleted black list entry
    if (file_exists($workdir .  SQUIDGUARD_BLK_ENTRIES)) {
        $file_for_del = array();

        # load blk entries
        $db_entries = explode("\n", file_get_contents($workdir . SQUIDGUARD_BLK_ENTRIES));

        # $db_entries + add user entries
        $dests = $squidguard_config[FLD_DESTINATIONS];
        if (!empty($dests)) {
            foreach($dests[FLD_ITEM] as $dst)
                $db_entries[] = $dst[FLD_NAME];
        }

        # diff between file list and entries list
        $file_list = scan_dir($dbhome);
        if (is_array($file_list) and is_array($db_entries)) {
            $file_for_del = array_diff($file_list, $db_entries);
        }

        # delete
        if (is_array($file_for_del) and !empty($file_for_del)) {
            foreach($file_for_del as $fd) {
                $file_fd = "$dbhome/$fd";
                if (!empty($fd) && ($fd != ".") && ($fd != "..")) {
                    if (file_exists($file_fd)) {
                        mwexec("rm -R $file_fd");
                        sg_addlog("sg_remove_unused_db_entries: Removed file '$file_fd'.", 1);
                    } else
                        sg_addlog("sg_remove_unused_db_entries: File'$file_fd' not found.", 2);
                }
            }
        }
    }
}
# ------------------------------------------------------------------------------
# sg_rebuild_db  Rebuild squidGuard DB from list items
# ------------------------------------------------------------------------------
# $shtag         - rebuild SH script TAG
# $rdb_dbhome    - DB directory  (default: '/var/db/squidGuard')
# $rdb_itemslist - items list as ['dest_key']='dest_DB_path'
#                  dest_DB_path - path without '$rdb_dbhome'
#                  example: ['ads_ban']='ads/banners' -> '/var/db/squidGuard/ads/banners'
# ------------------------------------------------------------------------------
function sg_rebuild_db($shtag, $rdb_dbhome, $rdb_itemslist)
{
    global $squidguard_config;
    $conf   = '';
    $conf_path = '';
    $logdir = $squidguard_config[FLD_LOGDIR];
    $dbhome = $squidguard_config[FLD_DBHOME];

    # current dbhome dir
    if (!empty($rdb_dbhome)) $dbhome = $rdb_dbhome;
    sg_addlog("sg_rebuild_db: Begin with path '$dbhome'.", 1);

    # define - where config will placed
    $conf_path = "/tmp/squidGuard_rebuild.conf" . $shtag;

    # make rebuild config; include all found dest items
    $conf = sg_create_simple_config($dbhome, $rdb_itemslist);
    file_put_contents($conf_path, $conf);
    set_file_access($conf_path, OWNER_NAME, 0750);
    sg_addlog("sg_rebuild_db: Create temporary config '$conf_path'.", 1);

    # *** SH script ***
    $sh_scr = Array();
    $sh_scr[] = "#!/bin/sh";
    $sh_scr[] = "cd $dbhome";
    $sh_scr[] = $squidguard_config[FLD_BINPATH] . "/squidGuard -c $conf_path -C all";
    $sh_scr[] = "wait"; # wait while SG rebuild DB

    # set DB owner and right access
    $sh_scr[] = "chown -R -v " . OWNER_NAME . " $dbhome";

    # restart squid for changes to take effects
    $sh_scr[] = "/usr/local/sbin/squid -k reconfigure";

    # store & exec sh
    $sh_scr = implode("\n", $sh_scr);
    $shfile =  DB_REBUILD_SH . $shtag;
    file_put_contents($shfile, $sh_scr);
    set_file_access($shfile, OWNER_NAME, 0750);
    # ! not background exec !
    mwexec($shfile);
    sg_addlog("sg_rebuild_db: Started SH script '$shfile'.", 1);
}

# ==============================================================================
# Log
# ------------------------------------------------------------------------------
# sg_addlog
# ------------------------------------------------------------------------------
function sg_addlog($log, $level = 0)
{
    global $squidguard_config;

    # log level
    if ($level < SQUIDGUARD_LOG_LEVEL) return;

    $logfile = '';
    $logfile = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARDCONF_LOGFILE;
    $log_content = array();

    setlocale(LC_TIME, '');
    $dt = date("d.m.Y H:i:s");

    # define logfile
    if (!empty($squidguard_config)) {
        if (file_exists($squidguard_config[FLD_LOGDIR]))
        $logfile = $squidguard_config[FLD_LOGDIR] . SQUIDGUARDCONF_LOGFILE;
    } else
        $log_content[] = "$dt : " . "sg_addlog: Error: squidguard_config is empty";

    $tmplog = '';
    if (file_exists($logfile))
        $tmplog = file_get_contents($logfile);
    $log_content = explode("\n", $tmplog);
    unset($tmplog);

    # shrink to MAXCOUNT log entries
    $log_content[] = "$dt : $log";
    if (count($log_content) > SQUIDGUARD_LOG_MAXCOUNT)
        array_splice($log_content, 0, SQUIDGUARD_LOG_MAXCOUNT - count($log_content));

    file_put_contents($logfile, implode("\n", $log_content));
}
# ------------------------------------------------------------------------------
# sg_getlog
# ------------------------------------------------------------------------------
function sg_getlog($last_entries_count)
{
    global $squidguard_config;
    $log_content = '';
    $logfile = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARDCONF_LOGFILE;

    # define logfile
    if (!empty($squidguard_config) && file_exists($squidguard_config[FLD_LOGDIR]))
        $logfile = $squidguard_config[FLD_LOGDIR] . SQUIDGUARDCONF_LOGFILE;

    # get log last 100 entries
    if (file_exists($logfile)) {
        $log_content = explode("\n", file_get_contents($logfile));
        if (count($log_content) > $last_entries_count)
            array_splice($log_content, 0, $last_entries_count - count($log_content));

        # insert log file name on top
        $log_content[0] = $logfile;
        $log_content = implode("\n", $log_content);
    }

    return $log_content;
}

# ==============================================================================
# make config
# ==============================================================================
# sg_create_config
# ------------------------------------------------------------------------------

function sg_create_config()
{
    global $squidguard_config;
    $sgconf = array();
    $sg_tag = new TSgTag;
    $error_res = '';
    $temp_str = '';

    if(!is_array($squidguard_config) || empty($squidguard_config)) {
        sg_addlog("sg_create_config: Error squidguard config data.", 2);
        return sg_create_simple_config('', '', SG_ERR0 . " (sg_create_config: [1]).");
    }

    # check configuration data
    if (!sg_check_config_data(&$error_res)) {
        sg_addlog("sg_create_config: Error config data. It's all error_res: \n$error_res", 2);
        sg_addlog("sg_create_config: Terminated.", 2);
        return sg_create_simple_config('', '', SG_ERR0 . " (sg_create_config: [2]).");
    }

    # --- Header ---
    $sgconf[] = CONFIG_SG_HEADER;
    $sgconf[] = "logdir {$squidguard_config[FLD_LOGDIR]}";
    $sgconf[] = "dbhome {$squidguard_config[FLD_DBHOME]}";

    # --- Times ---
    if ($squidguard_config[FLD_TIMES]) {
        $temp_str = '';
        foreach($squidguard_config[FLD_TIMES][FLD_ITEM] as $tm) {
            $sg_tag->clear();
            $sg_tag->set("time", $tm[FLD_NAME], "", $tm[FLD_DESCRIPTION]);

            foreach($tm[FLD_ITEM] as $itm) {
                $dts = ($itm[FLD_TIMETYPE] === "weekly") ? $itm[FLD_TIMEDAYS] : $itm[FLD_DATERANGE];
                $sg_tag->items[] = "{$itm[FLD_TIMETYPE]} $dts {$itm[FLD_TIMERANGE]}";
            }
            $sgconf[] = "";
            $sgconf[] = $sg_tag->tag_text();

            # log
            $temp_str .= " {$tm[FLD_NAME]}";
        }
        # log
        $temp_str = !empty($temp_str) ? $temp_str : "Nothing.";
        sg_addlog("sg_create_config: add times: \n $temp_str", 1);
    }

    # --- Sources ---
    if ($squidguard_config[FLD_SOURCES]) {
        $temp_str = '';
        foreach($squidguard_config[FLD_SOURCES][FLD_ITEM] as $src) {
            $sg_tag->clear();
            $sg_tag->set("src", $src[FLD_NAME], "", $src[FLD_DESCRIPTION]);

            # separate IP, domains, usernames
            $tsrc = explode(" ", trim($src[FLD_SOURCE]));
            foreach($tsrc as $sr) {
            	$sr = trim($sr);
            	if     (empty($sr))           continue;
            	if     (is_ipaddr_valid($sr)) $sg_tag->items[] = "ip     $sr";
            	elseif (is_domain_valid($sr)) $sg_tag->items[] = "domain $sr";
            	elseif (is_username($sr))     $sg_tag->items[] = "user   " . str_replace("'", "", $sr);
            }
            if ($src[FLD_LOG])      $sg_tag->items[] = "log {SQUIDGUARD_ACCESSBLOCK_FILE}";

            $sgconf[] = "";
            $sgconf[] = $sg_tag->tag_text();

            # log
            $temp_str .= " " . $src[FLD_NAME];
        }
        # log
        $temp_str = !empty($temp_str) ? $temp_str : "Nothing.";
        sg_addlog("sg_create_config: add sources: \n $temp_str", 1);
    }

    # --- Blacklist ---
    # Note! Blacklist must be added to config permanently. It's need for rebuild DB now

    $db_entries = sg_entries_blacklist();
    if (($squidguard_config[FLD_BLACKLISTENABLED] === 'on') and $db_entries) {
        $log_entr_added = '';
        $log_entr_ignored = '';
        sg_addlog("sg_create_config: add blacklist entries", 1);
        foreach($db_entries as $key => $ent) {
            $ent_state = array();
            $file_dms  = "{$squidguard_config[FLD_DBHOME]}/$ent/domains";
            $file_urls = "{$squidguard_config[FLD_DBHOME]}/$ent/urls";
            $file_expr = "{$squidguard_config[FLD_DBHOME]}/$ent/expressions";

            # check blacklist acl state
            if (file_exists($file_dms)) {
                $ent_state['exists'] = 'on';
                $ent_state[FLD_DOMAINS] = 'on';
            }
            if (file_exists($file_urls)) {
                $ent_state['exists'] = 'on';
                $ent_state[FLD_URLS] = 'on';
            }
            if (file_exists($file_expr)) {
                $ent_state['exists'] = 'on';
                $ent_state[FLD_EXPRESSIONS] = 'on';
            }

            # create config if blacklist item exists
            if ($ent_state['exists']) {
                $sg_tag->clear();
                $sg_tag->set("dest", $ent, "", "");

                if ($ent_state[FLD_DOMAINS])     $sg_tag->items[] = "domainlist $ent/domains";
                if ($ent_state[FLD_EXPRESSIONS]) $sg_tag->items[] = "expressionlist $ent/expressions";
                if ($ent_state[FLD_URLS])        $sg_tag->items[] = "urllist $ent/urls";
                $sg_tag->items[] = "log ". SQUIDGUARD_ACCESSBLOCK_FILE;

                $sgconf[] = "";
                $sgconf[] = $sg_tag->tag_text();

                # log
                $log_entr_added .= " $ent;";
            } else {
                $sgconf[] = "\t# Config ERROR: Destination '$ent' not found in DB";
                $log_entr_ignored .= " $ent;";
            }
        }

        # log 'added' and 'ignored'
        if (!empty($log_entr_added))   sg_addlog("sg_create_config: added: \n $log_entr_added \n", 1);
        if (!empty($log_entr_ignored)) sg_addlog("sg_create_config: ignored: \n $log_entr_ignored \n", 2);
    }

    # --- Destinations ---
    if ($squidguard_config[FLD_DESTINATIONS]) {
        $temp_str = '';
        foreach($squidguard_config[FLD_DESTINATIONS][FLD_ITEM] as $dst) {
            $dstname = $dst[FLD_NAME];
            $sg_tag->clear();
            $sg_tag->set("dest", $dst[FLD_NAME], "", $dst[FLD_DESCRIPTION]);

            if ($dst[FLD_DOMAINS])
                $sg_tag->items[] = "domainlist $dstname/domains";
            if ($dst[FLD_EXPRESSIONS])
                $sg_tag->items[] = "expressionlist $dstname/expressions";
            if ($dst[FLD_URLS])
                $sg_tag->items[] = "urllist $dstname/urls";
            if ($dst[FLD_RMOD] != RMOD_NONE)
                $sg_tag->items[] = "redirect " . sg_redirector_base_url($dst[FLD_REDIRECT], $dst[FLD_RMOD]);
            if ($dst[FLD_LOG])
                $sg_tag->items[] = "log {SQUIDGUARD_ACCESSBLOCK_FILE}";

            $sgconf[] = "";
            $sgconf[] = $sg_tag->tag_text();

            # log
            $temp_str .= " $dstname;";
        }
        # log
        $temp_str = !empty($temp_str) ? $temp_str : "Nothing.";
        sg_addlog("sg_create_config: add destinations: \n $temp_str", 1);
    }

    # --- Rewrites ---
    if ($squidguard_config[FLD_REWRITES]) {
        $temp_str = '';
        $log_entr_added = '';
        $log_entr_err = '';
        foreach($squidguard_config[FLD_REWRITES][FLD_ITEM] as $rew) {
            $sg_tag->clear();
            $sg_tag->set("rew", $rew[FLD_NAME], "", "");

            if (is_array($rew[FLD_ITEM])) {
                foreach ($rew[FLD_ITEM] as $rw)
                    $sg_tag->items[] = "s@.{$rw[FLD_TARGETURL]}@{$rw[FLD_REPLACETO]}@r";

                if ($rew[FLD_LOG])
                    $sg_tag->items[] = "log {SQUIDGUARD_ACCESSBLOCK_FILE}";

                $sgconf[] = "";
                $sgconf[] = $sg_tag->tag_text();
                # log
                $log_entr_added .= " {$rew[FLD_NAME]};";
            }
            else {
                $sgconf[] = "";
                $sgconf[] = "# Rewrite {$rew[FLD_NAME]} error.";
                # log
                $log_entr_err .= " {$rew[FLD_NAME]};";
            }
        }

        # log
        sg_addlog("sg_create_config: add rewrites: \n success $log_entr_added \n error $log_entr_err", 1);
    }

    # ----------------------------------------
    $entry_blacklist = sg_entries_blacklist();

    # --- ACL ---
    $sg_tag->clear();
    $sg_tag->set("acl", "", "", "");
    if ($squidguard_config[FLD_ACLS]) {
        $temp_str = '';
        $log_entr_added = '';
        foreach($squidguard_config[FLD_ACLS][FLD_ITEM] as $acl) {
            if (!$acl[FLD_DISABLED]) {
                $sg_acltag = new TSgTag;
                $sg_acltag->set($acl[FLD_NAME], "", $acl[FLD_TIMENAME], $acl[FLD_DESCRIPTION]);

                # delete blacklist entries from 'pass' if blacklist disabled
                if ($squidguard_config[FLD_BLACKLISTENABLED] !== 'on') {
                    acl_remove_blacklist_items(&$acl[FLD_DESTINATIONNAME]);
                    acl_remove_blacklist_items(&$acl[FLD_OVERDESTINATIONNAME]);
                }

                # not allowing IP in URL
                if ($acl[FLD_NOTALLOWINGIP]) {
                    $acl[FLD_DESTINATIONNAME]     = "!in-addr {$acl[FLD_DESTINATIONNAME]}";
                    $acl[FLD_OVERDESTINATIONNAME] = "!in-addr {$acl[FLD_OVERDESTINATIONNAME]}";
                }

                # re-order acl pass (<allow><deny<all|none>)
                $acl[FLD_DESTINATIONNAME]     = sg_aclpass_reorder($acl[FLD_DESTINATIONNAME]);
                $acl[FLD_OVERDESTINATIONNAME] = sg_aclpass_reorder($acl[FLD_OVERDESTINATIONNAME]);

                # ontime
                $sg_acltag->items[] = "pass {$acl[FLD_DESTINATIONNAME]}";
                if ($acl[FLD_RMOD] != RMOD_NONE)
                $sg_acltag->items[] = "redirect " . sg_redirector_base_url($acl[FLD_REDIRECT], $acl[FLD_RMOD]);
                if ($acl[FLD_REWRITENAME])
                    $sg_acltag->items[] = "rewrite {$acl[FLD_REWRITENAME]}";
                if ($acl[FLD_LOG])
                    $sg_acltag->items[] = "log {SQUIDGUARD_ACCESSBLOCK_FILE}";

                # overtime
                if ($acl[FLD_TIMENAME]) {
                    $sg_acltag->items[] = "} else {";
                    $sg_acltag->items[] = "pass {$acl[FLD_OVERDESTINATIONNAME]}";
                    if ($acl[FLD_REDIRECMODE] !== RMOD_NONE)
                        $sg_acltag->items[] = "redirect " . sg_redirector_base_url($acl[FLD_OVERREDIRECT], $acl[FLD_RMOD]);
                    if ($acl[FLD_OVERREWRITENAME])
                        $sg_acltag->items[] = "rewrite {$acl[FLD_OVERREWRITENAME]}";
                    if ($acl[FLD_LOG])
                        $sg_acltag->items[] = "log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                }
                $sg_tag->items[] = $sg_acltag;
            }
            $log_entr_added .= " {$acl[FLD_NAME]};";
        }
        # log
        $log_entr_added = !empty($log_entr_added) ? $log_entr_added : "Nothing.";
        sg_addlog("sg_create_config: add ACL's: \n $log_entr_added", 1);
    }

    # --- Default ---
    $sg_tag_def = new TSgTag;
    $sg_tag_def->set("default", "", "", "");
    $def = $squidguard_config[FLD_DEFAULT];
    sg_addlog("sg_create_config: add Default", 1);
    if ($def) {
        $temp_str = '';

        # delete blacklist entries from 'pass' if blacklist disabled
        if ($squidguard_config[FLD_BLACKLISTENABLED] !== 'on')
            acl_remove_blacklist_items(&$def[FLD_DESTINATIONNAME]);

        # not allowing IP in URL
        if ($def[FLD_NOTALLOWINGIP])
            $def[FLD_DESTINATIONNAME] = "!in-addr " . $def[FLD_DESTINATIONNAME];

        # re-order acl pass (<allow><deny<all|none>)
        $def[FLD_DESTINATIONNAME] = sg_aclpass_reorder($def[FLD_DESTINATIONNAME]);

        # ! 'Default' must use without times !
        $sg_tag_def->items[] = "pass {$def[FLD_DESTINATIONNAME]}";
        if ($def[FLD_RMOD] !== RMOD_NONE)
            $sg_tag_def->items[] = "redirect " .  sg_redirector_base_url($def[FLD_REDIRECT], $def[FLD_RMOD]);
        if ($def[FLD_LOG])
            $sg_tag_def->items[] = "log {SQUIDGUARD_ACCESSBLOCK_FILE}";

    } # <- if def
    else {
        $msg =  "Error: ACL 'default' is empty, will use default 'block all'";
        $sg_tag_def->items[] = "# $msg";
        $sg_tag_def->items[] = "pass none";
        $sg_tag_def->items[] = "redirect " . sg_redirector_base_url('', RMOD_INT_ERRORPAGE);
        sg_addlog("sg_create_config: $msg.", 2);
    }

    # --- ACL end ---
    $sg_tag->items[] = $sg_tag_def; # add 'default' ACL object
    $sgconf[] = "";
    $sgconf[] = $sg_tag->tag_text();

    # delete "\n" chars before each string - SG bug (first string of config must be not empty)
    foreach ($sgconf as $key => $val) $sgconf[$key] = ltrim($sgconf[$key], "\n");
    return implode("\n", $sgconf);
}

# ------------------------------------------------------------------------------
# sg_create_simple_config
#     Create config for DB rebuilding
#     Default rule - block all
#     Variables:
#         $blk_dbhome   - temporary DB home dir, may be different with DBHOME
#         $blk_destlist - is array as [dst_name] = 'path',
#                         where path - catalog without dbhome path
#                         For example: dbhome is '/var/db/squidGuard/',
#                         path can be 'usr/ads' or 'bl/poxy'
#         $redirect_to  - redirector string
# ------------------------------------------------------------------------------
function sg_create_simple_config($blk_dbhome, $blk_destlist, $redirect_to = "404")
{
    global $squidguard_config;
    $sgconf = array();
    $logdir = $squidguard_config[FLD_LOGDIR];
    $dbhome = $squidguard_config[FLD_DBHOME];

    # current dbhome dir
    if (!empty($blk_dbhome)) $dbhome = $blk_dbhome;
    sg_addlog("sg_create_simple_config: Begin with dbhome='$dbhome'.", 1);

    # header
    $sgconf[] = CONFIG_SG_HEADER;

    # init section
    $sgconf[] = "logdir $logdir";
    $sgconf[] = "dbhome $dbhome";
    $sgconf[] = "";

    # destination section
    if (is_array($blk_destlist)) {
        foreach($blk_destlist as $dst => $dpath) {
            $tmp_s = array();

            # check item elements
            if (file_exists("$dbhome/$dpath/domains"))     $tmp_s[] = "\t domainlist $dpath/domains";
            if (file_exists("$dbhome/$dpath/urls"))        $tmp_s[] = "\t urllist $dpath/urls";
            if (file_exists("$dbhome/$dpath/expressions")) $tmp_s[] = "\t expressionlist $dpath/expressions";

            # create only valid items
            if (!empty($tmp_s)) {
                $tmp_s = implode("\n", $tmp_s);
                $sgconf[] = "dest $dst {\n $tmp_s \n}\n";
                sg_addlog("sg_create_simple_config: -- added item '$dst' = '$dbhome/$dpath'.");
            } else
                sg_addlog("sg_create_simple_config: -- ignored empty item '$dst' = '$dbhome/$dpath'.");
        }
    }

    # acl section
    $sgconf[] = "acl {\n\t default {\n\t\t pass all ";
    $sgconf[] = "\t\t redirect " . sg_redirector_base_url($redirect_to, RMOD_INT_ERRORPAGE); # use sgerror only!
    $sgconf[] = "\t } \n}";

    # delete "\n" chars before each string - SG bug (first string of config must be not empty)
    foreach ($sgconf as $key => $val) $sgconf[$key] = ltrim($sgconf[$key], "\n");

    return implode("\n", $sgconf);
}

# -------------------------------------------------------------------------------------------------
# sg_redirector_base_url
#   $url - url where redirect to
#   $use_internal - ignore 'Redirect mode' option, use internal (for rebuild config, for example)
#
# -------------------------------------------------------------------------------------------------
function sg_redirector_base_url($rdr_info, $redirect_mode)
{
    global $squidguard_config;
    $rdr_path = '';

    # gui port, ip & proto
    $guiip = (!empty($squidguard_config[FLD_CURRENT_LAN_IP])) ? $squidguard_config[FLD_CURRENT_LAN_IP] : '127.0.0.1';
    $guiport = (!empty($squidguard_config[FLD_CURRENT_GUI_PORT])) ? $squidguard_config[FLD_CURRENT_GUI_PORT] : '80';
    $guiproto = ($squidguard_config[FLD_CURRENT_GUI_PROTO] === "https") ? "https" : "http";
    $rdr_path = "$guiproto://$guiip:$guiport" . REDIRECT_BASE_URL;

    # check redirect
    $errmsg = '';
    if (!sg_check_redirect($redirect_mode, $rdr_info, &$errmsg)) {
        $redirect_mode = RMOD_INT_ERRORPAGE;
        $rdr_info = "Redirect settings error. $errmsg Check you configuration.";
        sg_addlog("sg_redirector_base_url: ERROR: $errmsg");
    }

    switch($redirect_mode) {
        case RMOD_EXT_ERR:         $rdr_path = "$rdr_info" . REDIRECT_URL_ARGS; break;
        case RMOD_EXT_RDR:         $rdr_path = "$rdr_info"; break;
        case RMOD_EXT_MOVED:       $rdr_path = "301:$rdr_info"; break;
        case RMOD_EXT_FOUND:       $rdr_path = "302:$rdr_info"; break;
        case RMOD_INT_BLANKPAGE:   $rdr_path .= "?url=blank"; break;
        case RMOD_INT_BLANKIMG:    $rdr_path .= "?url=blank_img"; break;
        case RMOD_INT_SIZELIMIT:   $rdr_path .= "?url=maxlen_$rdr_info" . REDIRECT_URL_ARGS; break;
        case RMOD_INT_ERRORPAGE:
        default:                   $rdr_path .= "?url=" . rawurlencode("403 $rdr_info") . REDIRECT_URL_ARGS; break;
    }

    sg_addlog("sg_redirector_base_url: Select redirector base url ($rdr_path)");
    return $rdr_path;
}

# -------------------------------------------------------------------------------------------------
# sg_aclpass_reorder
# -------------------------------------------------------------------------------------------------
function sg_aclpass_reorder($pass)
{
    $ar_pass = explode(" ", $pass);

    # 'pass' order: <allow> <deny> <all|none>
    if (is_array($ar_pass)) {
        $pass_end = '';
        $pass_fst = array(); # whitelist - '^' prefix (will deleted)
        $pass_sec = array(); # blacklist - '!' prefix
        $pass_lst = array(); # allow
        foreach ($ar_pass as $val) {
            $tk = trim($val);
            if ($tk === 'all' or $tk === 'none')
                $pass_end   = $val;
            elseif (strpos($tk, "^") !== false)
                # delete '^' prefix
                $pass_fst[] = str_replace('^', '', $val);
            elseif (strpos($tk, "!") !== false)
                $pass_sec[] = $val;
            else
                    $pass_lst[] = $val;
        }
        $ar_pass = array_merge($pass_fst, $pass_sec, $pass_lst);
        $ar_pass[] = $pass_end;
    }
    return implode(" ", $ar_pass);
}

# ------------------------------------------------------------
# sg_check_config_data
# ------------------------------------------------------------
function sg_check_config_data ($input_errors)
{
    global $squidguard_config;
    $elog = array();
    $times        = sg_list_itemsfield($squidguard_config[FLD_TIMES], FLD_NAME);
    $sources      = sg_list_itemsfield($squidguard_config[FLD_SOURCES], FLD_NAME);
    $destinations = sg_list_itemsfield($squidguard_config[FLD_DESTINATIONS], FLD_NAME);
    $rewrites     = sg_list_itemsfield($squidguard_config[FLD_REWRITES], FLD_NAME);
    $acls = array();

    # --- Times ---
    if ($squidguard_config[FLD_TIMES]) {
        $key_tm = array_count_values($times);
        foreach($squidguard_config[FLD_TIMES][FLD_ITEM] as $tm) {
            # check name as unique and name format
            $tm_name = $tm[FLD_NAME];
            $err_s = '';
            if (!check_name_format($tm_name, &$err_s))
                $elog[] = "(T1) TIME '$tm_name' error: >>> $err_s";

            if ($key_tm[$tm_name] > 1)
                $elog[] = "(T2) TIME '$tm_name' error: duplicate time name '$tm_name'";

            # check time items format
            sg_check_time($tm, &$elog);
        }
    }

    # --- Sources ---
    if ($squidguard_config[FLD_SOURCES]) {
        $key_src = array_count_values($sources);
        foreach($squidguard_config[FLD_SOURCES][FLD_ITEM] as $src) {
            # check name as unique and name format
            $src_name = $src[FLD_NAME];
            $err_s = '';
            if (!check_name_format($src_name, &$err_s))
                $elog[] = "(A1) ACL '$src_name'error: $err_s";

            if ($key_src[$src_name] > 1)
                $elog[] = "(A2) ACL '$src_name' error: duplicate source name '$src_name'";

            sg_check_src($src, $elog);
        }
    }

    # --- Destinations ---
    if ($squidguard_config[FLD_DESTINATIONS]) {
        $key_dst = array_count_values($destinations);
        foreach($squidguard_config[FLD_DESTINATIONS][FLD_ITEM] as $dst) {
            # check name as unique and name format
            $dst_name = $dst[FLD_NAME];
            $err_s = '';
            if (!check_name_format($dst_name, &$err_s))
                $elog[] = "(D1) DEST '$dst_name' error: $err_s";

            if ($key_dst[$dst_name] > 1)
                $elog[] = "(D2) DEST '$dst_name' error: duplicate destination name '$dst_name'";
            #
            sg_check_dest($dst, &$elog);
        }
    }

    # --- Blacklist ---
    $blk_entries_file = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_BLK_ENTRIES;
    if (file_exists($blk_entries_file)) {
        $blk_entr = explode("\n", file_get_contents($blk_entries_file));
        foreach($blk_entr as $entr) {
            if ($entr) {
                $destinations[] = $entr;
                # check entry for exists
                $dbfile = $squidguard_config[FLD_DBHOME] . "/$entr";
                if (!file_exists($dbfile))
                    $elog[] = "(B1) BLACKLIST '$entr' error: file '$dbfile' not found";
            }
        }
    }

    # --- Rewrites ---
    if ($squidguard_config[FLD_REWRITES]) {
        $key_rw = array_count_values($rewrites);
        foreach($squidguard_config[FLD_REWRITES][FLD_ITEM] as $rw) {
            # check check name as unique and name format
            $rw_name = $rw[FLD_NAME];
            $err_s = '';
            if (!check_name_format($rw_name, &$err_s))
                $elog[] = "(R1) REWRITE '$rw_name' error: $err_s";

            if ($key_rw[$rw_name] > 1)
                $elog[] = "(R2) REWRITE '$rw_name' error: duplicate rewrite name '$rw_name'";
        }
    }

    $key_times = array_count_values($times);
    $key_sources = array_count_values($sources);
    $key_destinations = array_count_values($destinations);
    $key_rewrites = array_count_values($rewrites);

    # --- ACLs ---
    if ($squidguard_config[FLD_ACLS]) {
        $acls = array();
        foreach($squidguard_config[FLD_ACLS][FLD_ITEM] as $acl) {
            # skip disabled acl
            if ($acls[FLD_DISABLED]) continue;

            $acl_name = $acl[FLD_NAME];

            # check acl name for unique and exists (as source items)
            if ($acl_name and !$key_sources[$acl_name])
                $elog[] = "(A1) ACL '$acl_name' error: acl name '$acl_name' not found";

            $acls[] = $acl_name;
            $key_acls = array_count_values($acls);
            if ($key_acls[$acl_name] > 1)
                $elog[] = "(A2) ACL '$acl_name' error: duplicate acl name '$acl_name'";

            # check time
            $time = $acl[FLD_TIMENAME];
            if ($time and !$key_times[$time]) # time name must exists
                $elog[] = "(A3) ACL '$acl_name' error: time name '$time' not found";

            # check destinations
            if ($acl[FLD_DESTINATIONNAME]) {
                $acldest = $acl[FLD_DESTINATIONNAME];
                $acldest = str_replace("!", "", $acldest);
                $acldest = str_replace("^", "", $acldest);
                $acldest = explode(" ", $acldest);
                $key_acldest = array_count_values($acldest);
                foreach($acldest as $adest) {
                    # check duplicates destinations in acl
                    if ($key_acldest[$adest] > 1)
                        $elog[] = "(A4) ACL '$acl_name' error: duplicate destination name '$adest'. Any destination must included once.";
                    # check destinations for exists
                    if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                        $elog[] = "(A5) ACL '$acl_name' error: destination name '$adest' not found";
                }
            } else {
                $elog[] = "(A6) ACL '$acl_name' error: ontime pass list is empty. Added 'none'.";
                $acl[FLD_DESTINATIONNAME] = "none";
            }

            # check overtime destinations
            if ($time) {
                if ($acl[FLD_OVERDESTINATIONNAME]) {
                    $acloverdest = $acl[FLD_OVERDESTINATIONNAME];
                    $acloverdest = str_replace("!", "", $acloverdest);
                    $acloverdest = str_replace("^", "", $acloverdest);
                    $acloverdest = explode(" ", $acloverdest);
                    $key_acloverdest = array_count_values($acloverdest);
                    foreach($acloverdest as $adest) {
                        # check duplicates destinations in acl
                        if ($key_acloverdest[$adest] > 1)
                            $elog[] = "(A7) ACL '$acl_name' error: duplicate overtime destination name '$adest'. Any destination must included once.";
                        # check destinations for exists
                        if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                            $elog[] = "(A8) ACL '$acl_name' error: overtime destination name '$adest' not found";
                    }
                } else {
                    $elog[] = "(A9) ACL '$acl_name' error: overtime pass list is empty. Added 'none'.";
                    $acl[FLD_OVERDESTINATIONNAME] = "none";
                }
            }

            # check rewrite
            $rew = $acl[FLD_REWRITENAME];
            if ($rew and !$key_rewrites[$rew])
                $elog[] = "(AA) ACL '$acl_name' error: rewrite name '$rew' not found";

            # check overtime rewrite
            $overrew = $acl[FLD_OVERREWRITENAME];
            if ($time and $overrew and !$key_rewrites[$overrew])
                $elog[] = "(AB) ACL '$acl_name' error: overtime rewrite name '$overrew' not found";

            # check redirect
            $redir = $acl[FLD_REDIRECT];
            $overredir = $acl[FLD_OVERREDIRECT];
        }
    }

    # --- Default ---
    if ($squidguard_config[FLD_ACLS]) {
        $def = $squidguard_config[FLD_DEFAULT];

        # check time
        $time = $def[FLD_TIMENAME];
        if ($time and !$key_times[$time]) # time name must exists
            $elog[] = "(DF1) ACL 'default' error: time name '$time' not found";

        # check destinations
        if ($def[FLD_DESTINATIONNAME]) {
            $defdest = $def[FLD_DESTINATIONNAME];
            $defdest = str_replace("!", "", $defdest);
            $defdest = str_replace("^", "", $defdest);
            $defdest = explode(" ", $defdest);
            $key_defdest = array_count_values($defdest);
            foreach($defdest as $adest) {
                # check duplicates destinations in acl
                if ($key_defdest[$adest] > 1)
                    $elog[] = "(DF2) ACL 'default' error: duplicate destination name '$adest'. Any destination must included once.";
                # check destinations for exists
                if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                    $elog[] = "(DF3) ACL 'default' error: destination name '$adest' not found";
            }
        } else {
            $elog[] = "(DF4) ACL 'default' error: ontime pass list is empty. Added 'none'.";
            $def[FLD_DESTINATIONNAME] = "none";
        }

        # check rewrite
        $rew = $def[FLD_REWRITENAME];
        if ($rew and !$key_rewrites[$rew])
            $elog[] = "(DF5) ACL 'default' error: rewrite name '$rew' not found";

        # check overtime rewrite
        $overrew = $def[FLD_OVERREWRITENAME];
        if ($time and $overrew and !$key_rewrites[$overrew])
            $elog[] = "(DF6) ACL 'default' error: overtime rewrite name '$overrew' not found";

        # check redirect
        $redir = $def[FLD_REDIRECT];
        $overredir = $def[FLD_OVERREDIRECT];
    }

    # update log
    if (!empty($elog)) {
    	$input_errors = (is_array($input_errors)) ? array_merge($input_errors, $elog) : implode("\n", $elog);
    }

    return empty($elog);
}

# =============================================================================
# Blacklist
# =============================================================================
# sg_reconfigure_blacklist($source_filename, $opt)
#     $source_filename - file name or url
#     $opt - option:
#         '' or 'local'   - update from local file
#         'url'           - update from url
# -----------------------------------------------------------------------------
function sg_reconfigure_blacklist($source_filename, $opt = '')
{
    global $squidguard_config;
    $sf = trim($source_filename);
    $sf_contents = '';

    sg_addlog("sg_reconfigure_blacklist: Begin with '$sf'.", 1);

    # 1. check system
    sg_check_system();

    # 2. upload
    if ($sf[0] === "/") { # local file - example '/tmp/blacklists.tar'
        sg_addlog("sg_reconfigure_blacklist: Update from file '$sf'.", 1);
        if (file_exists($sf)) {
            $sf_contents = file_get_contents($sf);
        } else {
            sg_addlog("sg_reconfigure_blacklist: Error, file '$sf' not found.", 2);
            return;
        }
    }
    # from url
    else {
        sg_addlog("sg_reconfigure_blacklist: Upload from url '$sf'.", 1);
        $sf_contents = sg_uploadfile_from_url($sf, BLK_LOCALFILE, $opt);
    }

    # 3. update
    if (empty($sf_contents)) {
        sg_addlog("sg_reconfigure_blacklist: Error content from '$sf'.", 2);
        return;
    }
    # save black list archive content to local file
    file_put_contents(BLK_LOCALFILE, $sf_contents);

    # 4. update blacklist
    sg_update_blacklist(BLK_LOCALFILE);
}

# ------------------------------------------------------------------------------
# sg_update_blacklist - update blacklist from file
# How it's work:
#     - unpack tar archive to temp dir
#     - copy subdir's tree to one-level temp DB
#     - copy unrebuilded temp db to work db (for user's can configure with new Blacklist)
#     - create Blacklist files listing and copy to values dir and temp DB dir
#     - background rebuild temp DB via sh script (longer proccess) and copy to work DB
# ------------------------------------------------------------------------------

function sg_update_blacklist($from_file)
{
    global $squidguard_config;
    $dbhome  = SQUIDGUARD_DBHOME_DEF;
    $workdir = SQUIDGUARD_WORKDIR_DEF;
    $tmp_unpack_dir = SQUIDGUARD_TMP . SQUIDGUARD_BL_UNPACK;
    $arc_db_dir     = SQUIDGUARD_VAR . SG_BLK_ARC;

    sg_addlog("sg_update_blacklist: Begin with '$from_file'.", 1);

    if (file_exists($from_file)) {
        # check work and DB dir's
        if (file_exists($squidguard_config[FLD_DBHOME]))  $dbhome = $squidguard_config[FLD_DBHOME];
        if (file_exists($squidguard_config[FLD_WORKDIR])) $workdir = $squidguard_config[FLD_WORKDIR];
        # delete old tmp dir's
        if (file_exists($tmp_unpack_dir)) mwexec("rm -R  . $tmp_unpack_dir");
        if (file_exists($arc_db_dir))     mwexec("rm -R  . $arc_db_dir");
        # create new tmp/arc dir's
        mwexec("mkdir -p -m 0755 $tmp_unpack_dir");
        mwexec("mkdir -p -m 0755 $arc_db_dir");

        # 1. unpack archive
        mwexec("tar zxvf $from_file -C $tmp_unpack_dir");
        set_file_access($tmp_unpack_dir, OWNER_NAME, 0755);
        sg_addlog("sg_update_blacklist: Unpack uploaded file '$from_file' -> '$tmp_unpack_dir'.", 1);

        # 2. copy blacklist to squidGuard base & create entries list
        if (file_exists($tmp_unpack_dir)) {
            $blk_items = array();
            $blk_list  = array();

            # scan blacklist items
            scan_blacklist_cat($tmp_unpack_dir, "blk", & $blk_items);

            # move blacklist catalog structure to 'one level' (from tmp_DB to arch_DB)
            foreach ($blk_items as $key => $val) {
                $current_dbpath = "$arc_db_dir/$key";
                if (count($val)) {
                # make blk_list for config file
                $blk_list[$key] = $key;

                # delete '$current_dbpath' for correct moving
                # need moving $val['path'] to $current_dbpath
                # if $current_dbpath exists,
                # then $val['path'] will created as subdir - !it's worng!
                if (file_exists($current_dbpath))
                    mwexec("rm -R $current_dbpath");
                mwexec("mv -f {$val['path']}/ $current_dbpath");
                sg_addlog("sg_update_blacklist: Move {$val['path']}/ -> $current_dbpath.", 1);
            }
            }
            set_file_access($arc_db_dir, OWNER_NAME, 0755);

            # -- DISABLED -- copy unrebuilded blacklist from arch_DB_to work DB & set access rights
#           mwexec("cp -R $arc_db_dir/ $dbhome");
#           set_file_access($dbhome, OWNER_NAME, 0755);

            # create entries list
            if (count($blk_items)) {
                # save to temp DB
                $blklist_file = SQUIDGUARD_VAR . SQUIDGUARD_BLK_ENTRIES;
                file_put_contents($blklist_file, implode("\n", array_keys($blk_items)));
                set_file_access  ($blklist_file, OWNER_NAME, 0755);

                # -- DISABLED -- save copy to squidGuard config dir
#               $blklist_file = "{$squidguard_config[FLD_WORKDIR]}/" . SQUIDGUARD_BLK_ENTRIES;
#               file_put_contents($blklist_file, implode("\n", array_keys($blk_items)));
#               set_file_access  ($blklist_file, OWNER_NAME, 0755);
                sg_addlog("sg_update_blacklist: Create DB entries list '$blklist_file'.", 1);
            }

            # make rebuild config (included all found dest items) & save to work dir
            $conf_path = SQUIDGUARD_VAR . DB_REBUILD_BLK_CONF;   # "/tmp/squidGuard_rebuild_blk.conf";
            file_put_contents($conf_path, sg_create_simple_config($arc_db_dir, $blk_list));
            set_file_access($conf_path, OWNER_NAME, 0755);
            sg_addlog("sg_update_blacklist: Create rebuild config '$conf_path'.", 1);

            # *** SH script ***********************************************
            $sh_scr = Array();
            $sh_scr[] = "#!/bin/sh";
            $sh_scr[] = "cd $arc_db_dir";
            $sh_scr[] = $squidguard_config[FLD_BINPATH] . "/squidGuard -c $conf_path -C all";
            $sh_scr[] = "wait"; # wait while SG rebuild DB
            $sh_scr[] = "chown -R -v " . OWNER_NAME . " $arc_db_dir";
            $sh_scr[] = "chmod -R -v 0755 $arc_db_dir";

            # copy temp db to '/var/db/squidGuard (-R - recursive; -p - copy access rights)
            # '$bl_temp_dbhome/' - slash in end of path - copy only dir content (not self dir)
            $sh_scr[] = "cp -R -p $arc_db_dir/ $dbhome";
            $sh_scr[] = "cp -f -p $blklist_file " . SQUIDGUARD_WORKDIR_DEF;
            # set DB owner and right access
            $sh_scr[] = "chown -R -v " . OWNER_NAME . " $dbhome";
            $sh_scr[] = "chmod -R -v 0755 $dbhome";

            # if new blacklist some as already installed, then restart squid for changes to take effects
            $blk_items_old = '';
            $blk_file_old = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_BLK_ENTRIES;
            if (file_exists($blk_items_old))
                    $blk_items_old = file_get_contents($blk_file_old);
            if (!empty($blk_items_old) && ($blk_items_old === implode("\n", array_keys($blk_items)))) {
                    $sh_scr[] = "/usr/local/sbin/squid -k reconfigure";
                $sh_scr[] = "wait"; # wait while process
            }
            unset($blk_file_old);
            unset($blk_items_old);

            # store & exec sh
            $sh_scr = implode("\n", $sh_scr);
            $shfile =  DB_REBUILD_SH . "_blk";
            file_put_contents($shfile, $sh_scr);
            set_file_access($shfile, OWNER_NAME, 0755); # 0755 - script will execute
            # kill exists rebuild processes
#           mwexec("kill `ps auxw | grep \"$shfile\" | grep -v \"grep\" | awk '{print $2}'`");                # sh script
            mwexec("kill `ps auxw | grep \"squidGuard_blk_rebuild\" | grep -v \"grep\" | awk '{print $2}'`"); # squidGuard process
            mwexec_bg("nice -n 5 $shfile");
            sg_addlog("sg_update_blacklist: Started sh script '$shfile'.", 1);

            # clearing temp
#           mwexec("rm -R $bl_temp");
         }
    }
}


# -----------------------------------------------------------------------------
# sg_blacklist_rebuild_DB  - update blacklist from file
# -----------------------------------------------------------------------------
function sg_entries_blacklist()
{
    global $squidguard_config;
    $contents = '';

    $fl = SQUIDGUARD_WORKDIR_DEF . SQUIDGUARD_BLK_ENTRIES;
    if (file_exists($squidguard_config[FLD_WORKDIR]))
        $fl = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_BLK_ENTRIES;
    if (file_exists($fl))
        $contents = explode("\n", file_get_contents($fl));

    return $contents;
}
# -----------------------------------------------------------------------------
# sg_blacklist_rebuild_db - rebuild current Blacklist DB (default: '/var/db/squidGuard')
# -----------------------------------------------------------------------------
function sg_blacklist_rebuild_db()
{
    global $squidguard_config;
    $dst_list = array();
    $dbhome  = $squidguard_config[FLD_DBHOME];
    $workdir = $squidguard_config[FLD_WORKDIR];

    # current dbhome and work dir's
    sg_addlog("sg_blacklist_rebuild_db: Start with path '$dbhome'.", 1);

    # make dest list
    $blklist_file = "$workdir/" . SQUIDGUARD_BLK_ENTRIES;
    if (file_exists($blklist_file)) {
        $blklist = explode("\n", file_get_contents($blklist_file));
        if (is_array($blklist))
            foreach($blklist as $bl) { $dst_list[$bl] = $bl; }
    }

    # rebuild user db ('/var/db/squidGuard')
    sg_rebuild_db("_blkdb", $dbhome, $dst_list);
}

# ========================== UTILS =============================================
# sg_uploadfile_from_url
# upload file and put them to $destination_file
# return = upload content
# ------------------------------------------------------------------------------
function sg_uploadfile_from_url($url_file, $destination_file, $proxy = '')
{
    # open destination file
    sg_addlog("sg_uploadfile_from_url: begin url'$url_file' proxy'$proxy'", 1);

    $result = '';
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url_file);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    if (!empty($proxy)) {
        $ip    = '';
        $login = '';
        $s     = trim($proxy);
        if (strpos($s, ' ')) {
            $ip    = substr($s, 0, strpos($s, ' '));
            $login = substr($s, strpos($s, ' ') + 1);
        } else      $ip    = $s;

        if($ip != '') {
            curl_setopt($ch, CURLOPT_PROXY, $ip);
            if($login != '')
                curl_setopt($ch, CURLOPT_PROXYUSERPWD, $login);
        }
    }
    $result=curl_exec ($ch);
    curl_close ($ch);
    if (!empty($destination_file))
        file_put_contents($destination_file, $result);
    else sg_addlog("sg_uploadfile_from_url: Error upload file", 2);

    # for test
    file_put_contents(BLK_LOCALFILE, $result);

    return $result;
}

# ==============================================================================
# self utils
# ==============================================================================
# Set file access
# ------------------------------------------------------------------------------
function set_file_access($dir, $owner, $mod)
{
    $mod = sprintf("%o", $mod);
    if (!file_exists($dir)) return;
    # recursive change access
    mwexec("chown -R -v $owner $dir");
    mwexec("chgrp -R -v $owner $dir");
    mwexec("chmod -R -v $mod $dir");
}
# ------------------------------------------------------------------------------
# scan_dir - build files listing for $dir
# ------------------------------------------------------------------------------
function scan_dir($dir)
{
    $files = array();
    if (file_exists($dir)) {
        $dh = opendir($dir);
        while (false !== ($filename = readdir($dh))) {
            # skip '.' and '..' names
            if (($filename !== '.') and ($filename !== '..')) $files[] = $filename;
        }
        sort($files);
    }
    return $files;
}
# ------------------------------------------------------------------------------
# restore_arc_blacklist - copy arc blacklist to db
# ------------------------------------------------------------------------------
function restore_arc_blacklist()
{
    global $squidguard_config;
    $dbhome       = SQUIDGUARD_DBHOME_DEF;
    $blklist_file = SQUIDGUARD_WORKDIR_DEF . SQUIDGUARD_BLK_ENTRIES;
    $arc_db_dir   = SQUIDGUARD_VAR . SG_BLK_ARC;
    $arc_blklist_file = SQUIDGUARD_VAR . SQUIDGUARD_BLK_ENTRIES;

    if (file_exists($arc_db_dir) and file_exists($arc_blklist_file)) {

        # copy arc blacklist to work DB with permissions
        mwexec("cp -R -p $arc_db_dir/ $dbhome");
        set_file_access($dbhome, OWNER_NAME, 0755);
        sg_addlog("restore_arc_blacklist: Restore blacklist archive from '$arc_db_dir'.", 1);

        # copy black list file
        copy($arc_blklist_file, $blklist_file);
        set_file_access($blklist_file, OWNER_NAME, 0755);
        sg_addlog("restore_arc_blacklist: Restore black list file from '$arc_blklist_file' to '$blklist_file'.", 1);
    } else {
        sg_addlog("restore_arc_blacklist: Error, file '$arc_db_dir' or '$blklist_file' not found.", 2);
    }
}

# ------------------------------------------------------------------------------
# scan_blacklist_cat - scan all dirs and subdirs tree and make blk enrties list
#     $cur_dir - start directory
#     $key_name - current key name
# ------------------------------------------------------------------------------
# blk entry[key]:
#    ["domains"]     domains file path
#    ["urls"]        urls file path
#    ["expressions"] expressions file path
# ------------------------------------------------------------------------------
function scan_blacklist_cat($curdir, $key_name, $cat_array)
{

    if (file_exists($curdir) and is_dir($curdir)) {
        $blk_entry = array();
        $files = scan_dir($curdir);

        foreach($files as $fls) {
            $fls_file = "$curdir/$fls";

            if (($fls != ".") and ($fls != "..")) {
                if (is_file($fls_file)) {

                     # add files path
                     switch(strtolower($fls)) {
                         case "domains":
                             $blk_entry["domains"] = $fls_file;
                             $blk_entry["path"]    = $curdir;
                             break;
                         case "urls":
                             $blk_entry["urls"] = $fls_file;
                             $blk_entry["path"]    = $curdir;
                             break;
                         case "expressions":
                             $blk_entry["expressions"] = $fls_file;
                             $blk_entry["path"]    = $curdir;
                             break;
                     }
                 }
                 elseif (is_dir($fls_file)) {
                     $fls_key = $key_name . "_" . $fls;

                     # recursive call
                     scan_blacklist_cat($fls_file, $fls_key, & $cat_array);
                 }
            }
        }

        if (count($blk_entry))
            $cat_array[$key_name] = $blk_entry;
    }
}

# ******************************************************************************
# squidguard utils
# ******************************************************************************
# sg_list_itemsfield - get items field list
# ------------------------------------------------------------------------------
function sg_list_itemsfield($xml_items, $fld_name)
{
    $ls = array();
    if (is_array($xml_items[FLD_ITEM]))
        foreach($xml_items[FLD_ITEM] as $it) {
            $ls[] = $it[$fld_name];
        }
    return $ls;
}

# ------------------------------------------------------------------------------
# is_url - check url an err_codes
# ------------------------------------------------------------------------------
function is_url($url)
{
    if (empty($url))                return false;
    if (eregi("^http://", $url))    return true;
    if (eregi("^https://", $url))   return true;
    if (strstr("blank", $url))      return true;
    if (strstr("blank_img", $url))  return true;
    if (eregi("^((30[1235]{1})|(40[0-9]{1})|(41[0-7]{1})|(50[0-5]{1}))", $url)) return true; # http error code 30x, 4xx, 50x.
    return false;
}
# ------------------------------------------------------------------------------
# is_masksubnet - check ip/mask
# ------------------------------------------------------------------------------
function is_masksubnet($subnet)
{
    if (!is_string($subnet))
        return false;

    list($ip,$msk) = explode('/', $subnet);
    if (!is_ipaddr($ip) || !is_ipaddr($msk))
        return false;

    return true;
}
# ------------------------------------------------------------------------------
# is_iprange - check ip1-ip2
# ------------------------------------------------------------------------------
function is_iprange($ip_range) {
    if (!is_string($ip_range))  return false;

    list($ip1,$ip2) = explode('-', $ip_range);
    if (!is_ipaddr($ip1) || !is_ipaddr($ip2)) return false;

    # ip2 < ip1 - wrong
    if (ipcmp(ip2, ip1) === -1) return false;

    return true;
}
# ------------------------------------------------------------------------------
# is_ipaddr_valid - validate IP, subnet, IP range
# ------------------------------------------------------------------------------
function is_ipaddr_valid($val)
{
    return is_string($val) && (is_ipaddr($val) || is_masksubnet($val) || is_subnet($val) || is_iprange($val));
}

# ------------------------------------------------------------------------------
# is_domain_valid - check domain format
# ------------------------------------------------------------------------------
function is_domain_valid($domain)
{
    $dm_fmt = "([a-z][a-z0-9\-]{0,})";
    $dm_fmt = "^(($dm_fmt{1,}\.){1,}$dm_fmt{2,})+$"; # example: (my.)(super.)(domain.)com
    return is_string($domain) && eregi($dm_fmt, trim($domain));
}

# ------------------------------------------------------------------------------
# is_username - check username
# ------------------------------------------------------------------------------
function is_username($username)
{
    $unm_fmt = "^\'[a-zA-Z_0-9\-]{1,}\'$";
    return is_string($username) && eregi($unm_fmt, trim($username));
}
# ------------------------------------------------------------------------------
# check name
# ------------------------------------------------------------------------------
function check_name_format ($name, $input_errors)
{
    $elog = array();
    $val = trim($name);

    if ((strlen($val) < 2) || (strlen($val) > 16))
        $elog[] = " Size of name '$val' must be between [2..16].";

    # All symbols must be [a-zA-Z_0-9\-] First symbol = letter.
    if (!eregi("^([a-zA-Z]{1})([a-zA-Z_0-9\-]+)$", $val))
        $elog[] = " Invalid name $name. Valid name symbols: ['a-Z', '_', '0-9', '-']. First symbol must be a letter.";

    # update log
    if (!empty($elog)) {
    	$input_errors = (is_array($input_errors)) ? array_merge($input_errors, $elog) : implode("\n", $elog);
    }

    return empty($elog);
}
# ******************************************************************************
# squidguard check
# ******************************************************************************
# check redirect
# ------------------------------------------------------------------------------
function sg_check_redirect($rdr_mode, $rdr_info, $err_msg)
{
    $res = true;
    switch($rdr_mode) {
        case RMOD_EXT_ERR: case RMOD_EXT_RDR: case RMOD_EXT_MOVED: case RMOD_EXT_FOUND:
            $res = is_url($rdr_info);
            if (!$res) $err_msg = "Valid URL expected, but '$rdr_info' found.";
            break;
        case RMOD_INT_SIZELIMIT:
            $res = is_numeric($rdr_path);
            if (!$res) $err_msg = "Valid number value expected, but '$rdr_info' found.";
            break;
        case RMOD_INT_BLANKPAGE: case RMOD_INT_BLANKIMG: case RMOD_INT_ERRORPAGE:
        default:
             $res = true; break;
    }
    return $res;
}

# ------------------------------------------------------------------------------
# sg_check_time
# ------------------------------------------------------------------------------
function sg_check_time($sgtime, $input_errors)
{
    $err = '';
    $days = array("*", "mon", "tue", "wed", "thu", "fri", "sat", "sun");
    $timetypes = array("weekly", "date");

    if (is_array($sgtime[FLD_ITEM])) {
        # check date and time
        foreach ($sgtime[FLD_ITEM] as $item) {
            if (!in_array(trim($item[FLD_TIMETYPE]), $timetypes))
                $err .= " Invalid type '{$item[FLD_TIMETYPE]}'.";
            if (!in_array(trim($item[FLD_TIMEDAYS]), $days))
                $err .= " Invalid week day '{$item[FLD_TIMEDAYS]}'.";
            if (trim($item[FLD_DATERANGE])) $err  .= check_date(trim($item[FLD_DATERANGE]));
            if (trim($item[FLD_TIMERANGE])) $err  .= check_time(trim($item[FLD_TIMERANGE]));
        }
    }

    # errors update
    if (!empty($err)) $input_errors[] = "TIME '{$sgtime[FLD_NAME]}': $err";
    return empty($err);
}

# ------------------------------------------------------------------------------
# sg_check_dest
# ------------------------------------------------------------------------------
function sg_check_dest($sgx, $input_errors)
{
    $elog = array();
    $dm = explode(" ", $sgx[FLD_DOMAINS]);
#   $ex = explode(" ", $sgx[FLD_EXPRESSIONS]);
    $ur = explode(" ", $sgx[FLD_URLS]);
    array_packitems(&$dm);
    array_packitems(&$ur);

    # domain
    foreach ($dm as $d_it)
        if ($d_it && !is_domain_valid($d_it)) $elog[] = "Item '$d_it' is not a domain.";

    # url
    foreach ($ur as $u_it)
        if ($u_it && !is_url($u_it)) $elog[] = "Item '$u_it' is not a url";

    # check redirect
    sg_check_redirect($sgx[FLD_RMOD], $sgx[FLD_REDIRECT], &$elog);

    # update log
    if (!empty($elog)) {
        $elog = "DEST '{$sgx[FLD_NAME]}': " . implode(" ", $elog);
        if (is_array($input_errors))
    	     $input_errors[] = $elog;
    	else $input_errors   = $elog;
    }
    return empty($elog);
}

# ------------------------------------------------------------------------------
# sg_check_src
# ------------------------------------------------------------------------------
function sg_check_src($sgx, $input_errors)
{
    $elog = array();

    # source may be as one ('source') field or as two ('ip' and 'domain') fields
    $src = (isset($sgx[FLD_SOURCE])) ? $sgx[FLD_SOURCE] : $sgx[FLD_IP] . " " . $sgx[FLD_DOMAINS];
    $src = explode(" ", $src);
    foreach ($src as $s_item) {
        if ($s_item) {
            if (!is_ipaddr_valid($s_item) and !is_domain_valid($s_item) and !is_username($s_item))
                $elog[] = "SRC '{$sgx[FLD_NAME]}': Item '$s_item' is not a ip address or a domain or a 'username'.";
        }
    }

    # update log
    if (!empty($elog)) {
    	$input_errors = (is_array($input_errors)) ? array_merge($input_errors, $elog) : implode("\n", $elog);
    }

    return empty($elog);
}
# ------------------------------------------------------------------------------
# check rebuild blacklist
# ------------------------------------------------------------------------------
function is_blacklist_update_started()
{
    return exec("ps auxw | grep \"[s]quidGuard_blk_rebuild\" | awk '{print $2}' | wc -l | awk '{ print $1 }'");
}

# ------------------------------------------------------------------------------
# Strings
# ------------------------------------------------------------------------------
# str_pack_spaces - replace two and more space to single
# ------------------------------------------------------------------------------
function str_packspaces($str)
{
    while(strpos($str, '  ')) $str = str_replace('  ', ' ', $str);
}

function array_packitems($arval)
{
    if (is_array($arval)) {
        $arval = array_map("trim", $arval);          # trim all items
        $arval = array_diff($arval, array(' ', '')); # exclude ' ' abd '' elements
        $arval = array_unique($arval);               # unique items
        $arval = array_values($arval);               # pack array
    }
    return $arval;
}

# -----------------------------------------------------------------------------
# check date
#       date or date range format: 'yyyy-mm-dd', 'yyyy-m-d', 'yyyy.mm.dd'  'yyyy.mm.dd-yyyy.mm.dd'
#       date mask format: '*-mm-dd', 'yyyy-*-dd', 'yyyy.mm.*' (but not for range)
# -----------------------------------------------------------------------------
function check_date($date)
{
    $err = '';
    $val = trim($date);
    $dtfmt = "([0-9]{4})\.([0-9]{2})\.([0-9]{2})";

    # check date range
    if (eregi("^{$dtfmt}-{$dtfmt}$", $val)) {
        $val = explode("-", str_replace(".", '', $val));
        if (intval($val[0]) >= intval($val[1]))
            $err .= "Invalid date range, begin range must be less than the end. {$val[0]} - {$val[1]}";
    }
    elseif (!eregi("^(([0-9]{4})|[*])\.(([0-9]{2})|[*])\.(([0-9]{2})|[*])$", $val)) {
        $err .= "Bad date format.";
    }

    if ($err)
        $err = " Invalid date '$date'.
                 $err
                 You mast use date or date range format: 'yyyy.mm.dd' and 'yyyy.mm.dd-yyyy.mm.dd'.
                 Also possible use mask * (mean any). Example: '*-10-01', '1990-*-*'.";
    return $err;
}

# -----------------------------------------------------------------------------
# check time
# -----------------------------------------------------------------------------
function check_time($time)
{
    $err = '';
    $time = trim($time);

    if (empty($time)) return '';

    # time range format: 'HH:MM-HH:MM'
    if (!eregi("^([0-2][0-9])\:([0-5][0-9])-([0-2][0-9])\:([0-5][0-9])$", $time))
        $err = "Invalid time range '$time'. You must use 'HH:MM-HH:MM' time range format. ";
    else {
        $tms = str_replace("-", "\n", $time);
        $tmsview = explode("\n", $tms);
        $tms = str_replace(":", "", $tms);
        $tms = explode("\n", $tms);
        if ($tms[0] >= 2400)
            $err .= "Invalid time range var1='{$tmsview[0]}' must be < '24:00'. ";
        if ($tms[1] > 2400)
            $err .= "Invalid time range var2='{$tmsview[1]}' must be <= '24:00'. ";
        if ($tms[0] >= $tms[1])
            $err .= "Invalid time range var1='{$tmsview[0]}' must be < var2='{$tmsview[1]}'. ";
    }

    return $err;
}

# -----------------------------------------------------------------------------
# acl_remove_blacklist_items
# -----------------------------------------------------------------------------
function acl_remove_blacklist_items($items)
{
    # add !items and ^items
    $db_entries = sg_entries_blacklist();
    $tdb_entries = array();
    foreach ($db_entries as $ent) {
        $tdb_entries[] = $ent;
        $tdb_entries[] = "!$ent";
        $tdb_entries[] = "^$ent";
    }
    $db_entries = $tdb_entries;
    unset($tdb_entries);

    # delete blacklist entries from 'pass' if blacklist disabled
    $items = explode(" ", $items);
    $items = implode(" ", array_diff($items, $db_entries));
    return $items;
}

# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# classes
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
class TSgTag
{
    var $tag;
    var $name;
    var $time;
    var $items;
    var $desc;

    function __construct() {
        $this->clear();
    }

    function clear() {
        $this->tag    = '';
        $this->name   = '';
        $this->time   = '';
        $this->items  = array();
        $this->desc   = '';
    }

    function set($tag, $name, $time, $desc) {
        $this->tag    = $tag;
        $this->name   = $name;
        $this->time   = $time;
        $this->desc   = $desc;
    }

    function tag_text($offset = 0) {
        $str = array();
        $off = str_repeat("\t", $offset);

        $str[] = $off . "# {$this->desc}";
        if (empty($this->time))
             $str[] = $off . "{$this->tag} {$this->name} {";
        else $str[] = $off . "{$this->tag} {$this->name} within {$this->time} {";

        # get items
        foreach($this->items as $it) {
            if (is_a($it, "TSgTag"))
                $str[] = $off . $it->tag_text($offset + 1); # sub tag
            else $str[] = $off . "\t{$it}";                  # item
        }

        $str[] = $off . "}";
        return implode("\n", $str);
    }
}

?>
