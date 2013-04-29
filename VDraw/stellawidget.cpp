#include "stellawidget.h"
#include "ordering/vVirtualEdge.h"
#include <time.h>
#include <map>
/************************************************************************/
/* (B, G, R, A) means:							                        */
/* A: graph No.															*/
/*		B: the super node ID.											*/
/*		R=0 leaf node, G the leaf node ID.								*/
/*		R=1 super node.													*/
/*		R=2 blank area.													*/
/************************************************************************/

#define FPS				2
#define LINE_SEGMENT	4
//-------------const variable-----------------
#define LEAFNODE_RADIUS			6
#define SUPERNODE_RADIUS		10
#define LOD						3
#define VALID_EDGE_WIDTH		4
#define PREVIEW_EDGE_WIDTH		3
#define CURVE_TRANSIT_FRAMES	4
#define MAIN_GAP_WIDTH			5
//--------------------------------------------
#define GRAY_S			0
#define GRAY_V			70

#define ANIMATION_V		30

#define VERY_LIGHT_S	10
#define VERY_LIGHT_V	100

#define LIGHT_S			100//55
#define LIGHT_V			100

#define NORMAL_S		100
#define NORMAL_V		60//100

#define DARK_S			100
#define DARK_V			30//70//85

#define VERY_DARK_S		100
#define VERY_DARK_V		0//50//70

//-------------selection variables------------
#define WINDOW_BACKGROUND_COLOR	cv::Scalar(0xff, 0xff, 0xff, 0)
#define NO_SELECTION			100000
#define FREE_HSV_H				-1

#define LEAF_AREA				0
#define SUPER_AREA				1
#define BLANK_AREA				2

#define NODEID_CURVE_THICKNESS				3
#define IMAGE_UNSELECTED_CURVE_THICKNESS	2
#define IMAGE_SELECTED_CURVE_THICKNESS		3
#define SCALED_CURVE_THICKNESS				1

#define NODEID_CURVE			0
#define COLOR_IMAGE_CURVE		1
#define GRAY_IMAGE_CURVE		2
#define SCALED_IMAGE_CURVE		3
#define ANIMATION_IAMGE_CURVE	4

#define NO_PREVIOUS_GRAPH		-1

int fps = FPS;

//void alphaBlend(cv::Vec4b &bgColor, cv::Vec4b &fgColor, double alpha);
cv::Scalar HSV2RGB(HSV hsv);
cv::Scalar HSV2RGB(int hh, int ss, int vv);
void updateImage(cv::Mat &minDist, cv::Mat &nodeID, cv::Mat &image, int x, int y,  cv::Scalar &newid, cv::Scalar &newColor);
void drawVCurve(vCurve *curve, cv::Mat &img, double scale, int dx, int dy, cv::Scalar color, int thickness, int lineType = CV_AA);
bool deleteEdge(_Graph &g, NodeID nodeID1, NodeID nodeID2);
bool addEdge(_Graph &g, NodeID nodeID1, NodeID nodeID2);
void replaceElementsInVector(vector<vector<NodeID>> &v, int originalValue, int currentValue, int pos);
void copyElementsInVector(vector<vector<NodeID>> &v, int originalValue, int newone, int pos);
void removeRepeatedElementsInVector(vector<vector<NodeID>> &v);
cv::Scalar HSV2RGB(HSV hsv){
	return HSV2RGB(hsv.H, hsv.S, hsv.V);
}

cv::Scalar HSV2RGB(int hh, int ss, int vv){

	double h = hh;
	double s = (double)ss/100;
	double v = (double)vv/100;
	v *= 255;
	int i;
	double f, p, q, t;
	if( s == 0 ) {
		return cv::Scalar(v, v, v);
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
	case 0:
		return cv::Scalar(p, t, v);
	case 1:
		return cv::Scalar(q, v, p);
	case 2:
		return cv::Scalar(t, v, p);
	case 3:
		return cv::Scalar(t, q, p);
	case 4:
		return cv::Scalar(v, p, t);
	default:		// case 5:
		return cv::Scalar(q, p, v);
		break;
	}
}

void updateImage(cv::Mat &minDist, cv::Mat &nodeID, cv::Mat &image, int x, int y, cv::Scalar &newid, cv::Scalar &newColor){
	for(int i = 0; i< minDist.rows; i++){
		for(int j = 0; j< minDist.cols; j++){
			unsigned short &mind = minDist.at<unsigned short>(i, j);
			unsigned short newd = (x-i)*(x-i) + (y-j)*(y-j);
			cv::Vec4b &id = nodeID.at<cv::Vec4b>(i, j);
			//cv::Vec4b &color = image.at<cv::Vec4b>(i, j);

			if(mind > newd){
				//color = newColor;
				if(id[2] != LEAF_AREA){
					mind  = newd;
					id    = newid;
				}
			}
		}
	}
}

void drawVCurve(vCurve *curve, cv::Mat &img, double scale, int dx, int dy, cv::Scalar color, int thickness, int lineType){
	switch(curve->type){
	case curve->OPEN:
		{
			int n = curve->spline->curve2d.size();
			int npt[] = {n};
			cv::Point *plist = new cv::Point[n];
			const cv::Point *ppt[1] = {plist};
			for(int i =0; i < n; i++){
				plist[i] = cv::Point(curve->spline->curve2d[i].x()*scale + dx, 
					curve->spline->curve2d[i].y()*scale + dy);
			}
			cv::polylines(img, ppt, npt, 1, false, color, thickness);

			delete[] plist;
			break;
		}
	case curve->STRAIGHT:
		cv::Point start,end;
		start.x = curve->curve2d.front().x();
		start.y = curve->curve2d.front().y();
		end.x = curve->curve2d.back().x();
		end.y = curve->curve2d.back().y();

		int resolution = curve->Length()/3;

		// restore to pass the center
		cv::Point _cent(curve->center()[0], curve->center()[1]);
		cv::Point s(start.x,start.y);
		cv::Point e(end.x,end.y);
		cv::Point _tang = s-e;
		cv::Point _tangMiddle(_tang.x/2.0f, _tang.y/2.0f);
		s = _cent + _tangMiddle;
		e = _cent - _tangMiddle;
		cv::line(img, s * scale + cv::Point(dx, dy), e * scale + cv::Point(dx, dy),
			color, thickness);
		break;
	}
}

bool deleteEdge(_Graph &g, NodeID nodeID1, NodeID nodeID2){
	std::pair<_Graph::edge_descriptor, bool> &edge 
		= boost::edge(nodeID1, nodeID2, g);
	if(edge.second){
		g.remove_edge(edge.first);
	}
	return edge.second;
}
bool addEdge(_Graph &g, NodeID nodeID1, NodeID nodeID2){
	return true;
}

StellaWidget::StellaWidget(QWidget *parent)
	: QWidget(parent)
{
	setFocusPolicy(Qt::StrongFocus);
}

StellaWidget::~StellaWidget()
{

}

void StellaWidget::setLineType(int num){
	currentGraphNo=num;
	selectedSuperNodeID	= NO_SELECTION;
	selectedLeafNodeID	= NO_SELECTION;
	ms					= NO_EVENT;
	stopTimer();
	startTimer();
}

void StellaWidget::mouseDoubleClickEvent(QMouseEvent *me){
	using namespace std;
	std::cout<<"Press: "<<me->y()<<", "<<me->x()<<std::endl;
	cv::Vec4b info=nodeIDImage.at<cv::Vec4b>(me->y(), me->x());
	switch(me->button()){
	case Qt::LeftButton:
		cout<<"double click!"<<endl;
		break;
	}
}

