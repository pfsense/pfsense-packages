<?php

require_once("/usr/local/pkg/snort/snort_new.inc");

// set page vars

$a_whitelist = snortSql_fetchAllWhitelistTypes('SnortWhitelist', 'SnortWhitelistips');

$a_suppresslist = snortSql_fetchAllWhitelistTypes('SnortSuppress', '');

//$a_whitelist = snortSql_fetchAllSettings('snortDBrules', 'Snortrules', 'uuid', '42770');

  echo '<pre>' . "\n\n";

	print_r($a_suppresst);
	
  	//foreach ($a_whitelist as $value)
	//{
		//echo $value['filename'] . "\n";
	//}  

  echo "\n" . '</pre>';

?>




