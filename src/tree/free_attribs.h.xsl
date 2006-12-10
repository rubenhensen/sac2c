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

<xsl:import href="../xml/common_travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a free_attribs.h file defining all
     functions needed to free the attributes of a node -->

<xsl:template match="/">
<xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'free_attribs.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to free the attributes of node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_FREE_ATTRIBS_H_
#define _SAC_FREE_ATTRIBS_H_

#include "types.h"

  </xsl:text>
  <xsl:apply-templates select="//attributetypes/type[@copy != &quot;literal&quot;]"/>
  <xsl:text>

#endif /* _SAC_FREE_ATTRIBS_H_ */

  </xsl:text>
</xsl:template>

<xsl:template match="type">
  <xsl:value-of select="'extern '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="' FREEattrib'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'( '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="'attr, node *parent );'"/>
</xsl:template>

</xsl:stylesheet>
