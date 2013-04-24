<?php

function openvpn_tapfix_20x_install() {
	global $g, $config;
	$pfs_version = substr(trim(file_get_contents("/etc/version")),0,5);
	switch ($pfs_version) {
		case "2.0.3":
			$patch_file = "openvpn_tapfix_203.patch";
			break;
		default:
			$patch_file = "openvpn_tapfix_20x.patch";
	}
	// Test to make sure the patch is not already applied. 
	$out = `patch -fslC --reverse -p1 -b .before_openvpn_tapfix_20x -d / -i /usr/local/pkg/{$patch_file} |& grep -ci reject`;
	if ($out == 0) {
		// If the patch has not already been applied, test to see if it will apply cleanly.
		$out = `patch -fsNlC -p1 -b .before_openvpn_tapfix_20x -d / -i /usr/local/pkg/{$patch_file} |& grep -ci reject`;
		if ($out == 0) {
			// The patch should apply cleanly, let 'er rip.
			mwexec("patch -fsNl -p1 -b .before_openvpn_tapfix_20x -d / -i /usr/local/pkg/{$patch_file} ");
		}
	}
}

?>