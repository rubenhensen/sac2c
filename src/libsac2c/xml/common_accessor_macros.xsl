<?xml version="1.0"?>

<!--
 ***********************************************************************
 *                                                                     *
 *                      Copyright (c) 1994-2007                        *
 *         SAC Research Foundation (http://www.sac-home.org/)          *
 *                                                                     *
 *                        All Rights Reserved                          *
 *                                                                     *
 *   The copyright holder makes no warranty of any kind with respect   *
 *   to this product and explicitly disclaims any implied warranties   *
 *   of merchantability or fitness for any particular purpose.         *
 *                                                                     *
 ***********************************************************************
 -->

<!--  $Id$  -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_c_code.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- generate accessors for nodes -->
<xsl:template match="sons" mode="accessor-macros">
  <xsl:value-of select="'#ifdef CHECK_NODE_ACCESS'"/>
  <xsl:call-template name="newline"/>
  <xsl:apply-templates match="son" mode="accessor-macros-check" />
  <xsl:value-of select="'#else /* not CHECK_NODE_ACCESS */'" />
  <xsl:call-template name="newline"/>
  <xsl:apply-templates match="son" mode="accessor-macros-nocheck" />
  <xsl:value-of select="'#endif /* CHECK_NODE_ACCESS */'" />
  <xsl:call-template name="newline"/>
</xsl:template>

<xsl:template match="son" mode="accessor-macros-check">
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
  <xsl:value-of select="'(((node *) (((intptr_t) n) &amp; (~((intptr_t) 0) * ((intptr_t) (!((n)->nodetype - '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'))))))->sons.'"/> 
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

<xsl:template match="son" mode="accessor-macros-nocheck">
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
<xsl:template match="attributes" mode="accessor-macros">
  <xsl:value-of select="'#ifdef CHECK_NODE_ACCESS'"/>
  <xsl:call-template name="newline"/>
  <xsl:apply-templates match="attibute" mode="accessor-macros-check" />
  <xsl:value-of select="'#else /* not CHECK_NODE_ACCESS */'" />
  <xsl:call-template name="newline"/>
  <xsl:apply-templates match="attibute" mode="accessor-macros-nocheck" />
  <xsl:value-of select="'#endif /* CHECK_NODE_ACCESS */'" />
  <xsl:call-template name="newline"/>
</xsl:template>

<xsl:template match="attribute" mode="accessor-macros-check">
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
  <xsl:value-of select="'(((node *) (((intptr_t) n) &amp; (~((intptr_t) 0) * (((intptr_t) !((n)->nodetype - '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'))))))->attribs.'"/> 
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

<xsl:template match="attribute" mode="accessor-macros-nocheck">
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
<xsl:template match="flags[flag]" mode="accessor-macros" >
  <xsl:value-of select="'#define '"/>
  <!-- generate left side of macro -->
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">n</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="'FlagStructure'" />
    </xsl:with-param>
  </xsl:call-template>
  <!-- generate right side of macro -->
  <xsl:value-of select="'((n)->attribs.'"/> 
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'->flags'"/>
  <xsl:value-of select="')'" />
  <xsl:call-template name="newline"/>
  <!-- generate macros for each flag -->
  <xsl:value-of select="'#ifdef CHECK_NODE_ACCESS'"/>
  <xsl:call-template name="newline"/>
  <xsl:apply-templates select="flag" mode="accessor-macros-check" />
  <xsl:value-of select="'#else /* not CHECK_NODE_ACCESS */'" />
  <xsl:call-template name="newline"/>
  <xsl:apply-templates select="flag" mode="accessor-macros-nocheck" />
  <xsl:value-of select="'#endif /* CHECK_NODE_ACCESS */'" />
  <xsl:call-template name="newline"/>
</xsl:template>
  
<xsl:template match="flag" mode="accessor-macros-check">
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
  <xsl:value-of select="'(((node *) (((intptr_t) n) &amp; (~((intptr_t) 0) * (((intptr_t) !((n)->nodetype - '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'))))))->attribs.'"/> 
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

<xsl:template match="flag" mode="accessor-macros-nocheck">
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
