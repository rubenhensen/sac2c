<?xml version="1.0"?>
<!--
  $Id$
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-make-head.xsl"/>
<xsl:import href="common-make-body.xsl"/>
<xsl:import href="common-travfun.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- This stylesheet generates a node_basic.c file implementing all
     the make functions for nodes                                    -->

<xsl:template match="/">
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'node_basic.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions to allocate node structures'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text>
#include "node_basic.h"
#include "tree_basic.h"
#include "internal_lib.h"
#include "dbug.h"
#include "check_mem.h"

static node *MakeEmptyNode()
{
  node *result;

  DBUG_ENTER("MakeEmptyNode");

  result = (node *) ILIBmalloc( sizeof( node));

  NODE_ERROR( result) = NULL;

  DBUG_RETURN( result);
}

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>
  /* end of file */

  </xsl:text>
</xsl:template>

<xsl:template match="node">
  <xsl:text>

/*****************************************************************************
 * N_</xsl:text><xsl:value-of select="@name"/><xsl:text> :
 *****************************************************************************/

  </xsl:text>
  <xsl:apply-templates select="." mode="make-head"/>
  <xsl:apply-templates select="." mode="make-body"/>
</xsl:template>

</xsl:stylesheet>
