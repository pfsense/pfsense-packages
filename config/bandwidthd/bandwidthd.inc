<?php
/* $Id$ */
/*
	bandwidthd.inc
	Copyright (C) 2006 Scott Ullrich
	part of pfSense
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

// Check pfSense version
$pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
switch ($pfs_version) {
  case "1.2":
  case "2.0":
    define('PKG_BANDWIDTHD_BASE', '/usr/local/bandwidthd');
    break;
  default:
    define('PKG_BANDWIDTHD_BASE', '/usr/pbi/bandwidthd-' . php_uname("m") . '/bandwidthd');
 }
// End: Check pfSense version

function is_blank($value) {
    return empty($value) && !is_numeric($value);
}

function bandwidthd_install_deinstall() {
	conf_mount_rw();
	config_lock();
	exec("rm -f /usr/local/etc/rc.d/bandwidthd*");
	exec("rm -rf " . PKG_BANDWIDTHD_BASE . "/htdocs");
	exec("rm -f /usr/local/www/bandwidthd");
	// Remove the cron job, if it is there
	install_cron_job("/bin/kill -HUP `cat /var/run/bandwidthd.pid`", false);
	conf_mount_ro();
	config_unlock();
}

function bandwidthd_install_config() {
	global $config, $g;

	/* bandwidthd doesn't have a way to pass a custom config path, unfortunately */
	/* the conf file must be ./etc/bandwidthd.conf relative to the current dir */
	$bandwidthd_base_dir = PKG_BANDWIDTHD_BASE;
	$bandwidthd_config_dir = PKG_BANDWIDTHD_BASE . "/etc";

	conf_mount_rw();
	config_lock();

	/* user defined values */
	$bandwidthd_config = $config['installedpackages']['bandwidthd']['config'][0];
	$meta_refresh = $bandwidthd_config['meta_refresh'];
	if (is_numeric($meta_refresh))
		$meta_refresh = "meta_refresh $meta_refresh\n";
	else
		$meta_refresh = "";

	$graph = $bandwidthd_config['drawgraphs'];
	if ($graph)
		$graph = "graph true\n";
	else
		$graph = "graph false\n";

	$filter_text = $bandwidthd_config['filter'];
	if (!is_blank($filter_text))
		$filter_text = "filter $filter_text\n";
	else
		$filter_text = "";

	$recover_cdf = $bandwidthd_config['recovercdf'];
	if ($recover_cdf)
		$recover_cdf = "recover_cdf true\n";
	else
		$recover_cdf = "";

	$output_cdf = $bandwidthd_config['outputcdf'];
	if ($output_cdf)
		$output_cdf_string = "output_cdf true\n";
	else
		$output_cdf_string = "";

	$output_postgresql = $bandwidthd_config['outputpostgresql'];
	$postgresql_host = $bandwidthd_config['postgresqlhost'];
	$postgresql_database = $bandwidthd_config['postgresqldatabase'];
	$postgresql_username = $bandwidthd_config['postgresqlusername'];
	$postgresql_password = $bandwidthd_config['postgresqlpassword'];
	$postgresql_string = "";
	if ($output_postgresql) {
		if (!is_blank($postgresql_host) && !is_blank($postgresql_username) && !is_blank($postgresql_database) && !is_blank($postgresql_password))
			$postgresql_string = "pgsql_connect_string \"user = $postgresql_username dbname = $postgresql_database password = $postgresql_password host = $postgresql_host\"\n";
		else
			log_error("bandwidthd: You have to specify the postgreSQL Host, Database, Username and Password. postgreSQL details have been ignored.");
	}

	$sensor_id = $bandwidthd_config['sensorid'];

	if (!is_blank($sensor_id))
		$sensor_id_string = "sensor_id \"$sensor_id\"";
	else
		$sensor_id_string = "";

	$promiscuous = $bandwidthd_config['promiscuous'];
	if ($promiscuous)
		$promiscuous = "promiscuous true\n";
	else
		$promiscuous = "promiscuous false\n";

	$graph_cutoff = $bandwidthd_config['graphcutoff'];
	if (!is_blank($graph_cutoff))
		$graph_cutoff = "graph_cutoff $graph_cutoff\n";
	else
		$graph_cutoff = "";

	$skip_intervals = $bandwidthd_config['skipintervals'];
	if ($skip_intervals) {
		$skip_intervals = "skip_intervals $skip_intervals\n";
	} else {
		/* Includes the case where 0 is explicitly specified, which is the default anyway. */
		$skip_intervals = "";
	}

	if (!is_blank($bandwidthd_config['active_interface'])){
		$ifdescrs = array($bandwidthd_config['active_interface']);
	} else {
		log_error("You should specify an interface for bandwidthd to listen on. Exiting.");
	}

	$subnets_custom = explode(';',str_replace(' ','',$bandwidthd_config['subnets_custom']));

	/* initialize to "" */
	$subnets = "";
	//$ifdescrs = array("lan", "wan");
	//for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
		//$ifdescrs['opt' . $j] = "opt" . $j;
	//}
	if (is_array($ifdescrs)) {
		foreach ($ifdescrs as $int) {
			/* calculate interface subnet information */
			$ifcfg = $config['interfaces'][$int];
			$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
			$subnetmask = gen_subnet_mask($ifcfg['subnet']);
			$subnet_with_mask = "";
			if ($subnet == "pppoe") {
				$subnet = find_interface_ip("ng0");
				if ($subnet) {
					$subnet_with_mask = $subnet . "/32";
				}
			} else {
				if ($subnet) {
					$subnet_with_mask = $subnet . "/" . $ifcfg['subnet'];
				}
			}
			if (!empty($subnet_with_mask)) {
				/* Only add the subnet if the user has not specified it in the custom subnets. */
				/* This avoids generating an unnecessary syntax error message from the config. */
				if (!in_array($subnet_with_mask, $subnets_custom))
					$subnets .= "subnet {$subnet_with_mask}\n";
			}
		}
	}

	if (is_array($subnets_custom)) {
		foreach ($subnets_custom as $sub) {
			if (!empty($sub) && is_subnet($sub))
				$subnets .= "subnet {$sub}\n";
		}
	}

	/* initialize to "" */
	$dev = "";
	if (is_array($ifdescrs)) {
		foreach ($ifdescrs as $ifdescr) {
			$descr = convert_friendly_interface_to_real_interface_name($ifdescr);
			$dev .= "dev \"$descr\"\n";
		}
	}

	$config_file = <<<EOF
