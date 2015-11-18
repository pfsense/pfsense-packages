/*
	havp_alerts.js
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009 Jim Pingle
	Copyright (C) 2015 ESF, LLC
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

havplastsawtime = '<?php echo time(); ?>';
var havplines = Array();
var havptimer;
var havpupdateDelay = 25500;
var havpisBusy = false;
var havpisPaused = false;

<?php
	if (isset($config['syslog']['reverse'])) {
		echo "var isReverse = true;\n";
	} else {
		echo "var isReverse = false;\n";
	}
?>

if (typeof getURL == 'undefined') {
	getURL = function(url, callback) {
		if (!url)
			throw 'No URL for getURL';
		try {
			if (typeof callback.operationComplete == 'function')
				callback = callback.operationComplete;
		} catch (e) {}
		if (typeof callback != 'function')
			throw 'No callback function for getURL';
		var http_request = null;
		if (typeof XMLHttpRequest != 'undefined') {
			http_request = new XMLHttpRequest();
		} else if (typeof ActiveXObject != 'undefined') {
			try {
				http_request = new ActiveXObject('Msxml2.XMLHTTP');
			} catch (e) {
				try {
					http_request = new ActiveXObject('Microsoft.XMLHTTP');
				} catch (e) {}
			}
		}
		if (!http_request)
			throw 'Both getURL and XMLHttpRequest are undefined';
		http_request.onreadystatechange = function() {
			if (http_request.readyState == 4) {
				callback({
					success: true,
					content: http_request.responseText,
					contentType: http_request.getResponseHeader("Content-Type")
				});
			}
		}
		http_request.open('GET', url, true);
		http_request.send(null);
	}
}

function havp_alerts_fetch_new_rules() {
	if (havpisPaused)
		return;
	if (havpisBusy)
		return;
	havpisBusy = true;
	getURL('widgets/helpers/havp_alerts_helper.php?lastsawtime=' + havplastsawtime, havp_alerts_fetch_new_rules_callback);
}

function havp_alerts_fetch_new_rules_callback(callback_data) {
	if (havpisPaused)
		return;

	var data_split;
	var new_data_to_add = Array();
	var data = callback_data.content;
	data_split = data.split("\n");
	for (var x = 0; x < data_split.length - 1; x++) {
		/* loop through rows */
		row_split = data_split[x].split("||");
		var line = '';
		line += '<td width="25%" class="listr">' + row_split[4] + '<br/> ' + row_split[3] + '</td>';
		line += '<td width="75%" class="listr">' + row_split[0] + '<br/>' + row_split[1] + '</td>';
		havplastsawtime = row_split[2];
		new_data_to_add[new_data_to_add.length] = line;
	}
	havp_alerts_update_div_rows(new_data_to_add);
	havpisBusy = false;
}

function havp_alerts_update_div_rows(data) {
	if (havpisPaused)
		return;

	var isIE = navigator.appName.indexOf('Microsoft') != -1;
	var isSafari = navigator.userAgent.indexOf('Safari') != -1;
	var isOpera = navigator.userAgent.indexOf('Opera') != -1;
	var rulestable = document.getElementById('havp_alerts');
	var rows = rulestable.getElementsByTagName('tr');
	var showanim = 1;
	if (isIE) {
		showanim = 0;
	}
	//alert(data.length);
	for (var x = 0; x < data.length; x++) {
		var numrows = rows.length;
		// If reverse logging is enabled we need to show the records
		// in a reverse order with new items appearing on the top.
		if (isReverse == false) {
			for (var i = 1; i < numrows; i++) {
				nextrecord = i + 1;
				if (nextrecord < numrows)
					rows[i].innerHTML = rows[nextrecord].innerHTML;
			}
		} else {
			for (var i = numrows; i > 0; i--) {
				nextrecord = i + 1;
				if (nextrecord < numrows)
					rows[nextrecord].innerHTML = rows[i].innerHTML;
			}
		}
		var item = document.getElementById('havp-firstrow');
		if (x == data.length - 1) {
			/* nothing */
			showanim = false;
		} else {
			showanim = false;
		}
		if (showanim) {
			//item.style.display = 'none';
			item.innerHTML = data[x];
			//new Effect.Appear(item);
		} else {
			item.innerHTML = data[x];
		}
	}
	/* rechedule AJAX interval */
	//havptimer = setInterval('havp_alerts_fetch_new_rules()', havpupdateDelay);
}

function havp_alerts_toggle_pause() {
	if (havpisPaused) {
		havpisPaused = false;
		havp_alerts_fetch_new_rules();
	} else {
		havpisPaused = true;
	}
}
/* start local AJAX engine */
havptimer = setInterval('havp_alerts_fetch_new_rules()', havpupdateDelay);
