<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/07/31 16:18:31  sah
  Initial revision

  Revision 1.1  2004/07/11 18:21:13  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- generate accessors for flag attributes -->
<xsl:template match="flags[flag]" mode="flag-defines">
  <xsl:value-of select="'#define '"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_FLAGS(n) (n->attribs.N_'"/>
  <xsl:value-of select="../@name"/>
  <xsl:value-of select="'->_flags)'"/>
  <xsl:call-template name="newline"/>
  <!-- dbug flag -->
  <xsl:value-of select="'#define '"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_DBUG_FLAGS(n) (n->attribs.N_'"/>
  <xsl:value-of select="../@name"/>
  <xsl:value-of select="'->_dbug_flags)'"/>
  <xsl:call-template name="newline"/>
  <xsl:apply-templates select="flag" mode="flag-defines"/>
</xsl:template>

<!-- generate flags for nodes -->
<xsl:template match="flag" mode="flag-defines">
  <xsl:value-of select="'#define IS_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' (1 &lt;&lt; '"/>
  <xsl:value-of select="position() - 1"/>
  <xsl:value-of select="')'"/>
  <xsl:call-template name="newline"/>
</xsl:template>

</xsl:stylesheet>
