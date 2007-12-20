<?php
# ------------------------------------------------------------------------------
# squidguard.inc
# (C)2006, 2007 Serg Dvoriancev
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
# ------------------------------------------------------------------------------

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('filter.inc');
require_once('service-utils.inc');
require_once('squidguard_configurator.inc');
# ------------------------------------------------------------------------------
define('SQUIDGUARD_CONFBASE',  '/usr/local/etc/squid');
define('SQUIDGUARD_CONFFILE',  '/squidguard.conf');
define('SQUIDGUARD_CONFXML',   '/squidguard_conf.xml');
define('SQUIDGUARD_BINPATH',   '/usr/local/bin');
define('SQUIDGUARD_WORKDIR',   '/usr/local/etc/squidGuard');
define('SQUIDGUARD_LOGDIR',    '/var/squidGuard/log');
define('SQUIDGUARD_DBHOME',    '/var/db/squidGuard');

define('SQUIDGUARD_WEBGUI_LOG',         '/squidguard_gui.log');
define('SQUIDGUARD_WEBGUI_HISTORY_LOG', '/squidguard_gui_history.log');

# ------------------------------------------------------------------------------

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
define('FLD_TIMERANGE',       'sg_timerange');
define('FLD_IPLIST',          'iplist');
define('FLD_DESCRIPTION',     'description');
define('FLD_EXPRESSIONS',     'expressions');
define('FLD_DOMAINS',         'domains');
define('FLD_URLS',            'urls');
define('FLD_DISABLED',        'disabled');
define('FLD_ENABLELOG',       'enablelog');
define('FLD_SQUIDGUARDENABLE','squidguard_enable');
define('FLD_BLACKLIST',       'blacklist');

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

define('BLACKLIST_DEFAULT_URL',  'http://ftp.teledanmark.no/pub/www/proxy/squidGuard/contrib/blacklists.tar.gz'); // 360Kb
define('BLACKLIST_DEFAULT_URL1', 'http://squidguard.mesd.k12.or.us/blacklists.tgz'); // 3.5Mb
define('BLACKLIST_DEFAULT_URL2', 'http://blacklist.dansguardian.org/cgi-bin/download.pl?type=download&file=bigblacklist');
define('BLACKLIST_TMP_FILE',    '/var/tmp/blacklists.tar.gz');
define('BLACKLIST_BTN_STD',     'Upload Std');
define('BLACKLIST_BTN_URL',     'Upload Url');
define('BLACKLIST_BTN_FILE',    'Upload File');
define('BLACKLIST_LOGFILE',     'blacklist.log');

define('APPLY_BTN', 'Apply');

define('WEBGUI_HISTORY_LOG', 'on');

define('TEST_LOG', '/var/tmp/sqtest.test');

# ==============================================================================
# Initialization
# ==============================================================================
sg_init(convert_pfxml_to_sgxml());

# ==============================================================================
# Validations
# ==============================================================================

# ------------------------------------------------------------------------------
# validate default
# ------------------------------------------------------------------------------
function squidguard_validate_default($post, $input_errors) {
        squidguard_validate_acl($post, &$input_errors);
}

# ------------------------------------------------------------------------------
# validate acl
# ------------------------------------------------------------------------------
function squidguard_validate_acl($post, $input_errors) {
        global $config;
        $pass_up = array();
        $deny_up = array();
        $pass_up_val = '';
        $pass_over = array();
        $deny_over = array();
        $pass_over_val = '';

        // store destinations to 'dest' value
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

        // !ATTENTION! on pfSense XML config must be must(shall) be '!all' instead of 'none' - it is a must for correct work GUI

        // if not exists key 'all', then add 'none' - default 'deny all'
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

# ------------------------------------------------------------------------------
# validate times
# Format:
#    date:   <date(or range)><time (or range)> -- days not parsed (reset to *)
#    weekly: <day or *><time or range>         -- dates not parsed (reset to '')
# ------------------------------------------------------------------------------
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

         // check format
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
}

# ------------------------------------------------------------------------------
# validate sources
# ------------------------------------------------------------------------------
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

# ------------------------------------------------------------------------------
# validate destinations
# ------------------------------------------------------------------------------
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
         // check redirect
         $redirect = trim($post[FLD_REDIRECT]);
         if(!empty($redirect)) {
            // cut first redirect url, if entered more that one
            $redirect = explode("\n", $redirect);
            $redirect = $redirect[0];
            $post[FLD_REDIRECT] = $redirect;
            if (is_url($redirect) === false)
                 $input_errors[] = "Redirect must contains valid url. Example: 'http://www.my.com', 'https://my.com', 'ftp://my.com'";
         }
}

