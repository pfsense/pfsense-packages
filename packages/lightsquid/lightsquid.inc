<?php
/*
        lightsquid.inc
        Copyright (C) 2006 Serg Dvorianceev
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

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');
require_once('squid.inc');

define ('CMD_PKGDELETE', 'pkg_delete lightsquid-1.7.1');

// enable GUI debug
define('LS_GUI_DEBUG', 'on');
define('LS_LOG_FILE',  '/tmp/lightsquid_gui.log');

// configuration settings !-- CHECK THIS --!
define('LS_CONFIGPATH',       '/usr/local/etc/lightsquid');
define('LS_CONFIGFILE',       'lightsquid.cfg');
define('LS_CONFIGFILE_DIST',  'lightsquid.cfg.dist');
define('LS_WWWPATH',          '/usr/local/www/lightsquid');
define('LS_TEMPLATEPATH',     '/usr/local/www/lightsquid/tpl');
define('LS_LANGPATH',         '/usr/local/share/lightsquid/lang');
define('LS_REPORTPATH',       '/var/lightsquid/report');
define('LS_SQUIDLOGPATH',     '/var/squid/log');
define('LS_SQUIDLOG',         'access.log');
define('LS_IP2NAMEPATH',      '/usr/local/libexec/lightsquid');

define('CRONTAB_FILE',               '/var/cron/tabs/root');
define('CRONTAB_LS_TEMPLATE',        '/usr/bin/perl /usr/local/www/lightsquid/lightparser.pl');
define('CRONTAB_LS_JOBKEY',          '/lightparser.pl');
define('CRONTAB_SQUID_TEMPLATE',     '/usr/local/sbin/squid -k rotate > /dev/null');
define('CRONTAB_SQUID_JOBKEY',       '/squid -k rotate');

// default values
define('LS_DEF_IP2NAME',        'dns');
define('LS_DEF_SQUIDLOGTYPE',   '0');
define('LS_DEF_SKIPURL',        'zzz\.zzz');
define('LS_DEF_LANG',           'eng');
define('LS_DEF_TEMPLATE',       'base');
define('LS_DEF_BARCOLOR',       'orange');

// variable names
define('LS_VAR_CFGPATH',      'cfgpath');
define('LS_VAR_LOGPATH',      'logpath');
define('LS_VAR_TPLPATH',      'tplpath');
define('LS_VAR_LANGPATH',     'langpath');
define('LS_VAR_LANG',         'lang');
define('LS_VAR_REPORTPATH',   'reportpath');
define('LS_VAR_SQUIDLOGTYPE', 'squidlogtype');
define('LS_VAR_SKIPURL',      'skipurl');
define('LS_VAR_IP2NAMEPATH',  'ip2namepath');
define('LS_VAR_IP2NAME',      'ip2name');
define('LS_VAR_TEMPLATE',     'templatename');
define('LS_VAR_BARCOLOR',     'barcolor');

// xml variables
define('LS_XML_LANG',                         'lightsquid_lang');
define('LS_XML_SKIPURL',                      'lightsquid_skipurl');
define('LS_XML_IP2NAME',                      'lightsquid_ip2name');
define('LS_XML_TEMPLATE',                     'lightsquid_template');
define('LS_XML_BARCOLOR',                     'lightsquid_barcolor');
define('LS_XML_SHEDULERTIME',                 'lightsquid_refreshsheduler_time');
define('LS_XML_SQUID_SHEDULERTIME',           'lightsquid_squidrotatelog_sheduler_time');

function lightsquid_install() {
         update_log("lightsquid_install: started");
         // create lightsquid report catalog
         if (!file_exists(LS_REPORTPATH)) {
              update_log("lightsquid_install: Create report dir " . LS_REPORTPATH);
              mwexec("mkdir -p " . LS_REPORTPATH);
         }

         // install templates
         $lstpl_file = "/var/tmp/lightsquid_tpl.tbz";
         if (file_exists($lstpl_file)) {
              mwexec("tar zxvf $lstpl_file -C " . LS_TEMPLATEPATH);
              update_log("lightsquid_install: Install templates from $lstpl_file");
         }         
         
         # check perl
         if (!file_exists("/usr/bin/perl")) 
              mwexec("ln -s /usr/local/bin/perl /usr/bin/perl");
         
         update_log("lightsquid_install: stopped");
}

function lightsquid_deinstall() {
         update_log("lightsquid_deinstall: started");

         // delete cron task's
         ls_setup_cron("lightsquid_squid_rotate", "", "", false);
         ls_setup_cron("lightsquid_parser", "", "", false);
         // control shoot to head :)
#         mwexec(CMD_PKGDELETE);
         // remove '/usr/local/www/lightsquid' dir - any time can't be removed by templates
#         if (file_exists(LS_WWWPATH))
#             mwexec("rm -rf " . LS_WWWPATH);

         update_log("lightsquid_deinstall: stopped");
}

function lightsquid_resync() {
        global $config;
        $tm = '';
        $tm_squid = '';

        # check perl
        if (!file_exists("/usr/bin/perl")) 
            mwexec("ln -s /usr/local/bin/perl /usr/bin/perl");

        // debug
        $light_test = array();
        if (($_POST['Submit'] === 'Save') or !isset($_POST['Submit'])) {
             $lsconf_var = array();

             // variables for update
             $lsconf_var[LS_VAR_CFGPATH]      = "\"" . LS_CONFIGPATH . "\"";
             $lsconf_var[LS_VAR_LOGPATH]      = "\"" . LS_SQUIDLOGPATH . "\"";
             $lsconf_var[LS_VAR_TPLPATH]      = "\"" . LS_TEMPLATEPATH . "\"";
             $lsconf_var[LS_VAR_LANGPATH]     = "\"" . LS_LANGPATH . "\"";
             $lsconf_var[LS_VAR_REPORTPATH]   = "\"" . LS_REPORTPATH . "\"";
             $lsconf_var[LS_VAR_IP2NAMEPATH]  = "\"" . LS_IP2NAMEPATH . "\"";

             $lsconf_var[LS_VAR_LANG]         = "\"" . LS_DEF_LANG . "\"";
             $lsconf_var[LS_VAR_TEMPLATE]     = "\"" . LS_DEF_TEMPLATE . "\"";
             $lsconf_var[LS_VAR_IP2NAME]      = "\"" . LS_DEF_IP2NAME . "\"";
             $lsconf_var[LS_VAR_SKIPURL]      = "\"" . LS_DEF_SKIPURL . "\"";
             $lsconf_var[LS_VAR_SQUIDLOGTYPE] = LS_DEF_SQUIDLOGTYPE;

             // update variables from package GUI config
                if(isset($config['installedpackages']['lightsquid']['config'][0])) {
                        $cfg = $config['installedpackages']['lightsquid']['config'][0];

                        $tm = $cfg[LS_XML_SHEDULERTIME];
                        $tm_squid = $cfg[LS_XML_SQUID_SHEDULERTIME];

                        if (isset($cfg[LS_XML_LANG]) and !empty($cfg[LS_XML_LANG]))
                            $lsconf_var[LS_VAR_LANG] = "\"" . $cfg[LS_XML_LANG] . "\";";

                        if (isset($cfg[LS_XML_SKIPURL]) and !empty($cfg[LS_XML_SKIPURL]))
                            $lsconf_var[LS_VAR_SKIPURL] = "\"" . $cfg[LS_XML_SKIPURL] . "\";";

                        if (isset($cfg[LS_XML_IP2NAME]) and !empty($cfg[LS_XML_IP2NAME] ))
                            $lsconf_var[LS_VAR_IP2NAME] = "\"" . $cfg[LS_XML_IP2NAME] . "\";";

                        if (isset($cfg[LS_XML_TEMPLATE]) and !empty($cfg[LS_XML_TEMPLATE])) {
                            $tpl_val = $cfg[LS_XML_TEMPLATE];
                            // check template path
                            if (!file_exists(LS_TEMPLATEPATH."/$tpl_val")) $tpl_val = 'base';
                            $lsconf_var[LS_VAR_TEMPLATE] = "\"" . $tpl_val . "\";";
                        }

                        if (isset($cfg[LS_XML_BARCOLOR]) and !empty($cfg[LS_XML_BARCOLOR]))
                            $lsconf_var[LS_VAR_BARCOLOR] = "\"" . $cfg[LS_XML_BARCOLOR] . "\";";
                }

             $lsconf = "";
             $lsconf_file = LS_CONFIGPATH . "/" . LS_CONFIGFILE;
             // open lightsquid config
             if (file_exists($lsconf_file)) {
                 $lsconf = file_get_contents($lsconf_file);
                   update_log("Load config file $lsconf_file");
             } else {
                   update_log("Error loading config file $lsconf_file");
                 // or open from 'lightsquid.cfg.dist'
                 $lsconf_dist_file = LS_CONFIGPATH . "/" . LS_CONFIGFILE_DIST;
                 if (file_exists($lsconf_dist_file)) {
                           $lsconf = file_get_contents($lsconf_dist_file);
                           update_log("Load config dist. file $lsconf_dist_file");
                 } else    update_log("Error loading config dist. file $lsconf_dist_file");
             }

             // update lightsquid config
             if (!empty($lsconf)) {
                 $lsconf = explode("\n", $lsconf);
                 foreach ($lsconf_var as $key => $val) {
                          for($i = 0; $i < count($lsconf); $i++) {
                              $s = trim($lsconf[$i]);
                                   $e_key = "^[$]" . $key . "[ ]*[=]+";
#                                    update_log("Regular: eregi(\"$e_key," . "'$s')"); // debug regular template
                              if (eregi($e_key, $s)) {
#                                            update_log("Regular PASSED: eregi(\"$e_key," . "'$s')"); // debug regular template
                                  $lsconf[$i] = '$' . "$key = $val;";
                                        update_log("Update config: $key=$val");
                              }
                          }
                 }

                 $lsconf = implode("\n", $lsconf);
                 $fl = file_put_contents($lsconf_file, $lsconf);
                 update_log("Save config file $lsconf_file ($fl)");
             }

        // set shedule - refresh data job
		if ($tm) {
            $on = false;
            $opt = array("*", "*", "*", "*", "*", "root", CRONTAB_LS_TEMPLATE . " today");
            switch($tm) {
                  case 'lhp_none': $on = false; break;
                  case 'lhp_10m':  $on = true; $opt[0]= "*/10"; break;
                  case 'lhp_20m':  $on = true; $opt[0]= "*/20"; break;
                  case 'lhp_30m':  $on = true; $opt[0]= "*/30"; break;
                  case 'lhp_40m':  $on = true; $opt[0]= "*/40"; break;
                  case 'lhp_50m':  $on = true; $opt[0]= "*/50"; break;
                  case 'lhp_60m':  $on = true; $opt[0]= "*/60"; break;
                  case 'lhp_2h':   $on = true; $opt[0]= "0"; $opt[1]= "*/2";  break;
                  case 'lhp_3h':   $on = true; $opt[0]= "0"; $opt[1]= "*/3";  break;
                  case 'lhp_4h':   $on = true; $opt[0]= "0"; $opt[1]= "*/4";  break;
                  case 'lhp_6h':   $on = true; $opt[0]= "0"; $opt[1]= "*/6";  break;
                  case 'lhp_8h':   $on = true; $opt[0]= "0"; $opt[1]= "*/8";  break;
                  case 'lhp_12h':  $on = true; $opt[0]= "0"; $opt[1]= "*/12"; break;
                  case 'lhp_24h':  $on = true; $opt[0]= "0"; $opt[1]= "0"; $opt[2]= "*/24"; break;
            }
            ls_setup_cron("lightsquid_parser", $opt, CRONTAB_LS_JOBKEY, $on);
		}  else
            ls_setup_cron("lightsquid_parser", "", "", false);

        // set shedule - squid rotate
		if ($tm_squid) {
            $on = false;
            $opt = array("0", "0", "*", "*", "*", "root", CRONTAB_SQUID_TEMPLATE);
            switch($tm_squid) {
                    // day of month
                    case 'lsr_none':$on = false;     break;
                    case 'lsr_d1':  $on = true; $opt[2]= "*/1"; break;
                    case 'lsr_d2':  $on = true; $opt[2]= "*/2";  break;
                    case 'lsr_d3':  $on = true; $opt[2]= "*/3";  break;
                    case 'lsr_d4':  $on = true; $opt[2]= "*/4";  break;
                    case 'lsr_d5':  $on = true; $opt[2]= "*/5";  break;
                    case 'lsr_d6':  $on = true; $opt[2]= "*/6";  break;
                    case 'lsr_d10': $on = true; $opt[2]= "*/10"; break;
                    case 'lsr_d15': $on = true; $opt[2]= "*/15"; break;
                    case 'lsr_d20': $on = true; $opt[2]= "*/20"; break;
                    case 'lsr_d25': $on = true; $opt[2]= "*/25"; break;
                    case 'lsr_d30': $on = true; $opt[2]= "*/30"; break;
                    // day of week
                    case 'lsr_w1': $on = true; $opt[4]= "*/1"; break;
                    case 'lsr_w2': $on = true; $opt[4]= "*/1"; break;
                    case 'lsr_w3': $on = true; $opt[4]= "*/1"; break;
                    case 'lsr_w4': $on = true; $opt[4]= "*/1"; break;
                    case 'lsr_w5': $on = true; $opt[4]= "*/1"; break;
                    case 'lsr_w6': $on = true; $opt[4]= "*/1"; break;
                    case 'lsr_w7': $on = true; $opt[4]= "*/1"; break;
            }
            ls_setup_cron("lightsquid_squid_rotate", $opt, CRONTAB_SQUID_JOBKEY, $on);
		}  else
            ls_setup_cron("lightsquid_squid_rotate", "", "", false);
            
         // update squid conf
		if (isset($config['installedpackages']['squid']['config'][0])) {
			$config['installedpackages']['squid']['config'][0]['log_enabled'] = 'on';
			$config['installedpackages']['squid']['config'][0]['log_dir'] = LS_SQUIDLOGPATH;
			write_config();
			squid_resync();
		}
    }

    if ($_POST['Submit'] === 'Refresh now')  refresh_now();
    if ($_POST['Submit'] === 'Refresh full') refresh_full();
}

