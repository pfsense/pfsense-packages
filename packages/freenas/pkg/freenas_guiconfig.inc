<?php
/*
	guiconfig.inc
	part of m0n0wall (http://m0n0.ch/wall)
	
	Copyright (C) 2003-2005 Manuel Kasper <mk@neon1.net>.
	All rights reserved.

	Modified for FreeNAS (http://freenas.org) by Olivier Cochard <cochard@gmail.com>

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

$d_mountdirty_path = $g['varrun_path'] . "/mount.dirty";
$d_diskdirty_path = $g['varrun_path'] . "/disk.dirty";
$d_raidconfdirty_path = $g['varrun_path'] . "/raid.conf.dirty";
$d_userconfdirty_path = $g['varrun_path'] . "/user.conf.dirty";
$d_groupconfdirty_path = $g['varrun_path'] . "/group.conf.dirty";
$d_smbshareconfdirty_path = $g['varrun_path'] . "/smbshare.conf.dirty";
$d_upnpconfdirty_path = $g['varrun_path'] . "/upnp.conf.dirty";

/* ============================================================================= */
/* ==    Constants used together with executing a UNIX command line tool      == */
/* ============================================================================= */

define("DONE_PARAGRAPH", "
              <p>
                <span class='red' style='font-family: Courier, monospace; font-size: small;'><strong>Done!</strong></span>
              </p>
       ");

define("CMDOUT_PARA", "
              <p style='font-size: small;'>
                <strong>Command output:</strong> (use the toggle icon to unveil detailed infos):
              </p>
       ");

define("CMDOUT_TOGGLE_FUNC", "
function toggle_cmdout(image, totoggle) {
  var plusSrc = \"/themes/{$g['theme']}/images/misc/bullet_toggle_plus.png\";
  var minusSrc = \"/themes/{$g['theme']}/images/misc/bullet_toggle_minus.png\";
  var currentSrc = image.src;
  var newSrc = (currentSrc.indexOf(\"plus\") >= 0) ? minusSrc : plusSrc;

  image.src = newSrc;
  Effect.toggle(totoggle, 'appear', { duration: 0.75 });
}
");

define("CMDOUT_AJAX_SCRIPT", "
          <script type='text/javascript'>
            function execCMD() {
              var to_insert = \"<div style='visibility:hidden' id='loading' name='loading'><img src='/themes/nervecenter/images/misc/loader_tab.gif' \/><\/div>\";
              new Insertion.Before('doCMDSubmit', to_insert);
              
              $('doCMDSubmit').style.visibility = 'hidden';
              $('loading').style.visibility = 'visible';
              $('cmdOutputTD').innerHTML = '';
          
              new Ajax.Request(
                \"{$_SERVER['SCRIPT_NAME']}\", {
                  method     : 'post',
                  parameters : Form.serialize($('iform')),
                  onSuccess  : execCMDComplete,
                  onFailure  : execCMDFailure
                }
              );
            }
          
            function execCMDFailure(req) {
              if($('doCMDSubmit')) $('doCMDSubmit').style.visibility = 'visible';
              if($('loading')) $('loading').style.visibility = 'hidden';
              if($('inputerrors')) window.scrollTo(0, 0);
              if($('inputerrors')) new Effect.Shake($('inputerrors'));
              if($('inputerrors')) $('inputerrors').innerHTML = req.responseText;
            }

            function execCMDComplete(req) {
              $('cmdOutputTD').innerHTML = req.responseText;
              $('loading').style.visibility = 'hidden';
              $('doCMDSubmit').style.visibility = 'visible';
              $('cmdOutputTD').style.visibility = 'visible';
            }
          </script>
");

$freenas_config =& $config['installedpackages']['freenas']['config'][0];

/* ============================================================================= */
/* ==     Functions used together with executing a UNIX command line tool     == */
/* ============================================================================= */

function assemble_cmdout($button, $out, $done = false) {
  $dopara = DONE_PARAGRAPH;

  $retvalue =<<<EOD
{$button}
{$out}

EOD;

  if ($done) { $retvalue .= "{$dopara}\n"; }
  return $retvalue;
}

function create_cmdout_container($id = "", $cmd = "") {
  $diskinit_str = "";
  
  if (is_array($cmd)) {
    foreach ($cmd as $cmdline) {
      $a_out = exec_command_and_return_text_array($cmdline);
      $diskinit_str .= implode("\n", $a_out);
    }
  } else {
    $a_out = exec_command_and_return_text_array($cmd);
    $diskinit_str = implode("\n", $a_out);
  }
  
  $returnval =<<<EOD
              <div id="{$id}" style="display: none; font-family: Courier, monospace; font-size: small;">
              <pre style="font-family: Courier, monospace; font-size: small; font-style: italic;">{$diskinit_str}</pre>
              </div>

EOD;

  return $returnval;
}

function create_toggle_button($title, $totoggle) {
  global $g;
  
  $returnval =<<<EOD
                <table cellpadding="0" cellspacing="0" border="0" style="padding-bottom: 8px;">
                  <tr>
                    <td align="left" valign="middle" style="padding-right: 5px;">
                      <img src='/themes/{$g['theme']}/images/misc/bullet_toggle_plus.png' alt='' border='0' style='border: solid 1px silver; cursor: pointer;' onclick='toggle_cmdout(this, "{$totoggle}");' />
                    </td>
                    <td align="left" valign="middle" style='font-family: Courier, monospace; font-size: small;'>
                      {$title}:
                    </td>
                  </tr>
                </table>
EOD;

  return $returnval;
}

function print_error_box_np($msg) {
  global $g;

  echo "<table border=\"0\" cellspacing=\"0\" cellpadding=\"4\" width=\"100%\">\n";
  echo "<tr><td bgcolor=\"#A12A2A\" width=\"36\" align=\"center\" valign=\"top\"><img src=\"/themes/{$g['theme']}/images/icons/icon_error.gif\" width=\"28\" height=\"32\"></td>\n";
  echo "<td bgcolor=\"#FFD9D1\" style=\"padding-left: 8px; padding-top: 6px\">";
  echo "<span class=\"errmsg\">";
  echo $msg;
  echo "</span>";
  echo "</td></tr></table>";
}

function print_error_box($msg) {
  echo "<p>";
  print_error_box_np($msg);
  echo "</p>";
}

function mount_sort() {
	global $g, $freenas_config;

	function mountcmp($a, $b) {
		return strcmp($a['mdisk'], $b['mdisk']);
	}

	usort($freenas_config['mounts']['mount'], "mountcmp");
}

function disks_sort() {
	global $g, $freenas_config;

	function diskcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['disks']['disk'], "diskcmp");
}

function gvinum_sort() {
	global $g, $freenas_config;

	function gvinumcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['gvinum']['vdisk'], "gvinumcmp");
	
}

function gmirror_sort() {
	global $g, $freenas_config;

	function gmirrorcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['gmirror']['vdisk'], "gmirrorcmp");
	
}

function gconcat_sort() {
	global $g, $freenas_config;

	function gconcatcmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['gconcat']['vdisk'], "gconcatcmp");
	
}

function gstripe_sort() {
	global $g, $freenas_config;

	function gstripecmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['gstripe']['vdisk'], "gstripecmp");
	
}

function graid5_sort() {
	global $g, $freenas_config;

	function graid5cmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($freenas_config['graid5']['vdisk'], "graid5cmp");
	
}

/* TODO: This needs to be changed */
function users_sort() {
	global $g, $config;

	function userscmp($a, $b) {
		return strcmp($a['login'], $b['login']);
	}

	usort($config['system']['user'], "userscmp");
}

/* TODO: This needs to be changed */
function groups_sort() {
	global $g, $config;

	function groupscmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($config['system']['group'], "groupscmp");
}
?>