void StellaWidget::keyPressEvent(QKeyEvent *ke){
	switch(ke->key()){
		// 	case Qt::Key_A:// try add neighbors
		// 		switch(ms){
		// 		case NO_EVENT:
		// 
		// 			break; // terminated here
		// 		case LEFT_MOVE:
		// 			if(isSuperNodeSelected()){
		// 				tryToAddOneSuperNeighbor();
		// 			}else if(isLeafNodeSelected()){
		// 				tryToAddOneLeafNeighbor();
		// 			}
		// 			break;
		// 		case RIGHT_MOVE:
		// 			if(isSuperNodeSelected()){
		// 				tryToAddOneSuperVNeighbor();
		// 			}else if(isLeafNodeSelected()){
		// 				// do nothing
		// 			}
		// 			break;
		// 		}
		// 		draw();
		// 		repaint();
		// 		break;
		// 	case Qt::Key_D:// try to delete neighbors
		// 		switch(ms){
		// 		case NO_EVENT:
		// 
		// 			break; // terminated here
		// 		case LEFT_MOVE:
		// 			if(isSuperNodeSelected()){
		// 				tryToDeleteOneSuperNeighbor();
		// 			}else if(isLeafNodeSelected()){
		// 				tryToDeleteOneLeafNeighbor();
		// 			}
		// 			break;
		// 		case RIGHT_MOVE:
		// 			if(isSuperNodeSelected()){
		// 				tryToDeleteOneSuperVNeighbor();
		// 			}else if(isLeafNodeSelected()){
		// 				// do nothing
		// 			}
		// 			break;
		// 		}
		// 		break;
	case Qt::Key_PageDown: // change to previous graph
		setLineType((currentGraphNo + 1) % graphsNum);
		drawNodeIDCurrentMode();
		draw();
		repaint();
		break;
	case Qt::Key_PageUp: // change to current graph
		setLineType((currentGraphNo + graphsNum - 1) % graphsNum);
		drawNodeIDCurrentMode();
		draw();
		repaint();
		break;
	case Qt::Key_Up:
		fps /= 2;
		if(fps<10)
			fps = 10;
		timer->setInterval(fps);
		break;
	case Qt::Key_Down:
		fps *= 2;
		if(fps>=1000)
			fps = 1000;
		timer->setInterval(fps);
		break;
	case Qt::Key_Enter:
		reorder();
		setLineType(currentGraphNo);
		break;
	}
}

void StellaWidget::mousePressEvent(QMouseEvent *me){
	using namespace std;
	cv::Vec4b info=nodeIDImage.at<cv::Vec4b>(me->y(), me->x());
	std::cout<<"Press: "<<me->y()<<", "<<me->x()<<": "
		<<(int)info[0]<<", "<<(int)info[1]<<", "<<(int)info[2]<<", "<<(int)info[3]<<std::endl;
	int selectedGraphNo	= info[3];
	int areaProperty	= info[2];
	int superNodeID		= info[0];
	int leafNodeID		= info[1];
	//std::cout<<"Info:  "<<(int)info[0]<<" "<<(int)info[1]<<" "<<(int)info[2]<<" "<<(int)info[3]<<std::endl;
	//std::cout<<"size: "<<nodeID.rows<<""<<nodeID.cols<<std::endl;
	//std::cout<<"NodeID: "<<selectedSuperNodeID<<" "<<selectedLeafNodeID<<std::endl;
	switch(areaProperty){
	case LEAF_AREA:
		selectedSuperNodeID = superNodeID;
		selectedLeafNodeID  = leafNodeID;
		stopTimer();
		break;
	case SUPER_AREA:
		selectedSuperNodeID = superNodeID;
		selectedLeafNodeID  = NO_SELECTION;
		stopTimer();
		break;
	case BLANK_AREA:
		selectedSuperNodeID = NO_SELECTION;
		selectedLeafNodeID  = NO_SELECTION;
		startTimer();
		break;
	}
	switch(me->button()){
	case Qt::LeftButton:
		switch(areaProperty){
		case LEAF_AREA:
		case SUPER_AREA:
			ms = LEFT_DOWN;
			break;
		case BLANK_AREA:
			ms = NO_EVENT;
			break;
		}
		break;
	case Qt::RightButton:
		switch(areaProperty){
		case LEAF_AREA:
		case SUPER_AREA:
			ms = RIGHT_DOWN;
			break;
		case BLANK_AREA:
			ms = NO_EVENT;
			break;
		}
		break;
	case Qt::MidButton:
		ms=MIDDLE_DOWN;
		break;
	}
	downPos = cv::Vec2i(me->x(), me->y());
	draw();
	drawNodeIDCurrentMode();
	this->repaint();
	cout<<"Press end."<<endl;
}

void StellaWidget::mouseMoveEvent(QMouseEvent *me){
	using namespace std;
	//cout<<"Move:  "<<"pos : "<<me->y()<<", "<<me->x()<<std::endl;
	cv::Vec4b info=nodeIDImage.at<cv::Vec4b>(me->y(), me->x());
	deltaPos = cv::Vec2i(me->x() - downPos[0], me->y() - downPos[1]);
	switch(ms){
	case NO_EVENT:

		return; // terminated here
	case LEFT_DOWN:
		if(isSuperNodeSelected()){
			calculateSuperNeighbors();
		}else if(isLeafNodeSelected()){
			calculateLeafNeighbors();
		}
		drawNodeIDCurrentMode();
		ms = LEFT_MOVE;
	case LEFT_MOVE:

		break;
	case RIGHT_DOWN:
		if(isSuperNodeSelected()){
			calculateVirtualNeighbors();
			drawNodeIDPreviousMode();
		}else if(isLeafNodeSelected()){
			calculateSuperNeighbors();
			drawNodeIDCurrentMode();
		}
		ms = RIGHT_MOVE;
	case RIGHT_MOVE:

		break;
	}
	draw();
	repaint();
}

void StellaWidget::mouseReleaseEvent(QMouseEvent *me){
	using namespace std;
	//cout<<"Release: "<<me->y()<<", "<<me->x()<<std::endl;
	cv::Vec4b info=nodeIDImage.at<cv::Vec4b>(me->y(), me->x());
	//std::cout<<"Info: "<<(int)info[0]<<" "<<(int)info[1]<<" "<<(int)info[2]<<" "<<(int)info[3]<<std::endl;
	switch(ms){
	case NO_EVENT:

		break; // terminated here
	case LEFT_MOVE:
		if(isSuperNodeSelected()){
			processSuperNodeNeighbor();
		}else if(isLeafNodeSelected()){
			processLeafNodeNeighbor();
		}
		break;
	case RIGHT_MOVE:
		if(isSuperNodeSelected()){
			processSuperNodeVNeighbor();
		}else if(isLeafNodeSelected()){
			processLeafNodeMovingOrSuperNodeCreation();
		}
		break;
	}
	ms = NO_EVENT;
	draw();
	drawNodeIDCurrentMode();
	repaint();
	cout<<"Release end."<<endl;
}

void StellaWidget::paintEvent(QPaintEvent *e){
	QPainter pt(this);
	QImage qImg = QImage(displayImage.data, displayImage.cols, displayImage.rows, QImage::Format_RGB32);
	pt.drawImage(0, 0, qImg);
}

/************************************************************************/
/* Generate the global image, which shows in the main window.           */
/************************************************************************/
void StellaWidget::draw(){
	switch(ms){
	case NO_EVENT:
	case RIGHT_DOWN:
	case LEFT_DOWN:
	case MIDDLE_DOWN:
		if(isSuperNodeSelected()){
			drawSilentSuperMode();
		}else if(isLeafNodeSelected()){
			drawSilentLeafMode();
		}
		break;
	case LEFT_MOVE:
		if(isSuperNodeSelected()){
			drawSuperEdgeEditingMode();
		}else if (isLeafNodeSelected()){
			drawLeafEdgeEditingMode();
		}
		break;
	case RIGHT_MOVE:
		if(isSuperNodeSelected()){
			drawVEdgeEditingMode();
		}else if (isLeafNodeSelected()){
			drawLeafNodeMovingEditingMode();
		}
		break;
	}
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(200, 100), cv::Scalar(255, 0, 0), 1, 4);
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(300, 100), cv::Scalar(255, 0, 0), 2, 4);
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(400, 100), cv::Scalar(255, 0, 0), 3, 4);
	// 
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(200, 300), cv::Scalar(0, 255, 0), 1, 8);
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(300, 300), cv::Scalar(0, 255, 0), 2, 8);
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(400, 300), cv::Scalar(0, 255, 0), 3, 8);
	// 
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(200, 500), cv::Scalar(0, 0, 255), 1, CV_AA);
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(300, 500), cv::Scalar(0, 0, 255), 2, CV_AA);
	// 	cv::line(displayImage, cv::Point(0, 0), cv::Point(400, 500), cv::Scalar(0, 0, 255), 3, CV_AA);
}

void StellaWidget::drawVEdgeEditingMode(){
	int previousGraphNo = previousGraph[currentGraphNo];
	drawBackgroundLayer(previousGraphNo);

	int h = getSuperNodeHSV_H(currentGraphNo, selectedSuperNodeID);

	// draw neighbors
	drawSuperNodeVNeighbors(HSV(FREE_HSV_H, NORMAL_S, NORMAL_V));

	// draw passed one
	int passedNodeID = getPassedSuperNodeID();
	drawOneSuperNode(previousGraphNo, passedNodeID,
		0, 0, HSV(FREE_HSV_H, NORMAL_S, NORMAL_V), 
		std::find(neighborNodeID.begin(), neighborNodeID.end(), passedNodeID)
		== neighborNodeID.end());

	// draw selected one
	drawOneSuperNode(currentGraphNo, selectedSuperNodeID, 
		deltaPos[0], deltaPos[1], HSV(h, NORMAL_S, NORMAL_V), true);
}

