<?php
function phpsysinfo_install_deinstall() {
	conf_mount_rw();
	exec("cd /var/db/pkg/ && pkg_delete `ls | grep mbmon`");
	exec("rm -d -R /usr/local/www/phpsysinfo");
	conf_mount_ro();
}

function phpsysinfo_install_config() {
global $config, $g;
	conf_mount_rw();
	exec("cd ..");
	exec("tar -zxovf /usr/local/pkg/phpsysinfo-2.5.4.tar.gz");
	exec("mv phpsysinfo /usr/local/www/phpsysinfo");

//  link shared libraries
//  if ((!file_exists("/lib/libm.so.4")) AND (file_exists("/lib/libm.so.5"))) {
//      exec("ln -s /lib/libm.so.5 /lib/libm.so.4");
//    }

 /* lines need in config.php */
$default_lng = en;
$default_template= pfSense;
$hide_picklist = "false";
$show_vhostname = "false";
$sensor_program = '""';
$show_mount_point = "false";
$show_inodes = "false";
$hide_mounts = "array()";
$hide_fstypes = "array()";
$loadbar = "false";
$showerrors = "false";
$temperatureformat = '"c-f"';
$hide_picklist = $config['installedpackages']['phpsysinfo']['config'][0]['hidepicklist'];
if($hide_picklist)
  $hide_picklist = "true";
  else
  $hide_picklist = "false";
  $sensor_program = $config['installedpackages']['phpsysinfo']['config'][0]['sensorprogram'];
if($sensor_program)
  $sensor_program= "mbmon";
  else
  $sensor_program = '""';
  $show_mount_point = $config['installedpackages']['phpsysinfo']['config'][0]['showmountpoint'];
if($show_mount_point)
  $show_mount_point = "true";
  else
  $show_mount_point = "false";
  $hide_fstypes = $config['installedpackages']['phpsysinfo']['config'][0]['hidefstypes'];
if($hide_fstypes)
  $hide_fstypes = "$hide_fstypes";
  $show_inodes = $config['installedpackages']['phpsysinfo']['config'][0]['showinodes'];
if($show_inodes)
  $show_inodes = "true";
  else
  $show_inodes = "false";
  $loadbar = $config['installedpackages']['phpsysinfo']['config'][0]['loadbar'];
if($loadbar)
  $loadbar = "true";
  else
  $loadbar = "false";
  $showerrors = $config['installedpackages']['phpsysinfo']['config'][0]['showerrors'];
if($showerrors)
  $showerrors = "true";
  else
  $showerrors = "false";
  $hf = fopen("/usr/local/www/phpsysinfo/config.php","w");
if(!$hf) {
  log_error("could not open /usr/local/www/phpsysinfo/config.php for writing");
exit;
}

fwrite($hf, '<?php');
fwrite($hf, "\n");
// webpath but not used
fwrite($hf, '$webpath = "";');
fwrite($hf, "\n");
// define the default lang and template here
fwrite($hf, '$default_lng=');
fwrite($hf, "'");
fwrite($hf, $default_lng);
fwrite($hf, "'");
fwrite($hf, ';');
fwrite($hf, "\n");
fwrite($hf, '$default_template=');
fwrite($hf, "'");
fwrite($hf, $default_template);
fwrite($hf, "'");
fwrite($hf, ';');
fwrite($hf, "\n");
// hide language and template picklist
// false = display picklist
// true = do not display picklist
fwrite($hf, '$hide_picklist = ');
fwrite($hf, $hide_picklist);
fwrite($hf, ';');
fwrite($hf, "\n");
// display the virtual host name and address
// default is canonical host name and address
fwrite($hf, '$show_vhostname = ');
fwrite($hf, $show_vhostname);
fwrite($hf, ';');
fwrite($hf, "\n");
// define the motherboard monitoring program here
fwrite($hf, '$sensor_program = ');
fwrite($hf, $sensor_program);
fwrite($hf, ';');
fwrite($hf, "\n");
// show mount point
fwrite($hf, '$show_mount_point = ');
fwrite($hf, $show_mount_point);
fwrite($hf, ';');
fwrite($hf, "\n");
// show bind
fwrite($hf, '$show_bind = false;');
fwrite($hf, "\n");
// show inode usage
fwrite($hf, '$show_inodes = ');
fwrite($hf, $show_inodes);
fwrite($hf, ';');
fwrite($hf, "\n");
// show inode usage
fwrite($hf, '$hide_mounts = ');
fwrite($hf, $hide_mounts);
fwrite($hf, ';');
fwrite($hf, "\n");
// Hide filesystem typess. Example: 'tmpfs', 'usbfs' 
fwrite($hf, '$hide_fstypes = array(');
fwrite($hf, $hide_fstypes);
fwrite($hf, ');');
fwrite($hf, "\n");
// show a graph for current cpuload
fwrite($hf, '$loadbar = ');
fwrite($hf, $loadbar);
fwrite($hf, ';');
fwrite($hf, "\n");
// e.g. $addpaths = array('/opt/bin', '/opt/sbin');
fwrite($hf, '$addpaths = array();');
fwrite($hf, "\n");
// display error messages at the top of the page
fwrite($hf, '$showerrors = ');
fwrite($hf, $showerrors);
fwrite($hf, ';');
fwrite($hf, "\n");
// format in which temperature is displayed
fwrite($hf, '$temperatureformat = ');
fwrite($hf, $temperatureformat);
fwrite($hf, ';');
fwrite($hf, "\n");
// The end of the config file
fwrite($hf, '?>');
fwrite($hf, "\n");
fclose($hf);
	conf_mount_ro();
}
?>
