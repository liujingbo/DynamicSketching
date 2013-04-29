#include "vVirtualEdge.h"

vVirtualEdge::vVirtualEdge(){
	init();
}

vVirtualEdge::~vVirtualEdge(){}

void vVirtualEdge::init(){
	prevGraph = NULL;
	nextGraph = NULL;

}