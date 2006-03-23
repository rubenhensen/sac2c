<?xml version="1.0"?>

<!--
  $Id$
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">
  
  <xsl:import href="../xml/common-travfun.xsl"/>
  <xsl:import href="../xml/common-node-access.xsl"/>
  <xsl:import href="../xml/common-c-code.xsl"/>
  <xsl:import href="../xml/common-key-tables.xsl"/>
  
  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>
  
  <!-- This stylesheet generates a check.c file implementing all functions neede
       d to check a node -->
  
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
        <xsl:value-of select="'$Id: check.c.xsl 14618 2006-02-28 16:08:17Z 
                              jhb $'"/>
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
#include "check.h"
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "check_lib.h"
#include "check_mem.h"
#include "checktst.h"

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


/*****************************************************************************
 *
 * @fn node *CHKdoTreeCheck( node *syntax_tree)
 *
 *****************************************************************************/
node *CHKdoTreeCheck( node *syntax_tree)
{
  info *info;

  DBUG_ENTER( "CHKdoTreeCheck");

  DBUG_PRINT( "CHK", ("Starting the check mechanism"));

  syntax_tree = CHKTSTdoTreeCheckTest( syntax_tree);

  info = MakeInfo();
  
  TRAVpush( TR_chk);
  syntax_tree = TRAVdo( syntax_tree, info);
  TRAVpop();

  info = FreeInfo( info);

  DBUG_PRINT( "CHK", ("Checkmechanism complete"));

  DBUG_RETURN( syntax_tree);
}
    </xsl:text>
  
<!-- *************************************************************************** 
*
*   the nodesets-functions, the check functions and the enum typedef
*
***************************************************************************** --> 
    <!-- first the nodeset-functions --> 
    <xsl:apply-templates select="//nodesets/nodeset">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>

    <xsl:apply-templates select="//nodesets" />
    
    <!-- all the check functions for the nodes -->
    <xsl:apply-templates select="//syntaxtree/node" mode="function">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>

    <xsl:value-of select="$newline"/>
    <xsl:value-of select="$newline"/>

    <!-- the enums-functions for output -->
    <xsl:value-of select="'typedef enum {'"/>
    <xsl:apply-templates select="//syntaxtree/node" mode="enum">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'} attr_list;'"/>
  </xsl:template>


<!-- *************************************************************************** 
*
*    IsFun-template: call match node, mode isfun -> to check the nodesets
*
***************************************************************************** --> 
  <xsl:template match="nodeset">
    <xsl:value-of select="$newline"/>        
    <xsl:value-of select="'static bool is'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'( node *arg_node)'"/>
    <xsl:value-of select="'{'"/>
    <xsl:value-of select="'bool res = ('"/>  
    <xsl:apply-templates select="target/node" mode="isfun">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'));'"/>
    <xsl:value-of select="'return( res);'"/>    
    <xsl:value-of select="'}'"/>
  </xsl:template>

  <xsl:template match="nodeset" mode="dummy">
    <xsl:value-of select="'is'" />
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'( NULL);'" />
  </xsl:template>
 
  <xsl:template match="nodesets">
    <xsl:value-of select="$newline" />        
    <xsl:value-of select="'void isDummy()'"/>
    <xsl:value-of select="'{'" />
    <xsl:apply-templates select="nodeset" mode="dummy">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'}'"/>    
  </xsl:template>
 
  <xsl:template match="node" mode="isfun">
    <xsl:value-of select="'('"/>
    <xsl:value-of select="'NODE_TYPE'"/>
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
  </xsl:template>


<!-- *************************************************************************** 
*
*    Main-template: call match node, mode function -> main function include all checks
*
***************************************************************************** -->
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
    <xsl:value-of select="'{'"/>  
    <xsl:if test="attributes/attribute[key(&quot;arraytypes&quot;, ./type/@name)]">
      <xsl:value-of select="'int cnt;'" />
    </xsl:if>
    <xsl:value-of select="'DBUG_ENTER'"/>
    <xsl:value-of select="'(&quot;CHK'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'&quot;);'"/>
    <!-- exist son check -->
    <xsl:text>
/*
 * Son check: this sons must exist  --> mandatory is yes 
 */
    </xsl:text>

    <xsl:apply-templates select="./sons/son" mode="exist">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <!-- correct type of the sons check -->
    <xsl:text>
/*
 * Son check: if they exist, check the type with the ast.xml declaration 
 */
    </xsl:text>
    <xsl:apply-templates select="./sons/son" mode="correct">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <!-- exist attribute check-->
    <xsl:text>
/*
 * Attribute check: this attributes must exist  --> mandatory is yes 
 */
    </xsl:text>
    <xsl:apply-templates select="./attributes/attribute" mode="exist">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <!-- correct type of the attribute check -->
    <xsl:text>
/*
 * Attribute check: if they exist, check the type with the ast.xml declaration 
 */
    </xsl:text>
    <xsl:apply-templates select="./attributes/attribute" mode="correct">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <!-- customize checks -->
    <xsl:text>
