<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/07/11 18:21:13  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- generate accessors for nodes -->
<xsl:template match="son" mode="accessor-macros">
  <xsl:value-of select="'#define '"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">                                          
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'(n) n->node['"/>
  <xsl:value-of select="position() - 1"/>
  <xsl:value-of select="']'"/>
  <xsl:call-template name="newline"/>
</xsl:template>

<!-- generate accessors for attributes -->
<xsl:template match="attribute" mode="accessor-macros">
  <xsl:value-of select="'#define '"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'(n'" />
  <!-- if the attribute is an array, we need to add the index to the macro -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size" >
    <xsl:value-of select="', x'" />
  </xsl:if>
  <xsl:value-of select="') n->attribs.N_'"/> 
  <xsl:value-of select="../../@name"/>
  <xsl:value-of select="'.'"/>
  <xsl:value-of select="@name"/>
  <!-- if the attribute is an array, we need to add the index to the macro -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size" >
    <xsl:value-of select="'[x]'" />
  </xsl:if>
  <xsl:call-template name="newline"/>
</xsl:template>

</xsl:stylesheet>
