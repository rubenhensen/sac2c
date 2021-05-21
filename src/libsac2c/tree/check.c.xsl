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
  
  <xsl:import href="../xml/common_travfun.xsl"/>
  <xsl:import href="../xml/common_node_access.xsl"/>
  <xsl:import href="../xml/common_c_code.xsl"/>
  <xsl:import href="../xml/common_key_tables.xsl"/>
  
  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>
  
  <!-- This stylesheet generates a check.c file implementing all 
       functions needed to check a node -->
  
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
        <xsl:value-of select="'check.c.xsl'"/>
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
#ifndef DBUG_OFF

#include "check.h"
#include "globals.h"
#include "tree_basic.h"
#include "traverse.h"
#define DBUG_PREFIX "CHK"
#include "debug.h"
#include "check_lib.h"
#include "check_mem.h"


/*****************************************************************************
 *
 * @fn node *CHKdoTreeCheck( node *arg_node)
 *
 ****************************************************************************/
node *CHKdoTreeCheck( node *arg_node)
{
  node *keep_next=NULL;
  
  DBUG_ENTER ();
  
  DBUG_ASSERT( NODE_TYPE( arg_node) == N_module
               || NODE_TYPE( arg_node) == N_fundef,
               "Illegal argument node!");

  DBUG_ASSERT( NODE_TYPE( arg_node) == N_module
               || global.local_funs_grouped ,
               "If run fun-based, special funs must be grouped.");

  if (NODE_TYPE( arg_node) == N_fundef) {
    /* 
     * If this check is called function-based, we do not want to traverse
     * into the next fundef, but restrict ourselves to this function and
     * its subordinate special functions.
     */
    keep_next = FUNDEF_NEXT( arg_node);
    FUNDEF_NEXT( arg_node) = NULL;
  }

  DBUG_PRINT( "Starting the check mechanism");

  TRAVpush( TR_chk);
  arg_node = TRAVdo( arg_node, NULL);
  TRAVpop();

  DBUG_PRINT( "Check mechanism complete");

  if (NODE_TYPE( arg_node) == N_fundef) {
    /* 
     * If this check is called function-based, we must restore the original
     * fundef chain here.
     */
    FUNDEF_NEXT( arg_node) = keep_next;
  }
  
  DBUG_RETURN( arg_node);
}
    </xsl:text>
  
<!-- ************************************************************************* 
 *
 *   the nodesets-functions, the check functions and the enum typedef
 *
 ************************************************************************* --> 
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
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'#else  /* !DBUG_OFF */'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'extern int _dummy_check_c; int _dummy_check_c; /* C99 does not allow for empty files. */'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'#endif /* !DBUG_OFF */'"/>
    <xsl:value-of select="$newline"/>
  </xsl:template>


<!-- ************************************************************************* 
 *
 *    IsFun-template: call match node, mode isfun -> to check the nodesets
 *
 ************************************************************************* --> 
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
    <xsl:value-of select="'void isDummy(void)'"/>
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


<!-- ************************************************************************* 
 *
 *    Main-template: call match node, mode function -> main function include 
 *    all checks
 *
 ************************************************************************* -->
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
    <xsl:value-of select="'DBUG_ENTER ();'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  if (NODE_CHECKVISITED( arg_node)) {'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'    NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node), &quot;Node illegally shared: N_'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'&quot;);'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  } else {'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'    NODE_CHECKVISITED( arg_node) = TRUE;'"/>
    <xsl:value-of select="$newline"/>
    <xsl:value-of select="'  }'"/>
    <xsl:value-of select="$newline"/>

    <xsl:apply-templates select="sons/son" mode="checks">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
        <xsl:apply-templates select="attributes/attribute" mode="checks">
          <xsl:sort select="@name"/>
        </xsl:apply-templates>
    <!-- customize checks -->
    <xsl:apply-templates select="./checks/check" mode="customize">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:apply-templates select="./sons/son" mode="trav">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'DBUG_RETURN (arg_node);'"/>
    <xsl:value-of select="'}'"/>
  </xsl:template>


    <!-- *
         * <son> 
         * -->

  <xsl:template match="son" mode="checks">
    <xsl:text>

