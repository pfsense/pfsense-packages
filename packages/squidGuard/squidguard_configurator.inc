<?php
# ------------------------------------------------------------------------------
# squidguard_configurator.inc
# (C) 2006, 2007 Serg Dvoriancev
/*      Redistribution and use in source and binary forms, with or without
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
# Directories:
# work path       - $workdir
# log path        - $workdir + $logdir
# ------------------------------------------------------------------------------
# Functions:
#           sg_init($init_xml)
#           sg_load_configxml($filename)
#           sg_save_configxml($filename)
#           sg_reconfigure()
#           sg_reconfigure_blacklist($source_filename, $opt)
#               $source_filename - file name or url
#               $opt - option:
#                   '' or 'local'   - update from local file (example: '/tmp/blacklist.tar')
#                   'url'           - update from url
# ------------------------------------------------------------------------------
# Config XML structure:
# ------------------------------------------------------------------------------
/*
<?xml version="1.0"?>
<squidGuard>
        <logdir/>
        <dbhome/>
        <workdir/>
        <enabled/>
        <blacklist_enabled>
        <sources>
                <item>
                        <name/>
                        <ip/>
                        <log>on</log>
                        <description/>
                </item>
                <item>
                      ...
                </item>
        </sources>
        <destinations>
                <item>
                        <name/>
                        <urls/>
                        <domains/>
                        <expressions/>
                        <redirect/>
                        <description/>
                        <log>on</log>
                </item>
                <item>
                      ...
                </item>
        </destinations>
        <rewrites>
                <item>
                        <name/>
                        <description/>
                        <log>on</log>
                        <item>
                                <targeturl/>
                                <replaceto/>
                        </item>
                        <item>
                              ...
                        </item>
                </item>
                <item>
                      ...
                </item>
        </rewrites>
        <times>
                <item>
                        <name/>
                        <description/>
                        <item>
                                <timetype/>
                                <timedays/>
                                <daterange/>
                                <sg_timerange/>
                        </item>
                        <item>
                              ...
                        </item>
                </item>
                <item>
                      ...
                </item>
        </times>
        <acls>
                <item>
                        <name/>
                        <description/>
                        <disabled/>
                        <timename/>
                        <redirect/>
                        <rewritename/>
                        <overredirect/>
                        <overrewritename/>
                        <destname/>
                        <overdestname/>
                </item>
                <item>
                      ...
                </item>
        </acls>
        <default>
                <name/>
                <description/>
                <disabled/>
                <timename/>
                <redirect/>
                <rewritename/>
                <overredirect/>
                <overrewritename/>
                <destname/>
                <overdestname/>
        </default>
</squidGuard>
*/
require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');

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
define('REDIRECTOR_PROCESS_COUNT', '3');

# ------------------------------------------------------------------------------
# squidguard config options
# ------------------------------------------------------------------------------
# define default redirection url (redirector get this url for all blocked url's)
# * !ATTENTION! this url must be exists; IF url not exist, redirector will't block
#               (returned to squid some url, what blocked)
#               this may use '301:' or '302:' value (only)
#define('REDIRECT_BASE_URL', '302:');
define('REDIRECT_BASE_URL', 'http://127.0.0.1/sgerror.php');
define('REDIRECT_TRANSPARENT_BASE_URL', '/sgerror.php');

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
define('SQUIDGUARD_BLK_ENTRIES',       '/blacklist.files');
define('BLACKLIST_ARCHIVE',            '/blacklists.tar');
# ? may be not used ?
define('SQUIDGUARD_CONFBASE_DEF',      '/usr/local/etc/squid');
define('SQUIDGUARD_LOGDIR_DEF',        '/tmp');
define('SQUIDGUARD_WORKDIR_DEF',       '/usr/local/etc/squidGuard');
define('SQUIDGUARD_BINPATH_DEF',       '/usr/local/bin');
define('SQUIDGUARD_DBHOME_DEF',        '/var/db/squidGuard');
define('BLK_LOCALFILE',                '/tmp/sg_blacklists.tar');

