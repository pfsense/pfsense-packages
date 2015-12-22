<?php

global $priv_list;


$priv_list['page-surftoolgeneral'] = array();
$priv_list['page-surftoolgeneral']['name']  = gettext("WebCfg - Services: Surftool");
$priv_list['page-surftoolgeneral']['descr'] = "Indicates whether the user is able to login on the surftool add-on.";
$priv_list['page-surftoolgeneral']['match'] = array();
$priv_list['page-surftoolgeneral']['match'][] = "surftool/surftool_set.php*";

?>