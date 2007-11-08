<?php

# ---------------------------------------------------------------------------------------------------------------------
# squidguard.inc
# (C)2006 Serg Dvoriancev
/*        Redistribution and use in source and binary forms, with or without
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

if ($_SERVER['SCRIPT_FILENAME'] == '/usr/local/www/packages/config/squidguard.inc') {
        exit;
}

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');
require_once('squidguard_cls.inc');
# ---------------------------------------------------------------------------------------------------------------------
define('SQUIDGUARD_CONFBASE', '/usr/local/etc/squid');
define('SQUIDGUARD_CONFFILE', '/squidguard.conf.test');
define('SQUIDGUARD_BINPATH',  '/usr/local/bin');
define('SQUIDGUARD_WORKDIR',  '/usr/local/etc/squidGuard');
define('SQUIDGUARD_DBHOME',   '/var/db/squidGuard');

define('SQUIDGUARD_WEBGUI_LOG',         '/squidguard_gui.log');
define('SQUIDGUARD_WEBGUI_HISTORY_LOG', '/squidguard_gui_history.log');

# ---------------------------------------------------------------------------------------------------------------------

define('FLD_NAME',        'name');
define('FLD_DEST',        'dest');
define('FLD_SOURCE',      'source');
define('FLD_DESTINATION', 'dest');
define('FLD_REWRITE',     'rewrite');
define('FLD_REDIRECT',    'redirect');
define('FLD_TIME',        'time');
define('FLD_OVERDESTINATION', 'overdestination');
define('FLD_OVERREWRITE',     'overrewrite');
define('FLD_OVERREDIRECT',    'overredirect');
define('FLD_TARGETURL',       'targeturl');
define('FLD_REPLACETO',       'replaceto');
define('FLD_TIMETYPE',        'timetype');
define('FLD_TIMEDAYS',        'timedays');
define('FLD_DATERANGE',       'daterange');
define('FLD_TIMERANGE',       'timerange');
define('FLD_IPLIST',          'iplist');
define('FLD_DESCRIPTION',     'description');
define('FLD_EXPRESSIONS',     'expressions');
define('FLD_DOMAINS',         'domains');
define('FLD_URLS',            'urls');
define('FLD_DISABLED',        'disabled');
define('FLD_ENABLELOG',       'enablelog');

define('PREFLD_UPTIME',        'uptime_');
define('PREFLD_UPTIME_DENY',   'uptimedeny_');
define('PREFLD_OVERTIME',      'overtime_');
define('PREFLD_OVERTIME_DENY', 'overtimedeny_');

define('MODULE_GENERAL',         'squidguardgeneral');
define('MODULE_DEFAULT',         'squidguarddefault');
define('MODULE_ACL',             'squidguardacl');
define('MODULE_DESTINATION',     'squidguarddest');
define('MODULE_REWRITE',         'squidguardrewrite');
define('MODULE_SOURCE',          'squidguardsrc');
define('MODULE_TIME',            'squidguardtime');
define('MODULE_LOG',             'squidguardlog');

define('BLACKLIST_DEFAULT_URL', 'http://ftp.teledanmark.no/pub/www/proxy/squidGuard/contrib/blacklists.tar.gz'); // 360Kb
//define('BLACKLIST_DEFAULT_URL', 'http://squidguard.mesd.k12.or.us/blacklists.tgz'); // 3.5Mb
// define('BLACKLIST_DEFAULT_URL', 'http://blacklist.dansguardian.org/cgi-bin/download.pl?type=download&file=bigblacklist');
define('BLACKLIST_TMP_FILE',    '/var/tmp/blacklists.tar.gz');
define('BLACKLIST_BTN_STD',     'Upload Std');
define('BLACKLIST_BTN_URL',     'Upload Url');
define('BLACKLIST_BTN_FILE',    'Upload File');
define('BLACKLIST_LOGFILE',     'blacklist.log');

define('APPLY_BTN', 'Apply');

define('WEBGUI_HISTORY_LOG', 'on');

define('TEST_LOG', '/var/tmp/sqtest.test');

# ---------------------------------------------------------------------------------------------------------------------
# squidguard_object
# ---------------------------------------------------------------------------------------------------------------------
$squidguard_object = create_squidguard_object();


# ---------------------------------------------------------------------------------------------------------------------
# functions
# ---------------------------------------------------------------------------------------------------------------------

function create_squidguard_object() {
        $sg_object = new SquidGuardConfigClass();
        $sg_object->debug_history .= "->create_squidguard_object";
        $sg_object->set_options(SQUIDGUARD_BINPATH, SQUIDGUARD_WORKDIR, SQUIDGUARD_DBHOME);

        squidguard_ini_object(&$sg_object);
        return $sg_object;
}

function squidguard_ini_object($sg_object) {
        global $config;

        $sg_object->debug_history .= "->squidguard_ini_object";
        $sg_object->init();
        if ($config['installedpackages'][MODULE_GENERAL]['config'][0]['squidguard_enable'] != '')
             $sg_object->enabled = true;
        else $sg_object->enabled = false;

        if ($config['installedpackages'][MODULE_GENERAL]['config'][0]['blacklist'] != '')
             $sg_object->blacklist_enabled = true;
        else $sg_object->blacklist_enabled = false;

        // rewrite's
        $rewrite = $config['installedpackages'][MODULE_REWRITE]; // ['config'];
        if (is_array($rewrite)) {
                foreach($rewrite['config'] as $rew) {
                        // rewrite object
                        $sg_rew =& $sg_object->add_rewrite($rew[FLD_NAME], $rew[FLD_DESCRIPTION], $rew[FLD_ENABLELOG]);
                        if (is_array($rew) and count($rew['row'])) {
                            foreach($rew['row'] as $row) {
                                // rewrite items
                                $sg_rew->add_item($row[FLD_TARGETURL], $row[FLD_REPLACETO]);
                            }
                        }
                }
        }

        // time's
        $time = $config['installedpackages'][MODULE_TIME]['config'];
        if (is_array($time)) {
                foreach($time as $tm) {
                        // time object
                        $sg_time =& $sg_object->add_time($tm[FLD_NAME], $tm[FLD_DESCRIPTION]);
                        foreach($tm['row'] as $row) {
                                // time items
                                if ($row[FLD_TIMETYPE] == 'date') // 0 - "date"$FLD_TIMETYPE
                                     $sg_time->add_date   ($row[FLD_DATERANGE], $row[FLD_TIMERANGE]);
                                else $sg_time->add_weekly ($row[FLD_TIMEDAYS], /* $row[FLD_DATERANGE], */$row[FLD_TIMERANGE]);
                        }
                }
        }

        // source's
        $source = $config['installedpackages'][MODULE_SOURCE]['config'];
        if (is_array($source)) {
                foreach($source as $src) {
                        // source object
                        $sg_src =& $sg_object->add_src($src[FLD_NAME], $src[FLD_IPLIST],
                                   $src[FLD_DESCRIPTION], $src[FLD_ENABLELOG]);
                }
        }

        // destination's
        $destination = $config['installedpackages'][MODULE_DESTINATION]['config'];
        if (is_array($destination)) {
                foreach($destination as $dest) {
                        // destination object
                        $sg_ds =& $sg_object->add_dest($dest[FLD_NAME], $dest[FLD_DOMAINS], $dest[FLD_URLS],
                                  $dest[FLD_EXPRESSIONS], $dest[FLD_REDIRECT], $dest[FLD_DESCRIPTION],
                                  $dest[FLD_ENABLELOG]);
                }
        }

        // acl's
        $acl = $config['installedpackages'][MODULE_ACL]['config'];
        if (is_array($acl)) {
                foreach($acl as $ac) {
                        $passes = str_replace("]", "", trim($ac[FLD_DESTINATION]));
                        $passes = explode("[", $passes);
                        $pass     = "";
                        $overpass = "";
                        if (is_array($passes)) {
                                $pass = $passes[0];
                                $overpass = $passes[1];
                        }
                        // acl object; log defined only for default acl
                        $sg_acl =& $sg_object->add_acl(
                                $ac[FLD_SOURCE], $ac[FLD_TIME], $ac[FLD_DESCRIPTION],
                                array('pass' => $pass,     'rewrite'=>$ac[FLD_REWRITE], 'redirect'=>$ac[FLD_REDIRECT], 'log'=>''),
                                array('pass' => $overpass, 'rewrite'=>$ac[FLD_REWRITE], 'redirect'=>$ac[FLD_REDIRECT], 'log'=>''),
                                $ac[FLD_DISABLED]
                        );
                }
        }

        // default acl; log defined only for default acl
        $acdef = $config['installedpackages'][MODULE_DEFAULT]['config'][0];
        if (is_array($acdef)) {
            $passes = str_replace("]", "", trim($acdef[FLD_DESTINATION]));
            $passes = explode("[", $passes);
            $pass     = "";
            $overpass = "";
            if (is_array($passes)) {
                $pass = $passes[0];
                $overpass = $passes[1];
            }
            // default acl object
            $sg_object->set_default_acl(
                $acdef[FLD_TIME], $acdef[FLD_DESCRIPTION],
                array('pass' => $pass,     'rewrite'=>$acdef[FLD_REWRITE], 'redirect'=>$acdef[FLD_REDIRECT], 'log'=>$acdef[FLD_ENABLELOG]),
                array('pass' => $overpass, 'rewrite'=>$acdef[FLD_REWRITE], 'redirect'=>$acdef[FLD_REDIRECT], 'log'=>$acdef[FLD_ENABLELOG]),
                $acdef[FLD_DISABLED]
            );
        }

        // update squidguard object
        $sg_object->update();
}


