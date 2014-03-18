<?php
/*
	haproxy_htmllist.php
	part of pfSense (https://www.pfsense.org/)
	Copyright (C) 2013 PiBa-NL
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
	AUTHOR BE LIABLE FOR ANY DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/*
  This file contains functions which are NOT specific to HAProxy and may/could/should 
  be moved to the general pfSense php library for possible easy use by other parts of pfSense
*/

require_once("config.inc");
require_once("haproxy_utils.inc");

function haproxy_htmllist_get_values($html_list){
	$values = array();
	for($x=0; $x<99; $x++) {
		$value = array();
		$add_item = false;
		foreach($html_list as $item){
			$itemname = $item['name'];
			$value[$itemname] = $_POST[$itemname.$x];
			$add_item |= isset($_POST[$itemname.$x]);
		}
		if ($add_item)
			$values[] = $value;
	}
	return $values;
}

function haproxy_htmllist($tablename,$values,$items,$editstate=false){
	global $g, $counter;
	echo "<table class='' width='100%' cellpadding='0' cellspacing='0' id='$tablename'>
	<tr>";
	foreach($items as $item){
		echo "<td width='{$item['colwidth']}' class='listhdrr'>{$item['columnheader']}</td>";
	}
	echo "<td width='5%' class=''></td>
	</tr>";
	if (is_array($values)){
		foreach($values as $value){
			if (!$editstate) {
				echo "<tr id='tr_view_$counter' ondblclick='editRow($counter); return false;' >";
				$leftitem = true;
				foreach($items as $item){
					$tdclass = $leftitem ? "vtable listlr" : "vtable listr";
					echo "<td class='$tdclass'>";
					$itemname = $item['name'];
					$itemtype = $item['type'];
					$itemvalue = $value[$itemname];
					if ($itemtype == "select"){
						echo $item['items'][$itemvalue]['name'];
					} else
					if ($itemtype == "checkbox"){
						echo $itemvalue=='yes' ? gettext('yes') : gettext('no');
					} else
						echo $itemvalue;
					echo "</td>";
					$leftitem = false;
				}
				echo "
		<td class='list'>
			<table border='0' cellspacing='0' cellpadding='1'><tr>
			<td valign='middle'>
			<img src='/themes/{$g['theme']}/images/icons/icon_e.gif' title='edit entry' width='17' height='17' border='0' onclick='editRow($counter); return false;' />
			</td>
			<td valign='middle'>
			<img src='/themes/{$g['theme']}/images/icons/icon_x.gif' title='delete entry' width='17' height='17' border='0' onclick='deleteRow($counter, \"$tablename\"); return false;' />
			</td>
			<td valign='middle'>
			<img src='/themes/{$g['theme']}/images/icons/icon_plus.gif' title='duplicate entry' width='17' height='17' border='0' onclick='dupRow($counter, \"$tablename\"); return false;' />
			</td></tr></table>
		</td>";
				echo "</tr>";
			}			
			$displaystyle = $editstate ? "" : "display: none;";
			echo "<tr id='tr_edit_$counter' style='$displaystyle'>";
			foreach($items as $item){
				$itemname = $item['name'];
				$itemtype = $item['type'];
				$itemvalue = $value[$itemname];
				$itemnamenr = $itemname.$counter;
				echo "<td class='vtable'>";
				if ($itemtype == "select"){
					echo_html_select($itemnamenr, $item['items'], $itemvalue,"","updatevisibility();", "width:{$item['size']}");
				} else
				if ($itemtype == "checkbox"){
					$checked = $itemvalue=='yes' ? " checked" : "";
					echo "<input name='$itemnamenr' id='$itemnamenr' type='checkbox'$checked value='yes' size='{$item['size']}' />";
					
				}  else
					echo "<input name='$itemnamenr' id='$itemnamenr' type='text' value='{$itemvalue}' size='{$item['size']}' />";
				echo "</td>";
			}
			echo "
	<td class='list'>
		<table border='0' cellspacing='0' cellpadding='1'><tr>
		<td valign='middle'>
		<img src='/themes/{$g['theme']}/images/icons/icon_x.gif' title='delete entry' width='17' height='17' border='0' onclick='removeRow(this); return false;' />
		</td>
		<td valign='middle'>
		<img src='/themes/{$g['theme']}/images/icons/icon_plus.gif' title='duplicate entry' width='17' height='17' border='0' onclick='dupRow($counter, \"$tablename\"); return false;' />
		</td></tr></table>
	</td>";
			echo "</tr>";
			$counter++;
		}
	}
	echo "</table>
		<a onclick='javascript:addRowTo(\"$tablename\"); return false;' href='#'>
		<img border='0' src='/themes/{$g['theme']}/images/icons/icon_plus.gif' alt='' title='add another entry' />
		</a>";
}

