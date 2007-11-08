<?php
# ---------------------------------------------------------------------------------------------------------------------~
/*        squidguard_cls.inc
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
# ---------------------------------------------------------------------------------------------------------------------
# SquidGuardConfigClass
# Generator SquidGuard configuration
# email: dv_serg@mail.ru
# ---------------------------------------------------------------------------------------------------------------------
# squidGuard command:
# squidGuard -C all                          - apdate database
# squidGuard -c <configfile>        - create squidGuard with specified config file
# ---------------------------------------------------------------------------------------------------------------------
# Directories:
# work path        - $workdir
# log path        - $workdir + $logdir
# ---------------------------------------------------------------------------------------------------------------------

define('FILES_DB_HEADER', '
# -----------------------------------------------------------------------------
#        File created by squidGuard package GUI
#        (C)2006 Serg Dvoriancev
# -----------------------------------------------------------------------------
');

define('ACL_WARNING_ABSENSE_PASS', "!WARNING! Absence PASS 'all' or 'none' added as 'none'");

# ---------------------------------------------------------------------------------------------------------------------
define('REDIRECTOR_OPTIONS_REM',   '# squidGuard options');
define('REDIRECTOR_PROGRAM_OPT',   'redirect_program');
define('REDIRECT_BYPASS_OPT',      'redirector_bypass');
define('REDIRECT_CHILDREN_OPT',    'redirect_children');
define('REDIRECTOR_PROCESS_COUNT', '3');
define('SQUID_CONFIG_FILE',        '/usr/local/etc/squid/squid.conf');
define('SQUIDGUARD_CONFIG_FILE',   '/squidGuard.conf');
define('TMP_DIR',                  '/var/tmp');
define('SQUIDGUARD_LOG_FILE',      '/squidGuard_webgui.log');
define('SQUIDGUARD_ACCESSBLOCK_FILE', 'squidguard_accessblock.log');
define('SQUIDGUARD_BLK_ENTRIES',   '/squidguard_blk_entries.ini');
define('BLACKLIST_ARCHIVE',        '/blacklists.tar.gz');

define('REDIRECT_BASE_URL', 'http://10.62.0.3/sgerror.php');

// debug options
define('DEBUG_UPDATE_SQUID_CONF',        'true');
define('DEBUG_UPDATE_SQUIDGUARD_DB',     'true');
define('DEBUG_MAKE_SQUIDGUARD_CONFIG',   'true');

define('SQUIDGUARD_LOG_DIR',    '/log');
define('SQUIDGUARD_BACKUP_DIR', '/backup');

// options
define('SQUIDGUARD_LOG_MAX_COUNT', '500'); // max log lines

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

define('FLT_DEFAULT_ALL', 'all');

define('OWNER_NAME', 'proxy'); // owner user name

define('DEBUG_ON', 'true');
# ------------------------------------------------------------------------------
#
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

# ------------------------------------------------------------------------------

class SquidGuardConfigClass {
        // variables
        var $work_dir;
        var $dbhome;
        var $bin_path;
        var $log_dir;    // $work_dir + SQUIDGUARD_BACKUP_DIR
        var $log_file;   // $log_dir + SQUIDGUARD_LOG_FILE

        var $enabled;
        var $blacklist_enabled;

        var $debug_history;

        // objects
        var $sources;
        var $dests;
        var $blacklist;
        var $times;
        var $rewr;
        var $acls;
        var $acldef;
        var $enable_deflog;         // TODO: check for delete

        // temporary objects
        var $expr;                // temporary expressions list - not apply to config!
        var $redir;               // temporary expressions list - not apply to config!

        function __construct() {
                $this->work_dir   = '';
                $this->dbhome     = '';
                $this->bin_path   = '';
                $this->log_dir    = '';
                $this->log_file   = '';
                $this->init();
        }

        function init() {
                $this->enabled = true;
                $thus->blacklist_enabled = true;

                $this->acls    = Array();
                $this->enable_deflog  = '';
                $this->acldef  = new ACLClass();
                $this->acldef->ini("default", "", "default acl");
                $this->acldef->assign("all");

                $this->redir     = Array();
                $this->expr      = Array();
                $this->rewr      = Array();
                $this->times     = Array();
                $this->sources   = Array();
                $this->dests     = Array();
                $this->blacklist = Array();

                // log
                $this->add_log("Init squidGuard object.");
        }

        function set_options($bin_path, $work_dir, $dbhome) {
              $this->bin_path = $bin_path;
              $this->work_dir = $work_dir;
              $this->dbhome   = $dbhome;

              $this->backup_dir = $work_dir . SQUIDGUARD_BACKUP_DIR;
              $this->log_dir    = $work_dir . SQUIDGUARD_LOG_DIR;
              $this->log_file   = $this->log_dir . SQUIDGUARD_LOG_FILE;

              // log
              $this->add_log("Set bin path "  . $this->bin_path, DEBUG_ON);
              $this->add_log("Set work path " . $this->work_dir, DEBUG_ON);
              $this->add_log("Set db path "   . $this->dbhome,   DEBUG_ON);

              $this->update();
        }

        function update() {
              $this->add_log("Update squidGuard object.");

              // check work_dir, log_dir & create if not exists
              if (!empty($this->work_dir)) {
                  $this->add_log("Check work directory.");
                  // check dir's
                   if (!file_exists($this->work_dir)) {
                       mwexec("mkdir " . $this->work_dir);
                       $this->add_log("Create work dir " . $this->work_dir, DEBUG_ON);
                   }
                   if (!file_exists($this->log_dir)) {
                       mwexec("mkdir " . $this->log_dir);
                       $this->add_log("Create log dir " . $this->log_dir, DEBUG_ON);
                   }
                   // set access right
                   set_file_access($this->work_dir, OWNER_NAME, 0755);
              }

              // check squidguard db
              if (!empty($this->dbhome)) {
                  $this->add_log("Check db directory.");
                  if (!file_exists($this->dbhome)) {
                      mwexec("mkdir " . $this->dbhome);
                      $this->add_log("Create db dir " . $this->dbhome, DEBUG_ON);
                  }
                  // set access right
                  set_file_access($this->dbhome, OWNER_NAME, 0755);
              }

              // load blacklist configuration
              if ($this->blacklist_enabled) {
                  $this->add_log("Load blacklist configuration.");

                  if (file_exists($this->work_dir . SQUIDGUARD_BLK_ENTRIES)) {
                      $db_items = array();
                      $blk_entries = explode("\n", file_get_contents($this->work_dir . SQUIDGUARD_BLK_ENTRIES));

                      // check files as dir's
                      $std_blk = std_blacklist_get_description();
                      foreach($blk_entries as $df) {
                          if (($df != ".") && ($df != "..") && (is_dir($this->dbhome . "/" . $df))) {
                              $this->add_blacklist($df, $str_blk[$df]);
                              $this->add_log("Create $df", DEBUG_ON);
                          }
                      }
                  }
                  else $this->add_log("File " . $this->work_dir . SQUIDGUARD_BLK_ENTRIES . " not found.");
              }
              else     $this->add_log("Blacklist disabled.");
        }

        function reconfigure() {    // TODO: check restart function order
                $this->update();

                // check and rebuild squidguard db
                $this->add_log("Reconfigure squidGuard object");
                $this->update_destination_files();
                $this->remove_unused_db_entries();

                // update only user dest items; blacklist rebuilded with when updated
                if(!empty($this->dests)){
                    $dblist = array();
                    foreach($this->dests as $dst)
                             $dblist[] = $dst->name;
                    $this->rebuild_squidguard_db($dblist);
                }
                // generate squidGuard config
                $this->add_log("Generate squidGuard config.");
                $conf = $this->make_config();
                file_put_contents($this->work_dir . SQUIDGUARD_CONFIG_FILE, $conf);
                file_put_contents('/usr/local/etc/squid' . SQUIDGUARD_CONFIG_FILE, $conf); // << my squidGuard want config '/usr/local/etc/squid' by default
                set_file_access($this->work_dir, OWNER_NAME, 0755);

                // update squid config
                $this->update_squid_conf();

                // restart squid - squid automaticly restart redirector too
                mwexec('squid');
                if (!mwexec('killall -HUP squid'))
                     $this->add_log("Restart squid .. success.");
                else $this->add_log("Restart squid .. error.");
        }

        # --------------------------------------------------------------------------------------------------------------
        # BLACKLIST
        #     If backup list set 'disable' - all his entries not used for filtering
        # --------------------------------------------------------------------------------------------------------------

        #
        # Update blacklist db from file
        #
        function update_blacklist($upload_file) {
             $this->add_log("Upload file $upload_file.");
             if ($upload_file) {
                  // 1. unpack blacklist file
                  $this->add_log("Unpack file $upload_file .. ");
                  $bl_temp = '/var/tmp/blacklists';
                  if (!mwexec('tar zxvf ' . $upload_file . ' -C /var/tmp/'))
                       $this->add_log("Success.");
                  else $this->add_log("Error!");

                  // 2. copy blacklist to squidGuard base
                  if (file_exists($bl_temp)) {
                      $this->add_log("Copy files to db.. ");

                      // - copy blacklist & create entries list
                      $blk_files = scan_dir($bl_temp);
                      $blk_entries = array();
                      foreach($blk_files as $bf) {
                          if (($bf != '.') && ($bf != '..')) {
                              $blk_entries[] = $bf;
                              if (!mwexec("cp -Rf $bl_temp/$bf " . $this->dbhome)) {
                                     $this->add_log(" copy $bf success.");
                              } else $this->add_log(" copy $bl_temp/$bf -> $this->dbhome error.");
                          }
                      }

                      // create entries list
                      if (count($blk_entries)) {
                          file_put_contents($this->work_dir .  SQUIDGUARD_BLK_ENTRIES, implode("\n", $blk_entries));
                          set_file_access($this->work_dir .  SQUIDGUARD_BLK_ENTRIES, OWNER_NAME, 0755);
                      }
                      $this->remove_unused_db_entries();

                      // clearing temp
                      mwexec("rm -R $bl_temp");
                  }
                  set_file_access($this->dbhome, OWNER_NAME, 0755);
             }
        }

        #
        # remove unused DB entries
        #
        function remove_unused_db_entries() {
              $this->add_log("Remove unused db entries.");
                $db_entries = array();
              $file_list = '';
              // blk entries
              if (file_exists($this->work_dir .  SQUIDGUARD_BLK_ENTRIES)) {
                  $db_entries = explode("\n", file_get_contents($this->work_dir . SQUIDGUARD_BLK_ENTRIES));

                  // user entries
                  foreach($this->dests as $dst)
                      $db_entries[] = $dst->name;

                  $file_list = scan_dir($this->dbhome);
                  $file_for_del = array_diff($file_list, $db_entries);

                  foreach($file_for_del as $fd) {
                      $file_fd = $this->dbhome . "/" . $fd;
                      if (($fd != "") && ($fd != ".") && ($fd != "..")) {
                          if (file_exists($file_fd)) {
                              if (!mwexec("rm -R  . $file_fd"))
                                   $this->add_log("Delete $file_fd");
                              else $this->add_log("Error delete $file_fd");
                          } else $this->add_log("File $file_fd not found");
                      }
                  }
              }

        }

        #
        # Update squidGuard DB from user defined destinations
        #
        function update_destination_files() {
                 $this->add_log("Update destination files to db.");
                 if(!empty($this->dests)){
                     foreach($this->dests as $dst) {
                             $log = $dst->update_destination_files($this->dbhome);
                             $this->add_log($log);
                     }
                 }
                 $this->add_log($log);
                 set_file_access($this->dbhome, OWNER_NAME, 0755);
        }

        #
        # rebuild db: All ($dblist == '') or from listing ($dblist is array)
        #
        function rebuild_squidguard_db($dblist='') {

                 set_file_access($this->dbhome, OWNER_NAME, 0755);  // before

                 $this->add_log("Start rebuild database.");
                 if (is_array($dblist)) {
                     foreach($dblist as $dbn) {
                         if (($dbn !== ".") && ($dbn !== "..") && file_exists($this->dbhome . "/$dbn")) {
                             if (!mwexec($this->bin_path . "/squidGuard -C"))// $dbn"))
                                  $this->add_log("Rebuild $dbn - success.");
                             else $this->add_log("Rebuild $dbn - error.");
                         } else
                             $this->add_log("Error rebuild database! Dir " . $this->dbhome . "/$dbn not found.");
                     }
                 } else {
                     // rebuild squidGuard DB
                     // -C - create db files; -u - update '.diff' files to db
                     if (!mwexec($this->bin_path . '/squidGuard -C all')/* &&
                         !mwexec($this->bin_path . '/squidGuard -u')*/)
                          $this->add_log("Rebuild all db - success.");
                     else $this->add_log("Rebuild all db - error!");
                 }
                // set_file_access($this->dbhome, OWNER_NAME, 0755);  // after
        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Insert to '/usr/local/squid/etc/squid.conf' options:
        //         redirector_bypass on
        //         redirect_program /usr/local/squidGuard/bin/squidGuard -c /path_to_config_file
        //         redirect_children 1
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Command for restart squid with all redirectors
        // # killall -HUP squid
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        function update_squid_conf() {
                $this->add_log("Update 'squid.conf' file");
                $opt = '';
                $squid_conf_file = SQUID_CONFIG_FILE;
                $redirector_path = $this->bin_path . '/squidGuard';
                $redirector_conf = $this->work_dir . SQUIDGUARD_CONFIG_FILE;
                $this->add_log("Redirector path=$redirector_path", DEBUG_ON);

                // update squid.conf file
                if (file_exists($squid_conf_file)) {
                    $conf = file_get_contents($squid_conf_file);

                    // remove redirector options from 'squid.conf'
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

                    if ($this->enabled) {
                        // add redirector options from 'squid.conf'
                        $conf .= "\n";
                        $conf .= "\n" . REDIRECTOR_OPTIONS_REM;
                        $conf .= "\n" . REDIRECTOR_PROGRAM_OPT . " $redirector_path -c $redirector_conf";
                        $conf .= "\n" . REDIRECT_BYPASS_OPT    . ' on';
                        $conf .= "\n" . REDIRECT_CHILDREN_OPT  . " " . REDIRECTOR_PROCESS_COUNT;
                    }
                    $conf .= "\n";
                    file_put_contents($squid_conf_file, $conf);
                }
                else {
                    $this->add_log("File '$squid_conf_file' not found");
                }
        }

        # -------------------------------------------------------------------------------------------------------------
        # Operations
        # -------------------------------------------------------------------------------------------------------------

        function set_default_acl($time, $desc,
                 $ontime_rule,        /*[pass][$rewr][$redir][$log]*/
                 $overtime_rule = '', /*[$overpass][$overrew][$overredir][$overlog]*/
                 $disabled = '', $enable_deflog = '')
        {
                 $this->enable_deflog = $enable_deflog;
                 $this->acldef->ini("default", $time, $desc, $disabled);
                 if (is_array($ontime_rule))
                     $this->acldef->assign($ontime_rule['pass'], $ontime_rule['redirect'], $ontime_rule['rewrite'], $ontime_rule['log']);

                 if (is_array($overtime_rule))
                     $this->acldef->assign_over($overtime_rule['pass'], $overtime_rule['redirect'], $overtime_rule['rewrite'], $overtime_rule['log']);
        }

        function & add_acl($src_name, $time, $desc,
                 $ontime_rule,        /*[pass][$rewr][$redir][$log]*/
                 $overtime_rule = '', /*[$overpass][$overrew][$overredir][$overlog]*/
                 $disabled = '')
        {
                 $item = new ACLClass();
                 $item->ini($src_name, $time, $desc, $disabled);
                 if (is_array($ontime_rule))
                     $item->assign($ontime_rule['pass'], $ontime_rule['redirect'], $ontime_rule['rewrite'], $ontime_rule['log']);

                 if (is_array($overtime_rule))
                     $item->assign_over($overtime_rule['pass'], $overtime_rule['redirect'], $overtime_rule['rewrite'], $overtime_rule['log']);

                 $this->acls[$src_name] = $item;
                 return $this->acls[$src_name];
        }

        function del_acl($name, $time, $desc) {
                 $this->acls[$name] = '';
        }

        function & add_src($name, $ip, $desc, $enablelog) {
                $item = new SrcClass();
                $item->ini($name, $ip, $desc, $enablelog);
                $this->sources[$name] = $item;
                return $this->sources[$name];
        }

        function del_src($name) {
                $this->sources[$name] = "";
        }

        function & add_dest($name, $domains, $urls, $expr, $redir, $desc, $enablelog) {
                $item = new DestClass();
                $item->ini($name, $domains, $urls, $expr, $redir, $desc, $enablelog);
                $this->dests[$name] = $item;
                return $this->dests[$name];
        }

        function del_dest($name) {
                $this->dests[$name] = "";
        }

        function & add_blacklist($name, $desc) {
                $item = new DestClass();
                $item->ini($name, "", "", "", "", $desc);
                $this->blacklist[$name] = $item;
                return $this->blacklist[$name];
        }

        function & add_time($name, $desc) {
                $item = new TimeClass();
                $item->ini($name, $desc);
                $this->times[$name] = $item;
                return $this->times[$name];
        }

        function del_time($name) {
                $this->times[$name] = "";
        }

        function & add_rewrite($name, $desc, $enablelog) {
                $item = new RewrClass();
                $item->ini($name, $desc, $enablelog);
                $this->rewr[$name] = $item;
                return $this->rewr[$name];
        }

        function del_rewrite($name) {
                $this->rewr[$name] = '';
        }

        function goto_dir($workdir,  $subdir) {
             $res = false;
             $dir = @chdir($workdir);
             if ($dir) {  // workdir exists
                 $dir = "$workdir \\ $subdir";
                 if (@chdir($dir))             // dir exists
                     $res = true;
                 elseif (@mkdir($dir, 0755))   // need create dir
                      $res = true;
                 else $this->add_log("ERROR: Can't create dir $dir");
             } else   $this->add_log("ERROR: Work path $workdir not found.");
             return $res;
        }

        #
        # Make config
        #
        function make_config() {
                $conf = '';
                // header
                // =================================================================================================================
                $conf .= "\n# " . str_repeat("=", 119);
                $conf .= "\n# SquidGuard configuration file";
                $conf .= "\n# ";
                $conf .= "\n# This file generated automaticly with SquidGuardClass";
                $conf .= "\n# ";
                $conf .= "\n# SquidGuardConfigClass (C)2006 Serg Dvoriancev";
                $conf .= "\n# email: dv_serg@mail.ru";
                $conf .= "\n# ";
                $conf .= "\n# " . str_repeat("=", 119);
                $conf .= "\n";
                // =================================================================================================================

                if(!empty($this->log_dir))
                    $conf .= "\nlogdir " . $this->log_dir;

                if(!empty($this->dbhome))
                    $conf .= "\ndbhome " . $this->dbhome;

                  $conf .= "\n";

                // make times
                if(!empty($this->times)){
                    foreach($this->times as $time) {
                            $conf .= $time->make_config($this);
                    }
                }

                // make sources
                if(!empty($this->sources)){
                    foreach($this->sources as $src) {
                            $conf .= $src->make_config($this);
                    }
                }

                // make blacklist
                $conf .= "\n# Blacklist";
                if($this->blacklist_enabled) {
                    if (!empty($this->blacklist)) {
                        foreach($this->blacklist as $dest)
                            $conf .= $dest->make_config($this);
                    } else $conf .= "\n# Blacklist empty\n";
                } else $conf .= "\n# Blacklist disabled\n";

                // make destinations
                $conf .= "\n# User destinations";
                if(!empty($this->dests)){
                    foreach($this->dests as $dest) {
                            $conf .= $dest->make_config($this);
                    }
                }

                // make rewrites
                if (!empty($this->rewr)) {
                     foreach($this->rewr as $rewr) {
                             $conf .= $rewr->make_config($this);
                     }
                }

                // make acls
                $conf .= "\nacl {";
                if (!empty($this->acls)) {
                     foreach($this->acls as $acl) {
                             $conf .= $acl->make_config($this);
                     }
                }
                $conf .= $this->acldef->make_config($this);
                $conf .= "\n }";

                return $conf;
        }

      # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      # Log
      # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      function add_log($str, $is_debug = '') {

          // if set $is_debug as value, but ($is_debug != 'on') then return
          // this defined for include debug info to log;
          // add_log('str') - simple log, add_log('str', 'on') - debig log;
          // looking constant 'DEBUG_ON'
          if ($is_debug && ($is_debug != 'on')) return;

          if (file_exists($this->log_dir)) {
              if (file_exists($this->log_file))
                  $log = file_get_contents($this->log_file);
              $log = explode("\n", $log);

              $s = trim($str);
              if (!empty($s)) $log[] = date("d.m.Y H:i:s") . ": $s";

              // check log lines count
              while(count($log) > SQUIDGUARD_LOG_MAX_COUNT) :
                  $log = array_shift($log);
              endwhile;

              if ($log)
                  $log = implode("\n", $log);
              file_put_contents($this->log_file, $log);
              set_file_access($this->log_file, OWNER_NAME, 0755);
          }
      }

      function get_log() {
          $log = '';
          if (file_exists($this->log_file))
              $log = file_get_contents($this->log_file);
          return $log;
      }

      function get_log_html() {
          $log = $this->get_log();
          return str_replace("\n", "<br>", $this->get_log());
      }

      function test_work($urls_src) {
            $result = '';
            $urls   = '';
            if (is_array($urls_src))
                 $urls = implode("\n", $urls_src);
            else $urls = $urls_src;
            $test_file = "/var/tmp/test.urls";
            file_put_contents($test_file, $urls);
            $result = exec ("/usr/local/bin/squidGuard < $test_file");
            file_put_contents("$test_file.result", $result);
            return $result;
      }

}

