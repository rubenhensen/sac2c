<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.3  2004/10/19 14:07:06  sah
  added support for persist flag

  Revision 1.2  2004/09/23 21:18:53  sah
  ongoing implementation

  Revision 1.1  2004/09/20 16:15:58  sah
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
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#ifndef _SAC_SERIALIZE_ATTRIBS_H
#define _SAC_SERIALIZE_ATTRIBS_H

#include "attribs.h"  

  </xsl:text>
  <xsl:apply-templates select="//attributetypes/type[not( @persist = &quot;no&quot;)]"/>
  <xsl:text>

#endif /* _SAC_SERIALIZE_ATTRIBS_H */
  </xsl:text>
</xsl:template>

<xsl:template match="type">
  <xsl:value-of select="'extern void Serialize'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'Attrib('"/>
  <xsl:value-of select="' info*, '"/>
  <xsl:if test="@size">
    <xsl:value-of select="' int, '" />
  </xsl:if>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="', node* );'"/>
</xsl:template>

</xsl:stylesheet>
