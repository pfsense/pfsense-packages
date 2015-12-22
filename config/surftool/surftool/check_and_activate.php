<?php
/*
	check_and_activate.php
	Copyright (C) 2015 H-T Reimers <reimers@mail.de>
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

	//Read configuration
	$config=parse_ini_file ("surftool.ini");

	$debug=0;
	$max_execution_time=5;

	$commandpath="/tmp/surftool";
	
	

	$delay=2;
	if(isset($argv[1])){
		if($argv[1]>=0 AND $argv[1]<600){
			$delay=$argv[1];
		}
	}
	
	//check
	set_time_limit($max_execution_time);
	if (ini_get('max_execution_time') != $max_execution_time){
		echo "Warning max_execution_time is ".ini_get('max_execution_time')."s should be ".$max_execution_time."s\n";
	}

	include "surftool_class.inc";

	$sgcommands= new commandReader($commandpath);

	//Are there any changes?
	if($sgcommands->changes){

		//read squidGuard.conf
		$sgconf = new surftoolSquidGuardConf($config);

		//activate domain changes
		$sgconf->setDomains("onplus",$sgcommands->domainsOnplusAdd,$sgcommands->domainsOnplusRemove);
		$sgconf->setDomains("only",$sgcommands->domainsOnlyAdd,$sgcommands->domainsOnlyRemove);

		$sgconf->write_squidGuardDomains();
		
		//activate acl mode changes
		$sgconf->setAclMode($sgcommands->aclChangeMode);
		$sgconf->write_squidGuardConf();

		//Reload Squid Configuration
		$sgconf->squidReload();

		$sgcommands->deleteCommandFiles();
				
	}

?>
