<?php
		function sync_upnp() {
			global $config, $g;
			$lanif = $config['interfaces']['lan']['if'];
			$wanif = get_real_wan_interface();
			$lanip = find_interface_ip($lanif);
			/* copy original, we're going to overwrite ADDR with our internal ip */
			copy("/usr/local/etc/linuxigd/gatedesc.xml.bak", "/usr/local/etc/linuxigd/gatedesc.xml");
			/* replace ADDR with LAN ip */
			$file_contents = file_get_contents("/usr/local/etc/linuxigd/gatedesc.xml");
			$file_contents = str_replace("!ADDR!","{$lanip}",$file_contents);
			$fout = fopen("/usr/local/etc/linuxigd/gatedesc.xml", "w");
			fwrite($fout, $file_contents);
			fclose($fout);
			/* write out custom rc.d starter script */
			$fout = fopen("/usr/local/etc/rc.d/upnpd.sh", "w");
			fwrite($fout, "#!/bin/sh\n# This package was automatically generated\n# by the pfSense package system.\n\n");
			fwrite($fout, "# PACKAGE: UPNP\n");
			fwrite($fout, "# EXECUTABLE: upnpd\n\n");
			fwrite($fout, "/usr/local/bin/upnpd {$wanif} {$lanif}");
			fclose($fout);
			chmod("/usr/local/etc/rc.d/upnpd.sh", 0755);
			/* kill it off if its running */
			mwexec("killall upnpd");
			/* alright, thats all we need.  start up! */
			mwexec("/usr/local/etc/rc.d/upnpd.sh");
		}
		function upnp_install() {
			/* we need to update the files ADDR from time to time which is the lan ip */
			copy("/usr/local/etc/linuxigd/gatedesc.xml", "/usr/local/etc/linuxigd/gatedesc.xml.bak");
			unlink("/usr/local/etc/rc.d/linuxigd.sh.sample");		
		}
		function upnp_deinstall() {
			mwexec("rm -rf /usr/local/etc/rc.d/upnpd.sh");
			mwexec("rm -rf /usr/local/etc/linuxigd");		
		}
?>
	