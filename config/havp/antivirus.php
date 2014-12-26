<?php
/* $Id$ */
/*
    antivirus.php
    Copyright (C) 2010 Serg Dvoriancev
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

require_once("guiconfig.inc");
require_once("pkg-utils.inc");
require_once("service-utils.inc");

include("head.inc");
 
header("Content-type: text/html; charset=utf-8");

$pgtitle = "Antivirus: General page";

if (file_exists("/usr/local/pkg/havp.inc"))
   require_once("/usr/local/pkg/havp.inc");
else echo "No havp.inc found";

define('PATH_CLAMDB',   '/var/db/clamav');
define('PATH_HAVPLOG',  '/var/log/havp/access.log');
define('PATH_AVSTATUS', '/var/tmp/havp.status');

function get_avdb_info()
{
    $r    = '';
    $path = PATH_CLAMDB . "/{$filename}";
    $fl   = get_dir(PATH_CLAMDB . "/");

    array_shift($fl);
    array_shift($fl);

    foreach ($fl as $fname) {
        $path = PATH_CLAMDB . "/{$fname}";
        $ext  = end(explode(".", $fname));

        if ( $ext == "cvd" || $ext == "cld") {
            $stl = "style='padding-top: 0px; padding-bottom: 0px; padding-left: 4px; padding-right: 4px; border-left: 1px solid #999999;'";
            if (file_exists($path)) {
                $handle = '';
                if ($handle = fopen($path, "r")) {
                    $fsize = sprintf("%.2f M", filesize($path)/1024/1024);
            
                    $s = fread($handle, 1024);
                    $s = explode(':', $s);

                    # datetime
                    $dt = explode(" ", $s[1]);
                    $s[1] = strftime("%Y.%m.%d", strtotime("{$dt[0]} {$dt[1]} {$dt[2]}"));
                    if ($s[0] == 'ClamAV-VDB')
                        $r .= "<tr class='listr'><td $stl>{$fname}</td><td $stl>{$s[1]}</td><td $stl align='right'>$fsize</td><td $stl align='right'>{$s[2]}</td><td $stl align='right'>{$s[3]}</td><td $stl>{$s[7]}</td></tr>";
                }
                fclose($handle);
            }
        }
    }

    return $r;
}

function get_av_statistic()
{
    return function_exists("havp_get_av_statistic") ? havp_get_av_statistic() : "Function 'havp_get_av_statistic' not found.";
}

function get_av_viruslog()
{
    return function_exists("havp_get_av_viruslog") ? havp_get_av_viruslog() : "Function 'havp_get_av_viruslog' not found.";
}

function get_scanlist()
{
    return function_exists("havp_get_filescanlist") ? havp_get_filescanlist() : "Function 'havp_get_filescanlist()' not found.";
}

function get_scan_log()
{
    $s = function_exists("havp_get_scan_log") ? havp_get_scan_log() : "Function 'havp_get_scan_log()' not found.";
    $s = str_replace("\n", "<br>", $s);
    return $s;
}

function pfsense_version_A()
{
    return function_exists("pfsense_version_") ? pfsense_version_() : 1;
}

function havp_status()
{
    $s = "";
    if (HVDEF_HAVP_STATUS_FILE && file_exists(HVDEF_HAVP_STATUS_FILE))
        $s = file_get_contents(HVDEF_HAVP_STATUS_FILE);
    return $s;
}

function clamd_status()
{
    $s = "";
    if (HVDEF_CLAM_STATUS_FILE && file_exists(HVDEF_CLAM_STATUS_FILE))
        $s = file_get_contents(HVDEF_CLAM_STATUS_FILE);
    return $s;
}

function avupdate_status()
{
    $s = "Not found.";
    if (HVDEF_UPD_STATUS_FILE && file_exists(HVDEF_UPD_STATUS_FILE))
        $s = file_get_contents(HVDEF_UPD_STATUS_FILE);
    return str_replace( "\n", "<br>", $s );
}
# ------------------------------------------------------------------------------

/* start service */
if($_POST['start'] != '') {
	#start_service($_POST['start']);
       if (file_exists(HVDEF_HAVP_STARTUP_SCRIPT)) {
           mwexec_bg (HVDEF_HAVP_STARTUP_SCRIPT . " start");
	    sleep(3);
       }
} else
/* restart service */
if($_POST['restart'] != '') {
	#restart_service($_POST['restart']);
       if (file_exists(HVDEF_HAVP_STARTUP_SCRIPT)) {
           mwexec_bg (HVDEF_HAVP_STARTUP_SCRIPT . " restart");
	    sleep(3);
       }
} else
/* stop service */
if($_POST['stop'] != '') {
	#stop_service($_POST['stop']);
       if (file_exists(HVDEF_HAVP_STARTUP_SCRIPT)) {
           mwexec_bg (HVDEF_HAVP_STARTUP_SCRIPT . " stop");
	    sleep(3);
       }
}

