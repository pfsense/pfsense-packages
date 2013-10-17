<?php
/*
 * snort_rules_flowbits.php
 * Copyright (C) 2004 Scott Ullrich
 * Copyright (C) 2011-2012 Ermal Luci
 * All rights reserved.
 *
 * originially part of m0n0wall (http://m0n0.ch/wall)
 * Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 * All rights reserved.
 *
 * modified for the pfsense snort package
 * Copyright (C) 2009-2010 Robert Zelaya.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

require_once("guiconfig.inc");
require_once("/usr/local/pkg/snort/snort.inc");

global $g, $rebuild_rules;

$snortdir = SNORTDIR;
$flowbit_rules_file = FLOWBITS_FILENAME;
$rules_map = array();
$supplist = array();

if (!is_array($config['installedpackages']['snortglobal']['rule'])) {
	$config['installedpackages']['snortglobal']['rule'] = array();
}
$a_nat = &$config['installedpackages']['snortglobal']['rule'];

$id = $_GET['id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (is_null($id)) {
	header("Location: /snort/snort_interfaces.php");
	exit;
}

// Set who called us so we can return to the correct page with
// the RETURN button.  We will just trust this User-Agent supplied
// string for now.
$referrer = $_SERVER['HTTP_REFERER'];

$if_real = snort_get_real_interface($a_nat[$id]['interface']);
$snort_uuid = $a_nat[$id]['uuid'];

/* We should normally never get to this page if Auto-Flowbits are disabled, but just in case... */
if ($a_nat[$id]['autoflowbitrules'] == 'on') {
	if (file_exists("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}") &&
	    filesize("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}") > 0) {
		$rules_map = snort_load_rules_map("{$snortdir}/snort_{$snort_uuid}_{$if_real}/rules/{$flowbit_rules_file}");
	}
	else
		$savemsg = gettext("There are no flowbit-required rules necessary for the current enforcing rule set.");
}
else
	$input_errors[] = gettext("Auto-Flowbit rule generation is disabled for this interface!");

if ($_GET['act'] == "addsuppress" && is_numeric($_GET['sidid']) && is_numeric($_GET['gen_id'])) {
	$descr = snort_get_msg($rules_map[$_GET['gen_id']][$_GET['sidid']]['rule']);
	if (empty($descr))
		$suppress = "suppress gen_id {$_GET['gen_id']}, sig_id {$_GET['sidid']}\n";
	else
		$suppress = "# {$descr}\nsuppress gen_id {$_GET['gen_id']}, sig_id {$_GET['sidid']}";
	if (!is_array($config['installedpackages']['snortglobal']['suppress']))
		$config['installedpackages']['snortglobal']['suppress'] = array();
	if (!is_array($config['installedpackages']['snortglobal']['suppress']['item']))
		$config['installedpackages']['snortglobal']['suppress']['item'] = array();
	$a_suppress = &$config['installedpackages']['snortglobal']['suppress']['item'];
	$found_list = false;

	if (empty($a_nat[$id]['suppresslistname']) || $a_nat[$id]['suppresslistname'] == 'default') {
		$s_list = array();
		$s_list['uuid'] = uniqid();
		$s_list['name'] = $a_nat[$id]['interface'] . "suppress" . "_" . $s_list['uuid'];
		$s_list['descr']  =  "Auto-generated list for Alert suppression";
		$s_list['suppresspassthru'] = base64_encode($suppress);
		$a_suppress[] = $s_list;
		$a_nat[$id]['suppresslistname'] = $s_list['name'];
		$found_list = true;
	} else {
		/* If we get here, a Suppress List is defined for the interface so see if we can find it */
		foreach ($a_suppress as $a_id => $alist) {
			if ($alist['name'] == $a_nat[$id]['suppresslistname']) {
				$found_list = true;
				if (!empty($alist['suppresspassthru'])) {
					$tmplist = base64_decode($alist['suppresspassthru']);
					$tmplist .= "\n{$suppress}";
					$alist['suppresspassthru'] = base64_encode($tmplist);
					$a_suppress[$a_id] = $alist;
				}
				else {
					$alist['suppresspassthru'] = base64_encode($suppress);
					$a_suppress[$a_id] = $alist;
				}
			}
		}
	}
	if ($found_list) {
		write_config();
		$rebuild_rules = false;
		sync_snort_package_config();
		snort_reload_config($a_nat[$id]);
		$savemsg = gettext("An entry to suppress the Alert for 'gen_id {$_GET['gen_id']}, sig_id {$_GET['sidid']}' has been added to Suppress List '{$a_nat[$id]['suppresslistname']}'.");
	}
	else {
		/* We did not find the defined list, so notify the user with an error */
		$input_errors[] = gettext("Suppress List '{$a_nat[$id]['suppresslistname']}' is defined for this interface, but it could not be found!");
	}
}