// =====================================================================================================================
// RewrClass
// =====================================================================================================================

class RewrClass {
        var $name;
        var $desc;
        var $rewrites;
        var $enablelog;

        function __construct() {
                $this->name = '';
                $this->desc = '';
                $this->rewrites = Array();
                $this->enablelog = '';
        }

        function ini($name, $desc, $enablelog = '') {
                $this->name = $name;
                $this->desc = $desc;
                $this->enablelog = $enablelog;
        }

        function add_item($target_url, $replace_to, $case_ins='', $http_301='', $http_302='') {
                $item .= "s@$target_url@$replace_to@";
                if (!empty($case_ins)) $item .= 'i';
                if (!empty($case_ins)) $item .= 'r';  // TODO: r and R - check work options?
                if (!empty($case_ins)) $item .= 'R';  // -//-
                $this->rewrites[] = $item;
        }

        function make_config($owner) {
                $name = $this->name;
                $desc = $this->desc;
                $enablelog = $this->enablelog;
                $conf = '';
                $conf .= "\nrew $name { ";
                if (!empty($desc))
                        $conf .= "\t# $desc";

                if (is_array($this->rewrites)) {
                        foreach($this->rewrites as $rew) {
                                $conf .= "\n        $rew";
                        }
                }
                if ($enablelog)
                    $conf .= "\n        log = $name";
                $conf .= "\n    }";
                $conf .= "\n";
                return $conf;
        }
}

