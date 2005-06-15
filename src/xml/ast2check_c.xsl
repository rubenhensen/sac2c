<?xml version="1.0"?>

<!--
  $Log$
  Revision 1.15  2005/06/15 12:42:30  jhb
  little fixes

  Revision 1.14  2005/06/08 13:33:03  jhb
  attribute are now check correctly

  Revision 1.13  2005/05/19 13:34:02  jhb
  added the rangequery for the attributes

  Revision 1.12  2005/05/17 13:00:37  jhb
  added the isfun

  Revision 1.11  2005/02/16 14:33:36  jhb
  divide the attributecode to exist and correct

  Revision 1.10  2005/02/14 14:09:42  jhb
  right = correct

  Revision 1.9  2005/02/11 14:48:26  jhb
  added enum attr_list all attributes

  Revision 1.8  2005/02/10 12:57:33  jhb
  added to the compiler, changed some bugfixes

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
  <xsl:import href="common-key-tables.xsl"/>
  
  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>
  
  <!-- This stylesheet generates a check.c file implementing all functions needed to            check a node -->
  
  <xsl:variable name="newline">
    <xsl:text>
    </xsl:text>
  </xsl:variable>
  
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
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'typedef enum {'"/>
    <xsl:value-of select="$newline"/>
    <xsl:apply-templates select="//syntaxtree/node" mode="enum">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="' } attr_list;'"/>
    <xsl:value-of select="$newline"/>
    <xsl:apply-templates select="//nodesets/nodeset">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
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
    <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
      <xsl:value-of select="'int cnt;'" />
      <xsl:value-of select="$newline"/>
    </xsl:if>
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
    <xsl:value-of select="$newline"/>
    <xsl:apply-templates select="./attributes/attribute" mode="exist">
      <xsl:sort select="@name"/>
    </xsl:apply-templates><!--
    <xsl:apply-templates select="./attributes/attribute" mode="correct"> 
      <xsl:sort select="@name"/>
    </xsl:apply-templates> -->
    <xsl:apply-templates select="./sons/son" mode="trav">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  DBUG_RETURN( arg_node);'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'}'"/>
    <xsl:value-of select="$newline"/>
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
  
  
  <!-- exist attributes -->
  <xsl:template match="attribute" mode="exist">
    <xsl:choose>
      <!-- literal attributes are ignored -->
      <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
        <!-- do nothing -->
      </xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="((./phases/all) or (not(./phases)))">
            <xsl:if test="string-length(@mandatory) &gt; 2">
              <xsl:call-template name="CHKexistAttribute"/>                
            </xsl:if>
          </xsl:when>
          <xsl:when test="((./phases/range) or ( true()))">
            <xsl:value-of select="'if( ( FALSE)'"/>
            <xsl:apply-templates select="./phases/*"/>
            <xsl:value-of select="') {'"/>
            <xsl:if test="string-length(@mandatory) &gt; 2">
              <xsl:call-template name="CHKexistAttribute"/>
            </xsl:if>
            <xsl:value-of select="'}'"/>
            <xsl:value-of select="'else {'"/>
            <xsl:call-template name="CHKnotExistAttribute"/>
            <xsl:value-of select="'}'"/>
            <xsl:value-of select="$newline"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="CHKnotExistAttribute"/>                
          </xsl:otherwise>
        </xsl:choose>
        <xsl:value-of select="$newline"/>
      </xsl:otherwise> 
    </xsl:choose>
  </xsl:template>


  <xsl:template name="CHKexistAttribute">
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="'; cnt++) { '" />
      <xsl:value-of select="$newline"/>
    </xsl:if>
    <xsl:value-of select="'  CHKexistAttribute( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
      <xsl:with-param name="index">
        <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
          <xsl:value-of select="'cnt'"/>
        </xsl:if>
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
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'}'"/>
    </xsl:if>
  </xsl:template>


  <xsl:template name="CHKnotExistAttribute">
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="'; cnt++) { '" />
      <xsl:value-of select="$newline"/>
    </xsl:if>
    <xsl:value-of select="'  CHKnotExistAttribute( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
      <xsl:with-param name="index">
        <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
          <xsl:value-of select="'cnt'"/>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="', '"/>
    <xsl:value-of select="'arg_node'"/>
    <xsl:value-of select="', '"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'                     '"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'attribute '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
      <xsl:with-param name="index">
        <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
          <xsl:value-of select="'cnt'"/>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'must be NULL'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="');'"/>
    <xsl:value-of select="$newline"/>
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'}'"/>
    </xsl:if>
  </xsl:template>

 
