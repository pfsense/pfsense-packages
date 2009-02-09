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
$lineid = $_GET['id'];

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
$message = get_middle($tempstring, 'msg:"', '";', 0);

$content = get_middle($tempstring, 'content:"', '";', 0);
$classtype = get_middle($tempstring, 'classtype:', ';', 0);
$revision = get_middle($tempstring, 'rev:', ';',0);

$pgtitle = "Snort: Edit Rule";
require("guiconfig.inc");
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>
<?php
if(!$pgtitle_output)
	echo "<p class=\"pgtitle\"><?=$pgtitle?></p>";
?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr>
  	<td>
<?php
	$tab_array = array();
	$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=snort.xml&id=0");
	$tab_array[] = array(gettext("Update Rules"), false, "/snort_download_rules.php");
	$tab_array[] = array(gettext("Categories"), false, "/snort_rulesets.php");
	$tab_array[] = array(gettext("Rules"), true, "/snort_rules.php?openruleset=/usr/local/etc/snort/rules/attack-responses.rules");
	$tab_array[] = array(gettext("Blocked"), false, "/snort_blocked.php");
	$tab_array[] = array(gettext("Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
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
						<form action="snort_rules.php?openruleset=<?=$file;?>&id=<?=$lineid;?>" target="" method="post" name="editform" id="editform">
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
										<td class="listhdr" width="10%">Content: </td>
										<td class="listlr" width="30%"><?php echo $content; ?></td>
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
										<td><input class="formbtn" value="Save" type="submit" name="editsave" id="editsave">&nbsp&nbsp&nbsp<input type="button" class="formbtn" value="Cancel" onclick="history.back()"></td>
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