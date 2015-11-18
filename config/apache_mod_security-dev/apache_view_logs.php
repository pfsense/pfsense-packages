<?php
/* ========================================================================== */
/*
	apache_view_logs.php
	part of pfSense (http://www.pfSense.com)
	Copyright (C) 2009, 2010 Scott Ullrich <sullrich@gmail.com>
	Copyright (C) 2012 Marcello Coutinho
	Copyright (C) 2012 Carlos Cesario
	All rights reserved.
                                                                              */
/* ========================================================================== */
/*
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	 1. Redistributions of source code MUST retain the above copyright notice,
		this list of conditions and the following disclaimer.

	 2. Redistributions in binary form MUST reproduce the above copyright
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
$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
if ($pf_version < 2.0)
        $one_two = true;

$pgtitle = "Status: Apache VirtualHost Logs";
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
function showLog(content,url,logtype)
{
		jQuery.ajax({
			type: 'get',
			cache: false,
			url: url,
			dataType: 'json',
			data: 	{
						maxlines: jQuery('#maxlines').val(), 
						strfilter: jQuery('#strfilter').val(), 
						logfile: jQuery('#logs').val(), 
						logtype: logtype
					},
			complete: function(data){
				jQuery('#'+content).empty().html(data.responseText);
			}
		});		
}


	// Call function to show squid log
	jQuery(document).ready(function() {
		var refreshId = setInterval( function() 
		{
			showLog('accesslog', 'apache_logs_data.php','access');
			showLog('errorlog', 'apache_logs_data.php','error');
		}, 1000);
	});

</script>
<div id="mainlevel">
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
			<?php
				$tab_array = array();
				$tab_array[] = array(gettext("Apache"), true, "/pkg_edit.php?xml=apache_settings.xml&amp;id=0");
				$tab_array[] = array(gettext("ModSecurity"), false, "/pkg_edit.php?xml=apache_mod_security_settings.xml");
				$tab_array[] = array(gettext("Sync"), false, "/pkg_edit.php?xml=apache_mod_security_sync.xml");
				display_top_tabs($tab_array);
			?>
			</td></tr>
			<tr><td>
				<?php 
				unset ($tab_array);
				$tab_array[] = array(gettext("Daemon Options"), false, "pkg_edit.php?xml=apache_settings.xml");
				$tab_array[] = array(gettext("Backends / Balancers"), false, "/pkg.php?xml=apache_balancer.xml");
				$tab_array[] = array(gettext("Location(s)"), false, "/pkg.php?xml=apache_location.xml");
				$tab_array[] = array(gettext("Virtual Hosts"), false, "/pkg.php?xml=apache_virtualhost.xml");
				$tab_array[] = array(gettext("Logs"), true, "/apache_view_logs.php");
				display_top_tabs($tab_array);
			?>
			</td></tr>
	 		<tr>
	    		<td>
<div id="mainarea" style="padding-top: 0px; padding-bottom: 0px; ">
	<form id="paramsForm" name="paramsForm" method="post">
		<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tbody>
				<tr>
					<td width="22%" valign="top" class="vncellreq"><?=gettext("Max. lines:");?></td>
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
						   <?=gettext("Max. lines to be displayed.");?>
						</span>
					</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncellreq"><?=gettext("Log file:");?></td>
					<td width="78%" class="vtable">
						<select name="logs" id="logs">
						<?php
						if ($handle = opendir('/var/log')) {
							/* This is the correct way to loop over the directory. */
							while (false !== ($entry = readdir($handle))) {
								if (preg_match("/httpd-(\S+).log/",$entry,$matches))
									if (!preg_match("/error/",$matches[1]))
										print "<option value={$matches[1]}>{$matches[1]}.log</option>\n";
							    }
							closedir($handle);
							}
						?>
						</select>
						<br/>
						<span class="vexpl">
						   <?=gettext("Max. lines to be displayed.");?>
						</span>
					</td>
				</tr>
				<tr>
					<td width="22%" valign="top" class="vncellreq"><?=gettext("String filter:");?></td>
					<td width="78%" class="vtable">
						<input name="strfilter" type="text" class="formfld search" id="strfilter" size="50" value="">
						<br/>
						<span class="vexpl">
						   <?=gettext("Enter a grep like string/pattern to filterlog.");?><br>
						   <?=gettext("eg. username, ip addr, url.");?><br>
						   <?=gettext("Use <b>!</b> to invert the sense of matching, to select non-matching lines.");?>
						</span>
					</td>
				</tr>
			</tbody>
		</table>
	</form>
	<div id="browserinfo" style='padding: 5px; border: 1px dashed #990000; font-weight:bold; font-size: 0.9em; text-align: center; margin: 1px; display:block; height: 12px;'> 
		<span></span>
	</div>
	<!-- Squid Table --> 
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tbody>
			<tr>
				<td>
					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td colspan="5" class="listtopic"><center><?=gettext("Httpd Access Log"); ?><center></td>
						</tr>
						<tbody id="accesslog">
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
							<td colspan="3" class="listtopic"><center><?=gettext("Http error logs"); ?><center></td>
						</tr>
						<tbody id="errorlog">

						</tbody>
					</table>
				</td>
			</tr>
		</tbody>
	</table>
</div>
</td>
</tr>
</table>
</div>


<?php
include("fend.inc");
?>

</body>
</html>