# ------------------------------------------------------------------------------
# validate rewrites
# ------------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// squidguard_resync
// -----------------------------------------------------------------------------
function squidguard_resync() {
        $conf = '';
        $upload_file = '';

        // blacklist upload
        if ($_POST['submit'] == BLACKLIST_BTN_URL) {
            // upload from another url
            $url = $_POST['blacklist_url'];
            $proxy = $_POST['blacklist_proxy'];
            if ($url)
                sg_reconfigure_blacklist($url, $proxy);
        }
        // apply changes
        if ($_POST['submit'] == APPLY_BTN) sg_reconfigure();
}

// -----------------------------------------------------------------------------
// squidguard_resync_src
// -----------------------------------------------------------------------------
function squidguard_resync_src() {
       global $config;
       $conf = $config['installedpackages'][MODULE_SOURCE]['config'];
       $id = $_GET['id'];
       if (!$id)
           $id = $_POST['id'];

       // move current id by order
       if (($id != '') and is_array($conf) and ($id !== intval($conf[$id]['order']))) {
           // copy current item to temp and remove it's from list
           $src_new = array();
           $src_cur = $conf[$id];
           unset ($conf[$id]);

           // rebuild list and insert current item by order
           $i=0;
           foreach($conf as $src) {
                   if (intval($src_cur['order']) === $i) {
                       $src_new[] = $src_cur;
                       unset($src_cur);
                       $i++;
                   }
                   $src_new[] = $src;
                   $i++;
           }
           // if current item not inserted - insert him to the end
           if (isset($src_cur)) {
               $src_new[] = $src_cur;
               unset($src_cur);
           }

           // renew order values
           foreach($src_new as $key => $src) {
                   $src_new[$key]['order'] = $key;
           }

           unset ($config['installedpackages'][MODULE_SOURCE]['config']);
           $config['installedpackages'][MODULE_SOURCE]['config'] = $src_new;
           write_config('Update squidguardsrc config');
       } else if (($_GET['act'] === 'del') or ($_POST['act'] === 'del')) {
           // update order on delete item
           foreach($config['installedpackages'][MODULE_SOURCE]['config'] as $key => $src)
                   $config['installedpackages'][MODULE_SOURCE]['config'][$key]['order'] = $key;
           write_config('Update squidguardsrc config');
       }
}

// =============================================================================
// common functions
// =============================================================================

// -----------------------------------------------------------------------------
// get_pkg_items_list
// -----------------------------------------------------------------------------
function get_pkg_items_list($pkg_gui_name, $fieldname) {
       $res = '';
       global $config;
            $conf = $config['installedpackages'][$pkg_gui_name]['config'];
        foreach($conf as $cf)
                $res[] = $cf[$fieldname];
        return $res;
}

# ==============================================================================
# Before form
# ==============================================================================

// -----------------------------------------------------------------------------
// squidguard_before_form_src
// -----------------------------------------------------------------------------
function squidguard_before_form_src($pkg) {
         global $config;
         global $g;
         $i=0;
         foreach($pkg['fields']['field'] as $field) {
                 if ($field['fieldname'] == 'order') {
                         $fld = &$pkg['fields']['field'][$i];
                         $img_up   = "<img src='./themes/{$g['theme']}/images/icons/icon_up.gif'
                                      title='move up' width='17' height='17' border='0' onclick='on_moveup()'>";
                         $img_down = "<img src='./themes/{$g['theme']}/images/icons/icon_down.gif'
                                      title='move down' width='17' height='17' border='0' onclick='on_movedown()'>";
                         $s = "<b>Move to:</b> $img_up $img_down <br>" . $fld['description'];
                         $fld['description'] = $s;
                  }
                  $i++;
         }
}
// -----------------------------------------------------------------------------
// squidguard_before_form_dest
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

