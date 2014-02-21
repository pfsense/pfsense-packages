<?php
require_once("guiconfig.inc");
$image = $_GET['image'];
header("Content-type: image/png");
readfile("/tmp/$image");
?> 