# ---------------------------------------------------------------------------------------------------------------------
# install/deinstall
# ---------------------------------------------------------------------------------------------------------------------
function squidguard_deinstall() {

}

# ---------------------------------------------------------------------------------------------------------------------
# Upload file to /var/tmp directory
# ---------------------------------------------------------------------------------------------------------------------
function upload_file($file) {
        global $squidguard_object;

        $res = '';
        $cat = '/var/tmp/';

$squidguard_object->add_log("FILES: " . count($_FILES));

        if (isset($_FILES[$file])) {        // if file exists
                if (move_uploaded_file($_FILES[$file]['tmp_name'], $cat . $_FILES[$file]['name'])) {
                        $res = true;
                } // else  blacklist_add_log('Нифига нет Files=' . count($_FILES));
        }
        return $res;
}


# ---------------------------------------------------------------------------------------------------------------------
# default squidguard blacklist
# http://ftp.teledanmark.no/pub/www/proxy/squidGuard/contrib/blacklists.tar.gz
# ---------------------------------------------------------------------------------------------------------------------
// upload file and put them to temp catalog
function upload_file_from_url($url_file, $destination_file, $proxy) {
        global $squidguard_object;

        // open destination file
        $upload_tmp = '/var/tmp/blacklists.tar.gz';
        $result = '';
        $squidguard_object->add_log("Begin upload from URL $url_file");

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL,BLACKLIST_DEFAULT_URL);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);
        if ($proxy != '') {
            $ip    = '';
            $login = '';
            $s     = trim($proxy);
            if (strpos($s, ' ')) {
                       $ip    = substr($s, 0, strpos($s, ' '));
                       $login = substr($s, strpos($s, ' ') + 1);
            } else     $ip    = $s;

            if($ip != '') {
               $s_log = "Host[$ip] ";
               curl_setopt($ch, CURLOPT_PROXY, $ip);
               if($login != '') {
                  $s_log .= "Login[$login]";
                  curl_setopt($ch, CURLOPT_PROXYUSERPWD, $login);
               }
               $squidguard_object->add_log("Proxy settings: $s_log");
            }
        } else $squidguard_object->add_log("Proxy: not defined");
        $result=curl_exec ($ch);
        curl_close ($ch);

        // TODO: check uploaded content for '.gz' file format

        if (file_put_contents($upload_tmp, $result) > 0) {
               $squidguard_object->add_log("Uploaded success.");
        } else $squidguard_object->add_log("Upload error.");
        return $upload_tmp;
}