#
# This file was automatically generated by the pfSense
# package management system.  Changing this file
# will lead to it being overwritten again when
# the package manager resyncs.
#
####################################################
# Bandwidthd.conf
#
# Commented out options are here to provide
# documentation and represent defaults

# Subnets to collect statistics on.  Traffic that
# matches none of these subnets will be ignored.
# Syntax is either IP Subnet Mask or CIDR
$subnets

# Device to listen on
# Bandwidthd listens on the first device it detects
# by default.  Run "bandwidthd -l" for a list of
# devices.
$dev

###################################################
# Options that don't usually get changed

# An interval is 2.5 minutes, this is how many
# intervals to skip before doing a graphing run
$skip_intervals

# Graph cutoff is how many k must be transferred by an
# ip before we bother to graph it
$graph_cutoff

#Put interface in promiscuous mode to score to traffic
#that may not be routing through the host machine.
$promiscuous

#Log data to cdf file htdocs/log.cdf
$output_cdf_string

#Read back the cdf file on startup
$recover_cdf

# Standard postgres connect string, just like php, see postgres docs for
# details
$postgresql_string

# Arbitrary sensor name, I recommend the sensors fully qualified domain
# name
$sensor_id_string

#Libpcap format filter string used to control what bandwidthd sees
#Please always include "ip" in the string to avoid strange problems
$filter_text

#Draw Graphs - This defaults to true to graph the traffic bandwidthd is recording
#Usually set this to false if you only want cdf output or
#you are using the database output option.  Bandwidthd will use very little
#ram and cpu if this is set to false.
$graph

#Set META REFRESH seconds (default 150, use 0 to disable).
$meta_refresh

EOF;

	$fd = fopen("{$bandwidthd_config_dir}/bandwidthd.conf","w");
	if (!$fd) {
		log_error("could not open {$bandwidthd_config_dir}/bandwidthd.conf for writing");
		exit;
	}
	fwrite($fd, $config_file);
	fclose($fd);

	if ($g['platform'] == 'nanobsd') {
		$bandwidthd_nano_dir = "/var/bandwidthd";
		$bandwidthd_htdocs_dir = $bandwidthd_nano_dir . "/htdocs";
		if (!is_dir($bandwidthd_nano_dir)) {
			if (file_exists($bandwidthd_nano_dir)) {
				unlink($bandwidthd_nano_dir);
			}
			mkdir($bandwidthd_nano_dir);
		}
	} else {
		$bandwidthd_htdocs_dir = $bandwidthd_base_dir . "/htdocs";
	}

	$rc = array();
	$rc['file'] = 'bandwidthd.sh';
	$rc['stop'] = <<<EOD