void StellaWidget::drawSilentSuperMode(){
	drawBackgroundLayer(currentGraphNo);

	drawOneSuperNode(currentGraphNo, selectedSuperNodeID, 
		0, 0, HSV(FREE_HSV_H, NORMAL_S, NORMAL_V), true);
}

void StellaWidget::drawSuperEdgeEditingMode(){
	drawBackgroundLayer(currentGraphNo);

	int h = getSuperNodeHSV_H(currentGraphNo, selectedSuperNodeID);

	// draw neighbors
	drawSuperNodeNeighbors(HSV(FREE_HSV_H, NORMAL_S, NORMAL_V));

	// draw passed one
	int passedNodeID = getPassedSuperNodeID();
	bool notFound = std::find(neighborNodeID.begin(), neighborNodeID.end(), passedNodeID)
		== neighborNodeID.end();
	if(!notFound){

	}
	drawOneSuperNode(currentGraphNo, passedNodeID,
		0, 0, HSV(FREE_HSV_H, NORMAL_S, NORMAL_V), notFound);

	// draw selected one
	drawOneSuperNode(currentGraphNo, selectedSuperNodeID, 
		deltaPos[0], deltaPos[1], HSV(h, NORMAL_S, NORMAL_V), true);
}

void StellaWidget::drawSilentLeafMode(){
	drawBackgroundLayer(currentGraphNo);

	// draw selected super node
	drawOneSuperNode(currentGraphNo, selectedSuperNodeID, 
		0, 0, HSV(FREE_HSV_H, NORMAL_S, NORMAL_V));

	// draw itself
	drawOneLeafNode(currentGraphNo, selectedSuperNodeID, selectedLeafNodeID,
		0, 0, HSV(FREE_HSV_H, DARK_S, DARK_V), true);
}

void StellaWidget::drawLeafEdgeEditingMode(){
	drawBackgroundLayer(currentGraphNo);

	// draw selected super node
	drawOneSuperNode(currentGraphNo, selectedSuperNodeID, 
		0, 0, HSV(FREE_HSV_H, NORMAL_S, NORMAL_V));

	// draw neighbors
	drawLeafNodeNeighbors(HSV(FREE_HSV_H, DARK_S, DARK_V));

	// draw passed one
	int passedLeafNodeID = getPassedLeafNodeID(selectedSuperNodeID);
	bool notFound = std::find(neighborNodeID.begin(), neighborNodeID.end(), passedLeafNodeID)
		== neighborNodeID.end();
	drawOneLeafNode(currentGraphNo, selectedSuperNodeID, 
		passedLeafNodeID, 0, 0, HSV(FREE_HSV_H, DARK_S, DARK_V), notFound);
	// draw itself
	drawOneLeafNode(currentGraphNo, selectedSuperNodeID, selectedLeafNodeID, 
		deltaPos[0], deltaPos[1], HSV(FREE_HSV_H, DARK_S, DARK_V), true);
}

void StellaWidget::drawLeafNodeMovingEditingMode(){
	drawBackgroundLayer(currentGraphNo);
	// draw selected super node
	drawOneSuperNode(currentGraphNo, selectedSuperNodeID, 
		0, 0, HSV(FREE_HSV_H, NORMAL_S, NORMAL_V));
	// draw neighbors

	drawSuperNodeNeighbors(HSV(FREE_HSV_H, NORMAL_S, NORMAL_V));
	// draw itself
	int passedSuperNodeID = getPassedSuperNodeID();
	if(isSuperNodesLinked(currentGraphNo, selectedSuperNodeID, passedSuperNodeID)){
		int passedSuperNodeIDH = getSuperNodeHSV_H(currentGraphNo, passedSuperNodeID);
		drawOneSuperNode(currentGraphNo, passedSuperNodeID, 0, 0, 
			HSV(FREE_HSV_H, NORMAL_S, NORMAL_V), true);
		drawOneLeafNode(currentGraphNo, selectedSuperNodeID, selectedLeafNodeID,
			deltaPos[0], deltaPos[1], HSV(passedSuperNodeIDH, NORMAL_S, NORMAL_V), true);
	}else{
		drawOneLeafNode(currentGraphNo, selectedSuperNodeID, selectedLeafNodeID,
			deltaPos[0], deltaPos[1], HSV(FREE_HSV_H, DARK_S, DARK_V), true);
	}
}

/************************************************************************/
/* draw Node ID in current mode                                         */
/************************************************************************/
void StellaWidget::drawNodeIDCurrentMode(){
	graphBGIDImage[currentGraphNo].copyTo(nodeIDImage);
}

/************************************************************************/
/* draw Node ID in previous mode                                        */
/************************************************************************/
void StellaWidget::drawNodeIDPreviousMode(){
	graphBGIDImage[previousGraph[currentGraphNo]].copyTo(nodeIDImage);
}

void StellaWidget::drawOneSuperNode(int graphNo, int nodeID,int dx, int dy, 
	HSV hsv, bool isSelected){
		if(nodeID == NO_SELECTION)
			return;
		vNode *superNode = graphs[graphNo]->g[nodeID];
		_Graph &g = superNode->superNode->g;
		_Graph::vertex_iterator vi,ve;
		boost::tie(vi, ve) = boost::vertices(g);
		if(hsv.H == FREE_HSV_H){
			hsv.H = superNode->getH();
		}
		for(; vi != ve; vi++){
			drawOneLeafNode(graphNo, nodeID, *vi, dx, dy, hsv, isSelected);
		}
}

void StellaWidget::drawOneLeafNode(int graphNo, int superNodeID, int leafNodeID, 
	int dx, int dy, HSV hsv, bool isSelected){
		if(superNodeID == NO_SELECTION || leafNodeID == NO_SELECTION)
			return;

		vNode *node = graphs[graphNo]->g[superNodeID]->superNode->g[leafNodeID];
		vCurve *curve = node->getCurve();
		int thickness = (isSelected)? IMAGE_SELECTED_CURVE_THICKNESS : IMAGE_UNSELECTED_CURVE_THICKNESS;
		if(hsv.H == FREE_HSV_H){
			drawVCurve(curve, displayImage, scale, dx, dy, 
				HSV2RGB(node->getH(), hsv.S, hsv.V), thickness);
		}else{
			drawVCurve(curve, displayImage, scale, dx, dy, 
				HSV2RGB(hsv), thickness);
		}
}

bool StellaWidget::isSuperNodesLinked(int graphNo, int nodeID1, int nodeID2){
	return nodeID1 != NO_SELECTION && nodeID2 != NO_SELECTION 
		&& boost::edge(nodeID1, nodeID2, graphs[graphNo]->g).second;
}

bool StellaWidget::isLeafNodesLinked(int graphNo, int superNodeID,
	int leafNodeID1, int leafNodeID2){
		return superNodeID != NO_SELECTION &&leafNodeID1 != NO_SELECTION 
			&& leafNodeID1 != NO_SELECTION && boost::edge(leafNodeID1, leafNodeID2, 
			graphs[graphNo]->g[superNodeID]->superNode->g).second;
}

int StellaWidget::getSuperNodeHSV_H(int graphNo, int nodeID){
	assert(nodeID != NO_SELECTION);
	if(nodeID == NO_SELECTION)
		return -1;
	return graphs[graphNo]->g[nodeID]->getH();
}

int StellaWidget::getLeafNodeHSV_H(int graphNo, int superNodeID, int leafNodeID){
	if(leafNodeID != NO_SELECTION)
		return graphs[graphNo]->g[superNodeID]->superNode->g[leafNodeID]->getH();
	else
		return -1;
}

int StellaWidget::getPassedSuperNodeID(){
	cv::Vec4b info=nodeIDImage.at<cv::Vec4b>(downPos[1] + deltaPos[1], downPos[0] + deltaPos[0]);
	int areaProperty	= info[2];
	int superNodeID		= info[0];
	int graphNo			= info[3];
	switch(areaProperty){
	case SUPER_AREA:
	case LEAF_AREA:
		if(currentGraphNo == graphNo && selectedSuperNodeID == superNodeID)
			return NO_SELECTION;
		else
			return superNodeID;
	case BLANK_AREA:
		return NO_SELECTION;
	}
	return NO_SELECTION;
}

int StellaWidget::getPassedLeafNodeID(int superNodeID){
	cv::Vec4b info=nodeIDImage.at<cv::Vec4b>(downPos[1] + deltaPos[1], downPos[0] + deltaPos[0]);
	int areaProperty		= info[2];
	int passedSuperNodeID	= info[0];
	int leafNodeID			= info[1];

	switch(areaProperty){
	case SUPER_AREA:
		return NO_SELECTION;
	case LEAF_AREA:
		if(superNodeID != passedSuperNodeID || leafNodeID == selectedLeafNodeID)
			return NO_SELECTION;
		else
			return leafNodeID;
	case BLANK_AREA:
		return NO_SELECTION;
	}
	return NO_SELECTION;
}