/*
 *  Customize check: feel you free to add your own check
 */
    </xsl:text>
    <xsl:apply-templates select="./checks/check" mode="customize">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:text>
/*
 * trav functions: to get all sons
 */
    </xsl:text>
    <xsl:apply-templates select="./sons/son" mode="trav">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'DBUG_RETURN( arg_node);'"/>
    <xsl:value-of select="'}'"/>
  </xsl:template>


<!-- *************************************************************************** 
*
*    Son-template: call match sons :check if sons exist
*
***************************************************************************** -->
  <xsl:template match="son" mode="exist">
    <xsl:if test="string-length(targets/target/@mandatory) &gt; 2">
       <xsl:value-of select="'CHKexistSon( '"/>
      <xsl:call-template name="node-access">
        <xsl:with-param name="node">arg_node</xsl:with-param>
        <xsl:with-param name="nodetype">
          <xsl:value-of select="../../@name"/>
        </xsl:with-param>
        <xsl:with-param name="field">
          <xsl:value-of select="@name"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="','"/>
      <xsl:value-of select="'arg_node'"/>
      <xsl:value-of select="','"/>
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
    </xsl:if>
  </xsl:template>



<!-- *************************************************************************** 
*
*    Son-template: call match son, mode correct -> call the correct type function
*
***************************************************************************** -->
  <xsl:template match="son" mode="correct">
    <xsl:choose>
      <xsl:when test="type/@name = 'NodePointer'">
      </xsl:when>
      <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
      </xsl:when>
      <xsl:otherwise>
        <xsl:if test="not(./type/targets/target/any)">
          <xsl:value-of select="'if( '"/>
          <xsl:call-template name="upper_uppername">
            <xsl:with-param name="name1">
              <xsl:value-of select="../../@name" />
            </xsl:with-param>
            <xsl:with-param name="name2">
              <xsl:value-of select="@name"/>
            </xsl:with-param>
          </xsl:call-template>
          <xsl:value-of select="'( arg_node) != NULL){'"/>
          <xsl:value-of select="'if( !(( FALSE)'"/>
          <xsl:apply-templates select="./targets/target/*" mode="correctson" />
          <xsl:value-of select="'))'" />
          <xsl:value-of select="'{'" />      
          <xsl:value-of select="'CHKcorrectTypeInsertError('"/>
          <xsl:value-of select="'arg_node,'"/>
          <xsl:value-of select="'&quot;'"/>
          <xsl:call-template name="upper_uppername">
            <xsl:with-param name="name1">
              <xsl:value-of select="../../@name" />
            </xsl:with-param>
            <xsl:with-param name="name2">
              <xsl:value-of select="@name"/>
            </xsl:with-param>
          </xsl:call-template>
          <xsl:value-of select="' has not the right type.'"/>
          <xsl:value-of select="'&quot;'"/>
          <xsl:value-of select="');'"/>
          <xsl:value-of select="'}'" />      
          <xsl:value-of select="'}'" />      
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>  


<!-- *************************************************************************** 
*                                                                            
*  Attribute-template: call match=attribute   check if attributes exist
*                                                                              
***************************************************************************** -->
  <xsl:template match="attribute" mode="exist">
    <xsl:choose>
      <!-- literal attributes are ignored -->
      <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
        <!-- do nothing -->
      </xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="((./type/targets/target/phases/all) or (not(./type/targets/target/phases/*)))">
            <xsl:if test="string-length(./type/targets/target/@mandatory) &gt; 2">
              <xsl:call-template name="CHKexistAttribute"/>                
            </xsl:if>
          </xsl:when>
          <xsl:when test="((./type/targets/target/phases/range) or ( true()))">
            <xsl:value-of select="'if( ( FALSE)'"/>
            <xsl:apply-templates select="./type/targets/target/phases/*"/>
            <xsl:value-of select="') {'"/>
            <xsl:if test="string-length(./type/targets/target/@mandatory) &gt; 2">
              <xsl:call-template name="CHKexistAttribute"/>
            </xsl:if>
            <xsl:value-of select="'}'"/>
            <xsl:value-of select="'else {'"/>
            <xsl:call-template name="CHKnotExistAttribute"/>
            <xsl:value-of select="'}'"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="CHKnotExistAttribute"/>                
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise> 
    </xsl:choose>
  </xsl:template>


<!-- *************************************************************************** 
*
*  Attribute-template: call name CHKexistAttribute ->
*
***************************************************************************** -->
  <xsl:template name="CHKexistAttribute">
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="'; cnt++) { '" />
    </xsl:if>
    <xsl:value-of select="'CHKexistAttribute( '"/>
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
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'mandatory attribute '"/>
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
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'}'"/>
    </xsl:if>
  </xsl:template>