# ------------------------------------------------------------------------------
// debug options
define('DEBUG_UPDATE_SQUID_CONF',        'true');
define('DEBUG_UPDATE_SQUIDGUARD_DB',     'true');
define('DEBUG_MAKE_SQUIDGUARD_CONFIG',   'true');

// options
define('SQUIDGUARD_LOG_MAXCOUNT',        1000); // max log lines

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

# ------------------------------------------------------------------------------
# std_blacklist_get_description - black list std names description
# ------------------------------------------------------------------------------
function std_blacklist_get_description() {
    $dst_std = array();
    $dst_std[FLT_AD]         = 'Reclama & banners filter';
    $dst_std[FLT_AGGRESSIVE] = 'Agressive content sites filter';
    $dst_std[FLT_AUDIOVIDEO] = 'Audio and Video sites filter';
    $dst_std[FLT_DRUGGS]     = 'Druggs filter';
    $dst_std[FLT_GAMBLING]   = 'Games sites filter';
    $dst_std[FLT_HACKING]    = 'Hacking sites filter';
    $dst_std[FLT_MAIL]       = 'Mail sites filter';
    $dst_std[FLT_PORN]       = 'Porno sites filter';
    $dst_std[FLT_PROXY]      = 'Proxy sites filter';
    $dst_std[FLT_VIOLENCE]   = 'Violence content sites filter';
    $dst_std[FLT_WAREZ]      = 'Wares, soft, downloads sites filter';
    return $dst_std;
}

# ==============================================================================
# SquidGuard Configurator
# ==============================================================================
// squidGuard config array
$squidguard_config = array();
// default init
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

// transparent mode
define('FLD_SQUID_TRANSPARENT_MODE',   'squid_transparent_mode');
define('FLD_CURRENT_LAN_IP',           'current_lan_ip');

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
         global $squidguard_config;

         sg_addlog("squid_reconfigure: begin");

         // 1. update squid config
         $opt = '';
         $squid_conf_file = SQUID_CONFIGFILE;
         $redirector_path = $squidguard_config[FLD_BINPATH] . '/squidGuard';
         $redirector_conf = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_CONFIGFILE;

         // update squid.conf file
         if (file_exists($squid_conf_file)) {
             sg_addlog("squid_reconfigure: config file '$squid_conf_file'");
             $conf = file_get_contents($squid_conf_file);

             // remove old redirector options from 'squid.conf'
             sg_addlog("squid_reconfigure: remove old redirector options from 'squid.conf'");
             $conf = explode("\n", $conf);
             for($i=0; $i<count($conf); $i++) {
                 $s = trim($conf[$i]);
                 if (strpos($s, REDIRECTOR_OPTIONS_REM) === 0) $conf[$i] = '';
                 if (strpos($s, REDIRECTOR_PROGRAM_OPT) === 0) $conf[$i] = '';
                 if (strpos($s, REDIRECT_BYPASS_OPT)    === 0) $conf[$i] = '';
                 if (strpos($s, REDIRECT_CHILDREN_OPT)  === 0) $conf[$i] = '';
             }
             $conf = implode("\n", $conf);
             $conf = rtrim($conf);

             // if squidGuard enabled - add new options to squid config
             if (empty($remove_only) && ($squidguard_config[FLD_ENABLED] === 'on')) {
                 sg_addlog("squid_reconfigure: add new redirector options to 'squid.conf'");
                 // add redirector options to 'squid.conf'
                 $conf .= "\n";
                 $conf .= "\n" . REDIRECTOR_OPTIONS_REM;
                 $conf .= "\n" . REDIRECTOR_PROGRAM_OPT . " $redirector_path -c $redirector_conf";
                 $conf .= "\n" . REDIRECT_BYPASS_OPT    . ' on';
                 $conf .= "\n" . REDIRECT_CHILDREN_OPT  . " " . REDIRECTOR_PROCESS_COUNT;
             }
             $conf .= "\n";
             file_put_contents($squid_conf_file, $conf);
         } else
             sg_addlog("squid_reconfigure: error config file '$squid_conf_file' not found.");


         // 2. restart squid - squid automaticly restart redirector too
         if (is_service_running('squid'))
             mwexec("/usr/local/sbin/squid -k reconfigure");

