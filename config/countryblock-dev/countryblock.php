<?php 
function get_networks($cb){
	if ($cb==1)
		$return= file_get_contents('/usr/local/pkg/cb.txt');
	if ($cb==2)
		$return=file_get_contents('/usr/local/pkg/cbw.txt');
		#print "<pre>";
		print $return;
}

if ($_REQUEST['cb']== 1){# and $_SERVER['REMOTE_ADDR']== '127.0.0.1'){
	get_networks(1);
}
if ($_REQUEST['cbw']== 1){# and $_SERVER['REMOTE_ADDR']== '127.0.0.1'){
	get_networks(2);
}

function countryblock_get_countries(){
$files= array (	"Africa" => "/usr/local/pkg/Africa_cidr.txt",
				"Antartica" => "/usr/local/pkg/Antartica_cidr.txt",
				"Asia" => "/usr/local/pkg/Asia_cidr.txt",
				"Europe" => "/usr/local/pkg/Europe_cidr.txt",
				"North America" => "/usr/local/pkg/North_America_cidr.txt",
				"Oceania" => "/usr/local/pkg/Oceania_cidr.txt",
				"South America"=>"/usr/local/pkg/South_America_cidr.txt");
$cdir='/usr/local/pkg/countryblock';
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
			if (${$ISOCode}==0){
				${$ISOCode}++;
			$options.= '<option><name>'.$Country.' </name><value>'.$ISOCode.'</value></option>'."\n";
			}
			${$ISOCode}.=$line."\n";
		}
	}
$cont_name= preg_replace("/ /","",$cont);
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
    countryblock_{$cont_name}.xml
    part of the Countryblock package for pfSense
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
	<name>countryblock_{$cont_name}</name>
	<version>1.0</version>
	<title>Firewall: Countryblock</title>
	<include_file>/usr/local/pkg/countryblock.inc</include_file>
	<menu>
		<name>Countryblock</name>
		<tooltiptext>Configure Countryblock</tooltiptext>
		<section>Firewall</section>
		<url>pkg_edit.php?xml=countryblock.xml&amp;id=0</url>
	</menu>
	<service>
		<name>countryblock</name>
	</service>
<tabs>
		<tab>
			<text>General</text>
			<url>/pkg_edit.php?xml=countryblock.xml&amp;id=0</url>
		</tab>
		<tab>
			<text>Africa</text>
			<url>/pkg_edit.php?xml=countryblock_Africa.xml&amp;id=0</url>
			{$active['Africa']}
		</tab>
		<tab>
			<text>Antartica</text>
			<url>/pkg_edit.php?xml=countryblock_Antartica.xml&amp;id=0</url>
			{$active['Antartica']}
		</tab>
		<tab>
			<text>Asia</text>
			<url>/pkg_edit.php?xml=countryblock_Asia.xml&amp;id=0</url>
			{$active['Asia']}
		</tab>
		<tab>
			<text>Europe</text>
			<url>/pkg_edit.php?xml=countryblock_Europe.xml&amp;id=0</url>
			{$active['Europe']}
		</tab>
		<tab>
			<text>North America</text>
			<url>/pkg_edit.php?xml=countryblock_NorthAmerica.xml&amp;id=0</url>
			{$active['North America']}
		</tab>
		<tab>
			<text>Oceania</text>
			<url>/pkg_edit.php?xml=countryblock_Oceania.xml&amp;id=0</url>
			{$active['Oceania']}
		</tab>
		<tab>
			<text>South America</text>
			<url>/pkg_edit.php?xml=countryblock_SouthAmerica.xml&amp;id=0</url>
			{$active['South America']}
		</tab>
		<tab>
			<text>XMLRPC Sync</text>
			<url>/pkg_edit.php?xml=countryblock_sync.xml&amp;id=0</url>
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
			<![CDATA[Select Countries you want to block.]]>
			</description>
	    	<type>select</type>
 				<options>
				{$options}
 				</options>
				<size>{$total}</size>
				<multiple/>
		</field>	</fields>
	<custom_php_install_command>
		countryblock_php_install_command();
	</custom_php_install_command>
	<custom_php_deinstall_command>
		countryblock_php_deinstall_command();
	</custom_php_deinstall_command>
	<custom_php_validation_command>
		countryblock_validate_input(\$_POST, &amp;\$input_errors);
	</custom_php_validation_command>	
	<custom_php_resync_config_command>
		sync_package_countryblock();
	</custom_php_resync_config_command>
</packagegui>
EOF;
	file_put_contents('/usr/local/pkg/countryblock_'.$cont_name.'.xml',$xml,LOCK_EX);
	
	#var_dump($ips);

}
	
}
?>