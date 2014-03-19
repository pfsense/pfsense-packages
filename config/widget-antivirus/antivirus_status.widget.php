<?php
/*
        $Id: antivirus_statistics.widget.php 
        Copyright (C) 2010 Serg Dvoriancev <dv_serg@mail.ru>.
        Part of pfSense widgets (www.pfsense.org)
        originally based on m0n0wall (http://m0n0.ch/wall)

        Copyright (C) 2004-2005 T. Lechat <dev@lechat.org>, Manuel Kasper <mk@neon1.net>
        and Jonathan Watt <jwatt@jwatt.org>.
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
require_once("pfsense-utils.inc");
require_once("functions.inc");

define('PATH_CLAMDB',   '/var/db/clamav');
$pfSversion = str_replace("\s", "", file_get_contents("/etc/version"));
if(preg_match("/^2.0/",$pfSversion))
	define('PATH_HAVPLOG',  '/var/log/havp/access.log');
else
	define('PATH_HAVPLOG',  '/var/log/access.log');

define('PATH_AVSTATUS', '/var/tmp/havp.status');


if (file_exists("/usr/local/pkg/havp.inc"))
   require_once("/usr/local/pkg/havp.inc");
else echo "No havp.inc found";

function havp_avdb_info($filename)
{
    $stl = "style='padding-top: 0px; padding-bottom: 0px; padding-left: 4px; padding-right: 4px; border-left: 1px solid #999999;'";
    $r = '';
    $path = PATH_CLAMDB . "/{$filename}";
    if (file_exists($path)) {
        $handle = '';
        if ($handle = fopen($path, "r")) {
            $s = fread($handle, 1024);
            $s = explode(':', $s);

            # datetime
            $dt = explode(" ", $s[1]);
            $s[1] = strftime("%Y.%m.%d", strtotime("{$dt[0]} {$dt[1]} {$dt[2]}"));
            if ($s[0] == 'ClamAV-VDB')
                $r .= "<tr class='listr'><td>{$filename}</td><td $stl>{$s[1]}</td><td $stl>{$s[2]}</td><td $stl>{$s[7]}</td></tr>";
        }
        fclose($handle);
    }
    return $r;
}

function dwg_avbases_info()
{
    $db  = '<table width="100%" border="0" cellspacing="0" cellpadding="1" ><tbody>';
    $db .= '<tr class="vncellt" ><td>Database</td><td>Date</td><td>Ver.</td><td>Builder</td></tr>';
    $db .= havp_avdb_info("daily.cld");
    $db .= havp_avdb_info("daily.cvd");
    $db .= havp_avdb_info("bytecode.cld");
    $db .= havp_avdb_info("bytecode.cvd");
    $db .= havp_avdb_info("main.cld");
    $db .= havp_avdb_info("main.cvd");
    $db .= havp_avdb_info("safebrowsing.cld");
    $db .= havp_avdb_info("safebrowsing.cvd");
    $db .= '</tbody></table>';
    return $db;
}

function avupdate_status()
{
    $s = "Not found.";
    if (HVDEF_UPD_STATUS_FILE && file_exists(HVDEF_UPD_STATUS_FILE))
        $s = file_get_contents(HVDEF_UPD_STATUS_FILE);
    return str_replace( "\n", "<br>", $s );
}

function dwg_av_statistic()
{
    $s = "Unknown.";
    if (file_exists(PATH_HAVPLOG)) {
        $log   = file_get_contents(PATH_HAVPLOG);

$count = substr_count(strtolower($log), "virus clamd:");
$s     = "Found $count viruses (total).";

/*
# slowly worked - need apply cache or preparse stat

        $log   = explode("\n", $log);
        # counters: day, week, mon, total
        $count = 0;
        foreach($log as $ln) {
            $ln = explode(' ', $ln);
            # 0:date 1:time 2:ip 3:get 4:len 5:url 6:xx 7:status
            if (strpos(strtolower($ln[7]), "virus") !== false) {
                $count++;
            }
        }
        $s  = "Found viruses:<br>";
        $s .= "<table width='100%' border='0' cellspacing='0' cellpadding='0'><tbody>";
        $s .= "<tr align='center'><td>today</td><td>week</td><td>mon</td><td>total</td></tr>";
        $s .= "<tr align='center'><td>0</td><td>0</td><td>0</td><td>$count</td></tr>";
        $s .= "</tbody></table>";
*/
    }

    return $s;
}

?>

    <table width="100%" border="0" cellspacing="0" cellpadding="0">
     <tbody>
      <tr>
        <td class="vncellt">HTTP Scanner</td>
        <td class="listr" width=75%>
          <?php
              # havp version
              echo exec("pkg_info | grep \"[h]avp\"");
          ?>
        </td>
      </tr>	              
      <tr>
        <td class="vncellt">Antivirus Scanner</td>
        <td class="listr" width=75%>
          <?php 
              # Clamd version
              echo exec("clamd -V");
          ?>
        </td>
      </tr>	              
      <tr>
        <td class="vncellt">Antivirus Bases</td>
        <td class="listr" width=75%>
          <?php
              # Antivirus bases
              if (function_exists("dwg_avbases_info"))
                  echo dwg_avbases_info();
          ?>
        </td>
      </tr>	              
      <tr>
        <td class="vncellt">Last Update</td>
        <td class="listr" width=75%>
          <?php
                echo avupdate_status();
          ?>
        </td>
      </tr>	              
      <tr>
        <td class="vncellt">Statistic</td>
        <td class="listr" width=75%>
          <?php 
              echo dwg_av_statistic();
          ?>
        </td>
      </tr>	              
     </tbody>
    </table>

