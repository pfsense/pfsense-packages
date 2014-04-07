<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="2.0">
   <xsl:output method="html" encoding="UTF-8" indent="yes"/>
   <xsl:template match="pfsensepkgs">
       <html>
           <head>
               <title>pfSense Open Source Firewall Distribution - Packages</title>
               <link rel="shortcut icon" href="https://www.pfsense.org/images/favicon.ico"/>

               <link rel="stylesheet" href="templates/modular_plazza/css/template_css.css" type="text/css"/>
               <link rel="stylesheet" href="templates/modular_plazza/css/sfish.css" type="text/css"/>
           </head>
           <body class="bodies">
               <h2>pfSense Package list</h2>
               <xsl:apply-templates/>
           </body>
       </html>
   </xsl:template>

   <xsl:template match="packages">
       <xsl:for-each-group select="package" group-by="category">
           <h3>
               Category: <xsl:value-of select="current-grouping-key()"/>
           </h3>
           <xsl:for-each select="current-group()">
               <h4>
                   <xsl:value-of select="name"/>
               </h4>
               <span class="version">Version <xsl:value-of select="version"/> </span>
               <xsl:choose>
                   <xsl:when test="status = 'ALPHA'"><span style="color:red">alpha</span></xsl:when>
                   <xsl:when test="status = 'BETA'"><span style="color:blue">beta</span></xsl:when>
                   <xsl:otherwise><span style="color:green"><xsl:value-of select="status"/></span></xsl:otherwise>
               </xsl:choose>
               <br/>
               <xsl:value-of select="descr" disable-output-escaping="yes"/>
           </xsl:for-each>
       </xsl:for-each-group>
   </xsl:template>
</xsl:stylesheet>
