//
// @ORIGINAL_AUTHOR: Federico Ardanaz
//
// This file does not contain any code
// it just contains additional information for
// inclusion with doxygen


// ========================================================================================  
/*! 

\mainpage Dral Reference
 
Written by Roger Espasa, Federico Ardanaz, Pau Cabre, Joel Emer, Roger Gramunt


\section INTRO Introduction

 Dral is a language used to describe the flow of objects through the edges of
 a graph. Objects in dral are called "items". The language allows attaching
 arbitrary attributes to the items flowing through the graph as well as attaching
 attributes to the nodes and edges themselves. These attributes may vary
 dynamically over time. Dral also allows describing the nodes' internal
 structure as a n-dimensional array. Then, items flowing through the graph have
 the possibility of being temporarily stored within a "slot" in a given node.

 The language has statements to define the nodes and edges that conform the
 graph, such as:

 \code
     NewNode "Stage1" nodeid=1
     NewNode "Stage2" nodeid=2
     NewEdge from "Stage1" to "Stage2" edgeid=1 latency=3 bandwidth=2
 \endcode

 Once the graph is defined, the language allows describing the events ocurring
 during the simulation. A command to establish the current cycle sets the passing
 of time. Other dral commands are executed within a cycle and can be
 used to create items, move them through the graph and finally, when no longer
 needed, destroy them. Consider as an example the following sequence
 of (pseudo)dral commands:

 \code
  Cycle 1
  NewItem itemid=1
  SetItemTag 1 "name" "John"
  Cycle 2
  MoveItems 1 over edge 1
  Cycle 3
  Cycle 4
  Cycle 5
  Cycle 6
  DeleteItem 2
 \endcode

 In this sequence, a new item is created in cycle 1. Then, we associate a tag
 called "name" to the item and set the value of "name" to the string
 "John". In cycle 2, the item flows from Node "Stage1" to node "Stage2".
 Note that, as per the edge latency definition, the item will reach node
 "Stage2" in cycle 5 (starts traveling on cycle 2, latency is 3). Later, in
 cycle 6, the item is destroyed.

 The implied usage of the dral language is to communicate the activity and
 events occurring in a model (simulator) to an external visualization tool,
 using a client-server architecture (see figure ref{fg:basicArch}).  The
 simulator, the server, makes the appropriate calls to the dral server API at
 different points in time to describe the activity being modelled. The API maps
 the calls into commands and sends them to the clients through a dralchannel,
 which can be any communication media ranging from shared memory to a file or a
 socket. Then, each client parses the dral commands and produces a visually
 appealing representation of the activity being modelled in the simulator.

 figura{basicArchitecture.eps}{fg:basicArch}{dral Client-Server Architecture.}{0.75}

\subsection DESIGN_GOALS Design Goals

 A number of design goals where set for the dral language. These are:


 - The language should be <B>expressive</B> enough to convey the typical events and 
       changes that occur in discrete system simulation and, in
       particular, in a microprocessor simulation model.

 - The language should be <B>complete</B>, in the sense that a single dral session
       should be able to convey all the events that occur in a simulation.
       Multiple GUI clients, with different visualization goals and with
       different focus on different parts of the system being modeled,
       should all be able to read the same dral input stream and produce
       their visual output, possibly ignoring the bits and pieces in the
       stream not required for their correct operation.

 - The language should offer a <B>unique and simple API</B> that modelers can
       use to generate all the "item movements" that occur as a
       microprocessor executes a program. If possible, the model
       programmer should only require to sprinkle "one-liners" across
       the model's source code in a clean and obvious way. 
       
 - The dral API should make a clear separation between the model code and
       the GUI-based visualization tools. The calls made to the dral API
       should convey only the essential information to the external tools.
       Details such as the color to be used for a given event should be 
       stored in client-specific configuration files. Thus, the architect
       developing the model should only be concerned about passing to
       the clients high-level information such as: an object is moving
       from node to node, an object is destroyed, attaching an attribute
       to an object, etc. The final details of how to render object
       movements in the client GUI are left to the client.

       A second benefit of cleanly separating model and client GUIs is to avoid
       the all too common situation where, in order to change some aspect of
       the visualization, both the model and the GUI tool needed to be modified
       and recompiled. With the graph abstraction and the API provided by
       dral, user decisions on which nodes, edges and items are to be
       displayed are <I>local</I> to the GUI tool and can be customized on the
       fly. The model doesn't care about which data is being displayed in the
       GUI tool; it always generates all the necessary information. 
       <SMALL>(In the case of excessive data generation, server-side and/or client-side
       event filtering can be implemented)</SMALL>

 - The bit-encodings of the different dral commands should be efficient
       to allow storing on disk at least a 1,000,000 cycles worth of simulation 
       with about 1024 "events" occurring per cycle.

 - The language should be fully abstract, and not necessarily tied to 
       microprocessor modeling. In particular, this means there will be
       no implicit knowledge about attributes associated to tags, nodes
       or items, there will be no reserved attributes, and no implicit
       assumptions about time steps/cycles.

 - The language will be as small as possible. However, the language will be
       extensible throught the use of "typed comments".  By allowing a special
       command that carries a magic number and a number of bytes, users should be
       able to add to the dral stream information that may be relevant only to a
       specific client (or set of clients).


\subsection USING_DRAL Using dral to model Microprocessors

 A microprocessor can be effectively abstracted into a set of buffers
 (stations) that hold instructions and/or miscellaneous types of requests
 (such as misses, fills, prefetches, branch outcomes, etc.) and
 a set of wires that connect these buffers. More formally, a
 microprocessor can be abstracted as a directed graph \f$G=<N,E>\f$ where
 the nodes are the microprocessor buffers and the edges represent
 the wires over which information is transferred from buffer to buffer.

 The information traveling through the wires (edges) can be anything
 ranging from instructions, cache requests, acnowledgements, register
 identifiers, branch outcomes, etc. This information can be abstracted
 as items that flow through the graph. Attributes can be attached
 to items to decorate them with relevant information, such as
 instruction PC, effective address, instruction disassembly, etc.

 Dral is designed to be a very good match to this view of a microprocessor,
 without compromising other uses of the language. In fact, dral has been used
 to visualize events in several past and current microprocessor projects: 
 the Alpha EV8 and the Alpha Tarantula, an IPF product codenamed Tanglewood and
 an x86 processor codenamed Nehalem.

\subsection DRAL_NUTSHELL Dral in a nutshell

A typical dral stream starts with a number of <CODE>NewNode</CODE> and
<CODE>NewEdge</CODE> commands that define the graph used to abstract the model
being simulated. Edges have two fundamental properties associated with
them: their bandwidth and their latency. The edge bandwidth determines the
maximum number of items that can enter (move through) the edge on a
given cycle. The edge latency determines how many cycles the items
will take on their journey throught the edge before coming out
at the other (receiving) end of the edge. 

Then, the stream contains a sequence of dral commands
that establish the flow of time and the flow of objects through the graph.
The stream is in-order and state-based, i.e., the semantics of a command
may be affected by the "state" modified/created by previous commands.
The most common commands used in a typical stream are summarized below
(see chapter ref{chap:dral1enc} for the complete reference):
<ul>
 <li><strong>Cycle</strong> The <CODE>Cycle</CODE> command establishes the current simulation cycle.

 <li><strong>NewItem, SetItemTag, DeleteItem</strong> Items are created using the <CODE>NewItem</CODE> 
	command. Items are typically born inside a particular node. Upon
	creation, an item has no information associated with it except its item
	identifier (which is unique). The architect can "decorate" an item by
	adding information to it. This is accomplished using the
	<CODE>SetItemTag</CODE> command. The <CODE>SetItemTag</CODE> command assigns a tuple
	ilverb{<tag,value>} to the item. Using this command, an architect can
	"store" int the item information such as ilverb{<pc,0x1200abc>},
	ilverb{<effa,0xfff012>}, etc. The visualization tools can later query
	and display the set of tags associated to a given item. At the end of
	its life, a <CODE>DeleteItem</CODE> command must be issued to destroy the
	item.

 <li><strong>MoveItem</strong> The <CODE>MoveItem(ItemId, EdgeId)</CODE> command informs the dral clients
                 that a particular item (indicated by ItemId) starts on this cycle a
		 trip through the edge indicated by EdgeId. Since all edges have an
		 associated latency (in cycles), the cycle where the item will
		 arrive at the destination node can be trivially computed.

 <li><strong>EnterNode, ExitNode</strong> When an item arrives at a node, the architect can optionally
                  indicate to the dral clients the exact slot within the node that the
		  item will be occupying. This can be used, for example, to indicate the
		  exact slot within the instruction queue where a given instruction will
		  reside. It can also be used to indicate a cache access, etc. If the
		  <CODE>EnterNode(ItemId,slot)</CODE> command is used to this end, the corresponding
		  slot in the node will be considered "occupied" until a
		  complementary <CODE>ExitNode(slot)</CODE> command is issued.
</ul>
There are more advanced commands to set Tags to nodes and cycles that are described
in full detail in chapter ref{chap:}.


\section DEFINITION Definition 

In this chapter we introduce the very fundamental concepts
and definitions that conform the dral dralversion specification.
For a complete and detailed enumeration of the commands
check appendix ref{chap:reference}.

The dral language contains four fundamental notions: 

  - A graph used to describe and precisely define the system being modeled
  - Items that flow through the graph
  - The passing of time in the simulation, measured in discrete time 
        units called "cycles".
  - Attributes ("tags" in dral terminology) that can be associated to 
        graph elements, to items flowing through the graph and also to cycles.

The following sections describe these four abstractions.

\subsection DRAL_GRAPH The dral graph

The dral <I>graph</I> is a directed hierachical graph. The graph
can be formally defined as \f$G=<N, E, GN>\f$, where \f$N\f$ is the set of
nodes in the graph and \f$E\f$ is the set of edges in the graph. Edges
are directed and there can be multiple edges from one node to another.
\f$GN\f$ is a set of special nodes called "grouping nodes" used to give 
the hierarchical structure to the graph. Grouping nodes can not be
connected by edges.

Nodes, grouping nodes and edges are dynamically added to the
existing graph using the <CODE>NewNode</CODE> and <CODE>NewEdge</CODE> commands.
At the beginning of a dral simulation, the graph is empty. The user
then constructs the model's graph by repeatedly invoking these two
commands. Nodes and edges can also be created at any later point during 
the simulation footnote{Note, however, that nodes and edges can never be deleted}. 
Of course, any commands that refer to a node or an edge can only be
applied to nodes and edges that have been created using the
<CODE>NewNode</CODE> and <CODE>NewEdge</CODE> commands.


\subsection NODES Nodes

Nodes in the dral graph are connected to each other by one or more
unidirectional edges and are used to represent hardware storage.  Nodes can be
used to describe, for example, a branch predictor, a cache, and instruction
queue, a request fifo, etc. Each node in the graph is uniquely identified
by its "node identifier", a 32-bit unsigned integer. There can be no
nodes with the same node identifier.

The fundamental property of a node is its <i>capacity</i>. Each node has a number of
"slots", each of which can hold one (and only one) item. The total number of
slots in a node is called the node's <I>capacity</I>. For a node having a
capacity of \f$N\f$ slots, its slots are numbered 0 through \f$N-1\f$.  A given slot is
either empty or full, depending on whether an item has "entered" into said
slot or not. The total number of full slots in a node is called the node's <I>
occupancy</I>.

Additional properties of a node are its <i>name</i> and its <i>instance
number</i>. The <i>name</i> and <i>instance number</i> are combined to associate
to each node a unique ASCII string, so that clients can refer easily and
unambigously to each node in their configuration files. The combination
algorithm is as follows: If a node is unique, i.e., there are no other nodes
with the same name, then its instance number must be 0 and the final name is
simply the <i>name</i> string.  However, if multiple copies of a node named
"foo" exist in the graph, then each node will have a different instance
number starting at 0. Then, a unique name for each node will be inferred from
the name and the instance number by concatenating them in a C-like fashion: the
nodes will be called "foo[0]", "foo[1]", "foo[2]", etc. Note that the
instance number is chosen by the user and can be any arbitray number in the
0..65535 range. This two-level naming scheme has been chosen because 
hardware systems tend to have repetitions of the same structures. For example,
a dual-core processor will have two copies of pretty much every structure in
the cores: two branch predictors, two instruction caches, etc. Combining
a string and a number to generate the final unique string matches very
well the type of repetition (instantiation) that occurs in real hardware.

As already mentioned, the dral graph is hierachical. Encoding the
hierarchy is achieved using the <i>parent number</i> attribute. If a node
is a child node of some other node, then the id of the parent node should
be encoded in the <i>parent number</i> attribute. If a node
has no parent, this field should be set to 0.

\subsubsection NODE_LAYOUT Node Layout

Hardware structures are rarely as simple as a linear array of slots. Thus,
the notion of nodes being simply a linear array of size "capacity" is
not adequate to model many real life situations. More often than not, the
storage has some internal repetitive structure. To accomodate this notion,
dral allows defining a "layout" for a node's capacity.  In this case,
a node can be thought of as a multidimensional array (in the C-language
sense). As an example, assume we have a collapsing buffer that takes
instructions coming from the fetch unit and queues them into 4 queues, one
per thread in the machine. Each queue can hold 32 instructions. Using
the dral C++ API, we could declare such a node using:

\code
    ... = NewNode("collapsing buffer", 4, 32)
\endcode


which would be equivalent to a C-like declaration of

\code
  	... CollapsingBuffer[4][32];
\endcode

Notice that nodes are "homogeneous" in nature, i.e., they follow the array
semantics of the C language. If the modeler happens to have a hardware
structure that contains, say, two different storage structures (a request fifo
with 16 entries and a index-conflict-table with 128 entries) he/she would need
to declare two separate nodes for each storage space

\subsubsection STORING_ITEM_NODES Storing Items in Nodes

When an item arrives at a node through one of its incoming edges, the item can
be stored in one of the node's slots at the user's wish. To do so, the user
must issue the <CODE>EnterNode</CODE> command to explicitly tell dral into which
slot the item will be stored. Correspondingly, when an item is to leave from a
node, the user must issue the <CODE>ExitNode</CODE> command, explicitly indicating
which particular slot (and, hence, which item) is being freed in the node.

Contrary to initial intuition, dral <I> will not</I> force an item arriving at
a node to be stored in one of its slots. It's perfectly legal for an item to
arrive at a node and "vanish" in the air, without affecting the node's
occupation.  This behavior allows modeling the very frequent case of
"messages" that are sent to a node to alter the "state" of one of the items
stored in the node.  For example, an item arriving into a node might simply
carry the information that a certain miss has been serviced. Clearly, the
"serviced=true" message can not and should not take a slot in the receiving
node.

Similarly, and item can be "moved" through an outgoing edge from a node
without the item being necessarily removed from the node (through an
<CODE>ExitNode</CODE> command).  Again, this behavior mimics the common case of an
item sending a message to another node in the graph to force some desired
action. Clearly, the item "sending the message" has not left the node and,
therefore, it's slot is still busy.


Both the <CODE>EnterNode</CODE> and <CODE>ExitNode</CODE> commands fully support the notion
of multi-dimensional nodes as described in the previous section. The dral C++
syntax for these two functions is:

\code
  EnterNode (nodeid, itemid, <slot id>)  
  ExitNode  (nodeid, itemid, <slot id>) 
\endcode

The syntax to specify the <slot id> argument is a comma-separated list
of integers that index into each dimension of the layout (from left-to-right,
i.e., the left-most argument specifies the position for the left-most dimension
in the layout). It's in essence the same syntax used in C --except for the
missing brackets-- to designate a given (multi dimensional) array element.  

Assuming, for example, a two-dimensional layout, say [4][16], the
following example illustrates the available C++ syntax to store
a given item into the node:
\code
EnterNode(n, i, 3, 2)
\endcode 
Item \<i\> enters slot [3][2] in node \<n\>.  This matches exactly the C semantics of 
indexing a multi-dimensional array.

Additionally, dral introduces special syntax to allow useing "don't
care" values in the index list.  Any index in the list can be substituted
by a special value, called DRAL_ANY. The DRAL_ANY token acts as a "don't
care" index into the array, indicating that the user does not know (or
does not care) the exact position that the entering item will take in the
dimension where the DRAL_ANY token has been placed. In this situation,
the dral client is free to "choose" by itself which index the item
should go to (obeying, of course, the restriction not to use an already
taken slot and to never exceed the node's capacity).  Continuing our
previous layout example, the following commands illustrate the
possibilities of the DRAL_ANY token:
\code
EnterNode(n, i, DRAL_ANY, 2)
\endcode    
Item \<i\> has entered either into slot [0][2], [1][2], [2][2], [3][2] (ONLY one of the
four) of node \<n\>. The user does not know which index in the
first dimension will be occupied by the item, so it is left to the
choice of the final visualizer to decide how/where to draw the item.

\code
EnterNode(n, i, 3, DRAL_ANY)
\endcode    
Item \<i\> has entered either into slot [3][0],~[3][1],...,[3][15] (ONLY one of
the sixteen) of node \<n\>. The user does not know which of the
sixteen slots within a "queue" the item is in, so it is left to the
choice of the final visualizer to decide how to draw the item.

\code
EnterNode(n, i, DRAL_ANY, DRAL_ANY)
\endcode    
In this last case,
the user is treating the 64 slots in the node as a "bag" structure: the
user does not care exactly where the item entered, he just knows an item
entered the node.

\subsubsection NODE_TAGS Node Tags

Nodes are used in dral to represent both storage and control hardware
structures. Typically, physical structures may have "states" associated with
them. For example, a fifo may be full. Or it may be in "drain" state. Also,
the current state of a structure, say a memory controller, can be a function of
the recent history of items that passed through the controller. For example,
after a sufficiently large string of write requests, a memory controller might
change its operation mode from "prioritize-reads" into "prioritize-writes". 

Tags are the dral way of decorating the graph nodes with this kind of
dynamic information.  The user can attach <tag,value} pairs to
nodes. Clients can use these attributes to help the user better understand
the events happening in the simulation.  Once a tag is associated to a
node the tag can not be destroyed or otherwise removed. However, the tag
<I>value</I> is allowed to change. In other words, a user can set different
values to the same tag in different simulation cycles.  Continuing the
memory controller example, one could use node tags to easily express the
change of state in the node as:

\code
	SetNodeTag(<mem controller}, "priority", "write")
\endcode
This command could then be complemented in a client with rules to use different
coloring for the node depending on the value of the "priority" tag. In the
following example, when the controller is in "prioritize-reads" mode, the GUI 
would draw the node' s outline in blue, while when in  "prioritize-writes" mode 
the GUI would draw the node's outline in green.

\code
    <node "mem controller">
        <set bordercolor=white />
        <if tagname= "priority" value= "write"> <set bordercolor=green /> </if>
        <if tagname= "priority" value= "read">  <set bordercolor=blue />  </if>
    </node>
\endcode

When the user sets a tag on a node, the association of the
<tag, value} pair is maintained across cycles until the simulation 
ends or, alternatively, until a new "SetNodeTag" command for the same
tag and the same node is issued (possibly changing the value of
the tag). In other words, once a tag is set, it will maintain its
value unless changed.

The <CODE>SetNodeTag</CODE> command fully understands the semantics of a
multi-dimensional node layout, thereby allowing setting tags not only
to a node as a whole, but to individual slots in the node or to groups
of slots (slices) in the node.  The dral C++ syntax for this functions is:

\code
  SetNodeTag(<nodeid>, <tag>, <value>, <slot id>) 
\endcode
The syntax to specify the <slot id> argument is a comma-separated list
of integers that index into each dimension of the layout (from left-to-right,
i.e., the left-most argument specifies the position for the left-most dimension
in the layout). It's in essence the same syntax used in C --except for the
missing brackets-- to designate a given (multi dimensional) array element.  

Assuming, for example, a three-dimensional layout, say [3][4][5], the
following example illustrates the available C++ syntax to set tags
to individual slots in the node:
\code
SetNodeTag(n, "locked", true, 2, 1, 4) 
\endcode
Tag <locked,true> is associated to slot [2][1][4] of node \<n\>. Note that the
tag is associated to the slot, not to the item inside the slot.
Even more, a tag can be associated to a slot that is empty.

Additionally, dral introduces special syntax to allow setting a tag
to multiple slots at once. Any index in the list can be substituted by a special
value, called DRAL_ALL.  The DRAL_ALL token can be though of as the equivalent
of the "*" operator in regular expressions: when placed in a given dimension
it will apply the tag to ALL indices in said dimension.  Continuing our
previous layout example, the following commands illustrate the possibilities of
the DRAL_ALL token:
\code
SetNodeTag(n, "locked", true, 2, DRAL_ALL, 4)
\endcode
Tag <locked,true> is set on <B>ALL</B> the following slots in node \<n\>: [2][0][4],
[2][1][4], [2][2][4], [2][3][4]

\code
SetNodeTag(n, "locked", true, 2, 2, DRAL_ALL)
\endcode
Tag <locked,true> is set on <B>ALL</B> the following slots in node \<n\>: 
[2][2][0], [2][2][1], [2][2][2], [2][2][3], [2][2][4]

Furthermore, the syntax allows still another powerful feature: 
not all indices need to be specified for all dimensions. Indices can be omitted (right to
left). In such a case, a sub-slice of the original layout is specified and the
tag applies to the slice as a whole, not to every individual slot in the slice.
Continuing with our example, we could ommit the third index, the second
and third indices or all three of them, as the following commands illustrate:

\code
SetNodeTag(node, "locked", true, 2, 2)
\endcode
In this example, the third
index is ommited. The tag <locked, true> is set to 
the [2][2] slice as a whole, which is different from setting
the tag to all slots within slice [2][2]. For example, this could
be used to draw a special background color for slice [2][2], or
to outline it in a different border color. 

\code
SetNodeTag(node, "locked", true, 1)
\endcode
In this example, both
the second and third indeces are ommited. The tag <locked, true> 
is set to the [1] slice as a whole, which is different from setting
the tag to all slots within slice [1]. 

\code
SetNodeTag(node, "locked", true) 
\endcode
In this example, similar to
the first example introduced in this section, the tag <locked, true>
is set to the node as a whole. 

\subsection GROUPING_NODES Grouping Nodes

Grouping Nodes exist for the sole purpose of giving a hierachical
structure to the graph. A Grouping Node can be the parent of other
grouping nodes or can be the parent of nodes (or both). Grouping nodes can
not have edges connecting them. Only normal nodes can be linked by edges.

As with normal nodes, the hierarchy information is encoded in the
<i>parent number</i> attribute. If a node is a child node of some other
node, then the id of the parent node should be encoded in the <i>parent
number</i> attribute. If a node has no parent, this field should be set to 0.

<I> OPEN ISSUE in dralversion: Should we allow setting tags on a
grouping node? Why? Why not?</I>

\subsection EDGES Edges

Edges are unidirectional links used to connect different nodes in the
graph. Each edge is uniquely identified by its source and destination
node. Edges are the "roads" over which items travel from node to node
during a simulation session.  Edges are ordered, failure-free
communication links. Items entering an edge can not overtake other
items that have entered the edge in previous cycles. Also, all items that
enter and edge will reach the end of an edge. It's illegal to execute a
<CODE>DeleteItem</CODE> command while an item is still traveling through an edge.

An edge has two fundamental properties: its <i>bandwidth</i> and its
<i>latency</i>. 

The edge's <i>bandwidth</i> indicates how many items can simultaneously enter 
the edge on its input side in a given cycle. An edge with bandwidth \f$B\f$
can be though of as a road with \f$B\f$ lanes, numbered 0 through \f$B-1\f$.
Conceptually, one (and only one) item can enter one of the lanes of
the edge per cycle.

The edge's <i>latency</i> indicates how many simulation
cycles it takes for items to fully traverse the edge and reach the
node at the output side of the edge. An edge, therefore, can be thought
of being divided into different "segments", depending on its latency.
An edge with a latency of three cycles will contain three segments, named
"a", "b" and "c". As of version dralversion, zero-latency edges
are allowed (better: tolerated) but they will be phased out in favor
or fractional latencies.

During simulation, the command <CODE>moveitems</CODE> indicates that the user
wishes to move one (or more) items across an edge. Once this command
has been issued, the items will take a number of cycles to
traverse the edge (determined by the edge's <i>latency</i>) and,
eventually, the items will reach the output side of the edge and
disappear from the edge (possibly entering a node). A variant
of the <CODE>moveItems</CODE> command allows specifying exactly over which
"lane" and item will travel through the edge. Items can not
change lanes during their journey.

\subsubsection EDGE_TAGS Edge Tags

At present, there is no provision for associating tags to an edge.
If you find a good use for them, please send email describing
your suggestions to dreamsmail.

\subsection ITMES Items

Dral <I>items</I> are abstract representations for the "messages" traveling
through the graph. Items are created by the dral user, can be moved
through the graph edges and nodes and, eventually, are destroyed when
they abandon the simulated system.

Items can be created in any simulation cycle by using the <CODE>NewItem</CODE>
command. Upon creation, the fundamental (and only) property of an item is
its <i>item identifier</i>. It's a unique 32-bit unsigned integer chosen
by the user at item creation time. The <i>item identifier</i> (or
"itemid" for short) is the handle used later to refer to the item and to
manage it throught its lifetime. While dral enforces no restrictions on
the sequence of item identifiers, users of a dral server are encouraged
to use a numbering scheme as densly populated as possible. Furthermore, if
at all possible, using monotonically increasing "itemid"s will also
greatly help the performance of many dral clients. 

Once an item has been created, the user can add attributes to it,
using the <CODE>SetTag</CODE> command, can move it from node to node 
by traversing the graph edges, using the <CODE>moveItems</CODE> command,
can store the item in one (or more) nodes using the <CODE>EnterNode</CODE>
and <CODE>ExitNode</CODE> commands and, eventually, destroy the item
with the <CODE>DeleteItem</CODE> command.

\subsubsection ITEM_TAGS Item Tags 

Similar to nodes, items can have associated to them an unlimited number of
<tag,value> pairs. This allows attaching arbitrary information to
an item that can be later used by the visualization tools to display
relevant data to the final user.  The tag name is a string of
up to 2048 bytes (which must include the terminating "\0" character).
The value associated to a tag can have one of three fundamental
data types: "64bit", "string" or "set-of-64bit". These data types
are described below:
<ul>
    <li><strong>64bit</strong> The value is an unsigned 64 bit quantity. It's up to the dreams
               clients to choose the appropriate format (hex, floating point, signed,
    	       unsigned, etc.) to display the value. Clients are
               encouraged to have in their configuration files a rich
               syntax to allow users specify the exact data type to be used to
               display a particular tag.

    <li><strong>string</strong> The value is a null terminated string. The string, including the
                null termination character must occupy less than 65536 bytes.

    <li><strong>set-of-64bit</strong> The value is an ordered set of 64bit quantities. The set
                may contain up to 65536 values. This is useful for
                attaching information to an item that is array-like in
                nature. Again, the appropriate format (hex, floating
                point, signed, unsigned, etc.) used to display each of the
                values in the set is left to the dreams client.
</ul>
When the user sets a tag on an item, the association of the
<tag, value> pair is maintained across cycles until the item
is destroyed or, alternatively, until a new "SetItemTag" command for the same
tag and the same item is issued (possibly changing the value of
the tag) (or, of course, until the simulation ends).
In other words, once a tag is set on an item, it will maintain its
value unless changed. 

As mentioned, it is legal to set different
values to the same tag on the same item as the simulation progresses.
The semantics in this case are clear: a given <tag,value> pair
will be in effect from the cycle the corresponding <CODE>SetItemTag</CODE>
is issued until the next SetItemTag is issued. In the case of repeated
<CODE>SetItemTag</CODE> commands being issued on the exact same cycle, the
last value set to the tag will prevail and all others will be lost. 

The semantics are illustrated in the following example trace:

\code
    cycle 50
    newItem id 234
    cycle 100
    setItemTag item=234 tag=ADDRESS value=1
    cycle 200
    setItemTag item=234 tag=ADDRESS value=17
    setItemTag item=234 tag=ADDRESS value=19
    setItemTag item=234 tag=ADDRESS value=2
    cycle 201
    cycle 300
    setItemTag item=234 tag=ADDRESS value=3
    cycle 350
    deleteItem 234
\endcode

Given this sequence of dral commands, the values associated
to the "ADDRESS" tag in each different cycle of the
simulation would be the ones illustrated in the
following table:

<p><DIV ALIGN=center>
<table border>
<TR> <TD align=left>
Cycle </TD> <TD align=center> 0..99 </TD> <TD align=center> 100..199 </TD> <TD align=center> 200..299 </TD> <TD align=center> 300..350  </TD> <TD align=center> 351..end </TD></TR>
<TR> <TD align=left>
Value </TD> <TD align=center>   ?   </TD> <TD align=center>    1     </TD> <TD align=center>     2    </TD> <TD align=center>  3   </TD> <TD align=center>    ?      </TD></TR>
<TR> <TD align=left>
</table>
</DIV>

First, note how before the first <CODE>SetItemTag</CODE> the 
"ADDRESS" tag does not exist and, hence, its value is
undefined  --despite the item having been created in cycle 50. After
the initial  <CODE>SetItemTag</CODE> in cycle 100, the "ADDRESS" tag
will hold a value of 1 until the next   <CODE>SetItemTag</CODE> command.
Note also, in cycle 200, how two values set to the "ADDRESS" 
tag are lost (values 17 and 19). This is due to the fact that
there are three updates to the same tag and item in the exact
same cycle. Finally, the last update to the "ADDRESS" tag
happens in cycle 300 and the value "3" will be in effect until
the item is destroyed in cycle 350. After that cycle, since
the item has been deleted, no reference to the tag can
be possible.


\subsection Cycles

<I> Cycles</I> are the discrete units of time in which the simulation
advances. All events in dral (creation/deletion of an item,
item movement or tag associations) happen within a given cycle --though
their effect, of course, can span multiple cycles.

*/