void StellaWidget::drawBackgroundLayer(int graphNo){
	graphBGImage[graphNo].copyTo(displayImage);
}

void StellaWidget::drawSuperNodeNeighbors(HSV hsv){
	for(int i = 0; i<neighborNodeID.size(); i++){
		drawOneSuperNode(currentGraphNo, neighborNodeID[i], 0, 0, hsv, 
			getPassedSuperNodeID() != neighborNodeID[i]);
	}
}

void StellaWidget::drawLeafNodeNeighbors(HSV hsv){
	for(int i = 0; i<neighborNodeID.size(); i++){
		drawOneLeafNode(currentGraphNo, selectedSuperNodeID, neighborNodeID[i], 
			0, 0, hsv, getPassedLeafNodeID(selectedSuperNodeID) != neighborNodeID[i]);
	}
}

void StellaWidget::drawSuperNodeVNeighbors(HSV hsv){
	for(int i = 0; i<neighborNodeID.size(); i++){
		drawOneSuperNode(previousGraph[currentGraphNo], neighborNodeID[i], 0, 0, hsv, 
			getPassedSuperNodeID() != neighborNodeID[i]);
	}
}

void StellaWidget::calculateSuperNeighbors(){
	if(!neighborNodeID.empty())
		neighborNodeID.clear();
	_Graph &g = graphs[currentGraphNo]->g;
	_Graph::adjacency_iterator ai, ae;
	boost::tie(ai, ae) = boost::adjacent_vertices(selectedSuperNodeID, g);
	for(; ai != ae; ai++){
		neighborNodeID.push_back(*ai);
	}
}

void StellaWidget::calculateLeafNeighbors(){
	if(!neighborNodeID.empty())
		neighborNodeID.clear();
	_Graph &g = graphs[currentGraphNo]->g[selectedSuperNodeID]->superNode->g;
	_Graph::adjacency_iterator ai, ae;
	boost::tie(ai, ae) = boost::adjacent_vertices(selectedLeafNodeID, g);
	for(; ai != ae; ai++){
		neighborNodeID.push_back(*ai);
	}
}

void StellaWidget::calculateVirtualNeighbors(){
	if(!neighborNodeID.empty())
		neighborNodeID.clear();
	vector<vector<NodeID>> map2prevList = graphs[currentGraphNo]->virtualEdges->map2prev;
	for(int i=0; i < map2prevList.size(); i++){
		if(map2prevList[i][0] == selectedSuperNodeID)
			neighborNodeID.push_back(map2prevList[i][1]);
	}
}

void StellaWidget::processLeafNodeNeighbor(){
	int leafNodeID = getPassedLeafNodeID(selectedSuperNodeID);
	if(leafNodeID == NO_SELECTION)
		return;
	_Graph &g = graphs[currentGraphNo]->g[selectedSuperNodeID]->superNode->g;
	if(isLeafNodesLinked(currentGraphNo, selectedSuperNodeID, selectedLeafNodeID, leafNodeID))
		boost::remove_edge(selectedLeafNodeID, leafNodeID, g);
	else
		boost::add_edge(selectedLeafNodeID, leafNodeID, g);
}

void StellaWidget::processSuperNodeNeighbor(){
	int superNodeID = getPassedSuperNodeID();
	if(superNodeID == NO_SELECTION)
		return;
	_Graph &g = graphs[currentGraphNo]->g;
	if(isSuperNodesLinked(currentGraphNo, selectedSuperNodeID, superNodeID)){
		boost::remove_edge(selectedSuperNodeID, superNodeID, g);
	}else{
		boost::add_edge(selectedSuperNodeID, superNodeID, g);
	}
}

void StellaWidget::processSuperNodeVNeighbor(){
	int superNodeID = getPassedSuperNodeID();
	if(superNodeID == NO_SELECTION)
		return;
	int previousGraphNo = previousGraph[currentGraphNo];
	vVirtualEdge *currentVVE  = graphs[currentGraphNo]->virtualEdges;
	vVirtualEdge *previousVVE = graphs[previousGraphNo]->virtualEdges;
	std::vector<std::vector<NodeID>> &currentMap2prev  = currentVVE->map2prev;
	std::vector<std::vector<NodeID>> &previousMap2next = previousVVE->map2next;

	bool found = false;
	for(int i = 0; i < currentMap2prev.size(); i++){
		if(currentMap2prev[i][0] == selectedSuperNodeID 
			&& currentMap2prev[i][1] == superNodeID){
				currentMap2prev.erase(i + currentMap2prev.begin());
				for(int j = 0; j < previousMap2next.size(); j++){
					if(previousMap2next[j][0] == superNodeID
						&& previousMap2next[j][1] == selectedSuperNodeID){
							previousMap2next.erase(j + previousMap2next.begin());
							found = true;
							break;
					}
				}
				assert(found);
				break;
		}
	}
	if(!found){
		vector<NodeID> currentNewVEdge, previousNewVEdge;
		currentNewVEdge.push_back(selectedSuperNodeID);
		currentNewVEdge.push_back(superNodeID);
		currentMap2prev.push_back(currentNewVEdge);

		previousNewVEdge.push_back(superNodeID);
		previousNewVEdge.push_back(selectedSuperNodeID);
		previousMap2next.push_back(previousNewVEdge);
	}
}

void StellaWidget::replaceLeafNodeID(int originalSuperNodeID, int originalLeafNodeID, 
	int currentSuperNodeID, int currentLeafNodeID){
		for(int i = 0; i < graphBGIDImage[currentGraphNo].rows; i++){
			for(int j = 0; j < graphBGIDImage[currentGraphNo].cols; j++){
				cv::Vec4b &id = graphBGIDImage[currentGraphNo].at<cv::Vec4b>(i, j);
				if(id[2] != BLANK_AREA){
					if(id[0] == originalSuperNodeID && id[1] == originalLeafNodeID){
						id[0] = currentSuperNodeID;
						id[1] = currentLeafNodeID;
					}
				}
			}
		}
		for(int i = 0; i< order[currentGraphNo].size(); i++){
			if(order[currentGraphNo][i][0] == originalSuperNodeID
				&& order[currentGraphNo][i][1] == originalLeafNodeID){
					order[currentGraphNo][i][0] = currentSuperNodeID;
					order[currentGraphNo][i][1] = currentLeafNodeID;
			}
		}
}

void StellaWidget::replaceSuperNodeID(int originalSuperNodeID, int currentSuperNodeID){
	for(int i = 0; i < graphBGIDImage[currentGraphNo].rows; i++){
		for(int j = 0; j < graphBGIDImage[currentGraphNo].cols; j++){
			cv::Vec4b &id = graphBGIDImage[currentGraphNo].at<cv::Vec4b>(i, j);
			if(id[2] != BLANK_AREA){
				if(id[0] == originalSuperNodeID){
					id[0] = currentSuperNodeID;
				}
			}
		}
	}
	for(int i = 0; i< order[currentGraphNo].size(); i++){
		if(order[currentGraphNo][i][0] == originalSuperNodeID){
			order[currentGraphNo][i][0] = currentSuperNodeID;
		}
	}
	
}

void StellaWidget::copyVirtualEdgeToNewNode(int parent, int newone){
	vVirtualEdge *currentVVE = graphs[currentGraphNo]->virtualEdges;
	copyElementsInVector(currentVVE->map2prev, parent, newone, 0);
	copyElementsInVector(currentVVE->map2next, parent, newone, 0);
	if(previousGraph[currentGraphNo] != NO_PREVIOUS_GRAPH)
		copyElementsInVector(graphs[previousGraph[currentGraphNo]]->virtualEdges->map2next, parent, newone, 1);
	if(nextGraph[currentGraphNo] != NO_PREVIOUS_GRAPH)
		copyElementsInVector(graphs[nextGraph[currentGraphNo]]->virtualEdges->map2prev, parent, newone, 1);
}

void StellaWidget::combineVirtualEdgeToNewNode(int parent, int deletedone){
	replaceNodeIDInVertualEdge(deletedone, parent);
	vVirtualEdge *currentVVE = graphs[currentGraphNo]->virtualEdges;
	removeRepeatedElementsInVector(currentVVE->map2prev);
	removeRepeatedElementsInVector(currentVVE->map2next);

	if(previousGraph[currentGraphNo] != NO_PREVIOUS_GRAPH){
		removeRepeatedElementsInVector(graphs[previousGraph[currentGraphNo]]->virtualEdges->map2next);
	}
	
	if(nextGraph[currentGraphNo] != NO_PREVIOUS_GRAPH){
		removeRepeatedElementsInVector(graphs[nextGraph[currentGraphNo]]->virtualEdges->map2prev);
	}
}

