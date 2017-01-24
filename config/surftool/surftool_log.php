<?php
/* $Id$ */
/*
	surftool_log.php
	Copyright (C) 2015 H-T Reimers <reimers@mail.de>
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
$pgtitle = "Surftool: Log page";

require_once('guiconfig.inc');
require_once('notices.inc');
if (file_exists("/usr/local/pkg/surftool.inc")) {
   require_once("/usr/local/pkg/surftool.inc");
}

# ------------------------------------------------------------------------------
# defines
# ------------------------------------------------------------------------------
$selfpath = "/surftool/surftool_log.php";


# ------------------------------------------------------------------------------
# functions
# ------------------------------------------------------------------------------
function readBackward($file, $length = 0 ){
    $buffer = 8 * 1024;
    $nl = "\n";
    $lines = array();
    $last = '';
    $fh = fopen($file, 'r');
    
    if(!$fh) { die('Fehler beim öffnen von:' . $file . ' => ' . $php_errormsg);}
    
    fseek($fh, 0, SEEK_END);
    $pos = ftell($fh);

    while($pos > 1){
        $pos -= $buffer;
        if($pos < 0) {
            $buffer += $pos;
            $pos = 0;
        }
        fseek($fh, $pos, SEEK_SET);
        $tmp = explode($nl, fread($fh, $buffer) . $last);
        if($pos > 0) {$last = array_shift($tmp);}
        $lines = array_merge($tmp, $lines);
        if($length && count($lines) >= $length) { break; }
    }
    if($length && count($lines) >= $length) { 
        $lines = array_slice($lines, -$length);
    }
    return $lines;
}  

function print_file($filename){
			if(file_exists($filename)){
				$content=readBackward($filename,100);
				echo "<PRE>";
				foreach($content AS $line){
					echo $line."\n";
				}
				echo "</PRE>";
			}
			else{
				echo "Error: file $filename not exists<br>\n";
			}
}

# ------------------------------------------------------------------------------
# HTML Page
# ------------------------------------------------------------------------------

include("head.inc");
echo "\t<script type=\"text/javascript\" src=\"/javascript/scriptaculous/prototype.js\"></script>\n";
?>



<!-- HTML -->
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="sg_log.php" method="post">
<input type="hidden" id="reptype" val="">
<input type="hidden" id="offset"  val="0">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<!-- Tabs -->
  <tr>
    <td>
<?php
        $tab_array = array();
        $tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=surftool.xml&amp;id=0");
	$tab_array[] = array(gettext("Log"),              true,  "$selfpath");
        $tab_array[] = array(gettext("Switch groups"),       false, "/surftool/surftool_set.php");
	 $tab_array[] = array(gettext("external"),       false, "/surftool/index.php");
        display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
        <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
          <tr>
            <td>
		<h2>Read squidGuard configuration</h2>
		<?php
			require_once('surftool_class.inc');
			//Read configuration
			$surftool_config=parse_ini_file ("surftool.ini");
			$sgconf = new surftoolSquidGuardConf($surftool_config);
		?>		
		<h2>Inifile</h2>
		Last 100 lines<br>
		<?php
			$file=SURFTOOL_WWWCONFIGFILE;
			print_file($file);
		?>
		<h2>Logfile</h2>
		Last 100 lines<br>
		<?php
			$cfg = $config['installedpackages']['surftool']['config'][0];
			$file=$cfg[SURFTOOL_LOGFILE];
			print_file($file);

		?>
		<h2>Guilog</h2>
		Last 100 lines<br>
		<?php
			$file=SURFTOOL_LOG_FILE;
			print_file($file);
		?>		
            </td>
          </tr>
          <tr>
            <td id="reportarea" name="reportarea"></td>
          </tr>
        </table>
      </div>
    </td>
  </tr>
</table>
</form>

<?php include("fend.inc"); ?>


</body>
</html>
