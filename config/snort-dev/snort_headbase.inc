

<!-- START of Snort Package css and javascript -->

<link rel="stylesheet" type="text/css" href="./css/style_snort2.css" media="all" />
<!-- <link rel="stylesheet" type="text/css" href="./css/jquery.bubblepopup.v2.3.1.css" media="all" /> -->

<script type="text/javascript" src="./javascript/jquery-1.6.min.js"></script>
<script type="text/javascript" src="./javascript/snort_globalsend.js"></script>
<script type="text/javascript" src="./javascript/jquery.form.js"></script>
<!-- <script type="text/javascript" src="./javascript/jquery.bubblepopup.v2.3.1.min.js"></script>  -->

<!-- STOP of Snort Package css and javascript -->


<?php
// this has to be loaded at the bottom
$snort_custom_rnd_box = '
<script type="text/javascript" >
	/* makes boxes round */
	/* load at bottom    */
	NiftyCheck();
	Rounded("div#mainarea2","bl br tr","#FFF","#dddddd","smooth");
	Rounded("div#mainarea3","bl br tr","#FFF","#dddddd","smooth");
	Rounded("td#tdbggrey","bl br tr","#FFF","#dddddd","smooth");
	Rounded("td#tdbggrey2","bl br tr","#FFF","#dddddd","smooth");
	Rounded("div#mainarea4","all","#FFF","#dddddd","smooth");
	Rounded("div#mainarea6","all","#FFF","#dddddd","smooth");
	Rounded("div#mainarea5","all","#eeeeee","#dddddd","smooth");
</script>	
';
?>