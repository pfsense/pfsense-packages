
var snortlines = Array();
var snorttimer;
var snortupdateDelay = 22000;
var snortisBusy = false;
var snortisPaused = false;

function snort_alerts_fetch_new_rules_callback(callback_data) {
	if(snortisPaused)
		return;

	var data_split;
	var new_data_to_add = Array();
	var data = callback_data.content;

	data_split = data.split("\n");

	for(var x=0; x<data_split.length-1; x++) {
		/* loop through rows */
		row_split = data_split[x].split("||");
		var line = '';
		line = '<td width="30%"  class="listr" >' + row_split[6]  + '<br>' + row_split[7]+ '</td>';		
		line += '<td width="40%"  class="listr" >' + row_split[3] + '<br>' + row_split[4] + '</td>';
		line += '<td width="40%" class="listr" >' + 'Pri : ' +  row_split[1] + '<br>' + 'Cat : ' + row_split[2] + '</td>';
		new_data_to_add[new_data_to_add.length] = line;
	}
	snort_alerts_update_div_rows(new_data_to_add);
	snortisBusy = false;
}
function snort_alerts_update_div_rows(data) {
	if(snortisPaused)
		return;

	var isIE = navigator.appName.indexOf('Microsoft') != -1;
	var isSafari = navigator.userAgent.indexOf('Safari') != -1;
	var isOpera = navigator.userAgent.indexOf('Opera') != -1;
	var rulestable = document.getElementById('snort_alerts');
	var rows = rulestable.getElementsByTagName('tr');
	var showanim = 1;
	if (isIE) {
		showanim = 0;
	}
	//alert(data.length);
	for(var x=0; x<data.length; x++) {
		var numrows = rows.length;
		/*    if reverse logging is enabled we need to show the
		 *    records in a reverse order with new items appearing
		 *    on the top
		 */
		if(isReverse == false) {
			for (var i = 1; i < numrows; i++) {
				nextrecord = i + 1;
				if(nextrecord < numrows)
					rows[i].innerHTML = rows[nextrecord].innerHTML;
			}
		} else {
			for (var i = numrows; i > 0; i--) {
				nextrecord = i + 1;
				if(nextrecord < numrows)
					rows[nextrecord].innerHTML = rows[i].innerHTML;
			}
		}
		var item = document.getElementById('snort-firstrow');
		if(x == data.length-1) {
			/* nothing */
			showanim = false;
		} else {
			showanim = false;
		}
		if (showanim) {
			item.style.display = 'none';
			item.innerHTML = data[x];
			new Effect.Appear(item);
		} else {
			item.innerHTML = data[x];
		}
	}
	/* rechedule AJAX interval */
	snorttimer = setInterval('snort_alerts_fetch_new_rules()', snortupdateDelay);
}
function snort_alerts_toggle_pause() {
	if(snortisPaused) {
		snortisPaused = false;
		snort_alerts_fetch_new_rules();
	} else {
		snortisPaused = true;
	}
}
/* start local AJAX engine */
snorttimer = setInterval('snort_alerts_fetch_new_rules()', snortupdateDelay);
