<?php 
/* $Id$ */
/*

 part of pfSense
 All rights reserved.

 Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
 All rights reserved.

 Pfsense Old snort GUI 
 Copyright (C) 2006 Scott Ullrich.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
*/
?>

<!-- START of Snort Package css and javascript -->

<link rel="stylesheet" type="text/css" href="./css/style_snort2.css" media="all" />
<!-- <link rel="stylesheet" type="text/css" href="./css/jquery.bubblepopup.v2.3.1.css" media="all" /> -->

<script type="text/javascript" src="/snort/javascript/jquery-1.6.1.min.js"></script>
<script type="text/javascript" src="/snort/javascript/snort_globalsend.js"></script>
<script type="text/javascript" src="/snort/javascript/jquery.form.js"></script>
<script type="text/javascript" src="/snort/javascript/jquery.progressbar.min.js"></script>
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