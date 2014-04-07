#!/usr/local/bin/php
<?php
require_once("config.inc");
require_once("patches.inc");

global $g, $config;

echo "Applying patches...";
bootup_apply_patches();
echo "Done.\n";
?>