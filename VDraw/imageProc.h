#ifndef IMAGEPROC_H31415926535897932384626
#define IMAGEPROC_H31415926535897932384626

/**********************************************************
put image processing stuff here

**********************************************************/

/*
using prebuild version of opencv 2.10
://opencv.willowgarage.com/wiki/VisualC%2B%2B_VS2010
*/

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <vector>
#include <TriMesh.h>

#include "vCurve.h"
#include "fitting/Primitives.h"
#include "Vec.h"
#include <GL/glut.h>

#include <iostream>
#include <fstream>

#ifndef Q_MOC_RUN
#include "boost/graph/adjacency_list.hpp"
#include <boost/range/counting_range.hpp>
#include "boost/graph/breadth_first_search.hpp"
#endif 
//typedef int coord_t;         // coordinate type
//typedef long long coord2_t;  // must be big enough to hold 2*max(|coordinate|)^2
// may change, only need the Point struct

#include "ordering/vGraph.h"
#include "ordering/vVirtualEdge.h"
// struct Point;

template<typename T> // T is vCurve
class SortCurveSet
{
public:
	SortCurveSet(std::vector<T> *_sortArray, T* _reference) : sortArray(_sortArray), ref(_reference) {;}
	bool operator()(int lhs, int rhs)
	{
		return distance(&(sortArray->at(lhs)),ref) < distance(&(sortArray->at(rhs)),ref) ;
	}

private:
	double distance(vCurve* c1, vCurve* c2){
		return euclideanDistance(c1,c2);
		//double d12 = 
	}
	double euclideanDistance(vCurve* c1, vCurve* c2){
		double h = INT_MAX;
		for(int i=0; i<c1->size();i++){
			double shortest = INT_MAX;
			for(int j=0; j<c2->size(); j++){
				float dij = dist(c1->curve2d[i].toVec2(),c2->curve2d[j].toVec2());
				if(dij < shortest)
					shortest = dij;
			}
			if(shortest < h)
				h=shortest;
		}
		return h;
	}
	double hausdorffDiscance(vCurve* p, vCurve* q){

		double forward = directedHausdorffDiscance(p, q);
		double backward = directedHausdorffDiscance(q, p);

		return MAX(forward, backward);
	}
	double directedHausdorffDiscance(vCurve* c1, vCurve* c2){

		double h = 0.0;
		for(int i=0; i<c1->size();i++){
			double shortest = INT_MAX;
			for(int j=0; j<c2->size(); j++){
				double dij = dist(c1->curve2d[i].toVec2(),c2->curve2d[j].toVec2());
				if(dij < shortest)
					shortest = dij;
			}
			if(shortest> h)
				h=shortest;
		}
		return h;
	}
	std::vector<T> *sortArray;
	T* ref;
};

template<typename T> // T is NodeID
class SortNodeIDwrtDistance
{
public:
	SortNodeIDwrtDistance(_Graph& _graph, std::vector<T> *_sortArray, T* _reference) : graph(_graph),sortArray(_sortArray), ref(_reference) {}
	bool operator()(int lhs, int rhs)
	{
		vNode* u = graph[sortArray->at(lhs)];
		vNode* v = graph[sortArray->at(rhs)];
		vNode* r = graph[*ref];
		return distance(u->getCurve(),r->getCurve()) < distance(v->getCurve(),r->getCurve()) ;
	}

private:
	double distance(vCurve* c1, vCurve* c2){
		return euclideanDistance(c1,c2);
		//double d12 = 
	}
	double euclideanDistance(vCurve* c1, vCurve* c2){
		double h = INT_MAX;
		for(int i=0; i<c1->size();i++){
			double shortest = INT_MAX;
			for(int j=0; j<c2->size(); j++){
				float dij = dist(c1->curve2d[i].toVec2(),c2->curve2d[j].toVec2());
				if(dij < shortest)
					shortest = dij;
			}
			if(shortest < h)
				h=shortest;
		}
		return h;
	}
	double hausdorffDiscance(vCurve* p, vCurve* q){

		double forward = directedHausdorffDiscance(p, q);
		double backward = directedHausdorffDiscance(q, p);

		return MAX(forward, backward);
	}
	double directedHausdorffDiscance(vCurve* c1, vCurve* c2){

		double h = 0.0;
		for(int i=0; i<c1->size();i++){
			double shortest = INT_MAX;
			for(int j=0; j<c2->size(); j++){
				double dij = dist(c1->curve2d[i].toVec2(),c2->curve2d[j].toVec2());
				if(dij < shortest)
					shortest = dij;
			}
			if(shortest> h)
				h=shortest;
		}
		return h;
	}
	std::vector<T> *sortArray;
	T* ref;
	_Graph& graph;
};