void StellaWidget::replaceNodeIDInVertualEdge(int originalNodeID, int currentNodeID){
	vVirtualEdge *currentVVE = graphs[currentGraphNo]->virtualEdges;
	replaceElementsInVector(currentVVE->map2prev, originalNodeID, currentNodeID, 0);
	replaceElementsInVector(currentVVE->map2next, originalNodeID, currentNodeID, 0);
	
	if(previousGraph[currentGraphNo] != NO_PREVIOUS_GRAPH){
		replaceElementsInVector(graphs[previousGraph[currentGraphNo]]->virtualEdges->map2next,
			originalNodeID, currentNodeID, 1);
	}

	if(nextGraph[currentGraphNo] != NO_PREVIOUS_GRAPH){
		replaceElementsInVector(graphs[nextGraph[currentGraphNo]]->virtualEdges->map2prev,
			originalNodeID, currentNodeID, 1);
	}
}

void replaceElementsInVector(vector<vector<NodeID>> &v, int originalValue, int currentValue, int pos){
	for(int i = 0; i<v.size(); i++){
		if(v[i][pos] == originalValue){
			v[i][pos] = currentValue;
		}
	}
}

void copyElementsInVector(vector<vector<NodeID>> &v, int originalValue, int newone, int pos){
	int sz=v.size();
	for(int i = 0; i<sz; i++){
		if(v[i][pos] == originalValue){
			vector<NodeID> nv;
			nv.push_back(newone);
			if(pos == 0)
				nv.push_back(v[i][1]);
			else
				nv.insert(nv.begin(), v[i][0]);
			v.push_back(nv);
		}
	}
}

void removeRepeatedElementsInVector(vector<vector<NodeID>> &v){
	for(int i = v.size()-1; i>0; i--){
		for(int j = 0; j<i; j++){
			if(v[i][0] == v[j][0] && v[i][1] == v[j][1]){
				v.erase(v.begin()+i);
				break;
			}
		}
	}
}

// void StellaWidget::changeCurrentVVEOfCurrentGraph(int originalSuperNodeID, int currentSuperNodeID){
// 	vVirtualEdge *vve = graphs[currentGraphNo]->virtualEdges;
// 	if(vve){
// 		replaceElementsInVector(vve->map2next, originalSuperNodeID, currentSuperNodeID, 0);
// 		replaceElementsInVector(vve->map2prev, originalSuperNodeID, currentSuperNodeID, 0);
// 	}
// }
// 
// void StellaWidget::changeThePreviousVVEOfNextGraph(int originalSuperNodeID, int currentSuperNodeID){
// 	vVirtualEdge *vve = graphs[nextGraph[currentGraphNo]]->virtualEdges;
// 	if(vve){
// 		replaceElementsInVector(vve->map2prev, originalSuperNodeID, currentSuperNodeID, 1);
// 	}
// }
// 
// void StellaWidget::changeTheNextVVEOfPreviousGraph(int originalSuperNodeID, int currentSuperNodeID){
// 	vVirtualEdge *vve = graphs[previousGraph[currentGraphNo]]->virtualEdges;
// 	if(vve){
// 		replaceElementsInVector(vve->map2next, originalSuperNodeID, currentSuperNodeID, 1);
// 	}
// }

void StellaWidget::createOriginalVNode2NodeIDMap(_Graph &g, std::map<vNode*, NodeID> &vn2nid){
	_Graph::vertex_iterator vi, ve;
	boost::tie(vi, ve) = boost::vertices(g);
	for(; vi != ve; vi++){
		vn2nid.insert(std::pair<vNode*, NodeID>(g[*vi], *vi));
	}
}

void StellaWidget::processLeafNodeMovingOrSuperNodeCreation(){
	int superNodeID = getPassedSuperNodeID();
	int leafNumberOfCurrentSuperNode = 
		graphs[currentGraphNo]->g[selectedSuperNodeID]->
		superNode->g.vertex_set().size();

	if(superNodeID == NO_SELECTION){
		createSuperNode();
		//createSuperNode(graphs[selectedSuperNodeID], selectedLeafNodeID, selectedLeafNodeID);
	}else if(leafNumberOfCurrentSuperNode == 1){
		combineSuperNode(superNodeID);
	}else if(isSuperNodesLinked(currentGraphNo, selectedSuperNodeID, superNodeID)){
		// move leaf node
		moveLeafNode(superNodeID);
	}
}

void StellaWidget::createSuperNode(){
	// remove selected leaf vertex from selected super vertex
	vNode *originalFirstSuperNode = graphs[currentGraphNo]->g[selectedSuperNodeID];
	_Graph &firstSuperNodeVGraph = originalFirstSuperNode->superNode->g;
	vNode *theLeafNode = firstSuperNodeVGraph[selectedLeafNodeID];

	// generate the original map
	std::map<vNode*, NodeID> originalGraphMap;
	createOriginalVNode2NodeIDMap(graphs[currentGraphNo]->g, originalGraphMap);

	// generate the original map
	std::map<vNode*, NodeID> firstMap;
	createOriginalVNode2NodeIDMap(firstSuperNodeVGraph, firstMap);

	// remove the selected leaf
	boost::clear_vertex(selectedLeafNodeID, firstSuperNodeVGraph);
	boost::remove_vertex(selectedLeafNodeID, firstSuperNodeVGraph);

	// create new super node
	vNode *newSecondSuperNode = new vNode();
	newSecondSuperNode->isLeaf = false;
	newSecondSuperNode->superNode = new vGraph();
	srand((unsigned)time(NULL));
	newSecondSuperNode->setH(rand() % 360);
	std::cout<<"new super node h: "<<newSecondSuperNode->getH()<<std::endl;

	// add original leaf node to the new super node
	NodeID newLeafID = newSecondSuperNode->superNode->addNode(theLeafNode);// ----add!!


	// add new super node to the vgraph
	NodeID newSuperNodeID = boost::add_vertex(newSecondSuperNode, graphs[currentGraphNo]->g);
	replaceLeafNodeID(selectedSuperNodeID, selectedLeafNodeID,
		newSuperNodeID, newLeafID);

	// replace other leaves
	_Graph::vertex_iterator vi, ve;
	boost::tie(vi, ve) = boost::vertices(firstSuperNodeVGraph);
	for(; vi != ve; vi++){
		replaceLeafNodeID(selectedSuperNodeID, 
			firstMap[firstSuperNodeVGraph[*vi]],
			selectedSuperNodeID, *vi);
	}

	boost::tie(vi, ve) = boost::vertices(graphs[currentGraphNo]->g);
	NodeID originalSuperNodeNewID;
	for(; vi != ve; vi++){
		vNode *node = graphs[currentGraphNo]->g[*vi];
		if(node == newSecondSuperNode){
			continue;
		}else if(node == originalFirstSuperNode){
			originalSuperNodeNewID = *vi;
		}
		NodeID originalNodeID = originalGraphMap[node];
		replaceSuperNodeID(originalNodeID, *vi);
		replaceNodeIDInVertualEdge(originalNodeID, *vi);
	}
	copyVirtualEdgeToNewNode(originalSuperNodeNewID, newSuperNodeID);
	boost::add_edge(newSuperNodeID, originalSuperNodeNewID, graphs[currentGraphNo]->g);
	linkTheNeighborsOfNodeToNewNode(graphs[currentGraphNo]->g, 
		originalSuperNodeNewID, newSuperNodeID);
	drawGraphID(currentGraphNo, graphBGImage[currentGraphNo], scale, COLOR_IMAGE_CURVE);
	selectedSuperNodeID = newSuperNodeID;
	selectedLeafNodeID  = newLeafID;// remove selected leaf vertex from selected super vertex
}

void StellaWidget::linkTheNeighborsOfNodeToNewNode(_Graph &g, NodeID nodeID, NodeID newNode){
	_Graph::adjacency_iterator ai, ae;
	boost::tie(ai, ae) = boost::adjacent_vertices(nodeID, g);
	for(; ai != ae; ai++){
		boost::add_edge(newNode, *ai, g);
	}
}

