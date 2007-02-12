<?php
/* $Id$ */
/*
    edit_snortrule.php
    Copyright (C) 2004, 2005 Scott Ullrich
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
require("guiconfig.inc");

if(!is_dir("/usr/local/etc/snort/rules"))
	Header("Location: snort_download_rules.php");

function get_middle($source, $beginning, $ending, $init_pos) {
   $beginning_pos = strpos($source, $beginning, $init_pos);
   $middle_pos = $beginning_pos + strlen($beginning);
   $ending_pos = strpos($source, $ending, $beginning_pos);
   $middle = substr($source, $middle_pos, $ending_pos - $middle_pos);
   return $middle;
}

function write_rule_file($content_changed, $received_file)
{
    //read snort file with writing enabled
    $filehandle = fopen($received_file, "w");

    //delimiter for each new rule is a new line
    $delimiter = "\n";

    //implode the array back into a string for writing purposes
    $fullfile = implode($delimiter, $content_changed);

    //write data to file
    fwrite($filehandle, $fullfile);

    //close file handle
    fclose($filehandle);

}

function load_rule_file($incoming_file)
{

    //read snort file
    $filehandle = fopen($incoming_file, "r");

    //read file into string, and get filesize
    $contents = fread($filehandle, filesize($incoming_file));

    //close handler
    fclose ($filehandle);

    //string for populating category select
    $currentruleset = substr($file, 27);

    //delimiter for each new rule is a new line
    $delimiter = "\n";

    //split the contents of the string file into an array using the delimiter
    $splitcontents = explode($delimiter, $contents);

    return $splitcontents;

}

$ruledir = "/usr/local/etc/snort/rules/";
$dh  = opendir($ruledir);

$message_reload = "The Snort rule configuration has been changed.<br>You must apply the changes in order for them to take effect.";

while (false !== ($filename = readdir($dh)))
{
    //only populate this array if its a rule file
    $isrulefile = strstr($filename, ".rules");
    if ($isrulefile !== false)
    {
        $files[] = $filename;
    }
}

sort($files);

if ($_GET['openruleset'])
{
    $file = $_GET['openruleset'];
}
else
{
    $file = $ruledir.$files[0];

}

//Load the rule file
$splitcontents = load_rule_file($file);

if ($_POST)
{
	if (!$_POST['apply']) {
	    //retrieve POST data
	    $post_lineid = $_POST['lineid'];
	    $post_enabled = $_POST['enabled'];
	    $post_src = $_POST['src'];
	    $post_srcport = $_POST['srcport'];
	    $post_dest = $_POST['dest'];
	    $post_destport = $_POST['destport'];
	
		//clean up any white spaces insert by accident
		$post_src = str_replace(" ", "", $post_src);
		$post_srcport = str_replace(" ", "", $post_srcport);
		$post_dest = str_replace(" ", "", $post_dest);
		$post_destport = str_replace(" ", "", $post_destport);
	
	    //copy rule contents from array into string
	    $tempstring = $splitcontents[$post_lineid];
	
	    //search string
	    $findme = "# alert"; //find string for disabled alerts
	
	    //find if alert is disabled
	    $disabled = strstr($tempstring, $findme);
	
	    //if find alert is false, then rule is disabled
	    if ($disabled !== false)
	    {
	        //has rule been enabled
	        if ($post_enabled == "yes")
	        {
	            //move counter up 1, so we do not retrieve the # in the rule_content array
	            $tempstring = str_replace("# alert", "alert", $tempstring);
	            $counter2 = 1;
	        }
	        else
	        {
	            //rule is staying disabled
	            $counter2 = 2;
	        }
	    }
	    else
	    {
	        //has rule been disabled
	        if ($post_enabled != "yes")
	        {
	            //move counter up 1, so we do not retrieve the # in the rule_content array
	            $tempstring = str_replace("alert", "# alert", $tempstring);
	            $counter2 = 2;
	        }
	        else
	        {
	            //rule is staying enabled
	            $counter2 = 1;
	        }
	    }
	
	    //explode rule contents into an array, (delimiter is space)
	    $rule_content = explode(' ', $tempstring);
	
		//insert new values
	    $counter2++;
	    $rule_content[$counter2] = $post_src;//source location
	    $counter2++;
	    $rule_content[$counter2] = $post_srcport;//source port location
	    $counter2 = $counter2+2;
	    $rule_content[$counter2] = $post_dest;//destination location
	    $counter2++;
	    $rule_content[$counter2] = $post_destport;//destination port location
	
		//implode the array back into string
		$tempstring = implode(' ', $rule_content);
	
		//copy string into file array for writing
	    $splitcontents[$post_lineid] = $tempstring;
	
	    //write the new .rules file
	    write_rule_file($splitcontents, $file);
	
	    //once file has been written, reload file
	    $splitcontents = load_rule_file($file);
	    
	    $stopMsg = true;
	}
	
	if ($_POST['apply']) {
		stop_service("snort");
		sleep(2);
		start_service("snort");
		$savemsg = "The snort rules selections have been saved. Restarting Snort.";
		$stopMsg = false;
	}

}
else if ($_GET['act'] == "toggle")
{
    $toggleid = $_GET['id'];

    //copy rule contents from array into string
    $tempstring = $splitcontents[$toggleid];

    //explode rule contents into an array, (delimiter is space)
    $rule_content = explode(' ', $tempstring);

    //search string
    $findme = "# alert"; //find string for disabled alerts

    //find if alert is disabled
    $disabled = strstr($tempstring, $findme);

    //if find alert is false, then rule is disabled
    if ($disabled !== false)
    {
        //rule has been enabled
        //move counter up 1, so we do not retrieve the # in the rule_content array
        $tempstring = str_replace("# alert", "alert", $tempstring);

    }
    else
    {
        //has rule been disabled
        //move counter up 1, so we do not retrieve the # in the rule_content array
        $tempstring = str_replace("alert", "# alert", $tempstring);

    }

    //copy string into array for writing
    $splitcontents[$toggleid] = $tempstring;

    //write the new .rules file
    write_rule_file($splitcontents, $file);

    //once file has been written, reload file
    $splitcontents = load_rule_file($file);
    
    $stopMsg = true;
}


$pgtitle = "Snort: Rules";
require("guiconfig.inc");
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<form action="snort_rules.php" method="post" name="iform" id="iform">
<?php if ($savemsg){print_info_box($savemsg);} else if ($stopMsg){print_info_box_np($message_reload);}?>
<br>
</form>
<script type="text/javascript" language="javascript" src="row_toggle.js">
    <script src="/javascript/sorttable.js" type="text/javascript">
</script>

<script language="javascript" type="text/javascript">
<!--
function go()
{
    box = document.forms[1].selectbox;
    destination = box.options[box.selectedIndex].value;
    if (destination) location.href = destination;
}

// -->
</script>

<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr>
      <td>
<?php
    $tab_array = array();
    $tab_array[] = array(gettext("Snort Settings"), false, "/pkg_edit.php?xml=snort.xml&id=0");
    $tab_array[] = array(gettext("Update Snort Rules"), false, "/snort_download_rules.php");
    $tab_array[] = array(gettext("Snort Categories"), false, "/snort_rulesets.php");
    $tab_array[] = array(gettext("Snort Rules"), true, "/snort_rules.php");
    $tab_array[] = array(gettext("Snort Blocked"), false, "/snort_blocked.php");
    $tab_array[] = array(gettext("Snort Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
    $tab_array[] = array(gettext("Snort Alerts"), false, "/snort_alerts.php");
    $tab_array[] = array(gettext("Snort Advanced"), false, "/pkg_edit.php?xml=snort_advanced.xml&id=0");
    display_top_tabs($tab_array);
?>
      </td>
  </tr>
  <tr>
    <td>
        <div id="mainarea">
            <table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                    <td>
                        <table id="ruletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
                            <tr id="frheader">
                                    <td width="3%" class="list">&nbsp;</td>
                                    <td width="5%" class="listhdr">SID</td>
                                    <td width="6%" class="listhdrr">Proto</td>
                                    <td width="15%" class="listhdrr">Source</td>
                                    <td width="10%" class="listhdrr">Port</td>
                                    <td width="15%" class="listhdrr">Destination</td>
                                    <td width="10%" class="listhdrr">Port</td>
                                  <td width="32%" class="listhdrr">Message</td>

                              </tr>
                            <tr>
                            <?php

                                echo "<br>Category: ";

                                //string for populating category select
                                $currentruleset = substr($file, 27);
                                ?>
                                <form name="forms">
                                    <select name="selectbox" class="formfld" onChange="go()">
                                        <?php
                                        $i=0;
                                        foreach ($files as $value)
                                        {
                                            $selectedruleset = "";
                                            if ($files[$i] === $currentruleset)
                                                $selectedruleset = "selected";
                                            ?>
                                            <option value="?&openruleset=<?=$ruledir;?><?=$files[$i];?>" <?=$selectedruleset;?>><?=$files[$i];?></option>"
                                            <?php
                                            $i++;

                                        }
                                        ?>
                                    </select>
                                </form>
                            </tr>
                            <?php

                            $counter = 0;
                            $printcounter = 0;

                            foreach ( $splitcontents as $value )
                            {

                                $counter++;
                                $disabled = "False";
                                $comments = "False";

                                $tempstring = $splitcontents[$counter];
                                $findme = "# alert"; //find string for disabled alerts

                                //find alert
                                $disabled_pos = strstr($tempstring, $findme);


                                    //do soemthing, this rule is enabled
                                    $counter2 = 1;

                                    //retrieve sid value
                                    $sid = get_middle($tempstring, 'sid:', ';', 0);

                                    //check to see if the sid is numberical
                                    $is_sid_num = is_numeric($sid);

                                    //if SID is numerical, proceed
                                    if ($is_sid_num)
                                    {

                                        //if find alert is false, then rule is disabled
                                        if ($disabled_pos !== false){
                                            $counter2 = $counter2+1;
                                            $textss = "<span class=\"gray\">";
                                            $textse = "</span>";
                                            $iconb = "icon_block_d.gif";
                                        }
                                        else
                                        {
                                            $textss = $textse = "";
                                            $iconb = "icon_block.gif";
                                        }

                                        $rule_content = explode(' ', $tempstring);

                                        $protocol = $rule_content[$counter2];//protocol location
                                        $counter2++;
                                        $source = $rule_content[$counter2];//source location
                                        $counter2++;
                                        $source_port = $rule_content[$counter2];//source port location
                                        $counter2 = $counter2+2;
                                        $destination = $rule_content[$counter2];//destination location
                                        $counter2++;
                                        $destination_port = $rule_content[$counter2];//destination port location

                                        $message = get_middle($tempstring, 'msg:"', '";', 0);

                                        echo "<tr>";
                                        echo "<td class=\"listt\">";
                                        echo $textss;
                                        ?>
                                        <a href="?&openruleset=<?=$file;?>&act=toggle&id=<?=$counter;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/<?=$iconb;?>" width="11" height="11" border="0" title="click to toggle enabled/disabled status"></a>
                                        <?php
                                        echo $textse;
                                        echo "</td>";


                                        echo "<td class=\"listlr\">";
                                        echo $textss;
                                        echo $sid;
                                        echo $textse;
                                        echo "</td>";

                                        echo "<td class=\"listlr\">";
                                        echo $textss;
                                        echo $protocol;
                                        $printcounter++;
                                        echo $textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo $textss;
                                        echo $source;
                                        echo $textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo $textss;
                                        echo $source_port;
                                        echo $textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo $textss;
                                        echo $destination;
                                        echo $textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo $textss;
                                        echo $destination_port;
                                        echo $textse;
                                        echo "</td>";
                                        ?>
                                        <td class="listbg"><font color="white">
                                        <?php
                                        echo $textss;
                                        echo $message;
                                        echo $textse;
                                        echo "</td>";
                                        ?>
                                          <td valign="middle" nowrap class="list">
                                            <table border="0" cellspacing="0" cellpadding="1">
                                                <tr>
                                                  <td><a href="snort_rules_edit.php?openruleset=<?=$file;?>&id=<?=$counter;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" title="edit rule" width="17" height="17" border="0"></a></td>
                                                </tr>
                                            </table>
                                        </td>
                                        <?php
                                    }
                            }
                            echo "   ";
                            echo "There are ";
                            echo $printcounter;
                            echo " rules in this category. <br><br>";
                            ?>
                         </table>
                    </td>
                </tr>
                <table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
                                <tr>
                                  <td width="16"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_block.gif" width="11" height="11"></td>
                                  <td>Rule Enabled</td>
                                </tr>
                                <tr>
                                  <td><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_block_d.gif" width="11" height="11"></td>
                                  <td nowrap>Rule Disabled</td>


                                </tr>
                        <tr>
                          <td colspan="10">
                  <p>
                  <!--<strong><span class="red">Warning:<br>
                  </span></strong>Editing these r</p>-->
                         </td>
                            </tr>
              </table>
            </table>

    </td>
  </tr>
</table>


<?php include("fend.inc"); ?>
</div></body>
</html>