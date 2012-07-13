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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_key_tables.xsl"/>
<xsl:import href="../xml/common_accessor_macros.xsl"/>
<xsl:import href="../xml/common_make_head.xsl"/>
<xsl:import href="../xml/common_make_head_checkmem.xsl"/>
<xsl:import href="../xml/common_travfun.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

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
#ifndef _SAC_NODE_BASIC_H_
#define _SAC_NODE_BASIC_H_

#ifndef _SAC_TREE_BASIC_H_
#error "node_basic.h should only be included as part of tree_basic.h!"
#endif

#ifdef INLINE_MACRO_CHECKS
/**
 * This function is inlined at each macro call to check whether the 
 * nodetype matches. To allow us to print elaborate error messages
 * instead of a segfault, we make use of the nodeenum to nodename 
 * mapping. As this is a header file, and as header files are only
 * allowed to reference types.h, we add an explicit declaration
 * of the globals data type. For the same reason, we cannot use 
 * DBUG_ASSERT directly, but need to mimic it.
 */

extern global_t global;

static inline
node *NBMacroMatchesType( node *node, nodetype type)
{
#ifndef DBUG_OFF
  if ((node != NULL) &amp;&amp; (node->mnodetype != type)) {
    const char *ndtp_name = ((node->mnodetype &lt;= MAX_NODES))
                            ? global.mdb_nodetype[node->mnodetype] : "!invalid!";
    printf( "TRAVERSE ERROR: node of type %d:%s found where %d:%s was expected!\n\n",
            node->mnodetype, ndtp_name,
            type, global.mdb_nodetype[type]);
    fflush(stdout);
    *((int *) 0) = 1; /* segfault */
  }
#endif

  return( node);
}
#endif /* INLINE_MACRO_CHECKS */
  </xsl:text>
  <xsl:apply-templates select="/definition/@version"/>
  <xsl:apply-templates select="//syntaxtree/node"/>
  <xsl:text>


    #endif /* _SAC_NODE_BASIC_H_ */
  </xsl:text>
</xsl:template>

<xsl:template match="@version">
  <xsl:call-template name="newline"/>
  <xsl:value-of select="'#define _SAC_AST_VERSION_ &quot;'"/>
  <xsl:value-of select="." />
  <xsl:value-of select="'&quot;'"/>
  <xsl:call-template name="newline"/>
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
  <xsl:apply-templates select="sons" mode="accessor-macros"/>
  <xsl:apply-templates select="attributes" mode="accessor-macros"/>
  <xsl:apply-templates select="flags" mode="accessor-macros"/>
  <xsl:value-of select="'extern '"/>
  <xsl:apply-templates select="." mode="make-head-checkmem"/>
  <xsl:value-of select="';'"/>
  <xsl:apply-templates select="." mode="make-head-checkmem-define"/>
</xsl:template>


<!-- 
<xsl:template match="node" mode="make-head">
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'extern '"/>
  <xsl:apply-imports/>
  <xsl:value-of select="';'"/>
</xsl:template>
-->
</xsl:stylesheet>
