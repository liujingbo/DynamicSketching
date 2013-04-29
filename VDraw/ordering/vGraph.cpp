#include "vGraph.h"

vGraph::vGraph(){
	virtualEdges = NULL;
}


vGraph::~vGraph(){

	clear();
}

void vGraph::clear(){
	_Graph::vertex_iterator vertexIt, vertexEnd;
	
	boost::tie(vertexIt, vertexEnd) = boost::vertices(g);
	for (; vertexIt != vertexEnd; ++vertexIt){
		NodeID nID = *vertexIt; // dereference vertexIt, get the ID
		vNode* n = g[nID];
		if (n!=NULL){delete n;} // Gotcha
	}

	_Graph::edge_iterator eIt, eEnd;
	boost::tie(eIt, eEnd) = boost::edges(g);
	for(; eIt!=eEnd; ++eIt){
		vEdge* e = g[*eIt];
		if(e!=NULL)delete e;
	}

	this->g.clear();

	// clear virtual edges
	if(this->virtualEdges){
		delete virtualEdges;
		virtualEdges = NULL;
	}
	return;
}

void vGraph::clearVoting(){
	_Graph::vertex_iterator vertexIt, vertexEnd;
	
	boost::tie(vertexIt, vertexEnd) = boost::vertices(this->g);
	for (; vertexIt != vertexEnd; ++vertexIt){
		NodeID nID = *vertexIt; // dereference vertexIt, get the ID
		vNode* n = g[nID];
		if (n!=NULL){
			if(n->isLeaf){
				n->voting = 0;
			}
			else{// super node

				_Graph& sng = n->superNode->g;
				_Graph::vertex_iterator vertexIt2, vertexEnd2;
				for (boost::tie(vertexIt2, vertexEnd2) = boost::vertices(sng); vertexIt2 != vertexEnd2; ++vertexIt2){
					NodeID nID2 = *vertexIt2; // dereference vertexIt, get the ID
					vNode* n2 = sng[nID2];
					assert(n2->isLeaf);
					n2->voting = 0;
				}
			}
		} 
	}
	return;
}

void vGraph::clearFlag(){

_Graph::vertex_iterator vertexIt, vertexEnd;
	
	boost::tie(vertexIt, vertexEnd) = boost::vertices(this->g);
	for (; vertexIt != vertexEnd; ++vertexIt){
		NodeID nID = *vertexIt; // dereference vertexIt, get the ID
		vNode* n = g[nID];
		if (n!=NULL){
			if(n->isLeaf){
				n->visited = 0;
			}
			else{// super node
				_Graph& sng = n->superNode->g;
				_Graph::vertex_iterator vertexIt2, vertexEnd2;
				for (boost::tie(vertexIt2, vertexEnd2) = boost::vertices(sng); vertexIt2 != vertexEnd2; ++vertexIt2){
					NodeID nID2 = *vertexIt2; // dereference vertexIt, get the ID
					vNode* n2 = sng[nID2];
					assert(n2->isLeaf);
					n2->visited = 0;
				}
			}
		} 
	}
	return;
}