/*
 * Son check: </xsl:text>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text> 
 */
    </xsl:text>
    <xsl:apply-templates select="targets/target" mode="sons-check">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'{'"/>
    <xsl:call-template name="notexist" />
    <xsl:value-of select="'}'"/>
  </xsl:template>



    <!-- *
         * <attribute>  
         * -->

  <xsl:template match="attribute" mode="checks">
    <xsl:choose>
      <!-- literal attributes are ignored -->
      <xsl:when test="key(&quot;types&quot;, type/@name)[@copy = &quot;literal&quot;]">
        <!-- do nothing -->
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>

/*
 * Attribute check: </xsl:text> 
        <xsl:call-template name="upper_uppername">
          <xsl:with-param name="name1">
            <xsl:value-of select="../../@name" />
          </xsl:with-param>
          <xsl:with-param name="name2">
            <xsl:value-of select="@name"/>
          </xsl:with-param>
        </xsl:call-template>
        <xsl:text>
 */
        </xsl:text>
        <xsl:if test="key(&quot;arraytypes&quot;, ../../../type/@name)">
          <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
          <xsl:value-of select="key(&quot;types&quot;, ../../../type/@name)/@size"/>
          <xsl:value-of select="'; cnt++) { '" />
        </xsl:if>
        <xsl:apply-templates select="type/targets/target" mode="attributes-check">
          <xsl:sort select="@name"/>
        </xsl:apply-templates>
        <xsl:value-of select="'{'"/>
        <xsl:call-template name="notexist" />
        <xsl:value-of select="'}'"/>
        <!-- build code for the targets  -->
        <xsl:if test="key(&quot;arraytypes&quot;, ../../../type/@name)">
          <xsl:value-of select="'}'"/>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


    <!-- *
         * <target> of sons
         * -->

  <xsl:template match="target" mode="sons-check">
    <xsl:value-of select="'if( ( FALSE)'"/>
    <xsl:apply-templates select="./phases/*"/>
    <xsl:value-of select="') {'"/>
    <xsl:if test="string-length(./@mandatory) &gt; 2">
      <xsl:call-template name="exist_son"/>
    </xsl:if>
    <xsl:call-template name="correcttype_son"/>
    <xsl:value-of select="'} '"/>
    <xsl:value-of select="'else '"/>
    <!-- build code for the targets  -->
  </xsl:template>


    <!-- *
         * <target> of attributes
         * -->

  <xsl:template match="target" mode="attributes-check">
        <xsl:value-of select="'if( ( FALSE)'"/>
        <xsl:apply-templates select="./phases/*"/>
        <xsl:value-of select="') {'"/>
        <xsl:if test="string-length(./@mandatory) &gt; 2">
          <xsl:call-template name="exist_attribute"/>
        </xsl:if>
        <xsl:if test="../../../type/@name = 'Node'">
          <xsl:call-template name="correcttype_attribute"/>
        </xsl:if>
        <!-- <xsl:if test="not(./any)">
          <xsl:call-template name="correcttype_attribute"/>
        </xsl:if> -->
        <xsl:value-of select="'}'"/>
        <xsl:value-of select="'else '"/>
  </xsl:template>



