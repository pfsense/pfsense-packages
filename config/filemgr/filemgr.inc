<?php
/* $Id$ */
/*
/* ========================================================================== */
/*
	filemgr.inc
	
	All rights reserved.
																			*/
/* ========================================================================== */
/*

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



function php_sync_package_fm() 
{
	global $config;
}


function php_install_command_fm() 
{

	global $config;
	conf_mount_rw();

	if (!is_dir('/usr/local/www/packages/')) {
		exec("mkdir /usr/local/www/packages/");
	}

	if (!is_dir('/usr/local/www/packages/filemgr/')) {
		exec("mkdir /usr/local/www/packages/filemgr/");
	}

	if (!is_dir('/usr/local/www/packages/filemgr/rbfmimg/')) {
		exec("mkdir /usr/local/www/packages/filemgr/rbfmimg/");
	}

	if (!is_dir('/usr/local/www/packages/filemgr/rbfminc/')) {
		exec("mkdir /usr/local/www/packages/filemgr/rbfminc/");
	}



	
	
	//rename PHP files from .tmp to .php
	exec("cp /tmp/file_manager.tmp /usr/local/www/packages/filemgr/file_manager.php");
	unlink_if_exists("/tmp/file_manager.tmp");

	exec("cp /tmp/filemgr.inc /usr/local/www/packages/filemgr/filemgr.inc");
	unlink_if_exists("/tmp/filemgr.inc");
	
	exec("cp /tmp/filemgr.xml /usr/local/www/packages/filemgr/filemgr.xml");
	unlink_if_exists("/tmp/filemgr.xml");

	exec("cp /tmp/index.tmp /usr/local/www/packages/filemgr/index.php");
	unlink_if_exists("/tmp/index.tmp");

	exec("cp /tmp/bg_footer.png /usr/local/www/packages/filemgr/rbfmimg/bg_footer.png");
	unlink_if_exists("/tmp/bg_footer.png");

	exec("cp /tmp/bg_header.png /usr/local/www/packages/filemgr/rbfmimg/bg_header.png");
	unlink_if_exists("/tmp/bg_header.png");
	exec("cp /tmp/bg_page.png /usr/local/www/packages/filemgr/rbfmimg/bg_page.png");
	unlink_if_exists("/tmp/bg_page.png");
	exec("cp /tmp/file_editor_bg.png /usr/local/www/packages/filemgr/rbfmimg/file_editor_bg.png");
	unlink_if_exists("/tmp/file_editor_bg.png");
	exec("cp /tmp/folder.png /usr/local/www/packages/filemgr/rbfmimg/folder.png");
	unlink_if_exists("/tmp/folder.png");
	exec("cp /tmp/folder_go.png /usr/local/www/packages/filemgr/rbfmimg/folder_go.png");
	unlink_if_exists("/tmp/folder_go.png");
	exec("cp /tmp/folder_up.png /usr/local/www/packages/filemgr/rbfmimg/folder_up.png");
	unlink_if_exists("/tmp/folder_up.png");
	exec("cp /tmp/go.png /usr/local/www/packages/filemgr/rbfmimg/go.png");
	unlink_if_exists("/tmp/go.png");
	exec("cp /tmp/ico_delete.png /usr/local/www/packages/filemgr/rbfmimg/ico_delete.png");
	unlink_if_exists("/tmp/ico_delete.png");
	exec("cp /tmp/ico_download.png /usr/local/www/packages/filemgr/rbfmimg/ico_download.png");
	unlink_if_exists("/tmp/ico_download.png");
	exec("cp /tmp/ico_file.png /usr/local/www/packages/filemgr/rbfmimg/ico_file.png");
	unlink_if_exists("/tmp/ico_file.png");
	exec("cp /tmp/ico_html.png /usr/local/www/packages/filemgr/rbfmimg/ico_html.png");
	unlink_if_exists("/tmp/ico_html.png");
	exec("cp /tmp/ico_open_as_web.png /usr/local/www/packages/filemgr/rbfmimg/ico_open_as_web.png");
	unlink_if_exists("/tmp/ico_open_as_web.png");
	exec("cp /tmp/ico_php.png /usr/local/www/packages/filemgr/rbfmimg/ico_php.png");
	unlink_if_exists("/tmp/ico_php.png");
	exec("cp /tmp/ico_picture.png /usr/local/www/packages/filemgr/rbfmimg/ico_picture.png");
	unlink_if_exists("/tmp/ico_picture.png");
	exec("cp /tmp/ico_rename.png /usr/local/www/packages/filemgr/rbfmimg/ico_rename.png");
	unlink_if_exists("/tmp/ico_rename.png");
	exec("cp /tmp/ico_script_edit.png /usr/local/www/packages/filemgr/rbfmimg/ico_script_edit.png");
	unlink_if_exists("/tmp/ico_script_edit.png");
	exec("cp /tmp/ico_use_file.png /usr/local/www/packages/filemgr/rbfmimg/ico_use_file.png");
	unlink_if_exists("/tmp/ico_use_file.png");
	exec("cp /tmp/ico_use_file_inactive.png /usr/local/www/packages/filemgr/rbfmimg/ico_use_file_inactive.png");
	unlink_if_exists("/tmp/ico_use_file_inactive.png");
	exec("cp /tmp/index.html /usr/local/www/packages/filemgr/rbfmimg/index.html");
	unlink_if_exists("/tmp/index.html");
	exec("cp /tmp/new.png /usr/local/www/packages/filemgr/rbfmimg/new.png");
	unlink_if_exists("/tmp/new.png");

	exec("cp /tmp/config.tmp /usr/local/www/packages/filemgr/rbfminc/config.php");
	unlink_if_exists("/tmp/config.tmp");
	exec("cp /tmp/download.tmp /usr/local/www/packages/filemgr/rbfminc/download.php");
	unlink_if_exists("/tmp/download.tmp");
	exec("cp /tmp/file_editor_style.css /usr/local/www/packages/filemgr/rbfminc/file_editor_style.css");
	unlink_if_exists("/tmp/file_editor_style.css");
	exec("cp /tmp/functions.tmp /usr/local/www/packages/filemgr/rbfminc/functions.php");
	unlink_if_exists("/tmp/functions.tmp");
	exec("cp /tmp/index.html /usr/local/www/packages/filemgr/rbfminc/index.html");
	unlink_if_exists("/tmp/index.html");
	exec("cp /tmp/rename.tmp /usr/local/www/packages/filemgr/rbfminc/rename.php");
	unlink_if_exists("/tmp/rename.tmp");
	exec("cp /tmp/session.tmp /usr/local/www/packages/filemgr/rbfminc/session.php");
	unlink_if_exists("/tmp/session.tmp");

	

	conf_mount_ro();
}


function deinstall_command_fm() 
{
	conf_mount_rw();
	$handle = popen("/usr/local/etc/rc.d/filemgr.sh stop", "r");
	unlink_if_exists("/usr/local/pkg/filemgr.xml");
	unlink_if_exists("/usr/local/pkg/filemgr.inc");
	exec("rm -R /usr/local/www/packages/filemgr/filemgrs");
	exec("rm -R /usr/local/www/packages/filemgr");
	exec("rm /usr/local/etc/rc.d/filemgr.sh");
	exec("pfctl -t filemgr -T kill");
	exec("sed -i -e '/filemgr/d' /tmp/rules.debug");
	exec("pfctl -o basic -f /tmp/rules.debug");
	conf_mount_ro();
}

?>
