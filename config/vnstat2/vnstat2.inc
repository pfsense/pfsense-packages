<?php
function vnstat_install_deinstall() {
	conf_mount_rw();
	global $config;
// Remove Vnstat package and files 
	exec("rm -d -R /usr/local/www/vnstat2");
	exec("rm -d -R /usr/local/www/vnstati");
	exec("rm -d -R /usr/local/pkg/vnstat2");
	exec("rm /usr/local/etc/vnstat.conf");

// Remove vnstat cron entry from config.xml
	vnstat2_install_cron(false);
	conf_mount_ro();
}

function vnstat2_install_cron($vnstat_cron_value) {
	global $config;
	$is_installed = false;
	if(!$config['cron']['item'])
		return;
	$x=0;
	foreach($config['cron']['item'] as $item) {
		if(strstr($item['command'], "/usr/local/pkg/vnstat2/vnstat2.sh")) {
			$is_installed = true;
			break;
		}
		$x++;
	}
	switch($vnstat_cron_value) {
		case true:
			if(!$is_installed) {
				$cron_item = array();
				$cron_item['minute']    = "*/1";
	        		$cron_item['hour']      = "*";
	        		$cron_item['mday']      = "*";
	        		$cron_item['month']     = "*";
	        		$cron_item['wday']      = "*";
	        		$cron_item['who']       = "root";
	        		$cron_item['command']   = "/usr/local/pkg/vnstat2/vnstat2.sh";
	        		$config['cron']['item'][] = $cron_item;		
				write_config();
				configure_cron();
			}
		break;
		case false:
			if($is_installed == true) {
				if($x > 0) {
					unset($config['cron']['item'][$x]);
					write_config();
				}
				configure_cron();
			}
		break;
	}
}


function change_vnstat_conf(){
	conf_mount_rw();
	global $config;
	$config['installedpackages']['vnstat2']['config'][0]['monthrotate'] = $_POST['monthrotate'];
	$config['installedpackages']['vnstat2']['config'][0]['vnstat_phpfrontend'] = $_POST['vnstat_phpfrontend']; 
	write_conf_f();
	write_config();
	$no_vnstat_phpfrontend = $config['installedpackages']['vnstat2']['config'][0]['vnstat_phpfrontend'];
        if ($no_vnstat_phpfrontend == "on"){
 		vnstat_php_frontend();
		}
	else {
	exec("[ -d /usr/local/www/vnstat2 ] && rm -d -R /usr/local/www/vnstat2");
	}
	conf_mount_ro();
}

function write_conf_f(){
	global $config;
	$monthrotate = $config['installedpackages']['vnstat2']['config'][0]['monthrotate'];

// ************ Write new vnstat.conf *****************

	$vnstat_conf_file = <<<EOF
# vnStat 1.10 config file
##

# location of the database directory
DatabaseDir "/conf/vnstat"

# on which day should months change
MonthRotate $monthrotate

# vnstati
##

# image colors
CBackground     "F0F0F0"
CEdge           "AEAEAE"
CHeader         "990000"
CHeaderTitle    "F0F0F0"
CHeaderDate     "FFFFFF"
CText           "000000"
CLine           "B0B0B0"
CLineL          "-"
CRx             "666666"
CTx             "990000"
CRxD            "-"
CTxD            "-"
EOF;

	$hf2 = fopen("/usr/local/etc/vnstat.conf","w");
	if(!$hf2) {
	log_error("could not open /usr/local/etc/vnstat.conf for writing");
	exit;
	}
	fwrite($hf2, $vnstat_conf_file);
	fclose($hf2);
}

function create_vnstati_image() {
	conf_mount_rw();
	global $config;
	$iface = $_POST['vnstat_interface'];
	$ifaces_final = convert_friendly_interface_to_real_interface_name($iface);
	$config['installedpackages']['vnstat2']['config'][0]['vnstat_interface'] = $ifaces_final;
	exec("/usr/local/bin/vnstati -i ". $ifaces_final ." -vs -o /tmp/newpicture1.png");
	exec("/usr/local/bin/vnstati -i ". $ifaces_final ." -m -o /tmp/newpicture2.png");
	exec("/usr/local/bin/vnstati -i ". $ifaces_final ." -d -o /tmp/newpicture3.png");
	exec("/usr/local/bin/vnstati -i ". $ifaces_final ." -t -o /tmp/newpicture4.png");
	write_config();
	conf_mount_ro();
}

