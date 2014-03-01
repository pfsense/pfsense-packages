
var suricatalines = Array();
var suricatatimer;
var suricataisBusy = false;
var suricataisPaused = false;

function suricata_alerts_fetch_new_rules() {

	//get new alerts from suricata_alerts.widget.php
	url = "/widgets/widgets/suricata_alerts.widget.php?getNewAlerts=1" + new Date().getTime();

	jQuery.ajax(url, {
		type: 'GET',
		success: function(callback_data) {
			var data_split;
			var new_data_to_add = Array();
			var data = callback_data;

			data_split = data.split("\n");

			// Loop through rows and generate replacement HTML
			for(var x=0; x<data_split.length-1; x++) {
				row_split = data_split[x].split("||");
				var line = '';
				line = '<td width="22%" class="listMRr" nowrap>' + row_split[0] + '<br/>' + row_split[1] + '</td>';		
				line += '<td width="39%" class="listMRr">' + row_split[2] + '<br/>' + row_split[3] + '</td>';
				line += '<td width="39%" class="listMRr">' + 'Priority: ' +  row_split[4] + '<br/>' + row_split[5] + '</td>';
				new_data_to_add[new_data_to_add.length] = line;
			}
			suricata_alerts_update_div_rows(new_data_to_add);
			suricataisBusy = false;
		}
	});
}
function suricata_alerts_update_div_rows(data) {
	if(suricataisPaused)
		return;

	var isIE = navigator.appName.indexOf('Microsoft') != -1;
	var isSafari = navigator.userAgent.indexOf('Safari') != -1;
	var isOpera = navigator.userAgent.indexOf('Opera') != -1;

	var rows = jQuery('#suricata-alert-entries>tr');

	// Number of rows to move by
	var move = rows.length + data.length - nentries;
	if (move < 0)
		move = 0;

	for (var i = move; i < rows.length; i++) {
		jQuery(rows[i - move]).html(jQuery(rows[i]).html());
	}

	var tbody = jQuery('#suricata-alert-entries');
	for (var i = 0; i < data.length; i++) {
		var rowIndex = rows.length - move + i;
		if (rowIndex < rows.length) {
			jQuery(rows[rowIndex]).html(data[i]);
		} else {
			jQuery(tbody).append('<tr>' + data[i] + '</tr>');
		}
	}

	// Add the even/odd class to each of the rows now
	// they have all been added.
	rows = jQuery('#suricata-alert-entries>tr');
	for (var i = 0; i < rows.length; i++) {
		rows[i].className = i % 2 == 0 ? 'listMRodd' : 'listMReven';
	}
}

function fetch_new_surialerts() {
	if(suricataisPaused)
		return;
	if(suricataisBusy)
		return;

	//get new alerts from suricata_alerts.widget.php
	suricataisBusy = true;
	suricata_alerts_fetch_new_rules();
}

function suricata_alerts_toggle_pause() {
	if(suricataisPaused) {
		suricataisPaused = false;
		fetch_new_surialerts();
	} else {
		suricataisPaused = true;
	}
}
/* start local AJAX engine */
suricatatimer = setInterval('fetch_new_surialerts()', suricataupdateDelay);
