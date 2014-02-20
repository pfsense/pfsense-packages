<?php
/*
 * suricata_os_policy_engine.php
 * Copyright (C) 2014 Bill Meeks
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
require_once("/usr/local/pkg/suricata/suricata.inc");

global $g;

// Grab the incoming QUERY STRING or POST variables
$id = $_GET['id'];
$eng_id = $_GET['eng_id'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (isset($_POST['eng_id']))
	$eng_id = $_POST['eng_id'];

if (is_null($id)) {
 	header("Location: /suricata/suricata_interfaces.php");
	exit;
}

if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();
if (!is_array($config['installedpackages']['suricata']['rule'][$id]['host_os_policy']['item']))
	$config['installedpackages']['suricata']['rule'][$id]['host_os_policy']['item'] = array();
$a_nat = &$config['installedpackages']['suricata']['rule'][$id]['host_os_policy']['item'];

$pconfig = array();
if (empty($a_nat[$eng_id])) {
	$def = array( "name" => "engine_{$eng_id}", "bind_to" => "", "policy" => "bsd" );
	// See if this is initial entry and set to "default" if true
	if ($eng_id < 1) {
		$def['name'] = "default";
		$def['bind_to'] = "all";
	}
	$pconfig = $def;
}
else {
	$pconfig = $a_nat[$eng_id];

	// Check for any empty values and set sensible defaults
	if (empty($pconfig['policy']))
		$pconfig['policy'] = "bsd";
}

if ($_POST['Cancel']) {
	header("Location: /suricata/suricata_flow_stream.php?id={$id}");
	exit;
}

// Check for returned "selected alias" if action is import
if ($_GET['act'] == "import") {
	if ($_GET['varname'] == "bind_to" && !empty($_GET['varvalue']))
		$pconfig[$_GET['varname']] = $_GET['varvalue'];
}

if ($_POST['Submit']) {

	/* Grab all the POST values and save in new temp array */
	$engine = array();
	if ($_POST['policy_name']) { $engine['name'] = trim($_POST['policy_name']); } else { $engine['name'] = "default"; }
	if ($_POST['policy_bind_to']) {
		if (is_alias($_POST['policy_bind_to']))
			$engine['bind_to'] = $_POST['policy_bind_to'];
		elseif (strtolower(trim($_POST['policy_bind_to'])) == "all")
			$engine['bind_to'] = "all";
		else
			$input_errors[] = gettext("You must provide a valid Alias or the reserved keyword 'all' for the 'Bind-To IP Address' value.");
	}
	else {
		$input_errors[] = gettext("The 'Bind-To IP Address' value cannot be blank.  Provide a valid Alias or the reserved keyword 'all'.");
	}

	if ($_POST['policy']) { $engine['policy'] = $_POST['policy']; } else { $engine['policy'] = "bsd"; }

	/* Can only have one "all" Bind_To address */
	if ($engine['bind_to'] == "all" && $engine['name'] <> "default") {
		$input_errors[] = gettext("Only one default OS-Policy Engine can be bound to all addresses.");
		$pconfig = $engine;
	}

	/* if no errors, write new entry to conf */
	if (!$input_errors) {
		if (isset($eng_id) && $a_nat[$eng_id]) {
			$a_nat[$eng_id] = $engine;
		}
		else
			$a_nat[] = $engine;

		/* Reorder the engine array to ensure the */
		/* 'bind_to=all' entry is at the bottom   */
		/* if it contains more than one entry.    */
		if (count($a_nat) > 1) {
			$i = -1;
			foreach ($a_nat as $f => $v) {
				if ($v['bind_to'] == "all") {
					$i = $f;
					break;
				}
			}
			/* Only relocate the entry if we  */
			/* found it, and it's not already */
			/* at the end.                    */
			if ($i > -1 && ($i < (count($a_nat) - 1))) {
				$tmp = $a_nat[$i];
				unset($a_nat[$i]);
				$a_nat[] = $tmp;
			}
		}

		/* Now write the new engine array to conf */
		write_config();

		header("Location: /suricata/suricata_flow_stream.php?id={$id}");
		exit;
	}
}

