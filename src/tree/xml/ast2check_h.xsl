<?xml version="1.0"?>

<!--
  $Log$
  Revision 1.1  2004/11/19 13:55:29  jhb
  Initial revision

  Revision 1.1 2004/09/29 15:15:00 jhb
  Initial revision

-->

  <!-- This stylesheet generates a check.h file implementing all functions needed to check a node -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

 <xsl:variable name="newline">
<xsl:text>
</xsl:text>
 </xsl:variable>

<xsl:template match="/">

 <xsl:text>
#ifndef _sac_check_h
#define _sac_check_h

extern node *CHKCheck(  node *arg_node);
</xsl:text> 
<xsl:value-of select="$newline"/>

  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>

<xsl:text>

#endif /*_sac_check_h */
</xsl:text>

</xsl:template>

<xsl:template match="node">

<xsl:value-of select="'extern node *CHK'"/>
  <xsl:value-of select="translate(@name, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
  <xsl:value-of select="'(node *arg_node, info *arg_info)'"/>
  <xsl:value-of select="';'"/>
  <xsl:value-of select="$newline"/>

</xsl:template>

</xsl:stylesheet>