# ---------------------------------------------------------------------------------------------------------------------
# Validates
# ---------------------------------------------------------------------------------------------------------------------
function squidguard_validate_default($post, $input_errors) {
        squidguard_validate_acl($post, &$input_errors);
}

function squidguard_validate_acl($post, $input_errors) {
        global $config;
        $pass_up = array();
        $deny_up = array();
        $pass_up_val = '';
        $pass_over = array();
        $deny_over = array();
        $pass_over_val = '';

        // ===== store destinations to 'dest' value =====
        foreach ($post as $key => $val) {
                if (substr_count($key, PREFLD_UPTIME) != 0) {
                        $name = str_replace(PREFLD_UPTIME, '', $key);
                        if ($name)
                            $pass_up[$name] = 'on';
                }
                elseif (substr_count($key, PREFLD_UPTIME_DENY) != 0) {
                        $name = str_replace(PREFLD_UPTIME_DENY, '', $key);
                        if ($name)
                            $deny_up[$name] = 'on';
                }
                elseif (substr_count($key, PREFLD_OVERTIME) != 0) {
                        $name = str_replace(PREFLD_OVERTIME, '', $key);
                        if ($name)
                            $pass_over[$name] = 'on';
                }
                elseif (substr_count($key, PREFLD_OVERTIME_DENY) != 0) {
                        $name = str_replace(PREFLD_OVERTIME_DENY, '', $key);
                        if ($name)
                            $deny_over[$name] = 'on';
                }
        }

        foreach ($pass_up as $key => $val) {
                if ($deny_up[$key])
                     $pass_up_val .= " !$key";
                else $pass_up_val .= " $key";
        }

        foreach ($pass_over as $key => $val) {
                if ($deny_over[$key])
                     $pass_over_val .= " !$key";
                else $pass_over_val .= " $key";
        }

        // if not exists key 'all', then add '!all' - default 'deny all' (equiqalence 'none')
        if ((substr_count($pass_up_val, 'all') == 0)) {
             $pass_up_val .= ' !all';
        }

        if (!empty($pass_over_val) && (substr_count($pass_over_val, 'all') == 0)) {
             $pass_over_val .= ' !all';
        }

        if (empty($pass_over_val))
             $post[FLD_DEST] = "$pass_up_val";
        else $post[FLD_DEST] = "$pass_up_val [$pass_over_val]";
}

function squidguard_validate_times($post, $input_errors) {

         // check name
         $name = trim($post[FLD_NAME]);
         if(!empty($name)) {
             $err = check_name($name);
             if (!empty($err))
                  $input_errors[] = $err;
         }

          // check unique name
          if (!check_unique_name(MODULE_TIME, $name))
                $input_errors[] = "Name '$name' already exists";

         # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         # Format:
         #    date:   <date(or range)><time (or range)> -- days not parsed (reset to *)
         #    weekly: <day or *><time or range>         -- dates not parsed (reset to '')
         # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

         $err = '';
         for ($i=0; $post[FLD_TIMETYPE . "$i"] != ''; $i++) {
              $timetype = strtolower($post[FLD_TIMETYPE . "$i"]);
              if ($timetype == 'date') {
                  // set timedays to '*'
                  $post[FLD_TIMEDAYS . "$i"] = '*';

                  // check date and time
                  $date = trim($post[FLD_DATERANGE."$i"]);
                  $time = trim($post[FLD_TIMERANGE."$i"]);
                  $err  = check_date($date);
                  if (!empty($err)) $input_errors[] = $err;
                  $err  = check_time($time);
                  if (!empty($err)) $input_errors[] = $err;
              } else {
                  // set daterange to ''
                  $post[FLD_DATERANGE . "$i"] = '';

                  // check time
                  $time = trim($post[FLD_TIMERANGE."$i"]);
                  $err  = check_time($time);
                  if (!empty($err)) $input_errors[] = $err;
              }
        }

        if (!empty($err))
            $input_errors .= $err;
}

