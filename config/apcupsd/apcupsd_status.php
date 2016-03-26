<?php
/*
	apcupsd_status.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2013-2015 Danilo G. Baio <dbaio@bsd.com.br>
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
require("guiconfig.inc");
require_once("apcupsd.inc");
global $pfs_version;
$pfs_version = substr(trim(file_get_contents("/etc/version")), 0, 3);

$pgtitle = "Services: Apcupsd (Status)";
include("head.inc");

function puts($arg) {
	echo "$arg\n";
}

if (isset($_GET['strapcaccess'])) {
	$strapcaccess = trim($_GET['strapcaccess']);
}

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<div id="mainlevel">
		<table width="100%" border="0" cellpadding="0" cellspacing="0">
			<tr><td>
		<?php
			$tab_array = array();
			$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=apcupsd.xml&amp;id=0");
			$tab_array[] = array(gettext("Status"), true, "/apcupsd_status.php");
			display_top_tabs($tab_array);
		?>
			</td></tr>
		</table>
</div>

<div id="mainarea" style="padding-top: 0px; padding-bottom: 0px; ">
	<form name="frm_apcupsd_status" method="get" action="">
		<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="6">
			<tr>
				<td width="14%" valign="top" class="vncellreq">Host:</td>
				<td width="86%" class="vtable">
					<input name="strapcaccess" type="text" class="formfld unknown" id="strapcaccess" size="22" value="<? echo "{$strapcaccess}"; ?>" />
					<br />
					<span class="vexpl">
     Default: <strong>localhost</strong><br /><br />
     Note: apcaccess uses apcupsd's inbuilt Network Information Server (NIS) to obtain the current status information<br />
     from the UPS on the local or remote computer. It is therefore necessary to have the following configuration directives: <br />
     NETSERVER on<br />
     NISPORT 3551<br />
					<br />
					<?php if ($pfs_version < 2.2): ?>
						<input type="submit" value="Execute" class="formbtn" disabled="disabled" />
					<?php else: ?>
						<input type="submit" value="Execute" class="formbtn" />
					<?php endif; ?>
					</span>
				</td>
		
			</tr>
			<tr><td colspan="2">
<?php
	$nis_server = check_nis_running_apcupsd();

	if ($pfs_version >= 2.2) {
		if ($strapcaccess) {
			echo "Running: apcaccess -h {$strapcaccess} <br/>";
			puts("<pre>");
			putenv("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
			$ph = popen("apcaccess -h {$strapcaccess} 2>&1", "r" );
			while ($line = fgets($ph)) {
				echo htmlspecialchars($line);
			}
			pclose($ph);
			puts("</pre>");
		} elseif ($nis_server) {
			$nisip = (check_nis_ip_apcupsd() != ''? check_nis_ip_apcupsd() : "0.0.0.0");
			$nisport = (check_nis_port_apcupsd() != '' ? check_nis_port_apcupsd() : "3551");
			echo "Running: apcaccess -h {$nisip}:{$nisport} <br/>";
			puts("<pre>");
			putenv("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
			$ph = popen("apcaccess -h localhost 2>&1", "r" );
			while ($line = fgets($ph)) {
				echo htmlspecialchars($line);
			}
			pclose($ph);
			puts("</pre>");
		} else {
			echo "Network Information Server (NIS) not running; in order to run apcaccess on localhost, you need to enable it on APCupsd General settings. <br />";
		}
	} else {
		echo "pfSense version prior to 2.2 runs APCupsd 3.14.10 and apcaccess doesn't accept host parameter. <br />";
		if ($nis_server) {
			puts("<pre>");
			putenv("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
			$ph = popen("apcaccess 2>&1", "r" );
			while ($line = fgets($ph)) {
				echo htmlspecialchars($line);
			}
			pclose($ph);
			puts("</pre>");
		} else {
			echo "Network Information Server (NIS) not running, in order to run apcaccess on localhost, you need to enable it on APCupsd General settings. <br />";
		}
	}

?>
		</td></tr>
	</table>
	</form>
</div>	
<?php 
include("fend.inc");
?>
</body>
</html>
