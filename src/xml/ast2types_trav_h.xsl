<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/23 11:32:38  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="common-key-tables.xsl"/>
  <xsl:import href="common-travfun.xsl"/>
  <xsl:import href="common-name-to-nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'types_trav.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the trav_t phase enumeration.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_TYPES_TRAV_H_
#define _SAC_TYPES_TRAV_H_

    </xsl:text>
    <xsl:apply-templates select="/definition/traversals" />
    <xsl:text>
#endif /* _SAC_TYPES_TRAV_H_ */
    </xsl:text>
  </xsl:template>

  <xsl:template match="traversals" >
    <xsl:value-of select="'typedef enum { TR_undefined'" />
    <xsl:apply-templates select="traversal" />
    <xsl:value-of select="'} trav_t; '" />
  </xsl:template>

  <xsl:template match="traversal" >
    <xsl:value-of select="', TR_'" />
    <xsl:call-template name="lowercase" >
      <xsl:with-param name="string" >
        <xsl:value-of select="@id" />
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

</xsl:stylesheet>
