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

require_once('surftool_class.inc');

//Read configuration
$surftool_config=parse_ini_file ("surftool.ini");
$sgconf = new surftoolSquidGuardConf($surftool_config);

//Prepare GUI functions
$surftoolGUI = new surftoolGUI($sgconf);

//Read changes from POST-data
$change_msg=$surftoolGUI->changes();

# ------------------------------------------------------------------------------
# defines
# ------------------------------------------------------------------------------
$selfpath = "/surftool/surftool_set.php";


# ------------------------------------------------------------------------------
# functions
# ------------------------------------------------------------------------------




# ------------------------------------------------------------------------------
# HTML Page
# ------------------------------------------------------------------------------

$closehead=false;
include("head.inc");
echo "\t<script type=\"text/javascript\" src=\"/javascript/scriptaculous/prototype.js\"></script>\n";
echo $surftoolGUI->get_java_script();
echo $surftoolGUI->get_style();
echo $surftoolGUI->meta_refresh();
echo "\n</head>\n";
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
	$tab_array[] = array(gettext("Log"),              false, "/surftool/surftool_log.php");
        $tab_array[] = array(gettext("Switch groups"),       true, "$selfpath");
	 $tab_array[] = array(gettext("external"),       false, "/surftool/index.php");
        display_top_tabs($tab_array);
?>
	</form>
    </td>
  </tr>
  
 <form action="<?php  echo $_SERVER['PHP_SELF'] ?>" method="post"> 
  <tr>
    <td>
      <div id="mainarea">
        <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
          <tr>
            <td>
		<?php
			
		
		

			echo $surftoolGUI->print_switch_mode();
			
			echo $change_msg;
			
			//Get groups/acls
			$groupsAll=$sgconf->squidGuardConf;
			
			echo "<h2>Switch group</h2>";
			$surftoolGUI->print_table($groupsAll);
		?>
		
            </td>
          </tr>
          <tr>
            <td id="reportarea" name="reportarea"></td>
          </tr>
        </table>
	</form>
      </div>
    </td>
  </tr>
</table>
</form>

<?php include("fend.inc"); ?>


</body>
</html>
