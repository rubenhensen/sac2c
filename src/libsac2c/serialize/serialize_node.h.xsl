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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'serialize_node.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to serialize node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'serialize_node.h.xsl'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_SERIALIZE_NODE_H
#define _SAC_SERIALIZE_NODE_H

#include "types.h"

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:text>
#endif /* _SAC_SERIALIZE_NODE_H */
  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:value-of select="'extern '" />
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">SET</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="';'" />
</xsl:template>

</xsl:stylesheet>
