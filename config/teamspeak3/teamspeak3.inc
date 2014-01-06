<?php
/* $Id$ */
/* ========================================================================== */
/*
	teamspeak3.inc
	Copyright (C) 2013 Sander Peterse
	All rights reserved.
                                                                              */
/* ========================================================================== */
/*
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1.	Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	2.	Redistributions in binary form must reproduce the above copyright
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
/* ========================================================================== */

require_once('util.inc');
require_once('filter.inc');
require_once('interfaces.inc');
require_once('pfsense-utils.inc');
require_once('service-utils.inc');
require_once('teamspeak3_constants.inc');

function teamspeak3_custom_php_install_command()
{
	conf_mount_rw();
	
	// Create teamspeak user and group (binary file owner and service run-as user).
	exec("/usr/sbin/pw groupadd ".TeamSpeak3Constants::BinaryGroup);
	exec("/usr/sbin/pw useradd ".TeamSpeak3Constants::BinaryUser." -c \"TeamSpeak 3 server\" -g ".TeamSpeak3Constants::BinaryGroup." -d /nonexistent -s /usr/sbin/nologin");
	
	// Create binary directory and put copy default server ini file.
	safe_mkdir(TeamSpeak3Constants::BinaryDirectory);	
	if(!file_exists(teamspeak3_get_serverini_path()))
	{
		copy(TeamSpeak3Constants::PkgDirectory."/ts3server.ini.default", teamspeak3_get_serverini_path());
	}	
	teamspeak3_chown_file_or_directory(TeamSpeak3Constants::BinaryDirectory, true);
		
	mwexec("mv ".TeamSpeak3Constants::PkgDirectory."/service.sh.tmp /usr/local/etc/rc.d/".TeamSpeak3Constants::ServiceName.".sh");
		
	conf_mount_ro();
}

function teamspeak3_custom_php_resync_command()
{
	global $config;
	
	teamspeak3_write_server_config();
	filter_configure();
}

function teamspeak3_custom_php_deinstall_command()
{
	global $config;
	
	conf_mount_rw();
	
	stop_service(TeamSpeak3Constants::ServiceName);
	
	mwexec("rm -Rf ".TeamSpeak3Constants::BinaryDirectory);
	mwexec("rm -f ".RCFILEPREFIX.TeamSpeak3Constants::ServiceName.'.sh');
	
	exec("/usr/sbin/pw userdel ".TeamSpeak3Constants::BinaryUser);
	exec("/usr/sbin/pw groupdel ".TeamSpeak3Constants::BinaryGroup);
	unset($config['installedpackages']['teamspeak3']);
	
	write_config();
	conf_mount_ro();	
	
	// Make sure we remove our auto-created firewall rules.
	update_status("Reconfiguring filter...");
	filter_configure();
}

function teamspeak3_generate_rules($sType)
{
	global $config;
			
	if(!($config['installedpackages']['teamspeak3']['config'][0]['createrule'] == 'on'))
		return "";
	
	$aInterfaces = explode(',', $config['installedpackages']['teamspeak3']['config'][0]['serverinterfaces']);
	$aInterfaces = array_map('convert_friendly_interface_to_real_interface_name', $aInterfaces);
		
	$nVoicePort = (int)teamspeak3_get_config_value('default_voice_port');
	if(empty($nVoicePort)) $nVoicePort = 9987;
	
	$nFilePort = (int)teamspeak3_get_config_value('filetransfer_port');
	if(empty($nFilePort)) $nFilePort = 30033;	
	
	$sRules = "# TeamSpeak 3 package\n";
	switch($sType) 
	{
		case 'nat':
			/*foreach ($aInterfaces as $sInterface)
			{
			}*/
			break;
		case 'filter':
		case 'rule':
			foreach ($aInterfaces as $sInterface)
			{
				if($sInterface != "lo0")
				{
					$sRules .= "pass quick on {$sInterface} proto udp to port {$nVoicePort} keep state label \"TEAMSPEAK3: Voice\"\n";
					$sRules .= "pass quick on {$sInterface} proto tcp to port {$nFilePort} flags S/SA keep state label \"TEAMSPEAK3: Data\"\n";						
				}
			}
			break;
		default:
			break;
	}
	
	return $sRules;
}