class TimeClass {
      var $name;
      var $items;
      var $desc;

      function __construct() {
            $this->name = '';
            $this->desc = '';
            $this->items = Array();
      }

      function ini($name, $desc) {
            $this->name = $name;
            $this->desc = $desc;
      }

      function & add_date($daterange, $timerange) {
            $item = new TimeItem();
            $item->ini("date", '', $daterange, $timerange);
            $this->items[] = $item;
//            return $this->items[count($this->items-1)];
      }

      function & add_weekly($days, $timerange) {
            $item = new TimeItem();
            $item->ini("weekly", $days, '', $timerange);
            $this->items[] = $item;
//            return $this->items[count($this->items-1)];
      }

      function make_config($owner) {
            $conf = '';
            $name = $this->name;
            $desc = $this->desc;
            $conf .= "\ntime $name { ";
            if ($desc != "")
                $conf .= "\t# $desc";
             if (is_array($this->items)) {
                foreach($this->items as $item) {
                    $type = $item->type;
                    $days = $item->days;
                    $dates = $item->dates;
                    $hours = $item->hours;
                    if ($item->type == "date")
                         $conf .= "\n        date $dates $hours";
                    else $conf .= "\n        weekly $days $hours";
                }
             }
            $conf .= "\n    }";
            $conf .= "\n";
            return $conf;
      }
}

