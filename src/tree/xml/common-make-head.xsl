<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/08/07 16:19:05  sah
  most xsl files use key-tables for type lookups
  now which increases speed significantly.
  lots of small improvements

  Revision 1.1  2004/07/11 18:22:40  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">
<xsl:import href="common-c-code.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- generate a make function head -->
<xsl:template match="node" mode="make-head">
  <xsl:value-of select="'node *Make'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'( '"/>
  <!-- permanent attributes without default value first -->
  <xsl:apply-templates select="attributes/attribute[phases/all][not(@default)]" mode="make-head"/>
  <!-- add a , if needed -->
  <xsl:if test="attributes/attribute[phases/all][not( @default)]">
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

</xsl:stylesheet>
