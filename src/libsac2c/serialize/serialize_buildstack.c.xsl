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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="../xml/common_key_tables.xsl"/>
<xsl:import href="../xml/common_travfun.xsl"/>
<xsl:import href="../xml/common_node_access.xsl"/>
<xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- 
     This file generates all functions needed for the
     serialize_buildstack traversal. All nodes are
     pushed onto a stack, except those which are referenced
     by a link only. This way, the whole tree can be serialized
     and then links within the tree (which formally is a graph then)
     can be represented by edges between elements on the stack.

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
      <xsl:value-of select="'serialize_buildstack.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize buildstack traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'serializebst'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Serialize Build Stack Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize buildstack traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes -->
  <xsl:text>

#include &lt;stdio.h&gt;
#include "serialize_buildstack.h"
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

     puts the node onto the stack and traverses into all sons.
     Afterwards, node attributes are processed.

     layout of output:

     - function head and comment
       - call templates for @name
     - function body
       - call templates for sons
       - call templates for attributes of type node

     remarks:

-->
<xsl:template match="node">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;SBT'"/>
  <xsl:call-template name="lowercase" >
    <xsl:with-param name="string" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="'&quot;);'"/>
  <!-- DBUG PRINT -->
  <xsl:value-of select="'DBUG_PRINT( &quot;SBT&quot;, (&quot;Stacking '" />
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' node&quot;));'" />
  <!-- push this node onto the stack -->
  <xsl:value-of select="'SSpush( arg_node, INFO_SER_STACK( arg_info));'" />
  <!-- handle sons -->
  <xsl:apply-templates select="sons" />
  <!-- handle attributes -->
  <xsl:apply-templates select="attributes/attribute" />
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
    <xsl:with-param name="prefix">SBT</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Puts this node onto the stack</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">SBT</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
  </xsl:call-template>
</xsl:template>

<!--
     traversal main sons

     dispatches the handling of sons depending on node type
-->
<xsl:template match="sons[../@name=&quot;Fundef&quot;]">
  <xsl:apply-templates select="son[not( @name = &quot;Next&quot;)][not( @name = &quot;Body&quot;)]" />
</xsl:template>

<xsl:template match="sons[../@name=&quot;Objdef&quot;]">
  <xsl:apply-templates select="son[not( @name = &quot;Next&quot;)]" />
</xsl:template>

<xsl:template match="sons[../@name=&quot;Typedef&quot;]">
  <xsl:apply-templates select="son[not( @name = &quot;Next&quot;)]" />
</xsl:template>

<xsl:template match="sons">
  <xsl:apply-templates select="son" />
</xsl:template>

<!--
     traversal main son

     continues tree traversal in son node
-->
<xsl:template match="son">
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
  <xsl:value-of select="'= TRAVdo( '" />
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
  <xsl:value-of select="', arg_info);}'" />
</xsl:template>
  
     
<!--
     traversal main attribute of type Node

     Traverses the sub tree behind the node attribute
-->
<xsl:template match="attributes/attribute[type/@name = &quot;Node&quot;]" >
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
  <xsl:value-of select="'= TRAVdo( '" />
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
  <xsl:value-of select="', arg_info);}'" />
</xsl:template>

<!--
     template main attribute

     stops further traversal of attrubutes
-->
<xsl:template match="attribute" />
</xsl:stylesheet>
