<?php
# ----------------------------------------------------------------------------------------------------------------------
# Error page generator
# (C)2006-2007 Serg Dvoriancev
# ----------------------------------------------------------------------------------------------------------------------
# .php?url='redirect url' 
# ----------------------------------------------------------------------------------------------------------------------
# Forbidden 403 
# Not found 404 
# 410
# Internal Error 500 
# Moved 301 
# Found 302 
# ----------------------------------------------------------------------------------------------------------------------

define('ACTION_URL', 'url');
define('ACTION_RES', 'res');
define('ACTION_MSG', 'msg');

header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1

$url  = '';
$msg  = '';
if (count($_POST)) {
    $url  = $_POST['url'];    
    $msg  = $_POST['msg'];
} else {
    $url  = $_GET['url'];    
    $msg  = $_GET['msg'];
}

if ($url) {
    if     (strstr($url, "301")) header("HTTP/1.0 301"); 
    elseif (strstr($url, "302")) header("HTTP/1.0 302"); 
    elseif (strstr($url, "403")) header("HTTP/1.0 403"); 
    elseif (strstr($url, "404")) header("HTTP/1.0 404"); 
    elseif (strstr($url, "410")) header("HTTP/1.0 410"); 
#    elseif (strstr($url, "410")) header("HTTP/1.0 500"); 
    else {
            #
            # redirect to specified url
            #
            header("HTTP/1.0");
            header("Location: $url", '', 301);
    }
    exit();
} else {
    header("HTTP/1.0 410");
    exit();
}
?>