#        // 2.1. another method
#        if (is_service_running('squid'))
#            mwexec('killall -HUP squid');
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
// ------------------------------------------------------------
// sg_reconfigure_user_db
//  - reconfigure(update) db user entries
// ------------------------------------------------------------
function sg_reconfigure_user_db() {
         global $squidguard_config;
         $dbhome = $squidguard_config[FLD_DBHOME];

         sg_addlog("sg_reconfigure_user_db: begin");

         if (!file_exists($dbhome)) {
             sg_addlog("sg_reconfigure_user_db: error - path not found $dbhome");
             return;
         }

         // update destinations to db
         $dests = $squidguard_config[FLD_DESTINATIONS];
         if(!empty($dests)){
             $dst_names = array();

             foreach($dests[FLD_ITEM] as $dst) {
                     $path = "$dbhome/" . $dst[FLD_NAME];
                     $dst_names[] = $path;

                     // 1. check destination catalog
                     if (!file_exists($path)) {
                          if (!mkdir ($path, 0755)) {
                                 sg_addlog("sg_reconfigure_user_db: error create dir $path");
                                 return;
                          } else sg_addlog("Create dir $path");
                     }

                     // 2. build domains file
                     $domains = $dst[FLD_DOMAINS];
                     if (!empty($domains)) {
                          $content = '';
                          $content = str_replace(" ", "\n", $domains);
                          $content = trim($content);
                          file_put_contents($path . '/domains', $content);
                          sg_addlog("sg_reconfigure_user_db: add domains '$domains'");
                     }
                     unset($domains);

                     // 3. build urls file
                     $urls = $dst[FLD_URLS];
                     if (!empty($urls)) {
                          $content = '';
                          $content = str_replace(" ", "\n", $urls);
                          $content = trim($content);
                          file_put_contents($path . '/urls', $content);
                          sg_addlog("sg_reconfigure_user_db: add urls '$content'");
                     }
                     unset($urls);

                     // 4. build expression file
                     $expr = $dst[FLD_EXPRESSIONS];
                     if (!empty($expr)) {
                          $content = str_replace("|", " ", $expr);
                          $content = trim($content); // delete first and last unnecessary '|' symbols
                          $content = str_replace(" ", "|", $content);
                          file_put_contents($path . '/expressions', $content);
                          sg_addlog("sg_reconfigure_user_db: add expressions '$content'");
                     }
                     unset($expr);
             }

             // 4. recursive set files access
             set_file_access($dbhome, OWNER_NAME, 0755);

             // 5. rebuild user db
             //$dst_names
             foreach($dst_names as $dname)
                     sg_full_rebuild_db($dname);

         } else
             sg_addlog("sg_reconfigure_user_db: stopped - destinations list empty");

         // 6. remove unused db entries
         sg_remove_unused_db_entries();

         sg_addlog("sg_reconfigure_user_db: end");
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

         sg_addlog("sg_remove_unused_db_entries: begin");

         // black list entries
         // * worked only with 'blacklist entries list file - else may be deleted black list entry
         if (file_exists($dbhome .  SQUIDGUARD_BLK_ENTRIES)) {
             $db_entries = explode("\n", file_get_contents($dbhome . SQUIDGUARD_BLK_ENTRIES));
             // user entries
             $dests = $squidguard_config[FLD_DESTINATIONS];
             foreach($dests[FLD_ITEM] as $dst) {
                     $db_entries[] = $dst[FLD_NAME];

                     $file_list = scan_dir($dbhome);
                     $file_for_del = array_diff($file_list, $db_entries);

                     foreach($file_for_del as $fd) {
                             $file_fd = $dbhome . "/" . $fd;
                             if (($fd != "") && ($fd != ".") && ($fd != "..")) {
                                  if (file_exists($file_fd)) {
                                      if (!mwexec("rm -R  . $file_fd"))
                                           sg_addlog("sg_remove_unused_db_entries: Delete $file_fd");
                                      else sg_addlog("sg_remove_unused_db_entries: Error delete $file_fd");
                                  } else   sg_addlog("sg_remove_unused_db_entries: File $file_fd not found");
                             }
                     }
             }
         }
         sg_addlog("sg_remove_unused_db_entries: end");
}