// -----------------------------------------------------------------------------
// squidguard_before_form
// -----------------------------------------------------------------------------
function squidguard_before_form($pkg) {
        $i=0;

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


// -----------------------------------------------------------------------------
// squidguard_before_form_acl
// -----------------------------------------------------------------------------
function squidguard_before_form_acl($pkg) {
        global $config;
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
                foreach($sources as $src)
                        $source_items[] = $src['name'];
        }

        // generate sources list TODO: exclude used names from list, source name used in ACL unique
        $i=0;
        foreach($pkg['fields']['field'] as $field) {
                if ($field['fieldname'] == 'source') {
                        $fld = &$pkg['fields']['field'][$i];
                        if (is_array($source_items)) {
                            foreach($source_items as $nm)
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

        // --- Destinations ---
        $general_cfg = $config['installedpackages'][MODULE_GENERAL]['config'][0];
        $dest_cfg =  $config['installedpackages'][MODULE_DESTINATION]['config'];

        // Blacklist
        if ($general_cfg['blacklist'] == 'on') {
            $blk_entries = sg_entries_blacklist();
            if (!empty($blk_entries)) {
                foreach($blk_entries as $dst)
                        $dest_items[] = array ('name'=>$dst, 'upt_value'=>$acls_up[$dst],
                                               'ovt_value'=>$acls_over[$dst], 'description'=>'');
            }
        }

        // User destinations
        if ($dest_cfg) {
            foreach($dest_cfg as $dst)
                    $dest_items[] = array ('name'=>$dst[FLD_NAME], 'upt_value'=>$acls_up[$dst[FLD_NAME]],
                                           'ovt_value'=>$acls_over[$dst[FLD_NAME]], 'description'=>$dst[FLD_DESCRIPTION]);
        }

        // Default all
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

// -----------------------------------------------------------------------------
// squidguard_before_form_log
// -----------------------------------------------------------------------------
function squidguard_before_form_log($pkg) {
        global $config;
        $i=0;
        $move_pos = 0;
        $move_step = 50;

        foreach($pkg['fields']['field'] as $field) {
                if ($field['fieldname'] == 'logtype') {
                        $slog = '';
                        $mlog = $config['installedpackages'][MODULE_LOG]['config'][0];

                        $filename = '';
                        $fld = &$pkg['fields']['field'][$i];
                        if (empty($move_pos)) $move_pos = 0;
                        if ($mlog['logtype'] == 'block_log') {
                            $filename = SQUIDGUARD_LOGDIR . "/" . SQUIDGUARD_ACCESSBLOCK_FILE;
                            if (file_exists($filename)) {
                                $s = file_get_contents($filename);
                                $s = explode("\n", $s);
                                $move_pos = count ($s) - $move_step;
                                if ($move_pos < 0) $move_pos = 0;
                                $s = array_slice($s, $move_pos, $move_step);

                                $slog .= "<tr><th>Num</th><th>DateTime</th><th>IP</th><th>URL</th><th>Filter</th><th>ID</th><th>Mtd</th></tr>";
                                foreach($s as $key => $vs) {
                                    $nkey = $move_pos + intval($key);
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
                                              <td style='vertical-align: top; white-space: nowrap;'>" . strval($nkey) . "</td>
                                              <td style='vertical-align: top; white-space: nowrap;'>$sx[0] $sx[1]</td>
                                              <td style='vertical-align: top;'>$sx[5]</td>
                                              <td style='vertical-align: top;'>$sx[4]</td>
                                              <td style='vertical-align: top;'>$sx[3]</td>
                                              <td style='vertical-align: top;'>$sx[2]</td>
                                              <td style='vertical-align: top;'>$sx[6]</td></tr>";
                                }
#                                $slog .= "</table>";
                            }
                        } else
                        if ($mlog['logtype'] == 'configurator_log') {
                            $filename = SQUIDGUARD_LOGDIR . SQUIDGUARDCONF_LOGFILE;
                            $slog .= "<b>$filename</b><br>";
                            if (file_exists($filename)) {
                                $slog .= file_get_contents($filename);
                                $slog = str_replace("\n", "<br>", $slog);
                                $slog = "<tr><td>$slog</td></tr>";
                            }
                        } else
                        if ($mlog['logtype'] == 'squidguard_log') {
                            $filename = SQUIDGUARD_LOGDIR . '/squidGuard.log';
                            $slog .= "<b>$filename</b><br>";
                            if (file_exists($filename)) {
                                $slog .= file_get_contents($filename);
                                $slog = explode("\n", $slog);
                                while (count($slog) > 500)  array_shift($slog);
                                $slog = implode("\n", $slog);
                                $slog = str_replace("\n", "<br>", $slog);
                                $slog = "<tr><td>$slog</td></tr>";
                            }
                        } else
                        if ($mlog['logtype'] == 'squid_config') {
                            $filename = SQUID_CONFIGFILE;
                            $slog .= "<b>$filename</b><br>";
                            if (file_exists($filename)) {
                                $slog .= file_get_contents($filename);
                                $slog = str_replace("\n", "<br>", $slog);
                                $slog = str_replace("\t", " ", $slog);
                                $slog = "<tr><td>$slog</td></tr>";
                            }
                        } else
                        if ($mlog['logtype'] == 'squidguard_config') {
                            $filename = SQUIDGUARD_WORKDIR . "/squidGuard.conf";
                            $slog .= "<b>$filename</b><br>";
                            if (file_exists($filename)) {
                                $slog .= file_get_contents($filename);
                                $slog = str_replace("\n", "<br>", $slog);
                                $slog = str_replace("\t", " ", $slog);
                                $slog = "<tr><td>$slog</td></tr>";
                            }
                        }

                        $fld['description'] .=
                            "<INPUT class=formbtn type=submit value='Get log' name=Submit>
                            </tr><tr>
                             <td colspan='2'><table width='100%' class=tabcont cellSpacing=0 cellPadding=1 border=1>$slog</table>";
                            "<table class=tabcont cellSpacing=0 cellPadding=0 border=1 width='100%'>$slog</table></td></tr>";

                }
                $i++;
        }
}

// -----------------------------------------------------------------------------
// squidguard_update_acl_dest
// -----------------------------------------------------------------------------
function squidguard_update_acl_dest($acl_name, $post) {
        global $config;
        $post['dest'] = 'fig vam';
        foreach($post as $k => $p)
                $s .= $k . '=' . $p;
}

// -----------------------------------------------------------------------------
// make_grid_general_items
// -----------------------------------------------------------------------------
function make_grid_general_items($id = '')
{
        global $config;

        $res = '';
        $res .= "<table width='100%'>";

        if ($id == '') {
            // Apply
            $res .= "<tr bgcolor='#dddddd'><td><big>For saving configuration YOU need click button 'Save' on bottom of page</big></td></tr>
                     <tr><td><big>After changing configuration Squid or squidGuard you must <b><span style='color: #800000;'>apply all changes</span></b></big></td></tr>
                     <tr><td><input name='submit' type='submit' value='Apply'></td></tr>";

            // service state
            $sgstate = "<span style='color: #800000;'>STOPPED</span>";
            if (is_service_running("squidGuard")) $sgstate = "<span style='color: #008000;'>STARTED</span>";
            $res .= "<tr bgcolor='#dddddd'><td><big>SquidGuard service state: <b>$sgstate</b></big></td></tr>";
        } else
        if ($id == 'gui_log') {
            if ($config['installedpackages']['squidguardgeneral']['config'][0]['view_gui_log'] == 'on') {
                $log_content = sg_getlog(50);
                $log_content = str_replace("\n","<br>", $log_content);
                $res .= "<tr bgcolor='#dddddd'><td><font size='-1'><b>Web GUI log (Last 50)</b></font></td></tr>";
                $res .= "<tr bgcolor='#dddddd'><td vAlign=top width='100%'><font size='-2'>$log_content</font></td></tr>";
            }
        }

        $res .= "</table>";
        return $res;
}

// -----------------------------------------------------------------------------
// make_grid_blacklist
// -----------------------------------------------------------------------------
function make_grid_blacklist() {

        $style1 = ''; // 'style="background-color: ##CCCCC2;"';
        $style2 = 'class=vtable';
        $style2 = '';
        $e_size = '90';

        $res = '';
        $res .= "<table width='100%'>";
        $res .= "<tr $style1><td><b>Proxy</b> settings</b></td>";
        $res .= "<td $style1><input name='blacklist_proxy' id='blacklist_proxy' type='text' size='$e_size'></td></tr>";
        $res .= "<tr><td $style1>&nbsp;</td>
                 <td>Blacklist upload proxy - enter here, or leave blank.<br>
                 Format: host:[port login:pass] . Default proxy port 1080. <br>
                 Example: '192.168.0.1:8080 user:pass'
                 </td></tr>";
        $res .= "<tr><td $style3>Load from <b>Url </b></td> <td $style3><input name='blacklist_url' id='blacklist_url'
                type='text' size='$e_size' value='http://squidguard.mesd.k12.or.us/blacklists.tgz'></td></tr>";
        $res .= "<tr><td $style1>&nbsp;</td>
                 <td>FTP, HTTP or LOCAL (pfSense) path to blacklist archive enter here, or leave blank.<br></td></tr>";
        // button
        $res .= "<tr><td $style3>&nbsp;</td>
                 <td $style3><input name='submit' value='" . BLACKLIST_BTN_URL  . "' type='submit'></td></tr>";

#        $res .= "<tr><td $style3>Load from <b>Default</b></td> <td $style3><b>" . /*BLACKLIST_DEFAULT_URL .*/ "</b></td> <td $style3><input name='submit' value='" .
#                BLACKLIST_BTN_STD . "' type='submit'></td></tr>";

#        $res .= "<tr><td $style1>Load from <b>File</b></td> <td $style1><input name='blacklist_file' id='blacklist_file'
#                type='file' size='$e_size'></td> <td $style1><input name='submit' value='" . BLACKLIST_BTN_FILE .
#                "' type='submit'></td></tr>";

        $res .= "</table>";
        return $res;
}

// -----------------------------------------------------------------------------
// make_grid_controls
// -----------------------------------------------------------------------------
function make_grid_controls($type, $items) {
        $res = '';
        $tbl = '';
        $color = '';
        $color2 = '';
        $x = 0;

        foreach($items as $item) {
                if ($x == 0) {
                        $color = 'style="background-color: #dddddd;"';
                        $color2 = '';
                        $x = 1;
                } else {
                        $color = '';
                        $color2 = 'style="background-color: #dddddd;"';
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

                $res .= "<table cellspacing='0'>" . $tbl . "</table>";
        }
        return $res;
}

// -----------------------------------------------------------------------------
// check unique name
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// check date
// -----------------------------------------------------------------------------
function check_date($date) {
        $err = '';
        $val = trim($date);
        // date or date range format: 'yyyy-mm-dd', 'yyyy-m-d', 'yyyy.mm.dd'  'yyyy.mm.dd-yyyy.mm.dd'
        if (!eregi("^(([0-9]{4})|[*])\.(([0-9]{2})|[*])\.(([0-9]{2})|[*])$", $val) &&
            !eregi("^(([0-9]{4})|[*])\.(([0-9]{2})|[*])\.(([0-9]{2})|[*])-(([0-9]{4})|[*])\.(([0-9]{2})|[*])\.(([0-9]{2})|[*])$", $val))
            $err .= " Invalid date '$date'.
                         You mast use date or date range format: 'yyyy.mm.dd' and 'yyyy.mm.dd-yyyy.mm.dd'.
                      Any symbol in date can be set as * (any). Example: '*-10-01', '199*-*1-*1'.";
        return $err;
}

// -----------------------------------------------------------------------------
// check time
// -----------------------------------------------------------------------------
function check_time($time) {
        $err = '';
        $val = trim($time);

        if (empty($val)) return '';

        // time range format: 'HH:MM-HH:MM'
        if (!eregi("^([0-9]{2})\:([0-9]{2})-([0-9]{2})\:([0-9]{2})$", $val))
            $err = "Invalid time range '$time'. You must use 'HH:MM-HH:MM' time range format. ";
        else {
            $tms = $time;
            $tms = str_replace("-", "\n", $tms);
            $tmsv = $tms;
            $tms = str_replace(":", "", $tms);
            $tms = explode("\n", $tms);
            $tmsv = explode("\n", $tmsv);
            if ($tms[0] >= 2400)
                 $err .= "Invalid time range var1='$tmsv[0]' must be < '24:00'. ";
            if ($tms[1] > 2400)
                 $err .= "Invalid time range var2='$tmsv[1]'  must be <= '24:00'. ";
            if ($tms[0] >= $tms[1])
                 $err .= "Invalid time range var1='$tmsv[0]'  must be < var2='$tmsv[1]'. ";
        }

        return $err;
}

# ------------------------------------------------------------------------------
# Install & deinstall
# ------------------------------------------------------------------------------

function squidguard_install_command() {
        if (!is_service_running("squidGuard")) {
              sg_init(convert_pfxml_to_sgxml());
              sg_check_system();

              // generate squidGuard blacklist entries file (check with squidGuard PORT)
              $entries = array("ads", "aggressive", "audio-video", "drugs", "gambling", "hacking",
                               "mail", "porn", "proxy", "violence", "warez");
              file_put_contents(SQUIDGUARD_WORKDIR . SQUIDGUARD_BLK_ENTRIES, implode("\n", $entries));
              set_file_access(SQUIDGUARD_WORKDIR, OWNER_NAME, 0755);

              sg_reconfigure();
        }
}

function squidguard_deinstall_command() {
         // remove entries from squid config
         squid_reconfigure('remove redirector options');

         // remove package and his depends
         mwexec("pkg_delete squidGuard-1.2.0_1");
         mwexec("rm -rf " . SQUIDGUARD_WORKDIR);
         // i known't, really need delete blacklist base?
         mwexec("rm -rf " . SQUIDGUARD_DBHOME);
         mwexec("/bin/rm -f " . SQUIDGUARD_CONFBASE . "/squidGuard*");
}

# ------------------------------------------------------------------------------
# SquidGuard print JavaSrcript
# ------------------------------------------------------------------------------

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

        if ($xml == "squidguard_src.xml") {
            $javascript .= "\n<script language='JavaScript'>";
            $javascript .= "\n<!--";
            $javascript .= "\n    function on_updatecontrols() {";
            $javascript .= "\n        document.iform.elements['order'].disabled = 0;";
            $javascript .= "\n        document.iform.elements['order'].onfocus = on_orderfocus;";
            $javascript .= "\n    }";
            $javascript .= "\n    function on_orderfocus() {";
            $javascript .= "\n        document.iform.elements['order'].blur();";
            $javascript .= "\n    }";
            $javascript .= "\n    function on_moveup() {";
            $javascript .= "\n        var order = parseInt(document.iform.elements['order'].value)";
            $javascript .= "\n        if (order > 0)";
            $javascript .= "\n             order = order - 1;";
            $javascript .= "\n        else order = 0;";
            $javascript .= "\n        document.iform.elements['order'].value = order;";
            $javascript .= "\n    }";
            $javascript .= "\n    function on_movedown() {";
            $javascript .= "\n        var order = parseInt(document.iform.elements['order'].value)";
            $javascript .= "\n        if (order >= 0)";
            $javascript .= "\n             order = order + 1;";
            $javascript .= "\n        else order = 0;";
            $javascript .= "\n        document.iform.elements['order'].value = order;";
            $javascript .= "\n    }";
            $javascript .= "\n    on_updatecontrols();";
            $javascript .= "\n    ";
            $javascript .= "\n//-->";
            $javascript .= "\n</script>";
        }

         print($javascript);
}

# ==============================================================================
# SquidGuard 2
# ==============================================================================

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml() {
    global $config;
    $sgxml = array();
    $pfxml = $config['installedpackages'][MODULE_GENERAL]['config'][0];

    $sgxml[FLD_LOGDIR]           = SQUIDGUARD_LOGDIR;
    $sgxml[FLD_DBHOME]           = SQUIDGUARD_DBHOME;
    $sgxml[FLD_BINPATH]          = SQUIDGUARD_BINPATH;
    $sgxml[FLD_WORKDIR]          = SQUIDGUARD_WORKDIR;
    $sgxml[FLD_SGCONF_XML]       = SQUIDGUARD_WORKDIR . SQUIDGUARD_CONFXML;
    $sgxml[FLD_ENABLED]          = $pfxml[FLD_SQUIDGUARDENABLE];
    $sgxml[FLD_BLACKLISTENABLED] = $pfxml[FLD_BLACKLIST];
    $sgxml[FLD_SOURCES]          = convert_pfxml_to_sgxml_source($config);
    $sgxml[FLD_DESTINATIONS]     = convert_pfxml_to_sgxml_destination($config);
    $sgxml[FLD_REWRITES]         = convert_pfxml_to_sgxml_rewrite($config);
    $sgxml[FLD_TIMES]            = convert_pfxml_to_sgxml_time($config);
    $sgxml[FLD_ACLS]             = convert_pfxml_to_sgxml_acl($config);
    $sgxml[FLD_DEFAULT]          = convert_pfxml_to_sgxml_default($config);

    // transparent
    $squidxml = $config['installedpackages']['squid']['config'][0];
    if(isset($squidxml['transparent_proxy'])) {
             $lanip = $config['interfaces']['lan']['ipaddr'];
             $sgxml[FLD_SQUID_TRANSPARENT_MODE] = 'on';
             $sgxml[FLD_CURRENT_LAN_IP] = $lanip;
    } else {
             unset($sgxml[FLD_SQUID_TRANSPARENT_MODE]);
             unset($sgxml[FLD_CURRENT_LAN_IP]);
    }

    // store cfg cache
    $cfg_xml = dump_xml_config($sgxml, FLD_SQUIDGUARD);
    file_put_contents($sgxml[FLD_SGCONF_XML], $cfg_xml);

    return $sgxml;
}

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml_source
// sgxml_source: [name][ip][desc][log]
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml_source($pfconfig) {
        $sgxml = array();

        $pfxml = $pfconfig['installedpackages']['squidguardsrc']['config'];
        if (is_array($pfxml)) {
            foreach($pfxml as $pfx) {
                $sgx = array();
                $sgx[FLD_NAME]                 = $pfx['name'];
                $sgx[FLD_IP]                   = $pfx['iplist'];
                $sgx[FLD_DOMAINS]              = $pfx[FLD_DOMAINS];
                $sgx[FLD_LOG]                  = $pfx['enablelog'];
                $sgx[FLD_DESCRIPTION]          = $pfx['description'];
                $sgxml[FLD_ITEM][] = $sgx;
            }
        }
        return $sgxml;
}

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml_destination
// sgxml_destination: [name][domains][expr][urls][redir][desc][log]
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml_destination($pfconfig) {
        $sgxml = array();

        $pfxml = $pfconfig['installedpackages']['squidguarddest']['config'];
        if (is_array($pfxml)) {
            foreach($pfxml as $pfx) {
                $sgx = array();
                $sgx[FLD_NAME]                = $pfx['name'];
                $sgx[FLD_URLS]                = $pfx['urls'];
                $sgx[FLD_DOMAINS]             = $pfx[FLD_DOMAINS];
                $sgx[FLD_EXPRESSIONS]         = $pfx['expressions'];
                $sgx[FLD_REDIRECT]            = $pfx[FLD_REDIRECT];
                $sgx[FLD_DESCRIPTION]         = $pfx['description'];
                $sgx[FLD_LOG]                 = $pfx['enablelog'];
                $sgxml[FLD_ITEM][] = $sgx;
            }
        }
        return $sgxml;
}

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml_rewrite
// sgxml_rewrite: [name][desc][log][items(array): [targeturl][replaceto]]
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml_rewrite($pfconfig) {
        $sgxml = array();

        $pfxml = $pfconfig['installedpackages']['squidguardrewrite']['config'];
        if (is_array($pfxml)) {
            foreach($pfxml as $pfx) {
                $sgx = array();
                $sgx[FLD_NAME]                 = $pfx['name'];
                $sgx[FLD_DESCRIPTION]         = $pfx['description'];
                $sgx[FLD_LOG]                  = $pfx['enablelog'];

                foreach($pfx['row'] as $pfx_row) {
                        $sgx_row = array();
                        $sgx_row[FLD_TARGETURL] = $pfx_row['targeturl'];
                        $sgx_row[FLD_REPLACETO] = $pfx_row['replaceto'];
                        $sgx[FLD_ITEM][] = $sgx_row;
                }

                $sgxml[FLD_ITEM][] = $sgx;
            }
        }
        return $sgxml;
}

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml_time
// sgxml_time: [name][desc][items(array): [timetype][timedays][daterange][timerange]]
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml_time($pfconfig) {
        $sgxml = array();

        $pfxml = $pfconfig['installedpackages']['squidguardtime']['config'];
        if (is_array($pfxml)) {
            foreach($pfxml as $pfx) {
                $sgx = array();
                $sgx[FLD_NAME]                = $pfx[FLD_NAME];
                $sgx[FLD_DESCRIPTION]        = $pfx[FLD_DESCRIPTION];

                foreach($pfx['row'] as $pfx_row) {
                        $sgx_row = array();
                        $sgx_row[FLD_TIMETYPE]  = $pfx_row[FLD_TIMETYPE];
                        $sgx_row[FLD_TIMEDAYS]  = $pfx_row[FLD_TIMEDAYS];
                        $sgx_row[FLD_DATERANGE] = $pfx_row[FLD_DATERANGE];
                        $sgx_row[FLD_TIMERANGE] = $pfx_row[FLD_TIMERANGE];
                        $sgx[FLD_ITEM][] = $sgx_row;
                }

                $sgxml[FLD_ITEM][] = $sgx;
            }
        }

        return $sgxml;
}

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml_acl
// sgxml_acl: [name][desc][disabled][timename][destname][redirect][rewritename][over_redirect][over_rewritename]
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml_acl($pfconfig) {
        $sgxml = array();

        $pfxml = $pfconfig['installedpackages']['squidguardacl']['config'];
        if (is_array($pfxml)) {
            foreach($pfxml as $pfx) {
                $sgx = array();
                $sgx[FLD_NAME]               = $pfx[FLD_SOURCE];
                $sgx[FLD_DESCRIPTION]        = $pfx[FLD_DESCRIPTION];
                $sgx[FLD_DISABLED]           = $pfx[FLD_DISABLED];
                $sgx[FLD_TIMENAME]           = $pfx[FLD_TIME];
                $sgx[FLD_REDIRECT]           = $pfx[FLD_REDIRECT];
                $sgx[FLD_REWRITENAME]        = $pfx[FLD_REWRITE];

                // for overtime
                $sgx[FLD_OVERREDIRECT]       = $pfx[FLD_OVERREDIRECT];
                $sgx[FLD_OVERREWRITENAME]    = $pfx[FLD_OVERREWRITE];

                // destinations
                if (strpos($pfx['dest'], '[') === false) {
                    $sgx[FLD_DESTINATIONNAME]     = trim($pfx['dest']);
                    $sgx[FLD_OVERDESTINATIONNAME] = '';
                } else {
                    $sgx[FLD_DESTINATIONNAME]     = trim( substr($pfx['dest'], 0, strpos($pfx['dest'], '[')) );
                    $sgx[FLD_OVERDESTINATIONNAME] = trim( strstr($pfx['dest'], '[') );
                    $sgx[FLD_OVERDESTINATIONNAME] = trim( str_replace(']', '', $sgx[FLD_OVERDESTINATIONNAME]) );
                    $sgx[FLD_OVERDESTINATIONNAME] = trim( str_replace('[', '', $sgx[FLD_OVERDESTINATIONNAME]) );
                }

                // !ATTENTION! '!all' must be convert to 'none'
                $sgx[FLD_DESTINATIONNAME]     = str_replace("!all", "none", $sgx[FLD_DESTINATIONNAME]);
                $sgx[FLD_OVERDESTINATIONNAME] = str_replace("!all", "none", $sgx[FLD_OVERDESTINATIONNAME]);

                $sgxml[FLD_ITEM][] = $sgx;
            }
        }
        return $sgxml;
}

// -----------------------------------------------------------------
// convert_pfxml_to_sgxml_default
// sgxml_acl: [name][desc][disabled][timename][destname][redirect][rewritename][over_redirect][over_rewritename]
// -----------------------------------------------------------------
function convert_pfxml_to_sgxml_default($pfconfig) {
        $pfxml = $pfconfig['installedpackages']['squidguarddefault']['config'];

        $pfx = $pfxml[0];
        $sgx = array();
        $sgx[FLD_NAME]                = 'default';
        $sgx[FLD_DESCRIPTION]         = '';
        $sgx[FLD_DISABLED]            = '';
        $sgx[FLD_TIMENAME]            = $pfx[FLD_TIME];
        $sgx[FLD_REDIRECT]            = $pfx[FLD_REDIRECT];
        $sgx[FLD_REWRITENAME]         = $pfx[FLD_REWRITE];
        // for overtime
        $sgx[FLD_OVERREDIRECT]        = $pfx[FLD_OVERREDIRECT];
        $sgx[FLD_OVERREWRITENAME]     = $pfx[FLD_OVERREWRITE];

        // destinations
        if (strpos($pfx['dest'], '[') === false) {
            $sgx[FLD_DESTINATIONNAME]     = trim($pfx['dest']);
            $sgx[FLD_OVERDESTINATIONNAME] = '';
        } else {
            $sgx[FLD_DESTINATIONNAME]     = trim( substr($pfx['dest'], 0, strpos($pfx['dest'], '[')) );
            $sgx[FLD_OVERDESTINATIONNAME] = trim( strstr($pfx['dest'], '[') );
            $sgx[FLD_OVERDESTINATIONNAME] = trim( str_replace(']', '', $sgx[FLD_OVERDESTINATIONNAME]) );
            $sgx[FLD_OVERDESTINATIONNAME] = trim( str_replace('[', '', $sgx[FLD_OVERDESTINATIONNAME]) );
        }

        // !ATTENTION! '!all' must be convert to 'none'
        $sgx[FLD_DESTINATIONNAME]     = str_replace("!all", "none", $sgx[FLD_DESTINATIONNAME]);
        $sgx[FLD_OVERDESTINATIONNAME] = str_replace("!all", "none", $sgx[FLD_OVERDESTINATIONNAME]);

        return $sgx;
}

?>