function squidguard_validate_source($post, $input_errors) {
         // check name
         $name = trim($post[FLD_NAME]);
         if(!empty($name)) {
             $err = check_name($name);        // validate name content
             if (!empty($err))
                  $input_errors[] = $err;

                  // check unique name
                  if (!check_unique_name(MODULE_SOURCE, $name))
                        $input_errors[] = "Name '$name' already exists";
         }
}

function squidguard_validate_destination($post, $input_errors) {
         // check name
         $name = trim($post[FLD_NAME]);
         if(!empty($name)) {
             $err = check_name($name);        // validate name content
             if (!empty($err))
                  $input_errors[] = $err;

                  // check unique name
                  if (!check_unique_name(MODULE_DESTINATION, $name))
                        $input_errors[] = "Name '$name' already exists";
         }
}

function squidguard_validate_rewrite($post, $input_errors) {
         // check name
         $name = trim($post[FLD_NAME]);
         if(!empty($name)) {
              // check name format <char><symbols without space> - Ab123
             $err = check_name($name);
             if (!empty($err))
                  $input_errors[] = $err;

                // check unique name
                if (!check_unique_name(MODULE_REWRITE, $name))
                        $input_errors[] = "Name '$name' already exists";
         }
}

// ---------------------------------------------------------------------------------------------------------------------
// Resync & Make
// ---------------------------------------------------------------------------------------------------------------------

function squidguard_resync() {
        global $squidguard_object;
        $sg_object;
        if (is_null($squidguard_object)) {
            $squidguard_object = create_squidguard_object();
        }
        $sg_object = $squidguard_object;
        $sg_object->debug_history .= "->squidguard_resync";
        squidguard_ini_object(&$sg_object);

        $conf = '';
        $upload_file = '';

        if ($_POST['submit'] == BLACKLIST_BTN_STD) {
            // upload from std url
            $proxy = $_POST['blacklist_proxy'];
            $upload_file = upload_file_from_url(BLACKLIST_DEFAULT_URL, BLACKLIST_TMP_FILE, $proxy);
            $sg_object->update_blacklist($upload_file);
        } else

        if ($_POST['submit'] == BLACKLIST_BTN_URL) {
            // upload from another url
            $url = $_POST['blacklist_url'];
            $proxy = $_POST['blacklist_proxy'];
            if ($url)
                $upload_file = upload_file_from_url($url, BLACKLIST_TMP_FILE, $proxy);
            $sg_object->update_blacklist($upload_file);
        } else

        if ($_POST['submit'] == BLACKLIST_BTN_FILE) {
       /*     $upload_file = $_POST['blacklist_file'];
             blacklist_add_log('Begin upload from file ' . $upload_file . '.\r');
             if (upload_file(''))
                   blacklist_add_log('Success.\r');
             else {
                   blacklist_add_log('Error.\r');
                   $upload_file = '';
             }
             $sg_object->update_blacklist($upload_file); */
 //            $sg_object->update_blacklist('/var/tmp/blacklists.tar');
upload_file('');
        } else

        // apply changes
        if ($_POST['submit'] == APPLY_BTN) {
                // reconfigure service
                $sg_object->reconfigure();
        } else

        // start test squidGuard
        /*if ($_POST['test_squidguard'] != '')*/ {
                  $result = '';
                  $src_urls = $_POST['test_squidguard_urls'];
                  $result = $sg_object->test_work($src_urls);
                  file_put_contents(TEST_LOG, $result);
        }

}
// -----------------------------------------------------------------------------
// common functions
// -----------------------------------------------------------------------------
function get_pkg_items_list($pkg_gui_name, $fieldname) {
       $res = '';
       global $config;
            $conf = $config['installedpackages'][$pkg_gui_name]['config'];
        foreach($conf as $cf)
                $res[] = $cf[$fieldname];
        return $res;
}

// -----------------------------------------------------------------------------
// Resync
// -----------------------------------------------------------------------------

function squidguard_resync_rewrite() {    // rewrite
    $conf = "";
    // ----- nothing for do -----
    return $conf;
}

function squidguard_resync_time() {       // resync time
    global $config;

    $conf = '';
    $times = $config['installedpackages']['squidguardtime']['config'];

    if (is_array($times)) {
        foreach($times as $tm) {
            $timevalue = '';
            $conf .= "\n time " . $tm['timename'] . " {";
            foreach($tm['row'] as $rw) {
                if ($rw['timetype'] == 'date') {
                    $timevalue .= $rw['timetype'] . ' ' . $rw['timelist'];
                    $conf .= "\n        date" . $rw['timelist'];
                } else {
                    $timevalue .= $rw['timetype'] . ' ' . $rw['timedays'] . ' ' . $rw['timelist'];
                    $conf .= "\n        weekly " . $rw['timedays'] . ' ' . $rw['timelist'];
                }
            }
            $conf .= "\n    } \n";
            $config['installedpackages']['squidguardtime']['config']['timevalue'] = "1110";//$timevalue;
        }
    }

//    file_put_contents("/var/tmp/squidguard.test", $conf);

    return $conf;
}

function squidguard_resync_src() {        // source
    $conf = "";
    // ----- nothing for do -----
    return $conf;
}

