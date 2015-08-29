<?php
/*
	antivirus_status.widget.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2010 Serg Dvoriancev <dv_serg@mail.ru>
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
require_once("pfsense-utils.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");

define('PATH_CLAMDB', '/var/db/clamav');
define('PATH_HAVPLOG', '/var/log/access.log');
define('PATH_AVSTATUS', '/var/tmp/havp.status');

if (file_exists("/usr/local/pkg/havp.inc")) {
	require_once("/usr/local/pkg/havp.inc");
} else {
	echo "No havp.inc found. You must have HAVP package installed to use this widget.";
}

function havp_avdb_info($filename) {
	$stl = "style='padding-top: 0px; padding-bottom: 0px; padding-left: 4px; padding-right: 4px; border-left: 1px solid #999999;'";
	$r = '';
	$path = PATH_CLAMDB . "/{$filename}";
	if (file_exists($path)) {
		$handle = '';
		if ($handle = fopen($path, "r")) {
			$s = fread($handle, 1024);
			$s = explode(':', $s);
			# datetime
			$dt = explode(" ", $s[1]);
			$s[1] = strftime("%Y.%m.%d", strtotime("{$dt[0]} {$dt[1]} {$dt[2]}"));
			if ($s[0] == 'ClamAV-VDB') {
				$r .= "<tr class='listr'><td>{$filename}</td><td $stl>{$s[1]}</td><td $stl>{$s[2]}</td><td $stl>{$s[7]}</td></tr>";
			}
			fclose($handle);
		}
		return $r;
	}
}

function dwg_avbases_info() {
	$db = '<table width="100%" border="0" cellspacing="0" cellpadding="1"><tbody>';
	$db .= '<tr class="vncellt" ><td>Database</td><td>Date</td><td>Ver.</td><td>Builder</td></tr>';
	$db .= havp_avdb_info("daily.cld");
	$db .= havp_avdb_info("daily.cvd");
	$db .= havp_avdb_info("bytecode.cld");
	$db .= havp_avdb_info("bytecode.cvd");
	$db .= havp_avdb_info("main.cld");
	$db .= havp_avdb_info("main.cvd");
	$db .= havp_avdb_info("safebrowsing.cld");
	$db .= havp_avdb_info("safebrowsing.cvd");
	$db .= '</tbody></table>';
	return $db;
}

function avupdate_status() {
	$s = "Not found.";
	if (HVDEF_UPD_STATUS_FILE && file_exists(HVDEF_UPD_STATUS_FILE)) {
		$s = file_get_contents(HVDEF_UPD_STATUS_FILE);
		return str_replace( "\n", "<br />", $s );
	}
}

function dwg_av_statistic() {
	$s = "Unknown.";
	if (file_exists(PATH_HAVPLOG)) {
		$log = file_get_contents(PATH_HAVPLOG);
		$count = substr_count(strtolower($log), "virus clamd:");
		$s = "Found $count viruses (total).";
	}
	return $s;
}

?>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody>
		<tr>
			<td class="vncellt">HTTP Scanner</td>
			<td class="listr" width="75%">
			<?php
				// HAVP version
				$pfs_version = substr(trim(file_get_contents("/etc/version")), 0, 3);
				if ($pfs_version == "2.1") {
					echo exec("pkg_info | grep \"[h]avp\"");
				} elseif ($pfs_version == "2.2") {
					// Show package version at least, no good quick way to get the PBI version
					echo "pkg v{$config['installedpackages']['package'][get_pkg_id("havp")]['version']}";
				} else {
					echo exec("/usr/sbin/pkg info havp | /usr/bin/head -n 1");
				}
			?>
			</td>
		</tr>
		<tr>
			<td class="vncellt">Antivirus Scanner</td>
			<td class="listr" width="75%">
			<?php
				// ClamD version
				echo exec("clamd -V");
			?>
			</td>
		</tr>
		<tr>
			<td class="vncellt">Antivirus Bases</td>
			<td class="listr" width="75%">
				<?php echo dwg_avbases_info(); ?>
			</td>
		</tr>
		<tr>
			<td class="vncellt">Last Update</td>
			<td class="listr" width="75%">
				<?php echo avupdate_status(); ?>
			</td>
		</tr>
		<tr>
			<td class="vncellt">Statistics</td>
			<td class="listr" width="75%">
				<?php echo dwg_av_statistic(); ?>
			</td>
		</tr>
	</tbody>
</table>
