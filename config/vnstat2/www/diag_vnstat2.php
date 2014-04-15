<?php
/* $Id$ */
/*
    diag_system_pftop.php
    Copyright (C) 2008-2009 Scott Ullrich
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

/*
	pfSense_MODULE:	filter
*/

##|+PRIV
##|*IDENT=page-diag-system-activity
##|*NAME=Diagnostics: System Activity
##|*DESCR=Allows access to the 'Diagnostics: System Activity' page
##|*MATCH=diag_system_pftop*
##|-PRIV

require("guiconfig.inc");
global $config;
$aaaa = $config['installedpackages']['vnstat2']['config'][0]['vnstat_interface2'];
$bbbb = convert_real_interface_to_friendly_descr($aaaa);

$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
$pgtitle = gettext("Vnstat2 info for $bbbb ($aaaa)");

if($_REQUEST['getactivity']) {
	if($_REQUEST['sorttype'])
		$sorttype = escapeshellarg($_REQUEST['sorttype']);
	else
		$sorttype = gettext("-h");	
	$text = `vnstat -i $aaaa {$sorttype}`;
	echo $text;
	exit;
}

include("head.inc");

if($_REQUEST['sorttype'])
	$sorttype = htmlentities($_REQUEST['sorttype']);
else
	$sorttype = "-h";

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<form method="post" action="diag_vnstat2.php">
<script type="text/javascript">
	function getcpuactivity() {
		var url = "/diag_vnstat2.php";
		var pars = 'getactivity=yes&sorttype=' + $('sorttype').value;
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'post',
				parameters: pars,
				onComplete: activitycallback
			});
	}
	function activitycallback(transport) {
		$('cpuactivitydiv').innerHTML = '<font face="Courier"><font size="2"><b><pre>' + transport.responseText  + '</pre></font>';
		setTimeout('getcpuactivity()', 2500);		
	}
	setTimeout('getcpuactivity()', 1000);	
</script>
<div id='maincontent'>
<?php
	include("fbegin.inc"); 
	if ($pf_version < 2.0)
		echo "<p class=\"pgtitle\">{$pgtitle}</p>";
		echo "<a href=$myurl/pkg_edit.php?xml=vnstatoutput.xml&id=0>Go Back</a><br />";
	if($savemsg) {
		echo "<div id='savemsg'>";
		print_info_box($savemsg);
		echo "</div>";	
	}
	if ($input_errors)
		print_input_errors($input_errors);
?>
	<form method="post">
	<?=gettext("Sort type:"); ?>
	<select name='sorttype' id='sorttype' onChange='this.form.submit();'>
		<option value='<?=$sorttype?>'><?=$sorttype?></option>
		<option value='-h'><?=gettext("Show traffic for the last 24 hours.");?></option>
		<option value='-d'><?=gettext("Show traffic for days.");?></option>
		<option value='-m'><?=gettext("Show traffic for months.");?></option>
		<option value='-t'><?=gettext("Show all time top10 traffic.");?></option>
		<option value='-tr'><?=gettext("Calculate 5sec. of traffic.");?></option>
		<option value='-w'><?=gettext("Show traffic for 7 days, current and previous week.");?></option>														
	</select>
	<p/>
<table width="100%" border="0" cellpadding="0" cellspacing="0">  
  <tr>
    <td>
	<table id="backuptable" class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td>
				<center>
				<table>
					<tr><td>
						<div name='cpuactivitydiv' id='cpuactivitydiv'>
							<b><?=gettext("Gathering vnstat activity, please wait...");?>
						</div>
					</td></tr>
				</table>
			</td>
		</tr>
	</table>
	</div>
    </td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