function create_vnstat_output() {
	conf_mount_rw();
	global $config;
	$iface2 = $_POST['vnstat_interface2']; 
	$ifaces_final2 = convert_friendly_interface_to_real_interface_name($iface2);
	$config['installedpackages']['vnstat2']['config'][0]['vnstat_interface2'] = $ifaces_final2;
	write_config();
	conf_mount_ro();
}

function vnstat_install_config() {
        global $config;
	conf_mount_rw();
// Create vnstat database dir where it also will work for nanobsd
//	exec("[ -d /var/db/vnstat ] && mv /var/db/vnstat /conf/vnstat");
	exec("[ -d /usr/local/pkg/vnstat2/vnstat ] && mv /usr/local/pkg/vnstat2/vnstat /conf/vnstat");
	exec("[ ! -d /conf/vnstat ] && mkdir /conf/vnstat");
// Check for pbi install and arch type then create symlinks
	if (file_exists('/usr/pbi/vnstat-i386')) { exec("ln -s /usr/local/etc/vnstat.conf /usr/pbi/vnstat-i386/etc/vnstat.conf"); }
	if (file_exists('/usr/pbi/vnstat-amd64')) { exec("ln -s /usr/local/etc/vnstat.conf /usr/pbi/vnstat-amd64/etc/vnstat.conf"); }
// Add MonthRotate value to config.xml and write /usr/local/etc/vnstat.conf
	$no_monthrotate = $config['installedpackages']['vnstat2']['config'][0]['monthrotate'];	
	if ($no_monthrotate == ""){
		$config['installedpackages']['vnstat2']['config'][0]['monthrotate'] = "1";
	}
	$no_vnstat_phpfrontend = $config['installedpackages']['vnstat2']['config'][0]['vnstat_phpfrontend'];
        if ($no_vnstat_phpfrontend == "on"){
 		vnstat_php_frontend();
	}
	write_conf_f();
// Add cron job to config.xml
	vnstat2_install_cron(true);
// get system nic's and create a database for them
        $array_of_real_nic_names = array();
        $array_of_custom_nic_names = array();
        $ifdescrs = array('wan' => 'WAN', 'lan' => 'LAN');
		for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
			$ifdescrs['opt' . $j] = $config['interfaces']['opt' . $j]['descr'];
		}
         		foreach ($ifdescrs as $ifdescr => $ifname):
			$real_nic_names = convert_friendly_interface_to_real_interface_name($ifdescr);
			array_push($array_of_real_nic_names, $real_nic_names);
		endforeach;
        // Get already nic's assigned to vnstat
        exec("ls /conf/vnstat/ | grep -v '\.'", $vnstat_nic);
        // Compare real nic's with nic's assigned to vnstat
        $result = array_diff($array_of_real_nic_names, $vnstat_nic);
        // Create database for each nic 
        foreach ($result as $result2)
        {
           exec("/usr/local/bin/vnstat -u -i ". $result2);
        }
     	write_config();
     	conf_mount_ro();
}