function truncate($string, $length) {

	/********************************
	 * This function truncates the  *
	 * passed string to the length  *
	 * specified adding ellipsis if *
	 * truncation was necessary.    *
	 ********************************/
	if (strlen($string) > $length)
		$string = substr($string, 0, ($length - 3)) . "...";
	return $string; 
}

/* Load up an array with the current Suppression List GID,SID values */
$supplist = snort_load_suppress_sigs($a_nat[$id]);

$if_friendly = snort_get_friendly_interface($a_nat[$id]['interface']);
$pgtitle = "Services: Snort: {$if_friendly} Flowbit Rules";
include_once("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" >

<?php
include("fbegin.inc");
if($pfsense_stable == 'yes'){echo '<p class="pgtitle">' . $pgtitle . '</p>';}
if ($input_errors) print_input_errors($input_errors);
if ($savemsg)
	print_info_box($savemsg);
?>
<form action="snort_rules_flowbits.php" method="post" name="iform" id="iform">
<div id="boxarea">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
<td class="tabcont">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td valign="middle" class="listtopic"><?php echo gettext("Auto-Generated Flowbit-Required Rules"); ?></td>
	</tr>
	<tr>
		<td width="78%" class="vncell">
		<?php echo gettext("The rules listed below are required to be included in the rules set ") . 
		gettext("because they set flowbits that are checked and relied upon by rules in the enforcing rules set.  ") . 
		gettext("If these dependent flowbits are not set, then some of your chosen rules may not fire.  ") . 
		gettext("Enabling all the rules that set these dependent flowbits ensures your chosen rules fire as intended.  ") . 
		gettext("Most flowbits rules contain the \"noalert\" keyword to prevent an alert from firing ") . 
		gettext("when the flowbit is detected.  For those flowbit rules that do not contain the \"noalert\" option, click the ") . 
		gettext("icon displayed beside the Signature ID (SID) to add the alert to the Suppression List if desired."); ?></td> 
	</tr>
	<tr>
		<td valign="middle" class="listtopic"><?php echo gettext("Flowbit-Required Rules for {$if_friendly}"); ?></td>
	</tr>
	<tr>
		<td width="78%" class="vncell">
			<table width="100%" border="0" cellspacing="2" cellpadding="0">
				<tr>
					<td width="17px"><img src="../themes/<?=$g['theme']?>/images/icons/icon_plus.gif" width='12' height='12' border='0'/></td>
					<td><span class="vexpl"><?php echo gettext("Alert is Not Suppressed"); ?></span></td>
					<td rowspan="3" align="right"><input id="cancelbutton" name="cancelbutton" type="button" class="formbtn" onclick="parent.location='<?=$referrer;?>'" <?php 
					echo "value=\"" . gettext("Return") . "\" title=\"" . gettext("Return to previous page") . "\""; ?>/></td>
				</tr>
				<tr>
					<td width="17px"><img src="../themes/<?=$g['theme']?>/images/icons/icon_plus_d.gif" width='12' height='12' border='0'/></td>
					<td><span class="vexpl"><?php echo gettext("Alert has been Suppressed"); ?></span></td>
				</tr>
				<tr>
					<td width="17px"> </td>
					<td colspan="2" class="vexpl"><?php echo "<span class=\"red\"><strong>" . 
					gettext("Note:  ") . "</strong></span>". gettext("the icon is only ") . 
					gettext("displayed for flowbit rules without the \"noalert\" option."); ?></td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td>
		<table id="myTable" width="100%" class="sortable" border="1" cellpadding="0" cellspacing="0">
			<colgroup>
				<col width="11%" axis="number">
				<col width="10%" axis="string">
				<col width="14%" axis="string">
				<col width="14%" axis="string">
				<col width="20%" axis="string">
				<col axis="string">
			</colgroup>
			<thead>
			   <tr>
				<th class="listhdrr" axis="number"><?php echo gettext("SID"); ?></th>
				<th class="listhdrr" axis="string"><?php echo gettext("Proto"); ?></th>
				<th class="listhdrr" axis="string"><?php echo gettext("Source"); ?></th>
				<th class="listhdrr" axis="string"><?php echo gettext("Destination"); ?></th>
				<th class="listhdrr" axis="string"><?php echo gettext("Flowbits"); ?></th>
				<th class="listhdrr" axis="string"><?php echo gettext("Message"); ?></th>
			   </tr>
			<thead>
			<tbody>
				<?php
					$count = 0;
					foreach ($rules_map as $k1 => $rulem) {
						foreach ($rulem as $k2 => $v) {
							$sid = snort_get_sid($v['rule']);
							$gid = snort_get_gid($v['rule']);

							// Pick off the first section of the rule (prior to the start of the MSG field),
							// and then use a REGX split to isolate the remaining fields into an array.
							$tmp = substr($v['rule'], 0, strpos($v['rule'], "("));
							$tmp = trim(preg_replace('/^\s*#+\s*/', '', $tmp));
							$rule_content = preg_split('/[\s]+/', $tmp);

							$protocol = $rule_content[1]; //protocol
							$source = truncate($rule_content[2], 14); //source
							$destination = truncate($rule_content[5], 14); //destination
							$message = snort_get_msg($v['rule']);
							$flowbits = implode("; ", snort_get_flowbits($v['rule']));
							if (strstr($flowbits, "noalert"))
								$supplink = "";
							else {
								if (!isset($supplist[$gid][$sid])) {
									$supplink = "<a href=\"?id={$id}&act=addsuppress&sidid={$sid}&gen_id={$gid}\">";
									$supplink .= "<img src=\"../themes/{$g['theme']}/images/icons/icon_plus.gif\" ";
									$supplink .= "width='12' height='12' border='0' title='";
									$supplink .= gettext("Click to add to Suppress List") . "'/></a>";
								}
								else {
									$supplink = "<img src=\"../themes/{$g['theme']}/images/icons/icon_plus_d.gif\" ";
									$supplink .= "width='12' height='12' border='0' title='";
									$supplink .= gettext("Alert has been suppressed") . "'/>";
								}
							}

							// Use "echo" to write the table HTML row-by-row.
							echo "<tr>" . 
								"<td class=\"listr\">{$sid}&nbsp;{$supplink}</td>" . 
								"<td class=\"listr\">{$protocol}</td>" . 
								"<td class=\"listr\"><span title=\"{$rule_content[2]}\">{$source}</span></td>" . 
								"<td class=\"listr\"><span title=\"{$rule_content[5]}\">{$destination}</span></td>" . 
								"<td class=\"listr\" style=\"word-wrap:break-word; word-break:normal;\">{$flowbits}</td>" . 
								"<td class=\"listr\" style=\"word-wrap:break-word; word-break:normal;\">{$message}</td>" . 
							"</tr>";
							$count++;
						}
					}
					unset($rulem, $v);
				?>
			</tbody>
		</table>
		</td>
	</tr>
	<?php if ($count > 20): ?>
	<tr>
		<td align="center" valign="middle">
			<input id="cancelbutton" name="cancelbutton" type="button" class="formbtn" onclick="parent.location='<?=$referrer;?>'" <?php 
			echo "value=\"" . gettext("Return") . "\" title=\"" . gettext("Return to previous page") . "\""; ?>/>
			<input name="id" type="hidden" value="<?=$id;?>" />
		</td>
	</tr>
	<?php endif; ?>
</table>
</td>
</tr>
</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