// setup cron tasks
// original source from '/etc/inc/pfsense-utils.inc' function 'tdr_install_cron'
// this function safe for other tasks
// *****************************************************************************
// - $task_name: cron task name (for config identification) /for searching my cron tasks/
// - $options:   array=[0:minute][1:hour][2:mday][3:month][4:wday][5:who][6:cmd]
// - $task_key:  cron command key for searching
// - $on_off:    true-'on task', false-'off' task
// required: $task_nameand $on_off
// *****************************************************************************
define('FIELD_TASKNAME', 'task_name');

function ls_setup_cron($task_name, $options, $task_key, $on_off) {
        global $config;
        update_log("ls_setup_cron: start task_name=$task_name, task_key=$task_key, on_off=$on_off");

        // check input params
        if(!$task_name) {
            update_log("ls_setup_cron: exit - uncomplete input params.");
            return;
        }
        // search cron config settings
        if(!$config['cron']['item']) {
            update_log("ls_setup_cron: exit - 'config.xml'->[cron]->[items] not found.");
            return;
        }

        // searching task
        $x_name='';
        $x=0;
        foreach($config['cron']['item'] as $item) {
            if($item[FIELD_TASKNAME] and $task_name and ($item[FIELD_TASKNAME]==$task_name)) {
               update_log("ls_setup_cron: found cron task with name=$task_name on [$x_name].");
               $x_name = $x;
            }
            $x++;
        }
        unset($x);

        // install cron:
        //  - if not found with such name and not found 'task_key', when install task
        //  - if found task with such name, when renew this item (delete and add new with all check's)
        // deinstall cron:
        //  - deinstall only, if found such name
        switch($on_off) {
                case true:
                     if($task_key) {
                          // searching task
                          $x=0;
                          $x_task='';
                          foreach($config['cron']['item'] as $item) {
                             if(strstr($item['command'], $task_key)) {
                                $x_task = $x;
                                update_log("ls_setup_cron: found cron task with key=$task_key on [$x].");
                             }
                             $x++;
                          }
                          unset($x);

                          if($x_task and (!$x_name or ($x_task != $x_name))) { // other task with $task_key alredy installed
                                   update_log("ls_setup_cron: can't add cron task, while such task exists $task_key");
                                   break;
                          } else {
                              if(is_array($options)) {

                                   // delete this item (by name)
                                   if($x_name > 0)
                                      unset($config['cron']['item'][$x_name]);
                                   // and add new
                                   $cron_item = array();
                                   $cron_item[FIELD_TASKNAME] = $task_name;
                                   $cron_item['minute']    = $options[0];
                                   $cron_item['hour']      = $options[1];
                                   $cron_item['mday']      = $options[2];
                                   $cron_item['month']     = $options[3];
                                   $cron_item['wday']      = $options[4];
                                   $cron_item['who']       = $options[5];
                                   $cron_item['command']   = $options[6];
                                   // check options
                                   if(!$cron_item['who']) $cron_item['who'] = "nobody";
                                   $config['cron']['item'][] = $cron_item;
                                   write_config("Installed cron task '$task_name' for 'lightsquid' package");
                                   configure_cron();
                                   // log
                                   update_log("ls_setup_cron: add cron task '$task_name'='" . $cron_item['command'] . "'");
                              }
                          }
                     } else
                          // log
                          update_log("ls_setup_cron: input prm 'task_key' not defined");
                break;
                case false:
                          // delete cron task with only name $task_name
                          if($x_name > 0) {
                             unset($config['cron']['item'][$x_name]);
                             write_config();
                             // log
                             update_log("ls_setup_cron: delete cron task '$task_name'");
                          }
                break;
        }
        configure_cron();
        update_log("ls_setup_cron: end");
}

function update_log($log) {
        if (LS_GUI_DEBUG === 'on') {
                $t_ls_log = '';
                if (file_exists(LS_LOG_FILE))
                        $t_ls_log = file_get_contents(LS_LOG_FILE);
                $t_ls_log .= "\n$log";
                file_put_contents(LS_LOG_FILE, $t_ls_log);
        }
}

function refresh_now() {
        $cmd = CRONTAB_LS_TEMPLATE . " today";
        update_log("refresh_now: execute command '$cmd'");
        mwexec_bg($cmd);
}

function refresh_full() {
        $cmd = CRONTAB_LS_TEMPLATE;
        $log_name = LS_SQUIDLOG;
        update_log("refresh_full: start");

        // parse access.log
        update_log("refresh_full: execute command '$cmd'");
        mwexec_bg("$cmd $lg");
        // parse access.log.x
        for ($i=0; $i<100; $i++) {
             $lg = LS_SQUIDLOG . ".$i";
             if (file_exists(LS_SQUIDLOGPATH . "/$lg")) {
                 update_log("refresh_full: execute command '$cmd $lg'");
                 mwexec_bg("$cmd $lg");
             } else
                 // really go <= 10 cycles
                 break;
        }
        update_log("refresh_full: stop");
}

?>