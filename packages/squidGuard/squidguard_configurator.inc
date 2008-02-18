<?php
# ------------------------------------------------------------------------------
/*        squidguard_configurator.inc
        (C)2006 Serg Dvoriancev
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
# squidGuard -C all                          - apdate database
# squidGuard -c <configfile>        - create squidGuard with specified config file
# ------------------------------------------------------------------------------
# Notes:
#   -- for work squidGuard need present ALL destinations;
#      if dest not present in config - then this item will ignored in operations
#      (in rebuild DB for example)
# ------------------------------------------------------------------------------
# Directories:
# work path       - $workdir
# log path        - $workdir + $logdir
# ------------------------------------------------------------------------------

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');
require_once('squid.inc');

# ------------------------------------------------------------------------------
# files header
# ------------------------------------------------------------------------------
define('FILES_DB_HEADER', '
# ------------------------------------------------------------------------------
#        File created by squidGuard package GUI
#        (C)2006 Serg Dvoriancev
# ------------------------------------------------------------------------------
');

define('CONFIG_SG_HEADER', '
# ============================================================
# SquidGuard configuration file
#
# This file generated automaticly with SquidGuard configurator
#
# (C)2006 Serg Dvoriancev
# email: dv_serg@mail.ru
# ============================================================
');

define('ACL_WARNING_ABSENSE_PASS', "!WARNING! Absence PASS 'all' or 'none' added as 'none'");

# ------------------------------------------------------------------------------
# squid config options
# ------------------------------------------------------------------------------
define('REDIRECTOR_OPTIONS_REM',       '# squidGuard options');
define('REDIRECTOR_PROGRAM_OPT',       'redirect_program');
define('REDIRECT_BYPASS_OPT',          'redirector_bypass');
define('REDIRECT_CHILDREN_OPT',        'redirect_children');

# ------------------------------------------------------------------------------
# setup count redirector processes will started
# * for big count users service increase this option,
#   but you need use this on powerful system
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
# !check this!
define('SQUID_CONFIGFILE',             '/usr/local/etc/squid/squid.conf');
define('TMP_DIR',                      '/var/tmp');

# not need for check
define('SQUIDGUARD_CONFIGFILE',        '/squidGuard.conf');
define('SQUIDGUARDCONF_LOGFILE',       '/sg_configurator.log');
define('SQUIDGUARD_ACCESSBLOCK_FILE',  'block.log');
define('SQUIDGUARD_CONFBASE_DEF',      '/usr/local/etc/squid');
define('SQUIDGUARD_LOGDIR_DEF',        '/tmp');
define('SQUIDGUARD_WORKDIR_DEF',       '/usr/local/etc/squidGuard');
define('SQUIDGUARD_BINPATH_DEF',       '/usr/local/bin');
define('SQUIDGUARD_TMP',               '/var/tmp/squidGuard');                  // SG temp
define('SQUIDGUARD_VAR',               '/var/squidGuard');                      // SG variables

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
define('BLK_TEMP',                     '/tmp/sg_blk');
define('SG_BLK_ARC',                   '/arcdb');                               // blk db archive
define('SG_INFO_FILE',                 '/var/squidGuard/sg_db_upd.inf');

# ------------------------------------------------------------------------------
# debug options
# ------------------------------------------------------------------------------
define('DEBUG_UPDATE_SQUID_CONF',        'true');
define('DEBUG_UPDATE_SQUIDGUARD_DB',     'true');
define('DEBUG_MAKE_SQUIDGUARD_CONFIG',   'true');

# GUI options
define('SQUIDGUARD_LOG_MAXCOUNT',        1000);                                 // max log lines
# 0-all, 1-medium; 2-low
define('SQUIDGUARD_LOG_LEVEL',           0);

# ------------------------------------------------------------------------------
#
define('FLT_DEFAULT_ALL', 'all');
# ------------------------------------------------------------------------------
# owner user name (squid system user - need for define rights access)
# ------------------------------------------------------------------------------
define('OWNER_NAME', 'proxy');
# ------------------------------------------------------------------------------
#
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
$squidguard_config = array();                                                   // squidGuard config array

// call default init
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

// other fields
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
define('FLD_REDIRECTMODE',        'redirect_mode');                             // [redirect_mode] = rmod_int <base- use sgerror.php>; rmod_301; rmod_302;

// transparent mode
define('FLD_SQUID_TRANSPARENT_MODE',   'squid_transparent_mode');
define('FLD_CURRENT_LAN_IP',           'current_lan_ip');
define('FLD_CURRENT_GUI_PORT',         'current_gui_port');
define('FLD_CURRENT_GUI_PROTO',        'current_gui_protocol');

# ------------------------------------------------------------------------------
# sg_init
# - initialize config array
# ------------------------------------------------------------------------------
function sg_init($init = '') {
       global $squidguard_config;

       $squidguard_config = array();
       if(empty($init) or !is_array($init) ) {
           // default init (for generate minimal config)
           $squidguard_config[FLD_LOGDIR]          = SQUIDGUARD_LOGDIR_DEF;
           $squidguard_config[FLD_DBHOME]          = SQUIDGUARD_DBHOME_DEF;
           $squidguard_config[FLD_WORKDIR]         = SQUIDGUARD_WORKDIR_DEF;
           $squidguard_config[FLD_BINPATH]         = SQUIDGUARD_BINPATH_DEF;
           $squidguard_config[FLD_SQUIDCONFIGFILE] = SQUID_CONFIGFILE;
           $squidguard_config[FLD_PROCCESSCOUNT]   = REDIRECTOR_PROCESS_COUNT;
           sg_addlog("sg_init: default initialization squidguard_config");
       } else {
           $squidguard_config = $init;
           sg_addlog("sg_init: ext initialization squidguard_config");
       }
       return $squidguard_config;
}

# ------------------------------------------------------------------------------
# sg_loadconfig_xml
# ------------------------------------------------------------------------------
function sg_load_configxml($filename) {
       global $squidguard_config;
       sg_init();
       if (file_exists($filename)) {
           $xmlconf = file_get_contents($filename);
           sg_addlog("sg_load_configxml: load config from $filename");

           if (!empty($xmlconf)) {
                $squidguard_config = $xmlconf[FLD_SQUIDGUARD];
                sg_addlog("sg_load_configxml: update config success.");
           } else
                sg_addlog("sg_load_configxml: update config error.");
       } else
           sg_addlog("sg_load_configxml: error load config from $filename - file not exists.");
}

# ------------------------------------------------------------------------------
# sg_saveconfig_xml
# ------------------------------------------------------------------------------
function sg_save_configxml($filename) {
         global $squidguard_config;
         $xmlconf = dump_xml_config($squidguard_config, FLD_SQUIDGUARD);
         file_put_contents($filename, $xmlconf);
}

# ------------------------------------------------------------------------------
# sg_reconfigure
# - squidguard reconfiguration
# ------------------------------------------------------------------------------
function sg_reconfigure() {
         global $squidguard_config;
         sg_addlog("sg_reconfigure: start.");

         // 1. check system
         sg_check_system();

         // 2. reconfigure user db
         sg_reconfigure_user_db();

         // 3. generate squidGuard config
         $conf_file = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARD_CONFIGFILE;
         $conf = sg_build_config();
         if ($conf) {
             $conf = implode("\n", $conf);
             if ($squidguard_config[FLD_WORKDIR])
                 $conf_file = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_CONFIGFILE;
             file_put_contents($conf_file, $conf);
             file_put_contents('/usr/local/etc/squid' . SQUIDGUARD_CONFIGFILE, $conf); // << squidGuard want config '/usr/local/etc/squid' by default
             set_file_access($squidguard_config[FLD_WORKDIR], OWNER_NAME, 0755);
             sg_addlog("sg_reconfigure: generate squidGuard config and save to $conf_file.");
         }

         // 4. reconfigure squid
         squid_reconfigure();

         sg_addlog("sg_reconfigure: end.");
}

// ------------------------------------------------------------
// squid_reconfigure
// Insert in '/usr/local/squid/etc/squid.conf' options:
//         redirector_bypass on
//         redirect_program /usr/local/squidGuard/bin/squidGuard -c /path_to_config_file
//         redirect_children 1
// ------------------------------------------------------------

function squid_reconfigure($remove_only = '') {
         global $config;
         global $squidguard_config;
         $conf = '';
         $cust_opt = $config['installedpackages']['squid']['config'][0]['custom_options'];

         sg_addlog("squid_reconfigure: begin");

         // remove old options
         if (!empty($cust_opt)) {
              $conf = explode(";", $cust_opt);
              foreach ($conf as $key => $c_opt) {
                       $t_opt = ltrim($c_opt);
                       if ((strpos($t_opt, REDIRECTOR_PROGRAM_OPT) === 0) or
                           (strpos($t_opt, REDIRECT_BYPASS_OPT)    === 0) or
                           (strpos($t_opt, REDIRECT_CHILDREN_OPT)  === 0))
                          unset($conf[$key]);
              }
              sg_addlog("squid_reconfigure: remove old redirector options from Squid config.");
         }

         // add new options - if squidGuard enabled
         if (empty($remove_only) && ($squidguard_config[FLD_ENABLED] === 'on')) {

             $redirector_path = $squidguard_config[FLD_BINPATH] . '/squidGuard';
             $redirector_conf = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_CONFIGFILE;

             $conf[] = REDIRECTOR_PROGRAM_OPT . " $redirector_path -c $redirector_conf";
             $conf[] = REDIRECT_BYPASS_OPT    . " on";
             $conf[] = REDIRECT_CHILDREN_OPT  . " " . REDIRECTOR_PROCESS_COUNT;

             sg_addlog("squid_reconfigure: add new redirector options to Squid config.");
         }

         // update config
         if (is_array($conf))
             $conf = implode(";", $conf);
         $config['installedpackages']['squid']['config'][0]['custom_options'] = $conf;
         write_config('Update squidGuard option to squid config.');

         squid_resync();
}

// ------------------------------------------------------------
// sg_check_system
// - check squidguard catalog's and access right's
// ------------------------------------------------------------
function sg_check_system() {
         global $squidguard_config;

         // check work_dir & create if not exists
         $work_dir = $squidguard_config[FLD_WORKDIR];
         if (!empty($work_dir)) {
             // check dir's
             if (!file_exists($work_dir)) {
                  mwexec("mkdir -p " . $work_dir);
                  sg_addlog("Create work dir $work_dir");
                  // set access right
                  set_file_access($work_dir, OWNER_NAME, 0755);
             }
         }
         unset($work_dir);

         // check log_dir & create if not exists
         $log_dir = $squidguard_config[FLD_LOGDIR];
         if (!empty($log_dir)) {
            if (!file_exists($log_dir)) {
                 mwexec("mkdir -p " . $log_dir);
                 sg_addlog("Create log dir $log_dir");
            }
            // set access right - need start any time;
            // (SG possible start from console and log file will have only root access)
            set_file_access($log_dir, OWNER_NAME, 0755);
         }
         unset($log_dir);

         // check db dir
         $db_dir = $squidguard_config[FLD_DBHOME];
         if (!empty($db_dir)) {
            if (!file_exists($db_dir)) {
                 mwexec("mkdir -p $db_dir");
                 $sg_addlog("Create db dir $db_dir");
                 // set access right
                 set_file_access($db_dir, OWNER_NAME, 0755);
            }
         }
         unset($db_dir);
}
# ==============================================================================
# squidGuard DB
# ==============================================================================
# sg_reconfigure_user_db
#  - reconfigure(update) db user entries
# ------------------------------------------------------------------------------
function sg_reconfigure_user_db() {
         global $squidguard_config;
         $dbhome = $squidguard_config[FLD_DBHOME];

         sg_addlog("sg_reconfigure_user_db: begin at '$dbhome'");

         // create user DB catalog, if not extsts
         if (!file_exists($dbhome)) {
             if (!mkdir($dbhome, 0755)) {
                  sg_addlog("sg_reconfigure_user_db: ERROR create user DB directory $dbhome");
                  return;
             }
             set_file_access($dbhome, OWNER_NAME, 0755);
             sg_addlog("sg_reconfigure_user_db: Create user DB directory $dbhome");
         }

         // update destinations to db
         $dests = $squidguard_config[FLD_DESTINATIONS];
         if(!empty($dests)){
             $dst_names = Array();
             $dst_list  = Array();   // destinations list

             sg_addlog("sg_reconfigure_user_db: add user entries");
             foreach($dests[FLD_ITEM] as $dst) {
                     $path = "$dbhome/" . $dst[FLD_NAME];
                     $dst_names[]   = $path;
                     $dst_list["usr_{$dst[FLD_NAME]}"] = $dst[FLD_NAME];

                     // 1. check destination catalog and create them, if need
                     if (!file_exists($path)) {
                          if (!mkdir ($path, 0755)) {
                               sg_addlog("sg_reconfigure_user_db: ERROR create dir $path");
                               return;
                          }
                          sg_addlog("sg_reconfigure_user_db: Create dir $path");
                     }

                     // 2. build domains file
                     $domains = $dst[FLD_DOMAINS];
                     if (!empty($domains)) {
                          $content = str_replace(" ", "\n", $domains);
                          $content = trim($content);
                          file_put_contents($path . '/domains', $content);
                          sg_addlog("sg_reconfigure_user_db: -- add {$dst[FLD_NAME]} domains '$domains'");
                     }
                     unset($domains);

                     // 3. build urls file
                     $urls = $dst[FLD_URLS];
                     if (!empty($urls)) {
                          $content = str_replace(" ", "\n", $urls);
                          $content = trim($content);
                          file_put_contents($path . '/urls', $content);
                          sg_addlog("sg_reconfigure_user_db: -- add {$dst[FLD_NAME]} urls '$content'");
                     }
                     unset($urls);

                     // 4. build expression file
                     $expr = $dst[FLD_EXPRESSIONS];
                     if (!empty($expr)) {
                          $content = str_replace("|", " ", $expr);
                          $content = trim($content); // delete first and last unnecessary '|' symbols
                          $content = str_replace(" ", "|", $content);
                          file_put_contents($path . '/expressions', $content);
                          sg_addlog("sg_reconfigure_user_db: -- add {$dst[FLD_NAME]} expressions '$content'");
                     }
                     unset($expr);
             }

             // 4. recursive set files access
             set_file_access($dbhome, OWNER_NAME, 0755);

             // 5. rebuild user db ('/var/db/squidGuard')
             sg_rebuild_db("_usrdb", $dbhome, $dst_list);

         } else
             sg_addlog("sg_reconfigure_user_db: STOPPED; User destinations list empty");

         // 6. remove unused db entries
         sg_remove_unused_db_entries();

         sg_addlog("sg_reconfigure_user_db: end.");
}

// ------------------------------------------------------------
// sg_remove_unused_db_entries
// - remove unused DB entries
// ------------------------------------------------------------
function sg_remove_unused_db_entries() {
         global $squidguard_config;
         $db_entries = array();
         $file_list = '';
         $dbhome = $squidguard_config[FLD_DBHOME];
         $workdir = $squidguard_config[FLD_WORKDIR];

         sg_addlog("sg_remove_unused_db_entries: begin");

         // black list entries
         // * worked only with 'blacklist entries list file - else may be deleted black list entry
         if (file_exists($workdir .  SQUIDGUARD_BLK_ENTRIES)) {
             $file_for_del = array();

             // load blk entries
             $db_entries = explode("\n", file_get_contents($workdir . SQUIDGUARD_BLK_ENTRIES));

             // $db_entries + add user entries
             $dests = $squidguard_config[FLD_DESTINATIONS];
             if (!empty($dests)) {
                  foreach($dests[FLD_ITEM] as $dst) {
                          $db_entries[] = $dst[FLD_NAME];
                  }
             }

             // diff between file list and entries list
             $file_list = scan_dir($dbhome);
             if (is_array($file_list) and is_array($db_entries))
                 $file_for_del = array_diff($file_list, $db_entries);

             // delete
             if (is_array($file_for_del) and !empty($file_for_del)) {
                  foreach($file_for_del as $fd) {
                          $file_fd = "$dbhome/$fd";
                          if (($fd != "") && ($fd != ".") && ($fd != "..")) {
                               if (file_exists($file_fd)) {
                                   mwexec("rm -R  . $file_fd");
                                   sg_addlog("sg_remove_unused_db_entries: Removed file '$file_fd'");
                               } else
                                   sg_addlog("sg_remove_unused_db_entries: File $file_fd not found");
                          }
                  }
             }
         }
         sg_addlog("sg_remove_unused_db_entries: end");
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

function sg_rebuild_db($shtag, $rdb_dbhome, $rdb_itemslist) {
         global $squidguard_config;
         $conf   = '';
         $conf_path = '';
         $logdir = $squidguard_config[FLD_LOGDIR];
         $dbhome = $squidguard_config[FLD_DBHOME];

         // current dbhome dir
         if (!empty($rdb_dbhome)) $dbhome = $rdb_dbhome;
         sg_addlog("sg_rebuild_db: Begin with path '$dbhome'.");

         // define - where config will placed
         $conf_path = "/tmp/squidGuard_rebuild.conf" . $shtag;

         // make rebuild config; include all found dest items
         $conf = sg_create_rebuild_config($dbhome, $rdb_itemslist);
         file_put_contents($conf_path, $conf);
         set_file_access($conf_path, OWNER_NAME, 0750);
         sg_addlog("sg_rebuild_db: Create temporary config '$conf_path'.");

         // *** SH script ***
         $sh_scr = Array();
         $sh_scr[] = "#!/bin/sh";
         $sh_scr[] = "cd $dbhome";
         $cmd = $squidguard_config[FLD_BINPATH] . "/squidGuard -c $conf_path -C all";
         $sh_scr[] = $cmd;
         $sh_scr[] = "wait"; // wait while SG rebuild DB

         // set DB owner and right access
         $sh_scr[] = "chown -R -v " . OWNER_NAME . " $dbhome";

         // restart squid for changes to take effects
         $sh_scr[] = "/usr/local/sbin/squid -k reconfigure";

         // store & exec sh
         $sh_scr = implode("\n", $sh_scr);
         $shfile =  DB_REBUILD_SH . $shtag;
         file_put_contents($shfile, $sh_scr);
         set_file_access($shfile, OWNER_NAME, 0750);
         mwexec_bg($shfile);
         sg_addlog("sg_rebuild_db: Started SH script '$shfile'.");
         sg_addlog("sg_rebuild_db: End.");
}

# ------------------------------------------------------------------------------
# sg_create_rebuild_config
#     Create config for DB rebuilding
#     Default rule - block all
#     Variables:
#         $blk_dbhome   - temporary DB home dir, may be different with DBHOME
#         $blk_destlist - is array as [dst_name] = 'path',
#                         where path - catalog without dbhome path
#                         For example: dbhome is '/var/db/squidGuard/',
#                         path can be 'usr/ads' or 'bl/poxy'
# ------------------------------------------------------------------------------
function sg_create_rebuild_config($blk_dbhome, $blk_destlist) {
         global $squidguard_config;
         $sgconf = array();
         $logdir = $squidguard_config[FLD_LOGDIR];
         $dbhome = $squidguard_config[FLD_DBHOME];

         // current dbhome dir
         if (!empty($blk_dbhome)) $dbhome = $blk_dbhome;
         sg_addlog("sg_create_rebuild_config: Begin with dbhome='$dbhome'.");

         // header
         $sgconf[] = CONFIG_SG_HEADER;

         // init section
         $sgconf[] = "logdir $logdir";
         $sgconf[] = "dbhome $dbhome";
         $sgconf[] = "";

         // destination section
         if (is_array($blk_destlist)) {
             foreach($blk_destlist as $dst => $dpath) {
                 $tmp_s = array();
                 // check item elements
                 if (file_exists("$dbhome/$dpath/domains"))     $tmp_s[] = "\t domainlist $dpath/domains";
                 if (file_exists("$dbhome/$dpath/urls"))        $tmp_s[] = "\t urllist $dpath/urls";
                 if (file_exists("$dbhome/$dpath/expressions")) $tmp_s[] = "\t expressionlist $dpath/expressions";
                 // create only valid items
                 if (!empty($tmp_s)) {
                     $sgconf[] = "dest $dst {";
                     $sgconf[] = implode("\n", $tmp_s);
                     $sgconf[] = "}";
                     $sgconf[] = "";
                     sg_addlog("sg_create_rebuild_config: -- added item '$dst' = '$dbhome/$dpath'.");
                 } else
                     sg_addlog("sg_create_rebuild_config: -- ignored empty item '$dst' = '$dbhome/$dpath'.");
             }
         }

         // acl section
         $sgconf[] = "acl {";
         $sgconf[] = "\t default {";
         $sgconf[] = "\t\t pass all"; // yes, now this 'pass all' - this is only temp config
         $sgconf[] = "\t\t redirect " . sg_redirector_base_url('404', true); // use sgerror only!
         $sgconf[] = "\t }";
         $sgconf[] = "}";

         sg_addlog("sg_create_rebuild_config: End.");
         return implode("\n", $sgconf);
}

// ============================================================
// Log
// ------------------------------------------------------------
// sg_addlog
// ------------------------------------------------------------
function sg_addlog($log, $level = 0) {
         global $squidguard_config;

         // log level
         if ($level < SQUIDGUARD_LOG_LEVEL) return;

         $logfile = '';
         $logfile = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARDCONF_LOGFILE;
         $log_content = array();

         setlocale(LC_TIME, '');
         $dt = date("d.m.Y H:i:s");

         // define logfile
         if (!empty($squidguard_config)) {
             if (file_exists($squidguard_config[FLD_LOGDIR]))
                 $logfile = $squidguard_config[FLD_LOGDIR] . SQUIDGUARDCONF_LOGFILE;
         } else
             $log_content[] = "$dt : " . "sg_addlog: ERROR: squidguard_config is empty";

         $tmplog = '';
         if (file_exists($logfile))
             $tmplog = file_get_contents($logfile);
         $log_content = explode("\n", $tmplog);
         unset($tmplog);

         // shrink to MAXCOUNT log entries
         $log_content[] = "$dt : $log";
         while (count($log_content) > SQUIDGUARD_LOG_MAXCOUNT) array_shift($log_content);

         file_put_contents($logfile, implode("\n", $log_content));
}
// ------------------------------------------------------------
// sg_getlog
// ------------------------------------------------------------
function sg_getlog($last_entries_count) {
         global $squidguard_config;
         $log_content = '';
         $logfile = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARDCONF_LOGFILE;

         // define logfile
         if (!empty($squidguard_config))
                if (file_exists($squidguard_config[FLD_LOGDIR]))
                        $logfile = $squidguard_config[FLD_LOGDIR] . SQUIDGUARDCONF_LOGFILE;

         // get log last 100 entries
         if (file_exists($logfile)) {
             $log_content = file_get_contents($logfile);
             $log_content = explode("\n", $log_content);
             while (count($log_content) > $last_entries_count)  array_shift($log_content);
             // insert log file name on top
             $log_content[0] = $logfile;
             $log_content = implode("\n", $log_content);
         }

         return $log_content;
}
# -------------------------------------------------------------
# sg_build_default_config
# default rule - block all
#   $dest_list - is array as [dst_name] = path, where path - without dbhome
#   For example: dbhome is '/var/db/squidGuard', path can be 'usr/ads' or 'bl/poxy'
# -------------------------------------------------------------
function sg_build_default_config() {
         global $squidguard_config;
         $sgconf = array();

         // header
         $sgconf[] = CONFIG_SG_HEADER;
         // init section
         $sgconf[] = "logdir {$squidguard_config[FLD_LOGDIR]}";
         $sgconf[] = "dbhome {$squidguard_config[FLD_DBHOME]}";
         $sgconf[] = "";
         // acl section
         $sgconf[] = "acl {";
         $sgconf[] = "\t default {";
         $sgconf[] = "\t\t pass none";
         $sgconf[] = "\t\t redirect " . sg_redirector_base_url('404 Check proxy filter settings on errors.', true); # use only sgerror.php
         $sgconf[] = "\t }";
         $sgconf[] = "}";

         sg_addlog("sg_build_default_config: ATTENTION! Created default configuration. All content will blocked.");
         sg_addlog("sg_build_default_config: End.");
         return implode("\n", $sgconf);
}

// ------------------------------------------------------------------------------------------------
// sg_redirector_base_url
//   $url - url where redirect to
//   $use_internal - ignore 'Redirect mode' option, use internal (for rebuild config, for example)
// ------------------------------------------------------------------------------------------------
function sg_redirector_base_url($url, $use_internal = false) {
         global $squidguard_config;
         $rdr_path = '';

         // Redirect option must have any valid URL
         // 301:redirect_url
         if (!$use_internal and !empty($url) and ($squidguard_config[FLD_REDIRECTMODE] === 'rmod_301')) {
             $rdr_path = "301:$url";
         }
         // 302:redirect_url
         elseif (!$use_internal and !empty($url) and ($squidguard_config[FLD_REDIRECTMODE] === 'rmod_302')) {
             $rdr_path = "302:$url";
         }
         // sgerror.php
         else {
             $rdr_path = REDIRECT_BASE_URL;

             // check GUI port settings
             if (isset($squidguard_config[FLD_CURRENT_GUI_PORT]) and !empty($squidguard_config[FLD_CURRENT_GUI_PORT])) {
                    $rdr_path = ":" . $squidguard_config[FLD_CURRENT_GUI_PORT] . $rdr_path;
             }

             // check Lan IP port settings
             $rdr_path = $squidguard_config[FLD_CURRENT_LAN_IP] . $rdr_path;

             // check GUI protocol settings
             if ($squidguard_config[FLD_CURRENT_GUI_PROTO] === "https")
                  $rdr_path = "https://" . $rdr_path;
             else $rdr_path = "http://"  . $rdr_path;

             // arguments
             // Attention:
             //         order arg's must be: first-URL, last-SG variables
             //                SG have bug, what broke data after his var's
             $tmp_url = ltrim($url);
             // not need encoded url string. only err_code line
             if (eregi("^https{0,1}://", $tmp_url) === false)
                 $tmp_url = rawurlencode($tmp_url);
             $rdr_path = $rdr_path . "?url=" . $tmp_url . REDIRECT_URL_ARGS; // rawurlencode($tmp_url) . REDIRECT_URL_ARGS;
             unset($tmp_url);
         }

         sg_addlog("sg_redirector_base_url: select redirector base url ($rdr_path)");
         sg_addlog("sg_redirector_base_url: End.");
         return $rdr_path;
}

// ------------------------------------------------------------
// sg_build_config
// ------------------------------------------------------------
function sg_build_config() {
         global $squidguard_config;
         $sgconf = array();
#         $redirect_base_url = REDIRECT_BASE_URL;

         sg_addlog("sg_build_config: create squidGuard config.");
         if(!is_array($squidguard_config)) {
            sg_addlog("sg_build_config: error configuration in squidguard_config.");
            return sg_build_default_config();
         }

         // check configuration data
         sg_addlog("sg_build_config: checking configuration data.");
         $s = sg_check_config_data();
         if ($s) {
            sg_addlog("sg_build_config: error configuration data. It's all errors: \n$s");
            sg_addlog("sg_build_config: terminated.");
            return sg_build_default_config();
         }
         unset($s);

         // --- Header ---
         $sgconf[] = CONFIG_SG_HEADER;

         // redirector base url
#         $redirect_base_url = sg_redirector_base_url();

         // init
         $sgconf[] = "logdir " . $squidguard_config[FLD_LOGDIR];
         $sgconf[] = "dbhome " . $squidguard_config[FLD_DBHOME];

         // --- Times ---
         if ($squidguard_config[FLD_TIMES]) {
             sg_addlog("sg_build_config: add times");
             foreach($squidguard_config[FLD_TIMES][FLD_ITEM] as $tm) {
                 $sgconf[] = "";
                 if ($tm[FLD_DESCRIPTION])
                     $sgconf[] = "# " . $tm[FLD_DESCRIPTION];
                 $sgconf[] = "time " . $tm[FLD_NAME] . " {";
                 foreach($tm[FLD_ITEM] as $itm) {
                         switch ($itm[FLD_TIMETYPE]) {
                                 case "weekly":
                                      $sgconf[] = "\t weekly " . $itm[FLD_TIMEDAYS] . " " . $itm[FLD_TIMERANGE];
                                      break;
                                 case "date":
                                      $sgconf[] = "\t date "   . $itm[FLD_DATERANGE] . " " . $itm[FLD_TIMERANGE];
                                      break;
                         }
                 }
                 $sgconf[] = "}";
             }
         }

         // --- Sources ---
         if ($squidguard_config[FLD_SOURCES]) {
             sg_addlog("sg_build_config: add sources");
             foreach($squidguard_config[FLD_SOURCES][FLD_ITEM] as $src) {
                 $sgconf[] = "";
                 if ($src[FLD_DESCRIPTION])
                     $sgconf[] = "# " . $src[FLD_DESCRIPTION];
                 $sgconf[] = "src " . $src[FLD_NAME] . " {";
                 // IP
                 if ($src[FLD_IP]) {
                     $s_ip = explode(" ", $src[FLD_IP]);
                     foreach($s_ip as $ip)
                             if (!empty($ip)) $sgconf[] = "\t ip " . $ip;
                 }
                 // domains
                 if ($src[FLD_DOMAINS]) {
                     $dms =  explode(" ", $src[FLD_DOMAINS]);
                     foreach($dms as $dm)
                             if (!empty($dm)) $sgconf[] = "\t domain " . $dm;
                 }
                 if ($src[FLD_LOG])
                     $sgconf[] = "\t log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                 $sgconf[] = "}";
             }
         }

         // --- Blacklist ---
         #
         # Note! Blacklist must be added to config permanently. It's need for rebuild DB
         #
         $db_entries = sg_entries_blacklist();
         if (($squidguard_config[FLD_BLACKLISTENABLED] === 'on') and $db_entries) {
             $log_entr_added = '';
             $log_entr_ignored = '';
             sg_addlog("sg_build_config: add blacklist entries");
             foreach($db_entries as $key => $ent) {
                     $ent_state = array();
                     $file_dms  = $squidguard_config[FLD_DBHOME] . "/$ent/domains";
                     $file_urls = $squidguard_config[FLD_DBHOME] . "/$ent/urls";
                     $file_expr = $squidguard_config[FLD_DBHOME] . "/$ent/expressions";

                     // check blacklist acl state
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

                     // create config
                     $sgconf[] = "";
                     if ($ent_state['exists']) {
                         $sgconf[] = "dest $ent {";
                         $dstname = $ent;
                         if ($ent_state[FLD_DOMAINS])     $sgconf[] = "\t domainlist $ent/domains";
                         if ($ent_state[FLD_EXPRESSIONS]) $sgconf[] = "\t expressionlist $ent/expressions";
                         if ($ent_state[FLD_URLS])        $sgconf[] = "\t urllist $ent/urls";
                         $sgconf[] = "\t log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                         $sgconf[] = "}";
                         $log_entr_added .= " $ent;";
                     } else {
                         $sgconf[] = "\t# Config ERROR: Destination '$ent' not found in DB";
                         $log_entr_ignored .= " $ent;";
                     }
             }

             // log 'added'
             if (!empty($log_entr_added))
                 sg_addlog("sg_build_config: added: \n $log_entr_added \n");

             // log 'ignored'
             if (!empty($log_entr_ignored))
                 sg_addlog("sg_build_config: ignored: \n $log_entr_ignored \n");
         }

         // --- Destinations ---
         if ($squidguard_config[FLD_DESTINATIONS]) {
             sg_addlog("sg_build_config: add destinations");
             $sgconf[] = "";
#             $sgconf[] = "dest localhost { # fix localhost access problem on transparent proxy ";
#             $sgconf[] = "\t ip 127.0.0.1";
#             $sgconf[] = "}";
             $log_entr_added = '';
             foreach($squidguard_config[FLD_DESTINATIONS][FLD_ITEM] as $dst) {
                 $dstname = $dst[FLD_NAME];
                 $sgconf[] = "";
                 if ($dst[FLD_DESCRIPTION])
                     $sgconf[] = "# " . $dst[FLD_DESCRIPTION];
                 $sgconf[] = "dest $dstname {";
                 if ($dst[FLD_DOMAINS])
                     $sgconf[] = "\t domainlist $dstname/domains";
                 if ($dst[FLD_EXPRESSIONS])
                     $sgconf[] = "\t expressionlist $dstname/expressions";
                 if ($dst[FLD_URLS])
                     $sgconf[] = "\t urllist $dstname/urls";
                 if ($dst[FLD_REDIRECT] && is_url($dst[FLD_REDIRECT]))
                     $sgconf[] = "\t redirect " . sg_redirector_base_url($dst[FLD_REDIRECT]);
                 if ($dst[FLD_LOG])
                     $sgconf[] = "\t log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                 $sgconf[] = "}";
                 $log_entr_added .= " $dstname;";
             }
             // log
             if (!empty($log_entr_added))
                  sg_addlog("sg_build_config: added: \n $log_entr_added \n");
             else sg_addlog("sg_build_config: added: Nothing.");
         }

         // --- Rewrites ---
         if ($squidguard_config[FLD_REWRITES]) {
             sg_addlog("sg_build_config: add rewrites.");
             $log_entr_added = '';
             foreach($squidguard_config[FLD_REWRITES][FLD_ITEM] as $rew) {
                 $sgconf[] = "";
                 $sgconf[] = "rew " . $rew[FLD_NAME] . " {";
                 foreach ($rew[FLD_ITEM] as $rw)
                          $sgconf[] = "\t s@." . $rw[FLD_TARGETURL] . "@" . $rw[FLD_REPLACETO]."@r";
                 if ($rew[FLD_LOG])
                     $sgconf[] = "\t log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                 $sgconf[] = "}";
                 $log_entr_added .= " {$rew[FLD_NAME]};";
             }
             // log
             if (!empty($log_entr_added))
                  sg_addlog("sg_build_config: added: \n $log_entr_added \n");
             else sg_addlog("sg_build_config: added: Nothing.");
         }

         # ----------------------------------------
         $entry_blacklist = sg_entries_blacklist();

         // --- ACL ---
         $sgconf[] = "";
         $sgconf[] = "acl {";
         if ($squidguard_config[FLD_ACLS]) {
             sg_addlog("sg_build_config: add ACL");
             $log_entr_added = '';
             foreach($squidguard_config[FLD_ACLS][FLD_ITEM] as $acl) {

                 // delete blacklist entries from 'pass' if blacklist disabled
                 if ($squidguard_config[FLD_BLACKLISTENABLED] !== 'on') {
                     $tarray = explode(" ", $acl[FLD_DESTINATIONNAME]);
                     $varray = explode(" ", $acl[FLD_OVERDESTINATIONNAME]);
                     foreach($entry_blacklist as $entry) {
                         $tk = array_search($entry, $tarray);
                         if ($tk !== false) unset ($tarray[$tk]);

                         $tk = array_search("!$entry", $tarray);
                         if ($tk !== false) unset($tarray[$tk]);

                         $tk = array_search($entry, $varray);
                         if ($tk !== false) unset ($varray[$tk]);

                         $tk = array_search("!$entry", $varray);
                         if ($tk !== false) unset ($varray[$tk]);
                     }
                     $acl[FLD_DESTINATIONNAME]     = implode (" ", $tarray);
                     $acl[FLD_OVERDESTINATIONNAME] = implode (" ", $varray);
                 }

                 if (!$acl[FLD_DISABLED]) {
                      if ($acl[FLD_DESCRIPTION])
                          $sgconf[] = "\t # " . $acl[FLD_DESCRIPTION];

                      if ($acl[FLD_TIMENAME]) {
                          // ontime
                          $sgconf[] = "\t " . $acl[FLD_NAME] . " within " . $acl[FLD_TIMENAME] . " { ";
                          $sgconf[] = "\t\t pass " . $acl[FLD_DESTINATIONNAME];
                          if ($acl[FLD_REDIRECT]) {
                              if (is_url($acl[FLD_REDIRECT]))
                                   $sgconf[] = "\t\t redirect " . $redirect_user_url . rawurlencode($acl[FLD_REDIRECT]);
                              else $sgconf[] = "\t\t redirect " . $redirect_user_url . "?msg=" . htmlspecialchars($acl[FLD_REDIRECT]);
                          }
                          if ($acl[FLD_REWRITENAME])
                              $sgconf[] = "\t\t rewrite " . $acl[FLD_REWRITENAME];

                          // overtime
                          $sgconf[] = "\t } else {";
                          $sgconf[] = "\t\t pass " . $acl[FLD_OVERDESTINATIONNAME];
                          if ($acl[FLD_OVERREDIRECT] && is_url($acl[FLD_OVERREDIRECT]))
                                   $sgconf[] = "\t\t redirect " . sg_redirector_base_url($acl[FLD_OVERREDIRECT]); # $redirect_base_url . rawurlencode($acl[FLD_OVERREDIRECT]);
                          if ($acl[FLD_OVERREWRITENAME])
                              $sgconf[] = "\t\t rewrite " . $acl[FLD_OVERREWRITENAME];

                          $sgconf[] = "\t }";
                      } else {
                          $sgconf[] = "\t " . $acl[FLD_NAME] . " { ";

                          $sgconf[] = "\t\t pass " . $acl[FLD_DESTINATIONNAME];

                          if ($acl[FLD_REDIRECT] && is_url($acl[FLD_REDIRECT]))
                              $sgconf[] = "\t\t redirect " . sg_redirector_base_url($acl[FLD_REDIRECT]); # $redirect_base_url . rawurlencode($acl[FLD_REDIRECT]);
                          if ($acl[FLD_REWRITENAME])
                              $sgconf[] = "\t\t rewrite " . $acl[FLD_REWRITENAME];

                          $sgconf[] = "\t }";
                      }
                      $sgconf[] = "";
                 }
                 $log_entr_added .= " {$acl[FLD_NAME]};";
             }
             // log
             if (!empty($log_entr_added))
                  sg_addlog("sg_build_config: added: \n $log_entr_added \n");
             else sg_addlog("sg_build_config: added: Nothing.");
         }

         // --- Default ---
         $def = $squidguard_config[FLD_DEFAULT];
         sg_addlog("sg_build_config: add Default");
         if ($def) {
             // delete blacklist entries from 'pass' if blacklist disabled
             if ($squidguard_config[FLD_BLACKLISTENABLED] !== 'on') {
                 $tarray = explode(" ", $def[FLD_DESTINATIONNAME]);
                 $varray = explode(" ", $def[FLD_OVERDESTINATIONNAME]);
                 foreach($entry_blacklist as $entry) {
                         $tk = array_search($entry , $tarray);
                         if ($tk !== false) unset ($tarray[$tk]);

                         $tk = array_search("!$entry" , $tarray);
                         if ($tk !== false) unset ($tarray[$tk]);

                         $tk = array_search($entry , $varray);
                         if ($tk !== false) unset ($varray[$tk]);

                         $tk = array_search("!$entry" , $varray);
                         if ($tk !== false) unset ($varray[$tk]);
                 }
                 $def[FLD_DESTINATIONNAME]     = implode (" ", $tarray);
                 $def[FLD_OVERDESTINATIONNAME] = implode (" ", $varray);
             }

             if ($def[FLD_TIMENAME]) {
                 // ontime
                 $sgconf[] = "\t default within " . $def[FLD_TIMENAME] . " { ";
                 $sgconf[] = "\t\t pass " . $def[FLD_DESTINATIONNAME];
                 if ($def[FLD_REDIRECT] && is_url($def[FLD_REDIRECT]))
                      $sgconf[] = "\t\t redirect " . sg_redirector_base_url($def[FLD_REDIRECT]); # $redirect_base_url . rawurlencode($def[FLD_REDIRECT]);
                 else $sgconf[] = "\t\t redirect " . sg_redirector_base_url(''); # $redirect_base_url;
                 // overtime
                 $sgconf[] = "\t } else {";
                 $sgconf[] = "\t\t pass " . $def[FLD_OVERDESTINATIONNAME];
                 if ($def[FLD_OVERREDIRECT] && is_url($def[FLD_OVERREDIRECT])) {
                     $sgconf[] = "\t\t redirect " . sg_redirector_base_url($def[FLD_OVERREDIRECT]); # $redirect_base_url . rawurlencode($def[FLD_OVERREDIRECT]);
                 }
                 else $sgconf[] = "\t\t redirect " .  sg_redirector_base_url(''); # $redirect_base_url;
                 $sgconf[] = "\t }";
             } else {
                 // without time
                 $sgconf[] = "\t default { ";
                 $sgconf[] = "\t\t pass " . $def[FLD_DESTINATIONNAME];
                 if ($def[FLD_REDIRECT] && is_url($def[FLD_REDIRECT])) {
                      $sgconf[] = "\t\t redirect " .  sg_redirector_base_url($def[FLD_REDIRECT]); # $redirect_base_url . rawurlencode($def[FLD_REDIRECT]);
                 }
                 else $sgconf[] = "\t\t redirect " . sg_redirector_base_url(''); # $redirect_base_url;
                 $sgconf[] = "\t }";
             }
         } // if def
         else {
               sg_addlog("sg_build_config: error - ACL 'default' is empty, use as default 'block all'.");
               $sgconf[] = "\t default { ";
               $sgconf[] = "\t\t pass none";
               $sgconf[] = "\t\t redirect " . sg_redirector_base_url(''); # $redirect_base_url;
               $sgconf[] = "\t }";
         }

         // --- ACL end ---
         $sgconf[] = "}";

         return $sgconf;
}

// ------------------------------------------------------------
// sg_check_config_data
// ------------------------------------------------------------
function sg_check_config_data () {
         global $squidguard_config;
         $check_log = array();
         $times = array();
         $sources = array();
         $destinations = array();
         $rewrites = array();
         $acls = array();

         // --- Times ---
         if ($squidguard_config[FLD_TIMES]) {
             foreach($squidguard_config[FLD_TIMES][FLD_ITEM] as $tm) {
                 // check name as unique and name format
                 $tm_name = $tm[FLD_NAME];
                 $s = check_name($tm_name);
                 if ($s)
                     $check_log[] = "TIME '$tm_name' error: $s";

                 $times[] = $tm_name;
                 $key_tm = array_count_values($times);
                 if ($key_tm[$tm_name] > 1)
                     $check_log[] = "TIME '$tm_name' error: duplicate time name '$tm_name'";

                 // check time items format
             }
         }

         // --- Sources ---
         if ($squidguard_config[FLD_SOURCES]) {
             foreach($squidguard_config[FLD_SOURCES][FLD_ITEM] as $src) {
                 // check name as unique and name format
                 $src_name = $src[FLD_NAME];
                 $s = check_name($src_name);
                 if ($s)
                     $check_log[] = "SOURCE '$src_name'error: $s";

                 $sources[] = $src_name;
                 $key_src = array_count_values($sources);
                 if ($key_src[$src_name] > 1)
                     $check_log[] = "SOURCE '$src_name' error: duplicate source name '$src_name'";

                 // check IP's
             }
         }

         // --- Destinations ---
         if ($squidguard_config[FLD_DESTINATIONS]) {
             foreach($squidguard_config[FLD_DESTINATIONS][FLD_ITEM] as $dst) {
                 // check name as unique and name format
                 $dst_name = $dst[FLD_NAME];
                 $s = check_name($dst_name);
                 if ($s)
                     $check_log[] = "DESTINATION '$dst_name' error: $s";

                 $destinations[] = $dst_name;
                 $key_dst = array_count_values($destinations);
                 if ($key_dst[$dst_name] > 1)
                     $check_log[] = "DESTINATION '$dst_name' error: duplicate destination name '$dst_name'";

                 // check urls
                 // check domains
                 // check expressions
                 // check redirection url
             }
         }

         // --- Blacklist ---
         $blk_entries_file = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_BLK_ENTRIES;
         if (file_exists($blk_entries_file)) {
             $blk_entr = explode("\n", file_get_contents($blk_entries_file));
             foreach($blk_entr as $entr) {
                     if ($entr) {
                         $destinations[] = $entr;
                         // check entry for exists
                         $dbfile = $squidguard_config[FLD_DBHOME] . "/$entr";
                         if (!file_exists($dbfile))
                             $check_log[] = "BLACKLIST '$entr' error: file '$dbfile' not found";
                     }
             }
         }

         // --- Rewrites ---
         if ($squidguard_config[FLD_REWRITES]) {
             foreach($squidguard_config[FLD_REWRITES][FLD_ITEM] as $rw) {
                 // check check name as unique and name format
                 $rw_name = $rw[FLD_NAME];
                 $s = check_name($dst_name);
                 if ($s)
                     $check_log[] = "REWRITE '$rw_name' error: $s";

                 $rewrites[] = $rw_name;
                 $key_rw = array_count_values($rewrites);
                 if ($key_rw[$rw_name] > 1)
                     $check_log[] = "REWRITE '$rw_name' error: duplicate rewrite name '$rw_name'";
             }
         }

         $key_times = array_count_values($times);
         $key_sources = array_count_values($sources);
         $key_destinations = array_count_values($destinations);
         $key_rewrites = array_count_values($rewrites);

         // --- ACLs ---
         if ($squidguard_config[FLD_ACLS]) {
             $acls = array();
             foreach($squidguard_config[FLD_ACLS][FLD_ITEM] as $acl) {
                 // skip disabled acl
                 if ($acls[FLD_DISABLED]) continue;

                 $acl_name = $acl[FLD_NAME];

                 // check acl name for unique and exists (as source items)
                 if ($acl_name and !$key_sources[$acl_name])
                     $check_log[] = "ACL '$acl_name' error: acl name '$acl_name' not found";

                 $acls[] = $acl_name;
                 $key_acls = array_count_values($acls);
                 if ($key_acls[$acl_name] > 1)
                     $check_log[] = "ACL '$acl_name' error: duplicate acl name '$acl_name'";

                 // check time
                 $time = $acl[FLD_TIMENAME];
                 if ($time and !$key_times[$time]) // time name must exists
                     $check_log[] = "ACL '$acl_name' error: time name '$time' not found";

                 // check destinations
                 if ($acl[FLD_DESTINATIONNAME]) {
                     $acldest = str_replace("!", "", $acl[FLD_DESTINATIONNAME]);
                     $acldest = explode(" ", $acldest);
                     $key_acldest = array_count_values($acldest);
                     foreach($acldest as $adest) {
                         // check duplicates destinations in acl
                         if ($key_acldest[$adest] > 1)
                             $check_log[] = "ACL '$acl_name' error: duplicate destination name '$adest'. Any destination must included once.";
                         // check destinations for exists
                         if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                             $check_log[] = "ACL '$acl_name' error: destination name '$adest' not found";
                     }
                 } else {
                         $check_log[] = "ACL '$acl_name' error: ontime pass list is empty.";
                 }

                 // check overtime destinations
                 if ($time) {
                     if ($acl[FLD_OVERDESTINATIONNAME]) {
                         $acloverdest = str_replace("!", "", $acl[FLD_OVERDESTINATIONNAME]);
                         $acloverdest = explode(" ", $acloverdest);
                         $key_acloverdest = array_count_values($acloverdest);
                         foreach($acloverdest as $adest) {
                             // check duplicates destinations in acl
                             if ($key_acloverdest[$adest] > 1)
                                 $check_log[] = "ACL '$acl_name' error: duplicate overtime destination name '$adest'. Any destination must included once.";
                             // check destinations for exists
                             if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                                 $check_log[] = "ACL '$acl_name' error: overtime destination name '$adest' not found";
                         }
                     } else {
                         $check_log[] = "ACL '$acl_name' error: overtime pass list is empty.";
                     }
                 }

                 // check rewrite
                 $rew = $acl[FLD_REWRITENAME];
                 if ($rew and !$key_rewrites[$rew])
                     $check_log[] = "ACL '$acl_name' error: rewrite name '$rew' not found";

                 // check overtime rewrite
                 $overrew = $acl[FLD_OVERREWRITENAME];
                 if ($time and $overrew and !$key_rewrites[$overrew])
                     $check_log[] = "ACL '$acl_name' error: overtime rewrite name '$overrew' not found";

                // check redirect
                $redir = $acl[FLD_REDIRECT];
                $overredir = $acl[FLD_OVERREDIRECT];
             }
         }


         // --- Default ---
         if ($squidguard_config[FLD_ACLS]) {
             $def = $squidguard_config[FLD_DEFAULT];

             // check time
             $time = $def[FLD_TIMENAME];
             if ($time and !$key_times[$time]) // time name must exists
                 $check_log[] = "ACL 'default' error: time name '$time' not found";

             // check destinations
             if ($def[FLD_DESTINATIONNAME]) {
                 $defdest = str_replace("!", "", $def[FLD_DESTINATIONNAME]);
                 $defdest = explode(" ", $defdest);
                 $key_defdest = array_count_values($defdest);
                 foreach($defdest as $adest) {
                     // check duplicates destinations in acl
                     if ($key_defdest[$adest] > 1)
                         $check_log[] = "ACL 'default' error: duplicate destination name '$adest'. Any destination must included once.";
                     // check destinations for exists
                     if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                         $check_log[] = "ACL 'default' error: destination name '$adest' not found";
                 }
             } else {
                     $check_log[] = "ACL 'default' error: ontime pass list is empty.";
             }

             // check overtime destinations
             if ($time) {
                 if ($def[FLD_OVERDESTINATIONNAME]) {
                     $defoverdest = str_replace("!", "", $def[FLD_OVERDESTINATIONNAME]);
                     $defoverdest = explode(" ", $defoverdest);
                     $key_defoverdest = array_count_values($defoverdest);
                     foreach($defoverdest as $adest) {
                         // check duplicates destinations in acl
                         if ($key_defoverdest[$adest] > 1)
                             $check_log[] = "ACL 'default' error: duplicate overtime destination name '$adest'. Any destination must included once.";
                         // check destinations for exists
                         if ($adest and ($adest != 'all') and ($adest != 'none') and !$key_destinations[$adest])
                             $check_log[] = "ACL 'default' error: overtime destination name '$adest' not found";
                     }
                 } else {
                     $check_log[] = "ACL 'default' error: overtime pass list is empty.";
                 }
             }

             // check rewrite
             $rew = $def[FLD_REWRITENAME];
             if ($rew and !$key_rewrites[$rew])
                 $check_log[] = "ACL 'default' error: rewrite name '$rew' not found";

             // check overtime rewrite
             $overrew = $def[FLD_OVERREWRITENAME];
             if ($time and $overrew and !$key_rewrites[$overrew])
                 $check_log[] = "ACL 'default' error: overtime rewrite name '$overrew' not found";

             // check redirect
             $redir = $def[FLD_REDIRECT];
             $overredir = $def[FLD_OVERREDIRECT];
         }

         return implode("\n", $check_log);
}

// =============================================================================
// Blacklist
// =============================================================================
// sg_reconfigure_blacklist($source_filename, $opt)
//       $source_filename - file name or url
//       $opt - option:
//              '' or 'local'   - update from local file
//              'url'           - update from url
// -----------------------------------------------------------------------------
function sg_reconfigure_blacklist($source_filename, $opt = '') {
         global $squidguard_config;
         $sf = trim($source_filename);
         $sf_contents = '';

         sg_addlog("sg_reconfigure_blacklist: start ");

         // 1. check system
         sg_check_system();

         // 2. upload
         sg_addlog("sg_reconfigure_blacklist: begin upload from '$sf'.", 1);
         if ($sf[0] === "/") { // local file - example '/tmp/blacklists.tar'
             if (file_exists($sf)) {
                 $sf_contents = file_get_contents($sf);
                 sg_addlog("sg_reconfigure_blacklist: get file '$sf'.", 1);
             } else {
                 sg_addlog("sg_reconfigure_blacklist: ERROR: get file '$sf', file not found.", 2);
                 return;
             }
         }
         // url
         else {
             sg_addlog("sg_reconfigure_blacklist: get url '$sf'.", 1);
             $sf_contents = sg_uploadfile_from_url($sf, BLK_LOCALFILE, $opt);
         }

         // 3. update
         if (empty($sf_contents)) {
             sg_addlog("sg_reconfigure_blacklist: ERROR: file content '$sf'.", 2);
             return;
         }

         // 4. save black list archive content to local file
         file_put_contents(BLK_LOCALFILE, $sf_contents);

         // 5. update blacklist
         sg_update_blacklist(BLK_LOCALFILE);

         // 6. rebuild db
#         sg_full_rebuild_db();

         sg_addlog("sg_reconfigure_blacklist: end");
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

function sg_update_blacklist($from_file) {
         global $squidguard_config;
         $dbhome  = SQUIDGUARD_DBHOME_DEF;
         $workdir = SQUIDGUARD_WORKDIR_DEF;
         $tmp_unpack_dir = SQUIDGUARD_TMP . SQUIDGUARD_BL_UNPACK;
         $arc_db_dir     = SQUIDGUARD_VAR . SG_BLK_ARC;

         sg_addlog("sg_update_blacklist: begin.");

         if (file_exists($from_file)) {

             // check work and DB dir's
             if (file_exists($squidguard_config[FLD_DBHOME]))  $dbhome = $squidguard_config[FLD_DBHOME];
             if (file_exists($squidguard_config[FLD_WORKDIR])) $workdir = $squidguard_config[FLD_WORKDIR];
             // delete old tmp dir's
             if (file_exists($tmp_unpack_dir)) mwexec("rm -R  . $tmp_unpack_dir");
             if (file_exists($arc_db_dir))     mwexec("rm -R  . $arc_db_dir");
             // create new tmp/arc dir's
             mwexec("mkdir -p -m 0750 $tmp_unpack_dir");
             mwexec("mkdir -p -m 0750 $arc_db_dir");

             // unpack archive
             mwexec("tar zxvf $from_file -C $tmp_unpack_dir");
             set_file_access($tmp_unpack_dir, OWNER_NAME, 0750);
             sg_addlog("sg_update_blacklist: Unpack uploaded file '$from_file' -> '$tmp_unpack_dir'.", 1);

             // 2. copy blacklist to squidGuard base & create entries list
             if (file_exists($tmp_unpack_dir)) {
                 $blk_items = array();
                 $blk_list  = array();

                 // scan blacklist items
                 scan_blacklist_cat($tmp_unpack_dir, "blk", & $blk_items);

                 // modify blacklist catalog structure to 'one level'
                 foreach ($blk_items as $key => $val) {
                          $current_dbpath = "$arc_db_dir/$key";
                          if (count($val)) {
                              // make blk_list for config file
                              $blk_list[$key] = $key;

                              // delete '$current_dbpath' for correct moving
                              // need moving $val['path'] to $current_dbpath
                              // if $current_dbpath exists,
                              // then $val['path'] will created as subdir - !it's worng!
                              if (file_exists($current_dbpath))
                                  mwexec("rm -R $current_dbpath");
                              mwexec("mv -f {$val['path']}/ $current_dbpath");
                              sg_addlog("sg_update_blacklist: Move {$val['path']}/ -> $current_dbpath.", 1);
                          }
                 }
                 // copy unrebuilded blacklist to work DB
                 mwexec("cp -R $arc_db_dir/ $dbhome");

                 // create entries list
                 if (count($blk_items)) {
                      $blklist_file = SQUIDGUARD_VAR . SQUIDGUARD_BLK_ENTRIES;

                      // save to temp DB
                      file_put_contents($blklist_file, implode("\n", array_keys($blk_items)));
                      set_file_access  ($blklist_file, OWNER_NAME, 0750);

                      // save copy to squidGuard config dir
                      $blklist_file = "{$squidguard_config[FLD_WORKDIR]}/" . SQUIDGUARD_BLK_ENTRIES;
                      file_put_contents($blklist_file, implode("\n", array_keys($blk_items)));
                      set_file_access  ($blklist_file, OWNER_NAME, 0750);
                      sg_addlog("sg_update_blacklist: create entries '$blklist_file'", 1);
                 }
                 set_file_access($arc_db_dir, OWNER_NAME, 0750);

                 // make rebuild config; include all found dest items
                 $conf_path = "/tmp/squidGuard_rebuild_blk.conf";
                 $conf = sg_create_rebuild_config($arc_db_dir, $blk_list);

                 // copy to temp DB
                 file_put_contents($conf_path, $conf);
                 set_file_access($conf_path, OWNER_NAME, 0750);
                 sg_addlog("sg_update_blacklist: Create temporary config '$conf_path'.", 1);

                 // *** SH script ***
                 $sh_scr = Array();
                 $sh_scr[] = "#!/bin/sh";
                 $sh_scr[] = "cd $arc_db_dir";
                 $cmd = $squidguard_config[FLD_BINPATH] . "/squidGuard -c $conf_path -C all";
                 $sh_scr[] = $cmd;
                 $sh_scr[] = "wait"; // wait while SG rebuild DB

                 // copy temp db to '/var/db/squidGuard (-R - recursive; -p - copy access rights)
                 // '$bl_temp_dbhome/' - slash in end of path - copy only dir content (not self dir)
                 $sh_scr[] = "cp -R $arc_db_dir/ $dbhome";

                 // set DB owner and right access
#                 $sh_scr[] = "chown -R -v " . OWNER_NAME . " $dbhome";

                 // restart squid for changes to take effects
#                 $sh_scr[] = "/usr/local/sbin/squid -k reconfigure";

                 // store & exec sh
                 $sh_scr = implode("\n", $sh_scr);
                 $shfile =  DB_REBUILD_SH . "_blk";
                 file_put_contents($shfile, $sh_scr);
                 set_file_access($shfile, OWNER_NAME, 0750);
                 mwexec_bg($shfile);
                 sg_addlog("sg_update_blacklist: started SH script '$shfile'.", 1);

                 sg_remove_unused_db_entries();

                 // clearing temp
#                 mwexec("rm -R $bl_temp");
             }
#             set_file_access($squidguard_config[FLD_DBHOME], OWNER_NAME, 0755);
#             sg_full_rebuild_db();
         }
         sg_addlog("sg_update_blacklist: end");
}


// -----------------------------------------------------------------------------
// sg_blacklist_rebuild_DB  - update blacklist from file
// -----------------------------------------------------------------------------
function sg_entries_blacklist() {
         global $squidguard_config;
         $contents = '';
         $fl = SQUIDGUARD_WORKDIR_DEF . SQUIDGUARD_BLK_ENTRIES;
         if (file_exists($squidguard_config[FLD_WORKDIR]))
             $fl = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_BLK_ENTRIES;

         if (file_exists($fl)) {
             $contents = file_get_contents($fl);
             $contents = explode("\n", $contents);
         }
         return $contents;
}
// -----------------------------------------------------------------------------
// sg_blacklist_rebuild_db - rebuild current Blacklist DB (default: '/var/db/squidGuard')
// -----------------------------------------------------------------------------
function sg_blacklist_rebuild_db() { // $shtag, $rdb_dbhome, $rdb_workdir) {
         global $squidguard_config;
         $dst_list = array();
         $dbhome  = $squidguard_config[FLD_DBHOME];
         $workdir = $squidguard_config[FLD_WORKDIR];

         // current dbhome and work dir's
# disabled in current ver
#         if (!empty($rdb_dbhome))  $dbhome  = $rdb_dbhome;
#         if (!empty($rdb_workdir)) $workdir = $rdb_workdir;
         sg_addlog("sg_blacklist_rebuild_db: Begin with path '$dbhome'.");

         // make dest list
         $blklist_file = "$workdir/" . SQUIDGUARD_BLK_ENTRIES;
         if (file_exists($blklist_file)) {
             $blklist = explode("\n", file_get_contents($blklist_file));
             if (is_array($blklist))
                 foreach($blklist as $bl)
                         $dst_list[$bl] = $bl;
         }

         // rebuild user db ('/var/db/squidGuard')
         sg_rebuild_db("_blkdb", $dbhome, $dst_list);
         sg_addlog("sg_blacklist_rebuild_db: End.");
}

# ========================== UTILS =============================================
# sg_uploadfile_from_url
# upload file and put them to $destination_file
# return = upload content
# ------------------------------------------------------------------------------
function sg_uploadfile_from_url($url_file, $destination_file, $proxy = '') {
         // open destination file
         sg_addlog("sg_uploadfile_from_url: begin url'$url_file' proxy'$proxy'");

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
         else sg_addlog("sg_uploadfile_from_url: error upload file");


         // for test
         file_put_contents(BLK_LOCALFILE, $result);

         sg_addlog("sg_uploadfile_from_url: end");
         return $result;
}

# ==============================================================================
# self utils
# ==============================================================================
# Set file access
# ------------------------------------------------------------------------------
function set_file_access($dir, $owner, $mod) {

        if (!file_exists($dir)) return;

        chown($dir, $owner);
        chgrp($dir, $owner);
        chmod($dir, $mod);

        // if $dir - directory - set subitem's access
        if (is_dir($dir)) {
               $hd = opendir($dir);
               while (($item = readdir($hd)) !== false) {
                   if (($item != ".") && ($item != "..")) {
                       $path = "$dir/$item";
                       if (is_dir($path))
                          set_file_access($path, $owner, $mod);
                       else {
                           chown($path, $owner);
                           chgrp($path, $owner);
                           chmod($path, $mod);
                       }
                   }
               }
        }
}
# ------------------------------------------------------------------------------
# scan_dir - build files listing for $dir
# ------------------------------------------------------------------------------
function scan_dir($dir) {
    $files = array();
    if (file_exists($dir)) {
        $dh = opendir($dir);
        while (false !== ($filename = readdir($dh))) {
               // skip '.' and '..' names
               if (($filename !== '.') and ($filename !== '..')) $files[] = $filename;
        }
        sort($files);
    }
    return $files;
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
function scan_blacklist_cat($curdir, $key_name, $cat_array) {

    if (file_exists($curdir) and is_dir($curdir)) {
        $blk_entry = array();
        $files = scan_dir($curdir);

        foreach($files as $fls) {
            $fls_file = "$curdir/$fls";

            if (($fls != ".") and ($fls != "..")) {
                if (is_file($fls_file)) {

                     // add files path
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

                     // recursive call
                     scan_blacklist_cat($fls_file, $fls_key, & $cat_array);
                 }
            }
        }

        if (count($blk_entry))
            $cat_array[$key_name] = $blk_entry;
    }
}

# ------------------------------------------------------------------------------
# is_url - check url an err_codes
# ------------------------------------------------------------------------------
function is_url($url) {
    if (empty($url))                return false;
    if (eregi("^http://", $url))    return true;
    if (eregi("^https://", $url))   return true;
    if (strstr("blank", $url))      return true;
    if (strstr("blank_img", $url))  return true;
    if (eregi("^((30[1235]{1})|(40[0-9]{1})|(41[0-7]{1})|(50[0-5]{1}))", $url)) return true; // http error code 30x, 4xx, 50x.
    return false;
}
# ------------------------------------------------------------------------------
# check name
# ------------------------------------------------------------------------------
function check_name ($name) {
        $err = '';
        $val = trim($name);

        if ((strlen($val) < 2) || (strlen($val) > 16))
            $err .= " Size of name must be between [2..16].";

        // All symbols must be [a-zA-Z_0-9\-] First symbol = letter.
        if (!eregi("^([a-zA-Z]{1})([a-zA-Z_0-9\-]+)$", $val))
            $err .= " Invalid name $name. Valid name symbols: ['a-Z', '_', '0-9', '-']. First symbol must be a letter.";

        return $err;
}

?>