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

<!-- Function to call squid logs -->
<script language="JavaScript">
    function ShowLog(content,url,program)
    {
        var v_maxlines  = $('maxlines').getValue();
        var v_strfilter = $('strfilter').getValue();
        var pars = 'maxlines='+escape(v_maxlines) + '&strfilter=' + escape(v_strfilter) + '&program=' + escape(program);
    	new Ajax.Updater(content,url, {
	    				method: 'post',
                        parameters: pars,
			    		onSuccess: function() {
				    	    window.setTimeout( ShowLog(content,url,program), 100 );
                        }
        });
    }


</script>


<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td id="mainarea">
			<div class="tabcont">
				<div id="param">
					<form id="paramsForm" name="paramsForm" method="post">
						<table width="100%" border="0" cellpadding=5" cellspacing="0">
							<tr>
								<td width="15%" valign="top" class="vncell"><?php echo "Max lines:"; ?></td>
								<td width="85%" class="vtable">
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
									   <?php echo "Max. lines to be displayed."; ?>
									</span>
								</td>
							</tr>
							<tr>
								<td width="15%" valign="top" class="vncell"><?php echo "String filter:"; ?></td>
								<td width="85%" class="vtable">
									<input name="strfilter" type="text" class="formfld unknown" id="strfilter" size="50" value="">
									<br/>
									<span class="vexpl">
									   <?php echo "Enter the string filter: eg. username or ip addr or url."; ?>
									</span>
								</td>
							</tr>
						</table>
					</form>
				</div>

				<form>
					<table width="100%" border="0" cellpadding="0" cellspacing="0">
						<tr>
							<td colspan="2" valign="top" class="listtopic">
								<center>
									Squid Proxy
								</center>
							</td>
						</tr>
						<tr>
							<td>
								<table iD="squidView" width="100%" border="0" cellpadding="0" cellspacing="0">
									<script language="JavaScript">
                                        ShowLog('squidView', 'squid_monitor_data.php','squid');
									</script>
								</table>
							</td>
						</tr>
						<tr>
							<td colspan="2" valign="top" class="listtopic">
								<center>
									SquidGuard
								</center>
							</td>
						</tr>
						<tr>
							<td>
								<table id="sguardView" width="100%" border="0" cellpadding="5" cellspacing="0">
									<script language="JavaScript">
										ShowLog('sguardView', 'squid_monitor_data.php','sguard');
									</script>
								</table>
							</td>
						</tr>
					</table>
				</form>
			</div>
		</td>
	</tr>
</table>

<?php
include("fend.inc");
?>

</body>
</html>

