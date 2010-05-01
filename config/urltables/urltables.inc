<?php

function urltables_install() {
	global $g, $config;
	
	// Test to make sure the patch is not already applied. 
	$out = `patch -fslC --reverse -p1 -b .before_urltables -d / -i /usr/local/pkg/urltables.patch |& grep -ci reject`;
	if ($out == 0) {
		// If the patch has not already been applied, test to see if it will apply cleanly.
		$out = `patch -fsNlC -p1 -b .before_urltables -d / -i /usr/local/pkg/urltables.patch |& grep -ci reject`;
		if ($out == 0) {
			// The patch should apply cleanly, let 'er rip.
			mwexec("patch -fsNl -p1 -b .before_urltables -d / -i /usr/local/pkg/urltables.patch ");
		}
	}
}

?>