class TimeItem {
        var $type;
        var $days;
        var $dates;
        var $hours;

        function __construct() {
                $this->type = '';
                $this->days = '';
                $this->dates = '';
                $this->hours = '';
        }

        function ini($type, $days, $dates, $hours) {
                $this->type = $type;
                $this->days = $days;
                $this->dates = $dates;
                $this->hours = $hours;
        }
}

class DestClass {
        var $name;
        var $domains;   // array
        var $urls;      // array
        var $expr;      // array
        var $redir;
        var $desc;
        var $enablelog;

        function __construct() {
            $this->name    = '';
            $this->domains = array();
            $this->urls    = array();
            $this->expr    = array();
            $this->redir   = '';
            $this->desc    = '';
            $this->enablelog = '';
        }

        function ini($name, $domains, $urls, $expr, $redir, $desc, $enablelog = '') {
            $this->name      = $name;
            $this->domains   = explode(' ', $domains);
            $this->urls      = explode(' ', $urls);
            $this->expr      = explode(' ', $expr);
            $this->redir     = $redir;
            $this->desc      = $desc;
            $this->enablelog = $enablelog;
        }

        // create custom destination files
        function update_destination_files($dbcatalog) {
            $log = "\n\nUpdate destination file " . $this->name;
            $path = $dbcatalog . '/' . $this->name . '/';
            $log .= "\npath = $path";

            // 1 - set/check destination catalog
            if (!file_exists($path)) {
               if (!mkdir ($path, 0755)) {
                      $log .= "\nError create dir $path";
                      return $log; // error create dir
               } else $log .= "\nCreate dir.";
            }

            // 2 - build domains file
            if (!empty($this->domains)) {
                 $content = '';
                 foreach($this->domains as $dm) {
                     $content .= "\n$dm";
                 }

                 $content = trim($content);
                 if (!empty($content)) {
                         file_put_contents($path . '/domains', $content);
                     $log .= "\nadd domains";
                 }
            }

            // 3 - build urls file
            if (!empty($this->urls)) {
                 $content = '';
                 foreach($this->urls as $url) {
                     $content .= "\n$url";
                 }

                 $content = trim($content);
                 if (!empty($content)) {
                     file_put_contents($path . '/urls', $content);
                     $log .= "\nadd urls";
                 }
            }

            // 4 - build expression file
            if (!empty($this->expr)) {
                 $content = '';
                 foreach($this->expr as $ex) {
                    $content .= "\n$ex";
                 }

                 $content = trim($content);
                 if (!empty($content)) {
                     file_put_contents($path . '/expressions', $content);
                     $log .= "\nadd expressions";
                 }
            }

            return $log;
        }

