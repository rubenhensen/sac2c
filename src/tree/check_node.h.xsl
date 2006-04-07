<?xml version="1.0"?>
<!--
  $Id$
  Revision 1.1  2006/04/05 14:39:21  jhb
  Initial revision
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common-travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a check_node.h file implementing all
     functions needed to free a node -->

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'check_node.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to free node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id: free_check.h.xsl 14593 2006-01-31 17:09:55Z cg $'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_CHECK_NODE_H_
#define _SAC_CHECK_NODE_H_

#include "types.h"

extern node *CHKMpostfun( node * arg_node, info * arg_info);

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:text>

#endif /* _SAC_CHECK_NODE_H_ */

  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:value-of select="'extern '" />
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">CHKM</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
</xsl:template>

</xsl:stylesheet>
