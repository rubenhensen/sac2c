<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/23 11:36:15  sah
  Initial revision


 
 -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Some nice templates for accessing fields of the node structure
     by using the access macros -->

<xsl:template name="node-access">
  <!-- name of the variable pointing to the node -->
  <xsl:param name="node" />
  <!-- name of node type -->
  <xsl:param name="nodetype" />
  <!-- field to select -->
  <xsl:param name="field" />
  <!-- index for fields beeing an array, otherwise do not set -->
  <xsl:param name="index" />

  <!-- generate macro selector ala ARRAY_NEXT(n) -->
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="$nodetype"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="$field"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'( '" />
  <xsl:value-of select="$node" />
  <xsl:if test="$index != &quot;&quot;">
    <xsl:value-of select="', '" />
    <xsl:value-of select="$index" />
  </xsl:if>
  <xsl:value-of select="') '" />
</xsl:template>

</xsl:stylesheet>