function teamspeak3_write_server_config()
{
	global $config;
	
	conf_mount_rw();

	// Update server ini config when param has been set.
	if(!isset($config['installedpackages']['teamspeak3']['config']))
	{
		// Apply default package settings
		$config['installedpackages']['teamspeak3']['config'] = array();
		$config['installedpackages']['teamspeak3']['config'][] = 
			array(
				'ts3serverconfig' => base64_encode(file_get_contents(teamspeak3_get_serverini_path()))
			);
	}
	
	// Apply interface setting to configuration file.
	$sConfigFileData = base64_decode($config['installedpackages']['teamspeak3']['config'][0]['ts3serverconfig']);
	
	// Get array of interface IPs on where the teamspeak service should listen.
	$aSelectedInterfaceIPs = array();	
	if(!empty($config['installedpackages']['teamspeak3']['config'][0]['serverinterfaces']))
	{
		$aSelectedInterfaces = explode(',', $config['installedpackages']['teamspeak3']['config'][0]['serverinterfaces']);
		if(count($aSelectedInterfaces) > 0)
		{	
			foreach($aSelectedInterfaces as $sInterface)
			{
				$sInterface = convert_friendly_interface_to_real_interface_name($sInterface);
				$aSelectedInterfaceIPs[] = find_interface_ip($sInterface);
			}
		}	
	}
	
	// Create string of the selected interfaces or use LOOPBACK when no interfaces are selected.
	if(count($aSelectedInterfaceIPs) > 0) 
	{ 
		//$sSelectedInterfaceValue = implode(', ', $aSelectedInterfaceIPs);
		// Teamspeak currently supports only 1 interface per virtual server, so get the first element.
		$sSelectedInterfaceValue = $aSelectedInterfaceIPs[0];
	}
	else
	{
		$config['installedpackages']['teamspeak3']['config'][0]['serverinterfaces'] = "lo0";
		$sInterface = convert_friendly_interface_to_real_interface_name("lo0");
		$sSelectedInterfaceValue = find_interface_ip($sInterface);
	}
	
	// Update the configure file by replacing the values for each IP param.
	$aParamsToReplaceWithInterfaceValue = array('voice_ip', 'filetransfer_ip', 'query_ip');
		
	$sParsedConfigFileData = "";
	$aConfigLines = explode(PHP_EOL, $sConfigFileData);	
	foreach($aConfigLines as $sConfigLine)
	{
		if(!empty($sParsedConfigFileData)) $sParsedConfigFileData .= PHP_EOL;
		
		$aConfigKeyValuePair = explode('=', $sConfigLine, 2);		
		if(count($aConfigKeyValuePair) == 2
				&& in_array($aConfigKeyValuePair[0], $aParamsToReplaceWithInterfaceValue))
		{
			$sConfigLine = $aConfigKeyValuePair[0]."=".$sSelectedInterfaceValue;
		}
		$sParsedConfigFileData .= $sConfigLine;			
	}
	
	// Stop teamspeak service, update config (to disk) and restart it again.
	$bServiceRunning = is_service_running(TeamSpeak3Constants::ServiceName);
	if($bServiceRunning) stop_service(TeamSpeak3Constants::ServiceName);
		
	// Write server config file to disk & fix owner.
	$sServerIniFilePath = teamspeak3_get_serverini_path();
	file_put_contents(
		$sServerIniFilePath, 
		$sParsedConfigFileData);
	teamspeak3_chown_file_or_directory($sServerIniFilePath, false);	
	
	if($bServiceRunning) start_service(TeamSpeak3Constants::ServiceName);
	
	$config['installedpackages']['teamspeak3']['config'][0]['ts3serverconfig'] = base64_encode($sParsedConfigFileData);
	write_config();
	
	conf_mount_ro();
}

function teamspeak3_get_config_value($sProperty)
{
	global $config;
	
	$sConfigFileData = base64_decode($config['installedpackages']['teamspeak3']['config'][0]['ts3serverconfig']);
	
	if(!empty($sConfigFileData))
	{
		$aConfigLines = explode(PHP_EOL, $sConfigFileData);
		foreach($aConfigLines as $sConfigLine)
		{
			$aConfigKeyValuePair = explode('=', $sConfigLine, 2);
			if(count($aConfigKeyValuePair) == 2 && $aConfigKeyValuePair[0] == $sProperty)
			{
				return $aConfigKeyValuePair[1];
			}
		}
	}
	
	return null;
}

