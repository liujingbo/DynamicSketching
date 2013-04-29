#ifndef STELLAWIDGET_H
#define STELLAWIDGET_H
//#include <QtGui>
#include <QPainter>
#include <QTimer>
#include <QColor>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QWidget>
#include <vector>
#include <TriMesh.h>
#include <cmath>

#include <QtGui/QMouseEvent>
#include "vCurve.h"
#include "fitting/Primitives.h"
#include "Vec.h"
#include <GL/glut.h>
#include "imageProc.h"

#ifndef Q_MOC_RUN
#include "boost/graph/adjacency_list.hpp"
#include <boost/range/counting_range.hpp>
#include "boost/graph/breadth_first_search.hpp"
#endif 
//typedef int coord_t;         // coordinate type
//typedef long long coord2_t;  // must be big enough to hold 2*max(|coordinate|)^2
// may change, only need the Point struct

#include "ordering/vGraph.h"

class HSV{
public:
	int H, S, V;
	HSV(int H, int S, int V){
		this->H = H;
		this->S = S;
		this->V = V;
	}
};

class StellaWidget : public QWidget
{
	Q_OBJECT

		enum MouseStatus{
			NO_EVENT, LEFT_DOWN, RIGHT_DOWN, MIDDLE_DOWN, 
			LEFT_MOVE, RIGHT_MOVE, MIDDLE_MOVE
	};
	// 	enum EditingMode{
	// 		NORMAL_MODE, VEDGE_MODE
	// 	};

public:
	StellaWidget(QWidget *parent);
	~StellaWidget();
private:
	// Animation timer. The callback function is updateAnimation.
	QTimer* timer;

	// Editing mode
	//EditingMode editingMode;

	// The total number of graphs and the graph arrays.
	int graphsNum;
	vGraph** graphs;

	// The previous and next graph index.
	int* previousGraph;
	int* nextGraph;

	// The line drawing order. order[i][j][k] means the order 
	// of the kth leaf node of the jth super node in the ith graph.
	vector<vector<vector<NodeID>>> order;

	// image process, which is used to reorder
	imageProc* image_process;

	// The final displayed image, which directly painted on the widget.
	cv::Mat displayImage;

	// The background image of each graph, which helps to improve the 
	// updating efficiency.
	cv::Mat* graphBGImage;

	// The node ID image, which is used to do do pickup judgment. 
	// It mainly serves for mouse event function.
	cv::Mat nodeIDImage;

	// The background ID image.
	cv::Mat* graphBGIDImage;

	// Animation image, which is used to fast update the animation.
	// Each frame just update one spline. The animation is controlled
	// by the latter two variables.
	cv::Mat animationImage;
	int currentCurveSplineNo;
	int currentCurveNo;

	// Scale factor = window size / original size.
	double scale;

	// Mouse status, used to transport the mouse signal between mouse event
	// function. The downPos stores the mouse position to do mouse move operation.
	MouseStatus ms;
	cv::Vec2i downPos, deltaPos;

	//----------------selection variables------------------
	//Current edited graph No.
	int currentGraphNo;
	NodeID selectedSuperNodeID;
	NodeID selectedLeafNodeID;
	vector<NodeID> neighborNodeID;
	// 	NodeID selectedEdgeFrom;
	// 	NodeID selectedEdgeTo;
	// 	NodeID selectedVEdgeFrom;
	// 	NodeID selectedVEdgeTo;

public:
	//------------------new methods--------------------

	//void drawNormalMode();

	//void drawNormalSuperMode();

	//void drawNormalLeafMode();


	void draw();

	void drawSilentSuperMode();
	void drawSilentLeafMode();

	void drawSuperEdgeEditingMode();
	void drawVEdgeEditingMode();
	void drawLeafEdgeEditingMode();
	void drawLeafNodeMovingEditingMode();

	void drawNodeIDCurrentMode();
	void drawNodeIDPreviousMode();

	void resetSelectionVariables();
	bool isSuperNodesLinked(int graphNo, int nodeID1, int nodeID2);
	bool isLeafNodesLinked(int graphNo, int superNodeID,
		int leafNodeID1, int leafNodeID2);

	int getSuperNodeHSV_H(int graphNo, int nodeID);
	int getLeafNodeHSV_H(int graphNo, int superNodeID, int leafNodeID);

	int getPassedSuperNodeID();
	int getPassedLeafNodeID(int superNodeID);

	void drawBackgroundLayer(int graphNo);

