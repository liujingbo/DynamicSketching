#ifndef VNODE_H_351342151617124325
#define VNODE_H_351342151617124325

#include "Vec.h"
#include <vector>
#include <queue>
#include "..\vCurve.h"

// to draw
#include <opencv/cv.h>
#include <opencv/cxcore.h>

// graph
#ifndef Q_MOC_RUN
#include "boost/graph/adjacency_list.hpp"
#endif 

class vGraph;


class vNode{
private:
	void init();
	std::vector<int> v; // index
	const std::vector<vec>* vertices; // mesh data, stored elsewhere; one time use for now
	vCurve* stroke; // the stroke the node represent: elementary node

	float Dist(vec2 uc, vec2 u, vec2 v);
	float Dist(vec2 uc, vec2 u, vec2 vc, vec2 v); // distances between two line segments u and v
public:
	int segmentID; // -2 for global (sim Hull)
	std::vector<int> coveredSegments;
	vec center;
	//vec2 controlPoint;
	//float lengthAlpha;
	int hsv_h;

	bool flag_pose;// for pose construction
	bool visited;

	// ============= leaf property =======================
	bool isLeaf;	// use supernode or not
	double entropy;
	double information;
	static float beta;
	static float lamda;
	static float fieldSize;
	double voting; // sperially for content reference (exsc approximates)

	// =============  supernode property =======================
	// make super node, how to make two files include each other
	// seems working, use some forward declaration trick
	// and include .h in the other's cpp file
	vGraph* superNode;

	vNode();
	vNode(int _segID);
	~vNode();

	void push_vertex_index(int);
	void setVertices(const std::vector<vec>* _vertices);
	void setCurve(vCurve*);
	vCurve* getCurve();
	void computeCenter();
	void draw(IplImage* img, CvScalar color, int rate=4, int LOD=10, bool output = false);

	// entropy
	double computeCurEntropy(const vGraph& );
	double computeInformation();
	double megaForce( vCurve* c1,  vCurve* c2);
	double updateVoting(vGraph&); // if the node is chosen to put into the order, it needs to update the content reference's voting -> incremental entropy
	//void computeControlPoint();//--------------------new-----compute-----------------------
	void setH(int h);
	int getH();
};





#endif