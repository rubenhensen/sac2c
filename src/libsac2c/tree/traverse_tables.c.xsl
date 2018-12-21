<?xml version="1.0"?>

<!--
 ***********************************************************************
 *                                                                     *
 *                      Copyright (c) 1994-2017                        *
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
        <xsl:value-of select="'traverse_tables.c'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the function tables for traversal.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'traverse_tables.c.xsl'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#include "traverse_tables.h"
#include "traverse_helper.h"
#define DBUG_PREFIX "TRAVTABS"
#include "debug.h"
    </xsl:text>
      <xsl:apply-templates select="/definition/traversals/traversal" mode="include" />
    <xsl:text>

travtables_t travtables = {
    /* TR_undefined */
      { &amp;TRAVerror
    </xsl:text>
    <xsl:apply-templates select="/definition/syntaxtree/node" mode="errortraversal" />
    <xsl:value-of select="'} '" />
    <xsl:apply-templates select="/definition/traversals/traversal" />
    <xsl:text>
};

preposttable_t pretable = {
    NULL
    </xsl:text>
    <xsl:apply-templates select="/definition/traversals/traversal" mode="pretable" />
    <xsl:text>,
    NULL
};

preposttable_t posttable = {
    NULL
    </xsl:text>
    <xsl:apply-templates select="/definition/traversals/traversal" mode="posttable" />
    <xsl:text>,
    NULL
};

const char *travnames[ </xsl:text><xsl:value-of select="count(/definition/traversals/traversal) + 2"/><xsl:text>] = {
    "unknown"
    </xsl:text>
    <xsl:apply-templates select="/definition/traversals/traversal" mode="travnames" />
    <xsl:text>,
    "anonymous"
};
    </xsl:text>
  </xsl:template>

  <xsl:template match="node" mode="errortraversal" >
    <xsl:value-of select="', &amp;TRAVerror'" />
  </xsl:template>

  <xsl:template match="traversal">
    <xsl:variable name="phase">
      <xsl:value-of select="@id" />
    </xsl:variable>
    <xsl:variable name="ifndef">
      <xsl:value-of select="@ifndef" />
    </xsl:variable>
    <xsl:text>

    </xsl:text>
    <xsl:value-of select="'/* TR_'" />
    <xsl:call-template name="lowercase" >
      <xsl:with-param name="string" select="@id" />
    </xsl:call-template>
    <xsl:value-of select="' */'" />
    <xsl:text>
    </xsl:text>
    <xsl:if test="@ifndef">
      <xsl:value-of select="'#ifndef '" />
      <xsl:call-template name="uppercase" >
        <xsl:with-param name="string" select="@ifndef" />
      </xsl:call-template>
      <xsl:text>
      </xsl:text>
    </xsl:if>
    <xsl:value-of select="', { &amp;TRAVerror'" />
    <xsl:for-each select="/definition/syntaxtree/node" >
      <xsl:value-of select="', '" />
      <xsl:value-of select="'&amp;'" />
      <xsl:choose>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/travuser/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">user</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/travsons/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">sons</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/travnone/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">none</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="key( &quot;traversals&quot;, $phase)/traverror/node/@name = ./@name" >
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="current()/@id" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">error</xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="travtab-to-travname">
            <xsl:with-param name="phase">
              <xsl:value-of select="$phase" />
            </xsl:with-param>
            <xsl:with-param name="node">
              <xsl:value-of select="@name" />
            </xsl:with-param>
            <xsl:with-param name="style">
              <xsl:value-of select="key( &quot;traversals&quot;, $phase)/@default" />
            </xsl:with-param>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
    <xsl:value-of select="' }'" />
    <xsl:if test="@ifndef">
      <xsl:text>
      </xsl:text>
      <xsl:value-of select="'#else'" />
      <xsl:text>
        , { &amp;TRAVerror
      </xsl:text>
      <xsl:apply-templates select="/definition/syntaxtree/node" mode="errortraversal" />
      <xsl:value-of select="'} '" />
      <xsl:text>
      </xsl:text>
      <xsl:value-of select="'#endif '" />
    </xsl:if>
  </xsl:template>

  <xsl:template name="travtab-to-travname">
    <xsl:param name="phase" />
    <xsl:param name="node" />
    <xsl:param name="style" />
    <xsl:param name="ifndef" />

    <xsl:choose>
      <xsl:when test="$style = &quot;user&quot;">
        <xsl:call-template name="travfun-name">
          <xsl:with-param name="prefix">
            <xsl:value-of select="$phase" />
          </xsl:with-param>
          <xsl:with-param name="name">
            <xsl:value-of select="$node" />
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="$style = &quot;sons&quot;">
        <xsl:value-of select="'TRAVsons'" />
      </xsl:when>
      <xsl:when test="$style = &quot;none&quot;">
        <xsl:value-of select="'TRAVnone'" />
      </xsl:when>
      <xsl:when test="$style = &quot;error&quot;">
        <xsl:value-of select="'TRAVerror'" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$style" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="traversal" mode="include">
    <xsl:value-of select="'#include &quot;'" />
    <xsl:value-of select="@include" />
    <xsl:value-of select="'&quot;'" />
    <xsl:call-template name="newline" />
  </xsl:template>

  <xsl:template match="traversal" mode="pretable">
    <xsl:value-of select="', NULL'" />
  </xsl:template>

  <xsl:template match="traversal" mode="posttable">
    <xsl:value-of select="', NULL'" />
  </xsl:template>

  <xsl:template match="/definition/traversals/traversal" mode="travnames">
    <xsl:value-of select="', &quot;'" />
    <xsl:call-template name="lowercase" >
      <xsl:with-param name="string" select="@id" />
    </xsl:call-template>
    <xsl:value-of select="'&quot;'" />
  </xsl:template>

  <xsl:variable name="newline">
    <xsl:text>
    </xsl:text>
  </xsl:variable>

</xsl:stylesheet>
