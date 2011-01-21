<?php if ($config['version'] >= 6): ?>
<link rel="stylesheet" type="text/css" href="/themes/<?php echo $g['theme']; ?>/new_tab_menu.css"media="all">
<?php else: ?>
<link rel="stylesheet" type="text/css" href="./css/new_tab_menu.css" media="all">
<?php endif; ?>

<link rel="stylesheet" type="text/css" href="./css/sexybuttons.css" media="all" />
<link rel="stylesheet" type="text/css" href="./css/style_snort2.css" media="all" />

<?php


echo "\t<script type=\"text/javascript\" src=\"./javascript/jquery-1.4.4.min.js\"></script>\n";

echo "\t<script type=\"text/javascript\" src=\"./javascript/jquery.form.js\"></script>\n";

echo "\t<script type=\"text/javascript\" src=\"./javascript/jquery.dataTables.min.js\"></script>\n";

echo "\t<script type=\"text/javascript\" src=\"./javascript/snort_globalsend.js\"></script>\n";


?>

<?php
// this has to be loaded at the bottom
$snort_custom_rnd_box = '
<script>
	/* makes boxes round */
	/* load at bottom */
	NiftyCheck();
	Rounded("div#mainarea2","bl br tr","#FFF","#dddddd","smooth");
	Rounded("div#mainarea3","bl br tr","#FFF","#dddddd","smooth");
	Rounded("td#tdbggrey","bl br tr","#FFF","#dddddd","smooth");
	Rounded("div#mainarea4","all","#FFF","#dddddd","smooth");
	Rounded("div#mainarea5","all","#eeeeee","#dddddd","smooth");
</script>	
';
?>