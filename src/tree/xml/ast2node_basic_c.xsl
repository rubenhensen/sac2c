<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.5  2004/08/08 16:07:21  sah
  beautified generated code
  include more doxygen comments

  Revision 1.4  2004/08/07 16:19:05  sah
  most xsl files use key-tables for type lookups
  now which increases speed significantly.
  lots of small improvements

  Revision 1.3  2004/08/06 14:39:48  sah
  some ast improvements

  Revision 1.2  2004/07/11 18:24:57  sah
  modularizes the templates
  added support for default values and init values
  work-in-progress !


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
#ifdef NEW_AST
#include "node_basic.h"
#include "tree_basic.h"
#include "internal_lib.h"
#include "dbug.h"
#include "Error.h"

#define AST_NO_COMPAT
#include "node_compat.h"

  </xsl:text>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>
#undef AST_NO_COMPAT
#include "node_compat.h"

#endif /* NEW_AST */
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