<xsl:template match="attribute" mode="correct">
  <xsl:choose>
    <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="'if( '"/>
      <xsl:call-template name="nodeattributename"/>
      <xsl:value-of select="'(arg_node) != NULL) {'"/>
      <xsl:value-of select="'if( !((False)'"/>
      <xsl:apply-templates select="./target/*"/>
      <xsl:value-of select="')) }'"/>      

      <!--
      <xsl:value-of select="$newline"/>
      <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
        <xsl:value-of select="$newline"/>
        <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
        <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
        <xsl:value-of select="'; cnt++) { '" />
      </xsl:if>
      <xsl:value-of select="'CHKcorrectType('"/>
      <xsl:call-template name="node-access">
        <xsl:with-param name="node">arg_node</xsl:with-param>
        <xsl:with-param name="nodetype">
          <xsl:value-of select="../../@name"/>
        </xsl:with-param>
        <xsl:with-param name="field">
          <xsl:value-of select="@name"/>
        </xsl:with-param>
        <xsl:with-param name="index">
          <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
            <xsl:value-of select="'cnt'"/>
          </xsl:if>
        </xsl:with-param>
      </xsl:call-template>
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
        <xsl:value-of select="' hasnt the correct type'"/>
        <xsl:value-of select="'&quot;'"/>
        <xsl:value-of select="');'"/>
        <xsl:value-of select="$newline"/>
        <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
          <xsl:value-of select="'}'"/>
        </xsl:if>
        -->
      </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select="$newline"/>
  </xsl:template>



  <xsl:template match="range">
    <xsl:value-of select="'|| (( global.compiler_subphase &gt;= '"/>
    <xsl:value-of select="'SUBPH_'"/>
    <xsl:value-of select="@from"/>
    <xsl:value-of select="')'"/>
    <xsl:value-of select="' &amp;&amp; ( global.compiler_subphase &lt; '"/>
    <xsl:value-of select="'SUBPH_'"/>
    <xsl:value-of select="@to"/>
    <xsl:value-of select="' ))'"/>
  </xsl:template>

  <xsl:template match="phase">
    <xsl:value-of select="'|| (( global.compiler_subphase &gt;= '"/>
    <xsl:value-of select="'SUBPH_'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="')'"/>
    <xsl:value-of select="' &amp;&amp; ( global.compiler_subphase &lt; '"/>
    <xsl:value-of select="'SUBPH_'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="' ))'"/>
  </xsl:template>
  
  <xsl:template match="all">
    <xsl:value-of select="'|| ( TRUE)'"/>
  </xsl:template>


  <xsl:template match="node">
    <xsl:value-of select="' || ( NODE_TYPE( '"/>
    <xsl:call-template name="nodeattributename"/>
    <xsl:value-of select="') == '"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="')'"/>
  </xsl:template>

  <xsl:template match="set">
    <xsl:value-of select="' || ( is'"/>
    <xsl:value-of select="@name"/>
    <xsl:call-template name="'('"/>
    <xsl:call-template name="nodeattributename"/>
    <xsl:call-template name="' ))'"/>
  </xsl:template>




  <!-- create the enums for the output -->
  <xsl:template match="node" mode="enum">
    <xsl:if test="string-length(attributes/attribute/@name) &gt; 0">
      <xsl:value-of select="'  '"/>
      <xsl:value-of select="'CHK_'"/>
      <xsl:call-template name="lowercase">
        <xsl:with-param name="string" >
          <xsl:value-of select="@name"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="'_'"/>
      <xsl:call-template name="lowercase">
        <xsl:with-param name="string" >
          <xsl:value-of select="attributes/attribute/@name"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:if test="not(position() =last())">
        <xsl:value-of select="','"/>     
      </xsl:if>
      <xsl:value-of select="$newline"/>    
    </xsl:if>
  </xsl:template>
  

  <!-- the isfun functions to check if a son is a part of the nodeset--> 
  <xsl:template match="nodeset">
    <xsl:value-of select="$newline"/>    
    <xsl:value-of select="$newline"/>    
    <xsl:value-of select="'bool'"/>
    <xsl:value-of select="' is'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'( node *arg_node)'"/>
    <xsl:value-of select="$newline"/>    
    <xsl:value-of select="'{'"/>
    <xsl:value-of select="$newline"/>   
    <xsl:value-of select="'bool res = ('"/>  
    <xsl:apply-templates select="target/node" mode="isfun">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'));'"/>
    <xsl:value-of select="$newline"/>    
    <xsl:value-of select="'}'"/>        
    <xsl:value-of select="$newline"/>
  </xsl:template>
  
  
  <xsl:template match="node" mode="isfun">
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'('"/>
    <xsl:value-of select="' NODE_TYPE'"/>
    <xsl:value-of select="'( arg_node) == N_'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:if test="not(position() =last())">
      <xsl:value-of select="') ||'"/>
      <xsl:value-of select="$newline"/>
    </xsl:if>        
    <xsl:value-of select="$newline"/>
  </xsl:template>


  <xsl:template name="nodeattributename">
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
  </xsl:template>

</xsl:stylesheet>