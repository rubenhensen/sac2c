<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.3  2005/05/20 18:08:35  sah
  added _SAC_AST_VERSION_ define

  Revision 1.2  2004/11/23 13:14:02  ktr
  removed node_compat.h

  Revision 1.1  2004/11/23 11:30:50  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common-key-tables.xsl"/>
<xsl:import href="../xml/common-accessor-macros.xsl"/>
<xsl:import href="../xml/common-make-head.xsl"/>
<xsl:import href="../xml/common-travfun.xsl"/>
<xsl:import href="../xml/common-name-to-nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a node_basic.h file defining all the macros
     to the different attributes and the MakeXXX functions for nodes       -->

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'node_basic.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to allocate node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_NODE_BASIC_H_
#define _SAC_NODE_BASIC_H_

#include "types.h"

  </xsl:text>
  <xsl:apply-templates select="/definition/@version"/>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>

    #endif /* _SAC_NODE_BASIC_H_ */
  </xsl:text>
</xsl:template>

<xsl:template match="@version">
  <xsl:call-template name="newline"/>
  <xsl:value-of select="'#define _SAC_AST_VERSION_ &quot;'"/>
  <xsl:value-of select="." />
  <xsl:value-of select="'&quot;'"/>
  <xsl:call-template name="newline"/>
</xsl:template>

<xsl:template match="node">
  <xsl:text>

/*****************************************************************************
 * macros and functions for </xsl:text>
  <xsl:call-template name="name-to-nodeenum" >
    <xsl:with-param name="name" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template><xsl:text>
 *****************************************************************************/

  </xsl:text>
  <xsl:apply-templates select="sons/son" mode="accessor-macros"/>
  <xsl:apply-templates select="attributes/attribute" mode="accessor-macros"/>
  <xsl:apply-templates select="flags" mode="accessor-macros"/>
  <xsl:apply-templates select="." mode="make-head"/>
</xsl:template>

<xsl:template match="node" mode="make-head">
  <xsl:value-of select="'extern '"/>
  <xsl:apply-imports/>
  <xsl:value-of select="';'"/>
</xsl:template>

</xsl:stylesheet>
