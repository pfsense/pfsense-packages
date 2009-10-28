<?php
/*
    amanda.php
    Copyright (C) 2008, 2009 Robert Zelaya
    All rights reserved.

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

require("filter.inc");


/* create snort.xml for every interface selected */
function create_snort_xml() 
{
include("filter.inc");
include("config.inc");

        global $bconfig, $bg;

        conf_mount_rw();

        $first = 0;
        $snortInterfaces = array(); /* -gtm  */
        
        $if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface) {
                        $if = convert_friendly_interface_to_real_interface_name($iface);
                        
                        if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
                                $if = "ng0";
                        }
              
                        /* build a list of user specified interfaces -gtm */      
                        if($if){      
                          array_push($snortInterfaces, $if);
                          $first = 1;
                        }
                }       
            
                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }


        foreach($snortInterfaces as $snortIf)
        {

$snort_xml_text = <<<EOD
<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE packagegui SYSTEM "../schema/packages.dtd">
<?xml-stylesheet type="text/xsl" href="../xsl/package.xsl"?>
<packagegui>
        <copyright>
        <![CDATA[
/* \$Id\$ */
/* ========================================================================== */
/*
    authng.xml
    part of pfSense (http://www.pfsense.com)
    Copyright (C) 2007 Robert Zelaya
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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
        <name>Snort{$snortIf}</name>
        <version>2.8.4.1_5</version>
        <title>Services: Snort 2.8.4.1_5 pkg v. 1.6 {$snortIf}</title>
        <include_file>/usr/local/pkg/snort.inc</include_file>
        <tabs>
                <tab>
                        <text>Snort Interfaces</text>
                        <url>/snort_interfaces.php</url>
                </tab>
                <tab>
                        <text>Settings</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&amp;id=0</url>
                        <active/>
                </tab>
                <tab>
                        <text>Categories</text>
                        <url>snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php</url>
                </tab>
                <tab>
                        <text>Rules</text>
                        <url>snort/snort_{$snortIf}/snort_rules_{$snortIf}.php</url>
                </tab>
                <tab>
                        <text>Servers</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0</url>
                </tab>
                <tab>
                        <text>Threshold</text>
                        <url>/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml</url>
                </tab>
                <tab>
                        <text>Barnyard2</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml&amp;id=0</url>
                </tab>
        </tabs>
        <fields>
                <field>
                        <fielddescr>Interface</fielddescr>
                        <fieldname>iface_array</fieldname>
                        <description>Select the interface(s) Snort will listen on.</description>
                        <type>interfaces_selection</type>
                        <size>3</size>
                        <value>lan</value>
                        <multiple>true</multiple>
                </field>
                <field>
                        <fielddescr>Memory Performance</fielddescr>
                        <fieldname>performance</fieldname>
                        <description>Lowmem and ac-bnfa are recommended for low end systems, Ac: high memory, best performance, ac-std: moderate memory,high performance, acs: small memory, moderateperformance, ac-banded: small 
memory,moderate performance, ac-sparsebands: small memory, high performance.</description>
                        <type>select</type>
                        <options>
                                <option>
                                        <name>ac-bnfa</name>
                                        <value>ac-bnfa</value>
                                </option>
                                <option>
                                        <name>lowmem</name>
                                        <value>lowmem</value>
                                </option>
                                <option>
                                        <name>ac-std</name>
                                        <value>ac-std</value>
                                </option>
                                <option>
                                        <name>ac</name>
                                        <value>ac</value>
                                </option>
                                <option>
                                        <name>ac-banded</name>
                                        <value>ac-banded</value>
                                </option>
                                <option>
                                        <name>ac-sparsebands</name>
                                        <value>ac-sparsebands</value>
                                </option>
                                <option>
                                        <name>acs</name>
                                        <value>acs</value>
                                </option>
                        </options>
                </field>
                <field>
                        <fielddescr>BPF Buffer size</fielddescr>
                        <fieldname>bpfbufsize</fieldname>
                        <description>Changing this option adjusts the system BPF buffer size.  Leave blank if you do not know what this does. Default is 1024.</description>
                        <type>input</type>
                </field>
                <field>
                        <fielddescr>Maximum BPF buffer size</fielddescr>
                        <fieldname>bpfmaxbufsize</fieldname>
                        <description>Changing this option adjusts the system maximum BPF buffer size.  Leave blank if you do not know what this does. Default is 524288. This value should never be set above hardware cache size. The 
best (optimal size) is 50% - 80% of the hardware cache size.</description>
                        <type>input</type>
                </field>
                <field>
                        <fielddescr>Maximum BPF inserts</fielddescr>
                        <fieldname>bpfmaxinsns</fieldname>
                        <description>Changing this option adjusts the system maximum BPF insert size.  Leave blank if you do not know what this does. Default is 512.</description>
                        <type>input</type>
                </field>
                <field>
                        <fielddescr>Advanced configuration pass through</fielddescr>
                        <fieldname>configpassthru</fieldname>
                        <description>Add items to here will be automatically inserted into the running snort configuration</description>
                        <type>textarea</type>
                        <cols>40</cols>
                        <rows>5</rows>
                </field>
                <field>
                        <fielddescr>Snort signature info files.</fielddescr>
                        <fieldname>signatureinfo</fieldname>
                        <description>Snort signature info files will be installed during updates. At leats 500 mb of memory is needed.</description>
                        <type>checkbox</type>
                </field>
                <field>
                        <fielddescr>Alerts Tab logging type.</fielddescr>
                        <fieldname>snortalertlogtype</fieldname>
                        <description>Please choose the type of Alert logging you will like see in the Alerts Tab. The options are Full descriptions or Fast short descriptions</description>
                        <type>select</type>
                        <options>
                                <option>
                                        <name>fast</name>
                                        <value>fast</value>
                                </option>
                                <option>
                                        <name>full</name>
                                        <value>full</value>
                                </option>
                        </options>
                </field>
                <field>
                        <fielddescr>Send alerts to main System logs.</fielddescr>
                        <fieldname>alertsystemlog</fieldname>
                        <description>Snort will send Alerts to the Pfsense system logs.</description>
                        <type>checkbox</type>
                </field>
                <field>
                        <fielddescr>Log to a Tcpdump file.</fielddescr>
                        <fieldname>tcpdumplog</fieldname>
                        <description>Snort will log packets to a tcpdump-formatted file. The file then can be analyzed by a wireshark type of application. WARNING: File may become large.</description>
                        <type>checkbox</type>
                </field>
    </fields>
        <custom_php_deinstall_command>
                snort_deinstall();
        </custom_php_deinstall_command>
</packagegui>

EOD;

/* write out snort_xml */
$bconf = fopen("/usr/local/pkg/snort/snort_{$snortIf}/snort_{$snortIf}.xml", "w");
if(!$bconf)
{
        log_error("Could not open /usr/local/pkg/snort/snort_{$snortIf}/snort_{$snortIf}.xml for writing.");
        exit;
        }
        fwrite($bconf, $snort_xml_text);
         fclose($bconf);

        conf_mount_ro();

        }
}

/* create barnyard2.xml for every interface selected */
function create_snort_barnyard2_xml()
{
include("filter.inc");
include("config.inc");

        global $bconfig, $bg;

        conf_mount_rw();

        $first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface) {
                        $if = convert_friendly_interface_to_real_interface_name($iface);

                        if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
                                $if = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if){
                          array_push($snortInterfaces, $if);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }


        foreach($snortInterfaces as $snortIf)
        {

$snort_barnyard2_text = <<<EOD
<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE packagegui SYSTEM "../schema/packages.dtd">
<?xml-stylesheet type="text/xsl" href="../xsl/package.xsl"?>
<packagegui>
        <copyright>
        <![CDATA[
/* \$Id\$ */
/* ========================================================================== */
/*
    authng.xml
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2007 Robert Zelaya
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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
        <name>barnyard2{$snortIf}</name>
        <version>none</version>
        <title>Services: Barnyard2 {$snortIf}</title>
        <include_file>/usr/local/pkg/snort.inc</include_file>
        <tabs>
                <tab>
                        <text>Snort Interfaces</text>
                        <url>/snort_interfaces.php</url>
                </tab>
                <tab>
                        <text>Settings</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&amp;id=0</url>
                </tab>
                <tab>
                        <text>Categories</text>
                        <url>snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php</url>
                </tab>
                <tab>
                        <text>Rules</text>
                        <url>snort/snort_{$snortIf}/snort_rules_{$snortIf}.php</url>
                </tab>
                <tab>
                        <text>Servers</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0</url>
                </tab>
                <tab>
                        <text>Threshold</text>
                        <url>/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml</url>
                </tab>
                <tab>
                        <text>Barnyard2</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml&amp;id=0</url>
                        <active/>
                </tab>
        </tabs>
        <fields>
        <field>
            <fielddescr>Enable Barnyard2.</fielddescr>
            <fieldname>snortbarnyardlog</fieldname>
            <description>This will enable barnyard2 in the snort package. You will also have to set the database credentials.</description>
            <type>checkbox</type>
        </field>
                <field>
                        <fielddescr>Barnyard2 Log Mysql Database.</fielddescr>
                        <fieldname>snortbarnyardlog_database</fieldname>
                        <description>Example: output database: log, mysql, dbname=snort user=snort host=localhost password=xyz</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Barnyard2 Configure Hostname ID.</fielddescr>
                        <fieldname>snortbarnyardlog_hostname</fieldname>
                        <description>Example: pfsense.local</description>
                        <type>input</type>
                        <size>25</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Barnyard2 Configure Interface ID</fielddescr>
                        <fieldname>snortbarnyardlog_interface</fieldname>
                        <description>Example: vr0</description>
                        <type>input</type>
                        <size>25</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Log Alerts to a snort unified2 file.</fielddescr>
                        <fieldname>snortunifiedlog</fieldname>
                        <description>Snort will log Alerts to a file in the UNIFIED2 format. This is a requirement for barnyard2.</description>
                        <type>checkbox</type>
                </field>                
    </fields>
        <custom_php_deinstall_command>
                snort_advanced();
        </custom_php_deinstall_command>
</packagegui>

EOD;

/* write out snort_barnyard2_xml */
$bconf = fopen("/usr/local/pkg/snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml", "w");
if(!$bconf)
{
        log_error("Could not open /usr/local/pkg/snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml for writing.");
        exit;
        }
        fwrite($bconf, $snort_barnyard2_text);
         fclose($bconf);

        conf_mount_ro();

        }
}


/* create snort_define_servers.xml for every interface selected */
function create_snort_define_servers_xml()
{
include("filter.inc");
include("config.inc");

        global $bconfig, $bg;

        conf_mount_rw();

        $first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface) {
                        $if = convert_friendly_interface_to_real_interface_name($iface);

                        if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
                                $if = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if){
                          array_push($snortInterfaces, $if);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }


        foreach($snortInterfaces as $snortIf)
        {

$snort_define_servers_xml_text = <<<EOD
<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE packagegui SYSTEM "../schema/packages.dtd">
<?xml-stylesheet type="text/xsl" href="../xsl/package.xsl"?>
<packagegui>
        <copyright>
        <![CDATA[
/* \$Id\$ */
/* ========================================================================== */
/*
    authng.xml
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2007 Robert Zelaya
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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
        <name>SnortDefServers{$snortIf}</name>
        <version>none</version>
        <title>Services: Snort Define Servers {$snortIf}</title>
        <include_file>/usr/local/pkg/snort.inc</include_file>
        <tabs>
                <tab>
                        <text>Snort Interfaces</text>
                        <url>/snort_interfaces.php</url>
                </tab>
                <tab>
                        <text>Settings</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&amp;id=0</url>
                </tab>
                <tab>
                        <text>Categories</text>
                        <url>snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php</url>
                </tab>
                <tab>
                        <text>Rules</text>
                        <url>snort/snort_{$snortIf}/snort_rules_{$snortIf}.php</url>
                </tab>
                <tab>
                        <text>Servers</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0</url>
                        <active/>
                </tab>
                <tab>
                        <text>Threshold</text>
                        <url>/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml</url>
                </tab>
                <tab>
                        <text>Barnyard2</text>
                        <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml&amp;id=0</url>
                </tab>
        </tabs>
        <fields>
                <field>
                        <fielddescr>Define DNS_SERVERS</fielddescr>
                        <fieldname>def_dns_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define DNS_PORTS</fielddescr>
                        <fieldname>def_dns_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 53.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SMTP_SERVERS</fielddescr>
                        <fieldname>def_smtp_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SMTP_PORTS</fielddescr>
                        <fieldname>def_smtp_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 25.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define Mail_Ports</fielddescr>
                        <fieldname>def_mail_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 25,143,465,691.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define HTTP_SERVERS</fielddescr>
                        <fieldname>def_http_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define WWW_SERVERS</fielddescr>
                        <fieldname>def_www_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define HTTP_PORTS</fielddescr>
                        <fieldname>def_http_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 80.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                        <field>
                        <fielddescr>Define SQL_SERVERS</fielddescr>
                        <fieldname>def_sql_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define ORACLE_PORTS</fielddescr>
                        <fieldname>def_oracle_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 1521.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define MSSQL_PORTS</fielddescr>
                        <fieldname>def_mssql_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 1433.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define TELNET_SERVERS</fielddescr>
                        <fieldname>def_telnet_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define TELNET_PORTS</fielddescr>
                        <fieldname>def_telnet_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 23.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SNMP_SERVERS</fielddescr>
                        <fieldname>def_snmp_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SNMP_PORTS</fielddescr>
                        <fieldname>def_snmp_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 161.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define FTP_SERVERS</fielddescr>
                        <fieldname>def_ftp_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define FTP_PORTS</fielddescr>
                        <fieldname>def_ftp_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 21.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SSH_SERVERS</fielddescr>
                        <fieldname>def_ssh_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SSH_PORTS</fielddescr>
                        <fieldname>def_ssh_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is Pfsense SSH port.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define POP_SERVERS</fielddescr>
                        <fieldname>def_pop_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define POP2_PORTS</fielddescr>
                        <fieldname>def_pop2_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 109.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define POP3_PORTS</fielddescr>
                        <fieldname>def_pop3_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 110.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define IMAP_SERVERS</fielddescr>
                        <fieldname>def_imap_servers</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define IMAP_PORTS</fielddescr>
                        <fieldname>def_imap_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 143.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SIP_PROXY_IP</fielddescr>
                        <fieldname>def_sip_proxy_ip</fieldname>
                        <description>Example: "192.168.1.3/24,192.168.1.4/24". Leave blank to scan all networks.</description>
                        <type>input</type>
                        <size>101</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SIP_PROXY_PORTS</fielddescr>
                        <fieldname>def_sip_proxy_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 5060:5090,16384:32768.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define AUTH_PORTS</fielddescr>
                        <fieldname>def_auth_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 113.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define FINGER_PORTS</fielddescr>
                        <fieldname>def_finger_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 79.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define IRC_PORTS</fielddescr>
                        <fieldname>def_irc_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 6665,6666,6667,6668,6669,7000.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define NNTP_PORTS</fielddescr>
                        <fieldname>def_nntp_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 119.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define RLOGIN_PORTS</fielddescr>
                        <fieldname>def_rlogin_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 513.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define RSH_PORTS</fielddescr>
                        <fieldname>def_rsh_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 514.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
                <field>
                        <fielddescr>Define SSL_PORTS</fielddescr>
                        <fieldname>def_ssl_ports</fieldname>
                        <description>Example: Specific ports "25,443" or All ports betwen "5060:5090". Default is 25,443,465,636,993,995.</description>
                        <type>input</type>
                        <size>43</size>
                        <value></value>
                </field>
    </fields>
        <custom_php_deinstall_command>
                snort_define_servers();
        </custom_php_deinstall_command>
</packagegui>

EOD;

/* write out snort_define_servers_xml */
$bconf = fopen("/usr/local/pkg/snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml", "w");
if(!$bconf)
{
        log_error("Could not open /usr/local/pkg/snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml for writing.");
        exit;
        }
        fwrite($bconf, $snort_define_servers_xml_text);
         fclose($bconf);

        conf_mount_ro();

        }
}

/* create snort_threshold.xml for every interface selected */
function create_snort_threshold_xml()
{
include("filter.inc");
include("config.inc");

        global $bconfig, $bg;

        conf_mount_rw();

        $first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface) {
                        $if = convert_friendly_interface_to_real_interface_name($iface);

                        if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
                                $if = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if){
                          array_push($snortInterfaces, $if);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }


        foreach($snortInterfaces as $snortIf)
        {

$snort_threshold_xml_text = <<<EOD
<?xml version="1.0" encoding="utf-8" ?>
<!DOCTYPE packagegui SYSTEM "../schema/packages.dtd">
<?xml-stylesheet type="text/xsl" href="../xsl/package.xsl"?>
<packagegui>
        <copyright>
        <![CDATA[
/* \$Id\$ */
/* ========================================================================== */
/*
    authng.xml
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2004, 2005 Scott Robert Zelaya
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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
	<name>snort-threshold{$snortIf}</name>
	<version>0.1.0</version>
	<title>Snort: Alert Thresholding and Suppression {$snortIf}</title>
	<include_file>/usr/local/pkg/snort.inc</include_file>
	<!-- Menu is where this packages menu will appear -->
	<tabs>
            <tab>
                    <text>Snort Interfaces</text>
                    <url>/snort_interfaces.php</url>
            </tab>
            <tab>
                    <text>Settings</text>
                    <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&amp;id=0</url>
            </tab>
            <tab>
                    <text>Categories</text>
                    <url>snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php</url>
            </tab>
            <tab>
                    <text>Rules</text>
                    <url>snort/snort_{$snortIf}/snort_rules_{$snortIf}.php</url>
            </tab>
            <tab>
                    <text>Servers</text>
                    <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0</url>
            </tab>
            <tab>
                    <text>Threshold</text>
                    <url>/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml</url>
					<active/>
            </tab>
             <tab>
                    <text>Barnyard2</text>
                    <url>/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml&amp;id=0</url>
             </tab>	
	</tabs>
	<adddeleteeditpagefields>
		<columnitem>
			<fielddescr>Thresholding or Suppression Rule</fielddescr>
			<fieldname>threshrule</fieldname>
		</columnitem>
		<columnitem>
			<fielddescr>Description</fielddescr>
			<fieldname>description</fieldname>
		</columnitem>
	</adddeleteeditpagefields>
	<fields>
		<field>
			<fielddescr>Thresholding or Suppression Rule</fielddescr>
			<fieldname>threshrule</fieldname>
			<description>Enter the Rule. Example; "suppress gen_id 125, sig_id 4" or "threshold gen_id 1, sig_id 1851, type limit, track by_src, count 1, seconds 60"</description>
			<type>input</type>
			<size>40</size>
		</field>
		<field>
			<fielddescr>Description</fielddescr>
			<fieldname>description</fieldname>
			<description>Enter the description for this item</description>
			<type>input</type>
			<size>60</size>
		</field>
	</fields>
	<custom_php_command_before_form>
	</custom_php_command_before_form>
	<custom_delete_php_command>
	</custom_delete_php_command>
	<custom_php_resync_config_command>
		create_snort_conf();
	</custom_php_resync_config_command>
</packagegui>

EOD;

/* write out snort_threshold_xml */
$bconf = fopen("/usr/local/pkg/snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml", "w");
if(!$bconf)
{
        log_error("Could not open /usr/local/pkg/snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml for writing.");
        exit;
        }
        fwrite($bconf, $snort_threshold_xml_text);
         fclose($bconf);

        conf_mount_ro();

        }
}

/* create snort_rules.php for every interface selected */
function create_snort_rules_php()
{
include("filter.inc");
include("config.inc");

        global $bconfig, $bg;

        conf_mount_rw();

        $first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface) {
                        $if = convert_friendly_interface_to_real_interface_name($iface);

                        if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
                                $if = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if){
                          array_push($snortInterfaces, $if);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }


        foreach($snortInterfaces as $snortIf)
        {

$snort_rules_php_text = <<<EOD
<?php
/* \$Id\$ */
/*
    edit_snortrule.php
    Copyright (C) 2004, 2005 Scott Ullrich
	Copyright (C) 2004, 2005 Scott Robert Zelaya
    All rights reserved.

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
require("guiconfig.inc");
require("config.inc");

if(!is_dir("/usr/local/etc/snort_{$snortIf}/rules_{$snortIf}")) {
	conf_mount_rw();
	exec('mkdir /usr/local/etc/snort_{$snortIf}/rules_{$snortIf}/');
	conf_mount_ro();
}

/* Check if the rules dir is empy if so warn the user */
/* TODO give the user the option to delete the installed rules rules */
\$isrulesfolderempty = exec('ls -A /usr/local/etc/snort_{$snortIf}/rules_{$snortIf}/*.rules');
if (\$isrulesfolderempty == "") {

include("head.inc");
include("fbegin.inc");

echo "<body link=\"#000000\" vlink=\"#000000\" alink=\"#000000\">";

echo "<script src=\"/row_toggle.js\" type=\"text/javascript\"></script>\n
<script src=\"/javascript/sorttable.js\" type=\"text/javascript\"></script>\n
<table width=\"99%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
   <tr>\n
   		<td>\n";

	\$tab_array = array();
	\$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&id=0");
	\$tab_array[] = array(gettext("Categories"), false, "snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php");
	\$tab_array[] = array(gettext("Rules"), true, "snort/snort_{$snortIf}/snort_rules_{$snortIf}.php");
	\$tab_array[] = array(gettext("Servers"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0");
	\$tab_array[] = array(gettext("Threshold"), false, "/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml");
	\$tab_array[] = array(gettext("Barnyard2"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_barnyard2_{$snortIf}.xml&id=0");
	display_top_tabs(\$tab_array);

echo  		"</td>\n
  </tr>\n
  <tr>\n
    <td>\n
		<div id=\"mainarea\">\n
			<table id=\"maintable\" class=\"tabcont\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
				<tr>\n
					<td>\n
# The rules directory is empty.\n
		    		</td>\n
		  		</tr>\n
			</table>\n
		</div>\n
	</td>\n
  </tr>\n
</table>\n
\n
</form>\n
\n
<p>\n\n";

echo "Please click on the Update Rules tab to install your selected rule sets.";
include("fend.inc");

echo "</body>";
echo "</html>";

exit(0);

}

function get_middle(\$source, \$beginning, \$ending, \$init_pos) {
   \$beginning_pos = strpos(\$source, \$beginning, \$init_pos);
   \$middle_pos = \$beginning_pos + strlen(\$beginning);
   \$ending_pos = strpos(\$source, \$ending, \$beginning_pos);
   \$middle = substr(\$source, \$middle_pos, \$ending_pos - \$middle_pos);
   return \$middle;
}

function write_rule_file(\$content_changed, \$received_file)
{
    conf_mount_rw();

    //read snort file with writing enabled
    \$filehandle = fopen(\$received_file, "w");

    //delimiter for each new rule is a new line
    \$delimiter = "\n";

    //implode the array back into a string for writing purposes
    \$fullfile = implode(\$delimiter, \$content_changed);

    //write data to file
    fwrite(\$filehandle, \$fullfile);

    //close file handle
    fclose(\$filehandle);

    conf_mount_rw();
}

function load_rule_file(\$incoming_file)
{

    //read snort file
    \$filehandle = fopen(\$incoming_file, "r");

    //read file into string, and get filesize
    \$contents = fread(\$filehandle, filesize(\$incoming_file));

    //close handler
    fclose (\$filehandle);

    //string for populating category select
    \$currentruleset = substr(\$file, 27);

    //delimiter for each new rule is a new line
    \$delimiter = "\n";

    //split the contents of the string file into an array using the delimiter
    \$splitcontents = explode(\$delimiter, \$contents);

    return \$splitcontents;

}

\$ruledir = "/usr/local/etc/snort_{$snortIf}/rules_{$snortIf}/";
\$dh  = opendir(\$ruledir);

\$message_reload = "The Snort rule configuration has been changed.<br>You must apply the changes in order for them to take effect.";

while (false !== (\$filename = readdir(\$dh)))
{
    //only populate this array if its a rule file
    \$isrulefile = strstr(\$filename, ".rules");
    if (\$isrulefile !== false)
    {
        \$files[] = \$filename;
    }
}

sort(\$files);

if (\$_GET['openruleset'])
{
    \$file = \$_GET['openruleset'];
}
else
{
    \$file = \$ruledir.\$files[0];

}

//Load the rule file
\$splitcontents = load_rule_file(\$file);

if (\$_POST)
{
	if (!\$_POST['apply']) {
	    //retrieve POST data
	    \$post_lineid = \$_POST['lineid'];
	    \$post_enabled = \$_POST['enabled'];
	    \$post_src = \$_POST['src'];
	    \$post_srcport = \$_POST['srcport'];
	    \$post_dest = \$_POST['dest'];
	    \$post_destport = \$_POST['destport'];
	
		//clean up any white spaces insert by accident
		\$post_src = str_replace(" ", "", \$post_src);
		\$post_srcport = str_replace(" ", "", \$post_srcport);
		\$post_dest = str_replace(" ", "", \$post_dest);
		\$post_destport = str_replace(" ", "", \$post_destport);
	
	    //copy rule contents from array into string
	    \$tempstring = \$splitcontents[\$post_lineid];
	
	    //search string
	    \$findme = "# alert"; //find string for disabled alerts
	
	    //find if alert is disabled
	    \$disabled = strstr(\$tempstring, \$findme);
	
	    //if find alert is false, then rule is disabled
	    if (\$disabled !== false)
	    {
	        //has rule been enabled
	        if (\$post_enabled == "yes")
	        {
	            //move counter up 1, so we do not retrieve the # in the rule_content array
	            \$tempstring = str_replace("# alert", "alert", \$tempstring);
	            \$counter2 = 1;
	        }
	        else
	        {
	            //rule is staying disabled
	            \$counter2 = 2;
	        }
	    }
	    else
	    {
	        //has rule been disabled
	        if (\$post_enabled != "yes")
	        {
	            //move counter up 1, so we do not retrieve the # in the rule_content array
	            \$tempstring = str_replace("alert", "# alert", \$tempstring);
	            \$counter2 = 2;
	        }
	        else
	        {
	            //rule is staying enabled
	            \$counter2 = 1;
	        }
	    }
	
	    //explode rule contents into an array, (delimiter is space)
	    \$rule_content = explode(' ', \$tempstring);
	
		//insert new values
	    \$counter2++;
	    \$rule_content[\$counter2] = \$post_src;//source location
	    \$counter2++;
	    \$rule_content[\$counter2] = \$post_srcport;//source port location
	    \$counter2 = \$counter2+2;
	    \$rule_content[\$counter2] = \$post_dest;//destination location
	    \$counter2++;
	    \$rule_content[\$counter2] = \$post_destport;//destination port location
	
		//implode the array back into string
		\$tempstring = implode(' ', \$rule_content);
	
		//copy string into file array for writing
	    \$splitcontents[\$post_lineid] = \$tempstring;
	
	    //write the new .rules file
	    write_rule_file(\$splitcontents, \$file);
	
	    //once file has been written, reload file
	    \$splitcontents = load_rule_file(\$file);
	    
	    \$stopMsg = true;
	}
	
	if (\$_POST['apply']) {
//		stop_service("snort");
//		sleep(2);
//		start_service("snort");
		\$savemsg = "The snort rules selections have been saved. Please restart snort by clicking save on the settings tab.";
		\$stopMsg = false;
	}

}
else if (\$_GET['act'] == "toggle")
{
    \$toggleid = \$_GET['id'];

    //copy rule contents from array into string
    \$tempstring = \$splitcontents[\$toggleid];

    //explode rule contents into an array, (delimiter is space)
    \$rule_content = explode(' ', \$tempstring);

    //search string
    \$findme = "# alert"; //find string for disabled alerts

    //find if alert is disabled
    \$disabled = strstr(\$tempstring, \$findme);

    //if find alert is false, then rule is disabled
    if (\$disabled !== false)
    {
        //rule has been enabled
        //move counter up 1, so we do not retrieve the # in the rule_content array
        \$tempstring = str_replace("# alert", "alert", \$tempstring);

    }
    else
    {
        //has rule been disabled
        //move counter up 1, so we do not retrieve the # in the rule_content array
        \$tempstring = str_replace("alert", "# alert", \$tempstring);

    }

    //copy string into array for writing
    \$splitcontents[\$toggleid] = \$tempstring;

    //write the new .rules file
    write_rule_file(\$splitcontents, \$file);

    //once file has been written, reload file
    \$splitcontents = load_rule_file(\$file);
    
    \$stopMsg = true;
	
		//write disable/enable sid to config.xml
		if (\$disabled == false) {
            \$string_sid = strstr(\$tempstring, 'sid:');
            \$sid_pieces = explode(";", \$string_sid);
            \$sid_off_cut = \$sid_pieces[0];
            // sid being turned off
            \$sid_off  = str_replace("sid:", "", \$sid_off_cut);
			// rule_sid_on registers
			\$sid_on_pieces = \$config['installedpackages']['snort']['rule_sid_on'];
			// if off sid is the same as on sid remove it
			\$sid_on_old = str_replace("||enablesid \$sid_off", "", "\$sid_on_pieces");
			// write the replace sid back as empty
			\$config['installedpackages']['snort']['rule_sid_on'] = \$sid_on_old;
			// rule sid off registers
			\$sid_off_pieces = \$config['installedpackages']['snort']['rule_sid_off'];
			// if off sid is the same as off sid remove it
			\$sid_off_old = str_replace("||disablesid \$sid_off", "", "\$sid_off_pieces");
			// write the replace sid back as empty
			\$config['installedpackages']['snort']['rule_sid_off'] = \$sid_off_old;
			// add sid off registers to new off sid
			\$config['installedpackages']['snort']['rule_sid_off'] = "||disablesid \$sid_off" . \$config['installedpackages']['snort']['rule_sid_off'];
			write_config();
		}
		else
		{
            \$string_sid = strstr(\$tempstring, 'sid:');
            \$sid_pieces = explode(";", \$string_sid);
            \$sid_on_cut = \$sid_pieces[0];
            // sid being turned off
            \$sid_on  = str_replace("sid:", "", \$sid_on_cut);
			// rule_sid_off registers
			\$sid_off_pieces = \$config['installedpackages']['snort']['rule_sid_off'];
			// if off sid is the same as on sid remove it
			\$sid_off_old = str_replace("||disablesid \$sid_on", "", "\$sid_off_pieces");
			// write the replace sid back as empty
			\$config['installedpackages']['snort']['rule_sid_off'] = \$sid_off_old;
			// rule sid on registers
			\$sid_on_pieces = \$config['installedpackages']['snort']['rule_sid_on'];
			// if on sid is the same as on sid remove it
			\$sid_on_old = str_replace("||enablesid \$sid_on", "", "\$sid_on_pieces");
			// write the replace sid back as empty
			\$config['installedpackages']['snort']['rule_sid_on'] = \$sid_on_old;
			// add sid on registers to new on sid
			\$config['installedpackages']['snort']['rule_sid_on'] = "||enablesid \$sid_on" . \$config['installedpackages']['snort']['rule_sid_on'];
			write_config();
		}
	
}


\$pgtitle = "Snort: Rules";
require("guiconfig.inc");
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<?php
if(!\$pgtitle_output)
	echo "<p class=\"pgtitle\"><?=\$pgtitle?></p>";
?>
<form action="snort_rules.php" method="post" name="iform" id="iform">
<?php if (\$savemsg){print_info_box(\$savemsg);} else if (\$stopMsg){print_info_box_np(\$message_reload);}?>
<br>
</form>
<script type="text/javascript" language="javascript" src="row_toggle.js">
    <script src="/javascript/sorttable.js" type="text/javascript">
</script>

<script language="javascript" type="text/javascript">
<!--
function go()
{
    var agt=navigator.userAgent.toLowerCase();
    if (agt.indexOf("msie") != -1) {
        box = document.forms.selectbox;
    } else {
        box = document.forms[1].selectbox;
	}
    destination = box.options[box.selectedIndex].value;
    if (destination) 
		location.href = destination;
}
// -->
</script>

<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr>
      <td>
<?php
    \$tab_array = array();
    \$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&id=0");
    \$tab_array[] = array(gettext("Update Rules"), false, "/snort_download_rules.php");
    \$tab_array[] = array(gettext("Categories"), false, "snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php");
    \$tab_array[] = array(gettext("Rules"), true, "snort/snort_{$snortIf}/snort_rules_{$snortIf}.php");
	\$tab_array[] = array(gettext("Servers"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0");
    \$tab_array[] = array(gettext("Blocked"), false, "/snort_blocked.php");
    \$tab_array[] = array(gettext("Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
	\$tab_array[] = array(gettext("Threshold"), false, "/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml");
    \$tab_array[] = array(gettext("Alerts"), false, "/snort_alerts.php");
    \$tab_array[] = array(gettext("Advanced"), false, "/pkg_edit.php?xml=snort_advanced.xml&id=0");
    display_top_tabs(\$tab_array);
?>
      </td>
  </tr>
  <tr>
    <td>
        <div id="mainarea">
            <table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                    <td>
                        <table id="ruletable1" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
                            <tr id="frheader">
                                    <td width="3%" class="list">&nbsp;</td>
                                    <td width="5%" class="listhdr">SID</td>
                                    <td width="6%" class="listhdrr">Proto</td>
                                    <td width="15%" class="listhdrr">Source</td>
                                    <td width="10%" class="listhdrr">Port</td>
                                    <td width="15%" class="listhdrr">Destination</td>
                                    <td width="10%" class="listhdrr">Port</td>
                                  <td width="32%" class="listhdrr">Message</td>

                              </tr>
                            <tr>
                            <?php

                                echo "<br>Category: ";

                                //string for populating category select
                                \$currentruleset = substr(\$file, 27);
                                ?>
                                <form name="forms">
                                    <select name="selectbox" class="formfld" onChange="go()">
                                        <?php
                                        \$i=0;
                                        foreach (\$files as \$value)
                                        {
                                            \$selectedruleset = "";
                                            if (\$files[\$i] === \$currentruleset)
                                                \$selectedruleset = "selected";
                                            ?>
                                            <option value="?&openruleset=<?=\$ruledir;?><?=\$files[\$i];?>" <?=\$selectedruleset;?>><?=\$files[\$i];?></option>"
                                            <?php
                                            \$i++;

                                        }
                                        ?>
                                    </select>
                                </form>
                            </tr>
                            <?php

                            \$counter = 0;
                            \$printcounter = 0;

                            foreach ( \$splitcontents as \$value )
                            {

                                \$counter++;
                                \$disabled = "False";
                                \$comments = "False";

                                \$tempstring = \$splitcontents[\$counter];
                                \$findme = "# alert"; //find string for disabled alerts

                                //find alert
                                \$disabled_pos = strstr(\$tempstring, \$findme);


                                    //do soemthing, this rule is enabled
                                    \$counter2 = 1;

                                    //retrieve sid value
                                    \$sid = get_middle(\$tempstring, 'sid:', ';', 0);

                                    //check to see if the sid is numberical
                                    \$is_sid_num = is_numeric(\$sid);

                                    //if SID is numerical, proceed
                                    if (\$is_sid_num)
                                    {

                                        //if find alert is false, then rule is disabled
                                        if (\$disabled_pos !== false){
                                            \$counter2 = \$counter2+1;
                                            \$textss = "<span class=\"gray\">";
                                            \$textse = "</span>";
                                            \$iconb = "icon_block_d.gif";
                                        }
                                        else
                                        {
                                            \$textss = \$textse = "";
                                            \$iconb = "icon_block.gif";
                                        }

                                        \$rule_content = explode(' ', \$tempstring);

                                        \$protocol = \$rule_content[\$counter2];//protocol location
                                        \$counter2++;
                                        \$source = \$rule_content[\$counter2];//source location
                                        \$counter2++;
                                        \$source_port = \$rule_content[\$counter2];//source port location
                                        \$counter2 = \$counter2+2;
                                        \$destination = \$rule_content[\$counter2];//destination location
                                        \$counter2++;
                                        \$destination_port = \$rule_content[\$counter2];//destination port location

                                        \$message = get_middle(\$tempstring, 'msg:"', '";', 0);										
										
                                        echo "<tr>";
                                        echo "<td class=\"listt\">";
                                        echo \$textss;
                                        ?>
                                        <a href="?&openruleset=<?=\$file;?>&act=toggle&id=<?=\$counter;?>"><img src="./themes/<?= \$g['theme']; ?>/images/icons/<?=\$iconb;?>" width="11" height="11" border="0" title="click to toggle enabled/disabled status"></a>
                                        <?php
                                        echo \$textse;
                                        echo "</td>";


                                        echo "<td class=\"listlr\">";
                                        echo \$textss;
                                        echo \$sid;
                                        echo \$textse;
                                        echo "</td>";

                                        echo "<td class=\"listlr\">";
                                        echo \$textss;
                                        echo \$protocol;
                                        \$printcounter++;
                                        echo \$textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo \$textss;
                                        echo \$source;
                                        echo \$textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo \$textss;
                                        echo \$source_port;
                                        echo \$textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo \$textss;
                                        echo \$destination;
                                        echo \$textse;
                                        echo "</td>";
                                        echo "<td class=\"listlr\">";
                                        echo \$textss;
                                        echo \$destination_port;
                                        echo \$textse;
                                        echo "</td>";
                                        ?>
                                        <td class="listbg"><font color="white">
                                        <?php
                                        echo \$textss;
                                        echo \$message;
                                        echo \$textse;
                                        echo "</td>";
                                        ?>
                                          <td valign="middle" nowrap class="list">
                                            <table border="0" cellspacing="0" cellpadding="1">
                                                <tr>
                                                  <td><a href="snort_rules_edit.php?openruleset=<?=\$file;?>&id=<?=\$counter;?>"><img src="./themes/<?= \$g['theme']; ?>/images/icons/icon_e.gif" title="edit rule" width="17" height="17" border="0"></a></td>
                                                </tr>
                                            </table>
                                        </td>
                                        <?php
                                    }
                            }
                            echo "   ";
                            echo "There are ";
                            echo \$printcounter;
                            echo " rules in this category. <br><br>";
                            ?>
                         </table>
                    </td>
                </tr>
                <table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0">
                                <tr>
                                  <td width="16"><img src="./themes/<?= \$g['theme']; ?>/images/icons/icon_block.gif" width="11" height="11"></td>
                                  <td>Rule Enabled</td>
                                </tr>
                                <tr>
                                  <td><img src="./themes/<?= \$g['theme']; ?>/images/icons/icon_block_d.gif" width="11" height="11"></td>
                                  <td nowrap>Rule Disabled</td>


                                </tr>
                        <tr>
                          <td colspan="10">
                  <p>
                  <!--<strong><span class="red">Warning:<br>
                  </span></strong>Editing these r</p>-->
                         </td>
                            </tr>
              </table>
            </table>

    </td>
  </tr>
</table>


<?php include("fend.inc"); ?>
</div></body>
</html>

EOD;

/* write out snort_rules_php */
$bconf = fopen("/usr/local/pkg/snort/snort_{$snortIf}/snort_rules_{$snortIf}.php", "w");
if(!$bconf)
{
        log_error("Could not open /usr/local/pkg/snort/snort_{$snortIf}/snort_rules_{$snortIf}.php for writing.");
        exit;
        }
        fwrite($bconf, $snort_rules_php_text);
         fclose($bconf);

        conf_mount_ro();

        }
}

/* create snort_rules_edit.php for every interface selected */
function create_snort_rules_edit_php()
{
include("filter.inc");
include("config.inc");

        global $bconfig, $bg;

        conf_mount_rw();

        $first = 0;
        $snortInterfaces = array(); /* -gtm  */

        $if_list = $config['installedpackages']['snort']['config'][0]['iface_array'];
        $if_array = split(',', $if_list);
        //print_r($if_array);
        if($if_array) {
                foreach($if_array as $iface) {
                        $if = convert_friendly_interface_to_real_interface_name($iface);

                        if($config['interfaces'][$iface]['ipaddr'] == "pppoe") {
                                $if = "ng0";
                        }

                        /* build a list of user specified interfaces -gtm */
                        if($if){
                          array_push($snortInterfaces, $if);
                          $first = 1;
                        }
                }

                if (count($snortInterfaces) < 1) {
                        log_error("Snort will not start.  You must select an interface for it to listen on.");
                        return;
                }
        }


        foreach($snortInterfaces as $snortIf)
        {

$snort_rules_edit_php_text = <<<EOD
<?php
/* \$Id\$ */
/*
    snort_rules_edit.php
    Copyright (C) 2004, 2005 Scott Ullrich
    Copyright (C) 2004, 2005 Scott Robert Zelaya
    All rights reserved.

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

function get_middle(\$source, \$beginning, \$ending, \$init_pos) {
   \$beginning_pos = strpos(\$source, \$beginning, \$init_pos);
   \$middle_pos = \$beginning_pos + strlen(\$beginning);
   \$ending_pos = strpos(\$source, \$ending, \$beginning_pos);
   \$middle = substr(\$source, \$middle_pos, \$ending_pos - \$middle_pos);
   return \$middle;
}


\$file = \$_GET['openruleset'];

//read snort file
\$filehandle = fopen(\$file, "r");

//get rule id
\$lineid = \$_GET['id'];

//read file into string, and get filesize
\$contents = fread(\$filehandle, filesize(\$file));

//close handler
fclose (\$filehandle);

//delimiter for each new rule is a new line
\$delimiter = "\n";

//split the contents of the string file into an array using the delimiter
\$splitcontents = explode(\$delimiter, \$contents);

//copy rule contents from array into string
\$tempstring = \$splitcontents[\$lineid];

//explode rule contents into an array, (delimiter is space)
\$rule_content = explode(' ', \$tempstring);

//search string
\$findme = "# alert"; //find string for disabled alerts

//find if alert is disabled
\$disabled = strstr(\$tempstring, \$findme);

//get sid
\$sid = get_middle(\$tempstring, 'sid:', ';', 0);


//if find alert is false, then rule is disabled
if (\$disabled !== false)
{
	//move counter up 1, so we do not retrieve the # in the rule_content array
	\$counter2 = 2;
}
else
{
	\$counter2 = 1;
}


\$protocol = \$rule_content[\$counter2];//protocol location
\$counter2++;
\$source = \$rule_content[\$counter2];//source location
\$counter2++;
\$source_port = \$rule_content[\$counter2];//source port location
\$counter2++;
\$direction = \$rule_content[\$counter2];
\$counter2++;
\$destination = \$rule_content[\$counter2];//destination location
\$counter2++;
\$destination_port = \$rule_content[\$counter2];//destination port location
\$message = get_middle(\$tempstring, 'msg:"', '";', 0);

\$content = get_middle(\$tempstring, 'content:"', '";', 0);
\$classtype = get_middle(\$tempstring, 'classtype:', ';', 0);
\$revision = get_middle(\$tempstring, 'rev:', ';',0);

\$pgtitle = "Snort: Edit Rule";
require("guiconfig.inc");
include("head.inc");
?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">

<?php include("fbegin.inc"); ?>
<?php
if(!\$pgtitle_output)
	echo "<p class=\"pgtitle\"><?=\$pgtitle?></p>";
?>
<table width="99%" border="0" cellpadding="0" cellspacing="0">
  <tr>
  	<td>
<?php
    \$tab_array = array();
    \$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_{$snortIf}.xml&id=0");
    \$tab_array[] = array(gettext("Update Rules"), false, "/snort_download_rules.php");
    \$tab_array[] = array(gettext("Categories"), false, "snort/snort_{$snortIf}/snort_rulesets_{$snortIf}.php");
    \$tab_array[] = array(gettext("Rules"), true, "snort/snort_{$snortIf}/snort_rules_{$snortIf}.php");
	\$tab_array[] = array(gettext("Servers"), false, "/pkg_edit.php?xml=snort/snort_{$snortIf}/snort_define_servers_{$snortIf}.xml&amp;id=0");
    \$tab_array[] = array(gettext("Blocked"), false, "/snort_blocked.php");
    \$tab_array[] = array(gettext("Whitelist"), false, "/pkg.php?xml=snort_whitelist.xml");
	\$tab_array[] = array(gettext("Threshold"), false, "/pkg.php?xml=snort/snort_{$snortIf}/snort_threshold_{$snortIf}.xml");
    \$tab_array[] = array(gettext("Alerts"), false, "/snort_alerts.php");
    \$tab_array[] = array(gettext("Advanced"), false, "/pkg_edit.php?xml=snort_advanced.xml&id=0");
    display_top_tabs(\$tab_array);
?>
  	</td>
  </tr>
  <tr>
    <td>
		<div id="mainarea">
			<table id="maintable" class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td>
						<form action="snort_rules.php?openruleset=<?=\$file;?>&id=<?=\$lineid;?>" target="" method="post" name="editform" id="editform">
							<table id="edittable" class="sortable" width="100%" border="0" cellpadding="0" cellspacing="0">
								<tr>
										<td class="listhdr" width="10%">Enabled: </td>
										<td class="listlr" width="30%"><input name="enabled" type="checkbox" id="enabled" value="yes" <?php if (\$disabled === false) echo "checked";?>></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">SID: </td>
										<td class="listlr" width="30%"><?php echo \$sid; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Protocol: </td>
										<td class="listlr" width="30%"><?php echo \$protocol; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Source: </td>
										<td class="listlr" width="30%"><input name="src" type="text" id="src" size="20" value="<?php echo \$source;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Source Port: </td>
										<td class="listlr" width="30%"><input name="srcport" type="text" id="srcport" size="20" value="<?php echo \$source_port;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Direction:</td>
										<td class="listlr" width="30%"><?php echo \$direction;?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Destination:</td>
										<td class="listlr" width="30%"><input name="dest" type="text" id="dest" size="20" value="<?php echo \$destination;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Destination Port: </td>
										<td class="listlr" width="30%"><input name="destport" type="text" id="destport" size="20" value="<?php echo \$destination_port;?>"></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Message: </td>
										<td class="listlr" width="30%"><?php echo \$message; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Content: </td>
										<td class="listlr" width="30%"><?php echo \$content; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Classtype: </td>
										<td class="listlr" width="30%"><?php echo \$classtype; ?></td>
								</tr>
								<tr>
										<td class="listhdr" width="10%">Revision: </td>
										<td class="listlr" width="30%"><?php echo \$revision; ?></td>
								</tr>
								<tr><td>&nbsp</td></tr>
								<tr>
										<td><input name="lineid" type="hidden" value="<?=\$lineid;?>"></td>
										<td><input class="formbtn" value="Save" type="submit" name="editsave" id="editsave">&nbsp&nbsp&nbsp<input type="button" class="formbtn" value="Cancel" onclick="history.back()"></td>
								</tr>
							</table>
						</form>
					</td>
				</tr>
			</table>
	</td>
</tr>
</table>

<?php include("fend.inc"); ?>
</div></body>
</html>

EOD;

/* write out snort_rules_edit_php */
$bconf = fopen("/usr/local/pkg/snort/snort_{$snortIf}/snort_rules_edit_{$snortIf}.php", "w");
if(!$bconf)
{
        log_error("Could not open /usr/local/pkg/snort/snort_{$snortIf}/snort_rules_edit_{$snortIf}.php for writing.");
        exit;
        }
        fwrite($bconf, $snort_rules_edit_php_text);
         fclose($bconf);

        conf_mount_ro();

        }
}


create_snort_xml();

create_snort_barnyard2_xml();

create_snort_define_servers_xml();

create_snort_threshold_xml();

create_snort_rules_php();

create_snort_rules_edit_php();

?>
