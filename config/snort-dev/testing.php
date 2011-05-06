<?php

//require_once("/usr/local/pkg/snort/snort_new.inc");


// fetch db Settings NONE Json
function snortSql_fetchAllSettings($dbrule, $table, $type, $id_uuid) 
{

  if ($table == '') 
  {
    return false;
  }
  
  $db = sqlite_open("/usr/local/pkg/snort/$dbrule");
  
  if ($type == 'id')
  {   
    $result = sqlite_query($db,
            "SELECT * FROM {$table} where id = '{$id_uuid}';
     ");
  }
  
  if ($type == 'uuid')
  {   
    $result = sqlite_query($db,
            "SELECT * FROM {$table} where uuid = '{$id_uuid}';
     ");
  }  

  $chktable = sqlite_fetch_array($result, SQLITE_ASSOC);

  sqlite_close($db);
  
  return $chktable;
  
  
} // end func



$generalSettings = snortSql_fetchAllSettings('snortDB', 'SnortWhitelist', 'uuid', '2565656');

  echo '<pre>' . "\n\n";

	print_r($generalSettings);

  echo "\n" . '</pre>';
	
?>