function haproxy_htmllist_js(){
?><script type="text/javascript">	
	function htmllist_get_select_items(tableId) {
		var items;
		var i = tableId.lastIndexOf('_');
		var items_name = "fields_"+tableId.substr(i+1);
		items = eval(items_name);
		return items;
	}

	var addRowTo = (function() {
	return (function (tableId) {
		var d, tbody, tr, td, bgc, i, ii, j, type, seltext, items;
		var btable, btbody, btr, btd;
		d = document;
		
		items = htmllist_get_select_items(tableId);
		seltext = htmllist_get_select_options(tableId);

		tbody = d.getElementById(tableId).getElementsByTagName("tbody").item(0);
		tr = d.createElement("tr");
		totalrows++;
		tr.setAttribute("id","aclrow" + totalrows);
		
		for (var i in items) {
			td = d.createElement("td");
			if(items[i]['type'] == 'textbox') {
				td.innerHTML="<input size='" + items[i]['size'] + "' name='" + items[i]['name'] + totalrows +
				"' id='" + items[i]['name'] + totalrows +
				"'><\/input> ";
			} else if(items[i]['type'] == 'select') {
				td.innerHTML="<select style='width:" + items[i]['size'] + "'  name='" + items[i]['name'] + totalrows + 
				"' id='" + items[i]['name'] + totalrows +
				"'>" + seltext + "<\/select> ";
			} else {
				td.innerHTML="<input type='checkbox' name='" + items[i]['name'] + totalrows +
				"' id='" + items[i]['name'] + totalrows + "' value='yes'><\/input> ";
			}
			tr.appendChild(td);
		}
		td = d.createElement("td");
		td.rowSpan = "1";
		td.setAttribute("class","list");

		// Recreate the button table.
		btable = document.createElement("table");
		btable.setAttribute("border", "0");
		btable.setAttribute("cellspacing", "0");
		btable.setAttribute("cellpadding", "1");
		btbody = document.createElement("tbody");
		btr = document.createElement("tr");
		btd = document.createElement("td");
		btd.setAttribute("valign", "middle");
		btd.innerHTML = '<img src="/themes/' + theme + '/images/icons/icon_x.gif" title="delete entry" width="17" height="17" border="0" onclick="removeRow(this); return false;" />';
		btr.appendChild(btd);
		btd = document.createElement("td");
		btd.setAttribute("valign", "middle");
		btd.innerHTML = '<img src="/themes/' + theme + "/images/icons/icon_plus.gif\" title=\"duplicate entry\" width=\"17\" height=\"17\" border=\"0\" onclick=\"dupRow(" + totalrows + ", '" + tableId + "'); return false;\" />";
		btr.appendChild(btd);
		btbody.appendChild(btr);
		btable.appendChild(btbody);
		td.appendChild(btable);
		tr.appendChild(td);
		tbody.appendChild(tr);
	});
	})();
	function dupRow(rowId, tableId) {
		var dupEl;
		var newEl;
		addRowTo(tableId);
		items = htmllist_get_select_items(tableId);
		for (var i in items) {
			dupEl = document.getElementById(items[i]['name'] + rowId);
			newEl = document.getElementById(items[i]['name'] + totalrows);
			if (dupEl && newEl)
				if(items[i]['type'] == 'checkbox')
					newEl.checked = dupEl.checked;
				else
					newEl.value = dupEl.value;
		}
	}
	function editRow(num) {
		var trview = document.getElementById('tr_view_' + num);
		var tredit = document.getElementById('tr_edit_' + num);
		trview.style.display='none';
		tredit.style.display='';
	}
	function deleteRow(rowId, tableId) {
		var view = document.getElementById("tr_view_" + rowId);
		var edit = document.getElementById("tr_edit_" + rowId);
		view.parentNode.removeChild(view);
		edit.parentNode.removeChild(edit);
	}
	function removeRow(el) {
		var cel;
		// Break out of one table first
		while (el && el.nodeName.toLowerCase() != "table")
			el = el.parentNode;
		while (el && el.nodeName.toLowerCase() != "tr")
			el = el.parentNode;

		if (el && el.parentNode) {
			cel = el.getElementsByTagName("td").item(0);
			el.parentNode.removeChild(el);
		}
	}
</script><?
}

?>
