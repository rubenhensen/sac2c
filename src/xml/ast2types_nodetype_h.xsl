<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/11/24 17:47:01  sah
  update

  Revision 1.1  2004/11/23 11:32:29  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="common-travfun.xsl" />
  <xsl:import href="common-name-to-nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'types_nodetype.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the nodetype node enumeration.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_TYPES_NODETYPE_H_
#define _SAC_TYPES_NODETYPE_H_

    </xsl:text>
    <xsl:value-of select="'#define MAX_NODES '" />
    <xsl:value-of select="count(/definition/syntaxtree/node)" />
    <xsl:call-template name="newline" />
    <xsl:apply-templates select="/definition/syntaxtree" />
    <xsl:text>
#endif /* _SAC_TYPES_NODETYPE_H_ */
    </xsl:text>
  </xsl:template>

  <xsl:template match="syntaxtree" >
    <xsl:value-of select="'typedef enum { N_undefined = 0'" />
    <xsl:apply-templates select="node" />
    <xsl:value-of select="'} nodetype; '" />
  </xsl:template>

  <xsl:template match="node" >
    <xsl:value-of select="', '" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' = '" />
    <xsl:value-of select="position()" />
  </xsl:template>

</xsl:stylesheet>
