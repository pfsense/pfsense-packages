<?php
/* $Id$ */
/*
 edit_snortrule.php
 Copyright (C) 2004, 2005 Scott Ullrich
 Copyright (C) 2008, 2009 Robert Zelaya
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
require_once("/usr/local/pkg/snort/snort_gui.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g;

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
$if_real = convert_friendly_interface_to_real_interface_name2($pconfig['interface']);

$iface_uuid = $a_nat[$id]['uuid'];

// if(!is_dir("/usr/local/etc/snort/rules"))
//	exec('mkdir /usr/local/etc/snort/rules/');

/* Check if the rules dir is empy if so warn the user */
/* TODO give the user the option to delete the installed rules rules */
$isrulesfolderempty = exec("ls -A /usr/local/etc/snort/snort_{$iface_uuid}_{$if_real}/rules/*.rules");
if ($isrulesfolderempty == "") {

	include("/usr/local/pkg/snort/snort_head.inc");
	include("fbegin.inc");

	echo "<body link=\"#000000\" vlink=\"#000000\" alink=\"#000000\">";

	if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}

	echo "<script src=\"/row_toggle.js\" type=\"text/javascript\"></script>\n
<script src=\"/javascript/sorttable.js\" type=\"text/javascript\"></script>\n
<table width=\"99%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
   <tr>\n
   		<td>\n";

	echo '<div class="snorttabs" style="margin:1px 0px; width:775px;">' . "\n";
	echo '<!-- Tabbed bar code -->' . "\n";
	echo '<ul class="snorttabs">' . "\n";
	echo '<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>' . "\n";
	echo "<li><a href=\"/snort/snort_interfaces_edit.php?id={$id}\"><span>If Settings</span></a></li>\n";
	echo "<li><a href=\"/snort/snort_rulesets.php?id={$id}\"><span>Categories</span></a></li>\n";
	echo "<li class=\"snorttabs_active\"><a href=\"/snort/snort_rules.php?id={$id}\"><span>Rules</span></a></li>\n";
	echo "<li><a href=\"/snort/snort_define_servers.php?id={$id}\"><span>Servers</span></a></li>\n";
	echo "<li><a href=\"/snort/snort_preprocessors.php?id={$id}\"><span>Preprocessors</span></a></li>\n";
	echo "<li><a href=\"/snort/snort_barnyard.php?id={$id}\"><span>Barnyard2</span></a></li>\n";
	echo '</ul>' . "\n";
	echo '</div>' . "\n";

	echo  		"</td>\n
  </tr>\n
  <tr>\n
    <td>\n
		<div id=\"mainarea\">\n
			<table id=\"maintable\" class=\"tabcont\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
				<tr>\n
					<td>\n
# The rules directory is empty.\n
		    		</td>\n
		  		</tr>\n
			</table>\n
		</div>\n
	</td>\n
  </tr>\n
</table>\n
\n
</form>\n
\n
<p>\n\n";

	echo "Please click on the Update Rules tab to install your selected rule sets.";
	include("fend.inc");

	echo "</body>";
	echo "</html>";

	exit(0);

}

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
	$currentruleset = basename($rulefile);

	//delimiter for each new rule is a new line
	$delimiter = "\n";

	//split the contents of the string file into an array using the delimiter
	$splitcontents = explode($delimiter, $contents);

	return $splitcontents;

}

$ruledir = "/usr/local/etc/snort/snort_{$iface_uuid}_{$if_real}/rules/";
$dh  = opendir($ruledir);

if ($_GET['openruleset'] != '' && $_GET['ids'] != '')
{
	header("Location: /snort/snort_rules.php?id=$id&openruleset={$_GET['openruleset']}&saved=yes");
}

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
	$rulefile = $_GET['openruleset'];
}
else
{
	$rulefile = $ruledir.$files[0];

}

//Load the rule file
$splitcontents = load_rule_file($rulefile);

