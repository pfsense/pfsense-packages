<?php 
$uname=posix_uname();
if ($uname['machine']=='amd64')
        ini_set('memory_limit', '250M');

function get_networks($pfb){
	$file='/usr/local/pkg/pfblocker_aliases/'.$pfb.'.txt';
	if (file_exists($file))
		$return= file_get_contents($file);
	print $return;
}

if($_SERVER['REMOTE_ADDR']== '127.0.0.1'){
	if (preg_match("/(\w+)/",$_REQUEST['pfb'],$matches)){
		get_networks($matches[1]);
		}
	}
if ($argv[1]=='uc')
	pfblocker_get_countries();
if ($argv[1]=='cron'){
	require_once("/etc/inc/util.inc");
	require_once("/etc/inc/functions.inc");
	require_once("/etc/inc/pkg-utils.inc");
	require_once("/etc/inc/globals.inc");
	require_once("/etc/inc/filter.inc");
	require_once("/etc/inc/config.inc");
	$hour=date('H');
	$pfbdir='/usr/local/pkg/pfblocker';
	$updates=0;
	$cron=array('01hour' => 1,
				'04hours' => 4,
				'12hours' => 12,
				'EveryDay' => 23);
	
	if($config['installedpackages']['pfblockerlists']['config'] != "")
		foreach($config['installedpackages']['pfblockerlists']['config'] as $list){
			if (is_array($list['row']))
			  foreach ($list['row'] as $row){
			  	if ($row['url'] != "" && $hour > 0 ){
					$md5_url = md5($row['url']);
					$update_hour=(array_key_exists($list['cron'], $cron)?$cron[$list['cron']]:25);
					if($row['url'] && ($hour%$update_hour == 0)){
						print $update_hour." ".$pfbdir.'/'.$md5_url.'.txt'."\n";
						unlink_if_exists($pfbdir.'/'.$md5_url.'.txt');
						$updates++;
						}
			  		}
			  }
	}
	
	if ($updates > 0){
        include "/usr/local/pkg/pfblocker.inc";
        sync_package_pfblocker("cron");
		}
	}
	