        function make_config($owner) {
                $file = '';
                $conf = '';
                $name  = $this->name;
                $dbhome = $owner->dbhome;
                $domains = $this->domains;
                $urls  = $this->urls;
                $expr  = $this->expr;
                $redir = $this->redir;
                $desc  = $this->desc;
                $enablelog = $this->enablelog;

                $conf .= "\ndest $name { ";

                # description
                if ($desc != "")
                    $conf .= "\t# $desc";

                # domains
                if ($domains != "") {
                    $file = $dbhome . "/$name/domains";
                    if (file_exists($file))
                        $conf .= "\n        domainlist $name/domains";
                }

                # expressions
                if ($expr != "") {
                    $file = $dbhome . "/$name/expressions";
                    if (file_exists($file))
                        $conf .= "\n        expressionlist $name/expressions";
                }

                # urls
                if ($urls != "") {
                    $file = $dbhome . "/$name/urls";
                    if (file_exists($file))
                        $conf .= "\n        urllist $name/urls";
                }

                if ($redir != "")
                    $conf .= "\n        redirect $redir";

                if ($enablelog)
                    $conf .= "\n        log " . SQUIDGUARD_ACCESSBLOCK_FILE;

                $conf .= "\n    }";
                $conf .= "\n";

                return $conf;
        }
}