// ------------------------------------------------------------
// sg_full_rebuild_db
// squidguard inline options: -C - create db files; -u - update '.diff' files to db
// ------------------------------------------------------------
function sg_full_rebuild_db($dblist='') {
         global $squidguard_config;
         $sg_cfgfile = '';

         sg_addlog("sg_rebuild_db: begin with $dblist");

         if ($squidguard_config[FLD_WORKDIR])
              $sg_cfgfile = $squidguard_config[FLD_WORKDIR];
         else $sg_cfgfile = SQUIDGUARD_WORKDIR_DEF;
         $sg_cfgfile .= SQUIDGUARD_CONFIGFILE;

         // set files access
         set_file_access($squidguard_config[FLD_DBHOME], OWNER_NAME, 0755);

         // rebuild squidGuard DB (without waite)
         if (file_exists($sg_cfgfile)) {
             if (empty($dblist)) {
                 // full rebuild
                 $cmd = $squidguard_config[FLD_BINPATH] . '/squidGuard -c $sg_cfgfile -C all';
                 mwexec_bg($cmd);
                 sg_addlog("sg_rebuild_db: start full rebuild db");
             } else {
                 // partually rebuild
                 $cmd_domains = $squidguard_config[FLD_BINPATH] . "/squidGuard -c $sg_cfgfile -C $dblist/domains";
                 $cmd_urls = $squidguard_config[FLD_BINPATH] . "/squidGuard -c $sg_cfgfile -C $dblist/urls";
                 mwexec_bg($cmd_domains);
                 mwexec_bg($cmd_urls);
                 sg_addlog("sg_rebuild_db: start rebuild DB '$dblist'");
             }
             // update .diff files
#            mwexec_bg($squidguard_config[FLD_BINPATH] . '/squidGuard -c $sg_cfgfile -u')*/)
         } else {
             sg_addlog("sg_rebuild_db: error, config file '$sg_cfgfile' not found");
         }

         sg_addlog("sg_rebuild_db: end");
}
// ============================================================
// Log
// ============================================================
// ------------------------------------------------------------
// sg_addlog
// ------------------------------------------------------------
function sg_addlog($log) {
         global $squidguard_config;

         $logfile = '';
         $logfile = SQUIDGUARD_LOGDIR_DEF . SQUIDGUARDCONF_LOGFILE;
         $log_content = array();

         if (!empty($squidguard_config)) {
                // define logfile
                if (file_exists($squidguard_config[FLD_LOGDIR]))
                        $logfile = $squidguard_config[FLD_LOGDIR] . SQUIDGUARDCONF_LOGFILE;
         } else {
                $log_content[] = date("d.m.Y H:i:s") . ": " . "sg_addlog: Error, squidguard_config is empty";
         }

         $tmplog = '';
         if (file_exists($logfile))
             $tmplog = file_get_contents($logfile);
         $log_content = explode("\n", $tmplog);
         unset($tmplog);

         $log_content[] = date("d.m.Y H:i:s") . ": $log";
         while (count($log_content) > SQUIDGUARD_LOG_MAXCOUNT) array_shift($log_content);

         $tlog = implode("\n", $log_content);
         file_put_contents($logfile, $tlog);
#         file_put_contents("/tmp/_sg.log", $tmp_log);
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
# -------------------------------------------------------------
function sg_build_default_config() {
         global $squidguard_config;
         $sgconf = array();
         $redirect_base_url = REDIRECT_BASE_URL;

// TODO: need fix for transparentproxy

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
         $sgconf[] = "\t\t redirect " . $redirect_base_url;
         $sgconf[] = "\t }";
         $sgconf[] = "}";

         sg_addlog("sg_build_default_config: Created default configuration. All content will blocked.");
         return $sgconf;
}

// ------------------------------------------------------------
// sg_build_config
// ------------------------------------------------------------
function sg_build_config() {
         global $squidguard_config;
         $sgconf = array();
         $redirect_base_url = REDIRECT_BASE_URL;

         sg_addlog("sg_build_config: create squidGuard config");
         if(!is_array($squidguard_config)) {
            sg_addlog("sg_build_config: error configuration in squidguard_config");
            return sg_build_default_config();
         }

         // check configuration data
         sg_addlog("sg_build_config: check configuration data");
         $s = sg_check_config_data();
         if ($s) {
            sg_addlog("sg_build_config: error configuration data. It's all errors: \n$s");
            sg_addlog("sg_build_config: terminated.");
            return sg_build_default_config();
         }
         unset($s);

         // --- Header ---
         $sgconf[] = CONFIG_SG_HEADER;

         // Transparent redirector base url
         if (isset($squidguard_config[FLD_SQUID_TRANSPARENT_MODE]) and
             isset($squidguard_config[FLD_CURRENT_LAN_IP])) {
             $redirect_base_url = "http://" . $squidguard_config[FLD_CURRENT_LAN_IP] . REDIRECT_TRANSPARENT_BASE_URL;
             sg_addlog("sg_build_config: select LAN redirector base url ($redirect_base_url)");
         } else
             sg_addlog("sg_build_config: select localhost redirector base url ($redirect_base_url)");

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
         # Note! Blacklist must be added to config constantly. It's need for rebuild DB
         #
         $db_entries = sg_entries_blacklist();
         if (($squidguard_config[FLD_BLACKLISTENABLED] === 'on') and $db_entries) {
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
                         sg_addlog("sg_build_config: -- add '$ent' entry");
                     } else {
                         $sgconf[] = "\t# Config ERROR: Destination '$ent' not found in DB";
                         sg_addlog("sg_build_config: uncompleted or error '$ent' entry - disabled");
                     }
             }
         }

         // --- Destinations ---
         if ($squidguard_config[FLD_DESTINATIONS]) {
             sg_addlog("sg_build_config: add destinations");
             $sgconf[] = "";
#             $sgconf[] = "dest localhost { # fix localhost access problem on transparent proxy ";
#             $sgconf[] = "\t ip 127.0.0.1";
#             $sgconf[] = "}";
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
                     $sgconf[] = "\t redirect " . $redirect_base_url . "?url={$dst[FLD_REDIRECT]}";
                 if ($dst[FLD_LOG])
                     $sgconf[] = "\t log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                 $sgconf[] = "}";
             }
         }

         // --- Rewrites ---
         if ($squidguard_config[FLD_REWRITES]) {
             sg_addlog("sg_build_config: add rewrites");
             foreach($squidguard_config[FLD_REWRITES][FLD_ITEM] as $rew) {
                 $sgconf[] = "";
                 $sgconf[] = "rew " . $rew[FLD_NAME] . " {";
                 foreach ($rew[FLD_ITEM] as $rw)
                          $sgconf[] = "\t s@." . $rw[FLD_TARGETURL] . "@" . $rw[FLD_REPLACETO]."@";
                 if ($rew[FLD_LOG])
                     $sgconf[] = "\t log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                 $sgconf[] = "}";
             }
         }

         # ----------------------------------------
         $entry_blacklist = sg_entries_blacklist();

         // --- ACL ---
         $sgconf[] = "";
         $sgconf[] = "acl {";
         if ($squidguard_config[FLD_ACLS]) {
             sg_addlog("sg_build_config: add ACL");
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
                                   $sgconf[] = "\t\t redirect " . $redirect_user_url . "?url={$acl[FLD_REDIRECT]}";
                              else $sgconf[] = "\t\t redirect " . $redirect_user_url . "?msg=" . htmlspecialchars($acl[FLD_REDIRECT]);
                          }
                          if ($acl[FLD_REWRITENAME])
                              $sgconf[] = "\t\t rewrite " . $acl[FLD_REWRITENAME];

                          // overtime
                          $sgconf[] = "\t } else {";
                          $sgconf[] = "\t\t pass " . $acl[FLD_OVERDESTINATIONNAME];
                          if ($acl[FLD_OVERREDIRECT] && is_url($acl[FLD_OVERREDIRECT]))
                                   $sgconf[] = "\t\t redirect " . $redirect_base_url . "?url={$acl[FLD_OVERREDIRECT]}";
                          if ($acl[FLD_OVERREWRITENAME])
                              $sgconf[] = "\t\t rewrite " . $acl[FLD_OVERREWRITENAME];

                          $sgconf[] = "\t }";
                      } else {
                          $sgconf[] = "\t " . $acl[FLD_NAME] . " { ";

                          $sgconf[] = "\t\t pass " . $acl[FLD_DESTINATIONNAME];

                          if ($acl[FLD_REDIRECT] && is_url($acl[FLD_REDIRECT]))
                              $sgconf[] = "\t\t redirect " . $redirect_base_url . "?url={$acl[FLD_REDIRECT]}";
                          if ($acl[FLD_REWRITENAME])
                              $sgconf[] = "\t\t rewrite " . $acl[FLD_REWRITENAME];

                          $sgconf[] = "\t }";
                      }
                      $sgconf[] = "";
                 }
             }
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
                      $sgconf[] = "\t\t redirect " . $redirect_base_url . "?url={$def[FLD_REDIRECT]}";
                 else $sgconf[] = "\t\t redirect " . $redirect_base_url;
                 // overtime
                 $sgconf[] = "\t } else {";
                 $sgconf[] = "\t\t pass " . $def[FLD_OVERDESTINATIONNAME];
                 if ($def[FLD_OVERREDIRECT] && is_url($def[FLD_OVERREDIRECT])) {
                     $sgconf[] = "\t\t redirect " . $redirect_base_url . "?url={$def[FLD_OVERREDIRECT]}";
                 }
                 else $sgconf[] = "\t\t redirect " . $redirect_base_url;
                 $sgconf[] = "\t }";
             } else {
                 // without time
                 $sgconf[] = "\t default { ";
                 $sgconf[] = "\t\t pass " . $def[FLD_DESTINATIONNAME];
                 if ($def[FLD_REDIRECT] && is_url($def[FLD_REDIRECT])) {
                      $sgconf[] = "\t\t redirect " . $redirect_base_url . "?url={$def[FLD_REDIRECT]}";
                 }
                 else $sgconf[] = "\t\t redirect " . $redirect_base_url;
                 $sgconf[] = "\t }";
             }
         } // if def
         else {
               sg_addlog("sg_build_config: error - ACL 'default' is empty, use as default 'block all'.");
               $sgconf[] = "\t default { ";
               $sgconf[] = "\t\t pass none";
               $sgconf[] = "\t\t redirect " . $redirect_base_url;
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
// blacklist
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
         sg_addlog("sg_reconfigure_blacklist: begin upload from '$sf'.");
         if ($sf[0] === "/") { // local file - example '/tmp/blacklists.tar'
             if (file_exists($sf)) {
                 $sf_contents = file_get_contents($sf);
                 sg_addlog("sg_reconfigure_blacklist: get file '$sf'.");
             } else {
                 sg_addlog("sg_reconfigure_blacklist: error get file '$sf', file not found.");
                 return;
             }
         } else {// url
             sg_addlog("sg_reconfigure_blacklist: get url '$sf'.");
             $sf_contents = sg_uploadfile_from_url($sf, BLK_LOCALFILE, $opt);
         }

         // 3. update
         if (empty($sf_contents)) {
             sg_addlog("sg_reconfigure_blacklist: error file content '$sf'.");
             return;
         }
         // manually content save to local file
         file_put_contents(BLK_LOCALFILE, $sf_contents);
         sg_update_blacklist(BLK_LOCALFILE);

         // 4. rebuild db
         sg_full_rebuild_db();

         sg_addlog("sg_reconfigure_blacklist: end");
}

