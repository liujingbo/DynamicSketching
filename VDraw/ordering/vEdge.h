#ifndef VEDGE_H_9620374586296
#define VEDGE_H_9620374586296

#include "vNode.h"
#include "Vec.h"
#include <vector>

class vEdge{

private:
	int type;
	std::vector<int> v; // shared vertices (indices) between two nodes
	const std::vector<vec>* vertices; // mesh data, stored elsewhere
	
	void init();
public:

	// structure info
	vNode* from;
	vNode* to;

	//mesh info
	vec center;

	// properties
	float cost;

	vEdge();
	~vEdge();
	
	void setVertices(const std::vector<vec>* _vertices);
	void push_vertex_index(int _ind);
	void copy_vertices_index_array(std::vector<int> v_array);
	void computeCenter();


};


#endif