<!-- ************************************************************************* 
 *
 *    Son-template:
 *
 ************************************************************************* -->
  <xsl:template name="exist_son">
    <!-- exist test -->
    <xsl:value-of select="'CHKexistSon( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="','"/>
    <xsl:value-of select="'arg_node'"/>
    <xsl:value-of select="','"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'mandatory son '"/>
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="../../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'_'"/>
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' is NULL'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="');'"/>
  </xsl:template>

    <!-- test of the correct type -->
  <xsl:template name="correcttype_son">
    <xsl:value-of select="'if( '"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'( arg_node) != NULL){'"/>
    <xsl:value-of select="'if( !(( FALSE)'"/>
    <xsl:apply-templates select="./*" mode="correctson" />
    <xsl:value-of select="'))'" />
    <xsl:value-of select="'{'" />      
    <xsl:value-of select="'CHKcorrectTypeInsertError('"/>
    <xsl:value-of select="'arg_node,'"/>
    <xsl:call-template name="check_correct_output">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>      
      <xsl:with-param name="name3">
        <xsl:if test="node/@name" >
          <xsl:value-of select="'N_'"/>
          <xsl:call-template name="lowercase">
            <xsl:with-param name="string" >
              <xsl:value-of select="node/@name"/>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:if>
        <xsl:if test="set/@name" >
          <xsl:value-of select="'Nodeset: '"/>
          <xsl:value-of select="set/@name"/>
        </xsl:if>
      </xsl:with-param>      
    </xsl:call-template>
    <xsl:value-of select="');'"/>
    <xsl:value-of select="'}'" />      
    <xsl:value-of select="'}'" />      
   </xsl:template>




<!-- ************************************************************************* 
 *                                                                            
 *  Attribute-template:
 *                                                                              ************************************************************************* -->
  <xsl:template name="exist_attribute">
    <xsl:if test="key(&quot;arraytypes&quot;, ../../../type/@name)">
      <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
      <xsl:value-of select="key(&quot;types&quot;, ../../../type/@name)/@size"/>
      <xsl:value-of select="'; cnt++) { '" />
    </xsl:if>
    <xsl:value-of select="'CHKexistAttribute( (intptr_t)'"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="index">
        <xsl:if test="key(&quot;arraytypes&quot;, ../../../type/@name)">
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
        <xsl:value-of select="../../../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'_'"/>
    <xsl:call-template name="uppercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' is NULL'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="');'"/>
    <xsl:if test="key(&quot;arraytypes&quot;, ../../../type/@name)">
      <xsl:value-of select="'}'"/>
    </xsl:if>
  </xsl:template>


  <!-- test of the correct type -->
  <xsl:template name="correcttype_attribute">
    <xsl:value-of select="'if( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="index">
        <xsl:if test="key(&quot;arraytypes&quot;, ../../../type/@name)">
          <xsl:value-of select="'cnt'"/>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' != NULL) {'"/>
    <xsl:value-of select="'if( !(( FALSE)'"/>
    <xsl:apply-templates select="./*" mode="correctattribute" />
    <xsl:value-of select="'))'" />
    <xsl:value-of select="'{'" />      
    <xsl:value-of select="'CHKcorrectTypeInsertError('"/>
    <xsl:value-of select="'arg_node,'"/>
    <xsl:call-template name="check_correct_output">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../../@name"/>
      </xsl:with-param>      
      <xsl:with-param name="name3">
        <xsl:if test="node/@name" >
          <xsl:value-of select="'N_'"/>
          <xsl:call-template name="lowercase">
            <xsl:with-param name="string" >
              <xsl:value-of select="node/@name"/>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:if>
        <xsl:if test="set/@name" >
          <xsl:value-of select="'Nodeset: '"/>
          <xsl:value-of select="set/@name"/>
        </xsl:if>
      </xsl:with-param>      
    </xsl:call-template>
    <xsl:value-of select="');'"/>
    <xsl:value-of select="'}'" />      
    <xsl:value-of select="'}'" />      
  </xsl:template>





<!-- ************************************************************************* 
 *
 *  CHKnotExist
 *
 ************************************************************************* -->
  <xsl:template name="notexist">
    <xsl:if test="key(&quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'for( cnt = 0; cnt &lt; '" />
      <xsl:value-of select="key(&quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="'; cnt++) { '" />
    </xsl:if>
    <xsl:value-of select="'CHKnotExist( (intptr_t)'"/>
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
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1" >
        <xsl:value-of select="../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="name2" >
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



<!-- ************************************************************************* 
 *
 *    template: call match check, mode customize -> for own check functions
 *
 ************************************************************************* --> 
  <xsl:template match="check" mode="customize">
    <xsl:text>