$if_friendly = suricata_get_friendly_interface($config['installedpackages']['suricata']['rule'][$id]['interface']);
$pgtitle = gettext("Suricata: Interface {$if_friendly} Operating System Policy Engine");
include_once("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" >

<?php
include("fbegin.inc");
if ($input_errors) print_input_errors($input_errors);
if ($savemsg)
	print_info_box($savemsg);
?>

<form action="suricata_os_policy_engine.php" method="post" name="iform" id="iform">
<input name="id" type="hidden" value="<?=$id?>">
<input name="eng_id" type="hidden" value="<?=$eng_id?>">
<div id="boxarea">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
<td class="tabcont">
<table width="100%" border="0" cellpadding="6" cellspacing="0">
	<tr>
		<td colspan="2" valign="middle" class="listtopic"><?php echo gettext("Suricata Target-Based OS Policy Engine Configuration"); ?></td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Engine Name"); ?></td>
		<td class="vtable">
			<input name="policy_name" type="text" class="formfld unknown" id="policy_name" size="25" maxlength="25" 
			value="<?=htmlspecialchars($pconfig['name']);?>"<?php if (htmlspecialchars($pconfig['name']) == "default") echo "readonly";?>>&nbsp;
			<?php if (htmlspecialchars($pconfig['name']) <> "default") 
					echo gettext("Name or description for this engine.  (Max 25 characters)");
				else
					echo "<span class=\"red\">" . gettext("The name for the 'default' engine is read-only.") . "</span>";?><br/>
			<?php echo gettext("Unique name or description for this engine configuration.  Default value is ") . 
			"<strong>" . gettext("default") . "</strong>"; ?>.<br/>
		</td>
	</tr>
	<tr>
		<td valign="top" class="vncell"><?php echo gettext("Bind-To IP Address Alias"); ?></td>
		<td class="vtable">
		<?php if ($pconfig['name'] <> "default") : ?>
			<table width="95%" border="0" cellpadding="2" cellspacing="0">
				<tr>
					<td class="vexpl"><input name="policy_bind_to" type="text" class="formfldalias" id="policy_bind_to" size="32" 
					value="<?=htmlspecialchars($pconfig['bind_to']);?>" title="<?=trim(filter_expand_alias($pconfig['bind_to']));?>" autocomplete="off">&nbsp;
					<?php echo gettext("IP List to bind this engine to. (Cannot be blank)"); ?></td>
					<td class="vexpl" align="right"><input type="button" class="formbtns" value="Aliases" onclick="parent.location='suricata_select_alias.php?id=<?=$id;?>&eng_id=<?=$eng_id;?>&type=host|network&varname=bind_to&act=import&multi_ip=yes&returl=<?=urlencode($_SERVER['PHP_SELF']);?>'" 
					title="<?php echo gettext("Select an existing IP alias");?>"/></td>
				</tr>
				<tr>
					<td class="vexpl" colspan="2"><?php echo gettext("This policy will apply for packets with destination addresses contained within this IP List.");?></td>
				</tr>
			</table>
			<span class="red"><strong><?php echo gettext("Note: ") . "</strong></span>" . gettext("Supplied value must be a pre-configured Alias or the keyword 'all'.");?>
			&nbsp;&nbsp;&nbsp;&nbsp;
		<?php else : ?>
			<input name="policy_bind_to" type="text" class="formfldalias" id="policy_bind_to" size="32" 
			value="<?=htmlspecialchars($pconfig['bind_to']);?>" autocomplete="off" readonly>&nbsp;
			<?php echo "<span class=\"red\">" . gettext("IP List for the default engine is read-only and must be 'all'.") . "</span>";?><br/>
			<?php echo gettext("The default engine is required and will apply for packets with destination addresses not matching other engine IP Lists.");?><br/>
		<?php endif ?>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="top" class="vncell"><?php echo gettext("Target Policy"); ?> </td>
		<td width="78%" class="vtable">
			<select name="policy" class="formselect" id="policy">
			<?php
			$profile = array( 'BSD', 'BSD-Right', 'HPUX10', 'HPUX11', 'Irix', 'Linux', 'Mac-OS', 'Old-Linux', 'Old-Solaris', 'Solaris', 'Vista', 'Windows', 'Windows2k3' );
			foreach ($profile as $val): ?>
			<option value="<?=strtolower($val);?>" 
			<?php if (strtolower($val) == $pconfig['policy']) echo "selected"; ?>>
				<?=gettext($val);?></option>
				<?php endforeach; ?>
			</select>&nbsp;&nbsp;<?php echo gettext("Choose the OS target policy appropriate for the protected hosts.  The default is ") . 
			"<strong>" . gettext("BSD") . "</strong>"; ?>.<br/><br/>
			<?php echo gettext("Available OS targets are BSD, BSD-Right, HPUX10, HPUX11, Irix, Linux, Mac-OS, Old-Linux, Old-Solaris, Solaris, Vista, 'Windows' and Windows2k3."); ?><br/>
		</td>
	</tr>
	<tr>
		<td width="22%" valign="bottom">&nbsp;</td>
		<td width="78%" valign="bottom">
			<input name="Submit" id="submit" type="submit" class="formbtn" value=" Save " title="<?php echo 
			gettext("Save OS policy engine settings and return to Flow/Stream tab"); ?>">
			&nbsp;&nbsp;&nbsp;&nbsp;
			<input name="Cancel" id="cancel" type="submit" class="formbtn" value="Cancel" title="<?php echo 
			gettext("Cancel changes and return to Flow/Stream tab"); ?>"></td>
	</tr>
</table>
</td>
</tr>
</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
<script type="text/javascript" src="/javascript/autosuggest.js">
</script>
<script type="text/javascript" src="/javascript/suggestions.js">
</script>
<script type="text/javascript">
//<![CDATA[
	var addressarray = <?= json_encode(get_alias_list(array("host", "network"))) ?>;

function createAutoSuggest() {
	<?php
		echo "\tvar objAlias = new AutoSuggestControl(document.getElementById('policy_bind_to'), new StateSuggestions(addressarray));\n";
	?>
}

setTimeout("createAutoSuggest();", 500);
//]]>

</script>

</html>
