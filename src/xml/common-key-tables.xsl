<?xml version="1.0"?>
<!--
  $Log$
  Revision 1.4  2004/12/05 17:50:57  sah
  extended checks

  Revision 1.3  2004/11/29 13:35:39  sah
  added nodes key

  Revision 1.2  2004/11/24 00:25:04  sah
  added another key
  ,

  Revision 1.1  2004/11/23 11:34:56  sah
  Initial revision


 
 -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- as we make lookups to attributetypes quite often, we create a key -->
<xsl:key name="types" match="//attributetypes/type" use="@name"/>
<xsl:key name="arraytypes" match="//attributetypes/type[@size]" use="@name"/>

<!-- the same for traversal defaults -->
<xsl:key name="traversals" match="//phases//traversal" use="@id" />

<!-- and one for all nodes -->
<xsl:key name="nodes" match="/definition/syntaxtree/node" use="@name" />

<!-- and one for all nodesets -->
<xsl:key name="nodesets" match="/definition/nodesets/nodeset" use="@name" />

</xsl:stylesheet>
