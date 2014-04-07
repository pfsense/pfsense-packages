#!/usr/local/bin/php -q
<?php
/* $Id$ */
/*
	check_ip.php
	Copyright (C) 2013 Marcello Coutinho		
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
error_reporting(0);
// stdin loop
if (! defined(STDIN)) {
        define("STDIN", fopen("php://stdin", "r"));
}
if (! defined(STDOUT)){
        define("STDOUT", fopen('php://stdout', 'w'));
        }
while( !feof(STDIN)){
        $line = trim(fgets(STDIN));
        // %SRC
        
$pf_version=substr(trim(file_get_contents("/etc/version")),0,3);
unset($cp_db);
if ($pf_version > 2.0){
	$dir="/var/db";
	$files=scandir($dir);
	foreach ($files as $file){
		if (preg_match("/captive.*db/",$file)){
			$dbhandle = sqlite_open("$dir/$file", 0666, $error);
			if ($dbhandle){
				$query = "select * from captiveportal";
				$result = sqlite_array_query($dbhandle, $query, SQLITE_ASSOC);
				if ($result){
					foreach ($result as $rownum => $row){
						$cp_db[$rownum]=implode(",",$row);
						}
					sqlite_close($dbhandle);
					}
				}
			}
        }
	}
else{
       $filename="/var/db/captiveportal.db";
       if (file_exists($filename))
        	$cp_db=file($filename);	
}
 
        $usuario="";
        // 1376630450,2,172.16.3.65,00:50:56:9c:00:c7,admin,e1779ea20d0a11c7,,,,
        if (is_array($cp_db)){
	        foreach ($cp_db as $cpl){
	        	$fields=explode(",",$cpl);
	        	if ($fields[2] != "" && $fields[2]==$line)
	        		$usuario=$fields[4];
	        }
        }
        if ($usuario !="")
            $resposta="OK user={$usuario}";
        else
            $resposta="ERR";
        fwrite (STDOUT, "{$resposta}\n");
        unset($cp_db);
}
?>

