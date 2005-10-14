<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/23 11:30:21  sah
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
      <xsl:value-of select="'free_node.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to free node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id: ast2free_node_h.xsl 14294 2005-10-10 12:40:03Z sah $'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_FREE_NODE_H_
#define _SAC_FREE_NODE_H_

#include "types.h"

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:text>

#endif /* _SAC_FREE_NODE_H_ */

  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:value-of select="'extern '" />
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">FREE</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
</xsl:template>

</xsl:stylesheet>
