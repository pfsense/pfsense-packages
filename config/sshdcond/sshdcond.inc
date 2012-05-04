<?php

/**
 * Author: Han Van
 * Email: namezero@afim.info
 *
 * Developed for: pfSense.com
 *
 * Copyright (c) 2009, pfSense.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in thedocumentation and/or other materials provided with the distribution.
 * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 require_once("config.inc");
 require_once("util.inc");
  
function sshdcond_custom_php_install_command()
{
	global $g, $config;
	
	conf_mount_rw();
	
	// We need to generate an outfile for our extra commands
	// The patched g_szSSHDFileGenerate php file then reads and appends that config
	$fd = fopen("/etc/sshd_extra", 'w');
	fclose($fd);	
	
	conf_mount_ro();
}

function sshdcond_custom_php_deinstall_command()
{
	global $g, $config;
	
	conf_mount_rw();
	
	// 1. Delete our config file
	if(file_exists("/etc/sshd_extra"))
	{
		unlink("/etc/sshd_extra");
	}
	
	// 2. Re-run sshd config generation script
	restart_sshd();
	
	conf_mount_ro();
}

function sshdcond_custom_php_write_config()
{
	global $g, $config;
	
	conf_mount_rw();	
	
	// 1. Generate szOutFile
	$fd = fopen("/etc/sshd_extra", 'w');
	$szOut = "\n\n";
	for($i = 0; $i < count($config['installedpackages']['sshdcond']['config'][0]); $i++)
	{
		$szOut .= "Match " . $config['installedpackages']['sshdcond']['config'][0]['row'][$i]['matchtype'] .  " " . $config['installedpackages']['sshdcond']['config'][0]['row'][$i]['matchvalue'] . "\n";
		$aCommands = explode(';', $config['installedpackages']['sshdcond']['config'][0]['row'][$i]['restrictions']);
		for($j = 0; $j < count($aCommands); $j++)
		{
			$szCmd = trim($aCommands[$j]);
			if(strlen($szCmd) > 0)
			{
				$szOut .= "\t" . $szCmd . "\n";
			}
			
			// If the command is ChrootDirectory, then root:operator must be owner of the home directory
			{
				$szSplitCmd = explode(' ', $aCommands[$j]);
				if(count($szSplitCmd) > 1)
				{
					if(0 == strcasecmp(trim($szSplitCmd[0]), "ChrootDirectory"))
					{
						$szSetChown = "/usr/sbin/chown root:operator " . $szSplitCmd[1];
						mwexec_bg($szSetChown);
					}
				}
				
			}
		}
		
		$szOut .= "\n";
	}
	
	fwrite($fd, $szOut);
	fclose($fd);	
	
	// 2. Regenerate SSHD config and restart sshd
	restart_sshd();
	
	conf_mount_ro();
	

	

}

function restart_sshd()
{
	mwexec_bg("/etc/./sshd");
}




?>
