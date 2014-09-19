<?php
/*
 * snort_service_utils.php
 *
 * Copyright (C) 2006 Scott Ullrich
 * Copyright (C) 2009-2010 Robert Zelaya
 * Copyright (C) 2011-2012 Ermal Luci
 * Copyright (C) 2013,2014 Bill Meeks
 * part of pfSense
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
 * This file is called by the custom service start, stop and status handlers *
 * created for each enabled Snort and Barnyard interface.  The calling code  *
 * is expected to set the following varibles to indicate the action desired. *
 *                                                                           *
 *    $action = start, stop, restart or status                               *
 *   $service = executable to act upon (snort or barnyard2)                  *
 *      $uuid = Unique Identifier ID for the rule interface                  *
 *                                                                           *
 *****************************************************************************/

require_once("/usr/local/pkg/snort/snort.inc");

global $g, $config;

if (empty($uuid)) {
	log_error(gettext("[Snort] error in snort_service_utils.php ... no UUID provided."));
	return FALSE;
}
if (strtolower($service) != "snort" && strtolower($service) != "barnyard2") {
	log_error(gettext("[Snort] error in snort_service_utils.php ... unrecognized service '{$service}' provided."));
	return FALSE;
}

$service = strtolower($service);
$action = strtolower($action);

// First find the correct [rule] index in our config using the UUID
if (!is_array($config['installedpackages']['snortglobal']['rule']))
	return FALSE;
foreach ($config['installedpackages']['snortglobal']['rule'] as $rule) {
	if ($rule['uuid'] == $uuid) {
		$if_real = get_real_interface($rule['interface']);

		// Block changes when package is being started from shell script
		if (file_exists("{$g['varrun_path']}/snort_pkg_starting.lck") {
			log_error(gettext("[Snort] interface service start/stop commands locked-out during package start/restart."));
			return TRUE;
		}

		// If interface is manually stopped, then don't try to start it
		if (($action == 'start' || $action == 'restart') && file_exists("{$g['varrun_path']}/{$service}_{$uuid}.disabled")) {
			log_error(gettext("[Snort] auto-start locked out by previous manual shutdown...must be started using Snort INTERFACES tab."));
			return FALSE;
		}

		switch ($action) {
			case 'start':
				if ($service == "snort")
					snort_start($rule, $if_real, TRUE);
				elseif ($service == "barnyard2")
					snort_barnyard_start($rule, $if_real, TRUE);
				else
					return FALSE;
				return TRUE;

			case 'stop':
				if ($service == "snort")
					snort_stop($rule, $if_real);
				elseif ($service == "barnyard2")
					snort_barnyard_stop($rule, $if_real);
				else
					return FALSE;
				return TRUE;

			case 'restart':
				if ($service == "snort") {
					snort_stop($rule, $if_real);
					sleep(1);
					snort_start($rule, $if_real, TRUE);
				}
				elseif ($service == "barnyard2") {
					snort_barnyard_stop($rule, $if_real);
					sleep(1);
					snort_barnyard_start($rule, $if_real, TRUE);
				}
				else
					return FALSE;
				return TRUE;

			case 'status':
				if (isvalidpid("{$g['varrun_path']}/{$service}_{$if_real}{$uuid}.pid"))
					return TRUE;
				else
					return FALSE;

			default:
				log_error(gettext("[Snort] error in snort_service_utils.php ... unrecognized action '{$action}' provided."));
				return FALSE;
		}
	}
}

?>