function pfblocker_get_countries(){
$files= array (	"Africa" => "/usr/local/pkg/Africa_cidr.txt",
				"Asia" => "/usr/local/pkg/Asia_cidr.txt",
				"Europe" => "/usr/local/pkg/Europe_cidr.txt",
				"North America" => "/usr/local/pkg/North_America_cidr.txt",
				"Oceania" => "/usr/local/pkg/Oceania_cidr.txt",
				"South America"=>"/usr/local/pkg/South_America_cidr.txt");

$cdir='/usr/local/pkg/pfblocker';
if (! is_dir($cdir))
	mkdir ($cdir,0755);
foreach ($files as $cont => $file){
	$ips=file_get_contents($file);	
	$convert = explode("\n", $ips);
	print $cont."\n";
	$active= array("$cont" => '<active/>');
	$options="";
	$total=1;
	foreach ($convert as $line){
		if (preg_match('/#(.*):\s+(.*)$/',$line,$matches)){
			if ($ISOCode <> "" && $ISOCode <> $matches[2] && preg_match("/ISO Code/",$line)){
				file_put_contents($cdir.'/'.$ISOCode.'.txt',${$ISOCode},LOCK_EX);
				$total++;
				}
			${preg_replace("/\s/","",$matches[1])}=$matches[2];
			}
		else{
			if (${$ISOCode."c"}==""){
				${$ISOCode."c"}="ok";
			$options.= '<option><name>'.$Country .'-'.$ISOCode.' ('.$TotalNetworks.') '.' </name><value>'.$ISOCode.'</value></option>'."\n";
			}
			${$ISOCode}.=$line."\n";
		}
	}
#save last country networks
file_put_contents($cdir.'/'.$ISOCode.'.txt',${$ISOCode},LOCK_EX);
$cont_name= preg_replace("/ /","",$cont);
$cont_name_lower= strtolower($cont_name);
#file_put_contents($cdir.'/'.$cont_name.'.txt',$ips,LOCK_EX);
$xml= <<<EOF
<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE packagegui SYSTEM "./schema/packages.dtd">
<?xml-stylesheet type="text/xsl" href="./xsl/package.xsl"?>
<packagegui>
	<copyright>
	<![CDATA[
/* \$Id$ */
/* ========================================================================== */
/*
    pfblocker_{$cont_name}.xml
    part of the pfblocker package for pfSense
    Copyright (C) 2011 Marcello Coutinho
    All rights reserved.            
                                                                  */
/* ========================================================================== */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

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
/* ========================================================================== */
	]]>
	</copyright>
	<description>Describe your package here</description>
	<requirements>Describe your package requirements here</requirements>
	<faq>Currently there are no FAQ items provided.</faq>
	<name>pfblocker{$cont_name_lower}</name>
	<version>1.0</version>
	<title>Firewall: pfblocker</title>
	<include_file>/usr/local/pkg/pfblocker.inc</include_file>
	<menu>
		<name>pfBlocker</name>
		<tooltiptext>Configure pfblocker</tooltiptext>
		<section>Firewall</section>
		<url>pkg_edit.php?xml=pfblocker.xml&amp;id=0</url>
	</menu>
<tabs>
		<tab>
			<text>General</text>
			<url>/pkg_edit.php?xml=pfblocker.xml&amp;id=0</url>
		</tab>
		<tab>
			<text>Lists</text>
			<url>/pkg.php?xml=pfblocker_lists.xml</url>
		</tab>
		<tab>
			<text>Top Spammers</text>
			<url>/pkg_edit.php?xml=pfblocker_topspammers.xml&amp;id=0</url>
			{$active['top']}
		</tab>
		
		<tab>
			<text>Africa</text>
			<url>/pkg_edit.php?xml=pfblocker_Africa.xml&amp;id=0</url>
			{$active['Africa']}
		</tab>
		<tab>
			<text>Asia</text>
			<url>/pkg_edit.php?xml=pfblocker_Asia.xml&amp;id=0</url>
			{$active['Asia']}
		</tab>
		<tab>
			<text>Europe</text>
			<url>/pkg_edit.php?xml=pfblocker_Europe.xml&amp;id=0</url>
			{$active['Europe']}
		</tab>
		<tab>
			<text>North America</text>
			<url>/pkg_edit.php?xml=pfblocker_NorthAmerica.xml&amp;id=0</url>
			{$active['North America']}
		</tab>
		<tab>
			<text>Oceania</text>
			<url>/pkg_edit.php?xml=pfblocker_Oceania.xml&amp;id=0</url>
			{$active['Oceania']}
		</tab>
		<tab>
			<text>South America</text>
			<url>/pkg_edit.php?xml=pfblocker_SouthAmerica.xml&amp;id=0</url>
			{$active['South America']}
		</tab>
		<tab>
			<text>XMLRPC Sync</text>
			<url>/pkg_edit.php?xml=pfblocker_sync.xml&amp;id=0</url>
		</tab>
</tabs>
	<fields>
	<field>
		<name>Continent {$cont}</name>
		<type>listtopic</type>
	</field>
	<field>	
		<fielddescr>Countries</fielddescr>
		<fieldname>countries</fieldname>
		<description>
		<![CDATA[Select Countries you want to take an action.<br>
				<strong>Use CTRL + CLICK to unselect countries</strong>]]>
		</description>
		<type>select</type>
 			<options>
			{$options}
 			</options>
			<size>{$total}</size>
			<multiple/>
		</field>
		<field>
		<fielddescr>Action</fielddescr>
		<fieldname>action</fieldname>
		<description><![CDATA[Default:<strong>Disabled</strong><br>
						Select action for countries you have selected in {$cont}<br><br>
						<strong>Note: </strong><br>'Deny Both' - Will deny access on Both directions.<br>
								'Deny Inbound' - Will deny access from selected countries to your network.<br>
								'Deny Outbound' - Will deny access from your users to countries you selected to block.<br>
								'Permit Inbound' - Will allow access from selected countries to your network.<br>
								'Permit Outbound' - Will allow access from your users to countries you selected to block.<br>
								'Disabled' - Will just keep selection and do nothing to selected countries.<br>
								'Alias Only' - Will create alias <strong>pfBlocker{$cont}</strong> with selected countries to help custom rule assignments.<br><br>
								<strong>While creating rules with this alias, keep aliasname in the beggining of rule description and do not end description with 'rule'.<br></strong>
								Custom rules with 'Aliasname something rule' description will be removed by package.]]></description>
	    	<type>select</type>
 				<options>
				<option><name>Disabled</name><value>Disabled</value></option>
 				<option><name>Deny Inbound</name><value>Deny_Inbound</value></option>
				<option><name>Deny Outbound</name><value>Deny_Outbound</value></option>
				<option><name>Deny Both</name><value>Deny_Both</value></option>
				<option><name>Permit Inbound</name><value>Permit_Inbound</value></option>
				<option><name>Permit Outbound</name><value>Permit_Outbound</value></option>
				<option><name>Alias only</name><value>Alias_only</value></option>			
				</options>
			</field>
		</fields>
	<custom_php_install_command>
		pfblocker_php_install_command();
	</custom_php_install_command>
	<custom_php_deinstall_command>
		pfblocker_php_deinstall_command();
	</custom_php_deinstall_command>
	<custom_php_validation_command>
		pfblocker_validate_input(\$_POST, \$input_errors);
	</custom_php_validation_command>	
	<custom_php_resync_config_command>
		sync_package_pfblocker();
	</custom_php_resync_config_command>
</packagegui>
EOF;
	file_put_contents('/usr/local/pkg/pfblocker_'.$cont_name.'.xml',$xml,LOCK_EX);
	
}
	
}
?>
