<?php

require_once('/usr/local/pkg/snort/snort_new.inc');

$uuid = '2565656';

$a_list = snortSql_fetchAllWhitelistTypes('SnortWhitelist');



  echo '<pre>' . "\n\n";
    
    
    print_r($a_list);
  
  
  
  echo "\n" . '</pre>';




?>