class SrcClass {
        var $name;
        var $ip;
        var $domain;
        var $users;
        var $desc;
        var $enablelog;

        function __construct() {
            $this->name = '';
            $this->ip   = '';
            $this->domain = '';
            $this->desc = '';
            $this->enablelog = '';
            $this->users = Array();
        }

        function ini($name, $ip, $desc, $enablelog = '') {
            $this->name = $name;
            $this->ip   = $ip;
            $this->domain = '';
            $this->desc = $desc;
            $this->enablelog = $enablelog;
        }

        function make_config($owner) {
            $conf      = '';
            $name      = $this->name;
            $ip        = $this->ip;
            $desc      = $this->desc;
            $enablelog = $this->enablelog;
            $conf .= "\nsrc $name { ";
            if ($desc != "")
                $conf .= "\t# $desc";
            $conf .= "\n        ip $ip";
            if ($enablelog)
                $conf .= "\n        log " . SQUIDGUARD_ACCESSBLOCK_FILE;
            $conf .= "\n    }";
            $conf .= "\n";
            return $conf;
        }
}

class ACLClass {
      var $src;
      var $time;
      var $desc;
      var $disabled;

      var $pass;                // pass array. if last item = 'all' then 'pass all' or 'pass none'
      var $redir;
      var $rewr;
      var $log;

