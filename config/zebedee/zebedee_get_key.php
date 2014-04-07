<?

require_once("pkg-utils.inc");

$id= $_REQUEST['id'] ; 
//echo "<pre>" ; 
$external = $config['installedpackages']['zebedee']['config'][0]['external_address'] ; 
$chave = $config['installedpackages']['zebedeekeys']["config"][$id] ; 

//print_r($chave['row']) ; 



foreach ($chave['row'] as $k => $v) 
{
		// especify only one port for this host 
//		if($v['port']=="") $end=" " ; else $end = ":".$v['port'] ; 
		$tunnels .= "tunnel ".$v['loc_port'].":".$v['ipaddress'].":".$v['rmt_port']."\r\n" ; 
}


header('Content-Type: application/download');
header('Content-Disposition: filename=client.txt');

$chave_result = <<<EOF
verbosity 2	
server false   
message {$chave["ident"]}
detached true	
privatekey "{$chave["private_key"]}"
ipmode both
compression zlib:9

serverhost {$external}

{$tunnels}

EOF;


echo $chave_result ; 


?>  