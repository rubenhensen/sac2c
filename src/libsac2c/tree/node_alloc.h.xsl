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

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="../xml/common_key_tables.xsl"/>
  <xsl:import href="../xml/common_travfun.xsl"/>
  <xsl:import href="../xml/common_name_to_nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'node_alloc.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Defines the a structure that allows alligned allocation of entire node structures.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'node_alloc.h.xsl'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_NODE_ALLOC_H_
#define _SAC_NODE_ALLOC_H_

#include "types.h"
#include "tree_basic.h"

    </xsl:text>
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-alloc-structure"/>
    <xsl:text>
#endif /* _SAC_NODE_ALLOC_H_ */
    </xsl:text>
  </xsl:template>

  <xsl:template match="syntaxtree" mode="generate-alloc-structure" >
     <xsl:text>
/******************************************************************************
 * For each node a structure NODE_ALLOC_N_&lt;nodename&gt; containing all three 
 * sub-structures is defined to ensure proper alignment. 
 *****************************************************************************/
     </xsl:text>
     <xsl:apply-templates select="node" mode="generate-alloc-structure">
       <xsl:sort select="@name"/>
     </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="node" mode="generate-alloc-structure" >
    <xsl:value-of select="'struct NODE_ALLOC_N_'"/>
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' { '"/>
    <xsl:value-of select="' node nodestructure; '" />
    <xsl:if test="sons/son">
      <xsl:value-of select="' struct SONS_N_'" />
      <xsl:call-template name="uppercase" >
        <xsl:with-param name="string">
          <xsl:value-of select="@name" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="' sonstructure;'" />
    </xsl:if>
    <xsl:if test="attributes/attribute | flags/flag" >
      <xsl:value-of select="' struct ATTRIBS_N_'" />
      <xsl:call-template name="uppercase" >
        <xsl:with-param name="string">
          <xsl:value-of select="@name" />
        </xsl:with-param>
      </xsl:call-template>
      <xsl:value-of select="' attributestructure;'" />
    </xsl:if>
    <xsl:value-of select="' } ;'"/>
  </xsl:template>

</xsl:stylesheet>