function squidguard_resync_std_dest() {    // standart destinations
    global $config;
    $settings = $config['installedpackages']['squidguard']['config'][0];
    $conf = "";

    // ads
    $conf .= make_dest(FLT_ADS, "ads/domains", "ads/urls", "", "");
    // aggressive
    $conf .= make_dest(FLT_AGGRESSIVE, "aggressive/domains", "aggressive/urls", "", "");
    // audio & video
    $conf .= make_dest(FLT_AUDIOVIDEO, "audio-video/domains", "audio-video/urls", "", "");
    // Druggs
    $conf .= make_dest(FLT_DRUGGS, "drugs/domains", "drugs/urls", "", "");
    // Gambling
    $conf .= make_dest(FLT_GAMBLING, "gambling/domains", "gambling/urls", "", "");
    // Hacking
    $conf .= make_dest(FLT_HACKING, "hacking/domains", "hacking/urls", "", "");
    // Mail
    $conf .= make_dest(FLT_MAIL, "mail/domains", "mail/urls", "", "");
    // Porn
    $conf .= make_dest(FLT_PORN, "porn/domains", "porn/urls", "", "");
    // proxy
    $conf .= make_dest(FLT_PROXY, "proxy/domains", "proxy/urls", "", "");
    // Violence
    $conf .= make_dest(FLT_VIOLENCE, "violence/domains", "violence/urls", "", "");
    // Warez
    $conf .= make_dest(FLT_WAREZ, "warez/domains", "warez/urls", "", "");

    return $conf;
}

function squidguard_resync_user_dst() {    // user destinations
}

function squidguard_resync_dest() {       // destination
    $conf = "";
    // ----- nothing for do -----
    return $conf;
}

function squidguard_resync_acl() {        // acl
    $conf = "";
    // ----- nothing for do -----
    return $conf;
}

// -----------------------------------------------------------------------------
// Before form
// -----------------------------------------------------------------------------

function squidguard_before_form_dest($pkg) {
         global $config;
         $expr_names = '';
         $expressions = '';

         // update expressions list
         $expressions = $config['installedpackages']['squidguardexpr']['config'];
         if (is_array($expressions )) {
                 foreach($expressions as $expr)
                        $expr_names[] = $expr['name'];
         }

        $i = 0;
        foreach($pkg['fields']['field'] as $field) {
                if ($field['fieldname'] == 'exprname') {
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['options']['option'][] = array('name'=>'none (expressions not defined)', 'value'=>'');
                        foreach ($expr_names as $nm) {
                                $fld['options']['option'][] = array('name'=>$nm, 'value'=>$nm);
                        }
                        break;
                }
                $i++;
        }
}

function squidguard_before_form($pkg) {
        $i=0;

// <encType>multipart/form-data</encType>;

        foreach($pkg['fields']['field'] as $field) {
                // blacklist controls
                if ($field['fieldname'] == 'blacklist') {
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['description'] .= make_grid_blacklist();        // insert to description custom controls
                }
                // Apply button
                if ($field['fieldname'] == 'squidguard_enable'){
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['description'] .= make_grid_general_items();   // insert to description custom controls
                }
                // GUI log
                if ($field['fieldname'] == 'view_gui_log') {
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['description'] .= make_grid_general_items('gui_log');   // insert to description custom controls
                }
                $i++;
        }
}


function squidguard_before_form_acl($pkg) {
        global $config;
        global $squidguard_object;
        $current_id = '';

        $sources      = '';
        $source_items = '';
        $destinations = '';
        $dest_items   = '';
        $rewrites     = '';
        $rewr_names   = '';
        $times        = '';
        $time_names   = '';
        $acls_up      = '';
        $acls_over    = '';

        $current_id = $_GET['id'];
        if ($current_id == '')
            $current_id = $_POST['id'];
        if ($current_id == '')
            $current_id = 0;

        // sources
        $sources = $config['installedpackages']['squidguardsrc']['config'];
        if (is_array($sources)) {
                foreach($sources as $src) {
                        $source_items[] = $src['name'];
                }
        }

        // generate sources list TODO: exclude used names from list, source name used in ACL unique
        $i=0;
        foreach($pkg['fields']['field'] as $field) {
                if ($field['fieldname'] == 'source') {
                        $fld = &$pkg['fields']['field'][$i];
                        foreach($source_items as $nm) {
                                $fld['options']['option'][] = array('name'=>$nm, 'value'=>$nm);
                        }
                }
                $i++;
        }

        // destinations
        // acls pass ---> prepare data for destinations; dest format 'uptime_dests_list [overtime_dests_list]'
        $acl_dest = '';
        if ($pkg['name'] == MODULE_DEFAULT) {
             $current_id = 0;
             $acl_dest = $config['installedpackages'][MODULE_DEFAULT]['config'][$current_id]['dest'];
        }
        else $acl_dest = $config['installedpackages'][MODULE_ACL]['config'][$current_id]['dest'];

        if ($acl_dest) {
                $overtime = '';
                $pss = str_replace(']', '', $acl_dest);
                $pss = explode(' ', $pss);
                foreach($pss as $val) {
                        $name = str_replace('!', '', $val);
                        if ($val == '[')     { $overtime = 'yes'; continue; }
                        elseif ($val == ']') { break; }
                        if (empty($overtime)) {
                                if($val[0] != '!')
                                     $acls_up[$name] = 'pass';
                                else $acls_up[$name] = 'deny';
                        } else {
                                if($val[0] != '!')
                                     $acls_over[$name] = 'pass';
                                else $acls_over[$name] = 'deny';
                        }
                }
         }

        # --------------------------------------------------------------------------------------------------------------
        # destinations
        # --------------------------------------------------------------------------------------------------------------
        if ($squidguard_object->blacklist_enabled) {
            foreach($squidguard_object->blacklist as $dst)
                $dest_items[] = array ('name'=>$dst->name, 'upt_value'=>$acls_up[$dst->name],
                                'ovt_value'=>$acls_over[$dst->name], 'description'=>$dst->desc);
        }

        if ($squidguard_object->dests) {
            foreach($squidguard_object->dests as $dst)
                $dest_items[] = array ('name'=>$dst->name, 'upt_value'=>$acls_up[$dst->name],
                                 'ovt_value'=>$acls_over[$dst->name], 'description'=>$dst->desc);
        }

        // default all
        $dest_items[] = array('name'=>FLT_DEFAULT_ALL, 'upt_value'=>$acls_up[FLT_DEFAULT_ALL],
                        'ovt_value'=>$acls_over[FLT_DEFAULT_ALL], 'description'=>'Default access');

        $i=0;
        foreach($pkg['fields']['field'] as $field) {
                if (($field['fieldname'] == 'dest')/* || ($field['fieldname'] == 'overdest')*/) {
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['description'] .= make_grid_controls('checkbox', $dest_items);        // insert to description custom controls
                }
                $i++;
        }

        // rewrites
        $rewrites = $config['installedpackages'][MODULE_REWRITE]['config'];
        if (is_array($rewrites)) {
                foreach($rewrites as $rewr) {
                        $rewr_names[] = $rewr['name'];
                }
        }

        $i=0;
        foreach($pkg['fields']['field'] as $field) {
                if (($field['fieldname'] == 'rewrite') || ($field['fieldname'] == 'overrewrite')) {
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['options']['option'][] = array('name'=>'none (rewrite not defined)', 'value'=>'');
                        if (is_array($rewr_names)) {
                            foreach($rewr_names as $nm)
                                $fld['options']['option'][] = array('name'=>$nm, 'value'=>$nm);
                        }
                }
                $i++;
        }

        // times
        $times = $config['installedpackages']['squidguardtime']['config'];
        if (is_array($times)) {
                foreach($times as $tm) {
                        $time_names[] = $tm['name'];
                }
        }

        $i=0;
        foreach($pkg['fields']['field'] as $field) {
                if ($field['fieldname'] == 'time') {
                        $fld = &$pkg['fields']['field'][$i];
                        $fld['options']['option'][] = array('name'=>'none (time not defined)', 'value'=>'');
                        if (is_array($time_names)) {
                                foreach($time_names as $nm)
                                $fld['options']['option'][] = array('name'=>$nm, 'value'=>$nm);
                        }
                        break;
                }
                $i++;
        }
}

