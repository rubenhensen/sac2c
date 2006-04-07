<?xml version="1.0"?>
<!--
  $Id$
  Revision 1.1  2006/04/04 14:43:00 jhb
  Initial revision
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common-travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a check_attribs.h file defining all
     functions needed to free the attributes of a node -->

<xsl:template match="/">
<xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'check_attribs.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to free the attributes of node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id: check_attribs.h.xsl 14593 2006-01-31 17:09:55Z cg $'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_CHECK_ATTRIBS_H_
#define _SAC_CHECK_ATTRIBS_H_

#include "types.h"

  </xsl:text>
  <xsl:apply-templates select="//attributetypes/type[@copy != &quot;literal&quot;]"/>
  <xsl:text>

#endif /* _SAC_CHECK_ATTRIBS_H_ */

  </xsl:text>
</xsl:template>

<xsl:template match="type">
  <xsl:value-of select="'extern '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="' CHKMattrib'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'( '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="'attr, info * arg_info);'"/>
</xsl:template>

</xsl:stylesheet>
