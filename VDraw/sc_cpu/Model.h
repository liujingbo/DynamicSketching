/*
 * model.h
 *
 * A class representing a worldspace model
 *
 *      Author: Jeroen Baert
 *      Modified by Jingbo Liu
 */


//#include "Drawer.h"
// fitting
//#include "fitting\FittingCylinder.h" // help me 

// Warning : Putting the above lines below ifnedf.. will cause a lot of error your dont know how to solve
#ifndef MODEL_H_
#define MODEL_H_
#include <TriMesh.h>
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glui.h>
#include <iostream>
#include <fstream>
#include <string>

// hierarchy graph
// #include "../ordering/HierarchyGraph.h"
//#include "Xform.h"
#include "../ordering/vGraph.h"
#ifndef Q_MOC_RUN
#include "boost/graph/adjacency_list.hpp"
#endif 

// forward declarations, for cross include problem
class Drawer;
class FittingCylinder;

// convex hull
#include <algorithm>
#include <vector>
typedef int coord_t;         // coordinate type
typedef long long coord2_t;  // must be big enough to hold 2*max(|coordinate|)^2

// chaining
#include "vCurve.h"

struct Point {
		coord_t x, y;

		bool operator <(const Point &p) const {
			return x < p.x || (x == p.x && y < p.y);
		}
	};
class Model
{
/**
 * Definition of the Model class, representing a loaded mesh and its view-independent and view-dependent data.
 *
 * Author: Jeroen Baert
 */
	
private:
	// some private helper functions
	void computeViewIndependentData();
	void clearViewDependentData();
	void setupVBOs();

	void postSetPatchNumber();

public:

	// the mesh_ representing this model
	const TriMesh* mesh_;
	// the drawer stack
	vector<Drawer*> drawers_;

	// vertex buffer objects for GPU storage
	GLuint vbo_positions_;
	GLuint vbo_normals_;

	// VIEW INDEPENDENT VALUES
	vector<vec> facenormals_;
	float feature_size_;
	vector<int> facecolors_; // for patch color
	vector<int> pointcolors_;
	vector<float> colorlist_; // all available colors
	
	vector<double> faceareas_;// todo
	vector<vec> facecenters_;// todo

	// VIEW_DEPENDENT VALUES
	vector<float> ndotv_; // ndotv_
	vector<float> kr_; // radial curvature
	vector<float> num_; // second derivative of radial curv
	vector<float> den_; // second derivative of radial curv

	// constructor
	Model(const char* filename);
	~Model();

	// draw the model
	void draw(vec camera_position);
	// pop a drawer from the drawer stack
	void popDrawer();
	// push a drawer into the drawer stack
	void pushDrawer(Drawer* d);
	// clear all drawers_ from the drawer stack
	void clearDrawers();

	// compute ndotv_ for all vertices in this model, given a camera position
	void needNdotV(vec camera_position);
	// compute all curvature derivatives in this model, given a camera position
	// and a threshold for small derivatives
	void needCurvDerivatives(vec camera_position, float sc_threshold);

	/********************************************************************
	                         Hierarchy 
	*********************************************************************/	
	//bool setHierarchyGraph(HierarchyGraph* hg_);
	//HierarchyGraph* hierarchy;

	// segmentation connectivity graph
	vGraph* graph; // may consider let Model be responsible for vGraph
	std::vector<NodeID> segment2NodeID;
	void setGraph(vGraph* _g, std::vector<NodeID> _nIDs);

	/********************************************************************
	                        Primitive  Fitting
	*********************************************************************/
	vector<vector<int>> patches; // index of faces
	vector<vector<int>> patches_vertex_index; // index of vertices int[a][], the vertices' indices of segment a
	vector<FittingCylinder*> fitting_cylinders;
	vector<vector<vector<int>>> segmentationSharedVertices; //int[a][b][] : indices of shared vertices between segmenta a & b

	// TODO : supplement material to fit with "Mesh" class
	void initPatches();
	double ComputeFaceArea(int fIndex);
	double ComputeAllFaceAreaCenter();
	double fitCylinderPrimitive();
	double fitCylinderPrimitive(const char* st );

	/********************************************************************
	                Processing before pasing to 2D domain
	*********************************************************************/
	// edgecontour
	// if edge contour drawer is computed, then this is ready
	// std::vector<vec> edgeContour;
	std::vector<int> edgeVertices;
	bool* edgeVerticesIndexed;
	int numOfSources;
	bool inRingNeighbour(int, int);
	bool in2RingNeighbour(int , int);
	std::vector<std::vector<int>>* tryMergeChains(std::vector<std::vector<int>>*);
	std::vector<std::vector<int>> buildEdgeChains();
	std::vector<int> buildSingleChain(int);
	std::vector<std::vector<int>> buildEdgeChains2();
	std::vector<std::vector<int>>* tryMergeChains_strict(std::vector<std::vector<int>>*);