class imageProc{

public:
	IplImage* ori_image;
	IplImage* display_image;
	int w;
	int h;
	imageProc();
	~imageProc();
	void reset();
	void clearData();

	// curves properties
	std::vector<vCurve> curves;

	// drawers
	void draw();

	// textures
	IplImage* texture;
	void loadTexture(char* fileName);

	// segment relations: only affect ordering, or put them here
	std::vector<std::vector<int>> segmentSymmetricity;

	// others
	void setOrignalImage(IplImage* _img);
	void testBlack();

	/**********************************************************

						UI control from vDraw

	***********************************************************/
	// filters
	void canny();
	int houghTransform();

	// parameters
	double paraNameChanged_split(double & max, double & min );
	void paraValChanged_split(int);
	void setEdgeContourVisible(){this->isEdgeContourVisible = true;}
	void setEdgeContourUnvisible(){this->isEdgeContourVisible =false;}
	void setPoseLineVisible(){this->isPoseLinesVisible = true;}
	void setPoseLineUnisible(){this->isPoseLinesVisible = false;}
	void setSCVisible(){this->isSCVisible = true;}
	void setSCUnvisible(){this->isSCVisible = false;}
	void setConvexHull2dVisible(){this->isConvexHullVisible=true;}
	void setConvexHull2dUnvisible(){this->isConvexHullVisible=false;}
	void setPrimLinesVisible(){this->isPrimVisible=true;}
	void setPrimLinesUnisible(){this->isPrimVisible=false;}

	// loading
	bool loadSegmentRelations(const char* str);

	/**********************************************************

					order

	***********************************************************/
	
	// get from load segmentation, same as Model
	vGraph* SegConnectivity;
	std::vector<NodeID> segment2NodeID;

	// graph
	// g[0] - pose line
	// g[4] [1]- simplified hull
	// g[1] [2]- simplified edge
	// g[2] [3]- primitives / convex hull
	// g[3] [4]- exsc; the registration reference; the approximation of content
	
	vGraph graphs[5]; // clear these when reload trimesh
	bool useGraph;
	bool useOrder;
	
	int numOfStrokes;
	// curve order
	std::vector<int> SCorder;
	std::vector<int> PoseOrder;
	std::vector<int> SimEdgeOrder;
	std::vector<int> ConvexHull2dOrder;
	// graph order
	std::vector<int> segDrawingOrder;
	std::vector<std::vector<NodeID>> poseGraphOrder;
	std::vector<std::vector<NodeID>> simHullGraphOrder;
	std::vector<std::vector<NodeID>> SimEdgeGraphOrder;
	std::vector<std::vector<NodeID>> PrimsGraphOrder;
	std::vector<std::vector<NodeID>> EXSCGraphOrder;
	// erase order
	std::vector<std::vector<NodeID>> poseEraseGraphOrder;
	std::vector<std::vector<NodeID>> simHullEraseGraphOrder;
	std::vector<std::vector<NodeID>> SimEdgeEraseGraphOrder;
	std::vector<std::vector<NodeID>> PrimsEraseGraphOrder;
	std::vector<std::vector<NodeID>> EXSCEraseGraphOrder;

	// UI control
	void numOfStrokesChanged(int);

	// ouput control
	bool output;

	// definitions
	double priority(double _entro, double _info, vNode* _n);
		
	// commands
	int order2DStrokes(); // return the number of all curves
	std::vector<int> orderCurveSet(std::vector<vCurve>*);
	int findStartingIndex(std::vector<vCurve>*);
	std::vector<NodeID> findStartNode(vGraph&, vGraph&); // max C=E/I
	std::vector<NodeID> findTopNode(vGraph&); // from up to bottom
	std::vector<std::vector<NodeID>> orderCurveSet(std::vector<NodeID> start, vGraph& g, vGraph& ref, bool partBypart = false);
	std::vector<int> buidSegOrder(  vGraph& skeleton, std::vector<NodeID> startSeg,vGraph* connection , std::vector<NodeID>* seg2nID);
	// erasing strokes
	std::vector<std::vector<NodeID>> findErasingStrokes(std::vector<NodeID> start, vGraph& g, vGraph& ref, bool partBypart = false);