// -----------------------------------------------------------------------------
// sg_update_blacklist - update blacklist from file
// -----------------------------------------------------------------------------
function sg_update_blacklist($from_file) {
         global $squidguard_config;
         $dbhome = SQUIDGUARD_DBHOME_DEF;
         $workdir = SQUIDGUARD_WORKDIR_DEF;

         if (file_exists($squidguard_config[FLD_DBHOME])) $dbhome = $squidguard_config[FLD_DBHOME];
         if (file_exists($squidguard_config[FLD_WORKDIR])) $workdir = $squidguard_config[FLD_WORKDIR];
         sg_addlog("sg_update_blacklist: begin '$dbhome'");

         if (file_exists($from_file)) {
             // 1. unpack blacklist file
             $bl_temp = '/var/tmp/blacklists';
             mwexec('tar zxvf ' . $from_file . ' -C /var/tmp/');
             sg_addlog("sg_update_blacklist: unpack uploaded file $from_file -> $bl_temp");

             // 2. copy blacklist to squidGuard base
             if (file_exists($bl_temp)) {
                 // - copy blacklist & create entries list
                 sg_addlog("sg_update_blacklist: create entries");
                 $blk_files = scan_dir($bl_temp);
                 $blk_entries = array();
                 foreach($blk_files as $bf) {
                         if (($bf != '.') && ($bf != '..')) {
                              $blk_entries[] = $bf;
                              mwexec("cp -Rf $bl_temp/$bf  $dbhome");
                              sg_addlog("sg_update_blacklist: $bf");
                         }
                 }

                 // create entries list
                 if (count($blk_entries)) {
                     file_put_contents($workdir . SQUIDGUARD_BLK_ENTRIES, implode("\n", $blk_entries));
                     set_file_access($workdir . SQUIDGUARD_BLK_ENTRIES, OWNER_NAME, 0755);
                     sg_addlog("sg_update_blacklist: create entries "  . $workdir . SQUIDGUARD_BLK_ENTRIES);
                 }
                 sg_remove_unused_db_entries();

                 // clearing temp
                 mwexec("rm -R $bl_temp");
             }
             set_file_access($squidguard_config[FLD_DBHOME], OWNER_NAME, 0755);
#             sg_full_rebuild_db();
         }
         sg_addlog("sg_update_blacklist: end");
}

