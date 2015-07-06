<?php
require_once("guiconfig.inc");
$image = basename($_GET['image']);
header("Content-type: image/png");
readfile("/tmp/vnstat/{$image}");
?> 
