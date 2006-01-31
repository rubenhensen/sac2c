<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.4  2004/12/01 14:33:07  sah
  added support for TRAVsetPreFun TRAVsetPostFun

  Revision 1.3  2004/11/27 01:33:56  sah
  implemented TRAVgetName

  Revision 1.2  2004/11/26 11:58:04  sah
  implemented pre/post tables

  Revision 1.1  2004/11/23 22:18:57  sah
  Initial revision



-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  
  <xsl:import href="../xml/common-key-tables.xsl"/>
  <xsl:import href="../xml/common-travfun.xsl"/>
  <xsl:import href="../xml/common-name-to-nodeenum.xsl"/>

  <xsl:output method="text" indent="no"/>
  <xsl:strip-space elements="*"/>

  <!-- starting template -->
  <xsl:template match="/">
    <xsl:call-template name="travfun-file">
      <xsl:with-param name="file">
        <xsl:value-of select="'traverse_tables.h'"/>
      </xsl:with-param>
      <xsl:with-param name="desc">
        <xsl:value-of select="'This file defines the function tables for traversal.'"/>
      </xsl:with-param>
      <xsl:with-param name="xslt">
        <xsl:value-of select="'$Id$'"/>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:text>
#ifndef _SAC_TRAVERSE_TABLES_H_
#define _SAC_TRAVERSE_TABLES_H_

#include "types.h"

typedef travfun_p travfunarray_t [ </xsl:text><xsl:value-of select="count(//syntaxtree/node)+1" /><xsl:text>] ;

typedef travfunarray_t travtables_t [</xsl:text><xsl:value-of select="count(//traversal)+1" /><xsl:text>] ;

typedef travfun_p preposttable_t [</xsl:text><xsl:value-of select="count(//traversal)+1" /><xsl:text>] ;

extern travtables_t travtables;
extern preposttable_t pretable;
extern preposttable_t posttable;
extern const char *travnames[</xsl:text><xsl:value-of select="count(//traversal)+1" /><xsl:text>];

#endif /* _SAC_TRAVERSE_TABLES_H_ */
    </xsl:text>
  </xsl:template>

</xsl:stylesheet>
