<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/08/31 14:23:46  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:import href="common-key-tables.xsl"/>
<xsl:import href="common-travfun.xsl"/>
<xsl:import href="common-node-access.xsl"/>
<xsl:import href="common-name-to-nodeenum.xsl"/>

<xsl:output method="text" indent="no"/>
<xsl:strip-space elements="*"/>

<!-- 
     This stylesheet generates a serialize_node.c file implementing all
     functions needed to serialize the node structures of the ast to
     C code. 

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
      <xsl:value-of select="'serialize_node.c'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize traversal.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'serialize'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Serialize Tree Functions.'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by serialize traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes -->
  <xsl:text>

#define NEW_INFO

#include "serialize_node.h"
#include "serialize_attribs.h"
#include "serialize_info.h"
#include "tree_basic.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"

#define AST_NO_COMPAT
#include "node_compat.h"

  </xsl:text>
  <!-- functions -->
  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:text>
#undef AST_NO_COMPAT
#include "node_compat.h"
  </xsl:text>
  <!-- end of doxygen group -->
  <xsl:call-template name="travfun-group-end"/>
</xsl:template>

<!-- 
     traversal main node[@name = "Fundef"]

     generates a free functions for Fundef nodes

     layout of output:

     - function head and comment
       - call templates for @name
     - function body
       - call templates for sons
       - call templates for attributes except those needed for zombies

     remarks:

-->
<xsl:template match="node">
  <!-- generate head and comment -->
  <xsl:apply-templates select="@name"/>
  <!-- start of body -->
  <xsl:value-of select="'{'"/>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;Serialize'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- print start of new block -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;{\n&quot;);'"/>
  <!-- print comment of what is to be done -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;/* serialization of '" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' node /*\n&quot;);'" />
  <!-- print defines for all needed vars -->
  <xsl:apply-templates select="." mode="gen-vars"/>
  <!-- print generators of all vars -->
  <xsl:apply-templates select="." mode="gen-values"/>
  <!-- print end of block -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;}\n&quot;);'"/>
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
    <xsl:with-param name="prefix">Serialize</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Creates C code for this node type</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">Serialize</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
  </xsl:call-template>
</xsl:template>

<!--
     traversal gen-vars node

     generates C code to print the variable declaration for the current node.

     layout:

     - print c declaration statement for node
-->

<xsl:template match="node" mode="gen-vars">
  <!-- gen printf -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;node *tmp;\n&quot;);'" />
  <!-- continue traversal -->
  <xsl:apply-templates select="sons/son" mode="gen-vars" />
  <xsl:apply-templates select="attributes/attribute" mode="gen-vars" />
</xsl:template>

<!--
     traversal gen-vars son

     generates C code to print the variable declaration for a son.

     layout:

     - print c declaration statement for node
-->

<xsl:template match="son" mode="gen-vars">
  <!-- gen printf -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;node *son_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="';\n&quot;);'" />
</xsl:template>

<!--
     traversal gen-vars attribute

     generates C code to print the variable declaration for an attribute.

     layout:

     - print c declaration statement for attribute
-->

<xsl:template match="attribute" mode="gen-vars">
  <!-- gen printf -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;'" />
  <!-- type of attribute -->
  <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@ctype" />
  <xsl:value-of select="' attr_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="';\n&quot;);'" />
</xsl:template>

<!--
     traversal gen-values node

     generates C code to print code allocating a node structure. The current
     node is pushed to the reference stack. As well, code is printed that
     pushes the generated node to the reference stack.

     layout:

     - print node allocation statement
     - push current node
     - print push for current node
-->

<xsl:template match="node" mode="gen-values">
  <!-- allocate node -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;tmp = AllocateNode( '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name"><xsl:value-of select="@name"/></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="');\n&quot;);'" />
  <!-- push current node -->
  <xsl:value-of select="'PUSH( arg_node);'"/>
  <!-- print push for generated node -->
  <xsl:value-of select="'fprintf( INFO_FILE( arg_info), &quot;PUSH( tmp);\n&quot;);'" />
</xsl:template>

</xsl:stylesheet>
