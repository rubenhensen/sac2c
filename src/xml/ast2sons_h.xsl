<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.3  2004/12/12 08:02:09  ktr
  removed sons.any, attribs.any because they were incompatible with CLEANMEM

  Revision 1.2  2004/12/11 17:40:00  ktr
  sons/attribs are structs iff CLEANMEM is set.

  Revision 1.1  2004/11/24 16:46:55  sah
  Initial revision



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
        <xsl:value-of select="'sons.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'Defines the NodesUnion and node structures.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id: ast2sons_h.xsl 14294 2005-10-10 12:40:03Z sah $'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_SONS_H_
#define _SAC_SONS_H_

#include "types.h"

    </xsl:text>
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-sons-structs"/>
    <xsl:apply-templates select="/definition/syntaxtree" mode="generate-sons-union"/>
    <xsl:text>
#endif /* _SAC_SONS_H_ */
    </xsl:text>
  </xsl:template>

  <xsl:template match="syntaxtree" mode="generate-sons-structs">
     <xsl:text>
/******************************************************************************
 * For each node a structure of its sons is defined, named 
 * SONS_&lt;nodename&gt;
 *****************************************************************************/
     </xsl:text>
     <xsl:apply-templates select="node" mode="generate-sons-structs">
       <xsl:sort select="@name"/>
     </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="node" mode="generate-sons-structs">
    <xsl:value-of select="'struct SONS_N_'"/>
    <xsl:call-template name="uppercase" >
      <xsl:with-param name="string">
        <xsl:value-of select="@name" />
      </xsl:with-param>
    </xsl:call-template>
    <xsl:value-of select="' { '"/>
    <xsl:apply-templates select="sons/son" mode="generate-sons-structs"/>
    <xsl:value-of select="' } ;'"/>
  </xsl:template>

  <xsl:template match="son" mode="generate-sons-structs">
    <xsl:value-of select="'node * '"/>
    <xsl:value-of select="@name"/>
    <xsl:value-of select="'; '"/>
  </xsl:template>

  <xsl:template match="syntaxtree" mode="generate-sons-union">
    <xsl:text>
/*****************************************************************************
 * This union handles all different types of sons. Its members are
 * called N_nodename.
 ****************************************************************************/
#ifdef CLEANMEM
    </xsl:text>
    <xsl:value-of select="'struct SONUNION { '"/>
    <xsl:text>
#else
    </xsl:text>
    <xsl:value-of select="'union SONUNION { '"/>
    <xsl:text>
#endif
    </xsl:text>
    <xsl:apply-templates select="node" mode="generate-sons-union">
      <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:value-of select="'} ; '"/>
  </xsl:template>
 
  <!-- generate an entry for each node within the union -->
  <xsl:template match="node" mode="generate-sons-union">
    <xsl:value-of select="'struct SONS_N_'"/>
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
