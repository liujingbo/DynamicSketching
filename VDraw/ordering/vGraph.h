#ifndef VGRAPH_H_0997882606547236
#define VGRAPH_H_0997882606547236

#ifndef Q_MOC_RUN
#include "boost/graph/adjacency_list.hpp"
#endif 

#include "vNode.h"
#include "vEdge.h"

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, vNode*, vEdge*> _Graph;
typedef _Graph::vertex_descriptor NodeID;
typedef _Graph::edge_descriptor   EdgeID;

class vVirtualEdge;

class vGraph{
public:
	_Graph g;
	vGraph();
	~vGraph();

	vVirtualEdge* virtualEdges; // responsible to delete, but not create

	void clear();
	NodeID addNode(vNode* n){
		NodeID nID = boost::add_vertex(n,g);
		int tmp = g[nID]->segmentID;
		return nID;
	}

	EdgeID addEdge(NodeID u, NodeID v, vEdge* e){
		e->from = g[u];
		e->to = g[v];
		
		EdgeID eID;
		bool ok;
		boost::tie(eID, ok) = boost::add_edge(u,v,e,g);//
		if (ok)  // make sure there wasn't any error (duplicates, maybe)
		{
			assert(e == g[eID]);
		}
		return eID;
	}

	vNode* getNode(NodeID nID){
		return g[nID];
	}

	void clearVoting();
	void clearFlag();
};

#endif