<!-- *************************************************************************** 
*
*  Attribute-template: call name CHKnotExistAttribute
*
***************************************************************************** -->
  <xsl:template name="CHKnotExistAttribute">
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="'; cnt++) { '" />
    </xsl:if>
    <xsl:value-of select="'CHKnotExistAttribute( '"/>
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
    <xsl:value-of select="' must be NULL'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="');'"/>
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'}'"/>
    </xsl:if>
  </xsl:template>


<!-- *************************************************************************** 
*
*    template: call match attribute, mode correct -> call the correct type function
*
***************************************************************************** -->
  <xsl:template match="attribute" mode="correct">
    <xsl:choose>
      <xsl:when test="type/@name = 'NodePointer'">
      </xsl:when>
      <xsl:when test="key(&quot;types&quot;, ./type/@name)[@copy = &quot;literal&quot;]">
      </xsl:when>
      <xsl:otherwise>
        <xsl:if test="not(./type/targets/target/any)">
          <xsl:value-of select="'if( '"/>
          <xsl:call-template name="upper_uppername">
            <xsl:with-param name="name1">
              <xsl:value-of select="../../@name" />
            </xsl:with-param>
            <xsl:with-param name="name2">
              <xsl:value-of select="@name"/>
            </xsl:with-param>
          </xsl:call-template>
          <xsl:value-of select="'( arg_node) != NULL) {'"/>
          <xsl:value-of select="'if( !(( FALSE)'"/>
          <xsl:apply-templates select="./type/targets/target/*" mode="correctattribute" />
          <xsl:value-of select="'))'" />
          <xsl:value-of select="'{'" />      
          <xsl:value-of select="'CHKcorrectTypeInsertError('"/>
          <xsl:value-of select="'arg_node,'"/>
          <xsl:value-of select="'&quot;'"/>
          <xsl:call-template name="upper_uppername">
            <xsl:with-param name="name1">
              <xsl:value-of select="../../@name" />
            </xsl:with-param>
            <xsl:with-param name="name2">
              <xsl:value-of select="@name"/>
            </xsl:with-param>
          </xsl:call-template>
          <xsl:value-of select="' hasnt the right type.'"/>
          <xsl:value-of select="'&quot;'"/>
          <xsl:value-of select="');'"/>
          <xsl:value-of select="'}'" />      
          <xsl:value-of select="'}'" />      
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!-- *************************************************************************** 
*
*    template: call match check, mode customize -> for own check functions
*
***************************************************************************** --> 
  <xsl:template match="check" mode="customize">
    <xsl:value-of select="'arg_node= '" />
    <xsl:value-of select="@name" />
    <xsl:value-of select="'( arg_node);'"/>
  </xsl:template>

  
<!-- *************************************************************************** 
*
*    template: call match sons : mode trav -> for the trav functions
*
***************************************************************************** -->
  <xsl:template match="son" mode="trav">
    <xsl:value-of select="'if ( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' != NULL) {'"/>
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
    <xsl:value-of select="'}'"/>
  </xsl:template>
  

  <xsl:template match="range">
    <xsl:value-of select="'|| (( global.compiler_subphase &gt;= '"/>
    <xsl:value-of select="'SUBPH_'"/>
    <xsl:value-of select="@from"/>
    <xsl:value-of select="')'"/>
    <xsl:value-of select="' &amp;&amp; ( global.compiler_subphase &lt; '"/>
    <xsl:value-of select="'SUBPH_'"/>
    <xsl:value-of select="@to"/>
    <xsl:value-of select="'))'"/>
  </xsl:template>

  
  <xsl:template match="phase">
  </xsl:template>
  

  <xsl:template match="all">
    <xsl:value-of select="'|| ( TRUE)'"/>
  </xsl:template>
  
 
  <xsl:template match="node" mode="correctson" >
    <xsl:value-of select="'|| ( NODE_TYPE( '"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'(arg_node)'"/>
    <xsl:value-of select="') == '"/>
    <xsl:value-of select="'N_'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="')'"/>
  </xsl:template>


  <xsl:template match="node" mode="correctattribute" >
    <xsl:value-of select="'|| ( NODE_TYPE( '"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'(arg_node)'"/>
    <xsl:value-of select="') == '"/>
    <xsl:value-of select="'N_'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="')'"/>
  </xsl:template>

  <xsl:template match="set" mode="correct" >
    <xsl:value-of select="'|| ( is'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'('"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'(arg_node)'"/>
    <xsl:value-of select="'))'"/>
  </xsl:template>


  <xsl:template name="upper_uppername">
    <xsl:param name="name1" />
    <xsl:param name="name2" />
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="$name1"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'_'"/>
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="$name2"/>
      </xsl:with-param>
    </xsl:call-template>    
  </xsl:template>


<!-- *************************************************************************** 
*
*    Enum-template: call match node : a enumtype for the I/O
*
***************************************************************************** -->
  <xsl:template match="node" mode="enum">
    <xsl:if test="string-length(attributes/attribute/@name) &gt; 0">
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
      <xsl:value-of select="$newline" />
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>