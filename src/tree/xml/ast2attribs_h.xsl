<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.1  2004/07/03 15:14:25  sah
  Initial revision

-->
<!-- This xslt script generates a definition for type node from ast.xml. To
     generate attribs.h using the Sablotron XSLT Engine, execute
     > sabcmd ast2node.xslt ast.xml node.h
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:text>
/******************************************************************************
 * attribs.h generated by ast2attribs_h.xsl
 *
 * DO NOT EDIT THIS FILE. MODIFY ast.xml INSTEAD!
 *****************************************************************************/

#ifndef _sac_attribs_h
#define _sac_attribs_h

#include "types.h"
#include "new_types.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "scheduling.h"
    </xsl:text>
    <!-- start phase that generates a struct of attributes for each node -->
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-attrib-structs"/>
    <!-- start phase that unites all attribute structs to one union -->
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-attrib-union"/>
    <xsl:text>
#endif /* _sac_attribs_h */
    </xsl:text>
  </xsl:template>

  <!-- this template starts generation of attribute structs -->
  <xsl:template match="syntaxtree" mode="generate-attrib-structs">
     <xsl:text>
/******************************************************************************
 * For each node a structure of its attributes is defined, named 
 * nodenameAttribStruct. 
 *****************************************************************************/
     </xsl:text>
     <xsl:apply-templates select="node" mode="generate-attrib-structs"/>
  </xsl:template>

  <!-- generate a attribute structure for a son -->
  <xsl:template match="node" mode="generate-attrib-structs">
    <xsl:value-of select="'struct AttribS_N_'"/>
    <xsl:value-of select="concat( @name, '{ ')"/>
    <xsl:apply-templates select="attributes/attribute" mode="generate-attrib-structs"/>
    <xsl:value-of select="' } ;'"/>
  </xsl:template>

  <!-- generate an entry within the attribute structure -->
  <xsl:template match="attribute" mode="generate-attrib-structs">
    <xsl:value-of select="//attributetypes/type[@name = current()/type/@name]/@ctype"/>
    <xsl:value-of select="' '"/>
    <xsl:value-of select="@name"/>
    <xsl:if test="//attributetypes/type[@name = current()/type/@name]/@size">
      <xsl:value-of select="'['" />
      <xsl:value-of select="//attributetypes/type[@name = current()/type/@name]/@size" />
      <xsl:value-of select="']'" />
    </xsl:if>
    <xsl:value-of select="'; '"/>
  </xsl:template>

  <!-- this template starts generation of the attribstruct union -->
  <xsl:template match="syntaxtree" mode="generate-attrib-union">
    <xsl:text>
/*****************************************************************************
 * This union handles all different types of attributes. Its members are
 * called nodename.
 ****************************************************************************/
    </xsl:text>
    <xsl:value-of select="'union AttribUnion { '"/>
    <xsl:apply-templates select="node" mode="generate-attrib-union"/>
    <xsl:value-of select="' } ; '"/>
  </xsl:template>
 
  <!-- generate an entry for each node within the union -->
  <xsl:template match="node" mode="generate-attrib-union">
    <xsl:value-of select="'struct AttribS_N_'"/>
    <xsl:value-of select="concat( @name, ' N_')"/>
    <xsl:value-of select="concat( @name, '; ')"/>
  </xsl:template>
</xsl:stylesheet>
