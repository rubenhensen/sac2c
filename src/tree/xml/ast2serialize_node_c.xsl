<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.3  2004/09/23 21:18:34  sah
  ongoing implementation

  Revision 1.2  2004/09/20 19:56:01  sah
  ongoing work

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

#include &lt;stdio.h&gt;
#include "serialize_node.h"
#include "serialize_attribs.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "tree_basic.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"

#define AST_NO_COMPAT
#include "node_compat.h"

#define TRAVNONNULL( node, info) if (node != NULL) Trav( node, info)

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
  <!-- define a cnt var if needed -->
  <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
    <xsl:value-of select="'int cnt;'" />
  </xsl:if>
  <!-- DBUG_ENTER statement -->
  <xsl:value-of select="'DBUG_ENTER( &quot;SET'"/>
  <xsl:value-of select="@name"/>
  <xsl:value-of select="'&quot;);'"/>
  <!-- DBUG PRINT -->
  <xsl:value-of select="'DBUG_PRINT( &quot;SET&quot;, (&quot;Serializing '" />
  <xsl:value-of select="@name"/>
  <xsl:value-of select="' node&quot;));'" />
  <!-- print start of new block -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;{ &quot;);'"/>
  <!-- print comment of what is to be done -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;/* serialization of '" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' node */\n&quot;);'" />
  <!-- print defines for all needed vars -->
  <xsl:apply-templates select="." mode="gen-vars"/>
  <!-- print generators of all vars -->
  <xsl:apply-templates select="." mode="gen-values"/>
  <!-- print end of block -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;}\n&quot;);'"/>
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
    <xsl:with-param name="prefix">SET</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="." /></xsl:with-param>
    <xsl:with-param name="text">Creates C code for this node type</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">SET</xsl:with-param>
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
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;node *tmp;&quot;);'" />
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
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;node *son_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' = NULL;\n&quot;);'" />
</xsl:template>

<!--
     traversal gen-vars attribute

     generates C code to print the variable declaration for an attribute.

     layout:

     - print c declaration statement for attribute
-->

<xsl:template match="attribute" mode="gen-vars">
  <!-- gen printf -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;'" />
  <!-- type of attribute -->
  <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@ctype" />
  <xsl:value-of select="' attr_'" />
  <xsl:value-of select="@name" />
  <!-- check for array types -->
  <xsl:if test="key(&quot;types&quot;, ./type/@name)/@size">
    <xsl:value-of select="'['" />
    <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size" />
    <xsl:value-of select="']'" />
  </xsl:if>
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
  <!-- Allocate node pointer -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;tmp = SHLPAllocateNode( '" />
  <xsl:call-template name="name-to-nodeenum">
    <xsl:with-param name="name">
      <xsl:value-of select="@name" />
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', %d, \&quot;%s\&quot;);\n&quot;, NODE_LINE( arg_node), NODE_FILE( arg_node));'" />
  <!-- push current node -->
  <xsl:value-of select="'SerStackPush( arg_node, INFO_SER_STACK( arg_info));'"/>
  <!-- print push for generated node -->
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;PUSH( tmp);\n&quot;);'" />
  <!-- generate all sons and attributes -->
  <xsl:apply-templates select="." mode="gen-values-dispatch"/>
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;SHLPFillNode( tmp&quot;);'" />
  <!-- add all attributes and sons as argument -->
  <xsl:apply-templates select="attributes/attribute" mode="gen-allocnode-params" />
  <xsl:apply-templates select="sons/son" mode="gen-allocnode-params"
 />
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;);\n&quot;);'" />
</xsl:template>

<!-- 
     traversal gen-values-dispatch node

     this traversal decides on which sons/attributes have to be serilized
     or not on a per node basis. This is needed to not serialize some
     sons/attributes in case they have to be freshly created or to
     have some influence on the order in which sons/attributes
     are serialized.

-->

<!-- traversal gen-values-dispatch node @name=Fundef

     For Fundef nodes, the Next and Body sons are not traversed. This is
     done to allow the serialisation to be used on a per function-header /
     function-body basis.

     Furthermore, the Return attribute is not serialized, as it points
     into the body, which is not present at that time. This link is recreated
     later on
-->