// -----------------------------------------------------------------------------
// sg_entries_blacklist - update blacklist from file
// -----------------------------------------------------------------------------
function sg_entries_blacklist() {
         global $squidguard_config;
         $contentS = '';
         $fl = SQUIDGUARD_WORKDIR_DEF . SQUIDGUARD_BLK_ENTRIES;
         if (file_exists($squidguard_config[FLD_WORKDIR]))
             $fl = $squidguard_config[FLD_WORKDIR] . SQUIDGUARD_BLK_ENTRIES;

         if (file_exists($fl)) {
             $contents = file_get_contents($fl);
             $contents = explode("\n", $contents);
         }
         return $contents;
}

# -------------------------- UTILS ---------------------------------------------
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

// -----------------------------------------------------------------------------
// Set file access
// -----------------------------------------------------------------------------
function set_file_access($dir, $owner, $mod) {

        if (!file_exists($dir)) return;

        chown($dir, $owner);
        chgrp($dir, $owner);
        chmod($dir, $mod);

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
# ==============================================================================
# self utils
# ==============================================================================
# scan_dir - build files listing for $dir
# ------------------------------------------------------------------------------
function scan_dir($dir) {
    $files = array();
    if (file_exists($dir)) {
        $dh = opendir($dir);
        while (false !== ($filename = readdir($dh)))
            $files[] = $filename;
        sort($files);
    }
    return $files;
}
# ------------------------------------------------------------------------------
# is_url - build files listing for $dir
# ------------------------------------------------------------------------------
function is_url($url) {
    if (empty($url))              return false;
    if (eregi("^http://", $url))  return true;
    if (eregi("^https://", $url)) return true;
    if (eregi("^([0-9]{3})", $url)) return true; // http error code 403, 404, 410, 500,
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
