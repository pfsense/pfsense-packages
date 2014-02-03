<?php
/*
	haproxy_stats.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2013 PiBa-NL
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
require_once("authgui.inc");
require_once("config.inc");
require_once("haproxy_socketinfo.inc");

$pconfig = $config['installedpackages']['haproxy'];
if (isset($_GET['haproxystats']) || isset($_GET['scope']) || (isset($_POST) && isset($_POST['action']))){
	$fail = false;
	try{
		$request = "";
		if (is_array($_GET)){
			foreach($_GET as $key => $arg)
				$request .= ";$key=$arg";
		}
		$options = array(
		  'http'=>array(
			'method'=>"POST",
			'header'=>"Accept-language: en\r\n".
			          "Content-type: application/x-www-form-urlencoded\r\n",
			'content'=>http_build_query($_POST)
		));
		$context = stream_context_create($options);
		$response = file_get_contents("http://127.0.0.1:{$pconfig['localstatsport']}/haproxy_stats.php?haproxystats=1".$request, false, $context);
		if (is_array($http_response_header)){
			foreach($http_response_header as $header){
				if (strpos($header,"Refresh: ") == 0)
					header($header);
			}
		}
		$fail = $response === false;
	} catch (Exception $e) {
		$fail = true;
	}
	if ($fail)
		$response = "<br/><br/>Make sure HAProxy settings are applied and HAProxy is enabled and running";
	echo $response;
	exit(0);
}
require_once("guiconfig.inc");
if (isset($_GET['showsticktablecontent'])){
	header("Refresh: 2");
}
$shortcut_section = "haproxy";
require_once("haproxy.inc");
require_once("certs.inc");
require_once("haproxy_utils.inc");
require_once("pkg_haproxy_tabs.inc");

if (!is_array($config['installedpackages']['haproxy']['ha_backends']['item'])) {
	$config['installedpackages']['haproxy']['ha_backends']['item'] = array();
}
$a_frontend = &$config['installedpackages']['haproxy']['ha_backends']['item'];

function haproxy_add_stats_example() {
	global $config, $d_haproxyconfdirty_path;
	$a_backends = &$config['installedpackages']['haproxy']['ha_pools']['item'];
	$a_frontends = &$config['installedpackages']['haproxy']['ha_backends']['item'];
	$webcert = haproxy_find_create_certificate("HAProxy stats default");
	
	$backend = array();
	$backend["name"] = "HAProxy_stats_ssl_backend";
	$backend["stats_enabled"] = "yes";
	$backend["stats_uri"] = "/";
	$backend["stats_refresh"] = "10";
	$a_backends[] = $backend;
	$changecount++;
	
	$frontend = array();
	$frontend["name"] = "HAProxy_stats_ssl_frontend";
	$frontend["status"] = "active";
	$frontend["type"] = "http";
	$frontend["port"] = "444";
	$frontend["extaddr"] = "lan_ipv4";
	$frontend["ssloffload"] = "yes";
	$frontend["ssloffloadcert"] = $webcert['refid'];
	$frontend["backend_serverpool"] = $backend["name"];
	$a_frontends[] = $frontend;
	$changecount++;
	$changedesc = "add new HAProxy stats example";
	
	if ($changecount > 0) {
		echo "touching: $d_haproxyconfdirty_path";
		touch($d_haproxyconfdirty_path);
		write_config($changedesc);
	}
}
	
if (isset($_GET['add_stats_example'])) {
	haproxy_add_stats_example();
	write_config();
	touch($d_haproxyconfdirty_path);
	header("Location: haproxy_listeners.php");
	exit;
}

if ($_POST) {
	if ($_POST['apply']) {
		$result = haproxy_check_and_run($savemsg, true);
		if ($result)
			unlink_if_exists($d_haproxyconfdirty_path);
	}
}

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));
if(strstr($pfSversion, "1.2"))
	$one_two = true;
	
$pgtitle = "Services: HAProxy: Stats";
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="haproxy_stats.php" method="post">
<?php if($one_two): ?>
<p class="pgtitle"><?=$pgtitle?></p>
<?php endif; ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_haproxyconfdirty_path)): ?>
<?php print_info_box_np("The haproxy configuration has been changed.<br/>You must apply the changes in order for them to take effect.");?><br/>
<?php endif; ?>
</form>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
	haproxy_display_top_tabs_active($haproxy_tab_array['haproxy'], "stats");
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
		<table class="tabcont" width="100%" height="100%" cellspacing="0">
		<tr>
		<?
if (isset($_GET['showsticktablecontent'])){
	$sticktablename = $_GET['showsticktablecontent'];
echo "<td colspan='2'>";
	echo "TESTJe<br/>";
	$res = haproxy_socket_command("show table $sticktablename");
	foreach($res as $line){
		echo "<br/>".print_r($line,true);
	}
echo "</td>";
} else {
?>
		<td colspan="2">
			This page contains a 'stats' page available from haproxy accessible through the pfSense gui.<br/>
			<br/>
			As the page is forwarded through the pfSense gui, this might cause some functionality to not work.<br/>
			Though the normal haproxy stats page can be tweaked more, and doesn't use a user/pass from pfSense itself.<br/>
			Some examples are configurable automatic page refresh,<br/>
			only showing certain servers, not providing admin options, and can be accessed from wherever the associated frontend is accessible.(as long as rules permit access)<br/>
			To use this or for simply an example how to use SSL-offloading configure stats on either a real backend while utilizing the 'stats uri'.
			Or create a backend specifically for serving stats, for that you can start with  the 'stats example' template below.<br/>
		</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">Stats example template</td>
		</tr>
		<tr>
			<td width="22%" valign="top" class="vncell">Example</td>
			<td class="vtable">
				As an basic example you can use the link below to create a 'stats' frontend/backend page which offers with more options like setting user/password and 'admin mode' when you go to the backend settings.<br/>
				<a href="haproxy_stats.php?add_stats_example=1">TEMPLATE: Create stats example configuration using a frontend/backend combination with ssl</a><br/>
				<br/>
				After applying the changes made by the template use this link to visit the stats page: <a target="_blank" href="https://<?=get_interface_ip("lan");?>:444">https://pfSense-LAN-ip:444/</a>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">HAProxy stick-tables</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="vncell">	
			These tables are used to store information for session persistence and can be used with ssl-session-id information, application-cookies, or other information that is used to persist a user to a server.
			<table class="tabcont sortable" id="sortabletable" width="100%" cellspacing="0" cellpadding="6" border="0">
			<head>
				<td class="listhdrr">Stick-table</td>
				<td class="listhdrr">Type</td>
				<td class="listhdrr">Size</td>
				<td class="listhdrr">Used</td>
			</head>
			<? $tables = haproxy_get_tables();
			foreach($tables as $key => $table) { ?>
			<tr>
				<td class="listlr"><a href="/haproxy_stats.php?showsticktablecontent=<?=$key;?>"><?=$key;?></td>
				<td class="listr"><?=$table['type'];?></td>
				<td class="listr"><?=$table['size'];?></td>
				<td class="listr"><?=$table['used'];?></td>
			</tr>
			<? } ?>
			</table>
			</td>
		</tr>
		<tr>
			<td>&nbsp;</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="listtopic">HAProxy stats</td>
		</tr>
		<tr>
			<td colspan="2" valign="top" class="vncell"><a href="/haproxy_stats.php?haproxystats=1" target="_blank">Fullscreen stats page</a></td>
		</tr>
		<tr>
		<td colspan="2"  class="listlr">
		<? if (isset($pconfig['enable']) && $pconfig['localstatsport'] && is_numeric($pconfig['localstatsport'])){?>
			<iframe id="frame_haproxy_stats" width="1000px" height="1500px" seamless=1 src="/haproxy_stats.php?haproxystats=1<?=$request;?>"></iframe>
		<? } else { ?>
			<br/>
			In the "Settings" configure a internal stats port and enable haproxy for this to be functional.<br/>
			<br/>
		<? } ?>
<?}?>		
		</td>
		</tr>
		</table>
	</div>
	</td>
	</tr>
</table>
<?php include("fend.inc"); ?>
</body>
</html>