function teamspeak3_get_serverini_path()
{
	$sServerIniFilePath = TeamSpeak3Constants::BinaryDirectory."/";
	$sServerIniFilePath .= TeamSpeak3Constants::ServerConfigFile;
	return $sServerIniFilePath;
}

function teamspeak3_install_from_file($aFile, &$sErrorMessage)
{	
	$bStatus = false;
	$sArchiveTmpFile = $aFile['tmp_name'];
	if(!file_exists($sArchiveTmpFile)) 
	{
		$sErrorMessage = "Can't find (temporary) uploaded (archive) file.";
		return false;
	}

	$sExpectedResult = "teamspeak3-server_freebsd-".teamspeak3_current_architecture()."-3.xx.xx.xx.tar.gz";
	$sPregSearch = str_replace(array("-", ".", "xx"), array("\-", "\.", "[0-9]{1,3}"), $sExpectedResult);
	
	if(!preg_match("/".$sPregSearch."/", $aFile['name']))
	{
		$sErrorMessage = "The uploaded file doesn't match your system's operating system or architecture. The filename should match the expected format: '".$sExpectedResult."'.";
		return false;
	}	
	
	$sTmpExtractPath = "/tmp/".TeamSpeak3Constants::ServiceName.time();
	mkdir($sTmpExtractPath);
	chdir($sTmpExtractPath);	
	mwexec("tar -zxof ".$sArchiveTmpFile);
	
	$sBinaryDirectory = null;
	$sBinaryFile = null;
	if(teamspeak3_find_binary_recursive($sTmpExtractPath, &$sBinaryDirectory, &$sBinaryFile))
	{		
		$bServiceRunning = is_service_running(TeamSpeak3Constants::ServiceName);
		if($bServiceRunning) stop_service(TeamSpeak3Constants::ServiceName);
		
		mwexec("mv ".$sBinaryDirectory."/* ".TeamSpeak3Constants::BinaryDirectory);
		mwexec("mv ".TeamSpeak3Constants::BinaryDirectory."/".$sBinaryFile." ".TeamSpeak3Constants::BinaryDirectory."/".TeamSpeak3Constants::ServiceName);
		mwexec("rm -f ".TeamSpeak3Constants::BinaryDirectory."/ts3server_minimal_runscript.sh");
		mwexec("rm -f ".TeamSpeak3Constants::BinaryDirectory."/ts3server_startscript.sh");		
		teamspeak3_chown_file_or_directory(TeamSpeak3Constants::BinaryDirectory, true);	
			
		if($bServiceRunning) start_service(TeamSpeak3Constants::ServiceName);		
		$bStatus = true;
	}
	else
	{
		$sErrorMessage = "Can't find the required Teamspeak 3 server files inside the uploaded (archive) file.";
	}
		
	// Cleanup our temporary files.
	teamspeak3_rrmdir($sTmpExtractPath);
	
	return $bStatus;
}

function teamspeak3_find_binary_recursive($sSearchDirectory, &$sBinaryDirectory, &$sBinaryFile)
{
	$aFiles = glob("{$sSearchDirectory}/*");
	
	foreach($aFiles as $sFile)
	{
		$sFilename = basename($sFile);
		if(in_array($sFilename, TeamSpeak3Constants::$SupportedBinaryFiles))
		{
			$sBinaryDirectory = $sSearchDirectory;
			$sBinaryFile = $sFilename;
			return true;			
		}
		else if(is_dir($sFile))
		{
			if(teamspeak3_find_binary_recursive($sFile, &$sBinaryDirectory, &$sBinaryFile))
			{
				return true;
			}
		}
	}
	
	return false;
}

function teamspeak3_chown_file_or_directory($sPath, $bRecursive)
{
	if(!file_exists($sPath)) return;
	
	$sChownCmd = "chown ";
	if($bRecursive) $sChownCmd .= "-R ";
	$sChownCmd .= TeamSpeak3Constants::BinaryUser.":";
	$sChownCmd .= TeamSpeak3Constants::BinaryGroup." ";
	$sChownCmd .= $sPath;
	mwexec($sChownCmd);
}

