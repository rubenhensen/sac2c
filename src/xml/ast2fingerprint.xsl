<?xml version="1.0"?>
<!--
  $Id: node_basic.c.xsl 14646 2006-03-22 16:25:27Z jhb $
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a node_basic.c file implementing all
     the make functions for nodes                                    -->

<xsl:template match="/">
  <xsl:apply-templates select="definition/syntaxtree" />
</xsl:template>

<!-- filter away descriptions -->
<xsl:template match="description" />

<!-- generic template matching all elements and attributes -->
<xsl:template match="node() | @*">
  <xsl:copy>
    <xsl:apply-templates select="node() | @*"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>
