<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.3  2005/09/21 15:02:37  jhb
  *** empty log message ***

  Revision 1.2  2004/12/05 17:50:57  sah
  extended checks

  Revision 1.1  2004/11/29 10:30:13  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="../xml/common-key-tables.xsl"/>
  <xsl:import href="../xml/common-c-code.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:apply-templates select="//traversal" mode="check-traversal" />
    <xsl:apply-templates select="//nodeset" mode="check-nodeset" />
    <xsl:apply-templates select="//syntaxtree" mode="check-syntaxtree" />
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

  <xsl:template match="nodeset" mode="check-nodeset">
    <xsl:apply-templates select=".//node" />
  </xsl:template>

  <xsl:template match="node" mode="check-nodeset" >
    <xsl:if test="not( key( &quot;nodes&quot;, @name))">
      <xsl:value-of select="'Node '" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="' of nodeset '"/>
      <xsl:value-of select="../@name" />
      <xsl:value-of select="' unknown'" />
      <xsl:call-template name="newline" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="syntaxtree" mode="check-syntaxtree">
    <xsl:apply-templates select=".//target" mode="check-syntaxtree" />
  </xsl:template>

  <xsl:template match="type/targets/target/node" mode="check-syntaxtree" >
    <xsl:if test="not( key( &quot;nodes&quot;, @name))">
      <xsl:value-of select="'Target '" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="' of attribute '"/>
      <xsl:value-of select="../../../@name" />
      <xsl:value-of select="' of node '"/>
      <xsl:value-of select="../../../../../@name" />
      <xsl:value-of select="' unknown'" />
      <xsl:call-template name="newline" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="type/targets/target/set" mode="check-syntaxtree" >
    <xsl:if test="not( key( &quot;nodesets&quot;, @name))">
      <xsl:value-of select="'Targetset '" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="' of attribute '"/>
      <xsl:value-of select="../../../@name" />
      <xsl:value-of select="' of node '"/>
      <xsl:value-of select="../../../../../@name" />
      <xsl:value-of select="' unknown'" />
      <xsl:call-template name="newline" />
    </xsl:if>
  </xsl:template>  

  <xsl:template match="son/targets/target/node" mode="check-syntaxtree" >
    <xsl:if test="not( key( &quot;nodes&quot;, @name))">
      <xsl:value-of select="'Target '" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="' of son '"/>
      <xsl:value-of select="../../@name" />
      <xsl:value-of select="' of node '"/>
      <xsl:value-of select="../../../../@name" />
      <xsl:value-of select="' unknown'" />
      <xsl:call-template name="newline" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="son/targets/target/set" mode="check-syntaxtree" >
    <xsl:if test="not( key( &quot;nodesets&quot;, @name))">
      <xsl:value-of select="'Targetset '" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="' of son '"/>
      <xsl:value-of select="../../@name" />
      <xsl:value-of select="' of node '"/>
      <xsl:value-of select="../../../../@name" />
      <xsl:value-of select="' unknown'" />
      <xsl:call-template name="newline" />
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
