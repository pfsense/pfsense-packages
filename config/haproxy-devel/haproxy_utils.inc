<?php
/*
	haproxy_utils.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2013 PiBa-NL
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
	AUTHOR BE LIABLE FOR ANY DIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/*
  This file contains functions which are NOT specific to HAProxy and may/could/should 
  be moved to the general pfSense php library for possible easy use by other parts of pfSense
*/

require_once("config.inc");

function haproxy_interface_ip($interfacebindname,$userfriendly=false){
	$list = haproxy_get_bindable_interfaces();
	$item = $list[$interfacebindname];
	$result = $item['ip'];
	if ($userfriendly && !$result)
		$result = $item['name'];
	return $result;
}
function haproxy_get_bindable_interfaces($ipv="ipv4,ipv6", $interfacetype="any,localhost,real,carp,ipalias"){
	// returns a list of ALL interface/IPs that can be used to bind a service to.
	// filtered by the conditions given in the two filter parameters.
	// result array includes:
	// $bindable[key]                can be stored and compared with previous setings
	// $bindable[key]['ip']          the current IP (possibly changes for dhcp enabled interfaces..)
	// $bindable[key]['description'] can be shown to user in a selection box

	global $config;
	$ipverions = split(',',$ipv);
	$interfacetypes= split(',',$interfacetype);

	$bindable = array();
	if (in_array("ipv4",$ipverions)){
		if (in_array('any',$interfacetypes)){
			$item = array();
			$item[ip] = '0.0.0.0';
			$item[name] = 'any (IPv4)';
			$bindable['any_ipv4'] = $item;
		}
		if (in_array('localhost',$interfacetypes)){
			$item = array();
			$item[ip] = '127.0.0.1';
			$item[name] = 'localhost (IPv4)';
			$bindable['localhost_ipv4'] = $item;
		}
		if (in_array('real',$interfacetypes)){
			foreach($config['interfaces'] as $if => $ifdetail) {
				if (!isset($ifdetail['enable']))
					continue;
				if (!isset($ifdetail['ipaddr']))
					continue;
				$item = array();
				$item[ip] = get_interface_ip($if);
				$item[name] = $ifdetail['descr'].' address (IPv4)';
				$bindable[$if.'_ipv4'] = $item;
			}
		}
		if (in_array('carp',$interfacetypes)){
			$carplist = get_configured_carp_interface_list();
			foreach ($carplist as $carpif => $carpip){
				if (is_ipaddrv4($carpip)){
					$item = array();
					$item['ip'] = $carpip;
					$item['name'] = $carpip." (".get_vip_descr($carpip).")";
					$bindable[$carpip] = $item;
				}
			}

		}
		if (in_array('ipalias',$interfacetypes)){
			$aliaslist = get_configured_ip_aliases_list();
			foreach ($aliaslist as $aliasip => $aliasif){
				if (is_ipaddrv4($aliasip)){
					$item = array();
					$item['ip'] = $aliasip;
					$item['name'] = $aliasip." (".get_vip_descr($aliasip).")";
					$bindable[$aliasip.'_ipv4'] = $item;	
				}
			}
		}
	}
	if (in_array("ipv6",$ipverions)){
		if (in_array('any',$interfacetypes)){
			$item = array();
			$item[ip] = '::';
			$item[name] = 'any (IPv6)';
			$bindable['any_ipv6'] = $item;
		}
		if (in_array('localhost',$interfacetypes)){
			$item = array();
			$item[ip] = '::1';
			$item[name] = 'localhost (IPv6)';
			$bindable['localhost_ipv6'] = $item;
		}
		if (in_array('real',$interfacetypes)){
			foreach($config['interfaces'] as $if => $ifdetail) {
				if (!isset($ifdetail['enable']))
					continue;
				if (!isset($ifdetail['ipaddrv6']))
					continue;
				$item = array();
				$item[ip] = get_interface_ipv6($if);
				$item[name] = $ifdetail['descr'].' address (IPv6)';
				$bindable[$if.'_ipv6'] = $item;
			}
		}		
		if (in_array('carp',$interfacetypes)){
			$carplist = get_configured_carp_interface_list();
			foreach ($carplist as $carpif => $carpip){
				if (is_ipaddrv6($carpip)){
					$item = array();
					$item['ip'] = $carpip;
					$item['name'] = $carpip." (".get_vip_descr($carpip).")";
					$bindable[$carpip] = $item;
				}
			}

		}
		if (in_array('ipalias',$interfacetypes)){
			$aliaslist = get_configured_ip_aliases_list();
			foreach ($aliaslist as $aliasip => $aliasif){
				if (is_ipaddrv6($aliasip)){
					$item = array();
					$item['ip'] = $aliasip;
					$item['name'] = $aliasip." (".get_vip_descr($aliasip).")";
					$bindable[$aliasip] = $item;	
				}
			}
		}
	}
	return $bindable;
}

