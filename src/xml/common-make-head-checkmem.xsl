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


<!-- generate a make special function head for the check mechanismn -->
<xsl:template match="node" mode="make-head-checkmem">
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
  <xsl:value-of select="'At'" />
  <xsl:value-of select="'( '"/>
  <!-- permanent attributes without default value first -->
  <xsl:apply-templates select="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]" mode="make-head"/>
  <!-- add a , if needed -->
  <xsl:if test="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]">
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <!-- sons without default value are last parameters -->
  <xsl:apply-templates select="sons/son[ not( @default)]" mode="make-head"/>
  <xsl:if test="sons/son[ not( @default)]" >
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <xsl:value-of select="'char *file, int line)'"/>
</xsl:template>


<xsl:template match="node" mode="make-head-checkmem-define">  
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'#define '"/>
  <xsl:value-of select="'TBmake'"/>
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
  <xsl:value-of select="'('"/>
  <!-- permanent attributes without default value first -->
  <xsl:apply-templates select="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]" mode="make-head-checkmem"/>
  <!-- add a , if needed -->
  <xsl:if test="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]">
    <xsl:if test="sons/son[ not( @default)]">
      <xsl:value-of select="' ,'"/>
    </xsl:if>
  </xsl:if>
  <!-- sons without default value are last parameters -->
  <xsl:apply-templates select="sons/son[ not( @default)]" mode="make-head-checkmem"/>
  <xsl:value-of select="')'"/>

  <xsl:value-of select="' '"/>
  <xsl:value-of select="'TBmake'"/>
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
  <xsl:value-of select="'At'" />
  <xsl:value-of select="'('"/>
  <!-- permanent attributes without default value first -->
  <xsl:apply-templates select="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]" mode="make-head-checkmem"/>
  <!-- add a , if needed -->
  <xsl:if test="attributes/attribute[type/targets/target/phases/all][not(@default)][type/targets/target/@mandatory = &quot;yes&quot;]">
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <!-- sons without default value are last parameters -->
  <xsl:apply-templates select="sons/son[ not( @default)]" mode="make-head-checkmem"/>
  <xsl:if test="sons/son[ not( @default)]" >
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <xsl:value-of select="' __FILE__, __LINE__)'"/>
</xsl:template>


<xsl:template match="node" mode="make-head-checkmem-ifdef">
  <xsl:value-of select="$newline"/>
  <xsl:text>#ifdef SHOW_MALLOC</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="node" mode="make-head-checkmem-else">
  <xsl:value-of select="$newline"/>
  <xsl:text>#else</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="node" mode="make-head-checkmem-endif">
  <xsl:value-of select="$newline"/>
  <xsl:text>#endif /* SHOW_MALLOC */</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:template>


<!-- generate a son argument -->
<xsl:template match="son" mode="make-head-checkmem">
  <xsl:if test="position() != 1">
    <xsl:value-of select="' ,'"/>
  </xsl:if>
  <xsl:value-of select="@name"/>
</xsl:template>

<!-- generate an attribute argument -->
<xsl:template match="attribute" mode="make-head-checkmem">
  <xsl:if test="position() != 1">
    <xsl:value-of select="' ,'"/>   
  </xsl:if>
  <xsl:value-of select="' '"/>
  <xsl:value-of select="@name"/>
</xsl:template>

<xsl:variable name="newline">
  <xsl:text>
  </xsl:text>
</xsl:variable>

</xsl:stylesheet>