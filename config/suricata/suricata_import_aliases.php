<?php
/* $Id$ */
/*
	suricata_import_aliases.php
	Copyright (C) 2014 Bill Meeks
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
require_once("functions.inc");
require_once("/usr/local/pkg/suricata/suricata.inc");

// Retrieve any passed QUERY STRING or POST variables
$id = $_GET['id'];
$eng = $_GET['eng'];
if (isset($_POST['id']))
	$id = $_POST['id'];
if (isset($_POST['eng']))
	$eng = $_POST['eng'];

// Make sure we have a valid rule ID and ENGINE name, or
// else bail out to top-level menu. 
if (is_null($id) || is_null($eng)) {
 	header("Location: /suricata/suricata_interfaces.php");
	exit;
}

// Used to track if any selectable Aliases are found
$selectablealias = false;

// Initialize required array variables as necessary
if (!is_array($config['aliases']['alias']))
	$config['aliases']['alias'] = array();
$a_aliases = $config['aliases']['alias'];
if (!is_array($config['installedpackages']['suricata']['rule']))
	$config['installedpackages']['suricata']['rule'] = array();

// The $eng variable points to the specific Suricata config section
// engine we are importing values into.  Initialize the config.xml
// array if necessary.
if (!is_array($config['installedpackages']['suricata']['rule'][$id][$eng]['item']))
	$config['installedpackages']['suricata']['rule'][$id][$eng]['item'] = array();

// Initialize a pointer to the Suricata config section engine we are
// importing values into.
$a_nat = &$config['installedpackages']['suricata']['rule'][$id][$eng]['item'];

// Build a lookup array of currently used engine 'bind_to' Aliases 
// so we can screen matching Alias names from the list.
$used = array();
foreach ($a_nat as $v)
	$used[$v['bind_to']] = true;

// Construct the correct return URL based on the Suricata config section
// engine we were called with.  This lets us return to the page we were
// called from.
switch ($eng) {
	case "host_os_policy":
		$returl = "/suricata/suricata_flow_stream.php";
		$multi_ip = true;
		$title = "Host Operating System Policy";
		break;
	case "libhtp_policy":
		$returl = "/suricata/suricata_app_parsers.php";
		$multi_ip = true;
		$title = "HTTP Server Policy";
		break;
	default:
		$returl = "/suricata/suricata_interface_edit";
		$multi_ip = true;
		$title = "";
}

if ($_POST['cancel']) {
	header("Location: {$returl}?id={$id}");
	exit;
}

if ($_POST['save']) {

	// Define default engine configurations for each of the supported engines.
	$def_os_policy = array( "name" => "", "bind_to" => "", "policy" => "bsd" );

	$def_libhtp_policy = array( "name" => "default", "bind_to" => "all", "personality" => "IDS", 
				    "request-body-limit" => 4096, "response-body-limit" => 4096, 
				    "double-decode-path" => "no", "double-decode-query" => "no" );

	// Figure out which engine type we are importing and set up default engine array
	$engine = array();
	switch ($eng) {
		case "host_os_policy":
			$engine = $def_os_policy;
			break;
		case "libhtp_policy":
			$engine = $def_libhtp_policy;
			break;
		default:
			$engine = "";
			$input_errors[] = gettext("Invalid ENGINE TYPE passed in query string.  Aborting operation.");
	}

	// See if anything was checked to import
	if (is_array($_POST['toimport']) && count($_POST['toimport']) > 0) {
		foreach ($_POST['toimport'] as $item) {
			$engine['name'] = strtolower($item);
			$engine['bind_to'] = $item;
			$a_nat[] = $engine;
		}
	}
	else
		$input_errors[] = gettext("No entries were selected for import.  Please select one or more Aliases for import and click SAVE.");

	// if no errors, write new entry to conf
	if (!$input_errors) {
		// Reorder the engine array to ensure the 
		// 'bind_to=all' entry is at the bottom if 
		// the array contains more than one entry.
		if (count($a_nat) > 1) {
			$i = -1;
			foreach ($a_nat as $f => $v) {
				if ($v['bind_to'] == "all") {
					$i = $f;
					break;
				}
			}
			// Only relocate the entry if we 
			// found it, and it's not already 
			// at the end.
			if ($i > -1 && ($i < (count($a_nat) - 1))) {
				$tmp = $a_nat[$i];
				unset($a_nat[$i]);
				$a_nat[] = $tmp;
			}
		}

		// Now write the new engine array to conf and return
		write_config();

		header("Location: {$returl}?id={$id}");
		exit;
	}
}

$pgtitle = gettext("Suricata: Import Host/Network Alias for {$title}");
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="suricata_import_aliases.php" method="post">
<input type="hidden" name="id" value="<?=$id;?>">
<input type="hidden" name="eng" value="<?=$eng;?>">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<div id="boxarea">
<table width="100%" border="0" cellpadding="0" cellspacing="0">
<tr>
	<td class="tabcont"><strong><?=gettext("Select one or more Aliases to use as {$title} targets from the list below.");?></strong><br/>
	</td>
</tr>
<tr>
	<td class="tabcont">
		<table id="sortabletable1" style="table-layout: fixed;" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
			<colgroup>
				<col width="5%" align="center">
				<col width="25%" align="left" axis="string">
				<col width="35%" align="left" axis="string">
				<col width="35%" align="left" axis="string">
			</colgroup>
			<thead>
			   <tr>
				<th class="listhdrr"></th>
				<th class="listhdrr" axis="string"><?=gettext("Alias Name"); ?></th>
				<th class="listhdrr" axis="string"><?=gettext("Values"); ?></th>
				<th class="listhdrr" axis="string"><?=gettext("Description"); ?></th>
			   </tr>
			</thead>
		<tbody>
		  <?php $i = 0; foreach ($a_aliases as $alias): ?>
			<?php if ($alias['type'] <> "host" && $alias['type'] <> "network")
				continue;
			      if (isset($used[$alias['name']]))
				continue;
			      elseif (trim(filter_expand_alias($alias['name'])) == "") {
				$textss = "<span class=\"gray\">";
				$textse = "</span>";
				$disable = true;
			        $tooltip = gettext("Aliases representing a FQDN host cannot be used in Suricata Host OS Policy configurations.");
			      }
			      else {
				$textss = "";
				$textse = "";
				$disable = "";
				$selectablealias = true;
			        $tooltip = gettext("Selected entries will be imported. Click to toggle selection of this entry.");
			      }
			?>
			<?php if ($disable): ?>
			<tr title="<?=$tooltip;?>">
			  <td class="listlr" align="center"><img src="../themes/<?=$g['theme'];?>/images/icons/icon_block_d.gif" width="11" height"11" border="0"/>
			<?php else: ?>
			<tr>
			  <td class="listlr" align="center"><input type="checkbox" name="toimport[]" value="<?=htmlspecialchars($alias['name']);?>" title="<?=$tooltip;?>"/></td>
			<?php endif; ?>
			  <td class="listr" align="left"><?=$textss . htmlspecialchars($alias['name']) . $textse;?></td>
			  <td class="listr" align="left">
			      <?php
				$tmpaddr = explode(" ", $alias['address']);
				$addresses = implode(", ", array_slice($tmpaddr, 0, 10));
				echo "{$textss}{$addresses}{$textse}";
				if(count($tmpaddr) > 10) {
					echo "...";
				}
			    ?>
			  </td>
			  <td class="listbg" align="left">
			    <?=$textss . htmlspecialchars($alias['descr']) . $textse;?>&nbsp;
			  </td>
			</tr>
		  <?php $i++; endforeach; ?>
		</table>
	</td>
</tr>
<?php if (!$selectablealias): ?>
<tr>
	<td class="tabcont" align="center"><b><?php echo gettext("There are currently no defined Aliases eligible for import.");?></b></td>
</tr>
<tr>
	<td class="tabcont" align="center">
	<input type="Submit" name="cancel" value="Cancel" id="cancel" class="formbtn" title="<?=gettext("Cancel import operation and return");?>"/>
	</td>
</tr>
<?php else: ?>
<tr>
	<td class="tabcont" align="center">
	<input type="Submit" name="save" value="Save" id="save" class="formbtn" title="<?=gettext("Import selected item and return");?>"/>&nbsp;&nbsp;&nbsp;
	<input type="Submit" name="cancel" value="Cancel" id="cancel" class="formbtn" title="<?=gettext("Cancel import operation and return");?>"/>
	</td>
</tr>
<?php endif; ?>
<tr>
	<td class="tabcont">
	<span class="vexpl"><span class="red"><strong><?=gettext("Note:"); ?><br></strong></span><?=gettext("Fully-Qualified Domain Name (FQDN) host Aliases cannot be used as Suricata configuration parameters.  Aliases resolving to a single FQDN value are disabled in the list above.  In the case of nested Aliases where one or more of the nested values is a FQDN host, the FQDN host will not be included in the {$title} configuration.");?></span>
	</td>
</tr>
</table>
</div>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
