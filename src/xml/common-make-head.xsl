<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2005/09/21 16:52:10  sah
  adapted xsl generators to new xml structure.

  Revision 1.1  2004/11/23 11:35:39  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">
<xsl:import href="../xml/common-c-code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>


<!-- generate a make function head -->
<xsl:template match="node" mode="make-head">
  <xsl:value-of select="'node *TBmake'"/>
  <xsl:call-template name="uppercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="substring( @name, 1, 1)" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="substring( @name, 2, 30)" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'( '"/>
  <!-- permanent attributes without default value first -->
  <xsl:apply-templates select="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]" mode="make-head"/>
  <!-- add a , if needed -->
  <xsl:if test="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]">
    <xsl:if test="sons/son[ not( @default)]">
      <xsl:value-of select="' ,'"/>
    </xsl:if>
  </xsl:if>
  <!-- sons without default value are last parameters -->
  <xsl:apply-templates select="sons/son[ not( @default)]" mode="make-head"/>
  <xsl:value-of select="')'"/>
</xsl:template>

<!-- generate a son argument -->
<xsl:template match="son" mode="make-head">
  <xsl:if test="position() != 1">
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <xsl:value-of select="'node * '"/>
  <xsl:value-of select="@name"/>
</xsl:template>

<!-- generate an attribute argument -->
<xsl:template match="attribute" mode="make-head">
  <xsl:if test="position() != 1">
    <xsl:value-of select="' ,'"/>   
  </xsl:if>
  <xsl:value-of select="key( &quot;types&quot;, ./type/@name)/@ctype"/>
  <!-- if it is an array, we have to add an indirection to the type -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'*'" />
  </xsl:if>
  <xsl:value-of select="' '"/>
  <xsl:value-of select="@name"/>
</xsl:template>

<xsl:variable name="newline">
  <xsl:text>
  </xsl:text>
</xsl:variable>

</xsl:stylesheet>
