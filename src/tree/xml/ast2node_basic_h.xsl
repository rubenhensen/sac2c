<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.2  2004/07/11 18:24:57  sah
  modularizes the templates
  added support for default values and init values
  work-in-progress !

  Revision 1.1  2004/07/03 15:14:59  sah
  Initial revision

-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-accessor-macros.xsl"/>
<xsl:import href="common-make-head.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a node_basic.h file defining all the macros
     to the different attributes and the MakeXXX functions for nodes       -->

<xsl:template match="/">
  <xsl:text>
/*****************************************************************************
 * node_basic.h generated by ast2node_basic_h.xsl                            *
 *                                                                           *
 * DO NOT EDIT THIS FILE, EDIT ast.xml INSTEAD!                              *
 *****************************************************************************/

#ifndef _NODE_BASIC_H
#define _NODE_BASIC_H

#include "tree_basic.h"
#include "types.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "globals.h"

#define AST_NO_COMPAT
#include "node_compat.h"
  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>

#undef AST_NO_COMPAT
#include "node_compat.h"
#endif
  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:text>

/*****************************************************************************
 * macros and functions for N_</xsl:text><xsl:value-of select="@name"/><xsl:text>
 *****************************************************************************/

  </xsl:text>
  <xsl:apply-templates select="sons/son" mode="accessor-macros"/>
  <xsl:apply-templates select="attributes/attribute" mode="accessor-macros"/>
  <xsl:apply-templates select="." mode="make-head"/>
</xsl:template>

<xsl:template match="node" mode="make-head">
  <xsl:value-of select="'extern '"/>
  <xsl:apply-imports/>
  <xsl:value-of select="';'"/>
</xsl:template>

</xsl:stylesheet>
