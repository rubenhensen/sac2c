<?xml version="1.0"?>

<!--
  $Log$
  Revision 1.7  2005/02/08 18:40:51  jhb
  matched to the functions of Stephan - no redundance

  Revision 1.6  2005/02/07 16:09:09  jhb
  little things changed

  Revision 1.5  2005/02/03 16:08:33  jhb
  change Name  of function

  Revision 1.4  2005/01/21 13:45:48  jhb
  some little things

  Revision 1.3  2005/01/18 14:08:52  jhb
  added enums and fix some bugs

  Revision 1.2  2005/01/11 13:32:08  jhb
  changed some little things

  Revision 1.1  2004/11/23 11:29:56  sah
  Initial revision

  Revision 1.1  2004/11/19 13:54:26  jhb
  Initial revision

  Revision 1.1 2004/09/29 15:15:00 jhb
  Initial revision
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

  <xsl:import href="common-travfun.xsl"/>
  <xsl:import href="common-node-access.xsl"/>
  <xsl:import href="common-c-code.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>
  
  <!-- This stylesheet generates a check.c file implementing all functions needed to check a node -->
  
  <xsl:variable name="newline">
    <xsl:text>
    </xsl:text>
  </xsl:variable>
  
  <xsl:param name="nodeup"/>
  <xsl:param name="nodelo"/>
  <xsl:param name="sonatt"/>

  <xsl:template match="/">
    <!-- generate file header and doxygen group -->
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'check.c'"/>
      </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by check.'"/>
    </xsl:with-param>
    <xsl:with-param name="xslt">
      <xsl:value-of select="'$Id$'"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="travfun-group-begin">
    <xsl:with-param name="group">
      <xsl:value-of select="'check'"/>
    </xsl:with-param>
    <xsl:with-param name="name">
      <xsl:value-of select="'Check tree Functions'"/>
    </xsl:with-param>
    <xsl:with-param name="desc">
      <xsl:value-of select="'Functions needed by check traversal.'"/>
    </xsl:with-param>
  </xsl:call-template>
  <!-- includes -->
  <xsl:text>

#define NEW_INFO

#include "check.h"
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "check_lib.h"
#include "types.h"

#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "internal_lib.h"



struct INFO
{
};

static info *MakeInfo()
{
  info *result;

  DBUG_ENTER("MakeInfo");

  result = ILIBmalloc(sizeof(info));

  DBUG_RETURN(result);
} 

static info *FreeInfo(info *info)
{
  DBUG_ENTER("FreeInfo");

  info = ILIBfree(info);

  DBUG_RETURN(info);
}

</xsl:text>

  <xsl:value-of select="$newline"/>
  <xsl:apply-templates select="//syntaxtree/node" mode="function">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:value-of select="$newline"/>
  <!--
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'enum{ '"/> 
  <xsl:apply-templates select="//syntaxtree/node" mode="enum_node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:value-of select="'}; '"/>
  <xsl:value-of select="$newline"/>

  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'enum{ '"/> 
  <xsl:apply-templates select="//syntaxtree/node/sons/son" mode="enum_son">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:value-of select="'}; '"/>
  <xsl:value-of select="$newline"/>

  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'enum{ '"/> 
  <xsl:apply-templates select="//syntaxtree/node/attributes/attribute" 
                       mode="enum_attribute">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  <xsl:value-of select="'}; '"/>
  <xsl:value-of select="$newline"/>
  -->

  </xsl:template>


<xsl:template match="node" mode="function">

  <xsl:call-template name="travfun-comment">
    <xsl:with-param name="prefix">CHK</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
    <xsl:with-param name="text">Check the node and its sons/attributes</xsl:with-param>
  </xsl:call-template>  
  <xsl:call-template name="travfun-head">
    <xsl:with-param name="prefix">CHK</xsl:with-param>
    <xsl:with-param name="name"><xsl:value-of select="@name" /></xsl:with-param>
  </xsl:call-template>
  <xsl:value-of select="$newline"/>   
  <xsl:value-of select="'{'"/>  
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  DBUG_ENTER'"/>
  <xsl:value-of select="'( &quot;CHK'"/>

  <xsl:call-template name="lowercase">
    <xsl:with-param name="string" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>

  <xsl:value-of select="'&quot;);'"/>
  <xsl:value-of select="$newline"/>
  
  <xsl:apply-templates select="./sons/son" mode="check">
  <xsl:sort select="@name"/>
  </xsl:apply-templates>

  <xsl:apply-templates select="./attributes/attribute">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>

  <xsl:apply-templates select="./sons/son" mode="trav">
  <xsl:sort select="@name"/>
  </xsl:apply-templates>

  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  DBUG_RETURN( arg_node);'"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'}'"/>
  <xsl:value-of select="$newline"/>

