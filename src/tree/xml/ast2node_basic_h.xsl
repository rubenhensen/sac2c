<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.5  2004/08/08 16:18:35  sah
  doxygen improvement

  Revision 1.4  2004/08/07 16:19:05  sah
  most xsl files use key-tables for type lookups
  now which increases speed significantly.
  lots of small improvements

  Revision 1.3  2004/07/31 16:16:57  sah
  added support for flags and moved to memory saving attribute
  structure.

  Revision 1.2  2004/07/11 18:24:57  sah
  modularizes the templates
  added support for default values and init values
  work-in-progress !

  Revision 1.1  2004/07/03 15:14:59  sah
  Initial revision

-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-accessor-macros.xsl"/>
<xsl:import href="common-make-head.xsl"/>
<xsl:import href="common-flag-defines.xsl"/>
<xsl:import href="common-travfun.xsl"/>

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
 * macros and functions for N_</xsl:text><xsl:value-of select="@name"/><xsl:text>
 *****************************************************************************/

  </xsl:text>
  <xsl:apply-templates select="sons/son" mode="accessor-macros"/>
  <xsl:apply-templates select="attributes/attribute" mode="accessor-macros"/>
  <xsl:apply-templates select="flags" mode="flag-defines"/>
  <xsl:apply-templates select="." mode="make-head"/>
</xsl:template>

<xsl:template match="node" mode="make-head">
  <xsl:value-of select="'extern '"/>
  <xsl:apply-imports/>
  <xsl:value-of select="';'"/>
</xsl:template>

</xsl:stylesheet>