/* Scan start */
if ($_POST['scanpath'] != '') {
	$scandir = $_POST['scanpath'];
	if(function_exists("start_antivirus_scanner")) {
		start_antivirus_scanner($scandir);
      }
	else echo "No 'start_antivirus_scanner' function found.";
}

/* Start AV Update */
if ($_POST['startupdate'] != '') {
    if( function_exists("havp_update_AV")) {
        havp_update_AV();
    }
#    else echo "No 'start_antivirus_scanner' function found.";
}

/* Clear havp access log */
if ($_POST['clearlog_x'] != '') {
    file_put_contents(HVDEF_HAVP_ACCESSLOG, '');
}

# ------------------------------------------------------------------------------
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php 
if (pfsense_version_A() == '1') { 
    echo "<p class=\"pgtitle\">$pgtitle</p>";
} 
?>

<form action="antivirus.php" method="post">
<?php if ($savemsg) print_info_box($savemsg); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<!-- Tabs -->
  <tr>
    <td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("General page"), true, "antivirus.php");
	$tab_array[] = array(gettext("HTTP proxy"), false, "pkg_edit.php?xml=havp.xml&amp;id=0");
	$tab_array[] = array(gettext("Settings"), false, "pkg_edit.php?xml=havp_avset.xml&amp;id=0");

	display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr><td><div id="mainarea"><table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="tabcont" valign="top">
      <table width="100%" border="0" cellpadding="0" cellspacing="0">
<!-- Service -->
        <tr>
          <td class="listhdrr">Service</td>
          <td class="listhdrr">Status </td>
          <td class="listhdrr">&nbsp; </td>
          <td class="listhdrr">Version</td>
<!--          <td class="listhdrr">Settings</td> -->
        </tr>
        <tr>
          <td class="listlr">HTTP Antivirus Proxy ( <?php echo(havp_status()); ?> )</td>
          <td class="listr" ><center>
                  <?php
                          $running = (is_service_running("havp", $ps) or is_process_running("havp"));
                          if ($running) 
                               echo "<img src=\"/themes/" . $g["theme"] . "/images/icons/icon_pass.gif\"  > Running";
                          else echo "<img src=\"/themes/" . $g["theme"] . "/images/icons/icon_block.gif\"> Stopped";                          
                  ?>
          </td>
          <td class="listr" nowrap>
                  <?php
                          if($running) {
		                   echo "<input title='Restart Service' name='restart' type='image' value='havp' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_restart.gif'>";
		                   echo "&nbsp";
		                   echo "<input title='Stop Service'    name='stop'    type='image' value='havp' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_stop.gif'>";
		            } else echo "<input title='Start Service'   name='start'   type='image' value='havp' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_start.gif'>";		            
		    ?>
          </td>
          <td class="listr">
                  <?php echo exec("pkg_info | grep \"[h]avp\""); ?>
          </td>
<!--
          <td class="listr">
                  <a href="/pkg_edit.php?xml=havp.xml&amp;id=0">
                      <?php echo "<input height=14 title='Show Proxy settings page' name='scan' type='image' value='scan' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_start.gif'>"; ?>
                      <font size="2">&nbsp;Proxy  Settings</size>                    
                  </a>
          </td>
-->
        </tr>
        <tr>
          <td class="listlr">Antivirus Server ( <?php echo(clamd_status()); ?> )</td>
          <td class="listr"><center>
                  <?php
                          $running = (is_service_running("clamd", $ps) or is_process_running("clamd"));
                          if ($running) 
                               echo "<img src=\"/themes/" . $g["theme"] . "/images/icons/icon_pass.gif\"  > Running";
                          else echo "<img src=\"/themes/" . $g["theme"] . "/images/icons/icon_block.gif\"> Stopped";                          
                  ?>
          </td>
          <td class="listr">&nbsp;</td>
          <td class="listr">
                  <?php echo exec("clamd -V"); ?>
          </td>
<!--
          <td class="listr">
                  <a href="/pkg_edit.php?xml=havp_avset.xml&id=0">
                      <?php echo "<input height=14 title='Show Antivirus settings page' name='scan' type='image' value='scan' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_start.gif'>"; ?>
                      <font size="2">&nbsp;Antivirus Settings</size>                    
                  </a>
          </td>