	// pose lines
	// when fitting primitives are ready, get this ready
	// segmentationSharedVertices is one important data for this
	std::vector<std::vector<vPoint3D>> poselines; // final output
	std::vector<std::vector<vPoint3D>> * buildPoseLines();
	std::vector<std::vector<vec>> poseSpliting(vec center, std::vector<vec>);
	std::vector<std::vector<vPoint3D>> poseRecursiveSpan(NodeID v, EdgeID from,  bool root); // finally being used

	// suggestive contorus
	std::vector<std::vector<vec>> suggestiveContour; // the raw curves of sc
	std::vector<int> scSegmentationIndices;	// the segmentation each curve belongs to
	
	
	double connectDistThreshold;
	double connectDistThreshold_max;
	double connectDistThreshold_min;
	double connectAngThreshold;
	double connectAngThreshold_max;
	double connectAngThreshold_min;
	double paraNameChanged_coonectDist(double & max, double & min );
	double paraNameChanged_coonectAng(double & max, double & min );
	void paraValChanged_coonectDist(int val);
	void paraValChanged_coonectAng(int val);

	// preprocess
	std::vector<int> filterUnderDepthVertices(std::vector<int>* ,GLfloat*, int w, int h);
	std::vector<std::vector<vec>>* filterUnderDepthSegments(std::vector<std::vector<vec>>* allChains ,GLfloat*, int w, int h,std::vector<std::vector<vec>>*);
	std::vector<std::vector<vec>>* filtlerLongSegments(std::vector<std::vector<vec>>* allChains);
	
	// main algo
	// TODO: use segmentation to speed up merging: only merge the segments in the same and neighbor segmentations
	std::vector<std::vector<vec>>* tryMergeChains(std::vector<std::vector<vec>>* allChains);
	void tryMergeEdgeAndSC(std::vector<std::vector<int>>* , std::vector<std::vector<vec>>* , std::vector<std::vector<vec>>* );
	// helpers
	bool inNearNeighbour(vec v1, vec v2);
	int contourConnecting(vec,std::vector<int>* , double &); // return the spliting point's index of edge
	int contourConnecting(vec,std::vector<vec>* , double& ); // overload for merged contour act like edge candidate
	int TjunctionResolution_sc(std::vector<vec>* , std::vector<int>* , int, double&  ,bool); // connecting sc to edge at T-junction
	int TjunctionResolution_sc(std::vector<vec>* , std::vector<vec>* , int , double& , bool );// overloading
	int TjunctionResolution(vec , vec , vec, double& );
	void updateMOM(vec neib[], double& mom, int & mom_ind);
	vec2 projDir3(vec v);
	vec projVec3(vec v);

	// face based sc curves
	std::vector<int> scFaces; // raw faces of sc
	std::vector<std::vector<int>> buildFaceChains(std::vector<int>* faces);
	bool inFaceIndicesList(int, std::vector<int>*);
	std::vector<int> buildSingleFaceChain(int source, std::vector<int>* faces, std::vector<bool>* visited);
	std::vector<std::vector<int>> tryMergeFaces(std::vector<std::vector<int>>* faceChains);
	bool inFaceNeighbour( int ,int );
	std::vector<std::vector<int>> faceChain2VertexChain(std::vector<std::vector<int>>* faceChains);
	int TjunctionResolution_assert(vec _sc, vec _e_begin, vec _e_end, double& cos_val);
	int TjunctionResolution_sc(std::vector<int>* sc, std::vector<int>* e, int split_pos, double& cos_val, bool ishead);
	std::vector<std::vector<int>> buildEXSC(std::vector<std::vector<int>>* sc, std::vector<std::vector<int>>* edge);
	/********************************************************************
	                            covex hull
	*********************************************************************/
	// convex hull
	double* modelview;
	double* projection;
	int* viewport;
	vector<vector<vec2>> convex_hulls_2d;

	// Implementation of Andrew's monotone chain 2D convex hull algorithm.

	// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
	// Returns a positive value, if OAB makes a counter-clockwise turn,
	// negative for clockwise turn, and zero if the points are collinear.
	coord2_t cross(const Point &O, const Point &A, const Point &B);
	// Returns a list of points on the convex hull in counter-clockwise order.
	// Note: the last point in the returned list is the same as the first one.
	vector<Point> convex_hull_2d(vector<Point> P);
	bool compute_convex_hull_2d(vector<Point> P, int);
	void update_convex_hull_2d(int, bool);

	vector<vector<int>> branches; // from skeleton, int[a][]: segment numbers in branch a
	void buildBranches(std::vector<std::vector<vPoint3D>>* skeleton);
};

#endif /* MODEL_H_ */
