<?xml version="1.0"?>

<!--
  $Log$
  Revision 1.1  2004/11/23 11:29:56  sah
  Initial revision

  Revision 1.1  2004/11/19 13:54:26  jhb
  Initial revision

  Revision 1.1 2004/09/29 15:15:00 jhb
  Initial revision
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- This stylesheet generates a check.c file implementing all functions
 needed to check a node -->

 <xsl:variable name="newline">
<xsl:text>
</xsl:text>
 </xsl:variable>

<xsl:param name="nodeup"/>
<xsl:param name="nodelo"/>
<xsl:param name="sonatt"/>

<xsl:template match="/">

  <xsl:text>
#define NEW_INFO
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "check_lib.h"


struct INFO
{
};

static info *MakeInfo()
{
  info *result;
  DBUG_ENTER("MakeInfo");
  result = Malloc(sizeof(info));
  DBUG_RETURN(result);
} 

static info *FreeInfo(info *info)
{
  DBUG_ENTER("FreeInfo");
  info = Free(info);
  DBUG_RETURN(info);
}
 </xsl:text> 

  <xsl:apply-templates select="//syntaxtree/node">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>

</xsl:template>


<xsl:template match="node">

  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'node *CHK'"/>
  <xsl:apply-templates select="@name" mode="lowercase"/>
  <xsl:value-of select="'( node *arg_node, info *arg_info)'"/>
  <xsl:value-of select="$newline"/>   
  <xsl:value-of select="'{'"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  DBUG_ENTER'"/>
  <xsl:value-of select="'( &quot;CHK'"/>
  <xsl:apply-templates select="@name" mode="lowercase"/>
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







<!-- trav to the sons -->
<xsl:template match="son" mode="check">

  <xsl:if test="string-length(@mandatory) &gt; 2">
   <xsl:value-of select="$newline"/>
   <xsl:value-of select="'  // this son is mandatory = '"/>
   <xsl:value-of select="@mandatory"/>
   <xsl:value-of select="$newline"/>
   <xsl:value-of select="'  CHKExistChild( '"/>
   <xsl:apply-templates select="../../@name" mode="uppercase"/>
   <xsl:value-of select="'_'"/>
   <xsl:apply-templates select="@name" mode="uppercase"/>
   <xsl:value-of select="'( '"/>
   <xsl:value-of select="'arg_node'"/>
   <xsl:value-of select="')'"/>
   <xsl:value-of select="', '"/>
   <xsl:value-of select="'arg_node'"/>
   <xsl:value-of select="', '"/>
   <xsl:value-of select="$newline"/>
   <xsl:value-of select="'                 '"/>
   <xsl:value-of select="'&quot;'"/>
   <xsl:value-of select="'mandatory son '"/>
   <xsl:apply-templates select="../../@name" mode="uppercase"/>
   <xsl:value-of select="'_'"/>
   <xsl:apply-templates select="@name" mode="uppercase"/>
   <xsl:value-of select="' is NULL'"/>
   <xsl:value-of select="'&quot;'"/>
   <xsl:value-of select="');'"/>
   <xsl:value-of select="$newline"/>
  </xsl:if>

</xsl:template>







<xsl:template match="son" mode="trav">

  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  if ( '"/>
  <xsl:apply-templates select="../../@name" mode="uppercase"/>
  <xsl:value-of select="'_'"/>
  <xsl:apply-templates select="@name" mode="uppercase"/>
  <xsl:value-of select="'( arg_node) != NULL)'"/>

  <xsl:value-of select="' {'"/>
  <xsl:value-of select="$newline"/>

  <xsl:text>    </xsl:text>
  <xsl:apply-templates select="../../@name" mode="uppercase"/>
  <xsl:value-of select="'_'"/>
  <xsl:apply-templates select="@name" mode="uppercase"/>
  <xsl:value-of select="'( arg_node) = Trav( '"/>

  <xsl:apply-templates select="../../@name" mode="uppercase"/>
  <xsl:value-of select="'_'"/>
  <xsl:apply-templates select="@name" mode="uppercase"/>
  <xsl:value-of select="'( arg_node), arg_info);'"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'  }'"/>
  <xsl:value-of select="$newline"/>

</xsl:template>








<xsl:template match="attribute">

  <xsl:if test="string-length(@mandatory) &gt; 2">
   <xsl:value-of select="$newline"/>
   <xsl:value-of select="'  // this attribute is mandatory = '"/>
   <xsl:value-of select="@mandatory"/>
   <xsl:value-of select="$newline"/>
   <xsl:value-of select="'  CHKExistAttribute( '"/>
   <xsl:apply-templates select="../../@name" mode="uppercase"/>
   <xsl:value-of select="'_'"/>
   <xsl:apply-templates select="@name" mode="uppercase"/>
   <xsl:value-of select="'( '"/>
   <xsl:value-of select="'arg_node'"/>
   <xsl:value-of select="')'"/>
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
  <xsl:value-of select="'  CHKRightType( '"/>
  <xsl:apply-templates select="../../@name" mode="uppercase"/>
  <xsl:value-of select="'_'"/>
  <xsl:apply-templates select="@name" mode="uppercase"/>
  <xsl:value-of select="'( '"/>
  <xsl:value-of select="'arg_node'"/>
  <xsl:value-of select="')'"/>
  <xsl:value-of select="', '"/>
  <xsl:value-of select="'arg_node'"/>
  <xsl:value-of select="', '"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'                     '"/>
  <xsl:value-of select="'&quot;'"/>
  <xsl:apply-templates select="type/@name" mode="uppercase"/>
  <xsl:value-of select="'&quot;'"/>
  <xsl:value-of select="', '"/>
  <xsl:value-of select="$newline"/>
  <xsl:value-of select="'                     '"/>
  <xsl:value-of select="'&quot;'"/>
  <xsl:value-of select="'attribute '"/>
  <xsl:apply-templates select="../../@name" mode="uppercase"/>
  <xsl:value-of select="'_'"/>
  <xsl:apply-templates select="@name" mode="uppercase"/>
  <xsl:value-of select="' hasnt the right type'"/>
  <xsl:value-of select="'&quot;'"/>
  <xsl:value-of select="');'"/>
  <xsl:value-of select="$newline"/>

</xsl:template>

<xsl:template match="@name" mode="uppercase">
  <xsl:value-of select="translate(., 'abcdefghijklmnopqrstuvwxyz',
'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
</xsl:template>


<xsl:template match="@name" mode="lowercase">
  <xsl:value-of select="translate(., 'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
'abcdefghijklmnopqrstuvwxyz')"/>
</xsl:template>





</xsl:stylesheet>