function squidguard_before_form_log($pkg) {
        global $config;
        global $squidguard_object;
        $i=0;
        foreach($pkg['fields']['field'] as $field) {
                if ($field['fieldname'] == 'logtype') {
                        $slog = '';
                        $mlog = $config['installedpackages'][MODULE_LOG]['config'][0];

                        $filename = '';
                        $fld = &$pkg['fields']['field'][$i];
                        if ($mlog['logtype'] == 'access_log') {
                            $filename = $squidguard_object->log_dir . "/" . SQUIDGUARD_ACCESSBLOCK_FILE;
                            if (file_exists($filename)) {
                                $s = file_get_contents($filename);
                                $s = explode("\n", $s);

                                $slog .= "<table cellSpacing=1 cellPadding=1 width='100%'>";
                                $slog .= "<tr><th>DateTime</th><th>IP</th><th>URL</th><th>Filter</th><th>ID</th><th>Method</th></tr>";
                                foreach($s as $vs) {
                                    $slog .= "<tr>";
                                    $sx = $vs;
                                    $sx = str_replace("/- -", "", $sx);
                                    $sx = str_replace("/-", "", $sx);
                                    $sx = str_replace("Request", "", $sx);
                                    $sx = str_replace("request", "", $sx);
                                    $sx = str_replace("(", "", $sx);
                                    $sx = str_replace(")", "", $sx);
                                    $sx = str_replace("[", "", $sx);
                                    $sx = str_replace("]", "", $sx);
                                    $sx = explode(" ", $sx);
                                    if (!empty($vs))
                                        $slog .= "<tr>
                                              <td style='vertical-align: top; white-space: nowrap;'>$sx[0] $sx[1]</td>
                                              <td style='vertical-align: top;'>$sx[5]</td>
                                              <td style='vertical-align: top;'>$sx[4]</td>
                                              <td style='vertical-align: top;'>$sx[3]</td>
                                              <td style='vertical-align: top;'>$sx[2]</td>
                                              <td style='vertical-align: top;'>$sx[6]</td></tr>";
                                }
                                $slog .= "</table>";
                            }
                        } else
                        if ($mlog['logtype'] == 'webgui_log') {
                            $filename = $squidguard_object->log_dir . SQUIDGUARD_LOG_FILE;
                            if (file_exists($filename)) {
                                $slog .= file_get_contents($filename);
                                $slog = str_replace("\n", "<br>", $slog);
                                $slog = "<tr><td>$slog</td></tr>";
                            }
                        } else
                        if ($mlog['logtype'] == 'squidguard_log') {
                            $filename = $squidguard_object->log_dir . "/squidGuard.log";
                            if (file_exists($filename)) {
                                $slog .= file_get_contents($filename);
                                $slog = str_replace("\n", "<br>", $slog);
                                $slog = "<tr><td>$slog</td></tr>";
                            }
                        }

// $slog .= $filename . "\n"; // debug

                        $fld['description'] .=
                            "</tr><tr><td><br></td>
                             <td><table width='100%' class=tabcont cellSpacing=0 cellPadding=1 border=1>$slog</table>";
//                            "<table class=tabcont cellSpacing=0 cellPadding=0 border=1 width='100%'>$slog</table></td>";
                }
                $i++;
        }
}

