<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/23 11:31:00  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a serialize_attribs.h file defining all
     functions needed to serialize an node attrib -->

<xsl:template match="/">
<xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'serialize_attribs.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to serialize the attributes of node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id: ast2serialize_attribs_h.xsl 14294 2005-10-10 12:40:03Z sah $'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_SERIALIZE_ATTRIBS_H_
#define _SAC_SERIALIZE_ATTRIBS_H_

#include "types.h"  

  </xsl:text>
  <xsl:apply-templates select="//attributetypes/type[not( @persist = &quot;no&quot;)]"/>
  <xsl:text>

#endif /* _SAC_SERIALIZE_ATTRIBS_H_ */
  </xsl:text>
</xsl:template>

<xsl:template match="type">
  <xsl:value-of select="'extern void SATserialize'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'('"/>
  <xsl:value-of select="' info*, '"/>
  <xsl:if test="@size">
    <xsl:value-of select="' int, '" />
  </xsl:if>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="', node* );'"/>
</xsl:template>

</xsl:stylesheet>
