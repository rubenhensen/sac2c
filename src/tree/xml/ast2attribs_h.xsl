<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.8  2004/11/23 10:09:43  sah
  SaC DevCamp 04

  Revision 1.7  2004/11/22 17:16:56  sah
  changes.
  DK 04

  Revision 1.6  2004/10/12 13:20:13  sah
  added any entry to attribs union

  Revision 1.5  2004/08/29 18:10:05  sah
  general improvements

  Revision 1.4  2004/08/08 16:07:21  sah
  beautified generated code
  include more doxygen comments

  Revision 1.3  2004/08/07 16:19:05  sah
  most xsl files use key-tables for type lookups
  now which increases speed significantly.
  lots of small improvements

  Revision 1.2  2004/07/31 16:16:57  sah
  added support for flags and moved to memory saving attribute
  structure.

  Revision 1.1  2004/07/03 15:14:25  sah
  Initial revision

-->
<!-- This xslt script generates a definition for type node from ast.xml. To
     generate attribs.h using the Sablotron XSLT Engine, execute
     > sabcmd ast2node.xslt ast.xml node.h
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="common-key-tables.xsl"/>
  <xsl:import href="common-travfun.xsl"/>
  <xsl:import href="common-name-to-nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'attribs.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Defines the AttribUnion and attrib structures.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _sac_attribs_h
#define _sac_attribs_h

#include "types.h"
#include "new_types.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "scheduling.h"
#include "constants.h"
    </xsl:text>
    <!-- start phase that generates a struct of attributes for each node -->
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-attrib-structs"/>
    <!-- start phase that unites all attribute structs to one union -->
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-attrib-union"/>
    <xsl:text>
#endif /* _SAC_ATTRIBS_H_ */
    </xsl:text>
  </xsl:template>

  <!-- this template starts generation of attribute structs -->
  <xsl:template match="syntaxtree" mode="generate-attrib-structs">
     <xsl:text>
/******************************************************************************
 * For each node a structure of its attributes is defined, named 
 * ATTRIBS_&lt;nodename&gt;
 *****************************************************************************/
     </xsl:text>
     <xsl:apply-templates select="node" mode="generate-attrib-structs">
       <xsl:sort select="@name"/>
     </xsl:apply-templates>
  </xsl:template>

  <!-- generate a attribute structure for a son -->
  <xsl:template match="node" mode="generate-attrib-structs">
    <xsl:value-of select="'struct ATTRIBS_N_'"/>
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' { '"/>
    <xsl:apply-templates select="attributes/attribute" mode="generate-attrib-structs"/>
    <xsl:apply-templates select="flags" mode="generate-attrib-structs"/>
    <xsl:value-of select="' } ;'"/>
  </xsl:template>

  <!-- generate an entry for an attribute within the attribute structure -->
  <xsl:template match="attribute" mode="generate-attrib-structs">
    <xsl:value-of select="key( &quot;types&quot;, ./type/@name)/@ctype"/>
    <xsl:value-of select="' '"/>
    <xsl:value-of select="@name"/>
    <xsl:if test="key( &quot;arraytypes&quot;, ./type/@name)">
      <xsl:value-of select="'['" />
      <xsl:value-of select="key( &quot;types&quot;, ./type/@name)/@size"/>
      <xsl:value-of select="']'" />
    </xsl:if>
    <xsl:value-of select="'; '"/>
  </xsl:template>

  <!-- generate the fields required for a flag -->
  <xsl:template match="flags[flag]" mode="generate-attrib-structs">
    <xsl:value-of select="'struct { '" />
    <xsl:apply-templates select="flag" mode="generate-attrib-structs" />
    <xsl:value-of select="'} flags;'" />
  </xsl:template>

  <xsl:template match="flags" mode="generate-attrib-structs">
  </xsl:template>

  <xsl:template match="flag" mode="generate-attrib-structs">
    <xsl:value-of select="'unsigned int '" />
    <xsl:value-of select="@name" />
    <xsl:value-of select="' : 1;'" />
  </xsl:template>

  <!-- this template starts generation of the attribstruct union -->
  <xsl:template match="syntaxtree" mode="generate-attrib-union">
    <xsl:text>
/*****************************************************************************
 * This union handles all different types of attributes. Its members are
 * called N_nodename.
 ****************************************************************************/
    </xsl:text>
    <xsl:value-of select="'union ATTRIBUNION { '"/>
    <xsl:apply-templates select="node" mode="generate-attrib-union">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="' void *any; } ; '"/>
  </xsl:template>
 
  <!-- generate an entry for each node within the union -->
  <xsl:template match="node" mode="generate-attrib-union">
    <xsl:value-of select="'struct ATTRIBS_N'"/>
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' *'" />
    <xsl:call-template name="name-to-nodeenum">
      <xsl:with-param name="name">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="'; '"/>
  </xsl:template>
</xsl:stylesheet>
