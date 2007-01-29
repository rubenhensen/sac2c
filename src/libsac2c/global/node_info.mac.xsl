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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="../xml/common_travfun.xsl" />
  <xsl:import href="../xml/common_name_to_nodeenum.xsl" />

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'node_info.mac'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the node to nodename mapping.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef NIFname
#define NIFname( it_name)
#endif

#define NIF( it_name) NIFname( it_name)

    </xsl:text>
    <xsl:apply-templates select="/definition/syntaxtree" />
    <xsl:text>

#undef NIFname
#undef NIF

    </xsl:text>
  </xsl:template>

  <xsl:template match="syntaxtree" >
    <xsl:value-of select="'NIF( &quot;undefined&quot; )'" />
    <xsl:call-template name="newline" />
    <xsl:apply-templates select="node" />
  </xsl:template>

  <xsl:template match="node" >
    <xsl:value-of select="', NIF( &quot;'" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'&quot;)'" />
    <xsl:call-template name="newline" />
  </xsl:template>

</xsl:stylesheet>