<xsl:template match="node[@name= &quot;Fundef&quot;]" mode="gen-values-dispatch">
  <xsl:apply-templates select="attributes/attribute[@name != &quot;Return&quot;]" mode="gen-values"/>
  <xsl:apply-templates select="attributes/attribute[@name = &quot;Return&quot;]" mode="gen-null"/>
  <xsl:apply-templates select="sons/son[@name != &quot;Next&quot;][@name != &quot;Body&quot;]" mode="gen-values" />
  <xsl:apply-templates select="sons/son[@name = &quot;Next&quot;]" mode="gen-null" />
  <xsl:apply-templates select="sons/son[@name = &quot;Body&quot;]" mode="gen-null" />
</xsl:template>

<xsl:template match="node" mode="gen-values-dispatch">
  <xsl:apply-templates select="attributes/attribute" mode="gen-values"/>
  <xsl:apply-templates select="sons/son" mode="gen-values"/>
</xsl:template>

<xsl:template match="attributes/attribute" mode="gen-values">
  <!-- if it is an array, we have to build a for loop over its elements -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
    <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
    <xsl:value-of select="'; cnt++) { '" />
  </xsl:if>
  <!-- call serialization function for attribute -->
  <xsl:value-of select="'Serialize'" />
  <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@name" />
  <xsl:value-of select="'Attrib( &quot;attr_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="'&quot;, arg_info, '" />
  <!-- if it is an array, we need to pass the selector as well -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'cnt, '" />
  </xsl:if>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
    <!-- if its is an array, we have to add another parameter -->
    <xsl:with-param name="index">
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="'cnt'"/>
      </xsl:if>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', arg_node);'" />
  <!-- if it is an array, we have to complete the for loop -->
  <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
    <xsl:value-of select="'}'"/>
  </xsl:if>
</xsl:template>

<xsl:template match="sons/son" mode="gen-values">
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
  <xsl:value-of select="' == NULL)'" />
  <!-- generate code for a NULL pointer son -->
  <xsl:value-of select="'{ SerStackPush( NULL, INFO_SER_STACK( arg_info));'"/>
  <xsl:value-of select="' fprintf( INFO_SER_FILE( arg_info), &quot;PUSH( NULL);&quot;); }'" />
  <!-- call Trav otherwise -->
  <xsl:value-of select="'else { Trav( '" />
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
  <xsl:value-of select="' fprintf(INFO_SER_FILE(arg_info), &quot;son_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' = LOOKUP( %d);\n&quot;, SerStackFindPos( '"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="', INFO_SER_STACK( arg_info)));'" />
</xsl:template>

<!-- 
     traversal gen-null

     This traversal is used to assign a NULL pointer to certain
     links which are not traversed.

     see gen-values-dispatch
-->

<xsl:template match="attributes/attribute" mode="gen-null">
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;attr_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' = NULL;&quot;);'"/>
</xsl:template>

<xsl:template match="sons/son" mode="gen-null">
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;son_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="' = NULL;&quot;);'"/>
</xsl:template>

<!--
     traversal gen-allocnode-params attributes/attribute

     generates the name of an attr field for the argumentlist
     of the FillNode function.

     result:

     fprintf( INFO_SER_FILE( arg_info), &quot;, attr_Name&quot;);
-->

<xsl:template match="attributes/attribute" mode="gen-allocnode-params">
  <xsl:choose>
    <xsl:when test="key(&quot;arraytypes&quot;, ./type/@name)">
      <!-- array attributes have the size as first argument -->
      <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;,'" />
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size" />
      <xsl:value-of select="'&quot;);'" />
      <!-- generate for loop -->
      <xsl:value-of select="'for (cnt=0; cnt &lt; '"/>
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size" />
      <xsl:value-of select="'; cnt++) {'" />
      <!-- add each arg -->
      <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, attr_'" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="'[%d]&quot;, cnt);'"/>
      <!-- end of for loop -->
      <xsl:value-of select="'}'" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, attr_'" />
      <xsl:value-of select="@name" />
      <xsl:value-of select="'&quot;);'" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- 
     traversal gen-allocnode-params sons/son

     generates the name of a son field for the argumentlist
     of the FillNode function.

     result:

     , son_Next
-->

<xsl:template match="sons/son" mode="gen-allocnode-params">
  <xsl:value-of select="'fprintf( INFO_SER_FILE( arg_info), &quot;, son_'" />
  <xsl:value-of select="@name" />
  <xsl:value-of select="'&quot;);'" />
</xsl:template>
</xsl:stylesheet>
