<?php
/* $Id$ */
/*
    snort_rules_edit.php
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
require("config.inc");

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}

//nat_rules_sort();
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
	

if (isset($id) && $a_nat[$id]) {

	$pconfig['enable'] = $a_nat[$id]['enable'];
	$pconfig['interface'] = $a_nat[$id]['interface'];
	$pconfig['rulesets'] = $a_nat[$id]['rulesets'];
}

/* convert fake interfaces to real */
$if_real = convert_friendly_interface_to_real_interface_name($pconfig['interface']);

//

function get_middle($source, $beginning, $ending, $init_pos) {
   $beginning_pos = strpos($source, $beginning, $init_pos);
   $middle_pos = $beginning_pos + strlen($beginning);
   $ending_pos = strpos($source, $ending, $beginning_pos);
   $middle = substr($source, $middle_pos, $ending_pos - $middle_pos);
   return $middle;
}


$file = $_GET['openruleset'];

//read snort file
$filehandle = fopen($file, "r");

//get rule id
$lineid = $_GET['ids'];

//read file into string, and get filesize
$contents = fread($filehandle, filesize($file));

//close handler
fclose ($filehandle);

//delimiter for each new rule is a new line
$delimiter = "\n";

//split the contents of the string file into an array using the delimiter
$splitcontents = explode($delimiter, $contents);

//copy rule contents from array into string
$tempstring = $splitcontents[$lineid];

//explode rule contents into an array, (delimiter is space)
$rule_content = explode(' ', $tempstring);

//search string
$findme = "# alert"; //find string for disabled alerts

//find if alert is disabled
$disabled = strstr($tempstring, $findme);

//get sid
$sid = get_middle($tempstring, 'sid:', ';', 0);

//get the full alert string rob
$rulealertsting = explode(" ", $tempstring);

//get type rob
if ($rulealertsting[0] == 'alert' || $rulealertsting[1] == 'alert')
	$type = 'alert';
if ($rulealertsting[0] == 'drop' || $rulealertsting[1] == 'drop')
	$type = 'drop';



//if find alert is false, then rule is disabled
if ($disabled !== false)
{
	//move counter up 1, so we do not retrieve the # in the rule_content array
	$counter2 = 2;
}
else
{
	$counter2 = 1;
}


$protocol = $rule_content[$counter2];//protocol location
$counter2++;
$source = $rule_content[$counter2];//source location
$counter2++;
$source_port = $rule_content[$counter2];//source port location
$counter2++;
$direction = $rule_content[$counter2];
$counter2++;
$destination = $rule_content[$counter2];//destination location
$counter2++;
$destination_port = $rule_content[$counter2];//destination port location


if (strstr($tempstring, 'msg: "'))
	$message = get_middle($tempstring, 'msg: "', '";', 0);
	if (strstr($tempstring, 'msg:"'))
		$message = get_middle($tempstring, 'msg:"', '";', 0);

if (strstr($tempstring, 'flow: '))
	$flow = get_middle($tempstring, 'flow: ', ';', 0);
	if (strstr($tempstring, 'flow:'))
		$flow = get_middle($tempstring, 'flow:', ';', 0);

if (strstr($tempstring, 'content: "'))
	$content = get_middle($tempstring, 'content: "', '";', 0);
	if (strstr($tempstring, 'content:"'))
		$content = get_middle($tempstring, 'content:"', '";', 0);

if (strstr($tempstring, 'metadata: '))
	$metadata = get_middle($tempstring, 'metadata: ', ';', 0);
	if (strstr($tempstring, 'metadata:'))
		$metadata = get_middle($tempstring, 'metadata:', ';', 0);

if (strstr($tempstring, 'reference: '))
	$reference = get_middle($tempstring, 'reference: ', ';', 0);
	if (strstr($tempstring, 'reference:'))
		$reference = get_middle($tempstring, 'reference:', ';', 0);
		
if (strstr($tempstring, 'reference: '))
	$reference2 = get_middle($tempstring, 'reference: ', ';', 1);
	if (strstr($tempstring, 'reference:'))
		$reference2 = get_middle($tempstring, 'reference:', ';', 1);

if (strstr($tempstring, 'classtype: '))
	$classtype = get_middle($tempstring, 'classtype: ', ';', 0);
	if (strstr($tempstring, 'classtype:'))
		$classtype = get_middle($tempstring, 'classtype:', ';', 0);