void StellaWidget::combineSuperNode(NodeID superNodeID){
	// remove selected leaf vertex from selected super vertex
	vNode *originalSuperNode = graphs[currentGraphNo]->g[selectedSuperNodeID];
	_Graph &currentSuperNodeVGraph = originalSuperNode->superNode->g;
	vNode *originalLeafNode = currentSuperNodeVGraph[selectedLeafNodeID];

	// remove the selected leaf
	boost::clear_vertex(selectedLeafNodeID, currentSuperNodeVGraph);
	boost::remove_vertex(selectedLeafNodeID, currentSuperNodeVGraph);

	// generate the original map
	// get the passed super node
	vNode *newSuperNode = graphs[currentGraphNo]->g[superNodeID];
	std::map<vNode*, NodeID> originalSecondSuperGraphMap;
	_Graph &secondSuperVGraph = newSuperNode->superNode->g;
	createOriginalVNode2NodeIDMap(secondSuperVGraph, originalSecondSuperGraphMap);


	// add original leaf node to the passed super node
	NodeID newLeafID = newSuperNode->superNode->addNode(originalLeafNode);

	// replace the leaves of the passed one
	_Graph::vertex_iterator vi, ve;
	boost::tie(vi, ve) = boost::vertices(secondSuperVGraph);
	for(; vi != ve; vi++){
		if(secondSuperVGraph[*vi] == originalLeafNode){
			continue;
		}
		replaceLeafNodeID(superNodeID, 
			originalSecondSuperGraphMap[secondSuperVGraph[*vi]],
			superNodeID, *vi);
	}

	// update the original leaf node to the passed vgraph
	replaceLeafNodeID(selectedSuperNodeID, selectedLeafNodeID,
		superNodeID, newLeafID);

	// generate the original map
	std::map<vNode*, NodeID> originalGraphMap;
	createOriginalVNode2NodeIDMap(graphs[currentGraphNo]->g, originalGraphMap);

	linkTheNeighborsOfNodeToNewNode(graphs[currentGraphNo]->g, 
		selectedSuperNodeID, superNodeID);
	boost::clear_vertex(selectedSuperNodeID, graphs[currentGraphNo]->g);
	boost::remove_vertex(selectedSuperNodeID, graphs[currentGraphNo]->g);
	boost::tie(vi, ve) = boost::vertices(graphs[currentGraphNo]->g);
	combineVirtualEdgeToNewNode(superNodeID, selectedSuperNodeID);
	for(; vi != ve; vi++){
		vNode *node = graphs[currentGraphNo]->g[*vi];
		if(node == newSuperNode)
			superNodeID = *vi;
		NodeID originalNodeID = originalGraphMap[node];
		replaceSuperNodeID(originalNodeID, *vi);
		replaceNodeIDInVertualEdge(originalNodeID, *vi);
	}
	
	drawGraphID(currentGraphNo, graphBGImage[currentGraphNo], scale, COLOR_IMAGE_CURVE);
	selectedSuperNodeID = superNodeID;
	selectedLeafNodeID  = newLeafID;// remove selected leaf vertex from selected super vertex
	//calculateSuperNeighbors();
}

void StellaWidget::moveLeafNode(NodeID superNodeID){
	// remove selected leaf vertex from selected super vertex
	vNode *originalSuperNode = graphs[currentGraphNo]->g[selectedSuperNodeID];
	_Graph &firstSuperNodeVGraph = originalSuperNode->superNode->g;
	vNode *originalLeafNode = firstSuperNodeVGraph[selectedLeafNodeID];

	vNode *newSuperNode = graphs[currentGraphNo]->g[superNodeID];
	_Graph &secondSuperVGraph = newSuperNode->superNode->g;

	std::map<vNode*, NodeID> originalSecondSuperGraphMap;
	createOriginalVNode2NodeIDMap(secondSuperVGraph, originalSecondSuperGraphMap);


	// generate the original map
	std::map<vNode*, NodeID> originalSuperGraphMap;
	createOriginalVNode2NodeIDMap(firstSuperNodeVGraph, originalSuperGraphMap);

	// add original leaf node to the passed super node
	NodeID newLeafID = newSuperNode->superNode->addNode(originalLeafNode);

	// replace the leaves of the passed one
	_Graph::vertex_iterator vi, ve;
	boost::tie(vi, ve) = boost::vertices(secondSuperVGraph);
	for(; vi != ve; vi++){
		if(secondSuperVGraph[*vi] == originalLeafNode){
			continue;
		}
		assert(originalSecondSuperGraphMap.find(secondSuperVGraph[*vi]) != originalSecondSuperGraphMap.end());
		replaceLeafNodeID(superNodeID, 
			originalSecondSuperGraphMap[secondSuperVGraph[*vi]],
			superNodeID, *vi);
	}

	// update the original leaf node to the passed vgraph
	replaceLeafNodeID(selectedSuperNodeID, selectedLeafNodeID,
		superNodeID, newLeafID);

	// remove the selected leaf
	boost::clear_vertex(selectedLeafNodeID, firstSuperNodeVGraph);
	boost::remove_vertex(selectedLeafNodeID, firstSuperNodeVGraph);

	// replace other leaves
	boost::tie(vi, ve) = boost::vertices(firstSuperNodeVGraph);
	for(; vi != ve; vi++){
		assert(originalSuperGraphMap.find(firstSuperNodeVGraph[*vi]) != originalSuperGraphMap.end());

		replaceLeafNodeID(selectedSuperNodeID, 
			originalSuperGraphMap[firstSuperNodeVGraph[*vi]],
			selectedSuperNodeID, *vi);
	}
	cv::imwrite("moveLeafNode.jpg",graphBGIDImage[currentGraphNo]);
	selectedSuperNodeID = superNodeID;
	selectedLeafNodeID  = newLeafID;// remove selected leaf vertex from selected super vertex
}

// void StellaWidget::tryToDeleteOneSuperNeighbor(){
// 	int superNodeID = getPassedSuperNodeID();
// 	if(superNodeID == NO_SELECTION)
// 		return;
// 	std::vector<NodeID>::iterator result = std::find(neighborNodeID.begin(), neighborNodeID.end(), superNodeID);
// 	if(result != neighborNodeID.end()){
// 		neighborNodeID.erase(result);
// 	}
// }
// 
// void StellaWidget::tryToDeleteOneLeafNeighbor(){
// 	int leafNodeID = getPassedLeafNodeID(selectedSuperNodeID);
// 	if(leafNodeID == NO_SELECTION)
// 		return;
// 	std::vector<NodeID>::iterator result = std::find(neighborNodeID.begin(), neighborNodeID.end(), leafNodeID);
// 	if(result != neighborNodeID.end()){
// 		neighborNodeID.erase(result);
// 	}
// }
// 
// void StellaWidget::tryToDeleteOneSuperVNeighbor(){
// 	int superNodeID = getPassedSuperNodeID();
// 	if(superNodeID == NO_SELECTION)
// 		return;
// 	std::vector<NodeID>::iterator result = std::find(neighborNodeID.begin(), neighborNodeID.end(), superNodeID);
// 	if(result != neighborNodeID.end()){
// 		neighborNodeID.erase(result);
// 	}
// }
// 
// void StellaWidget::tryToAddOneSuperNeighbor(){
// 	int superNodeID = getPassedSuperNodeID();
// 	if(superNodeID == NO_SELECTION)
// 		return;
// 	std::vector<NodeID>::iterator result = std::find(neighborNodeID.begin(), neighborNodeID.end(), superNodeID);
// 	if(result == neighborNodeID.end()){
// 		neighborNodeID.push_back(superNodeID);
// 	}
// }
// 
// void StellaWidget::tryToAddOneLeafNeighbor(){
// 	int leafNodeID = getPassedLeafNodeID(selectedSuperNodeID);
// 	if(leafNodeID == NO_SELECTION)
// 		return;
// 	std::vector<NodeID>::iterator result = std::find(neighborNodeID.begin(), neighborNodeID.end(), leafNodeID);
// 	if(result == neighborNodeID.end()){
// 		neighborNodeID.push_back(leafNodeID);
// 	}
// }
// 
// void StellaWidget::tryToAddOneSuperVNeighbor(){
// 	int superNodeID = getPassedSuperNodeID();
// 	if(superNodeID == NO_SELECTION)
// 		return;
// 	std::vector<NodeID>::iterator result = std::find(neighborNodeID.begin(), neighborNodeID.end(), superNodeID);
// 	if(result == neighborNodeID.end()){
// 		neighborNodeID.push_back(superNodeID);
// 	}
// }

void StellaWidget::drawGraphID(int graphNo, cv::Mat &img, double scale, 
	int curveStyle, int arg0){
		_Graph &g = graphs[graphNo]->g;
		_Graph::vertex_iterator vi, ve;
		boost::tie(vi, ve) = boost::vertices(g);
		for(; vi!=ve; vi++){
			drawSuperNodeID(graphNo, img, *vi, scale, curveStyle, arg0);
		}
}

void StellaWidget::drawSuperNodeID(int graphNo, cv::Mat &img, int superNodeID, double scale, 
	int curveStyle, int arg0){
		_Graph &g = graphs[graphNo]->g[superNodeID]->superNode->g;
		_Graph::vertex_iterator vi, ve;
		boost::tie(vi, ve) = boost::vertices(g);
		for(; vi != ve; vi++){
			drawLeafNodeID(graphNo, img, superNodeID, *vi, scale, curveStyle, arg0);
		}
}

