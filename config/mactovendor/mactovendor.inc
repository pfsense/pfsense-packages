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

function mactovendor_custom_php_install_command()
{
        exec("mkdir -p /usr/local/pkg/mactovendor/backup");
	// Let's backup everything
	copy("/usr/local/www/diag_arp.php", "/usr/local/pkg/mactovendor/backup/diag_arp.php.bak");
	copy("/usr/local/www/status_dhcp_leases.php", "/usr/local/pkg/mactovendor/backup/status_dhcp_leases.php.bak");
	copy("/usr/local/www/status_interfaces.php", "/usr/local/pkg/mactovendor/backup/status_interfaces.php.bak");
        copy("/usr/local/www/status_wireless.php", "/usr/local/pkg/mactovendor/backup/status_wireless.php.php.bak");

	
	// Install patches
	copy("/usr/local/pkg/mactovendor/diag_arp.php_", "/usr/local/www/diag_arp.php");
	copy("/usr/local/pkg/mactovendor/status_dhcp_leases.php_", "/usr/local/www/status_dhcp_leases.php");
	copy("/usr/local/pkg/mactovendor/status_interfaces.php_", "/usr/local/www/status_interfaces.php");
        copy("/usr/local/pkg/mactovendor/status_wireless.php_", "/usr/local/www/status_wireless.php");
}

function mactovendor_custom_php_deinstall_command()
{
	copy("/usr/local/pkg/mactovendor/backup/diag_arp.php.bak", "/usr/local/www/diag_arp.php");
	copy("/usr/local/pkg/mactovendor/backup/status_dhcp_leases.php.bak", "/usr/local/www/status_dhcp_leases.php");
	copy("/usr/local/pkg/mactovendor/backup/status_interfaces.php.bak", "/usr/local/www/status_interfaces.php");
        copy("/usr/local/pkg/mactovendor/backup/status_wireless.php.bak", "/usr/local/www/status_wireless.php");
        exec("rm -r /usr/local/pkg/mactovendor");
}
	
?>