function squidguard_update_acl_dest($acl_name, $post) {
        global $config;
        $post['dest'] = 'fig vam';
        #s='TST';
        foreach($post as $k => $p)
                $s .= $k . '=' . $p;
}

function make_grid_general_items($id = '')
{
        global $squidguard_object;
        global $config;

        $res = '';
        $res .= "<table width='100%'>";

        if ($id == '') {
            // Apply
            $res .= "<tr><td>After changing configuration Squid or squidGuard you must
                     <b>apply all changes</b></td><td><input name='submit'
                     type='submit' value='Apply'></td></tr>";
        } else
        if ($id == 'gui_log') {
            if ($config['installedpackages']['squidguardgeneral']['config'][0]['view_gui_log'] == 'on') {
                $log_content = $squidguard_object->get_log();
                $res .= "<tr><td><br></td></tr>";
                $res .= "<tr><td>Web GUI log</td></tr>";
                $res .= "<tr><td vAlign=top width='100%'>
                         <textarea name='Name' rows=7 cols=65 wrap='on' readonly='on'>$log_content</textarea></td></tr>";
            }
        }

        $res .= "</table>";
        return $res;
}

function make_grid_blacklist() {
        global $squidguard_object;

        $style1 = 'class=vtable'; // 'class=vncell' ;// 'style="background-color: #FFF0E2;"';
        $style2 = 'class=vtable';
        $style2 = '';
        $e_size = '80';

        $res = '';
        $res .= "<table width='100%'>";
        $res .= "<tr><td><b>Proxy</b> settings</b></td> <td>Blacklist upload proxy - enter here, or leave blank.<br>
                Format: host:[port login:pass] . Default proxy port 1080. Example: '192.168.0.1:8080 user:pass'</td><td>&nbsp;</td></tr>";
        $res .= "<tr><td $style1>&nbsp;</td><td $style1><input name='blacklist_proxy' id='blacklist_proxy' type='text' size='$e_size'></td><td $style1><br></td></tr>";
        $res .= "<tr><td $style3>Load from <b>Default</b></td> <td $style3><b>" . /*BLACKLIST_DEFAULT_URL .*/ "</b></td> <td $style3><input name='submit' value='" .
                BLACKLIST_BTN_STD . "' type='submit'></td></tr>";
        $res .= "<tr><td $style3>Load from <b>Url </b></td> <td $style3><input name='blacklist_url'  id='blacklist_url'
                type='text' size='$e_size'></td> <td $style3><input name='submit' value='" . BLACKLIST_BTN_URL  . "' type='submit'>
                </td></tr>";
//        $res .= "<tr><td $style1>Load from <b>File</b></td> <td $style1><input name='blacklist_file' id='blacklist_file'
//                type='file' size='$e_size'></td> <td $style1><input name='submit' value='" . BLACKLIST_BTN_FILE .
//                "' type='submit'></td></tr>";
        $res .= "</table>";
        return $res;
}

function make_grid_controls($type, $items) {
        $res = '';
        $tbl = '';
        $color = '';
        $color2 = '';
        $x = 0;

        foreach($items as $item) {
                if ($x == 0) {
                        $color = 'style="background-color: #FFF0E2;"';
                        $color2 = '';
                        $x = 1;
                } else {
                        $color = '';
                        $color2 = 'style="background-color: #FFF0E2;"';
                        $x = 0;
                }

                $name    = $item['name'];
                $upt_val = $item['upt_value'];
                $ovt_val = $item['ovt_value'];
                $description = $item['description'];
                $upt_on   = '';
                $upt_deny = '';
                $ovt_on   = '';
                $ovt_deny = '';

                if ($upt_val) $upt_on = 'CHECKED';
                if ($upt_val == 'deny') $upt_deny = 'CHECKED';

                if ($ovt_val) $ovt_on = 'CHECKED';
                if ($ovt_val == 'deny') $ovt_deny = 'CHECKED';

                $tbl .= "<tr>";
                $tbl .= "<td $color><input type=\"$type\" name=\"" . PREFLD_UPTIME . "$name\" $upt_on></td>";
                $tbl .= "<td width=\"35%\" $color >$description [$name]</td>";
                $tbl .= "<td $color><input type=\"$type\" name=\"" . PREFLD_UPTIME_DENY . "$name\" $upt_deny></td>";
                $tbl .= "<td $color>deny access</td>";

                $tbl .= "<td $color><input type=\"$type\" name=\"" . PREFLD_OVERTIME . "$name\" $ovt_on></td>";
                $tbl .= "<td $color width=\"35%\" >$description [$name]</td>";
                $tbl .= "<td $color><input type=\"$type\" name=\"" . PREFLD_OVERTIME_DENY . "$name\" $ovt_deny></td>";
                $tbl .= "<td $color>deny access</td>";
                $tbl .= "</tr>";
        }

        if (!empty($tbl)) {
                $tbl = "<th/><th>Destinations in uptime</th><th/><th/><th/><th>Destinations in overtime</th><th/><th/>" .
                        "<tr><td/><td>Select destinations for uptime rule</td><td/><td/>
                             <td/><td>Select destination for overtime rule.<br>If time not defined, this part must be ignored</td><td/><td/></tr>"
                        . $tbl;

                $res .= "<table>" . $tbl . "</table>";
        }
        return $res;
}

