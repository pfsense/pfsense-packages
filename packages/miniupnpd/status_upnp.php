<?php
/* $Id$ */
/*
	status_upnp.php
	part of pfSense (http://www.pfsense.com/)

	Copyright (C) 2006 Seth Mos <seth.mos@xs4all.nl>.
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

$rdr_entries = array();
exec("/sbin/pfctl -aminiupnpd -sn", $rdr_entries, $pf_ret);

$now = time();
$year = date("Y");

$pgtitle = "Status: UPnP Status";
include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */
//$pfSenseHead->addMeta("<meta http-equiv=\"refresh\" content=\"120;url={$_SERVER['SCRIPT_NAME']}\" />");
//echo $pfSenseHead->getHTML();

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></font></p>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td>
	<div id="mainarea">
      <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
          <td width="10%" class="listhdrr"><?=gettext("Port")?></td>
          <td width="10%" class="listhdrr"><?=gettext("Protocol")?></td>
          <td width="20%" class="listhdrr"><?=gettext("Internal IP")?></td>
          <td width="60%" class="listhdr"><?=gettext("Description")?></td>
		</tr>
		<?php $i = 0; foreach ($rdr_entries as $rdr_entry) {
			if (preg_match("/rdr on (.*) inet proto (.*) from any to any port = (.*) -> (.*) port (.*)/", $rdr_entry, $matches))
			$rdr_proto = $matches[2];
			$rdr_port = $matches[3];
			$rdr_ip = $matches[4];
			$rdr_label =$matches[5];
		?>
        <tr>
          <td class="listlr">
		<?php print $rdr_proto;?>
          </td>
          <td class="listlr">
		<?php print $rdr_port;?>
          </td>
          <td class="listlr">
		<?php print $rdr_ip;?>
          </td>
          <td class="listlr">
		<?php print $rdr_label;?>
          </td>
        </tr>
        <?php $i++; }?>
      </table>
	</div>
	</table>

<?php include("fend.inc"); ?>
</body>
</html>
