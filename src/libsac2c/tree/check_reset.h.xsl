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

<!-- This stylesheet generates a checktst.h file implementing all functions needed to check a node -->

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
 * @file checktst.h
 *
 * Functions to CheckTest node structures
 * 
 * THIS FILE HAS BEEN GENERATED USING 
 * check_rest.h.xsl
 * DO NOT EDIT THIS FILE. EDIT THE AST SPEC IN ast.xml INSTEAD!
 *
 * ALL CHANGES MADE TO THIS FILE WILL BE OVERWRITTEN!
 *
 */

#ifndef _SAC_CHECK_RESET_H_
#define _SAC_CHECK_RESET_H_

#include "types.h"

extern node *CHKRSTdoTreeCheckReset( node *syntax_tree);
</xsl:text> 
<xsl:value-of select="$newline"/>

  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>


  <xsl:apply-templates select="//nodesets/nodeset">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>

<xsl:text>

#endif /* _SAC_CHECK_RESET_H_ */
</xsl:text>

</xsl:template>

<xsl:template match="node">

<xsl:value-of select="'extern node *CHKRST'"/>
  <xsl:value-of select="translate(@name, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz')"/>
  <xsl:value-of select="'( node *arg_node, info *arg_info)'"/>
  <xsl:value-of select="';'"/>
  <xsl:value-of select="$newline"/>
</xsl:template>
 
</xsl:stylesheet>