</xsl:template>



<xsl:template match="node" mode="enum_node">


  <xsl:value-of select="@name"/>
  <xsl:value-of select="', '"/>
   
</xsl:template>


<xsl:template match="node" mode="enum_son">

  <xsl:value-of select="@name"/>
  <xsl:value-of select="', '"/>
   
</xsl:template>


<xsl:template match="node" mode="enum_attribute">

  <xsl:value-of select="@name"/>
  <xsl:value-of select="', '"/>
   
</xsl:template>





<!-- ckeck the sons -->
<xsl:template match="son" mode="check">
  
  <xsl:if test="string-length(@mandatory) &gt; 2">
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="' /* this son is mandatory = '"/>
    <xsl:value-of select="@mandatory"/>
    <xsl:value-of select="' */ '"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  CHKexistSon( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="', '"/>
    <xsl:value-of select="'arg_node'"/>
    <xsl:value-of select="', '"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'                 '"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'mandatory son '"/>
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'_'"/>
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' is NULL'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="');'"/>
    <xsl:value-of select="$newline"/>
  </xsl:if>
  
</xsl:template>


<!-- trav to the sons -->
<xsl:template match="son" mode="trav">
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  if ( '"/>
  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' != NULL)'"/>
    <xsl:value-of select="' {'"/>
    <xsl:value-of select="$newline"/>
    <xsl:text>    </xsl:text>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    
    <xsl:value-of select="'= TRAVdo( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>
  
  <xsl:value-of select="', arg_info);'"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  }'"/>
  <xsl:value-of select="$newline"/>

</xsl:template>


<xsl:template match="attribute">
  
  <xsl:if test="string-length(@mandatory) &gt; 2">
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  /* this attribute is mandatory = '"/>
    <xsl:value-of select="@mandatory"/>
    <xsl:value-of select="' */'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  CHKexistAttribute( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="', '"/>
    <xsl:value-of select="'arg_node'"/>
    <xsl:value-of select="', '"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'                     '"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'mandatory attribute '"/>
    <xsl:apply-templates select="../../@name" mode="uppercase"/>
    <xsl:value-of select="'_'"/>
    <xsl:apply-templates select="@name" mode="uppercase"/>
    <xsl:value-of select="' is NULL'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="');'"/>
    <xsl:value-of select="$newline"/>
  </xsl:if> 
  
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  CHKrightType( '"/>

  <xsl:call-template name="node-access">
    <xsl:with-param name="node">arg_node</xsl:with-param>
    <xsl:with-param name="nodetype">
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
    <xsl:with-param name="field">
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>

  <!--
       <xsl:apply-templates select="../../@name" mode="uppercase"/>
       <xsl:value-of select="'_'"/>
       <xsl:apply-templates select="@name" mode="uppercase"/>
       <xsl:value-of select="'( '"/>
       <xsl:value-of select="'arg_node'"/>  
       <xsl:choose> 
       <xsl:when test="@name = 'BV'">
         <xsl:value-of select="','"/>
         <xsl:value-of select="' NULL'"/>
         <xsl:value-of select="')'"/>     
       </xsl:when>
       <xsl:otherwise>      
       <xsl:value-of select="')'"/>
     </xsl:otherwise>
   </xsl:choose>
   -->

<xsl:value-of select="', '"/>
<xsl:value-of select="'arg_node'"/>
<xsl:value-of select="', '"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'                     '"/>
  <xsl:value-of select="'&quot;'"/>

  <xsl:call-template name="uppercase">
    <xsl:with-param name="string" >
      <xsl:value-of select="type/@name"/>
    </xsl:with-param>
  </xsl:call-template>

  <xsl:value-of select="'&quot;'"/>
  <xsl:value-of select="', '"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'                     '"/>
  <xsl:value-of select="'&quot;'"/>
  <xsl:value-of select="'attribute '"/>

  <xsl:call-template name="uppercase">
    <xsl:with-param name="string" >
      <xsl:value-of select="../../@name"/>
    </xsl:with-param>
  </xsl:call-template>

  <xsl:value-of select="'_'"/>

  <xsl:call-template name="uppercase">
    <xsl:with-param name="string" >
      <xsl:value-of select="@name"/>
    </xsl:with-param>
  </xsl:call-template>

  <xsl:value-of select="' hasnt the right type'"/>
  <xsl:value-of select="'&quot;'"/>
  <xsl:value-of select="');'"/>
  <xsl:value-of select="$newline"/>

</xsl:template>

</xsl:stylesheet>