void StellaWidget::drawLeafNodeID(int graphNo, cv::Mat &img, int superNodeID, int leafNodeID, 
	double scale, int curveStyle, int arg0){
		vNode *node = graphs[graphNo]->g[superNodeID]->superNode->g[leafNodeID];
		vCurve *curve = node->getCurve();
		switch(curveStyle){
		case NODEID_CURVE:
			drawVCurve(curve, img, scale, 0, 0, 
				cv::Scalar(superNodeID, leafNodeID, LEAF_AREA, graphNo), 
				NODEID_CURVE_THICKNESS, 8);
			break;
		case COLOR_IMAGE_CURVE:
			drawVCurve(curve, img, scale, 0, 0,
				HSV2RGB(getSuperNodeHSV_H(graphNo, superNodeID), LIGHT_S, LIGHT_V),
				IMAGE_UNSELECTED_CURVE_THICKNESS);
			break;
		case GRAY_IMAGE_CURVE:
			drawVCurve(curve, img, scale, 0, 0, HSV2RGB(0, GRAY_S, arg0),
				IMAGE_UNSELECTED_CURVE_THICKNESS);
			break;
		case SCALED_IMAGE_CURVE:
			drawVCurve(curve, img, scale, 0, 0, 
				cv::Scalar(superNodeID, leafNodeID, LEAF_AREA, graphNo), 
				SCALED_CURVE_THICKNESS);
			break;
		}

}

void StellaWidget::generateBgOfPreviousGraphs(int graphNo){
	double scale = 0.3;

	cv::Size scaledSize(graphBGIDImage[graphNo].cols*scale / this->scale, graphBGIDImage[graphNo].rows*scale / this->scale);

	cv::Mat scaledNodeID = cv::Mat(scaledSize, graphBGIDImage[graphNo].type(), cv::Scalar(0, 0, BLANK_AREA, graphNo));
	cv::Mat scaledImage  = cv::Mat(scaledSize, graphBGIDImage[graphNo].type(), WINDOW_BACKGROUND_COLOR);
	drawGraphID(graphNo, scaledNodeID, scale, SCALED_IMAGE_CURVE);
	//cv::resize(nodeIDImage, scaledImage, sz, 0, 0, cv::INTER_NEAREST);

	cv::Scalar mind = cv::Scalar(20);
	cv::Mat dist = cv::Mat(scaledSize, CV_16U, mind);

	for(int i=0; i<scaledSize.height; i++){
		for(int j=0; j<scaledSize.width; j++){
			cv::Vec4b id = scaledNodeID.at<cv::Vec4b>(i, j);
			if(id[2] == LEAF_AREA){
				updateImage(dist, scaledNodeID, scaledImage, i, j, 
					cv::Scalar(id[0], id[1], SUPER_AREA, graphNo),
					HSV2RGB(getSuperNodeHSV_H(graphNo, id[0]), VERY_LIGHT_S, VERY_LIGHT_V));
			}
		}
	}
	cv::resize(scaledNodeID, graphBGIDImage[graphNo], 
		cv::Size(graphBGIDImage[graphNo].cols, graphBGIDImage[graphNo].rows),
		0, 0, cv::INTER_NEAREST);
	cv::resize(scaledImage, graphBGImage[graphNo], 
		cv::Size(graphBGImage[graphNo].cols, graphBGImage[graphNo].rows),
		0, 0, cv::INTER_CUBIC);
	scaledNodeID.release();
	scaledImage.release();
}

void StellaWidget::drawPreviousGraph(cv::Mat &img, std::vector<int> &previousGraphs, int startV, int endV){
	int validGraphNum = 1;
	for(int i = 0; i < graphsNum; i++){
		if(previousGraph[i] != NO_PREVIOUS_GRAPH){
			validGraphNum ++;
		}
	}
	double k = (endV - startV) / sqrtf(validGraphNum - 2);
	for(int i=previousGraphs.size()-1; i>=0; i--){
		drawGraphID(previousGraphs[i], img, scale, 
			GRAY_IMAGE_CURVE, startV + k * sqrtf(i));
	}

}

void StellaWidget::generateBgOfCurrentGraph(int graphNo){
	// 	graphBGImage[graphNo].setTo(WINDOW_BACKGROUND_COLOR);
	// 	for(int i=0; i<graphBGIDImage[graphNo].rows; i++){
	// 		for(int j=0; j<graphBGIDImage[graphNo].cols; j++){
	// 			cv::Vec4b id = graphBGIDImage[graphNo].at<cv::Vec4b>(i, j);
	// 			cv::Vec4b &color = graphBGImage[graphNo].at<cv::Vec4b>(i, j);
	// 			if(id[2] != BLANK_AREA){
	// 				color = HSV2RGB(getSuperNodeHSV_H(graphNo, id[0]), VERY_LIGHT_S, VERY_LIGHT_V);
	// 			}
	// 		}
	// 	}
	//char name[20];
	//sprintf(name, "image%d.jpg", graphNo);
	//cv::imwrite(name, graphBGImage[graphNo]);

	vector<int> prevGraphNo = getPreviousGraph(graphNo);
	drawPreviousGraph(graphBGImage[graphNo], prevGraphNo, GRAY_V, GRAY_V + 25);
	drawGraphID(graphNo, graphBGImage[graphNo], scale, COLOR_IMAGE_CURVE);

}

std::vector<int> StellaWidget::getPreviousGraph(int graphNo){
	vector<int> prevGraphNo;
	int prev = previousGraph[graphNo];
	while(prev != NO_PREVIOUS_GRAPH){
		prevGraphNo.push_back(prev);
		prev = previousGraph[prev];
	}
	return prevGraphNo;
}

void StellaWidget::generatePreviousAndNextGraphArrays(){
	previousGraph = new int[graphsNum];
	nextGraph = new int[graphsNum];
	for(int i = 0; i<graphsNum; i++){
		vVirtualEdge *vve = graphs[i]->virtualEdges;
		if(vve){
			for(int j = 0; j<graphsNum; j++){
				if(vve->prevGraph == graphs[j]){
					previousGraph[i] = j;
				}if(vve->nextGraph == graphs[j]){
					nextGraph[i] = j;
				}
			}
			if(previousGraph[i] < 0 || previousGraph[i] >= graphsNum){
				previousGraph[i] = NO_PREVIOUS_GRAPH;
			}
			if(nextGraph[i] < 0 || nextGraph[i] >= graphsNum){
				nextGraph[i] = NO_PREVIOUS_GRAPH;
			}
		}else{
			previousGraph[i] = NO_PREVIOUS_GRAPH;
			nextGraph[i]	 = NO_PREVIOUS_GRAPH;
		}
	}
}

void StellaWidget::generateBg(int graphNo){
	generateBgOfPreviousGraphs(graphNo);
	generateBgOfCurrentGraph(graphNo);
}

void StellaWidget::generateBg(){
	generatePreviousAndNextGraphArrays();
	for(int i=0; i<graphsNum; i++){
		std::cout<<"Graph: "<<i<<"...";
		generateBg(i);
		std::cout<<"Over."<<std::endl;
	}
}

void StellaWidget::initializeHSVH(){
	srand((unsigned)time(NULL));
	for(int i = 0; i < graphsNum; i++){
		_Graph &g = graphs[i]->g;
		_Graph::vertex_iterator vi,ve;
		boost::tie(vi, ve) = boost::vertices(g);
		for(; vi != ve; vi++){
			g[*vi]->setH(rand()%360);
			_Graph &g2 = g[*vi]->superNode->g;
			_Graph::vertex_iterator vi2,ve2;
			boost::tie(vi2, ve2) = boost::vertices(g2);
			for(; vi2 != ve2; vi2++){
				g2[*vi2]->setH(rand()%360);
			}
		}
	}
}

void StellaWidget::initializeSplines(){
	for(int i = 0; i < graphsNum; i++){
		_Graph &g = graphs[i]->g;
		_Graph::vertex_iterator vi,ve;
		boost::tie(vi, ve) = boost::vertices(g);
		for(; vi != ve; vi++){
			_Graph &g2 = g[*vi]->superNode->g;
			_Graph::vertex_iterator vi2,ve2;
			boost::tie(vi2, ve2) = boost::vertices(g2);
			for(; vi2 != ve2; vi2++){
				vCurve *curve = g2[*vi2]->getCurve();
				curve->spline = new vCurve();
				curve->getSplineCurve(*(curve->spline), 1, LOD);
			}
		}
	}
}

