<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/08/07 16:22:29  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a free_node.h file implementing all
     functions needed to free a node -->

<xsl:template match="/">
  <xsl:text>
/*****************************************************************************
 * free_attribs.c generated by ast2free_attribs_h.xsl                        *
 *                                                                           *
 * DO NOT EDIT THIS FILE, EDIT ast.xml INSTEAD!                              *
 *****************************************************************************/

#ifndef _SAC_FREE_ATTRIBS_H
#define _SAC_FREE_ATTRIBS_H

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
  <xsl:value-of select="'( '"/>
  <xsl:value-of select="@ctype"/>
  <xsl:value-of select="' );'"/>
</xsl:template>

</xsl:stylesheet>
