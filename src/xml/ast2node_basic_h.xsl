<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/11/23 11:30:50  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-accessor-macros.xsl"/>
<xsl:import href="common-make-head.xsl"/>
<xsl:import href="common-travfun.xsl"/>
<xsl:import href="common-name-to-nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a node_basic.h file defining all the macros
     to the different attributes and the MakeXXX functions for nodes       -->

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'node_basic.h'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to allocate node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
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
 * macros and functions for </xsl:text>
  <xsl:call-template name="name-to-nodeenum" >
    <xsl:with-param name="name" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template><xsl:text>
 *****************************************************************************/

  </xsl:text>
  <xsl:apply-templates select="sons/son" mode="accessor-macros"/>
  <xsl:apply-templates select="attributes/attribute" mode="accessor-macros"/>
  <xsl:apply-templates select="flags" mode="accessor-macros"/>
  <xsl:apply-templates select="." mode="make-head"/>
</xsl:template>

<xsl:template match="node" mode="make-head">
  <xsl:value-of select="'extern '"/>
  <xsl:apply-imports/>
  <xsl:value-of select="';'"/>
</xsl:template>

</xsl:stylesheet>