function haproxy_get_cert_extensions($crt){
	$cert = openssl_x509_parse(base64_decode($crt['crt']));
	return $cert['extensions'];
}

function haproxy_get_cert_authoritykeyidentifier($cert)
{
	$certextension = haproxy_get_cert_extensions($cert);
	$lines = preg_split('/[\n]+/',$certextension['authorityKeyIdentifier']);
	return substr($lines[0],6);// cut off the starting string 'keyid:'
}
function haproxy_get_cert_subjectKeyIdentifier($cert)
{
	$certextension = haproxy_get_cert_extensions($cert);
	$lines = preg_split('/[\n]+/',$certextension['subjectKeyIdentifier']);
	return $lines[0];
}

function haproxy_cert_signed_by($cert, $signedbycert) {
	// checks if $cert was signed by $signedbycert
	// this does NOT validate a proper signature but only checks if the extension properties match.
	$authoritykeyid = haproxy_get_cert_authoritykeyidentifier($cert);
	$subjectid = haproxy_get_cert_subjectKeyIdentifier($signedbycert);
	return $authoritykeyid == $subjectid;
}

function haproxy_get_certificates(){
	global $config;
	$allcerts = array();
	foreach($config['cert'] as &$cert)
		$allcerts[] = &$cert;
	foreach($config['ca'] as &$cert)
		$allcerts[] = &$cert;
	return $allcerts;
}
function haproxy_recalculate_certifcate_chain(){
	// and set "selfsigned" for certificates that where used to sign themselves
	// recalculate the "caref" for all certificates where it is currently unkown.
	
	$allcertificates = haproxy_get_certificates();
	$items_recalculated = 0;
	foreach($allcertificates as &$cert){
		$recalculate=false;
		if (!isset($cert['selfsigned'])){
			if (!isset($cert['caref']))
				$recalculate=true;
			else {		
				$ca = lookup_ca($cert['caref']);
				if (!$ca)	
					$recalculate=true;
			}
		}
		if ($recalculate){
			foreach($allcertificates as &$signedbycert){
				if(haproxy_cert_signed_by($cert, $signedbycert)){
					if ($cert['refid'] == $signedbycert['refid']){
						$cert['selfsigned'] = true;
					} else {					
						$cert['caref'] = $signedbycert['refid'];
					}
					$items_recalculated++;
				}
			}
		}
	}
	if ($items_recalculated > 0)
		write_config("Recalculated $items_recalculated certificate chains.");
}

function phparray_to_javascriptarray_recursive($nestID, $path, $items, $nodeName, $includeitems) {
	$offset = str_repeat('  ',$nestID);
	$itemName = "item$nestID";
	echo "{$offset}$nodeName = {};\n";
	if (is_array($items))
		foreach ($items as $key => $item)
		{
			if (in_array($path.'/'.$key, $includeitems))
				$subpath = $path.'/'.$key;
			else
				$subpath = $path.'/*';
			if (in_array($subpath, $includeitems) || in_array($path.'/*', $includeitems)) {
				if (is_array($item)) {
					$subNodeName = "item$nestID";
					phparray_to_javascriptarray_recursive($nestID+1, $subpath, $items[$key], $subNodeName, $includeitems);
					echo "{$offset}{$nodeName}['{$key}'] = $itemName;\n";
				} else
					echo "{$offset}{$nodeName}['$key'] = '$item';\n";
			}
		}
}
function phparray_to_javascriptarray($items, $javaMapName, $includeitems) {
		phparray_to_javascriptarray_recursive(1,'',$items, $javaMapName, $includeitems);
}

function echo_html_select($name, $keyvaluelist, $selected, $listEmptyMessage="", $onchangeEvent="") {
	if (count($keyvaluelist)>0){
		if ($onchangeEvent != "")
			$onchangeEvent .= " onchange=$onchangeEvent";
		echo "<select name=\"$name\" id=\"$name\" class=\"formselect\"$onchangeEvent>";
		foreach($keyvaluelist as $key => $desc){
			$selectedhtml = $key == $selected ? "selected" : "";
			echo "<option value=\"{$key}\" {$selectedhtml}>{$desc['name']}</option>";
		}
		echo "</select>";
	} else {
		echo $listEmptyMessage;
	}
}

?>