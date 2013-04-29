#include "vEdge.h"

vEdge::vEdge(){
	init();
}
vEdge::~vEdge(){}

void vEdge::init(){
	type = 0;
	cost = 1;
	vertices=NULL;
}

void vEdge::setVertices(const std::vector<vec>* _vertices){
	vertices = _vertices;
}
void vEdge::push_vertex_index(int _ind){
	v.push_back(_ind);
	return;
}
void vEdge::copy_vertices_index_array(std::vector<int> v_array){
	v =  v_array;
}

void vEdge::computeCenter(){
	vec accum;
	for(int i=0;i<v.size();i++){
		accum += vertices->at(v[i]);
	}
	this->center = accum /(float)v.size();
}