-->
        </tr>

        <tr><td>&nbsp;</td></tr>
<!-- Update -->
        <tr>
          <td class="listhdrr" colspan="3">Antivirus Update</td>
          <td class="listhdrr" colspan="1">Update status</td></tr>
        </tr>
        <tr>
          <td class="listlr" colspan="3" nowrap>
                  <?php echo "<input height=14 title='Start antivirus update' name='startupdate' type='image' value='startupdate' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_start.gif'>"; ?>
                  <font size="-1">&nbsp;Start Update</font>
          </td>
          <td class="listr" colspan="1">
                  <?php echo avupdate_status(); ?>
          </td>
        </tr>
        <tr>
          <td class="listlr"colspan="3">Antivirus Base Info</td>
          <td colspan="1">
            <table width="100%" border="0" cellspacing="0" cellpadding="1" ><tbody>
              <tr  align="center"><td class="listhdrr">Database</td><td class="listhdrr">Date</td><td class="listhdrr">Size</td><td class="listhdrr">Ver.</td><td class="listhdrr">Signatures</td><td class="listhdrr">Builder</td></tr>
              <?php echo get_avdb_info(); ?>
            </tbody></table>
          </td>
        </tr>
        <tr><td>&nbsp;</td></tr>
<!-- File Scanner -->
        <tr>
          <td class="listhdrr" colspan="3">File scanner</td>
          <td class="listhdrr" colspan="1">Scanner status</td>
        </tr>
        <tr>
          <td class="vtable" colspan="3">
              <table width="100%" border="0" cellpadding="6" cellspacing="0">
                <tr>
                  <td class="listlr">
                        &nbsp;Path: <br>
                        <input size="60%" id="scanpath" name="scanpath" value=""><br>
                        &nbsp;Enter file path or catalog for scanning.
                        <hr>
                        <?php
                          $scanlist = get_scanlist();
                          if (is_array($scanlist))
                              foreach($scanlist as $scan) {
                                  echo "<span onclick=\"document.getElementById('scanpath').value='{$scan['path']}';\" style=\"cursor: pointer;\">\n";
                                  echo "<img src='./themes/".$g['theme']."/images/icons/icon_pass.gif'>\n";
                                  echo "<u>{$scan['descr']}</u>\n";
                                  echo "</span>";
                                  echo "<br>";
                              }
                        ?>
                  </td>
                </tr>	              
                <tr>
                  <td class="vncellr" nowrap>
                        <?php echo "<input height=14 title='Scan selected file or catalog' name='scan' type='image' value='scan' border=0 src='./themes/".$g['theme']."/images/icons/icon_service_start.gif'>"; ?>
                        <font size="-1">&nbsp;Start Scanner</font>
                  </td>
                </tr>
              </table>
          </td>
          <td class="listr" colspan="1">
              <?php echo get_scan_log(); ?>
          </td>
        </tr>
        <tr><td>&nbsp;</td></tr>
<!-- Last Viruses -->
        <tr>
          <td colspan="4">
            <table width="100%" border="0" cellspacing="0" cellpadding="1" ><tbody>
              <tr class="vncellt"><td class="listhdrr" colspan="4">Last Viruses</td></tr>
              <?php 
                  $count = 30;
                  $stl   = "style='padding-right: 4px;'";
                  $s = get_av_viruslog();                       
                  krsort($s); # reverse sort
                  if (is_array($s) && !empty($s)) {
                      foreach($s as $val) {
                          if (!$count) break;
                          $ln = explode(' ', $val);
                          echo "<tr><td nowrap $stl>{$ln[0]} {$ln[1]}</td><td nowrap $stl>{$ln[2]}</td><td>{$ln[5]}</td><td nowrap>{$ln[9]}</td></tr>";
                          $count--;
                      }
                  }
                  else echo "<tr><td $stl>Not found</td></tr>";
              ?>              
              <tr class="listr"><td class="listr" colspan="4"><?php echo get_av_statistic(); ?><?php echo "<div style='float:right;'><input title='Clear antivirus log' name='clearlog' type='image' value='havp' border=0 src='./themes/".$g['theme']."/images/icons/icon_x.gif'>"; ?><font size="-1">&nbsp;Clear log</font></div></td></tr>	              
            </tbody></table>
          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>
</div></tr></td></table>
</form>

<?php include("fend.inc"); ?>

<script type="text/javascript"> 
  NiftyCheck(); 
  Rounded("div#mainarea","bl br","#FFF","#eeeeee","smooth");
</script>

</body>
</html>