function teamspeak3_rrmdir($sPath)
{
	return is_file($sPath) 
	? @unlink($sPath) 
	: array_map('teamspeak3_rrmdir', glob($sPath.'/*')) == @rmdir($sPath);
}

function teamspeak3_current_architecture()
{
	$sArchitecture = exec("uname -m");
	if($sArchitecture == "i386") return "x86";
	if($sArchitecture == "amd64") return "amd64";
	return null;
}

function teamspeak3_get_all_log_files()
{
	$sLogDirectory = teamspeak3_get_full_logs_path();
	
	$aLogFiles = glob($sLogDirectory."*.log");
	usort($aLogFiles, create_function('$a,$b', 'return filemtime($b) - filemtime($a);')); // Sort files array (DESC, new to old).
	
	$aLogFileames = array();
	foreach($aLogFiles as $aLogFile)
	{
		$aLogFileames[] = basename($aLogFile);
	}
	return $aLogFileames;
}

function teamspeak3_download_file($sFilePath, $sFilename, $b404OnError=true)
{
	// Send a 404 error to the browser when we can't find the file.
	if(!file_exists($sFilePath))
	{
		if($b404OnError)
		{
			header("HTTP/1.0 404 Not Found");
			echo "404 Not Found";
		}
		return;
	}
	
	// Make the file available for the browser (as a file download).
	$sContentType = (substr(strrchr($sFilePath,'.'),1) == 'sqlitedb') ? "application/x-sqlite3" : "text/plain";
	header("Content-Type: {$sContentType}");
	header("Content-Disposition: attachment; filename=\"{$sFilename}\"");
	echo file_get_contents($sFilePath);
}

function teamspeak3_download_log_file($sLogFile, $b404OnError=true)
{
	// Make sure it's a safe filename.
	// We don't want anyone downloading unexpected files by adding directories to the filename!!! E.g. changing it to /etc/passwd.
	if(strstr($sLogFile, '/') !== false 
			|| strstr($sLogFile, '\\') !== false
			|| substr($sLogFile, -4) != '.log')
	{
		if($b404OnError)
		{
			header("HTTP/1.0 404 Not Found");
			echo "404 Not Found";
		}		
		return;
	}
	else
	{
		$sLogPath = teamspeak3_get_full_logs_path().'/'.$sLogFile;
	}
	
	teamspeak3_download_file($sLogPath, $sLogFile, $b404OnError);
}

function teamspeak3_get_full_logs_path()
{
	$sLogsDirectory = teamspeak3_get_config_value('logs');	
			
	if(empty($sLogsDirectory))
	{
		// Apply default directory.
		$sLogsDirectory = "logs/";
	}
	
	if(substr($sLogsDirectory, 0, 1) == '/')
	{
		// Is already full path, so return it.
		return $sLogsDirectory;
	}
	
	// Relative path, so prefix with binary directory.
	return TeamSpeak3Constants::BinaryDirectory.'/'.$sLogsDirectory;
}

function teamspeak3_download_backup($sBackupType, &$sMessage)
{
	if(!in_array($sBackupType, TeamSpeak3Constants::$BackupTypes)) 
	{
		$sMessage = "Can't download backup, unsupported backup file.";
		return false;
	}
	
	// The ts3server.sqlitedb file requires the TeamSpeak 3 service to stop (and start again afterwards).
	// This because we have to make sure that all data has been flushed to the sqlite database before downloading it.
	$bShouldRestartService = ($sBackupType == "ts3server.sqlited");
	
	if($sBackupType == "ts3files.tar.gz")
	{		
		// Create an archive to allow downloading of the files.
		$sFilePath = teamspeak3_archive_file_browser();
	}
	else
	{
		$sFilePath = TeamSpeak3Constants::BinaryDirectory.'/'.$sBackupType;
	}
		
	if($bShouldRestartService)
	{
		$bServiceRunning = is_service_running(TeamSpeak3Constants::ServiceName);
		if($bServiceRunning) stop_service(TeamSpeak3Constants::ServiceName);
	}
	
	teamspeak3_download_file($sFilePath, $sBackupType);
	if($sBackupType == "ts3files.tar.gz")
	{
		// Remove temporary archive file from the system.
		mwexec("rm -f ".$sFilePath);		
	}
	
	if($bShouldRestartService && $bServiceRunning) start_service(TeamSpeak3Constants::ServiceName);
	return true;
}

