<?php

/**
 * Author: Evgeny Yurchenko
 * Email: ey@tm-k.com
 * 
 * Developed for: pfSense.com
 * 
 * Copyright (c) 2010, pfSense.com
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in thedocumentation and/or other materials provided with the distribution.
 * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

require('filter.inc');

function widescreen_custom_php_install_command()
{
        exec("mkdir -p /usr/local/pkg/widescreen/backup");
	// Let's backup everything
	copy("/usr/local/www/themes/pfsense_ng/all.css", "/usr/local/pkg/widescreen/backup/all.css.bak");
	copy("/usr/local/www/fbegin.inc", "/usr/local/pkg/widescreen/backup/fbegin.inc.bak");
	copy("/usr/local/www/fend.inc", "/usr/local/pkg/widescreen/backup/fend.inc.bak");
	copy("/usr/local/www/index.php", "/usr/local/pkg/widescreen/backup/index.php.bak");
 	
	// Install patches
	copy("/usr/local/pkg/widescreen/all.css_", "/usr/local/www/themes/pfsense_ng/all.css");
	copy("/usr/local/pkg/widescreen/fbegin.inc_", "/usr/local/www/fbegin.inc");
	copy("/usr/local/pkg/widescreen/fend.inc_", "/usr/local/www/fend.inc");
	copy("/usr/local/pkg/widescreen/index.php_", "/usr/local/www/index.php");
	copy("/usr/local/pkg/widescreen/footer-left.png", "/usr/local/www/themes/pfsense_ng/images/footer-left.png");
	copy("/usr/local/pkg/widescreen/footer-middle.png", "/usr/local/www/themes/pfsense_ng/images/footer-middle.png");
	copy("/usr/local/pkg/widescreen/footer-right.png", "/usr/local/www/themes/pfsense_ng/images/footer-right.png");
	copy("/usr/local/pkg/widescreen/header-mid.png", "/usr/local/www/themes/pfsense_ng/images/header-mid.png");
	copy("/usr/local/pkg/widescreen/horiz-left.png", "/usr/local/www/themes/pfsense_ng/images/horiz-left.png");
	copy("/usr/local/pkg/widescreen/horiz-right.png", "/usr/local/www/themes/pfsense_ng/images/horiz-right.png");
}

function widescreen_custom_php_deinstall_command()
{
	copy("/usr/local/pkg/widescreen/backup/all.css.bak", "/usr/local/www/themes/pfsense_ng/all.css");
	copy("/usr/local/pkg/widescreen/backup/fbegin.inc.bak", "/usr/local/www/fbegin.inc");
	copy("/usr/local/pkg/widescreen/backup/fend.inc.bak", "/usr/local/www/fend.inc");
	copy("/usr/local/pkg/widescreen/backup/index.php.bak", "/usr/local/www/index.php");
        exec("rm /usr/local/www/themes/pfsense_ng/images/footer-left.png");
        exec("rm /usr/local/www/themes/pfsense_ng/images/footer-middle.png");
        exec("rm /usr/local/www/themes/pfsense_ng/images/footer-right.png");
        exec("rm /usr/local/www/themes/pfsense_ng/images/header-mid.png");
        exec("rm /usr/local/www/themes/pfsense_ng/images/horiz-left.png");
        exec("rm /usr/local/www/themes/pfsense_ng/images/horiz-right.png");
        exec("rm -r /usr/local/pkg/widescreen");
}
	
?>