if ($_POST)
{

	conf_mount_rw();

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
		write_rule_file($splitcontents, $rulefile);

		//once file has been written, reload file
		$splitcontents = load_rule_file($rulefile);
	  
		$stopMsg = true;
	}
}
else if ($_GET['act'] == "toggle")
{

	conf_mount_rw();

	$toggleid = $_GET['ids'];

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
	write_rule_file($splitcontents, $rulefile);

	//once file has been written, reload file
	$splitcontents = load_rule_file($rulefile);

	$stopMsg = true;

	//write disable/enable sid to config.xml
	if ($disabled == false) {
		$string_sid = strstr($tempstring, 'sid:');
		$sid_pieces = explode(";", $string_sid);
		$sid_off_cut = $sid_pieces[0];
		// sid being turned off
		$sid_off  = str_replace("sid:", "", $sid_off_cut);
		// rule_sid_on registers
		$sid_on_pieces = $a_nat[$id]['rule_sid_on'];
		// if off sid is the same as on sid remove it
		$sid_on_old = str_replace("||enablesid $sid_off", "", "$sid_on_pieces");
		// write the replace sid back as empty
		$a_nat[$id]['rule_sid_on'] = $sid_on_old;
		// rule sid off registers
		$sid_off_pieces = $a_nat[$id]['rule_sid_off'];
		// if off sid is the same as off sid remove it
		$sid_off_old = str_replace("||disablesid $sid_off", "", "$sid_off_pieces");
		// write the replace sid back as empty
		$a_nat[$id]['rule_sid_off'] = $sid_off_old;
		// add sid off registers to new off sid
		$a_nat[$id]['rule_sid_off'] = "||disablesid $sid_off" . $a_nat[$id]['rule_sid_off'];
		write_config();
		conf_mount_rw();
			
	}
	else
	{
		$string_sid = strstr($tempstring, 'sid:');
		$sid_pieces = explode(";", $string_sid);
		$sid_on_cut = $sid_pieces[0];
		// sid being turned off
		$sid_on  = str_replace("sid:", "", $sid_on_cut);
		// rule_sid_off registers
		$sid_off_pieces = $a_nat[$id]['rule_sid_off'];
		// if off sid is the same as on sid remove it
		$sid_off_old = str_replace("||disablesid $sid_on", "", "$sid_off_pieces");
		// write the replace sid back as empty
		$a_nat[$id]['rule_sid_off'] = $sid_off_old;
		// rule sid on registers
		$sid_on_pieces = $a_nat[$id]['rule_sid_on'];
		// if on sid is the same as on sid remove it
		$sid_on_old = str_replace("||enablesid $sid_on", "", "$sid_on_pieces");
		// write the replace sid back as empty
		$a_nat[$id]['rule_sid_on'] = $sid_on_old;
		// add sid on registers to new on sid
		$a_nat[$id]['rule_sid_on'] = "||enablesid $sid_on" . $a_nat[$id]['rule_sid_on'];
		write_config();
		conf_mount_rw();
	}

}

if ($_GET['saved'] == 'yes')
{
	$message = "The Snort rule configuration has been changed.<br>You must restart this snort interface in order for the changes to take effect.";

	//		stop_service("snort");
	//		sleep(2);
	//		start_service("snort");
	//		$savemsg = "";
	//		$stopMsg = false;
}

$currentruleset = basename($rulefile);

$ifname = strtoupper($pconfig['interface']);

require("guiconfig.inc");
include("/usr/local/pkg/snort/snort_head.inc");

$pgtitle = "Snort: $id $iface_uuid $if_real Category: $currentruleset";

?>

<body
	link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>
<?if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}?>

<?php
echo "{$snort_general_css}\n";
?>

<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img
	src="./images/transparent.gif" border="0"></img></a></div>

<div class="body2">

<noscript>
<div class="alert" ALIGN=CENTER><img
	src="../themes/<?php echo $g['theme']; ?>/images/icons/icon_alert.gif" /><strong>Please
enable JavaScript to view this content
</CENTER></div>
</noscript>


<?php
echo "<form action=\"snort_rules.php?id={$id}\" method=\"post\" name=\"iform\" id=\"iform\">";
?> <?php if ($_GET['saved'] == 'yes') {print_info_box_np2($message);}?>
</form>
<script type="text/javascript" language="javascript" src="row_toggle.js">
    <script src="/javascript/sorttable.js" type="text/javascript">
</script> <script language="javascript" type="text/javascript">
<!--
function go()
{
    var agt=navigator.userAgent.toLowerCase();
    if (agt.indexOf("msie") != -1) {
        box = document.forms.selectbox;
    } else {
        box = document.forms[1].selectbox;
	}
    destination = box.options[box.selectedIndex].value;
    if (destination) 
		location.href = destination;
}
// -->
</script> <script type="text/javascript">
<!--
function popup(url) 
{
 params  = 'width='+screen.width;
 params += ', height='+screen.height;
 params += ', top=0, left=0'
 params += ', fullscreen=yes';

 newwin=window.open(url,'windowname4', params);
 if (window.focus) {newwin.focus()}
 return false;
}
// -->
</script>

