<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.4  2004/08/07 16:19:05  sah
  most xsl files use key-tables for type lookups
  now which increases speed significantly.
  lots of small improvements

  Revision 1.3  2004/08/06 21:19:57  sah
  uses common-node-access.xsl to define the macros now

  Revision 1.2  2004/07/31 16:16:57  sah
  added support for flags and moved to memory saving attribute
  structure.

  Revision 1.1  2004/07/11 18:21:13  sah
  Initial revision


-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>
<xsl:import href="common-node-access.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- generate accessors for nodes -->
<xsl:template match="son" mode="accessor-macros">
  <xsl:value-of select="'#define '"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">n</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' n->node['"/>
  <xsl:value-of select="position() - 1"/>
  <xsl:value-of select="']'"/>
  <xsl:call-template name="newline"/>
</xsl:template>

<!-- generate accessors for attributes -->
<xsl:template match="attribute" mode="accessor-macros">
  <xsl:value-of select="'#define '"/>
  <!-- generate left side of macro -->
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">n</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
    <!-- if the attribute is an array, we need to add the index to the macro -->
    <xsl:with-param name="index">
      <xsl:if test="key( &quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'x'" />
      </xsl:if>
    </xsl:with-param>
  </xsl:call-template>
  <!-- generate right side of macro -->
  <xsl:value-of select="'n->attribs.N_'"/> 
  <xsl:value-of select="../../@name"/>
  <xsl:value-of select="'->'"/>
  <xsl:value-of select="@name"/>
  <!-- if the attribute is an array, we need to add the index to the macro -->
  <xsl:if test="key( &quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'[x]'" />
  </xsl:if>
  <xsl:call-template name="newline"/>
</xsl:template>

</xsl:stylesheet>
