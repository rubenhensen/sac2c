<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/07/11 18:22:03  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- templates for generating the body of a make function -->
<xsl:template match="node" mode="make-body">
  <xsl:value-of select="'{'"/>
  <!-- DBUG_ENTER call -->
  <xsl:value-of select="'DBUG_ENTER( &quot;Make'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- allocate new node this -->
  <xsl:value-of select="'node *this = Malloc(sizeof(node));'"/>
  <!-- assign sons and attributes a value -->
  <xsl:apply-templates select="sons/son" mode="make-body"/>
  <xsl:apply-templates select="attributes/attribute" mode="make-body"/>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( this);'"/>
  <xsl:value-of select="'}'"/>
</xsl:template>

<!-- generate the assignment for a son -->
<xsl:template match="sons/son" mode="make-body">
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'(this) = '"/>
  <xsl:value-of select="@name" />
  <xsl:value-of select="';'"/>
</xsl:template>
 
<!-- generate the assignmnent for an attribute -->
<xsl:template match="attributes/attribute" mode="make-body">
  <!-- if it is an array, we have to build a for loop over its elements -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
    <xsl:value-of select="'for( int x; x&lt;'" />
    <xsl:value-of select="//attributetypes/type[@name = current()/type/@name]/@size" />
    <xsl:value-of select="'; x++) { '" />
  </xsl:if>
  <!-- left side of assignment -->
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="../../@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'_'"/>
  <xsl:call-template name="uppercase">
    <xsl:with-param name="string"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'( this'" />
  <!-- if its is an array, we have to add another parameter -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
    <xsl:value-of select="', x'"/>
  </xsl:if>
  <xsl:value-of select="') = '" />
  <!-- right side of assignment -->
  <xsl:apply-templates select="@name" mode="make-body" />
  <xsl:value-of select="';'"/>
  <!-- finally, end the for loop if it was an array -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
    <xsl:value-of select="'}'" />
  </xsl:if>
</xsl:template> 

<!-- a default value implies that this attribute is not 
     passed as an argument to the make function, thus the r-value
     of the assignment is its default -->
<xsl:template match="@name[../@default]" mode="make-body">
  <xsl:value-of select="../@default"/>
</xsl:template>

<!-- no default and beeing a permanent attribute implies that this 
     attribute is passed as an argument thus the r-value is the 
     argument -->
<xsl:template match="@name[not(../@default)][../phases/all]" mode="make-body">
  <xsl:value-of select="."/>
  <!-- if its an array, we have to add the selector -->
  <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
    <xsl:value-of select="'[x]'" />
  </xsl:if>
</xsl:template>

<!-- no default and beeing a temporary attribute implies using the
     init value for this attributes type -->
<xsl:template match="@name[not(../@default)][not(../phases/all)]" mode="make-body">
  <xsl:value-of select="//attributetypes/type[@name = current()/../type/@name]/@init"/>
</xsl:template>

</xsl:stylesheet>
