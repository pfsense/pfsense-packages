<?php
/* $Id$ */
/*
    autoconfigbackup.inc
    Copyright (C) 2008 Scott Ullrich

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

/* ensures patches match */
function custom_php_validation_command($post, $input_errors) {
	global $_POST, $savemsg, $config;

	if($post['password'] <> $post['passwordagain']) 
		$input_errors[] = "Sorry, the entered passwords do not match.";

	if($post['crypto_password'] <> $post['crypto_password2']) 
		$input_errors[] = "Sorry, the entered encryption passwords do not match.";

	if($post['testconnection']) {
		$status = test_connection($post);
		if($status) 
			$savemsg = "Connection to portal.pfsense.org was tested with no errors.";
	}
	
	// We do not need to store this value.
	unset($_POST['testconnection']);
}

function test_connection($post) {
	global $savemsg, $config;

	// Seperator used during client / server communications
	$oper_sep			= "\|\|";

	// Encryption password
	$decrypt_password 	= $post['crypto_password'];

	// Defined username
	$username			= $post['username'];

	// Defined password
	$password			= $post['password'];

	// Set hostname
	$hostname			= $config['system']['hostname'];

	// URL to restore.php
	$get_url			= "https://{$username}:{$password}@portal.pfsense.org/pfSconfigbackups/restore.php";

	// Populate available backups
	$curl_session = curl_init();
	curl_setopt($curl_session, CURLOPT_URL, $get_url);
	curl_setopt($curl_session, CURLOPT_SSL_VERIFYPEER, 0);
	curl_setopt($curl_session, CURLOPT_POST, 1);
	curl_setopt($curl_session, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($curl_session, CURLOPT_POSTFIELDS, "action=showbackups&hostname={$hostname}");
	$data = curl_exec($curl_session);

	if (curl_errno($curl_session)) 
		return("An error occurred " . curl_error($curl_session));
	else 
		curl_close($curl_session);

	return;
}

?>