      var $over_pass;           // array of overtime dest's
      var $over_redir;
      var $over_rewr;
      var $over_log;

      function __construct() {
               $this->src  = '';
               $this->time = '';
               $this->desc = '';
                 $this->disabled = '';
      }

      function ini($src, $time, $desc, $disabled='') {
               $this->src  = $src;
               $this->time = $time;
               $this->desc = $desc;
                 $this->disabled = $disabled;
      }

      function assign($pass, $redir='', $rewr='', $log='') {
                     $this->pass  = $pass;
                        // check and fix absence of "all" and "none"
                        if (!empty($pass) && (substr_count($pass, "all") == 0) && (substr_count($pass, "none") == 0)) {
                                $this->pass .= " none";
                                $this->desc .= " <Uptime> " . ACL_WARNING_ABSENSE_PASS . " ";
                        }

                     $this->redir = $redir;
                     $this->rewr  = $rewr;
                     $this->log   = $log;
      }

      function assign_over($pass, $redir='', $rewr='', $log='') { // used only if assigned time
                     $this->over_pass  = $pass;
                     // check and fix absence of "all" and "none"
                     if (!empty($pass) && (substr_count($pass, "all") == 0) && (substr_count($pass, "none") == 0)) {
                         $this->over_pass .= " none";
                         $this->desc .= " <Overtime> " . ACL_WARNING_ABSENSE_PASS;
                     }

                     $this->over_redir = $redir;
                     $this->over_rewr  = $rewr;
                     $this->over_log   = $log;
      }

