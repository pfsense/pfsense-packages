<?php
/* $Id$ */
/*
 snort_interfaces.php
 part of m0n0wall (http://m0n0.ch/wall)

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 Copyright (C) 2008-2009 Robert Zelaya.
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
require_once("/usr/local/pkg/snort/snort_new.inc");
require_once("/usr/local/pkg/snort/snort_gui.inc");

// set page vars

$uuid = $_GET['uuid'];
if (isset($_POST['uuid']))
$uuid = $_POST['uuid'];

if ($uuid == '') {
	echo 'error: no uuid';
	exit(0);
}


$a_list = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'uuid', $uuid);

	$pgtitle = "Snort: Interface Preprocessors and Flow";
	include("/usr/local/pkg/snort/snort_head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<div id="loadingWaiting">
  <p class="loadingWaitingMessage"><img src="./images/loading.gif" /> <br>Please Wait...</p>
</div>

<?php include("fbegin.inc"); ?>
<!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2">
<a href="../index.php" id="status-link2">
<img src="./images/transparent.gif" border="0"></img>
</a>
</div>

<div class="body2"><!-- hack to fix the hardcoed fbegin link in header -->
<div id="header-left2"><a href="../index.php" id="status-link2"><img src="./images/transparent.gif" border="0"></img></a></div>

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>

		<div class="newtabmenu" style="margin: 1px 0px; width: 775px;"><!-- Tabbed bar code-->
		<ul class="newtabmenu">
				<li><a href="/snort/snort_interfaces.php"><span>Snort Interfaces</span></a></li>
				<li><a href="/snort/snort_interfaces_edit.php?uuid=<?=$uuid;?>"><span>If Settings</span></a></li>
				<li><a href="/snort/snort_rulesets.php?uuid=<?=$uuid;?>"><span>Categories</span></a></li>
				<li><a href="/snort/snort_rules.php?uuid=<?=$uuid;?>"><span>Rules</span></a></li>
				<li><a href="/snort/snort_define_servers.php?uuid=<?=$uuid;?>"><span>Servers</span></a></li>
				<li class="newtabmenu_active"><a href="/snort/snort_preprocessors.php?uuid=<?=$uuid;?>"><span>Preprocessors</span></a></li>
				<li><a href="/snort/snort_barnyard.php?uuid=<?=$uuid;?>"><span>Barnyard2</span></a></li>			
		</ul>
		</div>

		</td>
	</tr>
	<tr>
		<td id="tdbggrey">		
		<table width="100%" border="0" cellpadding="10px" cellspacing="0">
		<tr>
		<td class="tabnavtbl">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
		<!-- START MAIN AREA -->
		
		<form id="iform" >
		<input type="hidden" name="snortSaveSettings" value="1" /> <!-- what to do, save -->
		<input type="hidden" name="dbName" value="snortDBrules" /> <!-- what db-->
		<input type="hidden" name="dbTable" value="Snortrules" /> <!-- what db table-->
		<input type="hidden" name="ifaceTab" value="snort_preprocessors" /> <!-- what interface tab -->



			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%"><span class="vexpl">
					<span class="red"><strong>Note:</strong></span>
					<br>
					<span class="vexpl">Rules may be dependent on preprocessors!<br>
					Defaults will be used when there is no user input.</span><br>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Performance Statistics</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Enable</td>
				<td width="78%" class="vtable">
					<input name="perform_stat" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['perform_stat'] == 'on' || $a_list['perform_stat'] == '' ? 'checked' : '';?> > 
					<span class="vexpl">Performance Statistics for this interface.</span>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">HTTP Inspect Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Enable</td>
				<td width="78%" class="vtable">
					<input name="http_inspect" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['http_inspect'] == 'on' || $a_list['http_inspect'] == '' ? 'checked' : '';?> > 
					<span class="vexpl">Use HTTP Inspect to Normalize/Decode and detect HTTP traffic and protocol anomalies.</span>
				</td>
			</tr>
			<tr>
				<td valign="top" class="vncell2">HTTP server flow depth</td>
				<td class="vtable">
					<table cellpadding="0" cellspacing="0">
						<tr>
							<td>
								<input name="flow_depth" type="text" class="formfld" id="flow_depth" size="5" value="<?=$a_list['flow_depth']; ?>"> 
								<span class="vexpl"><strong>-1</strong> to <strong>1460</strong> (<strong>-1</strong> disables HTTP inspect, <strong>0</strong> enables all HTTP inspect)</span>
							</td>
						</tr>
					</table>
					<span class="vexpl">Amount of HTTP server response payload to inspect. Snort's performance may increase by adjusting this value.
					<br>
					Setting this value too low may cause false negatives. Values above 0 are specified in bytes. Default value is <strong>0</strong></span>
					<br>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">Stream5 Settings</td>
			</tr>
			<tr>
				<td valign="top" class="vncell2">Max Queued Bytes</td>
				<td class="vtable">
					<table cellpadding="0" cellspacing="0">
						<tr>
							<td>
								<input name="max_queued_bytes" type="text" class="formfld" id="max_queued_bytes" size="5" value="<?=$a_list['max_queued_bytes']; ?>">	
								<span class="vexpl">Minimum is <strong>1024</strong>, Maximum is <strong>1073741824</strong> ( default value is <strong>1048576</strong>, <strong>0</strong>means Maximum )</span>
							</td>
						</tr>
					</table>
					<span class="vexpl">The number of bytes to be queued for reassembly for TCP sessions in memory. Default value is <strong>1048576</strong></span>
					<br>
				</td>
			</tr>
			<tr>
				<td valign="top" class="vncell2">Max Queued Segs</td>
				<td class="vtable">
					<table cellpadding="0" cellspacing="0">
						<tr>
							<td>
								<input name="max_queued_segs" type="text" class="formfld" id="max_queued_segs" size="5" value="<?=$a_list['max_queued_segs']; ?>" >
								<span class="vexpl">Minimum is <strong>2</strong>, Maximum is <strong>1073741824</strong> ( default value is <strong>2621</strong>, <strong>0</strong> means Maximum )</span>
							</td>
						</tr>
					</table>
					<span class="vexpl">The number of segments to be queued for reassembly for TCP sessions in memory. Default value is <strong>2621</strong></span>
					<br>
				</td>
			</tr>
			<tr>
				<td colspan="2" valign="top" class="listtopic">General Preprocessor Settings</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
					Enable <br>
					RPC Decode and Back Orifice detector
				</td>
				<td width="78%" class="vtable">
					<input name="other_preprocs" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['other_preprocs'] == 'on' || $a_list['other_preprocs'] == '' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Normalize/Decode RPC traffic and detects Back Orifice traffic on the network.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
					Enable 
					<br>
					FTP and Telnet Normalizer
				</td>
				<td width="78%" class="vtable">
					<input name="ftp_preprocessor" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['ftp_preprocessor'] == 'on' || $a_list['ftp_preprocessor'] == '' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Normalize/Decode FTP and Telnet traffic and protocol anomalies.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
					Enable 
					<br>
					SMTP Normalizer
				</td>
				<td width="78%" class="vtable">
					<input name="smtp_preprocessor" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['smtp_preprocessor'] == 'on' || $a_list['smtp_preprocessor'] == '' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Normalize/Decode SMTP protocol for enforcement and buffer overflows.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
					Enable 
					<br>
					Portscan Detection
				</td>
				<td width="78%" class="vtable">
					<input name="sf_portscan" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['sf_portscan'] == 'on' || $a_list['sf_portscan'] == '' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">Detects various types of portscans and portsweeps.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
					Enable 
					<br>
					DCE/RPC2 Detection
				</td>
				<td width="78%" class="vtable">
					<input name="dce_rpc_2" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['dce_rpc_2'] == 'on' || $a_list['dce_rpc_2'] == '' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">The DCE/RPC preprocessor detects and decodes SMB and DCE/RPC traffic.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">
					Enable 
					<br>
					DNS Detection
				</td>
				<td width="78%" class="vtable">
					<input name="dns_preprocessor" type="checkbox" value="on" <?=$ifaceEnabled = $a_list['dns_preprocessor'] == 'on' || $a_list['dns_preprocessor'] == '' ? 'checked' : '';?> >
					<br>
					<span class="vexpl">The DNS preprocessor decodes DNS Response traffic and detects some vulnerabilities.</span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top" class="vncell2">Define SSL_IGNORE</td>
				<td width="78%" class="vtable">
					<input name="def_ssl_ports_ignore" type="text" class="formfld" id="def_ssl_ports_ignore" size="40" value="<?=$a_list['def_ssl_ports_ignore']; ?>" > 
					<br>
					<span class="vexpl">Encrypted traffic should be ignored by Snort for both performance reasons and to reduce false positives.
					<br>
					Default: "443 465 563 636 989 990 992 993 994 995". <strong>Please use spaces and not commas.</strong></span>
				</td>
			</tr>
			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save"> 
					<input id="cancel" type="button" class="formbtn" value="Cancel" > 				
					<input name="uuid" type="hidden" value="<?=$a_list['uuid']; ?>"> 
				</td>
			</tr>

			<tr>
				<td width="22%" valign="top">&nbsp;</td>
				<td width="78%"><span class="vexpl">
					<span class="vexpl"><span class="red"><strong>Note:</strong></span> Please save your settings before you click Start.</span>
				</td>
			</tr>
			
		
		</form>
		<!-- STOP MAIN AREA -->
		</table>
		</td>
		</tr>			
		</table>
	</td>
	</tr>
</table>
</div>


<!-- footer do not touch below -->
<?php 
include("fend.inc"); 
echo $snort_custom_rnd_box;
?>


</body>
</html>
