<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/08/08 16:26:34  sah
  check tree traversal now compiles;)

  Revision 1.1  2004/08/07 17:58:55  sah
  Initial revision

  Revision 1.1  2004/08/07 16:22:04  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-travfun.xsl"/>
<xsl:import href="common-node-access.xsl"/>
<xsl:import href="common-make-assertion.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a free_node.c file implementing all
     functions needed to free a node -->

<xsl:template match="/">
  <!-- generate file header and doxygen group -->
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'check_node.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by check tree traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'checktree'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Check Tree Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by check tree traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#include "check_node.h"
#include "tree_basic.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"
#include "Error.h"

#define AST_NO_COMPAT
#include "node_compat.h"

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>
#undef AST_NO_COMPAT
#include "node_compat.h"
  </xsl:text>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<xsl:template match="node">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">Check</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
    <xsl:with-param name="text">Checks the current node against its xml specification</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">Check</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;Check'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- perform check -->
  <xsl:apply-templates select="sons/son" mode="make-assertion">
    <xsl:with-param name="self"><xsl:value-of select="'arg_node'"/></xsl:with-param>
  </xsl:apply-templates>
  <!-- call Check for sons -->
  <xsl:apply-templates select="sons/son"/>
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( arg_node);'"/>
  <!-- end of body -->
  <xsl:value-of select="'}'"/>
</xsl:template>

<!-- check all sons -->
<xsl:template match="son">
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' = Trav( '"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_info);'"/>
</xsl:template>

</xsl:stylesheet>