	void drawSuperNodeNeighbors(HSV hsv);
	void drawLeafNodeNeighbors(HSV hsv);
	void drawSuperNodeVNeighbors(HSV hsv);
	void calculateSuperNeighbors();
	void calculateLeafNeighbors();
	void calculateVirtualNeighbors();

	void processLeafNodeNeighbor();
	void processSuperNodeNeighbor();
	void processSuperNodeVNeighbor();
	void processLeafNodeMovingOrSuperNodeCreation();
	void replaceLeafNodeID(int originalSuperNodeID, int originalLeafNodeID, 
		int currentSuperNodeID, int currentLeafNodeID);
	void replaceSuperNodeID(int originalSuperNodeID, int currentSuperNodeID);
	void createOriginalVNode2NodeIDMap(_Graph &, std::map<vNode*, NodeID> &vn2nid);
	void createSuperNode();
	void combineSuperNode(NodeID superNodeID);
	void moveLeafNode(NodeID superNodeID);
	void replaceVirtualEdgeEnd(int originalNodeID, int currentNodeID);
	void deleteVirtualEdge(int originalNodeID);
	void linkTheNeighborsOfNodeToNewNode(_Graph &g, NodeID nodeID, NodeID newNode);
	void copyVirtualEdgeToNewNode(int parent, int newone);
	void combineVirtualEdgeToNewNode(int parent, int deletedone);
	void replaceNodeIDInVertualEdge(int originalNodeID, int currentNodeID);
	//void changeCurrentVVEOfCurrentGraph(int originalSuperNodeID, int currentSuperNodeID);
	//void changeThePreviousVVEOfNextGraph(int originalSuperNodeID, int currentSuperNodeID);
//	void changeTheNextVVEOfPreviousGraph(int originalSuperNodeID, int currentSuperNodeID);
	// 	void tryToDeleteOneSuperNeighbor();
	// 	void tryToDeleteOneLeafNeighbor();
	// 	void tryToDeleteOneSuperVNeighbor();
	// 
	// 	void tryToAddOneSuperNeighbor();
	// 	void tryToAddOneLeafNeighbor();
	// 	void tryToAddOneSuperVNeighbor();

	void drawOneSuperNode(int graphNo, int nodeID,int dx, int dy, HSV hsv, bool isSelected = false);
	void drawOneLeafNode(int graphNo, int superNodeID, int leafNodeID, int dx, int dy, HSV hsv, bool isSelected = false);

	void drawGraphID(int graphNo, cv::Mat &img, 
		double scale, int curveStyle, int arg0 = 0);
	void drawSuperNodeID(int graphNo, cv::Mat &img, int superNodeID, 
		double scale, int curveStyle, int arg0 = 0);
	void drawLeafNodeID(int graphNo, cv::Mat &img, int superNodeID, 
		int leafNodeID, double scale, int curveStyle, int arg0 = 0);

	std::vector<int> getPreviousGraph(int graphNo);
	void drawPreviousGraph(cv::Mat &img, std::vector<int> &previousGraphs, int startV, int endV);
	void generatePreviousAndNextGraphArrays();

	void generateBg(int graphNo);
	void generateBgOfPreviousGraphs(int graphNo);
	void generateBgOfCurrentGraph(int graphNo);
	void generateBg();

	void drawAnimation();
	void initializeAnimation();

	bool isLeafNodeSelected();
	bool isSuperNodeSelected();
	bool isNodeSelected();
	bool isNodeNoSelection();

	void deleteEdge();
	void deleteVEdge();
	void setLineType(int num);

	/************************************************************************/
	/* interfaces                                                           */
	/************************************************************************/

	void initializeHSVH();
	void initializeSplines();
	void initializeTimer();
	void initializeVariables();
	void initializeImages();
	void initializeVNodeVariables();
	void initializeDrawing();
	void initialize();

	void startTimer();
	void stopTimer();

	void setGraphs(vGraph* gs, int n);
	void setScale(int ww, int hh);
	void setOrder(vector<vector<vector<NodeID>>> &order);
	void setImgProcess(imageProc* image_process);
	void reorder();
protected:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *me);
	void mouseMoveEvent(QMouseEvent *me);
	void mouseReleaseEvent ( QMouseEvent * me);
	void mouseDoubleClickEvent(QMouseEvent *me);
	void keyPressEvent(QKeyEvent *ke);

	public slots:
		void updateAnimation();
};

cv::Scalar HSV2RGB(int h, int s, int v);
cv::Scalar HSV2RGB(HSV hsv);

#endif // STELLAWIDGET_H
