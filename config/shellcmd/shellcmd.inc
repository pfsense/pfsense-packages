<?php
/*
	shellcmd.inc
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2008 Mark J Crane
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
function shellcmd_install_command() {
	global $config;
	conf_mount_rw();

	/* Remove garbage left over by previous broken package versions */
	unlink_if_exists("/usr/local/etc/rc.d/shellcmd.sh");
	if (is_dir("/usr/local/www/packages/shellcmd")) {
		mwexec("/bin/rm -rf /usr/local/www/packages/shellcmd/");
	}

	shellcmd_import_config();
	shellcmd_sync_package();
	conf_mount_ro();

}


function shellcmd_delete_php_command() {
	global $config;

	/* When 'Delete item' is clicked in Shellcmd Settings */
	if ($_GET['act'] == "del") {

		/* System earlyshellcmd commands */
		$a_earlyshellcmd = &$config['system']['earlyshellcmd'];
		if (!is_array($a_earlyshellcmd)) {
			$a_earlyshellcmd = array();
		}
		/* Shellcmd package commands */
		$a_shellcmd_config = &$config['installedpackages']['shellcmdsettings']['config'];
		if (!is_array($a_shellcmd_config)) {
			$a_shellcmd_config = array();
		}

		/* First check for a couple of special cases that we do NOT want deleted */
		/* TODO: Create a function for these checks */
		$pkg = '';
		/* pfBlockerNG - function to restore archived aliastables on nanobsd (see pfblockerng.inc) */
		$pfbcmd = "/usr/local/pkg/pfblockerng/pfblockerng.sh";
		/* If the entry exists in system config ... */
		if (in_array($pfbcmd, $a_earlyshellcmd)) {
			$cnta = 0;
			/* ... but does not exist in package config ... */
			foreach ($a_shellcmd_config as $item => $value) {
				if (in_array($pfbcmd, $value)) {
					$cnta++;
				}
			}
			/* ... the user has deleted this protected entry. */
			if ($cnta === 0) {
				$pkg .= "[pfBlockerNG]";
				/* Force reimport. */
				shellcmd_forced_restore($pkg);
			}
		}
		/* System Patches auto-apply patch feature (see patches.inc) */
		$spcmd = "/usr/local/bin/php -f /usr/local/bin/apply_patches.php";
		if (in_array($spcmd, $a_earlyshellcmd)) {
			$cntb = 0;
			foreach ($a_shellcmd_config as $item => $value) {
				if (in_array($spcmd, $value)) {
					$cntb++;
				}
			}
			if ($cntb === 0) {
				$pkg .= "[System Patches]";
				shellcmd_forced_restore($pkg);
			}
		}

		/* Otherwise, sync package and system configuration normally */
		shellcmd_sync_package();
	}
}

/* Force restore of protected (early)shellcmds from system config */
function shellcmd_forced_restore($pkg) {
	log_error("[shellcmd] Refused to delete {$pkg} earlyshellcmd. Use {$pkg} to configure this entry.");
	shellcmd_import_config();
	write_config("[shellcmd] Restore of {$pkg} earlyshellcmd forced.");
	/* Send the user back to settings */
	header("Location: pkg.php?xml=shellcmd.xml");
	exit;

}


function shellcmd_sync_package() {
	global $config;
	conf_mount_rw();

	$cmd = '';
	$cmdtype = '';
	$a_shellcmd = array();
	$a_earlyshellcmd = array();
	$a_shellcmd_config = &$config['installedpackages']['shellcmdsettings']['config'];
	if (!is_array($a_shellcmd_config)) {
		$a_shellcmd_config = array();
	}
	$i = 0;
	/* When shellcmd/earlyshellcmd is added to shellcmd package configuration, make sure */
	/* we add corresponding shellcmd/earlyshellcmd entry to $config['system'] as well */
	foreach ($a_shellcmd_config as $item) {
		/* Get the command from package configuration here */
		$cmd = $a_shellcmd_config[$i]['cmd'];
		/* Lets see what type of command we are adding first... */
		$cmdtype = $a_shellcmd_config[$i]['cmdtype'];
		/* shellcmd */
		if ($cmdtype == "shellcmd") {
			$a_shellcmd[] = $cmd;
			$i++;
		/* earlyshellcmd */
		} elseif ($cmdtype == "earlyshellcmd") {
			$a_earlyshellcmd[] = $cmd;
			$i++;
		/* Should never happen; someone messing with config.xml manually?! */
		} else {
			return;
			$i++;
		}
	}

	/* Write the new system configuration to config.xml from scratch when done */
	unset($config['system']['shellcmd']);
	$config['system']['shellcmd'] = $a_shellcmd;
	unset($config['system']['earlyshellcmd']);
	$config['system']['earlyshellcmd'] = $a_earlyshellcmd;
	write_config("[shellcmd] Successfully (re)synced shellcmd configuration.");

}