      function make_config($owner) {
              $conf = '';
              $src  = $this->src;
              $time = $this->time;
              $desc = $this->desc;

              // if item disabled - skip make_config
              if ($this->disabled == 'on') {
                  $conf .= "\n\n        # rule $src disabled";
                  return $conf;
              }

              $conf .= "\n";
              $conf .= "\n        $src $time { ";
              if (!empty($desc))
                   $conf .= "\t# $desc";
              $err_comment = '';
              $pass = $this->pass;
              if (is_array($pass)) {
                        // check dest's
                        foreach ($pass as $dst) {
                                 if (($dst != 'all') && ($dst != 'none')) {
                                      $obj = $owner->dests[$dst];
                                      if (!is_object($obj)) $err_comment .= " $dst";
                                 }
                        }
                        $err_comment .= "***!!!***";
                        if (!empty($err_comment))
                             $err_comment .= " error: dest's '$err_comment' not found";
                        $pass = implode(" ", $pass);
              }

              $conf .= "\n            pass $pass";
              if (!empty($err_comment)) $conf .= " # $err_comment";
              unset($err_comment);

              $redir = $this->redir;
              if (!empty($redir)) {
                  $rstr = strtolower(trim($redir));
                  if ((strpos($rstr, 'http://')  !== false) ||
                      (strpos($rstr, 'https://') !== false) ||
                      (strpos($rstr, 'ftp://')   !== false))
                       $conf .= "\n            redirect " . REDIRECT_BASE_URL . "?url=" . $rstr;
                  else $conf .= "\n            redirect " . REDIRECT_BASE_URL . "?msg=" . urlencode($redir);
              }

              $rewr = $this->rewr;
              if (!empty($rewr)) {
                  // check redirection name for exists
                  $err_comment = '';
                  $obj = $owner->rewr[$rewr];
                  if (empty($obj) || !is_object($obj))
                      $err_comment = " rewrite '$rewr' not found";

                  $conf .= "\n            rewrite $rewr # $err_comment";
              }

              $log = $this->log;
              if (!empty($log) && ($src=='default'))
                  $conf .= "\n            log " . SQUIDGUARD_ACCESSBLOCK_FILE;

              $conf .= "\n        }";

              // alternative
              $pass  = $this->over_pass;
              $redir = $this->over_redir;
              $rewr  = $this->over_rewr;
              $log   = $this->over_log;
              if (!empty($time) && !empty($pass)) {
                  $conf .= " else {";
                  $conf .= "\n            pass $pass";
                  if (empty($this->pass) || ($this->pass[count($this->pass) - 1] != 'all'))
                      $conf .= " none";
                  if (!empty($redir))
                      $conf .= "\n            redirect $redir";
                  if (!empty($rewr))
                      $conf .= "\n            rewrite $rewrite";
                  // log only for default
                  if (!empty($log) && ($src=='default'))
                      $conf .= "\n            log " . SQUIDGUARD_ACCESSBLOCK_FILE;
                  $conf .= "\n        }";
              }

              return $conf;
      }
}

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Set file access
#
function set_file_access($dir, $owner, $mod) {
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

?>
