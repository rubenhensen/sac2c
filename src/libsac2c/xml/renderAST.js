function display()
{
  var xmlObj;
  var tree;

  xmlObj = initXMLObj();
  tree = generateInitialDataTree(xmlObj, "Module");
  putGraph( tree);
}


function initXMLObj(){
  
  var xmlhttp;
  var xmlDoc;

  if (window.XMLHttpRequest) {// code for IE7+, Firefox, Chrome, Opera, Safari
    xmlhttp=new XMLHttpRequest();
  } else {// code for IE6, IE5
    xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
  }

  xmlhttp.open("GET","ast.xml",false);
  xmlhttp.send();
  xmlDoc=xmlhttp.responseXML;
  return xmlDoc.documentElement;
}

/*******************************************************************************
 * Functions for generating the data tree from the info provided in ast.js
 * which has been generated from ast.xml using ast2js.xsl.
 ******************************************************************************/

function createDataNode( node) {
  return { name: node.name, set: node.set, unfolded: node.unfolded,
           children: [], labels: node.labels };
}

function collapse(d) {
  if (d.children) {
    d._children = d.children;
    d._children.forEach(collapse);
    d.children = null;
  }
}

function unfoldDataNode( node) {
  var i=0;
  var node_sons = window[node.name+'_sons'];
  for( i=0; i < node_sons.length-1; i++) {
    var new_node = createDataNode( node_sons[i]);
    collapse( new_node);
    node.children.push( new_node);
  }
  node.unfolded = true;
  return node;
}

function generateInitialDataTree( xmlObj, node_name) {
  tree_obj = createDataNode( N_Module);
  tree_obj = unfoldDataNode( tree_obj);
  return { dataTree: tree_obj, roots: [tree_obj]};
}

/*******************************************************************************
 * Global variables and functions for manipulating the root of the tree.
 * These are used from the buttons of the page.
 ******************************************************************************/

var dataTree, roots, root;

function previousRoot() {
  if( roots && (roots.length > 0)) {
    root = roots.pop();
    update( root);
  }
}

function initialRoot() {
  if( roots) {
    root = dataTree;
    roots = [];
    update( root);
  }
}

/*******************************************************************************
 * Global variables and functions for doing the display magic.
 ******************************************************************************/

// global structures:
var tree, diagonal, svg;

// node id counter:
var i=0;

function putGraph( graph){

  var margin = {top: 20, right: 40, bottom: 20, left: 40},
      width = screen.width - margin.right - margin.left,
      height = screen.height - margin.top - margin.bottom;
  
  root = graph.dataTree;
  roots = [];
  dataTree = graph.dataTree;

  tree = d3.layout.tree()
        .size([width, height]);

  diagonal = d3.svg.diagonal()
            .projection(function(d) { return [d.x, d.y]; });

  svg = d3.select("body").append("svg")
       .attr("width", width + margin.right + margin.left)
       .attr("height", height + margin.top + margin.bottom)
       .append("g")
       .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
  
  root.x0 = width / 2;
  root.y0 = 0;
  
  root.children.forEach(collapse);  
  collapse(root);
 
  update(root);

  d3.select(self.frameElement).style("height", "2px");
  
}
  