// -----------------------------------------------------------------------------

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

// --- Check unique name ---
function check_unique_name($module_id, $name, $log='') {
        $res = true;
       $count_names = 0;
        $is_self ='notself';

        $id = $_GET['id'];
        if (!isset($id))
                $id = $_POST['id'];

        $name_list = get_pkg_items_list($module_id, 'name');
        $name_list_count = count($name_list);

        for($j=0; $j<$name_list_count; $j++) {
                $val = $name_list[$j];
                if ($val == $name)        {
                        $count_names++;
                        if ($j==$id) $is_self='self';
                }
        }

        // if count names = 1, then check if add new record with this name(not valid) / or this is a self record(valid)
        // else if count names > 1 - not valid
        if ($count_names == 1) {
                if ($id == name_list_count) { // this new record
                        $res = false;
                        $log = "*ID*";
                }
                elseif($is_self != 'self') {        // not self record
                        $res = false;
                        $log = '*NOT SELF*';
                }
        }
        elseif($count_names > 1) {
                $res = false;
                $log = '*COUNT*';
        }
        $log .= " id=$id is_self=$is_self count=$name_list_count";
        return $res;
}

function check_date($date) {
        $err = '';
        $val = trim($date);
        // date or date range format: 'yyyy-mm-dd', 'yyyy-m-d', 'yyyy.mm.dd'  'yyyy.mm.dd-yyyy.mm.dd'
        if (!eregi("^([0-9*]{4})\.([0-9*]{1,2})\.([0-9*]{1,2})$", $val) &&
            !eregi("^([0-9*]{4})\.([0-9*]{1,2})\.([0-9*]{1,2})-([0-9*]{4})\.([0-9*]{1,2})\.([0-9*]{1,2})$", $val))
            $err .= " Invalid date " . $date .
                    " You mast use date or date range format: 'yyyy.mm.dd' and 'yyyy.mm.dd-yyyy.mm.dd' .
                      Any symbol in date can be set to *(any number). Example: '*-10-01', '199*-*1-*1' ." . $val ;
        return $err;
}

function check_time($time) {
        $err = '';
        $val = trim($time);
        // time range format: 'HH:MM-HH:MM'
        if (!eregi("^([0-9]{2})\:([0-9]{2})-([0-9]{2})\:([0-9]{2})$", $val))
            $err = '';//" Invalid time range time . You must use 'HH:MM-HH:MM' time range format. ";
        return $err;
}

function check_ip($ip) {

}


function squidguard_print_javascript_main() {
}

# ---------------------------------------------------------------------------------------------------------------------
# Install & deinstall
# ---------------------------------------------------------------------------------------------------------------------

function squidguard_install_command() {
        global $squidguard_object;
}

function squidguard_deinstall_command() {
        mwexec('rm -rf ' . SQUIDGUARD_WORKDIR);
        mwexec('rm -rf ' . SQUIDGUARD_DBHOME);
}

# ---------------------------------------------------------------------------------------------------------------------
# SquidGuard print JavaSrcript
# ---------------------------------------------------------------------------------------------------------------------

function squidGuard_print_javascript() {
        $javascript = '';

        $xml = $_GET["xml"];
        if ($xml == "")
            $xml = $_POST["xml"];

        // squidguard_default.xml
        if ($xml == "squidguard_default.xml") {
            $javascript .= "\n<script language='JavaScript'>";
            $javascript .= "\n<!--";
            $javascript .= "\n    document.iform.dest.disabled=1;";
            $javascript .= "\n//-->";
            $javascript .= "\n</script>";
        } // if

        // squidguard_acl.xml
        if ($xml == "squidguard_acl.xml") {
            $javascript .= "\n<script language='JavaScript'>";
            $javascript .= "\n<!--";
            $javascript .= "\n    document.iform.dest.disabled=1;";
            $javascript .= "\n//-->";
            $javascript .= "\n</script>";

        } // if

        if ($xml == "squidguard_time.xml") {
            $javascript .= "\n<script language='JavaScript'>";
            $javascript .= "\n<!--";
            $javascript .= "\n    function on_updatecontrols() {";
            $javascript .= "\n        for (var i=0; i<99; i++) {";
            $javascript .= "\n            var elm = document.iform.elements['timetype' + i];";
            $javascript .= "\n            if (elm) {";
            $javascript .= "\n                document.iform.elements['timetype' + i].onclick = on_updatecontrols;";
            $javascript .= "\n                if (document.iform.elements['timetype' + i].value == 'weekly') {";
            $javascript .= "\n                    document.iform.elements['timedays' + i].disabled = 0;";
            $javascript .= "\n                    document.iform.elements['daterange' + i].disabled = 1;";
            $javascript .= "\n                }";
            $javascript .= "\n                else {";
            $javascript .= "\n                    document.iform.elements['timedays' + i].disabled = 1;";
            $javascript .= "\n                    document.iform.elements['daterange' + i].disabled = 0;";
            $javascript .= "\n                }";
            $javascript .= "\n            }";
            $javascript .= "\n        }";
            $javascript .= "\n    }";
            $javascript .= "\n    on_updatecontrols();";
            $javascript .= "\n    ";
            $javascript .= "\n//-->";
            $javascript .= "\n</script>";
        }

         print($javascript);
}

?>
