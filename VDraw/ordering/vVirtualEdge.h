#ifndef VVIRTUALEDGE_h_510395012950
#define VVIRTUALEDGE_h_510395012950


#include "vGraph.h"
#include <vector>

class vVirtualEdge{

public:
	vVirtualEdge();
	~vVirtualEdge();

	// this class is not responsible to create or delete the followings
	vGraph* prevGraph;
	vGraph* nextGraph;

	std::vector<std::vector<NodeID>> map2prev;
	std::vector<std::vector<NodeID>> map2next;



private:
	void init();

};


#endif