function update(source) {

   // transition time
   var duration = 750;

   // Compute the new tree layout.
   var nodes = tree.nodes(root).reverse(),
       links = tree.links(nodes);
  
   // Normalize for fixed-depth.
   nodes.forEach(function(d) {  d.y = d.depth * 100; });

   // Update the nodes.
   var node = svg.selectAll("g.node")
    .data(nodes, function(d) { return d.id || (d.id = ++i); });  

   // Enter any new nodes at the parent's previous position.
   var nodeEnter = node.enter().append("g")
    .attr("class", "node")
    .attr("transform", function(d) {   
      return "translate(" + source.x0 + "," + source.y0 + ")";       
    });

   var rec_width = 60,
       rec_height = 20;

   
   nodeEnter.append("rect")
    .attr("width", 1e-6)    
    .attr("height", 1e-6)    
    .attr("x", (rec_width/2))    
    .attr("y", (rec_height/2) - 3)    
    .attr("rx", 1e-6)    
    .attr("ry", 1e-6)    
     .style("fill", function(d) {
       if(d.set)
         return "#e79392";
       return "white";        
     })
     .style("stroke", function(d) {
       if(d.set)
        return "red";        
     })
    .on("dblclick", dblclick)
    .on("click", click)
    .on("mouseover", mouseover)
    .on("mouseout", mouseout);

   nodeEnter.append("text")
    .attr("text-anchor", "middle")
    .text(function(d) { return d.set? d.name : "N_"+d.name; })    
    .style("fill-opacity", 1e-6)
    .on("dblclick", dblclick)
    .on("click", click)
    .on("mouseover", mouseover)
    .on("mouseout", mouseout);
   
   // Transition nodes to their new position.
   var nodeUpdate = node.transition()
    .duration(duration)
    .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });

   nodeUpdate.select("rect")
    .attr("width", rec_width)    
    .attr("height", rec_height)    
    .attr("x", -(rec_width/2))    
    .attr("y", -(rec_height/2) - 3)    
    .attr("rx", 8)    
    .attr("ry", 8)    
    .style("fill", function(d) { 
      if(d.set)
        return d._children ? "#e79392" : "white";
      return !d.unfolded || (d._children && d._children.length > 0) ? "lightsteelblue" : "white"; 
     });

   nodeUpdate.select("text")
    .style("fill-opacity", 1);
   
  
   // Transition exiting nodes to the parent's new position.
   var nodeExit = node.exit().transition()
    .duration(duration)
    .attr("transform", function(d) {
                         return "translate(" + (source.x+(rec_width/2)) + ","
                                             + (source.y+(rec_height/2)) + ")"; })
    .remove();

   nodeExit.select("rect")
    .attr("width", 1e-6)    
    .attr("height", 1e-6)    
    .attr("rx", 1e-6)    
    .attr("ry", 1e-6)    

   nodeExit.select("text")
    .style("fill-opacity", 1e-6);

   // Update the links¦
   var link = svg.selectAll("g.link")
    .data(links, function(d) { return d.target.id; });
  
   // Enter any new links at the parent's previous position.
   var linkEnter = link.enter().append("g")
                   .attr("class", "link");

   linkEnter.append("path")
      .attr("d", function(d) {
                    var o = {x: source.x0, y: source.y0};
                    return diagonal({source: o, target: o});
                 });

   linkEnter.append("text")
    .attr("dy", ".35em")
    .attr("text-anchor", "middle")
    .style("display", "none")
    .text(function(d) {
         return d.source.labels[d.source.children.indexOf( d.target)];
    })
    .attr("transform", function(d) {
      return "translate(" + source.x0 + "," + source.y0 + ")";
    });
     
   // Transition links to their new position.
   var linkUpdate = link.transition()
    .duration(duration);

   linkUpdate.select("path")
    .attr("d", function(d) {     
                 return diagonal({source: {x:d.source.x, y:d.source.y+rec_height/2},
                                  target: {x:d.target.x, y:d.target.y-rec_height/2-5} });
                 });

   linkUpdate.select("text")
     .attr("transform", function(d) {
        return "translate(" +
            (d.target.x) + "," + 
            ((d.target.y) - rec_height - 3) + ")";
     })   
    .style("display", "block");

   // Transition exiting nodes to the parent's new position.
   var linkExit = link.exit().transition()
    .duration(duration);

   linkExit.select("path")
    .attr("d", function(d) {
    var o = {x: source.x, y: source.y};    
    return diagonal({source: o, target: o});
    });

   linkExit.select("text")
     .attr("transform", function(d) {
        return "translate(" +
            (source.x) + "," + 
            (source.y) + ")";
     })   
    .style("display", "none");

   // Stash the old positions for transition.
   nodes.forEach(function(d) {
                   d.x0 = d.x;
                   d.y0 = d.y;
                   });
  
}

/*******************************************************************************
 * Evenet handlers for the nodes:
 ******************************************************************************/

function dblclick(d) {
  roots.push(root)
  root = d;
  update( d);
}

function click(d) {
  if(! d.unfolded ){
     if( d.children == null) {
       d.children = d._children;
       d._children = null;
     }
     d = unfoldDataNode( d);
  } else {
        if (d.children) {
            d._children = d.children;
            d.children = null;
        } else {
            d.children = d._children;
            d._children = null;
        }
  }
  update(d);
}

function mouseover(d) {
  if( !d.set) {
    var target = "ast.xml#"+d.name;
    window.frames["frame"].location=target;
  }
}

function mouseout() {
}

