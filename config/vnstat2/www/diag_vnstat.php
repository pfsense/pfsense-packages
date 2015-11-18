<?php
/*
	diag_vnstat.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009 PerryMason
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
require_once("guiconfig.inc");
$pgtitle = gettext("Vnstat2 summary");
if ($_REQUEST['getactivity']) {
	$text = shell_exec("/usr/local/bin/vnstat");
	$text .= "<p/>";
	echo $text;
	exit;
}

include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<script type="text/javascript">
//<![CDATA[
	function getcpuactivity() {
		var url = "/diag_vnstat.php";
		var pars = 'getactivity=yes';
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'post',
				parameters: pars,
				onComplete: activitycallback
			});
	}
	function activitycallback(transport) {
		$('cpuactivitydiv').innerHTML = '<font face="Courier"><font size="2"><b><pre>' + transport.responseText + '</pre></font>';
		setTimeout('getcpuactivity()', 2000);
	}
	setTimeout('getcpuactivity()', 5000);
//]]>
</script>
<div id='maincontent'>
<?php
	include("fbegin.inc");

	if ($savemsg) {
		echo "<div id='savemsg'>";
		print_info_box($savemsg);
		echo "</div>";
	}
	if ($input_errors) {
		print_input_errors($input_errors);
	}
?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
			<table id="backuptable" class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
				<tr>
					<td>
						<table>
							<tr>
								<td>
									<div name='cpuactivitydiv' id='cpuactivitydiv'>
										<strong><?=gettext("Gathering vnstat information, please wait...");?></strong>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
