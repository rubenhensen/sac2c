<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/08/08 16:26:34  sah
  check tree traversal now compiles;)

  Revision 1.1  2004/08/07 17:59:05  sah
  Initial revision




-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a free_node.h file implementing all
     functions needed to free a node -->

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'check_node.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by check tree traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_CHECK_NODE_H
#define _SAC_CHECK_NODE_H

#include "tree_basic.h"
#include "internal_lib.h"

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>
#endif /* _SAC_CHECK_NODE_H */
  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:value-of select="'extern '" />
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">Check</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
</xsl:template>

</xsl:stylesheet>
