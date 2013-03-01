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

function bandwidthd_install_deinstall() {
	conf_mount_rw();
	config_lock();
	exec("rm -f /usr/local/etc/rc.d/bandwidthd*");
	exec("rm -rf " . PKG_BANDWIDTHD_BASE . "/htdocs");
	exec("rm -f /usr/local/www/bandwidthd");
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
	$meta_refresh = $config['installedpackages']['bandwidthd']['config'][0]['meta_refresh'];
	if($meta_refresh)
		$meta_refresh = "meta_refresh $meta_refresh\n";
	$graph = $config['installedpackages']['bandwidthd']['config'][0]['drawgraphs'];
	if($graph)
		$graph = "graph true\n";
	else
		$graph = "graph false\n";

	$filter_text = $config['installedpackages']['bandwidthd']['config'][0]['filter'];
	if($filter_text)
		$filter_text = "filter $filter_text\n";
	$recover_cdf = $config['installedpackages']['bandwidthd']['config'][0]['recovercdf'];
	if($recover_cdf)
		$recover_cdf = "recover_cdf true\n";
	$output_cdf = $config['installedpackages']['bandwidthd']['config'][0]['outputcdf'];
	if($output_cdf)
		$output_cdf = "output_cdf true\n";
	$promiscuous = $config['installedpackages']['bandwidthd']['config'][0]['promiscuous'];
	if($promiscuous)
		$promiscuous = "promiscuous true\n";
	else
		$promiscuous = "promiscuous false\n";

	$graph_cutoff = $config['installedpackages']['bandwidthd']['config'][0]['graphcutoff'];
	if($graph_cutoff)
		$graph_cutoff = "graph_cutoff $graph_cutoff\n";
	$skip_intervals = $config['installedpackages']['bandwidthd']['config'][0]['skipintervals'];
	if($skip_intervals)
		$skip_intervals = "skip_intervals $skip_intervals\n";

	if($config['installedpackages']['bandwidthd']['config'][0]['active_interface']){
		$ifdescrs = array($config['installedpackages']['bandwidthd']['config'][0]['active_interface']);
	} else {
		log_error("You should specify an interface for bandwidthd to listen on. Exiting.");
	}

	$subnets_custom = explode(';',str_replace(' ','',$config['installedpackages']['bandwidthd']['config'][0]['subnets_custom']));

	/* initialize to "" */
	$subnets = "";
	//$ifdescrs = array("lan", "wan");
	//for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
		//$ifdescrs['opt' . $j] = "opt" . $j;
	//}
	if(is_array($ifdescrs)) {
		foreach($ifdescrs as $int) {
			/* calculate interface subnet information */
			$ifcfg = $config['interfaces'][$int];
			$subnet = gen_subnet($ifcfg['ipaddr'], $ifcfg['subnet']);
			$subnetmask = gen_subnet_mask($ifcfg['subnet']);
			if($subnet == "pppoe") {
				$subnet = find_interface_ip("ng0");
				if($subnet)
					$subnets .= "subnet {$subnet}/32\n";
			} else {
				if($subnet)
					$subnets .= "subnet {$subnet}/{$ifcfg['subnet']}\n";
			}
		}
	}

	if(is_array($subnets_custom)) {
		foreach($subnets_custom as $sub) {
			if (!empty($sub) && is_subnet($sub))
				$subnets .= "subnet {$sub}\n";
		}
	}

	/* initialize to "" */
	$dev = "";
	if(is_array($ifdescrs)) {
		foreach($ifdescrs as $ifdescr) {
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

# Graph cutoff is how many k must be transfered by an
# ip before we bother to graph it
$graph_cutoff

#Put interface in promiscuous mode to score to traffic
#that may not be routing through the host machine.
$promiscuous

#Log data to cdf file htdocs/log.cdf
$output_cdf

#Read back the cdf file on startup
$recover_cdf

#Libpcap format filter string used to control what bandwidthd sees
#Please always include "ip" in the string to avoid strange problems
$filter_text

#Draw Graphs - This default to true to graph the traffic bandwidthd is recording
#Usually set this to false if you only want cdf output or
#you are using the database output option.  Bandwidthd will use very little
#ram and cpu if this is set to false.
$graph

#Set META REFRESH seconds (default 150, use 0 to disable).
$meta_refresh

EOF;

	$fd = fopen("{$bandwidthd_config_dir}/bandwidthd.conf","w");
	if(!$fd) {
		log_error("could not open {$bandwidthd_config_dir}/bandwidthd.conf for writing");
		exit;
	}
	fwrite($fd, $config_file);
	fclose($fd);

	$rc = array();
	$rc['file'] = 'bandwidthd.sh';
	$rc['stop'] = <<<EOD
/usr/bin/killall bandwidthd
EOD;

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
		$bandwidthd_nano_dir = "/var/bandwidthd";
		$bandwidthd_htdocs_dir = $bandwidthd_nano_dir . "/htdocs";
		$rc['start'] = <<<EOD
/bin/mkdir -p {$bandwidthd_nano_dir}
/bin/mkdir -p {$bandwidthd_htdocs_dir}
/bin/ln -s {$bandwidthd_base_dir}/bandwidthd {$bandwidthd_nano_dir}/bandwidthd
/bin/ln -s {$bandwidthd_config_dir} {$bandwidthd_nano_dir}/etc
cd {$bandwidthd_nano_dir}
{$bandwidthd_nano_dir}/bandwidthd
cd -
EOD;
	} else {
		$bandwidthd_htdocs_dir = $bandwidthd_base_dir . "/htdocs";
		$rc['start'] = <<<EOD
/usr/local/bandwidthd/bandwidthd
EOD;
	}

	/* write out rc.d start/stop file */
	write_rcfile($rc);

	exec("/bin/rm /usr/local/www/bandwidthd");
	exec("/bin/ln -s " . $bandwidthd_htdocs_dir . " /usr/local/www/bandwidthd");
	exec("echo \"Please start bandwidthd to populate this directory.\" > " . $bandwidthd_htdocs_dir . "/index.html");

	conf_mount_ro();
	config_unlock();

	stop_service("bandwidthd");
	start_service("bandwidthd");

}

?>
