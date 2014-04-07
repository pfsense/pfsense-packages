<?php 
/* $Id$ */
/*

	cron_edit.php
	Copyright (C) 2008 Mark J Crane
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
require_once("/usr/local/pkg/cron.inc");


$a_cron 	   = &$config['cron']['item'];

$id = $_GET['id'];
if (isset($_POST['id'])) {
	$id = $_POST['id'];
}

if ($_GET['act'] == "del") {
  if ($_GET['type'] == 'php') {
      if ($a_cron[$_GET['id']]) {
          unset($a_cron[$_GET['id']]);
          write_config();
          cron_sync_package();
          header("Location: cron.php");
          exit;
      }
  }
}

if (isset($id) && $a_cron[$id]) {

	$pconfig['minute'] = $a_cron[$id]['minute'];
	$pconfig['hour'] = $a_cron[$id]['hour'];
	$pconfig['mday'] = $a_cron[$id]['mday'];
	$pconfig['month'] = $a_cron[$id]['month'];
	$pconfig['wday'] = $a_cron[$id]['wday'];
	$pconfig['who'] = $a_cron[$id]['who'];
	$pconfig['command'] = $a_cron[$id]['command'];
	
}

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;
  
	if (!$input_errors) {

		$ent = array();
		$ent['minute'] = $_POST['minute'];
		$ent['hour'] = $_POST['hour'];
		$ent['mday'] = $_POST['mday'];
		$ent['month'] = $_POST['month'];
		$ent['wday'] = $_POST['wday'];
		$ent['who'] = $_POST['who'];
		$ent['command'] = $_POST['command'];

		if (isset($id) && $a_cron[$id]) {
		  	//update
      		$a_cron[$id] = $ent;
		}
		else {
		  	//add	  
			$a_cron[] = $ent;
		}
		
		write_config();
		cron_sync_package();
		
		header("Location: cron.php");
		exit;
	}
}

$pgtitle = array(gettext("Cron"),gettext("Edit"));
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<script type="text/javascript">
//<![CDATA[
function show_advanced_config() {
	document.getElementById("showadvancedbox").innerHTML='';
	aodiv = document.getElementById('showadvanced');
	aodiv.style.display = "block";
//]]>
</script>

<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>


<div id="mainlevel">
<table width="100%" border="0" cellpadding="0" cellspacing="0" summary="mainlevel">
<tr><td class="tabnavtbl">
<?php
			
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/packages/cron/cron.php");
	$tab_array[] = array(gettext("Edit"), true, "/packages/cron/cron_edit.php");
	display_top_tabs($tab_array);
 	
?>
</td></tr>
</table>
<table width="100%" border="0" cellpadding="0" cellspacing="0" summary="mainarea">
   <tr>
     <td class="tabcont" >

	<!--
  	<table width="100%" border="0" cellpadding="6" cellspacing="0" summary="title">              
      <tr>
        <td><p><span class="vexpl"><span class="red"><strong>Cron<br />
            </strong></span>
            </p></td>
      </tr>
    </table>
    -->
    <br />
    
        <form action="cron_edit.php" method="post" name="iform" id="iform">
            <table width="100%" border="0" cellpadding="6" cellspacing="0" summary="form">



		
            <tr>
              <td width="25%" valign="top" class="vncellreq">minute</td>
              <td width="75%" class="vtable"> 
                <input name="minute" type="text" class="formfld" id="minute" size="40" value="<?=htmlspecialchars($pconfig['minute']);?>" />
              </td>
            </tr> 

            <tr>
              <td width="25%" valign="top" class="vncellreq">hour</td>
              <td width="75%" class="vtable">
                <input name="hour" type="text" class="formfld" id="hour" size="40" value="<?=htmlspecialchars($pconfig['hour']);?>" />
              </td>
            </tr>

            <tr>
              <td width="25%" valign="top" class="vncellreq">mday</td>
              <td width="75%" class="vtable">
                <input name="mday" type="text" class="formfld" id="mday" size="40" value="<?=htmlspecialchars($pconfig['mday']);?>" />
              </td>
            </tr>

            <tr>
              <td width="25%" valign="top" class="vncellreq">month</td>
              <td width="75%" class="vtable">
                <input name="month" type="text" class="formfld" id="month" size="40" value="<?=htmlspecialchars($pconfig['month']);?>" />
              </td>
            </tr>

            <tr>
              <td width="25%" valign="top" class="vncellreq">wday</td>
              <td width="75%" class="vtable">
                <input name="wday" type="text" class="formfld" id="wday" size="40" value="<?=htmlspecialchars($pconfig['wday']);?>" />
              </td>
            </tr>
            
            <tr>
              <td width="25%" valign="top" class="vncellreq">who</td>
              <td width="75%" class="vtable">
                <input name="who" type="text" class="formfld" id="who" size="40" value="<?=htmlspecialchars($pconfig['who']);?>" />
              </td>
            </tr>
            
            <tr>
              <td width="25%" valign="top" class="vncellreq">command</td>
              <td width="75%" class="vtable">
                <!-- <input name="command" type="text" class="formfld" id="command" size="40" value="<?=htmlspecialchars($pconfig['command']);?>" /> -->
				  <textarea rows="3" cols="68" name="command" id="command"><?=htmlspecialchars($pconfig['command']);?></textarea>
              </td>
            </tr>
            <!--
            <tr>
              <td width="22%" valign="top" class="vncell">PHP</td>
              <td width="78%" class="vtable">
            -->
              <?php
                //echo "<textarea name=\"php\" id=\"php\" cols=\"70\" rows=\"15\" wrap=\"off\">".htmlspecialchars($pconfig['php'])."</textarea>\n";
              ?>
            <!--
                <br>
                <select name='dialplan_expression_select' id='dialplan_expression_select' onchange="document.getElementById('dialplan_expression').value += document.getElementById('dialplan_expression_select').value + '\n';" class='formfld'>
                <option></option>                
                <option value='snortkillsessions'>snort kill sessions</option>                               
                </select>
                <span class="vexpl">
                <br />
                </span>
                </td>
            </tr>
						-->
            <!--
            <tr>
              <td width="22%" valign="top" class="vncellreq">Enabled</td>
              <td width="78%" class="vtable"> 
                <?php                
                echo "              <select name='enabled' class='formfld'>\n";
                echo "                <option></option>\n";
                switch (htmlspecialchars($pconfig['enabled'])) {
                case "true":
                    echo "              <option value='true' selected='selected'>true</option>\n";
                    echo "              <option value='false'>false</option>\n";
                    break;
                case "false":
                    echo "              <option value='true'>true</option>\n";
                    echo "              <option value='false' selected='selected'>false</option>\n";

                    break;
                default:
                    echo "              <option value='true' selected='selected'>true</option>\n";
                    echo "              <option value='false'>false</option>\n";
                }
                echo "              </select>\n";
                ?>          
              </td>
            </tr>
            -->
            <!--
            <tr>
              <td width="25%" valign="top" class="vncellreq">Description</td>
              <td width="75%" class="vtable"> 
                <input name="description" type="text" class="formfld" id="description" size="40" value="<?=htmlspecialchars($pconfig['description']);?>" />
                <br /><span class="vexpl">Enter the description here.<br /></span>
              </td>
            </tr>
            -->
            
            <tr>
              <td valign="top">&nbsp;</td>
              <td>
                <input name="Submit" type="submit" class="formbtn" value="Save" /> <input class="formbtn" type="button" value="Cancel" onclick="history.back()" />
                <?php if (isset($id) && $a_cron[$id]): ?>
                  <input name="id" type="hidden" value="<?=$id;?>" />
                <?php endif; ?>
              </td>
            </tr>
            </table>
        </form>

      <br />
      <br />
      <br />
      <br />
      <br />
      <br />

     </td>
   </tr>
</table>

</div>

<?php include("fend.inc"); ?>
</body>
</html>
