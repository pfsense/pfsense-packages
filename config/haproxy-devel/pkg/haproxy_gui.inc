<?php

$haproxy_icons = array(
	'moveup' => array(
		"faicon" => "fa-arrow-up",
		"icon" => "icon_up.gif",
		"iconsize" => 17),
	'movedown' => array(
		"faicon" => "fa-arrow-down",
		"icon" => "icon_down.gif",
		"iconsize" => 17),
	'add' => array(
		"faicon" => "fa-level-down",
		"icon" => "icon_plus.gif",
		"iconsize" => 17),
	'delete' => array(
		"faicon" => "fa-trash",
		"icon" => "icon_x.gif",
		"iconsize" => 17),
	'edit' => array(
		"faicon" => "fa-pencil",
		"icon" => "icon_e.gif",
		"iconsize" => 17),
	'clone' => array(
		"faicon" => "fa-clone",
		"icon" => "icon_plus.gif"),
	'acl' => array(
		"faicon" => "fa-random",
		"icon" => "icon_ts_rule.gif",
		"iconsize" => 11),
	'cert' => array(
		"faicon" => "fa-lock",
		"icon" => "icon_frmfld_cert.png",
		"iconsize" => 11),
	'advanced' => array(
		"faicon" => "fa-cog",
		"icon" => "icon_advanced.gif",
		"iconsize" => 11),
	'enabled' => array(
		"faicon" => "fa-check",
		"icon" => "icon_pass.gif",
		"iconsize" => 11),
	'disabled' => array(
		"faicon" => "fa-ban",
		"icon" => "icon_reject.gif",
		"iconsize" => 11),
	'stats' => array(
		"faicon" => "fa-tasks",
		"icon" => "icon_log_s.gif",
		"iconsize" => 11)
);

global $pf_version;
$pf_version = substr(trim(file_get_contents("/etc/version")), 0, 3);

function pf_version() {
	global $pf_version;
	return $pf_version;
}

function haproxyicon($iconname, $title) {
	global $g, $haproxy_icons;
	$title = htmlspecialchars($title);
	$title = str_replace("'", "&#39;", $title);
	
	$faicon = $haproxy_icons[$iconname]["faicon"];
	if (pf_version() < "2.3") {
		$icon = $haproxy_icons[$iconname]["icon"];
		$iconsize = $haproxy_icons[$iconname]["iconsize"];
		return "<img src='/themes/{$g['theme']}/images/icons/$icon' title='$title' width='$iconsize' height='$iconsize' border='0' />";	
	} else {
		return "<i class='fa $faicon' title='$title'></i>";
	}

}