if (strstr($tempstring, 'rev: '))
	$revision = get_middle($tempstring, 'rev: ', ';', 0);
	if (strstr($tempstring, 'rev:'))
		$revision = get_middle($tempstring, 'rev:', ';', 0);


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
    $currentruleset = basename($file);

    //delimiter for each new rule is a new line
    $delimiter = "\n";

    //split the contents of the string file into an array using the delimiter
    $splitcontents = explode($delimiter, $contents);

    return $splitcontents;

}

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
	    
//		stop_service("snort");
//		sleep(2);
//		start_service("snort");
		
	}
}

//


$currentruleset = basename($file);

$pgtitle = "Snort: Interface: $id$if_real Rule File: $currentruleset Edit SID: $sid";
require("guiconfig.inc");
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php if ($savemsg){print_info_box($savemsg);}?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr>
  	<td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=snort.xml&id=0");
	$tab_array[] = array(gettext("Update Rules"), false, "/snort_download_rules.php");
	$tab_array[] = array(gettext("Categories"), false, "/snort_rulesets.php");
	$tab_array[] = array(gettext("Rules"), true, "/snort_rules.php?openruleset=/usr/local/etc/snort/rules/attack-responses.rules");
	$tab_array[] = array(gettext("Servers"), false, "/pkg_edit.php?xml=snort_define_servers.xml&amp;id=0");
	$tab_array[] = array(gettext("Blocked"), false, "/snort_blocked.php");
	$tab_array[] = array(gettext("Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
	$tab_array[] = array(gettext("Threshold"), false, "/pkg.php?xml=snort_threshold.xml");
	$tab_array[] = array(gettext("Alerts"), false, "/snort_alerts.php");
	$tab_array[] = array(gettext("Advanced"), false, "/pkg_edit.php?xml=snort_advanced.xml&id=0");
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
						<form action="snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$file;?>&ids=<?=$lineid;?>" target="" method="post" name="editform" id="editform">
							<table id="edittable" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
								<tr>
										<td class="listhdr" width="10%">Enabled: </td>
										<td class="listlr" width="30%"><input name="enabled" type="checkbox" id="enabled" value="yes" <?php if ($disabled === false) echo "checked";?>></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">SID: </td>
										<td class="listlr" width="30%"><?php echo $sid; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Type: </td>
										<td class="listlr" width="30%"><input name="type" type="text" id="type" size="20" value="<?php echo $type;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Protocol: </td>
										<td class="listlr" width="30%"><?php echo $protocol; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Source: </td>
										<td class="listlr" width="30%"><input name="src" type="text" id="src" size="20" value="<?php echo $source;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Source Port: </td>
										<td class="listlr" width="30%"><input name="srcport" type="text" id="srcport" size="20" value="<?php echo $source_port;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Direction:</td>
										<td class="listlr" width="30%"><?php echo $direction;?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Destination:</td>
										<td class="listlr" width="30%"><input name="dest" type="text" id="dest" size="20" value="<?php echo $destination;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Destination Port: </td>
										<td class="listlr" width="30%"><input name="destport" type="text" id="destport" size="20" value="<?php echo $destination_port;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Message: </td>
										<td class="listlr" width="30%"><?php echo $message; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Flow: </td>
										<td class="listlr" width="30%"><input name="flow" type="text" id="flow" size="20" value="<?php echo $flow;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Content: </td>
										<td class="listlr" width="30%"><?php echo $content; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Metadata: </td>
										<td class="listlr" width="30%"><input name="metadata" type="text" id="metadata" size="80" value="<?php echo $metadata;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Reference: </td>
										<td class="listlr" width="30%"><input name="reference" type="text" id="reference" size="80" value="<?php echo $reference;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Reference2: </td>
										<td class="listlr" width="30%"><input name="reference2" type="text" id="reference2" size="80" value="<?php echo $reference2;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Classtype: </td>
										<td class="listlr" width="30%"><?php echo $classtype; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Revision: </td>
										<td class="listlr" width="30%"><?php echo $revision; ?></td>
								</tr>
								<tr><td>&nbsp</td></tr>
								<tr>
										<td><input name="lineid" type="hidden" value="<?=$lineid;?>"></td>
										<td><input name="Submit" type="submit" class="formbtn" value="Save">	<input type="button" class="formbtn" value="Cancel" onclick="history.back()"></td>
								</tr>
							</table>
						</form>
					</td>
				</tr>
			</table>
	</td>
</tr>
</table>

<?php include("fend.inc"); ?>
</div></body>
</html>