/*
 *  Customize check: feel you free to add your own check
 */
    </xsl:text>
    <xsl:value-of select="'arg_node= '" />
    <xsl:value-of select="@name" />
    <xsl:value-of select="'( arg_node);'"/>
  </xsl:template>

  
<!-- ************************************************************************* 
 *
 *    template: call match sons : mode trav -> for the trav functions
 *
 ************************************************************************* -->
  <xsl:template match="son" mode="trav">
    <xsl:text>

/*
 * trav functions: to get all sons
 */
    </xsl:text>
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
  




<!-- ************************************************************************* 
 *
 *    range specifier: range/phase/all
 *
 ************************************************************************* -->

  <xsl:template match="range">
    <xsl:value-of select="'|| (( global.compiler_anyphase &gt;= '"/>
    <xsl:value-of select="'PH_'"/>
    <xsl:value-of select="@from"/>
    <xsl:value-of select="')'"/>
    <xsl:value-of select="' &amp;&amp; ( global.compiler_anyphase &lt;= '"/>
    <xsl:value-of select="'PH_'"/>
    <xsl:value-of select="@to"/>
    <xsl:value-of select="'))'"/>
  </xsl:template>

  
  <xsl:template match="phase">
  </xsl:template>
  

  <xsl:template match="all">
    <xsl:value-of select="'|| ( TRUE)'"/>
  </xsl:template>
  


<!-- ************************************************************************* 
 *
 *    correct specifier: 
 *           
 *              son: node/set
 *        attribute: node/any
 *
 ************************************************************************* -->
 
  <xsl:template match="node" mode="correctson" >
    <xsl:value-of select="'|| ( NODE_TYPE( '"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../../@name" /> <!-- node name -->
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../../@name"/> <!-- son name -->
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'(arg_node)'"/>
    <xsl:value-of select="') == '"/>
    <xsl:value-of select="'N_'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/> <!-- son name -->
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="')'"/>
  </xsl:template>

  <xsl:template match="set" mode="correctson" >
    <xsl:value-of select="'|| ( is'"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'('"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="../../../../../@name" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="../../../@name"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'(arg_node)'"/>
    <xsl:value-of select="'))'"/>
  </xsl:template>

 

  <xsl:template match="node" mode="correctattribute" >
    <xsl:value-of select="'|| ( NODE_TYPE( '"/>
    <xsl:call-template name="node-access">
      <xsl:with-param name="node">arg_node</xsl:with-param>
      <xsl:with-param name="nodetype">
        <xsl:value-of select="../../../../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="field">
        <xsl:value-of select="../../../../@name"/>
      </xsl:with-param>
      <xsl:with-param name="index">
        <xsl:if test="key(&quot;arraytypes&quot;, ../../../../type/@name)">
          <xsl:value-of select="'cnt'"/>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="') == '"/>
    <xsl:value-of select="'N_'"/>
    <xsl:call-template name="lowercase">
      <xsl:with-param name="string" >
        <xsl:value-of select="@name"/> <!-- son name -->
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="')'"/>
  </xsl:template>


  <xsl:template match="any" mode="correctattribute" >
  </xsl:template>




<!-- ************************************************************************* 
 *
 *    help functions
 *
 ************************************************************************* -->

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


  <xsl:template name="check_correct_output">
    <xsl:param name="name1" />
    <xsl:param name="name2" />
    <xsl:param name="name3" />
    <xsl:value-of select="'&quot;'"/>
    <xsl:call-template name="upper_uppername">
      <xsl:with-param name="name1">
        <xsl:value-of select="$name1" />
      </xsl:with-param>
      <xsl:with-param name="name2">
        <xsl:value-of select="$name2"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' hasnt the right type.'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="' It should be: '"/>
    <xsl:value-of select="'&quot;'"/>
    <xsl:value-of select="'&quot;'"/>
        <xsl:value-of select="$name3"/>
    <xsl:value-of select="'&quot;'"/>
  </xsl:template>


<!-- ************************************************************************* 
 *
 *    Enum-template: call match node : a enumtype for the I/O
 *
 ************************************************************************* -->
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