void StellaWidget::initializeTimer(){
	timer = new QTimer(this);
	timer->setInterval(FPS);
	connect(timer,SIGNAL(timeout()), this, SLOT(updateAnimation()));
}

void StellaWidget::initializeImages(){
	QSize s=size();
	displayImage		= cv::Mat(s.height(), s.width(), CV_8UC4, WINDOW_BACKGROUND_COLOR);
	nodeIDImage			= cv::Mat(s.height(), s.width(), CV_8UC4, WINDOW_BACKGROUND_COLOR);
	graphBGImage		= new cv::Mat[graphsNum];
	graphBGIDImage	= new cv::Mat[graphsNum];
	for(int i=0; i < graphsNum; i++){
		graphBGImage[i]	 	= cv::Mat(s.height(), s.height(), CV_8UC4, WINDOW_BACKGROUND_COLOR);
		graphBGIDImage[i]	= cv::Mat(s.height(), s.height(), CV_8UC4, WINDOW_BACKGROUND_COLOR);
	}
}

void StellaWidget::initializeDrawing(){
	generateBg();
	draw();
	drawNodeIDCurrentMode();
}

void StellaWidget::initializeVariables(){
	resetSelectionVariables();
	initializeImages();
}

void StellaWidget::initializeVNodeVariables(){
	initializeHSVH();
	initializeSplines();
}

void StellaWidget::initialize(){
	initializeTimer();
	initializeVariables();
	initializeVNodeVariables();
	initializeDrawing();

	//repaint();
	startTimer();
}

void StellaWidget::resetSelectionVariables(){
	ms							= NO_EVENT;
	currentCurveNo				= 0;
	currentGraphNo				= 4;
	selectedSuperNodeID			= NO_SELECTION;
	selectedLeafNodeID			= NO_SELECTION;
}

bool StellaWidget::isLeafNodeSelected(){
	return selectedLeafNodeID!=NO_SELECTION && 
		selectedLeafNodeID != NO_SELECTION;
}

bool StellaWidget::isSuperNodeSelected(){
	return selectedSuperNodeID != NO_SELECTION && selectedLeafNodeID == NO_SELECTION;
}

bool StellaWidget::isNodeSelected(){
	return selectedSuperNodeID != NO_SELECTION;
}

bool StellaWidget::isNodeNoSelection(){
	return selectedSuperNodeID ==NO_SELECTION && selectedLeafNodeID == NO_SELECTION;
}
// 
// /************************************************************************/
// /* Move one node from one super node(from) to another super node(to),   */
// /* 'node' means the node No. in super node vgraph 'from'.				*/
// /************************************************************************/
// void StellaWidget::moveNode(vGraph *from, vGraph *to, NodeID node){
// 	//std::cout<<"move node: "<<node<<std::endl;
// }
// 
// /************************************************************************/
// /* Create a super node with only one leaf node, which comes from the    */
// /* leaf node of the super node of graph vg.                             */
// /************************************************************************/
// void StellaWidget::createSuperNode(vGraph *vg, NodeID superNode, NodeID leafNode){
// 	//create
// }
// 
// /************************************************************************/
// /* add one edge (nid1, nid2) in graph vg,which may be a super node graph.*/
// /************************************************************************/
// void StellaWidget::addEdge(vGraph *vg, NodeID nid1, NodeID nid2){
// 	//std::cout<<"add edge: "<<nid1<<", "<<nid2<<std::endl;
// }
// 
// /************************************************************************/
// /* add one virtual edge(supernode1, supernode2) between graph1 and		*/
// /* graph2                                                               */
// /************************************************************************/
// void StellaWidget::addVEdge(int graphNo1, int graphNo2, NodeID superNode1, NodeID superNode2){
// 	//
// }
// 
// /************************************************************************/
// /* delete edge(nid1, nid2) in graph vg, which may be a super node graph.*/
// /************************************************************************/
// void StellaWidget::deleteEdge(vGraph *vg, NodeID nid1, NodeID nid2){
// 	std::cout<<"Delete edge:\t("<<nid1<<", "<<nid2<<")"<<std::endl;
// }
// 
// /************************************************************************/
// /* delete one virtual edge(supernode1, supernode2) between graph1 and	*/
// /* graph2                                                               */
// /************************************************************************/
// void StellaWidget::deleteVEdge(int graphNo1, int graphNo2, NodeID superNode1, NodeID superNode2){
// 	
// }

void StellaWidget::deleteEdge(){

}

void StellaWidget::deleteVEdge(){

}

void StellaWidget::setScale(int ww, int hh){
	QSize s=size();
	scale = std::min((double)s.width()/ww, (double)s.height()/hh);
}

void StellaWidget::setGraphs(vGraph* gs, int n){
	graphsNum = n;
	graphs = new vGraph*[n];
	for(int i=0;i<n; i++){
		graphs[i] = &(gs[i]);
	}
}

void StellaWidget::setOrder(vector<vector<vector<NodeID>>> &order){
	for(int i=0; i<order.size(); i++){
		this->order.push_back(order[i]);
	}
}

void StellaWidget::setImgProcess(imageProc* image_process){
	this->image_process = image_process;
}

/************************************************************************/
/* reorder interface. You can call the imageProc                        */
/************************************************************************/
void StellaWidget::reorder(){
	//reorder


}

void StellaWidget::startTimer(){
	//std::cout<<"startTimer!"<<std::endl;
	currentCurveNo = 0;
	currentCurveSplineNo = 0;
	initializeAnimation();
	if(order[currentGraphNo].empty())
		return;
	if(!timer->isActive()){
		timer->start();
	}
}

void StellaWidget::stopTimer(){
	//std::cout<<"stop Timer!"<<std::endl;
	if(timer->isActive()){
		timer->stop();
		draw();
		repaint();
	}
}

void StellaWidget::initializeAnimation(){
	std::vector<int> previousGraphs = getPreviousGraph(currentGraphNo);
	displayImage.setTo(WINDOW_BACKGROUND_COLOR);
	drawGraphID(currentGraphNo, displayImage, scale, GRAY_IMAGE_CURVE, ANIMATION_V + 65);
	drawPreviousGraph(displayImage, previousGraphs, ANIMATION_V + 30, ANIMATION_V + 60);
}

void StellaWidget::updateAnimation(){
	vNode* superNode= graphs[currentGraphNo]->g[order[currentGraphNo][currentCurveNo][0]];
	vNode* leafNode	=	superNode->superNode->g[order[currentGraphNo][currentCurveNo][1]];
	vCurve* curve = leafNode->getCurve();
	vCurve* spline	= curve->spline;
	switch(curve->type){
	case vCurve::OPEN:
		{
			vPoint2D p1 = spline->curve2d[currentCurveSplineNo];
			vPoint2D p2 = spline->curve2d[currentCurveSplineNo + 1];

			cv::line(displayImage, 
				cv::Point(p1.x() * scale, p1.y() * scale),
				cv::Point(p2.x() * scale, p2.y() * scale), 
				HSV2RGB(0, 0, ANIMATION_V), IMAGE_UNSELECTED_CURVE_THICKNESS, CV_AA);
			cv::circle(displayImage,
				cv::Point(p2.x() * scale, p2.y() * scale),
				1, cv::Scalar(0, 0, 1, 0), -1);
		}
		break;
	case vCurve::STRAIGHT:
		{
			cv::Point start,end;
			start.x = curve->curve2d.front().x();
			start.y = curve->curve2d.front().y();
			end.x = curve->curve2d.back().x();
			end.y = curve->curve2d.back().y();

			int resolution = curve->Length()/3;

			// restore to pass the center
			cv::Point _cent(curve->center()[0], curve->center()[1]);
			cv::Point s(start.x,start.y);
			cv::Point e(end.x,end.y);
			cv::Point lineVect = e-s;
			cv::Point halfVect(lineVect.x/2.0f, lineVect.y/2.0f);
			s = _cent - halfVect + currentCurveSplineNo * lineVect * (1.0/LINE_SEGMENT);
			e = _cent - halfVect + (currentCurveSplineNo + 1) * lineVect * (1.0/LINE_SEGMENT);
			cv::line(displayImage, s * scale, e * scale,
				HSV2RGB(0, 0, ANIMATION_V), IMAGE_UNSELECTED_CURVE_THICKNESS, CV_AA);
		}
		break;
	}

	currentCurveSplineNo++;
	repaint();
	if((curve->type == vCurve::OPEN && currentCurveSplineNo >= spline->curve2d.size() - 1)
		||(curve->type == vCurve::STRAIGHT && currentCurveSplineNo >= LINE_SEGMENT)){
			currentCurveSplineNo = 0;
			currentCurveNo = (currentCurveNo + 1) % order[currentGraphNo].size();
			if(currentCurveNo == 0){
				initializeAnimation();
			}
	}
}
