<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.4  2004/08/29 18:10:05  sah
  general improvements

  Revision 1.3  2004/08/08 16:18:35  sah
  doxygen improvement

  Revision 1.2  2004/08/08 16:07:21  sah
  beautified generated code
  include more doxygen comments

  Revision 1.1  2004/08/07 16:22:29  sah
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
#ifndef _SAC_FREE_ATTRIBS_H
#define _SAC_FREE_ATTRIBS_H

#include "attribs.h"  

  </xsl:text>
  <xsl:apply-templates select="//attributetypes/type[@copy != &quot;literal&quot;]"/>
  <xsl:text>
#endif /* _SAC_FREE_NODE_H */
  </xsl:text>
</xsl:template>

<xsl:template match="type">
  <xsl:value-of select="'extern '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="' Free'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'Attrib( '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="' );'"/>
</xsl:template>

</xsl:stylesheet>