	// distance metrics
	double distance(vCurve*, vCurve*);
	double euclideanDistance(vCurve*, vCurve*);
	double hausdorffDiscance(vCurve*, vCurve*);
	double directedHausdorffDiscance(vCurve*, vCurve*);
	double computeEntropy();
	double computeInformation();
	double computeDistortion();

	// common functionalities
	void addGraphEdgesWRTDistances(vGraph& g, std::vector<NodeID>& nodeids);
	void addSuperEdgesWRTConnectivity(vGraph& g, vGraph* connection);
	bool chkSegmentConnectByID(int segid1, int segid2, vGraph* connection);

	//projection
	vec2 projDir3(vec v);
	vec projVec3(vec v);



	/**********************************************************

						edge contours

	***********************************************************/
	bool isEdgeContourVisible;
	double splitThreshold;
	double splitThreshold_max,splitThreshold_min;
	std::vector<vCurve> edgeContours; // to be drawn
	double* modelview;
	double* projection;
	int* viewport;
	void addEdgeContourChains(std::vector<std::vector<int>> * , const std::vector<vec>*,  vec);//  set intensity
	void splitEdgeContourChains();
	void splitEdgeContourChainsRecursively();
	void splitVertexChainsRecursively(std::vector<vCurve>& vertexChain, double error_tolerance);
	bool splitConstrain(int, int); // constrains: (1) not splitting at either end of the curve
	std::vector<vCurve> straghtLineApproximation(vCurve*, double );// approximate an almost straight curve to a straight line
	// well, my idea is essentially the reinvention of Ramer–Douglas–Peucker algorithm, wish I had born earlier
	double straightLineError(vCurve*, int&);
	void buildSimEdgeGraph(vGraph&, std::vector<int>* );
	/*
		Visvalingam's algorithm
	*/
	void visvalingamSplitting(std::vector<vCurve>& vertexChain, double error_tolerance);
	std::vector<vCurve> visvalingamAlgorithm(vCurve*, double );

	/**********************************************************

						pose Lines

	***********************************************************/
	bool isPoseLinesVisible;
	std::vector<vCurve> poseLines; // to be drawn
	void addPoseLine(std::vector<std::vector<vec>> * );
	void addPoseLine(std::vector<std::vector<vPoint3D>> *  );
	void buildPoseGraph(vGraph& g);

	/**********************************************************

						Convex Hull

	***********************************************************/
	bool isConvexHullVisible;
	std::vector<vCurve> convexHulls2d; // to be drawn
	int numOfGloabHull;
	std::vector<std::vector<vec2>> buildSimHull(std::vector<std::vector<vec2>> * _chains, double frac=0.01);
	void addConvexHullLines(std::vector<std::vector<vec2>> *  _chains);
	void addConvexHullLinesAndBuildSimHullGraph
		(std::vector<std::vector<vec2>> *  _chains, vGraph& g, std::vector<std::vector<int>> branches,bool usebranch = false);

	/**********************************************************

						Primitives

	***********************************************************/
	bool isPrimVisible;
	std::vector<vCurve> PrimitiveLines; // to be drawn
	void addPrimitiveLinesAndBuildPrimsGraph(std::vector<Prism>* ps, vGraph&, vec); // each supper node I ~ one primitive,  set intensity

	/**********************************************************

						Suggestive Contours

	***********************************************************/
	bool isSCVisible;
	std::vector<vCurve> SCLines; // to be drawn
	std::vector<vCurve> silhouetteLines; // to be drawn
	void addSCLine(std::vector<std::vector<vec>> * _edge);
	void addSCFaces(std::vector<std::vector<int>>*, const vector<TriMesh::Face>* fList, const std::vector<vec>* vList);
	void addSCLines(std::vector<std::vector<int>> * _chains, std::vector<std::vector<int>> * silhouette,const std::vector<vec>* vertices, 
		vec light, vec camera_pos, const std::vector<vec>* normals, const vector<float>* curv1, const vector<float>* curv2);
	void splitEXSCChainsRecursively();
	void buildEXSCGraph(vGraph&, std::vector<int>* );
};




#endif // IMAGEPROC_H31415926535897932384626