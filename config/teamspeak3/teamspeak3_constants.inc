<?php
/* $Id$ */
/* ========================================================================== */
/*
	teamspeak3_constants.inc
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

class TeamSpeak3Constants
{
	const ServiceName = "teamspeak3";
	const PkgDirectory = "/usr/local/pkg";
	const BinaryDirectory = "/etc/teamspeak3";
	const BinaryStartScript = "ts3server_startscript.sh";
	const BinaryUser = "teamspeak";
	const BinaryGroup = "teamspeak";
	const ServerConfigFile = "ts3server.ini";
	static $BackupTypes = array("ts3server.ini", "ts3server.sqlitedb", "ts3files.tar.gz");
	static $SupportedBinaryFiles = array("ts3server_freebsd_x86", "ts3server_freebsd_amd64");
}
?>