<?php
/* $Id$ */
/* ========================================================================== */
/*
    squid_monitor.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2012 ccesario @ pfsense forum
    All rights reserved.

/* ========================================================================== */
/*
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
/* ========================================================================== */

require_once("/etc/inc/util.inc");
require_once("/etc/inc/functions.inc");
require_once("/etc/inc/pkg-utils.inc");
require_once("/etc/inc/globals.inc");
require_once("guiconfig.inc");

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
        $one_two = true;

$pgtitle = "Status: Proxy Monitor";
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>

<?php if($one_two): ?>

	<p class="pgtitle"><?=$pgtitle?></font></p>

<?php endif; ?>

<?php if ($savemsg) print_info_box($savemsg); ?>

<!-- Function to call programs logs -->
<script language="JavaScript">
	function showLog(content,url,program)
    {
		new PeriodicalExecuter(function(pe) {
			new Ajax.Updater(content, url, { 
									method: 'post', 
									asynchronous: true, 
									evalScripts: true, 
									parameters: { 	maxlines: $('maxlines').getValue(), 
			   										strfilter: $('strfilter').getValue(), 
													program: program } 
			}) 
		}, 1)
	}
</script>


<div id="mainarea" style="padding-top: 0px; padding-bottom: 0px; ">
	<form id="paramsForm" name="paramsForm" method="post">
		<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tbody>
				<tr>
					<td width="22%" valign="top" class="vncellreq">Max lines:</td>
					<td width="78%" class="vtable">
						<select name="maxlines" id="maxlines">
							<option value="5">5 lines</option>
							<option value="10" selected="selected">10 lines</option>
							<option value="15">15 lines</option>
							<option value="20">20 lines</option>
							<option value="25">25 lines</option>
							<option value="30">30 lines</option>
						</select>
						<br/>
						<span class="vexpl">
						   Max. lines to be displayed.
						</span>
					</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncellreq">String filter:</td>
					<td width="78%" class="vtable">
						<input name="strfilter" type="text" class="formfld search" id="strfilter" size="50" value="">
						<br/>
						<span class="vexpl">
						   Enter the string filter: eg. username or ip addr or url.	
						</span>
					</td>
				</tr>
			</tbody>
		</table>
	</form>

	<!-- Squid Table --> 
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tbody>
			<tr>
				<td>
					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td colspan="6" class="listtopic"><center><?=gettext("Squid Logs"); ?><center></td>
						</tr>
						<tbody id="squidView">
							<script language="JavaScript">
								// Call function to show squid log
								showLog('squidView', 'squid_monitor_data.php','squid');
							</script>
						</tbody>
					</table>
				</td>
			</tr>
		</tbody>
	</table>

	<!-- SquidGuard Table --> 
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tbody>
			<tr>
				<td>
					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td colspan="5" class="listtopic"><center><?=gettext("SquidGuard Logs"); ?><center></td>
						</tr>
						<tbody id="sguardView">
							<script language="JavaScript">
								// Call function to show squidGuard log
								showLog('sguardView', 'squid_monitor_data.php','sguard');
							</script>
						</tbody>
					</table>
				</td>
			</tr>
		</tbody>
	</table>
</div>

<?php
include("fend.inc");
?>

</body>
</html>
