<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/29 10:30:13  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="common-key-tables.xsl"/>
  <xsl:import href="common-c-code.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:apply-templates select="//traversal" mode="check-traversal" />
  </xsl:template>

  <xsl:template match="traversal" mode="check-traversal" >
    <xsl:apply-templates select=".//node" mode="check-traversal" />
  </xsl:template>

  <xsl:template match="node" mode="check-traversal" >
    <xsl:if test="not( key( &quot;nodes&quot;, @name))">
      <xsl:value-of select="'Node '" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="' of traversal '"/>
      <xsl:value-of select="../../@name" />
      <xsl:value-of select="' unknown'" />
      <xsl:call-template name="newline" />
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