<table width="99%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td><?php
		echo '<div class="snorttabs" style="margin:1px 0px; width:775px;">' . "\n";
		echo '<!-- Tabbed bar code -->' . "\n";
		echo '<ul class="snorttabs">' . "\n";
		echo '<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>' . "\n";
		echo "<li><a href=\"/snort/snort_interfaces_edit.php?id={$id}\"><span>If Settings</span></a></li>\n";
		echo "<li><a href=\"/snort/snort_rulesets.php?id={$id}\"><span>Categories</span></a></li>\n";
		echo "<li class=\"snorttabs_active\"><a href=\"/snort/snort_rules.php?id={$id}\"><span>Rules</span></a></li>\n";
		echo "<li><a href=\"/snort/snort_define_servers.php?id={$id}\"><span>Servers</span></a></li>\n";
		echo "<li><a href=\"/snort/snort_preprocessors.php?id={$id}\"><span>Preprocessors</span></a></li>\n";
		echo "<li><a href=\"/snort/snort_barnyard.php?id={$id}\"><span>Barnyard2</span></a></li>\n";
		echo '</ul>' . "\n";
		echo '</div>' . "\n";
		?></td>
	</tr>
	<tr>
		<td>
		<div id="mainarea2">
		<table id="maintable" class="tabcont" width="100%" border="0"
			cellpadding="0" cellspacing="0">
			<tr>
				<td>
				<table id="ruletable1" class="sortable" width="100%" border="0"
					cellpadding="0" cellspacing="0">
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
					$currentruleset = basename($rulefile);

					?>
						<form name="forms"><select name="selectbox" class="formfld"
							onChange="go()">
							<?php
							$i=0;
							foreach ($files as $value)
							{
								$selectedruleset = "";
								if ($files[$i] === $currentruleset)
								$selectedruleset = "selected";
								?>
							<option
								value="?id=<?=$id;?>&openruleset=<?=$ruledir;?><?=$files[$i];?>"
								<?=$selectedruleset;?>><?=$files[$i];?></option>
							"
							<?php
							$i++;

							}
							?>
						</select></form>
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

							if ($disabled_pos !== false){
								$ischecked = "";
							}else{
								$ischecked = "checked";
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

							if (strstr($tempstring, 'msg: "'))
							$message = get_middle($tempstring, 'msg: "', '";', 0);
							if (strstr($tempstring, 'msg:"'))
							$message = get_middle($tempstring, 'msg:"', '";', 0);

							echo "<tr>
                                        <td class=\"listt\">
                                        $textss\n";
                                        ?>
					<a
						href="?id=<?=$id;?>&openruleset=<?=$rulefile;?>&act=toggle&ids=<?=$counter;?>"><img
						src="../themes/<?= $g['theme']; ?>/images/icons/<?=$iconb;?>"
						width="10" height="10" border="0"
						title="click to toggle enabled/disabled status"></a>
					<!-- <input name="enable" type="checkbox" value="yes" <?= $ischecked; ?> onClick="enable_change(false)"> -->
					<!-- TODO: add checkbox and save so that that disabling is nicer -->
						<?php
						echo "$textse
                                        </td>
                                        <td class=\"listlr\">
                                        $textss
                                        $sid
                                        $textse
                                        </td>
                                        <td class=\"listlr\">
                                        $textss
                                        $protocol";
                                        ?>
                                        <?php
                                        $printcounter++;
                                        echo "$textse
                                        </td>
                                        <td class=\"listlr\">
                                        $textss
                                        $source
                                        $textse
                                        </td>
                                        <td class=\"listlr\">
                                        $textss
                                        $source_port
                                        $textse
                                        </td>
                                        <td class=\"listlr\">
                                        $textss
                                        $destination
                                        $textse
                                        </td>
                                        <td class=\"listlr\">
                                        $textss
                                        $destination_port
                                        $textse
                                        </td>";
                                        ?>
					<td class="listbg"><font color="white"> <?php
					echo "$textss
					$message
					$textse
                                        </td>";
					?>
					<td valign="middle" nowrap class="list">
					<table border="0" cellspacing="0" cellpadding="1">
						<tr>
							<td><a href="javascript: void(0)"
								onclick="popup('snort_rules_edit.php?id=<?=$id;?>&openruleset=<?=$rulefile;?>&ids=<?=$counter;?>')"><img
								src="../themes/<?= $g['theme']; ?>/images/icons/icon_e.gif"
								title="edit rule" width="17" height="17" border="0"></a></td>
							<!-- Codes by Quackit.com -->
						</tr>
					</table>
					</td>
					<?php
						}
					}
					echo "   There are $printcounter rules in this category. <br><br>";
					?>
				
				</table>
				</td>
			</tr>
			<table class="tabcont" width="100%" border="0" cellspacing="0"
				cellpadding="0">
				<tr>
					<td width="16"><img
						src="../themes/<?= $g['theme']; ?>/images/icons/icon_block.gif"
						width="11" height="11"></td>
					<td>Rule Enabled</td>
				</tr>
				<tr>
					<td><img
						src="../themes/<?= $g['theme']; ?>/images/icons/icon_block_d.gif"
						width="11" height="11"></td>
					<td nowrap>Rule Disabled</td>
				</tr>
				<table class="tabcont" width="100%" border="0" cellspacing="0"
					cellpadding="0">
					<tr>
						<!-- TODO: add save and cancel for checkbox options -->
						<!-- <td><pre><input name="Submit" type="submit" class="formbtn" value="Save">	<input type="button" class="formbtn" value="Cancel" onclick="history.back()"><pre></td> -->
					</tr>
				</table>
				<tr>
					<td colspan="10">
					<p><!--<strong><span class="red">Warning:<br>
                  </span></strong>Editing these r</p>-->
					
					</td>
				</tr>
			</table>
		</table>
		
		</td>
	</tr>

</table>

</div>

					<?php

					include("fend.inc");

					echo $snort_custom_rnd_box;

					?>

</div>
</body>
</html>