function shellcmd_import_config() {
	global $config;

	/* Import earlyshellcmd entries which were either created by previous package versions, */
	/* or manually, or added by some other package(s) (if there are any in config.xml) */
	/* Two currently known special cases are handled here - System Patches and pfBlockerNG */
	$shellcmd_config = &$config['installedpackages']['shellcmdsettings']['config'];
	if (!is_array($shellcmd_config)) {
		$shellcmd_config = array();
	}

	$i = 0;
	if (is_array($config['system']['earlyshellcmd'])) {
		$earlyshellcmds = &$config['system']['earlyshellcmd'];
		$pfbcmd = "/usr/local/pkg/pfblockerng/pfblockerng.sh";
		$spcmd = "/usr/local/bin/php -f /usr/local/bin/apply_patches.php";
		foreach ($earlyshellcmds as $earlyshellcmd) {
			/* pfBlockerNG - function to restore archived aliastables on nanobsd (see pfblockerng.inc) */
			if (stristr($earlyshellcmd, "{$pfbcmd}")) {
				$shellcmd_config[$i]['cmd'] = $earlyshellcmd;
				$shellcmd_config[$i]['cmdtype'] = "earlyshellcmd";
				$shellcmd_config[$i]['description'] = "pfBlockerNG default earlyshellcmd. DO NOT EDIT/DELETE!";
				$i++;
			/* System Patches auto-apply patch feature (see patches.inc) */
			} elseif (stristr($earlyshellcmd, "{$spcmd}")) {
				$shellcmd_config[$i]['cmd'] = $earlyshellcmd;
				$shellcmd_config[$i]['cmdtype'] = "earlyshellcmd";
				$shellcmd_config[$i]['description'] = "System Patches default earlyshellcmd. DO NOT EDIT/DELETE!";
				$i++;
			/* Other manually added earlyshellcmd entries */
			} else {
				$shellcmd_config[$i]['cmd'] = $earlyshellcmd;
				$shellcmd_config[$i]['cmdtype'] = "earlyshellcmd";
				$shellcmd_config[$i]['description'] = "Imported earlyshellcmd ({$i})";
				$i++;
			}

		}
	}
	/* Import shellcmd entries which were created manually (if there are any in config.xml) */
	if (is_array($config['system']['shellcmd'])) {
		$shellcmds = &$config['system']['shellcmd'];
		foreach ($shellcmds as $shellcmd) {
			$shellcmd_config[$i]['cmd'] = $shellcmd;
			$shellcmd_config[$i]['cmdtype'] = "shellcmd";
			$shellcmd_config[$i]['description'] = "Imported shellcmd ({$i})";
			$i++;
		}
	}

	/* Write the new config.xml when import is finished */
	write_config("[shellcmd] Successfully imported package configuration from config.xml.");

}

function shellcmd_validate_input($post, &$input_errors) {
	global $config;
	$a_shellcmd = &$config['system']['shellcmd'];
	if (!is_array($a_shellcmd)) {
		$a_shellcmd = array();
	}
	$a_earlyshellcmd = &$config['system']['earlyshellcmd'];
	if (!is_array($a_earlyshellcmd)) {
		$a_earlyshellcmd = array();
	}

	/* Make sure we don't add the same command twice as it's just pointless */
	if (($post['cmd']) != '') {
		if ($post['cmdtype'] == "shellcmd") {
			if (in_array($post['cmd'], $a_shellcmd)) {
				$input_errors[] = "{$post['cmd']} already exists as shellcmd.";
			}
		}
		if ($post['cmdtype'] == "earlyshellcmd") {
			if (in_array($post['cmd'], $a_earlyshellcmd)) {
				$input_errors[] = "{$post['cmd']} already exists as earlyshellcmd.";
			}
		}
	}

}

?>