/usr/bin/killall bandwidthd
EOD;

	// If this is an old config before the enable checkbox was added, then enable by default
	$bandwidthd_enable = (!isset($bandwidthd_config['enable']) || ($bandwidthd_config['enable']));
	if ($bandwidthd_enable) {
		if ($g['platform'] == 'nanobsd') {
			// On nanobsd, /var/bandwidthd is created.
			// In that is a real /var/bandwidth/htdocs, where the graph data is written
			// A soft link to the real bandwidth program is made - /var/bandwidthd/bandwidthd
			// A soft link to the etc folder with the conf file is made - /var/bandwidthd/etc
			// bandwidthd is started from /var/bandwidthd with the current dir /var/bandwidth
			// This way, it:
			//   looks in ./etc for the conf file
			//   writes graph files in ./htdocs
			//   writes cdf log files (if selected in the config) to ./
			// All of this is on the /var filesystem, which is a read-write memory disk on nanobsd
			$rc['start'] = <<<EOD
if [ ! -d "{$bandwidthd_nano_dir}" ] ; then
	if [ -e "{$bandwidthd_nano_dir}" ] ; then
		/bin/rm -f {$bandwidthd_nano_dir}
	fi
	/bin/mkdir -p {$bandwidthd_nano_dir}
fi
if [ ! -d "{$bandwidthd_htdocs_dir}" ] ; then
	if [ -e "{$bandwidthd_htdocs_dir}" ] ; then
		/bin/rm -f {$bandwidthd_htdocs_dir}
	fi
	/bin/mkdir -p {$bandwidthd_htdocs_dir}
fi
if [ ! -L "{$bandwidthd_nano_dir}/bandwidthd" ] ; then
	if [ -e "{$bandwidthd_nano_dir}/bandwidthd" ] ; then
		/bin/rm -Rf {$bandwidthd_nano_dir}/bandwidthd
	fi
	/bin/ln -s {$bandwidthd_base_dir}/bandwidthd {$bandwidthd_nano_dir}/bandwidthd
fi
if [ ! -L "{$bandwidthd_nano_dir}/etc" ] ; then
	if [ -e "{$bandwidthd_nano_dir}/etc" ] ; then
		/bin/rm -Rf {$bandwidthd_nano_dir}/etc
	fi
	/bin/ln -s {$bandwidthd_config_dir} {$bandwidthd_nano_dir}/etc
fi
if [ ! -f "{$bandwidthd_htdocs_dir}/legend.gif" ] ; then
	/bin/cp {$bandwidthd_base_dir}/htdocs/legend.gif {$bandwidthd_htdocs_dir}
fi
if [ ! -f "{$bandwidthd_htdocs_dir}/logo.gif" ] ; then
	/bin/cp {$bandwidthd_base_dir}/htdocs/logo.gif {$bandwidthd_htdocs_dir}
fi
cd {$bandwidthd_nano_dir}
{$bandwidthd_nano_dir}/bandwidthd
cd -
EOD;
		} else {
			$rc['start'] = <<<EOD
/usr/local/bandwidthd/bandwidthd
EOD;
		}
	} else {
		// bandwidthd is disabled, so do not put any real start commands in the script.
		// This effectively disables it but keeps all the files in place (e.g. saved logs) ready to reload when it is enabled.
		$rc['start'] = "return";
	}

	/* write out rc.d start/stop file */
	write_rcfile($rc);

	if (!is_dir($bandwidthd_htdocs_dir)) {
		if (file_exists($bandwidthd_htdocs_dir)) {
			unlink($bandwidthd_htdocs_dir);
		}
		mkdir($bandwidthd_htdocs_dir);
	}
	$bandwidthd_www_link = $g["www_path"] . "/bandwidthd";
	if (!is_link($bandwidthd_www_link)) {
		if (file_exists($bandwidthd_www_link)) {
			// It is a file and not a link - clean it up.
			unlink($bandwidthd_www_link);
		}
		symlink($bandwidthd_htdocs_dir, $bandwidthd_www_link);
	}

	$bandwidthd_index_file = $bandwidthd_htdocs_dir . "/index.html";
	if (!file_exists($bandwidthd_index_file)) {
		exec("echo \"Please start bandwidthd to populate this directory.\" > " . $bandwidthd_index_file);
	}

	if (($bandwidthd_enable) && ($output_cdf)) {
		// Use cron job to rotate logs every day at 00:01
		install_cron_job("/bin/kill -HUP `cat /var/run/bandwidthd.pid`", true, "1", "0");
	}
	else
	{
		// Remove the cron job, if it is there
		install_cron_job("/bin/kill -HUP `cat /var/run/bandwidthd.pid`", false);
	}
	conf_mount_ro();
	config_unlock();
	stop_service("bandwidthd");
	if ($bandwidthd_enable) {
		start_service("bandwidthd");
	}
}

?>