function vnstat_php_frontend(){
	global $config;
// Copy vnstat_php_frontend to www 
	exec("cp -a /usr/local/pkg/vnstat2/vnstat_php_frontend/. /usr/local/www/vnstat2/");
// Find information to be writing in config.php 
	// $iface_list_array_items
        exec("ls /conf/vnstat/ | grep -v '\.'", $vnstat_nic_in);
        $iface_list_array_items = implode("', '", $vnstat_nic_in);
        $iface_list_array = "\$iface_list = array('$iface_list_array_items');";
        // $iface_title_array_items
        $iface_title_array_items = array();
        $iface_title_array_items2 = array();
        foreach ($vnstat_nic_in as $vnstat_nic_out)
        {
        $ifdescrs = array('wan' => 'WAN', 'lan' => 'LAN');
		for ($j = 1; isset($config['interfaces']['opt' . $j]); $j++) {
			$ifdescrs['opt' . $j] = $config['interfaces']['opt' . $j]['descr'];
		}
         	foreach ($ifdescrs as $ifdescr => $ifname):
         		$real_nic_names3 = get_real_interface($ifdescr);
         		If ($real_nic_names3 == $vnstat_nic_out)
               {
               $ifname_out = convert_friendly_interface_to_friendly_descr($ifdescr);
               $iface_title_array_items = "\$iface_title['$vnstat_nic_out'] = '$ifname_out';\n";
	       array_push($iface_title_array_items2, $iface_title_array_items);
               }
		endforeach;
	}
        $iface_title_array = implode($iface_title_array_items2);
	// php in php static items
	// added to new items for the front end version 1.5.1
		$locale = "\$locale = 'en_US.UTF-8';";
		$language = "\$language = 'en';";
        $vnstat_bin2 = "\$vnstat_bin = '/usr/local/bin/vnstat';";
        $data_dir2 = "\$data_dir = './dumps';";
        $graph_format2 ="\$graph_format='svg';";
        $colorscheme2 = "\$colorscheme['light'] = array(";
        $colorscheme3 = "\$colorscheme['red'] = array(";
        $colorscheme4 = "\$colorscheme['pfSense'] = array(";
// ************ Write new config.php ******************
        $config_file = <<<EOF
<?php
    //
    // vnStat PHP frontend 1.5.1 (c)2006-2008 Bjorge Dijkstra (bjd@jooz.net)
    //
    // This program is free software; you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation; either version 2 of the License, or
    // (at your option) any later version.
    //
    // This program is distributed in the hope that it will be useful,
    // but WITHOUT ANY WARRANTY; without even the implied warranty of
    // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    // GNU General Public License for more details.
    //
    // You should have received a copy of the GNU General Public License
    // along with this program; if not, write to the Free Software
    // Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    //
    //
    // see file COPYING or at http://www.gnu.org/licenses/gpl.html 
    // for more information.
    //
    error_reporting(E_ALL | E_NOTICE);
    
    //
    // configuration parameters
    //
    // edit these to reflect your particular situation
    //
$locale
$language
    // list of network interfaces monitored by vnStat
$iface_list_array

    //
    // optional names for interfaces
    // if there's no name set for an interface then the interface identifier
    // will be displayed instead
$iface_title_array

    //
    // There are two possible sources for vnstat data. If the 
    // variable is set then vnstat is called directly from the PHP script
    // to get the interface data. 
    //
    // The other option is to periodically dump the vnstat interface data to
    // a file (e.g. by a cronjob). In that case the  variable
    // must be cleared and set  to the location where the dumps
    // are stored. Dumps must be named 'vnstat_dump_'.
    //
    // You can generate vnstat dumps with the command:
    //   vnstat --dumpdb -i  > /path/to/data_dir/vnstat_dump_
    // 
$vnstat_bin2
$data_dir2

    // graphics format to use: svg or png
$graph_format2
    
    // Font to use for PNG graphs
    define('GRAPH_FONT',dirname(__FILE__).'/VeraBd.ttf');

    // Font to use for SVG graphs
    define('SVG_FONT', 'Verdana');

    // color schemes
    // colors are defined as R,G,B,ALPHA quads where R, G and B range from 0-255
    // and ALPHA from 0-127 where 0 is opaque and 127 completely transparent.
    //
    define('DEFAULT_COLORSCHEME', 'pfSense');
?>
EOF;

     $hf = fopen("/usr/local/www/vnstat2/config.php","w");
     if(!$hf) {
     log_error("could not open /usr/local/www/vnstat2/config.php for writing");
     exit;
     }
     fwrite($hf, $config_file);
     fclose($hf);
}
?>