function teamspeak3_restore_backup($sBackupType, $aFile, &$sMessage)
{
	global $config;
	
	if(!in_array($sBackupType, TeamSpeak3Constants::$BackupTypes)) 
	{
		$sMessage = "Can't restore backup, unsupported backup file.";
		return;
	}
	
	if(empty($aFile) || $aFile['error'] != UPLOAD_ERR_OK)
	{
		$sMessage = "Can't restore backup due to an upload error. Please try again.";
		return false;
	}
		
	if($sBackupType == "ts3server.ini")
	{
		if(substr($aFile['name'], -4) != '.ini')
		{
			$sMessage = "Can't restore backup, the uploaded file should have the '.ini' extension.";
			return false;
		}
	}
	else if($sBackupType == "ts3server.sqlitedb")
	{
		if(substr($aFile['name'], -9) != '.sqlitedb')
		{
			$sMessage = "Can't restore backup, the uploaded file should have the '.sqlitedb' extension.";
			return false;
		}
	}	
	else if($sBackupType == "ts3files.tar.gz")
	{
		if(substr($aFile['name'], -7) != '.tar.gz')
		{
			$sMessage = "Can't restore backup, the uploaded file should have the '.tar.gz' extension.";
			return false;
		}
	}
	
	// Stop teamspeak service, write file (to disk) and restart it again.
	$bServiceRunning = is_service_running(TeamSpeak3Constants::ServiceName);
	if($bServiceRunning) stop_service(TeamSpeak3Constants::ServiceName);
		
	$bStatus = false;
	if($sBackupType == "ts3files.tar.gz")
	{
		$sTargetFilePath = TeamSpeak3Constants::BinaryDirectory.'/files';
		mwexec("rm -Rf {$sTargetFilePath}");
		
		if(!file_exists($sTargetFilePath))
		{		
			chdir(TeamSpeak3Constants::BinaryDirectory);		
			mwexec("tar -zxof {$aFile['tmp_name']}");
			$bStatus = file_exists($sTargetFilePath);
			
			if($bStatus)
			{
				teamspeak3_chown_file_or_directory($sTargetFilePath, true);				
			}
		}
	}
	else
	{
		$sTargetFilePath = TeamSpeak3Constants::BinaryDirectory.'/'.$sBackupType;
		$bStatus = move_uploaded_file($aFile['tmp_name'], $sTargetFilePath);
	}
	
	if($bStatus)
	{
		teamspeak3_chown_file_or_directory($sTargetFilePath, false);	
				
		conf_mount_rw();		
		
		$config['installedpackages']['teamspeak3']['config'][0]['ts3serverconfig'] = base64_encode(file_get_contents(teamspeak3_get_serverini_path()));
		write_config();		
		
		conf_mount_ro();
		
		$sMessage = "The configuration file '{$sBackupType}' has been restored.";
	}	
	
	if($bServiceRunning) start_service(TeamSpeak3Constants::ServiceName);
	
	return $bStatus;
}

function teamspeak3_archive_file_browser()
{
	$sDate = date("Ymd", time());
	$sTmpArchivePath = "/tmp/ts3files-{$sDate}.tar.gz";
	$sFileBrowserPath = TeamSpeak3Constants::BinaryDirectory.'/files';	
	if(!is_dir($sFileBrowserPath)) return null;
	
	chdir(TeamSpeak3Constants::BinaryDirectory);
	mwexec("tar -zcf {$sTmpArchivePath} files/");
	return $sTmpArchivePath;
}

function teamspeak3_is_new_install()
{
	$oFileSystem = new FilesystemIterator(TeamSpeak3Constants::BinaryDirectory, FilesystemIterator::SKIP_DOTS);
	return (iterator_count($oFileSystem) <= 1);
}

function teamspeak3_custom_php_after_form_head()
{
	global $savemsg;
	
	if(empty($savemsg) && teamspeak3_is_new_install())
	{
		$savemsg = "It looks like this is a new installation of the Teamspeak 3 package on your device. Please goto <a href=\"/teamspeak3_install.php\">Install or update page</a> and complete the installation first.";
	}
}
?>