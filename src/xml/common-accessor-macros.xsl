<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.3  2004/12/01 18:48:13  sah
  added some extra () in flags macros

  Revision 1.2  2004/11/24 16:46:06  sah
  changed node representation

  Revision 1.1  2004/11/23 11:34:00  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-c-code.xsl"/>
<xsl:import href="common-node-access.xsl"/>
<xsl:import href="common-name-to-nodeenum.xsl"/>

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
  <xsl:value-of select="'((n)->sons.'"/> 
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'->'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="')'" />
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
  <xsl:value-of select="'((n)->attribs.'"/> 
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'->'"/>
  <xsl:value-of select="@name"/>
  <!-- if the attribute is an array, we need to add the index to the macro -->
  <xsl:if test="key( &quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'[x]'" />
  </xsl:if>
  <xsl:value-of select="')'" />
  <xsl:call-template name="newline"/>
</xsl:template>

<!-- generate macros for flags -->
<xsl:template match="flag" mode="accessor-macros">
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
  </xsl:call-template>
  <!-- generate right side of macro -->
  <xsl:value-of select="'((n)->attribs.'"/> 
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'->flags.'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="')'" />
  <xsl:call-template name="newline"/>
</xsl:template>

</xsl:stylesheet>
