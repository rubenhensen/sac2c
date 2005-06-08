<?xml version="1.0"?>

<!--
  $Log$
  Revision 1.3  2005/06/08 13:33:03  jhb
  attribute are now check correctly

  Revision 1.2  2005/05/17 13:00:37  jhb
  added the isfun

  Revision 1.1  2005/02/10 12:59:58  jhb
  Initial revision

  Revision 1.1  2004/11/19 13:55:29  jhb
  Initial revision

  Revision 1.1 2004/09/29 15:15:00 jhb
  Initial revision

-->

  <!-- This stylesheet generates a check.h file implementing all functions needed to check a node -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

 <xsl:variable name="newline">
<xsl:text>
</xsl:text>
 </xsl:variable>

<xsl:template match="/">

 <xsl:text>
/**
 * @file check.h
 *
 * Functions to check node structures
 * 
 * THIS FILE HAS BEEN GENERATED USING 
 * $Id$.
 * DO NOT EDIT THIS FILE. EDIT THE AST SPEC IN ast.xml INSTEAD!
 *
 * ALL CHANGES MADE TO THIS FILE WILL BE OVERWRITTEN!
 *
 */

#ifndef _SAC_CHECK_H_
#define _SAC_CHECK_H_

#include "types.h"

</xsl:text> 
<xsl:value-of select="$newline"/>

  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>


  <xsl:apply-templates select="//nodesets/nodeset">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>

<xsl:text>

#endif /* _SAC_CHECK_H_ */
</xsl:text>

</xsl:template>

<xsl:template match="node">

<xsl:value-of select="'extern node *CHK'"/>
  <xsl:value-of select="translate(@name, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
  <xsl:value-of select="'( node *arg_node, info *arg_info)'"/>
  <xsl:value-of select="';'"/>
  <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="nodeset">
  <xsl:value-of select="'extern bool is'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'( node *arg_node)'"/>
  <xsl:value-of select="';'"/>
  <xsl:value-of select="$newline"/> 
</xsl:template>

</xsl:stylesheet>