<?php
/*
	system_advanced_create_certs.php
	part of pfSense (https://www.pfSense.org/)
	Copyright (C) 2009 Mark J Crane
	Copyright (C) 2015 ESF, LLC
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
require("guiconfig.inc");

if (file_exists("/var/etc/ssl/openssl.cnf")) {
	$openssl = file_get_contents("/var/etc/ssl/openssl.cnf");
}

/* Lets match the fileds in the read file and populate the variables for the form */
preg_match('/C\=(.*)\n/', $openssl, $countrycodeA);
preg_match('/\nST\=(.*)\n/', $openssl, $stateorprovinceA);
preg_match('/\nL\=(.*)\n/', $openssl, $citynameA);
preg_match('/\nO\=(.*)\n/', $openssl, $orginizationnameA);
preg_match('/\nOU\=(.*)\n/', $openssl, $orginizationdepartmentA);
preg_match('/\nCN\=(.*)\n/', $openssl, $commonnameA);

$countrycode = $countrycodeA[1];
$stateorprovince = $stateorprovinceA[1];
$cityname = $citynameA[1];
$orginizationname = $orginizationnameA[1];
$orginizationdepartment = $orginizationdepartmentA[1];
$commonname = $commonnameA[1];

if ($_POST) {

	/* Grab posted variables and create a new openssl.cnf */
	$countrycode=$_POST['countrycode'];
	$stateorprovince=$_POST['stateorprovince'];
	$cityname=$_POST['cityname'];
	$orginizationname=$_POST['orginizationname'];
	$orginizationdepartment=$_POST['orginizationdepartment'];
	$commonname=$_POST['commonname'];

	/* Write out /var/etc/ssl/openssl.cnf */
	conf_mount_rw();
	safe_mkdir("/var/etc/ssl/");
	$fd = fopen("/var/etc/ssl/openssl.cnf", "w");
	fwrite($fd, "");
	fwrite($fd, "[ req ]\n");
	fwrite($fd, "distinguished_name=req_distinguished_name \n");
	fwrite($fd, "req_extensions = v3_req \n");
	fwrite($fd, "prompt=no\n");
	fwrite($fd, "default_bits            = 1024\n");
	fwrite($fd, "default_keyfile         = privkey.pem\n");
	fwrite($fd, "distinguished_name      = req_distinguished_name\n");
	fwrite($fd, "attributes              = req_attributes\n");
	fwrite($fd, "x509_extensions = v3_ca # The extentions to add to the self signed cert\n");
	fwrite($fd, "[ req_distinguished_name ] \n");
	fwrite($fd, "C=" . $countrycode . " \n");
	fwrite($fd, "ST=" . $stateorprovince. " \n");
	fwrite($fd, "L=" . $cityname . " \n");
	fwrite($fd, "O=" . $orginizationname . " \n");
	fwrite($fd, "OU=" . $orginizationdepartment . " \n");
	fwrite($fd, "CN=" . $commonname . " \n");
	fwrite($fd, "[EMAIL PROTECTED] \n");
	fwrite($fd, "[EMAIL PROTECTED] \n");
	fwrite($fd, "[ v3_req ] \n");
	fwrite($fd, "basicConstraints = critical,CA:FALSE \n");
	fwrite($fd, "keyUsage = nonRepudiation, digitalSignature, keyEncipherment, dataEncipherment, keyAgreement \n");
	fwrite($fd, "extendedKeyUsage=emailProtection,clientAuth \n");
	fwrite($fd, "[ ca ]\n");
	fwrite($fd, "default_ca      = CA_default\n");
	fwrite($fd, "[ CA_default ]\n");
	fwrite($fd, "certificate             = /tmp/cacert.pem \n");
	fwrite($fd, "private_key             = /tmp/cakey.pem \n");
	fwrite($fd, "dir                     = /tmp/\n");
	fwrite($fd, "certs                   = /tmp/certs\n");
	fwrite($fd, "crl_dir                 = /tmp/crl\n");
	fwrite($fd, "database                = /tmp/index.txt \n");
	fwrite($fd, "new_certs_dir           = /tmp/newcerts \n");
	fwrite($fd, "serial                  = /tmp/serial \n");
	fwrite($fd, "crl                     = /tmp/crl.pem \n");
	fwrite($fd, "RANDFILE                = /tmp/.rand  \n");
	fwrite($fd, "x509_extensions         = usr_cert  \n");
	fwrite($fd, "name_opt                = ca_default \n");
	fwrite($fd, "cert_opt                = ca_default \n");
	fwrite($fd, "default_days            = 365 \n");
	fwrite($fd, "default_crl_days        = 30  \n");
	fwrite($fd, "default_md              = md5 \n");
	fwrite($fd, "preserve                = no \n");
	fwrite($fd, "policy                  = policy_match\n");
	fwrite($fd, "[ policy_match ]\n");
	fwrite($fd, "countryName             = match\n");
	fwrite($fd, "stateOrProvinceName     = match\n");
	fwrite($fd, "organizationName        = match\n");
	fwrite($fd, "organizationalUnitName  = optional\n");
	fwrite($fd, "commonName              = supplied\n");
	fwrite($fd, "emailAddress            = optional\n");
	fwrite($fd, "[ policy_anything ]\n");
	fwrite($fd, "countryName             = optional\n");
	fwrite($fd, "stateOrProvinceName     = optional\n");
	fwrite($fd, "localityName            = optional\n");
	fwrite($fd, "organizationName        = optional\n");
	fwrite($fd, "organizationalUnitName  = optional\n");
	fwrite($fd, "commonName              = supplied\n");
	fwrite($fd, "emailAddress            = optional\n");
	fwrite($fd, "[ req_distinguished_name ]\n");
	//fwrite($fd, "countryName                     = US\n");
	fwrite($fd, "[ req_attributes ]\n");
	fwrite($fd, "challengePassword               = A challenge password\n");
	fwrite($fd, "unstructuredName                = An optional company name\n");
	fwrite($fd, "[ usr_cert ]\n");
	fwrite($fd, "basicConstraints=CA:FALSE\n");
	fwrite($fd, "[ v3_ca ]\n");
	fwrite($fd, "subjectKeyIdentifier=hash\n");
	fwrite($fd, "authorityKeyIdentifier=keyid:always,issuer:always\n");
	fwrite($fd, "basicConstraints = CA:true\n");
	fwrite($fd, "[ crl_ext ]\n");
	fwrite($fd, "authorityKeyIdentifier=keyid:always,issuer:always\n");
	fclose($fd);

$pgtitle = "System: Advanced - Create Certificates";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<form action="system_advanced_create_certs.php" method="post" name="iform" id="iform">
	<?php if ($input_errors) print_input_errors($input_errors); ?>
	<?php if ($savemsg) print_info_box($savemsg); ?>
	<div>One moment please...</div>
	<?php
		mwexec("cd /tmp/ && /usr/bin/openssl req -new -x509 -keyout /tmp/cakey.pem -out /tmp/cacert.pem -days 3650 -config /var/etc/ssl/openssl.cnf -passin pass:test -nodes");
		$cacert1 = file_get_contents("/tmp/cacert.pem");
		$cakey1  = file_get_contents("/tmp/cakey.pem");
		$cacertA = str_replace("\r", "", $cacert1);
		$cakeyA = str_replace("\r", "", $cakey1);
		$cacert = str_replace("\n", "\\n", $cacertA);
		$cakey = str_replace("\n", "\\n", $cakeyA);
	?>
	<script type="text/javascript">
	//<![CDATA[
		var cacert='<?=$cacert?>';
		var cakey='<?=$cakey?>';
		opener.document.forms[0].certificate.value=cacert;
		opener.document.forms[0].privatekey.value=cakey;
		this.close();
	//]]>
	</script>
</form>
<?php include("fend.inc"); ?>
</body>
</html>

<?php

} else {

$pgtitle = "System: Advanced - Create Certificates";
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
	<form action="system_advanced_create_certs.php" method="post" name="iform" id="iform">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
			<tr>
				<td width="35%" valign="top" class="vncell"><B>Country Code (2 Letters)</td>
				<td width="78%" class="vtable">
					<input name="countrycode" type="text" value="<?=$countrycode?>" />
				</td>
			</tr>

			<tr>
				<td width="35%" valign="top" class="vncell"><B>State or Province name</td>
				<td width="78%" class="vtable">
					<input name="stateorprovince" type="text" value="<?=$stateorprovince?>" />
				</td>
			</tr>

			<tr>
				<td width="35%" valign="top" class="vncell"><B>City name</td>
				<td width="78%" class="vtable">
					<input name="cityname" type="text" value="<?=$cityname?>" />
				</td>
			</tr>

			<tr>
				<td width="35%" valign="top" class="vncell"><B>Organization name</td>
				<td width="78%" class="vtable">
					<input name="orginizationname" type="text" value="<?=$orginizationname?>" />
				</td>
			</tr>

			<tr>
				<td width="35%" valign="top" class="vncell"><B>Organization department</td>
				<td width="78%" class="vtable">
					<input name="orginizationdepartment" type="text" value="<?=$orginizationdepartment?>" />
				</td>
			</tr>

			<tr>
				<td width="35%" valign="top" class="vncell"><B>Common Name (Your name)</td>
				<td width="78%" class="vtable">
					<input name="commonname" type="text" value="<?=$commonname?>" />
				</td>
			</tr>

			<!--
			<tr>
				<td width="35%" valign="top" class="vncell"><B>E-Mail address</td>
				<td width="78%" class="vtable">
					<input name="email" type="text" value="<?=$email?>" />
				</td>
			</tr>
			-->

			<tr>
				<td width="35%" valign="top">&nbsp;</td>
				<td width="78%">
					<input name="Submit" type="submit" class="formbtn" value="Save" />
				</td>
			</tr>
		</table>
	</form>

<?php include("fend.inc"); ?>
</body>
</html>

<?php
}
	conf_mount_ro();

?>
