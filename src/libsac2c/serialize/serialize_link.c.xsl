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
<xsl:import href="../xml/common_travfun.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- 
     This stylesheet generates a serialize_link.c file implementing all
     functions needed to serialize links within the node structures of 
     the ast to C code. 

     templates:

     traversals:

     main         generates the complete file. see templates for
                  details.
-->

<!--
     traversal main /

     The output is generated using the following layout:

     - doxygen file header
     - doxygen file group tag
     - includes
     - call to subtemplates to generate the functions itself
     - doxygen file group end tag

-->
<xsl:template match="/">
  <!-- generate file header and doxygen group -->
  <xsl:call-template name="travfun-file">
    <xsl:with-param name="file">
      <xsl:value-of select="'serialize_link.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize link traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'serializelink'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Serialize Tree Links Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize link traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes -->
  <xsl:text>

#include &lt;stdio.h&gt;
#include "serialize_node.h"
#include "serialize_attribs.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

  </xsl:text>
  <!-- functions -->
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<!-- 
     traversal main node

     generates a serialize link function for each node type

     layout of output:

     - function head and comment
       - call templates for @name
     - function body
       - call templates for sons

     remarks:

-->

<xsl:template match="node">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;SET'"/>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'&quot;);'" />
  <!-- serialize link attributes -->
  <xsl:apply-templates select="attributes/attribute[type/@name=&quot;Link&quot;] | attributes/attribute[type/@name=&quot;CodeLink&quot;]" />
  <!-- trav sons -->
  <xsl:apply-templates select="sons" />
  <!-- trav node attributes -->
  <xsl:apply-templates select="attributes/attribute[type/@name=&quot;Node&quot;]" mode="trav-node-attribs" />
  <!-- DBUG_RETURN call -->
  <xsl:value-of select="'DBUG_RETURN( arg_node);'"/>
  <!-- end of body -->
  <xsl:value-of select="'}'"/>
</xsl:template>

<!--
     traversal main @name

     generates a comment and function head

     layout:
   
     - call travfun-comment template
     - call travfun-head template
-->

<xsl:template match="@name">
  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">SEL</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Creates C code for links within this node type</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">SEL</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template match="attributes/attribute" >
  <!-- only process those attributes not beeing NULL -->
  <xsl:value-of select="'if ('" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'arg_node'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' != NULL) {'" />
  <!-- and only those, where the target is on the stack -->
  <xsl:value-of select="'if (SSfindPos( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'arg_node'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', INFO_SER_STACK( arg_info)) != SERSTACK_NOT_FOUND) {'" />
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;/* fix link for '" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' attribute */\n&quot;);'" />
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;SHLPfixLink( stack, %d, '" />
  <xsl:value-of select="position()" />
  <xsl:value-of select="', %d);\n&quot;, SSfindPos( arg_node, INFO_SER_STACK( arg_info)) , SSfindPos( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">
      <xsl:value-of select="'arg_node'" />
    </xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name" />
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', INFO_SER_STACK( arg_info))); }}'" />
</xsl:template>

<xsl:template match="sons[../@name=&quot;Fundef&quot;]" >
  <xsl:apply-templates select="son[not( @name = &quot;Next&quot;)][not( @name = &quot;Body&quot;)]" />
</xsl:template>

<xsl:template match="sons[../@name=&quot;Typedef&quot;]" >
  <xsl:apply-templates select="son[not( @name = &quot;Next&quot;)]" />
</xsl:template>

<xsl:template match="sons[../@name=&quot;Objdef&quot;]" >
  <xsl:apply-templates select="son[not( @name = &quot;Next&quot;)]" />
</xsl:template>

<xsl:template match="sons" >
  <xsl:apply-templates select="son" />
</xsl:template>

<xsl:template match="son" >
  <!-- check for NULL pointer son -->
  <xsl:value-of select="'if ( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' != NULL) {'" />
  <xsl:value-of select="'TRAVdo( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_info); }'" />
</xsl:template>

<xsl:template match="attributes/attribute" mode="trav-node-attribs" >
  <!-- check for NULL pointer son -->
  <xsl:value-of select="'if ( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="' != NULL) {'" />
  <xsl:value-of select="'TRAVdo( '" />
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_info); }'" />
</xsl:template>

</xsl:stylesheet>
