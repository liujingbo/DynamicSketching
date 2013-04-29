#include "imageProc.h"

imageProc::imageProc(){
	//QImage m_qImgPalette = QImage(w, h, QImage::Format_RGB32);
	//cv::Mat   m_cvImgPalette = cv::Mat(m_qImgPalette.size().height(), m_qImgPalette.size().width(), CV_8UC4, m_qImgPalette.bits());

	//IplImage *img = cvLoadImage("lena.jpg", 1);
	//QImage qImage(img->imageData, img->width, img->height, img->widthStep, QImage::Format_RGB888);

	//ori_image=cvLoadImage("203-QT2486-Scenic-Drive-Lakeside.jpg");
	ori_image=NULL;
	display_image = NULL;

	// edge contour -- can be manipulated from the UI
	isEdgeContourVisible = false;
	splitThreshold = 0.2;//0.4;// 0.96;//10.0/255*25;// splitThreshold = 10.0/255*max_color;
	splitThreshold_max = 0.5;splitThreshold_min=0;

	// pose
	this->isPoseLinesVisible = true;

	// prims
	this->isPrimVisible = false;

	// sc
	isSCVisible = false;
	this->isConvexHullVisible = false;

	numOfStrokes = 0;
	useGraph = true;
	useOrder = false;

	this->output = false;
	this->SegConnectivity = NULL;
	texture = NULL;
	this->loadTexture("..\\testdata\\textures\\charcoal_2.png");

	numOfGloabHull = 0;
}

imageProc::~imageProc(){
	//cvReleaseMatHeader(&ori_image );
	cvReleaseImage(& this->display_image);
	cvReleaseImage(& ori_image);
}


void imageProc::reset(){
	return;
}
void imageProc::clearData(){
	
	// clear stroke data
	this->edgeContours.clear();
	this->poseLines.clear();
	this->convexHulls2d.clear();
	this->PrimitiveLines.clear();
	this->SCLines.clear();
	this->silhouetteLines.clear();


	// clear order graphs
	for(int i=0;i<5;i++){
		this->graphs[i].clear();
	}

	this->segDrawingOrder.clear();
	this->segment2NodeID.clear();
	this->segmentSymmetricity.clear();

	return;
}

void imageProc::setOrignalImage(IplImage* _img){
	if(_img){
		if(this->display_image){
			cvReleaseImage(& this->display_image);
		}
		if(this->ori_image){
			cvReleaseImage(& this->ori_image);
		}
		this->display_image = _img;
		// make the original image a new copy
		//this->ori_image = _img;
		this->ori_image=cvCloneImage(_img);

		w=(ori_image->width);
		h=(ori_image->height);
	}
	else{
		this->ori_image = NULL;
		this->display_image = NULL;
		w=h=0;
	}
}

void imageProc::testBlack(){
	for(int i=0; i<h; i++){
		for(int j=0;j<w;j++){
			if(i>j){
				CvScalar s;
				s.val[0]=s.val[1]=s.val[2]=111;s.val[3]=0.5;
				cvSet2D(ori_image,i,j,s);
			}
		}
	}
}

void imageProc::draw(){
	// curves
	if(false){
		for(int i=0;i<this->curves.size();i++){
			vCurve* c= &(curves[i]);
			for(int j=0;j+1<c->size(); j++){
				CvPoint start,end;
				start.x = c->curve2d[j].x();
				start.y = c->curve2d[j].y();
				end.x = c->curve2d[j+1].x();
				end.y = c->curve2d[j+1].y();
				cvLine( this->display_image, start, end, CV_RGB(255,0,0), 3, 8 );
			}
		}
	}

	
	// clear the stroke output file if needed
	if(this->output){
		std::ofstream myfile ("..\\testdata\\strokes\\1_1.strokes", std::ios::out | std::ios::trunc | std::ios::binary);
		myfile.close();
	}

	// global marks
	// sometimes the global is not 'believable'
	// global hull is not in the order, draw it first
	if(this->isConvexHullVisible ){
		vCurve::fading_factor = 0.3;
		if(useGraph){
			using namespace boost;
			_Graph& g=this->graphs[1].g;
			
			if(this->simHullGraphOrder.size()>0 && useOrder){

				for(int i=0;i<numOfGloabHull && (numOfStrokes>0 );i++, numOfStrokes--){
					this->convexHulls2d[i].draw(this->display_image,CV_RGB(10,10,10),1, this->output);
				}

			}
			else{
				_Graph::vertex_iterator vIt, vEnd;
				tie(vIt,vEnd)=vertices(g);
				int i=0;
				for(;vIt != vEnd && (numOfStrokes>0 || !useOrder) && i<numOfGloabHull;vIt++, numOfStrokes--,i++){
					g[*vIt]->draw( this->display_image, CV_RGB(10,10,10),1,5);
				}
			}

		}
	}

	if(this->isPoseLinesVisible){
		// phasal control of ouput
		vCurve::fading_factor = 0.2;
		if(useGraph){
			using namespace boost;
			_Graph& g=this->graphs[0].g;
			_Graph::vertex_iterator vIt, vEnd;
			int i=0;
			for(tie(vIt,vEnd)=vertices(g);vIt != vEnd && (numOfStrokes>0 || !useOrder);vIt++, numOfStrokes--,i++){
				vCurve* c=NULL;

				if(this->poseGraphOrder.size()>0 && useOrder && i<this->poseGraphOrder.size()){ // not all lines are selected to draw
					g[poseGraphOrder[i][0]]->superNode->g[poseGraphOrder[i][1]]->draw( this->display_image,  CV_RGB(0,0,50),1,20, this->output);
				}
				else{
					g[*vIt]->draw( this->display_image, CV_RGB(0,0,50),1,5);
				}
			}
		}
		else{
			for(int i=0;i<this->poseLines.size()&& (numOfStrokes>0 || !useOrder);i++,numOfStrokes--){
				int ind =i;
				if(this->PoseOrder.size()>0){ind = PoseOrder[i];}

				vCurve* c=&(poseLines[ind]);
				c->draw( this->display_image, CV_RGB(0,0,50),1,5);
			}
		}
	}

	if(this->isConvexHullVisible ){
		
		vCurve::fading_factor = 0.2;
		if(useGraph){
			using namespace boost;
			_Graph& g=this->graphs[1].g;
			bool useGlobalHull = true;
			if(this->simHullGraphOrder.size()>0 && useOrder){

				for(int i=0;i<this->simHullGraphOrder.size() && (numOfStrokes>0 );i++ , numOfStrokes--){
					vNode* orderedN = g[simHullGraphOrder[i][0]]->superNode->g[simHullGraphOrder[i][1]];
					orderedN->draw( this->display_image, CV_RGB(10,10,10),1,5,this->output);
				}
			}
			else{
				_Graph::vertex_iterator vIt, vEnd;
				tie(vIt,vEnd)=vertices(g);
				if(vIt != vEnd ){ // skip first super node -> global hull
					vIt++;
				}
				for(;vIt != vEnd && (numOfStrokes>0 || !useOrder);vIt++, numOfStrokes--){
					g[*vIt]->draw( this->display_image, CV_RGB(10,10,10),1,5);
				}
			}

		}
		/*else{
			for(int i=0;i<this->convexHulls2d.size()&& (numOfStrokes>0 || !useOrder);i++, numOfStrokes--){
				int ind=i;
				if(this->ConvexHull2dOrder.size()>0)
					ind = this->ConvexHull2dOrder[i];

				vCurve* c=&(convexHulls2d[ind]);
				c->draw(this->display_image,CV_RGB(100,100,100),1);
			}
		}*/

		//for(int i=0;i<this->convexHulls2d.size()&& numOfStrokes>0;i++, numOfStrokes--){
		//	vCurve* c=&(convexHulls2d[i]);
		//	for(int j=0;j+1<c->size(); j++){
		//		CvPoint start,end;
		//		start.x = c->curve2d[j].x();
		//		start.y = c->curve2d[j].y();
		//		end.x = c->curve2d[j+1].x();
		//		end.y = c->curve2d[j+1].y();
		//		cvLine( this->display_image, start, end, CV_RGB(100,100,100), 1, 8 );
		//	}
		//}
	}	

	
	if(this->isPrimVisible){
		// phasal control of ouput
		vCurve::fading_factor = 0.4;
		if(useGraph){
			using namespace boost;
			_Graph& g=this->graphs[3].g;

			if(this->PrimsGraphOrder.size()>0 && useOrder){
				for(int i=0;i<this->PrimsGraphOrder.size() && numOfStrokes>0 ;i++ , numOfStrokes--){
					vNode* orderedN = g[PrimsGraphOrder[i][0]]->superNode->g[PrimsGraphOrder[i][1]];
					orderedN->draw( this->display_image, CV_RGB(0,0,200),1,10, this->output);
				}
			}
			else{
				_Graph::vertex_iterator vIt, vEnd;
				for(tie(vIt,vEnd)=vertices(g);vIt != vEnd && (numOfStrokes>0 || !useOrder);vIt++, numOfStrokes--){
					/*vCurve* c= g[*vIt]->getCurve();
					if(c!= NULL){
					c->draw( this->display_image, CV_RGB(0,100,0));
					}*/
					g[*vIt]->draw( this->display_image, CV_RGB(0,0,200),1);
				}
			}
		}
	}


	//edge contours
	if(this->isEdgeContourVisible){
		// phasal control of ouput
		vCurve::fading_factor = 0.6;
		if(useGraph){
			using namespace boost;
			_Graph& g=this->graphs[2].g;
			if(this->SimEdgeGraphOrder.size()>0 && useOrder){
				for(int i=0;i<this->SimEdgeGraphOrder.size() && (numOfStrokes>0 );i++ , numOfStrokes--){
					vNode* orderedN = g[SimEdgeGraphOrder[i][0]]->superNode->g[SimEdgeGraphOrder[i][1]];
					orderedN->draw( this->display_image, CV_RGB(0,100,0),4,10,this->output);
				}
			}
			else{
				_Graph::vertex_iterator vIt, vEnd;
				for(tie(vIt,vEnd)=vertices(g);vIt != vEnd && (numOfStrokes>0 || !useOrder);vIt++, numOfStrokes--){
					/*vCurve* c= g[*vIt]->getCurve();
					if(c!= NULL){
					c->draw( this->display_image, CV_RGB(0,100,0));
					}*/
					g[*vIt]->draw( this->display_image, CV_RGB(0,100,0));

				}
			}
		}
		else{
			for(int i=0;i<this->edgeContours.size() && (numOfStrokes>0 || !useOrder);i++, numOfStrokes--){

				int ind =i;
				if(this->SimEdgeOrder.size()>0){
					ind = this->SimEdgeOrder[i];
				}

				vCurve* c=&(edgeContours[ind]);

				if(false){
					c->draw(this->display_image,CV_RGB(0,250,0));
				}
				if (false){
					for(int j=0;j<c->size(); j++){
						const double* color = c->curve2d[j].color;
						cvCircle(this->display_image, cvPoint(c->curve2d[j].x(),c->curve2d[j].y()), color[0]*4.0/255, CV_RGB(color[0],color[1],color[2]), -1); // cvScalar->BGR, opencv people have shit heads
					}
				}
				if(true){ // use straight lines
					CvPoint start,end;
					start.x = c->curve2d[0].x();
					start.y = c->curve2d[0].y();
					end.x = c->curve2d.back().x();
					end.y = c->curve2d.back().y();
					cvLine( this->display_image, start, end, CV_RGB(0,100,0), 1, 8 );
				}
			}
		}
	}

	
	
	if(this->isSCVisible ){
		// phasal control of ouput
		vCurve::fading_factor = 1;
		if(useGraph){
			using namespace boost;
			_Graph& g=this->graphs[4].g;
			
			if(this->EXSCGraphOrder.size()>0 && useOrder){
				for(int i=0;i<this->EXSCGraphOrder.size() && numOfStrokes>0 ;i++ , numOfStrokes--){
					vNode* orderedN = g[EXSCGraphOrder[i][0]]->superNode->g[EXSCGraphOrder[i][1]];
					orderedN->draw( this->display_image, CV_RGB(0,0,0),2,5, this->output);
				}
			}
			else{
				_Graph::vertex_iterator vIt, vEnd;
				for(tie(vIt,vEnd)=vertices(g);vIt != vEnd && (numOfStrokes>0 || !useOrder);vIt++, numOfStrokes--){
					/*vCurve* c= g[*vIt]->getCurve();
					if(c!= NULL){
					c->draw( this->display_image, CV_RGB(0,100,0));
					}*/
					g[*vIt]->draw( this->display_image, CV_RGB(0,0,0),2);
				}
			}
		}
		else{
			for(int i=0;i<this->SCLines.size() && (numOfStrokes>0 || !useOrder);i++, numOfStrokes--){
				int ind=i;
				if(this->SCorder.size()>0)	ind = this->SCorder[i];
				vCurve* c=&(SCLines[ind]);
				if(true){
					c->draw(this->display_image,CV_RGB(0,0,0),2);
				}
				if (false){
					for(int j=0;j<c->size(); j++){
						//const double* color = c->curve2d[j].color;
						cvCircle(this->display_image, cvPoint(c->curve2d[j].x(),c->curve2d[j].y()), 2, CV_RGB(0,0,0), 1); // cvScalar->BGR, opencv people have shit heads
					}
				}
				if(false){ // use straight lines
					CvPoint start,end;
					start.x = c->curve2d[0].x();
					start.y = c->curve2d[0].y();
					end.x = c->curve2d.back().x();
					end.y = c->curve2d.back().y();
					cvLine( this->display_image, start, end, CV_RGB(0,0,0), 1, 8 );
				}
			}

			for(int i=0;i<this->silhouetteLines.size() && (numOfStrokes>0 || !useOrder);i++, numOfStrokes--){
				int ind=i;
				vCurve* c=&(silhouetteLines[ind]);
				c->draw(this->display_image,CV_RGB(0,0,0),1);
			}
		}
	}


	return;
}
void imageProc::loadTexture(char* fileName){
	this->texture = cvLoadImage(fileName);
	return;
}
/**********************************************************

					UI control from vDraw

***********************************************************/
void imageProc::canny(){

	IplImage* dst = cvCreateImage( cvGetSize(ori_image), 8, 1 );
	cvCvtColor(ori_image,dst,CV_RGB2GRAY );
	cvNormalize(dst,dst,255,0,CV_MINMAX );
	cvCanny( dst, dst, 30, 70, 3 );
	cvNormalize(dst,dst,255,0,CV_MINMAX );
	cvCvtColor(dst,ori_image,CV_GRAY2RGB);
	
	cvReleaseImage(&dst);
	
	//cvLaplace( tmp, ori_image );
	//testBlack();
}
int imageProc::houghTransform(){

	IplImage* src=this->ori_image;
	this->curves.clear();
    if( ori_image!= 0)
    {
        IplImage* dst = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
        CvMemStorage* storage = cvCreateMemStorage(0);
        CvSeq* lines = 0;
        int i;
		cvCvtColor(ori_image,dst,CV_RGB2GRAY );
		cvNormalize(dst,dst,255,0,CV_MINMAX );
        cvCanny( dst, dst, 30, 70, 3 );
		cvDilate(dst,dst);
		cvNormalize(dst,dst,255,0,CV_MINMAX );
        cvCvtColor( dst, src, CV_GRAY2BGR );
#if 0
        lines = cvHoughLines2( dst, storage, CV_HOUGH_STANDARD, 5, CV_PI/180*5, 150, 0, 0 );

        for( i = 0; i < MIN(lines->total,100); i++ )
        {
            float* line = (float*)cvGetSeqElem(lines,i);
            float rho = line[0];
            float theta = line[1];
            CvPoint pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            pt1.x = cvRound(x0 + 1000*(-b));
            pt1.y = cvRound(y0 + 1000*(a));
            pt2.x = cvRound(x0 - 1000*(-b));
            pt2.y = cvRound(y0 - 1000*(a));
            cvLine( src, pt1, pt2, CV_RGB(255,0,0), 3, 8 );
        }
#else
        lines = cvHoughLines2( dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180*3, 100, 30, 20 );
        for( i = 0; i < lines->total; i++ )
        {
            CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
            //cvLine( src, line[0], line[1], CV_RGB(255,0,0), 3, 8 );
			
			vCurve new_line;
			for(int j=0;j<2;j++){
				new_line.addPoint( vPoint2D( line[j].x, line[j].y));
			}
			curves.push_back(new_line);
			
        }
#endif
		cvReleaseImage(&dst);
		return lines->total;
    }

	return 0;

}
// give the para's value back
double imageProc::paraNameChanged_split(double & max, double & min ){
	max = splitThreshold_max;
	min = splitThreshold_min;
	return splitThreshold;
}
void imageProc::paraValChanged_split(int val){
	splitThreshold = val/100.0 * (splitThreshold_max-splitThreshold_min)+splitThreshold_min;
	return;
}
bool imageProc::loadSegmentRelations(const char* str){
	if (str==NULL || strlen(str)==0) return false;
	std::ifstream ifs(str);
	if (ifs.fail()) return false;


	segmentSymmetricity.clear();
	while(!ifs.eof()){
		char thisLetter;
		std::vector<int> in_data;
		ifs >> thisLetter;
        switch (thisLetter)
        {
        case '#':
			char thisLine[50];
			ifs.getline(thisLine, 50);
			int seg1,seg2;
			ifs>>seg1>>seg2;
			in_data.push_back(seg1);
			in_data.push_back(seg2);
			this->segmentSymmetricity.push_back(in_data);
            break;
		default:
			assert(0);// false format
        }
	
	}

	ifs.clear();
    ifs.close();

}

/**********************************************************

					order

***********************************************************/
void imageProc::numOfStrokesChanged(int val){
	this->numOfStrokes = val;

	return;
}
int imageProc::order2DStrokes(){

	this->output = true;

	this->useOrder = true;

	numOfStrokes = 0;
	segDrawingOrder.clear();
	// re-order
		//edgeContours
		//poseLines
		//convexHulls2d
		//SCLines
	this->PoseOrder.clear();
	this->SimEdgeOrder.clear();
	this->ConvexHull2dOrder.clear();
	this->SCorder.clear();
	
	// order individual curve set
	/*if(this->isPoseLinesVisible && this->poseLines.size()>0){
		this->PoseOrder = this->orderCurveSet(&(this->poseLines));
	}

	if(this->isEdgeContourVisible && this->edgeContours.size()>0)
		this->SimEdgeOrder = this->orderCurveSet(&(this->edgeContours));

	if(this->isConvexHullVisible && this->convexHulls2d.size()>0)
		this->ConvexHull2dOrder = this->orderCurveSet(&(this->convexHulls2d));

	if(this->isSCVisible && SCLines.size()>0){
		this->SCorder = this->orderCurveSet(&(this->SCLines));
	}*/


	// initiate virtual edges
	const int numOfPhases = 5;
	bool phaseUsing[numOfPhases] ;
	phaseUsing[0] = this->isPoseLinesVisible && this->poseLines.size()>0 ;
	phaseUsing[1] = this->isConvexHullVisible && this->convexHulls2d.size()>0;
	phaseUsing[2] = this->isEdgeContourVisible && this->edgeContours.size()>0;
	phaseUsing[3] = this->isPrimVisible && this->PrimitiveLines.size()>0;
	phaseUsing[4] = this->isSCVisible && SCLines.size()>0;

	for(int i=0;i<numOfPhases;i++){
		if(phaseUsing[i]){
			this->graphs[i].virtualEdges = new vVirtualEdge();
		}
	}
	for(int i=0;i<numOfPhases-1;i++){
		if(phaseUsing[i]){
			// find the next visible phase and build connection
			for(int j=i+1;j<numOfPhases;j++){
				if(phaseUsing[j]){
					this->graphs[i].virtualEdges->nextGraph = &(this->graphs[j]);
					this->graphs[j].virtualEdges->prevGraph = &(this->graphs[i]);

					// pair up the super nodes between phase i and phase j
					using namespace boost;
					_Graph::vertex_iterator vIt, vEnd;
					vGraph& fromGraph = this->graphs[i];
					vGraph& toGraph = this->graphs[j];
					for(tie(vIt,vEnd)=boost::vertices(fromGraph.g);vIt != vEnd ;vIt++){
						int segmentID = fromGraph.g[*vIt]->segmentID;

						_Graph::vertex_iterator vIt2, vEnd2;
						for(tie(vIt2,vEnd2)=boost::vertices(toGraph.g);vIt2 != vEnd2 ;vIt2++){
							int segmentID2 = toGraph.g[*vIt2]->segmentID;
							if(segmentID2 == segmentID){
								// add map
								std::vector<NodeID> map2next;
								map2next.push_back(*vIt);
								map2next.push_back(*vIt2);
								fromGraph.virtualEdges->map2next.push_back(map2next);

								std::vector<NodeID> map2prev;
								map2prev.push_back(*vIt2);
								map2prev.push_back(*vIt);
								toGraph.virtualEdges->map2prev.push_back(map2prev);
								break;
							}
						}
					}

					break;
				}
			}
		}
	}


	// set the phase graphs
	vGraph& poseGraph = this->graphs[0];
	vGraph& firstSketchGraph = this->graphs[1];
	vGraph& simLineGraph = this->graphs[2];
	vGraph& primGraph = this->graphs[3];
	vGraph& exscGraph = this->graphs[4];

	std::vector<NodeID> poseStartNID;
	std::vector<NodeID> firstSketchStartNID;
	std::vector<NodeID> simLineStartNID;
	std::vector<NodeID> PrimStartNID;
	std::vector<NodeID> exscStartNID;

	// set and clear the reference graph
	vGraph& ref = this->graphs[4];
	ref.clearVoting();

	// build drawing orders
	if(this->graphs[4].g.m_vertices.size() ==0) return 0;
	if(this->isPoseLinesVisible && this->poseLines.size()>0){
		if(segDrawingOrder.size()<=0){
			std::vector<NodeID> nID = this->findTopNode(ref);
			this->segDrawingOrder = this->buidSegOrder( 
				ref,nID, this->SegConnectivity, &(this->segment2NodeID));
		}

		poseStartNID = this->findTopNode(poseGraph);//this->findStartNode(this->graphs[0],this->graphs[3]);
		poseGraphOrder = this->orderCurveSet(poseStartNID, poseGraph,ref);
	}

	if(this->isConvexHullVisible && this->convexHulls2d.size()>0){
		if(segDrawingOrder.size()<=0){
			std::vector<NodeID> nID = this->findTopNode(ref);
			this->segDrawingOrder = this->buidSegOrder( 
				ref,nID, this->SegConnectivity, &(this->segment2NodeID));
		}
		firstSketchStartNID = this->findTopNode(firstSketchGraph);
		simHullGraphOrder = this->orderCurveSet(firstSketchStartNID, firstSketchGraph, ref, true);
	}

	if(this->isEdgeContourVisible && this->edgeContours.size()>0){
		simLineStartNID =  this->findTopNode(simLineGraph);//this->findStartNode(this->graphs[2],this->graphs[3]);
		this->segDrawingOrder = this->buidSegOrder( simLineGraph,simLineStartNID, this->SegConnectivity, &(this->segment2NodeID));
		SimEdgeGraphOrder = this->orderCurveSet(simLineStartNID, simLineGraph,ref,false);
	}

	if(this->isPrimVisible && this->PrimitiveLines.size()>0){
		PrimStartNID = this->findTopNode(primGraph);
		this->segDrawingOrder = this->buidSegOrder( primGraph,PrimStartNID, this->SegConnectivity, &(this->segment2NodeID));
		PrimsGraphOrder = this->orderCurveSet(PrimStartNID, primGraph,ref,true);
	}

	if(this->isSCVisible && SCLines.size()>0){
		exscStartNID = this->findTopNode(exscGraph);
		this->segDrawingOrder = this->buidSegOrder( exscGraph,exscStartNID, this->SegConnectivity, &(this->segment2NodeID));
		EXSCGraphOrder = this->orderCurveSet(exscStartNID, exscGraph,ref,true);
	}


	// build erasing orders
	if(this->isPoseLinesVisible && this->poseLines.size()>0){
		this->poseEraseGraphOrder = this->findErasingStrokes(poseStartNID, poseGraph,ref);
	}
	if(this->isConvexHullVisible && this->convexHulls2d.size()>0){
		simHullEraseGraphOrder = this->findErasingStrokes(firstSketchStartNID, firstSketchGraph, ref, true);
	}
	if(this->isEdgeContourVisible && this->edgeContours.size()>0){
		SimEdgeEraseGraphOrder = this->findErasingStrokes(simLineStartNID, simLineGraph,ref,false);
	}

	if(this->isPrimVisible && this->PrimitiveLines.size()>0){
		PrimsEraseGraphOrder = this->findErasingStrokes(PrimStartNID, primGraph,ref,true);
	}
	// last phase do not contain easing strokes

	// sum of total drawing strokes
	int totNumOfStrokes=0;

	if(isPoseLinesVisible)	totNumOfStrokes	+=	this->poseGraphOrder.size();
	if(isEdgeContourVisible)	totNumOfStrokes	+=	this->SimEdgeGraphOrder.size();
	if(isConvexHullVisible)	{
		totNumOfStrokes	+= this->convexHulls2d.size(); // global hull is not in Graph order
		// convexHulls2d.size() == numOfGloabHull + simHullGraphOrder.size()
	}
	if(isPrimVisible)	totNumOfStrokes += this->PrimsGraphOrder.size();
	if(this->isSCVisible)	{ totNumOfStrokes += EXSCGraphOrder.size();}

	numOfStrokes = totNumOfStrokes;
	return totNumOfStrokes;
}

/*
startSeg is the first segmentation to be drawn, e.g. the toppest -> head
*/
std::vector<int> imageProc::buidSegOrder( vGraph& skeleton, std::vector<NodeID> startSeg, vGraph* connection , std::vector<NodeID>* seg2nID){
	std::vector<int> segOrder;

	int startSegNum = -1;
	if(startSeg.size() == 1){
		vNode* n = skeleton.g[startSeg[0]];
		if(n->segmentID == -1){
			assert(n->coveredSegments.size()>0);
			startSegNum = n->coveredSegments[0];
		}
		else{
			startSegNum = n->segmentID;
		}
	}
	else if (startSeg.size() == 2){
		vNode* n = skeleton.g[startSeg[0]]->superNode->g[startSeg[1]];
		if(n->segmentID == -1){
			assert(n->coveredSegments.size()>0);
			startSegNum = n->coveredSegments[0];
		}
		else{
			startSegNum = n->segmentID;
		}
	}
	assert(startSegNum != -1);

	vGraph*& connectivity = this->SegConnectivity;
	connectivity->clearFlag();
	NodeID startID = this->segment2NodeID[startSegNum];

	_Graph::adjacency_iterator  vIt, vEnd;

	//BFS visiting
	std::deque<NodeID> q;
	connectivity->g[startID]->visited = true;
	q.push_back(startID);

	while(!q.empty()){
		// TODO: find the max priority node
		// currently: a neighbor of the last node if exist, similar to DFS
		NodeID n = q.back();
		q.pop_back();
		segOrder.push_back(connectivity->g[n]->segmentID);

		// ****************      add neighbors  ***********************************************
		// ==========  direct connected neighbors ======================
		boost::tie(vIt, vEnd) = boost::adjacent_vertices( n, connectivity->g);// TODO: sure using connectivity graph? not the actual ordering graph?
		for(;vIt!=vEnd;vIt++){
			NodeID n2 = *vIt;
			if( connectivity->g[n2]->visited  ){}
			else{
				connectivity->g[n2]->visited = true;
				q.push_back(n2);
			}
		}

		// =============  edges come from spatial relations ===============
		// symmetry: directional, paralell
		// [][0] shows, the next must be [][1]
		for(int i=0;i<this->segmentSymmetricity.size();i++){
			int n_segID = connectivity->g[n]->segmentID;
			if(n_segID == this->segmentSymmetricity[i][0]){
				NodeID n2=this->segment2NodeID[this->segmentSymmetricity[i][1]];
				if( connectivity->g[n2]->visited  ){
					// if already in the to be drawn list, move it the the next one
					for(int j=0;j<q.size();j++){
						if(q[j] == this->segmentSymmetricity[i][1]){
							if(j!=q.size()-1){
								int tmp = q.back();
								q.back() = this->segmentSymmetricity[i][1];
								q[j] = tmp;
							}
							break;
						}
					}
				}
				else{
					connectivity->g[n2]->visited = true;
					q.push_back(n2);
				}
				break;
			}
		}
		


		// =============  indirect neighbors through virtual edges  ========
		//vGraph* prevGraph = skeleton.virtualEdges->prevGraph;
		//if(prevGraph!=NULL){
		//	std::vector<std::vector<NodeID>>& map2prev = skeleton.virtualEdges->map2prev;
		//	for(int i=0;i<map2prev.size();i++){
		//		if(map2prev[i][0] == n){
		//			NodeID virtualParent = map2prev[i][1];
		//			// neighbors of virtual parent
		//			boost::tie(vIt, vEnd) = boost::adjacent_vertices( virtualParent, connectivity->g); // TODO: not sure using connectivity graph
		//			for(;vIt!=vEnd;vIt++){
		//				NodeID n2 = *vIt;
		//				if( connectivity->g[n2]->visited  ){}
		//				else{
		//					connectivity->g[n2]->visited = true;
		//					q.push_back(n2);
		//				}
		//			}
		//		}
		//	}
		//}
	}

	connectivity->clearFlag();
	return segOrder;
}

// order one set
std::vector<int> imageProc::orderCurveSet(std::vector<vCurve>* cSet){
	
	// make a reference line-> the starting line
	// the upperest
	int ind = findStartingIndex(cSet);
	assert(ind!=-1);

	std::vector<int> indexList;
	for(int i=0;i<cSet->size();i++)	indexList.push_back(i);
	std::sort(indexList.begin(),indexList.end(),SortCurveSet<vCurve>(cSet,&(cSet->at(ind))));

	return indexList;
}
std::vector<std::vector<NodeID>> imageProc::orderCurveSet(std::vector<NodeID> start, vGraph& g, vGraph& ref, bool partBypart ){
	// clear voting
	// ref.clearVoting();

	std::vector<std::vector<NodeID>> order;
	std::vector<std::vector<NodeID>> S;
	std::vector<std::vector<NodeID>> T;
	std::vector<std::vector<NodeID>> zero_entropies;
	std::vector<float> all_entropies; // record the entropies of ordered strokes

	if(start.size() == 2){ // super node I case
		// neighbor: lines in the same segmentation

		vNode* start_node = g.g[start[0]]->superNode->g[start[1]];
		if(g.g[start[0]]->segmentID>=0 && start_node->segmentID >= 0){ // when segmentID<0, it has special meaning for ordering: e.g. global hull, not in order graph
			
			double reentro = start_node->updateVoting(ref);// update the voting for the start node
			
			// compute priority
			float _info = (start_node->information != 0)?
				start_node->information: start_node->computeInformation();
			assert (_info !=0 );
			float _c = this->priority( reentro ,_info,start_node);

			order.push_back(start); // first node trace
			all_entropies.push_back(_c);
		}

		// for each segment, find the non-zero entropy strokes
		// and leave all zero entropy strokes to the last;

		//_Graph::vertex_iterator vIt, vEnd;
		//for(tie(vIt,vEnd)=boost::vertices(g.g);vIt != vEnd ;vIt++){
		for(int i=0;i<this->segDrawingOrder.size();i++){
			NodeID nID;

			int seg2bdrawn = segDrawingOrder[i];
			_Graph::vertex_iterator vIt, vEnd;
			bool foundASeg = false;
			for(tie(vIt,vEnd)=boost::vertices(g.g);vIt != vEnd ;vIt++){
				if(g.g[*vIt]->segmentID == seg2bdrawn){
					nID = *vIt;
					foundASeg = true;
					break;
				}
			}
			if(!foundASeg) continue; // only happens for brach component
 
			vNode* n = g.g[nID];
			assert(!n->isLeaf);
			// go to leaves
			_Graph& sng=n->superNode->g;

			// init set S
			// init set T: all inner strokes
			S.clear();
			T.clear();
			_Graph::vertex_iterator vIt2, vEnd2;
			for(tie(vIt2,vEnd2)=boost::vertices(sng);vIt2 != vEnd2 ;vIt2++){
				NodeID nID2 = *vIt2;
				vNode* n2 = sng[nID2];
				assert(n2->isLeaf);

				// separate into S(outter) and T(inner) two groups
				if(n2->getCurve()->isInner){
					std::vector<NodeID> tmp;
					tmp.push_back(nID);
					tmp.push_back(nID2);
					T.push_back(tmp);
				}
				else{
					if(nID != start[0] || nID2 != start[1]){
						std::vector<NodeID> tmp;
						tmp.push_back(nID);
						tmp.push_back(nID2);
						S.push_back(tmp);
					}
				}
			}

			while(S.size()>0){
				// pick max C= delta.E/I
				double max_c = -0.001;
				int max_ind = -1;
				for(int i=0;i<S.size();i++){
					vNode* n = g.g[S[i][0]]->superNode->g[S[i][1]];
					float entro = n->computeCurEntropy(ref);
					float info = (n->information != 0)?n->information: n->computeInformation();
					if (info ==0 ) continue;

					float c = this->priority( entro ,info,n);
					if(c > max_c){
						max_c = c;
						max_ind = i;
					}
				}

				// TODO: check the new choosen
				if(max_c<=0) break;

				// update voting map
				g.g[S[max_ind][0]]->superNode->g[S[max_ind][1]]->updateVoting(ref);
				// add new chosen into order
				order.push_back(S[max_ind]);
				all_entropies.push_back(max_c);

				

				// erase the chosen from S
				S.erase(S.begin()+max_ind);

				// add chosen's neighbor (which are not in S) into S
				// for sim edge, they are all added at the beginning
			}

			// zero entropies
			if(S.size()>0){
				for(int i=0;i<S.size();i++){
					if(partBypart){
						order.push_back(S[i]);
						all_entropies.push_back(0);
					}
					else{
						zero_entropies.push_back(S[i]);
					}
					
				}
			}

			// ordering T set: the inner strokes
			while(T.size()>0){
				// pick max C= delta.E/I
				double max_c = -0.001;
				int max_ind = -1;
				for(int i=0;i<T.size();i++){
					vNode* n = g.g[T[i][0]]->superNode->g[T[i][1]];
					float entro = n->computeCurEntropy(ref);
					float info = (n->information != 0)?n->information: n->computeInformation();
					if (info ==0 ) continue;

					float c = this->priority( entro ,info,n);
					if(c > max_c){
						max_c = c;
						max_ind = i;
					}
				}

				// TODO: check the new choosen
				if(max_c<=0) break;

				// update voting map
				g.g[T[max_ind][0]]->superNode->g[T[max_ind][1]]->updateVoting(ref);
				// add new chosen into order
				order.push_back(T[max_ind]);
				all_entropies.push_back(max_c);
				// erase the chosen from T
				T.erase(T.begin()+max_ind);
			}

			// zero entropies
			if(T.size()>0){
				for(int i=0;i<T.size();i++){
					if(partBypart){
						order.push_back(T[i]);
						all_entropies.push_back(0);
					}
					else{
						zero_entropies.push_back(T[i]);
					}
					
				}
			}
		}

		if(zero_entropies.size()>0){
			

			order.insert(order.end(), zero_entropies.begin(), zero_entropies.end());
			// put in the zeros
			for(int i=0;i<zero_entropies.size();i++){
				all_entropies.push_back(0);
			}
		}
	}
	else{ // error case
		assert(0);
	}

	return order;
}
inline double imageProc::priority(double _entro, double _info, vNode* _n){
	double repri =0;
	assert(_n->isLeaf);
	double len = _n->getCurve()->Length();
	repri = _entro / _info * len;
	if(repri == 0){
		int dummy = 38941;
	}
	return repri;
}

// erasing strokes
// similar to entropy
// but with a very small float vNode::fieldSize;
std::vector<std::vector<NodeID>> imageProc::findErasingStrokes(std::vector<NodeID> start, vGraph& g, vGraph& ref, bool partBypart){
	// clear voting
	ref.clearVoting();

	float erasingFieldSize = vNode::fieldSize;
	vNode::fieldSize = 3;

	std::vector<std::vector<NodeID>> order;
	std::vector<std::vector<NodeID>> S;
	std::vector<std::vector<NodeID>> zero_entropies;
	std::vector<float> all_entropies; // record the entropies of ordered strokes

	if(start.size() == 2){ // super node I case
		// neighbor: lines in the same segmentation

		vNode* start_node = g.g[start[0]]->superNode->g[start[1]];
		if(g.g[start[0]]->segmentID>=0 && start_node->segmentID >= 0){ // when segmentID<0, it has special meaning for ordering: e.g. global hull, not in order graph
			
			// update the voting for the start node
			double reentro = start_node->updateVoting(ref);
			// compute priority
			float _info = (start_node->information != 0)?
				start_node->information: start_node->computeInformation();
			assert (_info !=0 );
			float _c = this->priority( reentro ,_info,start_node);

			
			if(_c == 0){ zero_entropies.push_back(start);}
			else {
				order.push_back(start); // first node trace
				all_entropies.push_back(_c);
			}
			
		}
		// for each segment, find the non-zero entropy strokes
		// and leave all zero entropy strokes to the last;

		//_Graph::vertex_iterator vIt, vEnd;
		//for(tie(vIt,vEnd)=boost::vertices(g.g);vIt != vEnd ;vIt++){
		for(int i=0;i<this->segDrawingOrder.size();i++){
			NodeID nID;

			int seg2bdrawn = segDrawingOrder[i];
			_Graph::vertex_iterator vIt, vEnd;
			bool foundASeg = false;
			for(tie(vIt,vEnd)=boost::vertices(g.g);vIt != vEnd ;vIt++){
				if(g.g[*vIt]->segmentID == seg2bdrawn){
					nID = *vIt;
					foundASeg = true;
					break;
				}
			}
			if(!foundASeg) continue; // only happens for brach component
 
			vNode* n = g.g[nID];
			assert(!n->isLeaf);
			// go to leaves
			_Graph& sng=n->superNode->g;

			// init set S
			S.clear();
			_Graph::vertex_iterator vIt2, vEnd2;
			for(tie(vIt2,vEnd2)=boost::vertices(sng);vIt2 != vEnd2 ;vIt2++){
				NodeID nID2 = *vIt2;
				vNode* n2 = sng[nID2];
				assert(n2->isLeaf);
				
				if(nID != start[0] || nID2 != start[1]){
					std::vector<NodeID> tmp;
					tmp.push_back(nID);
					tmp.push_back(nID2);
					S.push_back(tmp);
				}
			}

			while(S.size()>0){
				// pick max C= delta.E/I
				double max_c = -0.001;
				int max_ind = -1;
				for(int i=0;i<S.size();i++){
					vNode* n = g.g[S[i][0]]->superNode->g[S[i][1]];
					float entro = n->computeCurEntropy(ref);
					float info = (n->information != 0)?n->information: n->computeInformation();
					if (info ==0 ) continue;

					float c = this->priority( entro ,info,n);
					if(c > max_c){
						max_c = c;
						max_ind = i;
					}
				}

				// TODO: check the new choosen
				if(max_c<=0) break;

				// update voting map -- do not update when computing erasing entropy
				// g.g[S[max_ind][0]]->superNode->g[S[max_ind][1]]->updateVoting(ref);
				// add new chosen into order
				order.push_back(S[max_ind]);
				all_entropies.push_back(max_c);

				// try set color for strokes of entropies < threshold
				if(max_c<100){
					vNode* n = g.g[S[max_ind][0]]->superNode->g[S[max_ind][1]];
					n->getCurve()->setColor(255,0,0);
				}

				// erase the chosen from S
				S.erase(S.begin()+max_ind);

				// add chosen's neighbor (which are not in S) into S
				// for sim edge, they are all added at the beginning
			}

			// zero entropies
			if(S.size()>0){
				for(int i=0;i<S.size();i++){
					if(partBypart){
						order.push_back(S[i]);
						all_entropies.push_back(0);

						// color code the zero entropy strokes
						vNode* n = g.g[S[i][0]]->superNode->g[S[i][1]];
						n->getCurve()->setColor(0,0,255);
					}
					else{
						zero_entropies.push_back(S[i]);
					}
					
				}
			}
		}

		if(zero_entropies.size()>0){
			// color code the zero entropy strokes
			for(int i=0;i<zero_entropies.size();i++){
				vNode* n = g.g[zero_entropies[i][0]]->superNode->g[zero_entropies[i][1]];
				n->getCurve()->setColor(0,0,255);
			}

			order.insert(order.end(), zero_entropies.begin(), zero_entropies.end());
			// put in the zeros
			for(int i=0;i<zero_entropies.size();i++){
				all_entropies.push_back(0);
			}
		}
	}
	else{ // error case
		assert(0);
	}


	vNode::fieldSize = erasingFieldSize;
	return order;
}

int imageProc::findStartingIndex(std::vector<vCurve>* cSet){
	int h=INT_MAX;
	int reInd=-1;
	for(int i=0;i<cSet->size(); i++){
		for(int j=0;j<cSet->at(i).size();j++){
			if(cSet->at(i).curve2d[j].y()<h){
				h = cSet->at(i).curve2d[j].y();
				reInd = i;
			}
		}
	}

	return reInd;
}
// find the node with max E/I=C
std::vector<NodeID> imageProc::findStartNode(vGraph& g, vGraph& ref){
	// clear voting
	ref.clearVoting();

	NodeID start= -1;
	NodeID start2= -1; // for two level clustered graph
	float max_c = 0;
	std::vector<float> entroList;
	std::vector<float> infoList;
	std::vector<float> cList;
	using namespace boost;
	_Graph::vertex_iterator vIt, vEnd;
	for(tie(vIt,vEnd)=boost::vertices(g.g);vIt != vEnd ;vIt++){
		NodeID nID = *vIt;
		vNode* n = g.g[nID];
		if(n->isLeaf){ // n is a leaf node
			float entro = n->computeCurEntropy(ref);
			float info = n->computeInformation();
			//assert(info!=0);
			if (info ==0 ) continue;
			float c = this->priority( entro ,info,n);
			if(c > max_c){
				max_c = c;
				start = nID;
			}
			entroList.push_back(entro);
			infoList.push_back(info);
			cList.push_back(c);
		}
		else{ // n is a super node I
			_Graph::vertex_iterator vIt2, vEnd2;
			_Graph& sng = n->superNode->g;
			for(tie(vIt2,vEnd2)=boost::vertices(sng);vIt2 != vEnd2 ;vIt2++){
				NodeID nID2 = *vIt2;
				vNode* n2 = sng[nID2];
				assert(n2->isLeaf);
				
				float entro = n2->computeCurEntropy(ref);
				float info = n2->computeInformation();
				//assert(info!=0);
				if (info ==0 ) continue;

				float c = this->priority( entro ,info,n2);
				if(c > max_c){
					max_c = c;
					start = nID;
					start2 = nID2;
				}
				entroList.push_back(entro);
				infoList.push_back(info);
				cList.push_back(c);
			}
		}
	}

	if(start == -1){
		assert(0);
	}
	assert(start != -1);
	
	std::vector<NodeID> trace;
	trace.push_back(start);
	if(start2!=-1)
		trace.push_back(start2);
	return trace;
}
std::vector<NodeID> imageProc::findTopNode(vGraph& g){
	
	NodeID start= -1;
	NodeID start2= -1; // for two level clustered graph
	int top_h = INT_MAX;
	using namespace boost;
	_Graph::vertex_iterator vIt, vEnd;
	for(tie(vIt,vEnd)=boost::vertices(g.g);vIt != vEnd ;vIt++){
		NodeID nID = *vIt;
		vNode* n = g.g[nID];
		if(n->isLeaf){ // n is a leaf node
			vCurve* c = n->getCurve();
			for(int i=0;i<c->curve2d.size();i++){
				if(c->curve2d[i].y() < top_h){
					top_h = c->curve2d[i].y();
					start = nID;
				}
			}
		}
		else{ // n is a super node I
			_Graph::vertex_iterator vIt2, vEnd2;
			_Graph& sng = n->superNode->g;
			for(tie(vIt2,vEnd2)=boost::vertices(sng);vIt2 != vEnd2 ;vIt2++){
				NodeID nID2 = *vIt2;
				vNode* n2 = sng[nID2];
				assert(n2->isLeaf);
				
				vCurve* c = n2->getCurve();
				for(int i=0;i<c->curve2d.size();i++){
					if(c->curve2d[i].y() < top_h){
						top_h = c->curve2d[i].y();
						start = nID;
						start2 = nID2;
					}
				}
			}
		}
	}

	if(start == -1){
		assert(0);
	}
	assert(start != -1);
	
	std::vector<NodeID> trace;
	trace.push_back(start);
	if(start2!=-1)
		trace.push_back(start2);
	return trace;

}

double imageProc::distance(vCurve* c1, vCurve* c2){

	return euclideanDistance(c1,c2);

}
double imageProc::euclideanDistance(vCurve* c1, vCurve* c2){
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
double imageProc::hausdorffDiscance(vCurve* p, vCurve* q){

	double forward = directedHausdorffDiscance(p, q);
	double backward = directedHausdorffDiscance(q, p);
 
	return MAX(forward, backward);
}
double imageProc::directedHausdorffDiscance(vCurve* c1, vCurve* c2){

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

void imageProc::addGraphEdgesWRTDistances(vGraph& g, std::vector<NodeID>& nodeids){
	// add neighbors to each node (kNN)
	// add edges into graph
	for(int i=0;i<nodeids.size();i++){
		std::vector<int> indexList;
		for(int j=0;j<nodeids.size();j++)	{indexList.push_back(j);}
		std::sort(indexList.begin(),indexList.end(),SortNodeIDwrtDistance<NodeID>(g.g, &nodeids,&(nodeids[i]) ));

		int kNN=3;
		for(int j=0;j<kNN && j<indexList.size();j++){
			NodeID u = nodeids[i];
			NodeID v = nodeids[indexList[j]];
			if(u == v){
				kNN +=1;
			}
			else{
				vEdge* e = new vEdge();
				e->from = g.g[u];
				e->to = g.g[v];
				g.addEdge( nodeids[i], nodeids[indexList[j]],e);
			}
		}
	}

	return;
}

void imageProc::addSuperEdgesWRTConnectivity(vGraph& g, vGraph* connection){
	return;
}
bool imageProc::chkSegmentConnectByID(int segid1, int segid2, vGraph* connection){
	bool isConnect = false;

	_Graph& _g = connection->g;
	_Graph::edge_iterator eIt, eEnd;
	boost::tie(eIt, eEnd) = boost::edges(_g);
	for(; eIt!=eEnd; ++eIt){
		vEdge* e = _g[*eIt];
		NodeID u = boost::source(*eIt, _g);
		NodeID v = boost::target(*eIt, _g);
		if ( (_g[u]->segmentID == segid1 && _g[v]->segmentID == segid2) 
			|| (_g[u]->segmentID == segid2 && _g[v]->segmentID == segid1)){ // there is an edge in the connection graph
			return true;
		}
	}
	return isConnect;

}


vec2 imageProc::projDir3(vec v){
	GLdouble pos3D_x, pos3D_y, pos3D_z;
	gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
	vec2 tmp = vec2(pos3D_x, pos3D_y);
	gluProject(0, 0 , 0,
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
	vec2 tmp2 = vec2(pos3D_x, pos3D_y);
	return tmp-tmp2;
}
vec imageProc::projVec3(vec v){
	GLdouble pos3D_x, pos3D_y, pos3D_z;
	gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
	return vec(pos3D_x, pos3D_y,pos3D_z);
}

/**********************************************************

					edge contours

***********************************************************/
/**************************
// clear edge contours 
// then add _edge into edge contour
// so every gl repaint happends, repaint the edge curves
**************************/
void imageProc::addEdgeContourChains(std::vector<std::vector<int>> * _chains, const std::vector<vec>* vertices, vec lighting)
{
	this->edgeContours.clear();
	for(int i=0; i<_chains->size();i++){
		
		std::vector<int>& c = _chains->at(i);
		if(c.size()>5){ // ignore small curves
			vCurve newC;
			for(int k=0; k<c.size();k++ ){
				double v[3];
				for(int j=0; j<3; j++){
					v[j] =vertices->at( c.at(k))[j];
				}
			
				GLdouble pos3D_x, pos3D_y, pos3D_z;
				gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);

				newC.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y,c[k]));
			}

			// set intensity
			vec p1=vertices->at(newC.curve2d.back().get3DIndex());
			vec p2=vertices->at(newC.curve2d.front().get3DIndex());
			vec tan = p1-p2;
			float _intense = len((normalize(tan)) CROSS (lighting));
			for(int k=0;k<newC.size();k++){
				newC.curve2d[k].intensity = _intense;
			}

			this->edgeContours.push_back(newC);
		}
	}
	return;
}
void imageProc::splitEdgeContourChains(){
	// find max concavity
	double max_color = -1;
	for(int i=0; i<this->edgeContours.size(); i++){
		vCurve* c = & (edgeContours[i]);
		if(c->size() < 3) continue;
		// get the second derivative's absolute value -> split according to concavity
		//c->computeTangentAndConcavity();
		//int tmp_laplacian_kernel = 5;//c->size()*(1-0.618);
		//tmp_laplacian_kernel=(tmp_laplacian_kernel>15)?15:tmp_laplacian_kernel;
		c->computeLoG(3, 3);

		//std::vector<double> d2;
		for (int j=0; j< c->size();j++  ){
			double con0 = c->curve2d[j].concavity()[0];
			double con1 = c->curve2d[j].concavity()[1];
			//d2.push_back(con0*con0 + con1*con1);
			double con = con0*con0 + con1*con1 ;
			if(con > max_color ) max_color = con;
		}
	}
	max_color=25;
	if(true){
		for(int i=0; i<this->edgeContours.size(); i++){
			vCurve* c = & (edgeContours[i]);
			if(c->size() < 5) continue;
			//std::vector<double> d2;
			for (int j=0; j< c->size();j++  ){
				double con0 = c->curve2d[j].concavity()[0];
				double con1 = c->curve2d[j].concavity()[1];
				//d2.push_back(con0*con0 + con1*con1);
				double con = con0*con0 + con1*con1 ;
				double red_element=con/(double)max_color*255;
				double blue_element=(1-con/(double)max_color)*255;
				c->curve2d[j].setColor(red_element, 0, blue_element);
			}
		}
	}
	if(true){
		std::vector<vCurve> splited;
		
		for(int i=0; i<this->edgeContours.size(); i++){
			vCurve* c = & (edgeContours[i]);
			if(c->size() < 5) continue;
			// get the second derivative's absolute value -> split according to concavity
			//c->computeTangentAndConcavity();
		
			int headOfNew = 0;
			for (int j=0; j< c->size();j++  ){
				double con0 = c->curve2d[j].concavity()[0];
				double con1 = c->curve2d[j].concavity()[1];
				double con = con0*con0 + con1*con1 ;

				//if(c->curve2d[j].color[0]>= 50){
				if(con > splitThreshold){
					// split and add to splited
					vCurve newC;
					int tmp_head = headOfNew;
					for(int k=headOfNew; k<=j; k++, headOfNew++){
						newC.addPoint( c->curve2d[k]);
					}
					if(newC.size()>3){splited.push_back(newC);} // this will ignore the adjacent short high score splitting points
					else{
						headOfNew = tmp_head;
					}
				}else{
					// continue
				}
			}
			if(headOfNew == 0){
				int a =0;
			}
			if(headOfNew < c->size()-1){
				vCurve newC;
				for(int k=headOfNew; k< c->size(); k++){
					newC.addPoint( c->curve2d[k]);
				}
				if(newC.size()>3){splited.push_back(newC);}
			}
		}

		this->edgeContours = splited;
	}
	return;
}
/**************************
recursively spliting until under a threshold
TODO: multi-level thresholding -> generate smaller lines
**************************/
void imageProc::splitEdgeContourChainsRecursively(){
	//for(int i=0; i<this->edgeContours.size(); i++){
	//	vCurve* c = & (edgeContours[i]);
	//	if(c->size() < 3) continue;
	//	c->computeLoG(3, 3); // end up not using LoG at all -> curvature is one unrealiable bastered
	//}
	
	this->splitVertexChainsRecursively(this->edgeContours,0.5*this->splitThreshold);

	return;

}
void imageProc::splitVertexChainsRecursively(std::vector<vCurve>& vertexChain, double error_tolerance){
	std::vector<vCurve> splitted;
		
	for(int i=0; i<vertexChain.size(); i++){
		vCurve* c = & (vertexChain[i]);
		std::vector<vCurve> re = straghtLineApproximation(c,error_tolerance );
		splitted.insert(splitted.end(),re.begin(), re.end());
	}
	vertexChain = splitted;

	return;
}
std::vector<vCurve> imageProc::straghtLineApproximation(vCurve* curve, double error_tolerance){

	std::vector<vCurve> splitted;
	// do not like small curves
	if(curve->size()<6 ) {
		splitted.push_back(*curve);
		return splitted;
	}

	int splitting_pos = -1;
	double error = this->straightLineError(curve, splitting_pos);

	// split
	if(error > error_tolerance){
		vCurve new1, new2;
		assert(splitting_pos != -1);

		for(int i=0; i<=splitting_pos ;i++ ){
			new1.addPoint(curve->curve2d[i]);
		}
		for(int i=splitting_pos; i< curve->size(); i++){ // not throw away the splitting point, avoid infinite recursion when reture the split value
			new2.addPoint(curve->curve2d[i]);
		}
		std::vector<vCurve> re1 = straghtLineApproximation( & new1,error_tolerance);
		std::vector<vCurve> re2 = straghtLineApproximation( & new2,error_tolerance);

		splitted.insert(splitted.end(),re1.begin(), re1.end());
		splitted.insert(splitted.end(),re2.begin(), re2.end());
	}
	// not split
	else{
		splitted.push_back(*curve);
	}

	return splitted;
}
double imageProc::straightLineError(vCurve* c, int& split_ind){
	if(c->size()<6) return 0;
	
	double error = 0;
	double max_con = -1;
	split_ind = -1; // potential splitting point

	vec2 start(c->curve2d[0].x(), c->curve2d[0].y());
	vec2 end(c->curve2d.back().x(), c->curve2d.back().y());
	// normal of the line
	vec2 normal(-(end-start)[1],(end-start)[0]);
	
	if(len(normal)==0){ // probably close loop!!!, use the middle point for initialization
		vec2 mid (c->curve2d[c->curve2d.size()/2].x(), c->curve2d[c->curve2d.size()/2].y());

		normal[0] = -(mid-start)[1];
		normal[1] = (mid-start)[0];
	}
	//assert(len(normal)>0); // TODO: if hit again, the curve is seriesly strange, may need reparameterization of curves
	if(len(normal)==0)
		return 0;
	normal /= len(normal);

	for(int i=0; i< c->size(); i++){
		vec2 p(c->curve2d[i].x(), c->curve2d[i].y());
		double ortho = (start -p)^normal;
		ortho *= ortho;
		error += ortho;
		//double con0= c->curve2d[i].concavity()[0];
		//double con1= c->curve2d[i].concavity()[1];
		//double con = con0*con0 + con1*con1;
		if(ortho > max_con && splitConstrain(i,c->size())){
			max_con = ortho;
			split_ind = i;
		}
	}

	if(split_ind == -1) return 0;// no satisfying splitting positions
	error = sqrt(error);
	double length = len(start-end);//c->size(); 
	error /= length;
	return error;
}
bool imageProc::splitConstrain(int tentative_pos, int curve_size){
	if(tentative_pos <6) return false;
	if((curve_size-tentative_pos) < 6) return false;
	if(tentative_pos < curve_size*0.2) return false;
	if((curve_size-tentative_pos) < curve_size*0.2) return false;
	return true;
}

void imageProc::visvalingamSplitting(std::vector<vCurve>& vertexChain, double error_tolerance){
	std::vector<vCurve> splitted;

	for(int i=0; i<vertexChain.size(); i++){
		vCurve* c = & (vertexChain[i]);
		std::vector<vCurve> re = visvalingamAlgorithm(c,error_tolerance );
		splitted.insert(splitted.end(),re.begin(), re.end());
	}

	vertexChain = splitted;
}
std::vector<vCurve> imageProc::visvalingamAlgorithm(vCurve* curve, double error_tolerance){
	std::vector<vCurve> splitted;

	std::vector<bool> selected;
	selected.resize(curve->size());
	for(int i=0;i<selected.size();i++) selected[i]=true;

	// filter perceptually unimportant points
	// list V<ind, area>
	// remove the min V if 

	return splitted;
}

void imageProc::buildSimEdgeGraph(vGraph& g, std::vector<int>* pointColor){
	if(pointColor->size() == 0) {this->useGraph=false;return;} // segmentation is not ready, do not construct the graph
	
	this->useGraph = true;
	g.clear();
	std::vector<vNode*> leaves;
	for(int i=0;i<this->edgeContours.size();i++){

		// ignore small curves (after projected)
		vCurve& edgec = edgeContours[i];
		if( dist(edgec.curve2d.front().toVec2(), edgec.curve2d.back().toVec2())<8) continue;

		vNode* n = new vNode();
		edgeContours[i].type = vCurve::STRAIGHT;
		edgeContours[i].setTexture(this->texture);
		edgeContours[i].removeRedundancy();
		n->setCurve(&(this->edgeContours[i]));
		std::vector<vPoint2D>& c = edgeContours[i].curve2d;
		int maxSegNum=-1;
		for(int j=0;j<c.size();j++){
			int seg = c[j].segment = pointColor->at(c[j].get3DIndex());
			
			if(seg>maxSegNum) maxSegNum = seg;
			bool _exist=false;
			for(int k=0;k<n->coveredSegments.size();k++){
				if(n->coveredSegments[k] == seg){
					_exist = true;
					break;
				}
			}
			if(! _exist){
				n->coveredSegments.push_back(seg);
			}
		}
		//g.addNode(n);
		
		// find the principal segment#
		std::vector<int>& segs = n->coveredSegments;
		assert(segs.size() >0);
		if(segs.size() == 1){
			n->segmentID = segs[0];
		}
		else{
			// move principal seg# to segs[0], and set segmentID = principal seg#
			std::vector<double> lens;
			lens.resize(maxSegNum+1);
			for(int j=0;j<lens.size();j++) lens[j]=0;
			for(int j=0;j<c.size()-1;j++){
				double len = dist(c[j].toVec2(), c[j+1].toVec2());
				lens[c[j].segment] += len/2;
				lens[c[j+1].segment] += len/2;
			}

			int maxInd=-1; double maxLen =-1;
			for(int j=0;j<lens.size();j++){
				if(lens[j]>maxLen){
					maxInd = j;
					maxLen = lens[j];
				}
			}

			if(segs[0] != maxInd){// swap
				int tmp = segs[0];
				segs[0] = maxInd;
				for(int j=0;j<segs.size();j++){
					if(segs[j] == maxInd){
						segs[j] = tmp;
						break;
					}
				}
			}

			n->segmentID = maxInd;
		}

		leaves.push_back(n);
	}

	// Node type I - segment level supernode
	// organize the leaf nodes w.r.t their principal segment#
	std::vector<std::vector<int>> superNodesI;
	superNodesI.resize(30);
	for(int i=0; i<leaves.size(); i++){
		int pSegNum = leaves[i]->segmentID;
		if(pSegNum >= superNodesI.size()){ // size checking
			superNodesI.resize(pSegNum+2);
		}
		superNodesI[pSegNum].push_back(i);
	}

	// map seg# to super node ID, for adding super edges
	std::vector<NodeID> seg2nodeID_simEdge;
	std::vector<bool> nodeExist_simEdge;
	seg2nodeID_simEdge.resize(superNodesI.size());
	nodeExist_simEdge.resize(superNodesI.size());

	// create  super nodes
	for(int i=0;i<superNodesI.size();i++){
		nodeExist_simEdge[i]=false;
		if(superNodesI[i].size()>0){
			vNode* superN = new vNode();
			superN->isLeaf = false;
			superN->superNode = new vGraph();

			std::vector<NodeID> nodeids; // for sorting and add edges

			for(int j=0;j<superNodesI[i].size();j++){
				NodeID nID = superN->superNode->addNode(leaves[superNodesI[i][j]]);
				nodeids.push_back(nID);
			}

			this->addGraphEdgesWRTDistances(*(superN->superNode), nodeids);

			superN->segmentID = i;
			NodeID nID = g.addNode(superN);

			// for super edge
			seg2nodeID_simEdge[i] = nID;
			nodeExist_simEdge[i] = true;
		}
	}

	// ======  add super edges ============
	vGraph*& connectivity = this->SegConnectivity;
	connectivity->clearFlag();
	_Graph::vertex_iterator vStart, vTail;
	boost::tie(vStart, vTail) = boost::vertices(g.g); // any start ID is fine
	NodeID startID = *vStart;

	//BFS visiting
	std::deque<NodeID> q;
	connectivity->g[startID]->visited = true;
	q.push_back(startID);
	_Graph::adjacency_iterator  vIt, vEnd;
	while(!q.empty()){
		NodeID n = q.back();
		q.pop_back();
		// add neighbors
		// direct neighbors
		boost::tie(vIt, vEnd) = boost::adjacent_vertices( n, connectivity->g);
		for(;vIt!=vEnd;vIt++){
			NodeID n2 = *vIt;
			// make sure the edge is only added once between two nodes (no paralell edges)
			if( connectivity->g[n2]->visited  ){}
			else{
				connectivity->g[n2]->visited = true;
				q.push_back(n2);

				int segNum1 = connectivity->g[n]->segmentID;
				int segNum2 = connectivity->g[n2]->segmentID;

				if(nodeExist_simEdge[segNum1] && nodeExist_simEdge[segNum2]){
					NodeID u=seg2nodeID_simEdge[segNum1];
					NodeID v=seg2nodeID_simEdge[segNum2];

					vEdge* e=new vEdge();
					e->from = g.g[u];
					e->to = g.g[v];
					g.addEdge(u,v,e);
				}
			}
		}
	}
	connectivity->clearFlag();
	return;
}


/**********************************************************

					pose Lines

***********************************************************/
void imageProc::addPoseLine(std::vector<std::vector<vec>> *  _chains){
	this->poseLines.clear();
	for(int i=0; i<_chains->size();i++){
		std::vector<vec>& c = _chains->at(i);
		if(true){ // set constrains on each line, or not
			vCurve newC;
			for(int k=0; k<c.size();k++ ){
				double v[3];
				for(int j=0; j<3; j++){
					v[j] =c[k][j];
				}
			
				GLdouble pos3D_x, pos3D_y, pos3D_z;
				gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
				newC.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y));
			}
			this->poseLines.push_back(newC);
		}
	}

	this->buildPoseGraph(this->graphs[0]);
	return;
}
void imageProc::buildPoseGraph(vGraph& g){
	// clear the graph
	g.clear();

	// std::vector<NodeID> nodeids;
	// set the graph
	for(int i=0;i<this->poseLines.size(); i++){
		vNode* n = new vNode();
		n->setCurve(&(this->poseLines[i]));
		poseLines[i].type = vCurve::OPEN;
		poseLines[i].setTexture(this->texture);
		poseLines[i].removeRedundancy();
		std::vector<vPoint2D>& c = poseLines[i].curve2d;
		for(int j=0;j<c.size();j++){
			int seg = c[j].segment;

			bool _exist=false;
			for(int k=0;k<n->coveredSegments.size();k++){
				if(n->coveredSegments[k] == seg){
					_exist = true;
					break;
				}
			}
			if(! _exist){
				n->coveredSegments.push_back(seg);
			}
		}
		// TODO: better stratege to assign segmentID
		n->segmentID = n->coveredSegments.back();
		
		// create a supernode to contain each new node

		vNode* sn = new vNode( n->coveredSegments.back());
		sn->coveredSegments = n->coveredSegments;
		sn->isLeaf = false;
		sn->superNode = new vGraph();
		sn->superNode->addNode(n);
		g.addNode(sn);
		// nodeids.push_back(nID);
	}

	// this->addGraphEdgesWRTDistances(g, nodeids);

	// add super-edge
	// if the two super-nodes have adjacent segments
	using namespace boost;
	_Graph& _g= g.g;
	_Graph::vertex_iterator vIt, vEnd, vChk;
	for(tie(vIt,vEnd)=vertices(_g);vIt != vEnd ;vIt++){
		vChk = vIt;
		vChk++;
		for(; vChk!= vEnd; vChk++){
			// 1. at least one of the two super nodes is spine
			// 2. if two nodes contains a same segment(ID)
			vNode* n1=_g[*vIt];
			vNode* n2=_g[*vChk];

			bool foundID=false; // determine if two nodes are connected meshly
			if(n1->coveredSegments.size() ==1  && n2->coveredSegments.size() == 1){
				// needs to consult the connectivity
				int id1 = n1->coveredSegments[0];
				int id2 = n2->coveredSegments[0];

				foundID = (id1 == id2); // both part of the same spine
				if(!foundID){
					foundID = this->chkSegmentConnectByID(id1,id2,this->SegConnectivity);
				}
			
			}
			else if(n1->coveredSegments.size() ==1 ){
				int id=n1->coveredSegments[0];
				// verify if id exist in n2
				for(int i=0;i<n2->coveredSegments.size();i++){
					if(n2->coveredSegments[i] == id){ // they are indeed neighbors
						foundID = true;
						break;
					}
				}
			}
			else if(n2->coveredSegments.size() == 1){
				int id = n2->coveredSegments[0];
				// verify if id exist in n2
				for(int i=0;i<n1->coveredSegments.size();i++){
					if(n1->coveredSegments[i] == id){ // they are indeed neighbors
						foundID = true;
						break;
					}
				}
			}

			if(foundID){
				// add a super edge between  vIt and vChk
				vEdge* e = new vEdge();
				NodeID u = *vIt;
				NodeID v = *vChk;
				e->from = _g[u];
				e->to = _g[v];
				g.addEdge(u,v, e);
			}
			else{}// do nothing
			

		}
	}
	return;
}
void imageProc::addPoseLine(std::vector<std::vector<vPoint3D>> *  _chains){
	this->poseLines.clear();
	for(int i=0; i<_chains->size();i++){
		std::vector<vPoint3D>& c = _chains->at(i);
		if(true){ // set constrains on each line, or not
			vCurve newC;
			for(int k=0; k<c.size();k++ ){
				double v[3];
				for(int j=0; j<3; j++){
					v[j] =c[k].v[j];
				}
			
				GLdouble pos3D_x, pos3D_y, pos3D_z;
				gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);

				vPoint2D p = vPoint2D(pos3D_x,this->h - pos3D_y, c[k].segment,false);
				p.intensity = 1;
				newC.addPoint(p);
			}
			this->poseLines.push_back(newC);
		}
	}

	this->buildPoseGraph(this->graphs[0]);
	return;
}

/**********************************************************

					 2D convex hull

***********************************************************/
void imageProc::addConvexHullLines(std::vector<std::vector<vec2>> *  _chains){
	this->convexHulls2d.clear();
	//for(int i=0; i<_chains->size();i++){
	//	std::vector<vec2> c = _chains->at(i);
	//	if(true){ // set constrains on each line, or not
	//		vCurve newC;
	//		for(int j=0; j<c.size();j++ ){
	//			newC.addPoint(vPoint2D(c[j][0],c[j][1]));
	//		}
	//		this->convexHulls2d.push_back(newC);
	//	}
	//}

	std::vector<std::vector<vec2>> simHulls = this->buildSimHull(_chains);
	for(int i=0;i<simHulls.size();i++){
		vCurve newC;
		for(int j=0;j<simHulls[i].size();j++){
			newC.addPoint(vPoint2D(simHulls[i][j][0],simHulls[i][j][1]));
		}
		this->convexHulls2d.push_back(newC);
	}

	return;
}

std::vector<std::vector<vec2>> imageProc::buildSimHull(std::vector<std::vector<vec2>> * _chains, double frac ){
	 using namespace cv;
	 using namespace std;


	std::vector<std::vector<cv::Point>> cvHulls;
	for(int i=0;i< _chains->size();i++){
		std::vector<cv::Point> cvHull;
		for(int j=0;j<_chains->at(i).size();j++ ){
			cvHull.push_back(cv::Point(_chains->at(i)[j][0], _chains->at(i)[j][1]));
		}
		std::vector<cv::Point> poly;

		double epsilon = cv::arcLength( cv::Mat(cvHull),true) * frac;
		cv::approxPolyDP( cv::Mat(cvHull), poly, epsilon, false);
		cvHulls.push_back( poly);
	}

	// type conversion
	std::vector<std::vector<vec2>> simHulls;
	for(int i=0;i<cvHulls.size();i++){
		std::vector<vec2> vecHull;
		for(int j=0;j<cvHulls[i].size();j++){
			vecHull.push_back(vec2(cvHulls[i][j].x,cvHulls[i][j].y));
		}
		simHulls.push_back(vecHull);
	}

	return simHulls;
}

void imageProc::addConvexHullLinesAndBuildSimHullGraph
	(std::vector<std::vector<vec2>> *  _chains, vGraph& g, std::vector<std::vector<int>> branches,bool usebranch  ){

	
	if(usebranch){
		if(branches.size() == 0) return;
		this->convexHulls2d.clear();
		g.clear();

		std::vector<std::vector<vec2>> simHulls = this->buildSimHull(_chains);
		assert((branches.size() + 1) == simHulls.size());

		std::vector<int> curvesInBranch; // number of curve in branch
		
		for(int i=0;i<simHulls.size();i++){

			// create a threshold for sim hull, only keep a few salient edge
			float hull_thres=0;
			for(int j=0;j+1 <simHulls[i].size();j++){
				float len = dist(simHulls[i][j],simHulls[i][j+1]);
				if(len > hull_thres) hull_thres = len;
			}

			hull_thres *= 1-0.618; // magic number

			int cnt_numOfCurv = 0;
			for(int j=0;j+1<simHulls[i].size();j++){
				float len = dist(simHulls[i][j],simHulls[i][j+1]);
				if(len>hull_thres){
					vCurve newC;
					newC.addPoint(vPoint2D(simHulls[i][j][0],simHulls[i][j][1]));
					newC.addPoint(vPoint2D(simHulls[i][j+1][0],simHulls[i][j+1][1]));
					newC.type = vCurve::STRAIGHT;
					this->convexHulls2d.push_back(newC);
					cnt_numOfCurv++;
				}
			}
			curvesInBranch.push_back(cnt_numOfCurv);
		}

		int cnt_cur_curve = 0; // central control of vNode's curve setting

		{ // global hull
			vNode* sn = new vNode();
			sn->isLeaf = false;
			sn->superNode = new vGraph();
			for(int i=0;i<this->segment2NodeID.size();i++){
				sn->coveredSegments.push_back(i); 
			}
			sn->segmentID = -2;
			std::vector<NodeID> nodeids;
			for(int i=0;i<curvesInBranch[0] ;i++, cnt_cur_curve++){
				vNode* n = new vNode();

				vCurve& curv = this->convexHulls2d[cnt_cur_curve];
				curv.type = vCurve::STRAIGHT;
				curv.setTexture(this->texture);
				curv.removeRedundancy();
				n->setCurve(&curv);
				n->coveredSegments = branches[0];
				n->segmentID = n->coveredSegments[0];

				NodeID nID = sn->superNode->addNode(n);
				nodeids.push_back(nID);
			}

			numOfGloabHull = cnt_cur_curve;
			this->addGraphEdgesWRTDistances( *(sn->superNode), nodeids); // add edges
			g.addNode(sn);
		}

		// branch hulls
		for(int i=1;i<simHulls.size();i++){
			vNode* sn = new vNode();
			sn->isLeaf = false;
			sn->superNode = new vGraph();
			sn->coveredSegments = branches[i-1];
			sn->segmentID = sn->coveredSegments[0];

			std::vector<NodeID> nodeids;
			for(int j=0;j<curvesInBranch[i];j++, cnt_cur_curve++){
				vNode* n = new vNode();

				vCurve& curv = this->convexHulls2d[cnt_cur_curve];
				curv.type = vCurve::STRAIGHT;
				curv.setTexture(this->texture);
				curv.removeRedundancy();
				n->setCurve(&curv);
				n->coveredSegments = branches[i-1];
				n->segmentID = n->coveredSegments[0];

				NodeID nID = sn->superNode->addNode(n);
				nodeids.push_back(nID);
			}
			this->addGraphEdgesWRTDistances( *(sn->superNode), nodeids); // add edges
			g.addNode(sn);
		}
	}
	else{
		this->convexHulls2d.clear();
		g.clear();

		std::vector<std::vector<vec2>> simHulls = this->buildSimHull(_chains, 0.02);

		std::vector<int> curvesInBranch; // number of curve in branch
		
		{ // global hull curves
			// // create a threshold for sim hull, only keep a few salient edge
			//int i=0;
			//float hull_thres=0;
			//for(int j=0;j+1 <simHulls[i].size();j++){
			//	float len = dist(simHulls[i][j],simHulls[i][j+1]);
			//	if(len > hull_thres) hull_thres = len;
			//}
			//hull_thres *= 1-0.618; // magic number
			//int cnt_numOfCurv = 0;
			//for(int j=0;j+1<simHulls[i].size();j++){
			//	float len = dist(simHulls[i][j],simHulls[i][j+1]);
			//	if(len>hull_thres){
			//		vCurve newC;
			//		newC.addPoint(vPoint2D(simHulls[i][j][0],simHulls[i][j][1]));
			//		newC.addPoint(vPoint2D(simHulls[i][j+1][0],simHulls[i][j+1][1]));
			//		newC.type = vCurve::STRAIGHT;
			//		this->convexHulls2d.push_back(newC);
			//		cnt_numOfCurv++;
			//	}
			//}
			//curvesInBranch.push_back(cnt_numOfCurv);

			// someone does not like global hull
			// change it to boundary marks
			// the top, bottom, left, right extrem points of the global hull
			// the marks are either vertical or horizantal, except that
			// if two of them are the same point -> TODO: find normal for that mark

			double minX=9999,maxX=-1;
			double minY=9999,maxY=-1;
			int minX_ind=-1,maxX_ind=-1;
			int minY_ind=-1,maxY_ind=-1;

			int i=0;
			for(int j=0;j<simHulls[i].size();j++){
				vec2 pt= simHulls[i][j];
				if(pt[0] < minX){
					minX = pt[0];
					minX_ind = j;
				}
				if(pt[0] > maxX){
					maxX = pt[0];
					maxX_ind = j;
				}
				if(pt[1] < minY){
					minY = pt[1];
					minY_ind = j;
				}
				if(pt[1] > maxY){
					maxY = pt[1];
					maxY_ind = j;
				}
			}

			// add the mark into convexHull2d data structure
			int cnt_numOfCurv = 0;
			const int markLen=20;
			const int boundaryGap =2;
			if(true){
				// minY
				vCurve newC3;
				newC3.addPoint(vPoint2D(simHulls[i][minY_ind][0]-markLen, simHulls[i][minY_ind][1] - boundaryGap));
				newC3.addPoint(vPoint2D(simHulls[i][minY_ind][0]+markLen, simHulls[i][minY_ind][1] - boundaryGap));
				newC3.type = vCurve::STRAIGHT;
				this->convexHulls2d.push_back(newC3);
				cnt_numOfCurv++;

				// maxY
				vCurve newC4;
				newC4.addPoint(vPoint2D(simHulls[i][maxY_ind][0]-markLen, simHulls[i][maxY_ind][1] + boundaryGap));
				newC4.addPoint(vPoint2D(simHulls[i][maxY_ind][0]+markLen, simHulls[i][maxY_ind][1] + boundaryGap));
				newC4.type = vCurve::STRAIGHT;
				this->convexHulls2d.push_back(newC4);
				cnt_numOfCurv++;

				// minX
				vCurve newC;
				newC.addPoint(vPoint2D(simHulls[i][minX_ind][0] - boundaryGap, simHulls[i][minX_ind][1]-markLen));
				newC.addPoint(vPoint2D(simHulls[i][minX_ind][0] - boundaryGap, simHulls[i][minX_ind][1]+markLen));
				newC.type = vCurve::STRAIGHT;
				this->convexHulls2d.push_back(newC);
				cnt_numOfCurv++;

				// maxX
				vCurve newC2;
				newC2.addPoint(vPoint2D(simHulls[i][maxX_ind][0] + boundaryGap, simHulls[i][maxX_ind][1]-markLen));
				newC2.addPoint(vPoint2D(simHulls[i][maxX_ind][0] + boundaryGap, simHulls[i][maxX_ind][1]+markLen));
				newC2.type = vCurve::STRAIGHT;
				this->convexHulls2d.push_back(newC2);
				cnt_numOfCurv++;

				
			}
			curvesInBranch.push_back(cnt_numOfCurv);
		}

		// local hulls
		for(int i=0;i<branches.size() && i<simHulls.size()-1;i++){ // TODO, branch.size > simHulls, not sure what happenned, quick fix
			// branch[i] -> simHulls[i+1]
			// spine (degree(v)>2): draw the sim hull
			if((branches[i].size() == 1)){
				// create a threshold for sim hull, only keep a few salient edge
				float hull_thres=0;
				for(int j=0;j+1 <simHulls[i+1].size();j++){
					float len = dist(simHulls[i+1][j],simHulls[i+1][j+1]);
					if(len > hull_thres) hull_thres = len;
				}

				hull_thres *= 0.2; // wish for magic

				int cnt_numOfCurv = 0;
				for(int j=0;j+1 <simHulls[i+1].size();j++){
					float len = dist(simHulls[i+1][j],simHulls[i+1][j+1]);
					if(len>hull_thres){
						vCurve newC;
						newC.addPoint(vPoint2D(simHulls[i+1][j][0],simHulls[i+1][j][1]));
						newC.addPoint(vPoint2D(simHulls[i+1][j+1][0],simHulls[i+1][j+1][1]));
						newC.type = vCurve::STRAIGHT;
						this->convexHulls2d.push_back(newC);
						cnt_numOfCurv++;
					}
				}
				curvesInBranch.push_back(cnt_numOfCurv);
			}
			// branch (degree(v)<=2, degree(neighbor)<=2): draw the joint marks
			else if (branches[i].size() > 1){
				std::vector<int> & curbranch = branches[i];
				// the marker between j & j+1
				int cnt_numOfCurv = 0;
				for(int j=0;j+1<curbranch.size();j++){
					NodeID u = this->segment2NodeID[curbranch[j]];
					NodeID v = this->segment2NodeID[curbranch[j+1]];

					EdgeID e;
					bool exist=false;
					boost::tie(e,exist) = boost::edge(u,v,this->SegConnectivity->g);// boost::edge(u,v,*(this->SegConnectivity));
					assert(exist);

					_Graph& cong = this->SegConnectivity->g;
					vNode* nu = cong[u];
					vNode* nv = cong[v];
					vEdge* ne = cong[e];
					//newC.addPoint()

					vec cu = nu->center;
					vec cv = nv->center;
					vec ce = ne->center;

					vec2 norm = this->projDir3(cu-cv);
					normalize(norm);

					vec2 tang(norm[1],-norm[0]);
					vec cMarker = this->projVec3(ce);
					float markerLen = 13;
					
					vCurve newC;
					vec2 p1 = vec2(cMarker[0], cMarker[1])+markerLen/2*tang;
					vec2 p2 = vec2(cMarker[0], cMarker[1])-markerLen/2*tang;
					newC.addPoint(vPoint2D(p1[0], this->h-  p1[1] ,nu->segmentID,false));
					newC.addPoint(vPoint2D(p2[0],this->h-  p2[1] ,nu->segmentID,false));
					newC.type = vCurve::STRAIGHT;
					this->convexHulls2d.push_back(newC);
					cnt_numOfCurv++;
				}
				curvesInBranch.push_back(cnt_numOfCurv);
			}
		}

		int cnt_cur_curve = 0; 

		{ // global hull
			vNode* sn = new vNode();
			sn->isLeaf = false;
			sn->superNode = new vGraph();
			for(int i=0;i<this->segment2NodeID.size();i++){
				sn->coveredSegments.push_back(i); 
			}
			sn->segmentID = -2;
			std::vector<NodeID> nodeids;
			for(int i=0;i<curvesInBranch[0] ;i++, cnt_cur_curve++){
				vNode* n = new vNode();

				vCurve& curv = this->convexHulls2d[cnt_cur_curve];
				curv.type = vCurve::STRAIGHT;
				curv.setTexture(this->texture);
				curv.removeRedundancy();
				n->setCurve(&curv);
				n->coveredSegments = sn->coveredSegments;
				n->segmentID = n->coveredSegments[0];

				NodeID nID = sn->superNode->addNode(n);
				nodeids.push_back(nID);
			}

			numOfGloabHull = cnt_cur_curve;
			this->addGraphEdgesWRTDistances( *(sn->superNode), nodeids); // add edges
			g.addNode(sn);
		}

		// local hulls/markers
		for(int i=1;i<curvesInBranch.size();i++){
			vNode* sn = new vNode();
			sn->isLeaf = false;
			sn->superNode = new vGraph();
			sn->coveredSegments = branches[i-1];
			sn->segmentID = sn->coveredSegments[0];

			std::vector<NodeID> nodeids;
			for(int j=0;j<curvesInBranch[i];j++, cnt_cur_curve++){
				vNode* n = new vNode();

				vCurve& curv = this->convexHulls2d[cnt_cur_curve];
				curv.type = vCurve::STRAIGHT;
				curv.setTexture(this->texture);
				curv.removeRedundancy();
				n->setCurve(&curv);
				n->coveredSegments = branches[i-1];
				n->segmentID = n->coveredSegments[0];

				NodeID nID = sn->superNode->addNode(n);
				nodeids.push_back(nID);
			}
			this->addGraphEdgesWRTDistances( *(sn->superNode), nodeids); // add edges
			g.addNode(sn);
			
		}
	}


	// get the mapping from seg # to node ids
	// map seg# to super node ID, for adding super edges
	std::vector<NodeID> seg2nodeID_simHull;
	std::vector<bool> nodeExist_simHull;
	seg2nodeID_simHull.resize(this->segment2NodeID.size());
	nodeExist_simHull.resize(this->segment2NodeID.size());

	for(int i=0;i<nodeExist_simHull.size();i++) nodeExist_simHull[i]=false;

	_Graph::vertex_iterator vertexIt, vertexEnd;
	boost::tie(vertexIt, vertexEnd) = boost::vertices(g.g);
	for (; vertexIt != vertexEnd; ++vertexIt){
		NodeID nID = *vertexIt; // dereference vertexIt, get the ID
		int segNum = g.g[nID]->segmentID;
		if(segNum>=0){
			nodeExist_simHull[segNum] = true;
			seg2nodeID_simHull[segNum] = nID;
		}
	}

	// ======  add super edges ============
	vGraph*& connectivity = this->SegConnectivity;
	connectivity->clearFlag();
	_Graph::vertex_iterator vStart, vTail;
	boost::tie(vStart, vTail) = boost::vertices(g.g); // any start ID is fine
	NodeID startID = *vStart;

	//BFS visiting
	std::deque<NodeID> q;
	connectivity->g[startID]->visited = true;
	q.push_back(startID);
	_Graph::adjacency_iterator  vIt, vEnd;
	while(!q.empty()){
		NodeID n = q.back();
		q.pop_back();
		// add neighbors
		// direct neighbors
		boost::tie(vIt, vEnd) = boost::adjacent_vertices( n, connectivity->g);
		for(;vIt!=vEnd;vIt++){
			NodeID n2 = *vIt;
			// make sure the edge is only added once between two nodes (no paralell edges)
			if( connectivity->g[n2]->visited  ){}
			else{
				connectivity->g[n2]->visited = true;
				q.push_back(n2);

				int segNum1 = connectivity->g[n]->segmentID;
				int segNum2 = connectivity->g[n2]->segmentID;

				if(nodeExist_simHull[segNum1] && nodeExist_simHull[segNum2]){
					NodeID u=seg2nodeID_simHull[segNum1];
					NodeID v=seg2nodeID_simHull[segNum2];

					vEdge* e=new vEdge();
					e->from = g.g[u];
					e->to = g.g[v];
					g.addEdge(u,v,e);
				}
			}
		}
	}
	connectivity->clearFlag();


	return;
}

/**********************************************************

						Primitives

***********************************************************/
void imageProc::addPrimitiveLinesAndBuildPrimsGraph(std::vector<Prism>* ps, vGraph& graph, vec lighting){
	this->PrimitiveLines.clear();
	for(int i=0;i<ps->size();i++){
		Prism& p = ps->at(i);
		vec p1,p2;

		// look for the farest point assume z-buffer[0,1], 0 is close to the screen
		float maxd = -1;
		int max_ind = -1;
		for(int j=0;j<8;j++){
			vec pt = p.points[j];
			GLdouble pos3D_x, pos3D_y, pos3D_z;
			gluProject(pt[0], pt[1] , pt[2],
				modelview, projection, viewport, 
				&pos3D_x, &pos3D_y, &pos3D_z);
			if(pos3D_z > maxd){
				maxd = pos3D_z;
				max_ind = j;
			}
		}
		int far_point = max_ind;
		// 12 edges
		for(int j=0;j<4;j++){
			int pi;
			GLdouble pos3D_x, pos3D_y, pos3D_z;
			// upper plane
			if(0+j != far_point && (1+j)%4!= far_point) {
			vCurve newC;
			pi = 0+j;
			p1 = p.points[pi];
			
				gluProject(p1[0], p1[1] , p1[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
			newC.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y, i,false));
			pi= (1+j)%4;
			p2 = p.points[pi];
			gluProject(p2[0], p2[1] , p2[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
			newC.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y, i,false));
			newC.type = vCurve::STRAIGHT;
			newC.setTexture(this->texture);
			newC.removeRedundancy();
			this->PrimitiveLines.push_back(newC);
			}

			// lower plane
			if(0+j +4 != far_point && (1+j)%4 +4 != far_point){
			vCurve newC1;
			pi = 0+j +4;
			p1 = p.points[pi];
			gluProject(p1[0], p1[1] , p1[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
			newC1.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y, i,false));
			pi= (1+j)%4 +4;
			p2 = p.points[pi];
			gluProject(p2[0], p2[1] , p2[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
			newC1.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y, i,false));
			newC1.type = vCurve::STRAIGHT;
			newC1.setTexture(this->texture);
			newC1.removeRedundancy();
			this->PrimitiveLines.push_back(newC1);
			}

			// axis
			if(0+j != far_point && 0 +j + 4 != far_point){
			vCurve newC2;
			pi = 0+j;
			p1 = p.points[pi];
			gluProject(p1[0], p1[1] , p1[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
			newC2.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y, i,false));
			pi= 0 +j + 4;
			p2 = p.points[pi];
			gluProject(p2[0], p2[1] , p2[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
			newC2.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y, i,false));
			newC2.type = vCurve::STRAIGHT;
			newC2.setTexture(this->texture);
			newC2.removeRedundancy();
			this->PrimitiveLines.push_back(newC2);
			}
		}

	}

	graph.clear();


	//2d lighting 
	GLdouble pos3D_x, pos3D_y, pos3D_z;
	gluProject(lighting[0], lighting[1] , lighting[2],
		modelview, projection, viewport, 
		&pos3D_x, &pos3D_y, &pos3D_z);
	vec2 lighting2d = vec2(pos3D_x,this->h - pos3D_y);
	assert(len(lighting2d)>0);
	normalize(lighting2d);

	for(int i=0;i<this->PrimitiveLines.size();i+=9){
		// super node I
		vNode* sn = new vNode(i/9);
		sn->isLeaf = false;
		sn->superNode = new vGraph();

		std::vector<NodeID> nodesids;
		// 9 visible curves in one segmentation
		for(int j=0;j<9;j++){
			vNode* n = new vNode(i/9);
			n->setCurve(&(PrimitiveLines[i+j]));
			n->coveredSegments.push_back(i/9);

			// set intensity
			vCurve* c = n->getCurve();
			vec2 p1 = c->curve2d.back().toVec2();
			vec2 p2 = c->curve2d.front().toVec2();

			vec2 tan = (p1- p2 );
			normalize(tan);
			float _intense = abs( (vec2(tan[1],-tan[0])) DOT lighting2d);
			for(int k=0;k<c->curve2d.size();k++){
				c->curve2d[k].intensity = _intense;
			}

			NodeID tmp_nID = sn->superNode->addNode(n);
			nodesids.push_back(tmp_nID);
		}
		// 1-2-3 -... - 12
		// add edges: 1-3-9-7; 4-6-12-10; 2-5-8-11, the neighbor 2, plus one closest
		// or, follow the routine
		this->addGraphEdgesWRTDistances( *(sn->superNode), nodesids);


		graph.addNode(sn);
	}

	// alais
	vGraph& g=graph;
	// ======  add super edges ============
	// get the mapping from seg # to node ids
	// map seg# to super node ID, for adding super edges
	std::vector<NodeID> seg2nodeID_primitive;
	std::vector<bool> nodeExist_primitive;
	seg2nodeID_primitive.resize(this->segment2NodeID.size());
	nodeExist_primitive.resize(this->segment2NodeID.size());

	for(int i=0;i<nodeExist_primitive.size();i++) nodeExist_primitive[i]=false;

	_Graph::vertex_iterator vertexIt, vertexEnd;
	boost::tie(vertexIt, vertexEnd) = boost::vertices(g.g);
	for (; vertexIt != vertexEnd; ++vertexIt){
		NodeID nID = *vertexIt; // dereference vertexIt, get the ID
		int segNum = g.g[nID]->segmentID;
		if(segNum>=0){
			nodeExist_primitive[segNum] = true;
			seg2nodeID_primitive[segNum] = nID;
		}
	}

	// add super edges 
	vGraph*& connectivity = this->SegConnectivity;
	connectivity->clearFlag();
	_Graph::vertex_iterator vStart, vTail;
	boost::tie(vStart, vTail) = boost::vertices(g.g); // any start ID is fine
	NodeID startID = *vStart;

	//BFS visiting
	std::deque<NodeID> q;
	connectivity->g[startID]->visited = true;
	q.push_back(startID);
	_Graph::adjacency_iterator  vIt, vEnd;
	while(!q.empty()){
		NodeID n = q.back();
		q.pop_back();
		// add neighbors
		// direct neighbors
		boost::tie(vIt, vEnd) = boost::adjacent_vertices( n, connectivity->g);
		for(;vIt!=vEnd;vIt++){
			NodeID n2 = *vIt;
			// make sure the edge is only added once between two nodes (no paralell edges)
			if( connectivity->g[n2]->visited  ){}
			else{
				connectivity->g[n2]->visited = true;
				q.push_back(n2);

				int segNum1 = connectivity->g[n]->segmentID;
				int segNum2 = connectivity->g[n2]->segmentID;

				if(nodeExist_primitive[segNum1] && nodeExist_primitive[segNum2]){
					NodeID u=seg2nodeID_primitive[segNum1];
					NodeID v=seg2nodeID_primitive[segNum2];

					vEdge* e=new vEdge();
					e->from = g.g[u];
					e->to = g.g[v];
					g.addEdge(u,v,e);
				}
			}
		}
	}
	connectivity->clearFlag();

	return;

}

/**********************************************************

						Suggestive Contours

***********************************************************/
void imageProc::addSCLine(std::vector<std::vector<vec>> *  _chains){
	this->SCLines.clear();
	for(int i=0; i<_chains->size();i++){
		std::vector<vec> c = _chains->at(i);
		if(true){ // set constrains on each line, or not
			vCurve newC;
			for(int k=0; k<c.size();k++ ){
				double v[3];
				for(int j=0; j<3; j++){
					v[j] =c[k][j];
				}
			
				GLdouble pos3D_x, pos3D_y, pos3D_z;
				gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
				newC.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y));
			}
			this->SCLines.push_back(newC);
		}
	}
	return;
}
void imageProc::addSCFaces(std::vector<std::vector<int>>* faceChain, const vector<TriMesh::Face>* fList, const std::vector<vec>* vList ){

	this->SCLines.clear();
	for(int i=0;i<faceChain->size();i++){
		std::vector<int>& chain=faceChain->at(i);
		if(chain.size()<3) continue;
		vCurve newC;
		for(int j=0;j<chain.size();j++){
			TriMesh::Face f = fList->at(chain[j]);
			vec p = vList->at(f[0]);

			GLdouble pos3D_x, pos3D_y, pos3D_z;
			gluProject(p[0], p[1] , p[2],
				modelview, projection, viewport, 
				&pos3D_x, &pos3D_y, &pos3D_z);
			newC.addPoint(vPoint2D(pos3D_x,this->h - pos3D_y));
		}
		this->SCLines.push_back(newC);
	}
	return;
}
void imageProc::addSCLines(std::vector<std::vector<int>> * _chains, std::vector<std::vector<int>> * silhouette,
	const std::vector<vec>* vertices,vec light, vec camera_pos, const std::vector<vec>* normals, 
	const vector<float>* curv1, const vector<float>* curv2)
{
	this->SCLines.clear();
	for(int i=0; i<_chains->size();i++){
		std::vector<int> c = _chains->at(i);
		if(c.size()>3){ // ignore small curves
			vCurve newC;
			for(int k=0; k<c.size();k++ ){
				double v[3];
				for(int j=0; j<3; j++){
					v[j] =vertices->at( c.at(k))[j];
				}
			
				GLdouble pos3D_x, pos3D_y, pos3D_z;
				gluProject(v[0], v[1] , v[2],
					modelview, projection, viewport, 
					&pos3D_x, &pos3D_y, &pos3D_z);
				
				vPoint2D p=vPoint2D(pos3D_x,this->h - pos3D_y,c[k]);

				// intensity: light cross normal
				float _intensity = len(  (normals->at(c[k])) CROSS light  ) ;
				p.intensity = _intensity;

				// thickness
				vec view_vec = camera_pos - vertices->at(c[k]);
				normalize(view_vec);
				float _thickness = len( ((normals->at(c[k])) CROSS (view_vec)) );
				p.thickness = _thickness;

				// velocity
				float _velocity = sqrt( curv1->at(c[k])* curv1->at(c[k]) + curv2->at(c[k])*curv2->at(c[k]));
				p.velocity = _velocity;

				newC.addPoint(p);
			}
			this->SCLines.push_back(newC);
		}
	}

	if(silhouette->size()>0){
		this->silhouetteLines.clear();
		for(int i=0; i<silhouette->size();i++){
			std::vector<int> c = silhouette->at(i);
			if(c.size()>1){ // ignore small curves
				vCurve newC;
				for(int k=0; k<c.size();k++ ){
					double v[3];
					for(int j=0; j<3; j++){
						v[j] =vertices->at( c.at(k))[j];
					}
			
					GLdouble pos3D_x, pos3D_y, pos3D_z;
					gluProject(v[0], v[1] , v[2],
						modelview, projection, viewport, 
						&pos3D_x, &pos3D_y, &pos3D_z);
					
					vPoint2D p=vPoint2D(pos3D_x,this->h - pos3D_y,c[k]);

					// intensity: light cross normal
					float _intensity = len(  (normals->at(c[k])) CROSS light  ) ;
					p.intensity = _intensity;

					// thickness
					vec view_vec = camera_pos - vertices->at(c[k]);
					normalize(view_vec);
					float _thickness = len( ((normals->at(c[k])) CROSS (view_vec)) );
					p.thickness = _thickness;

					// velocity
					float _velocity = sqrt( curv1->at(c[k])* curv1->at(c[k]) + curv2->at(c[k])*curv2->at(c[k]));
					p.velocity = _velocity;

					newC.addPoint(p);
				}
				this->silhouetteLines.push_back(newC);
			}
		}
	}

	return;
}
void imageProc::splitEXSCChainsRecursively(){
	if(this->SCLines.size()>0){
		this->splitVertexChainsRecursively(this->SCLines, 2.5*this->splitThreshold);
	}
	if(this->silhouetteLines.size()>0){
		this->splitVertexChainsRecursively(this->silhouetteLines, 2.5*this->splitThreshold);
	}
}
void imageProc::buildEXSCGraph(vGraph& g, std::vector<int>* pointColor){
	if(pointColor->size() == 0) {this->useGraph=false;return;} // segmentation is not ready, do not construct the graph
	
	this->useGraph = true;
	g.clear();
	std::vector<vNode*> leaves;
	//std::vector<std::vector<int>> exsc = this->SCLines;
	//exsc.insert(exsc.end(), this->silhouetteLines.begin(), this->silhouetteLines.end());
	int totSize = this->SCLines.size()+this->silhouetteLines.size();
	//int totSize = this->silhouetteLines.size();
	for(int i=0;i<totSize;i++){ // first sc, then edge
		
		
		vCurve& curv = ( i< this->SCLines.size())? this->SCLines[i]:this->silhouetteLines[i-this->SCLines.size()];
		if( i< this->SCLines.size()){
			curv.isInner = true;
		}
		else{
			curv.isInner = false;
		}
		//vCurve& curv = this->silhouetteLines[i];
		curv.type = vCurve::OPEN;
		curv.setTexture(this->texture);
		curv.removeRedundancy();

		// ignore extremely small sc curves
		if((curv.size()<5 || curv.Length()<10 ) && curv.isInner){
			continue;
		}

		vNode* n = new vNode();
		n->setCurve(&curv);
		std::vector<vPoint2D>& c = curv.curve2d;

		int maxSegNum=-1;
		for(int j=0;j<c.size();j++){
			int seg = c[j].segment = pointColor->at(c[j].get3DIndex());
			
			if(seg>maxSegNum) maxSegNum = seg;
			bool _exist=false;
			for(int k=0;k<n->coveredSegments.size();k++){
				if(n->coveredSegments[k] == seg){
					_exist = true;
					break;
				}
			}
			if(! _exist){
				n->coveredSegments.push_back(seg);
			}
		}
		//g.addNode(n);
		
		// find the principal segment#
		std::vector<int>& segs = n->coveredSegments;
		assert(segs.size() >0);
		if(segs.size() == 1){
			n->segmentID = segs[0];
		}
		else{
			// move principal seg# to segs[0], and set segmentID = principal seg#
			std::vector<double> lens;
			lens.resize(maxSegNum+1);
			for(int j=0;j<lens.size();j++) lens[j]=0;
			for(int j=0;j<c.size()-1;j++){
				double len = dist(c[j].toVec2(), c[j+1].toVec2());
				lens[c[j].segment] += len/2;
				lens[c[j+1].segment] += len/2;
			}

			int maxInd=-1; double maxLen =-1;
			for(int j=0;j<lens.size();j++){
				if(lens[j]>maxLen){
					maxInd = j;
					maxLen = lens[j];
				}
			}

			if(segs[0] != maxInd){// swap
				int tmp = segs[0];
				segs[0] = maxInd;
				for(int j=0;j<segs.size();j++){
					if(segs[j] == maxInd){
						segs[j] = tmp;
						break;
					}
				}
			}

			n->segmentID = maxInd;
		}

		leaves.push_back(n);
	}

	// Node type I - segment level supernode
	// organize the leaf nodes w.r.t their principal segment#
	std::vector<std::vector<int>> superNodesI;
	superNodesI.resize(30);
	for(int i=0; i<leaves.size(); i++){
		int pSegNum = leaves[i]->segmentID;
		if(pSegNum >= superNodesI.size()){ // size checking
			superNodesI.resize(pSegNum+2);
		}
		superNodesI[pSegNum].push_back(i);
	}
	// create 
	int chk=0;
	for(int i=0;i<superNodesI.size();i++){
		if(superNodesI[i].size()>0){
			vNode* superN = new vNode();
			superN->isLeaf = false;
			superN->superNode = new vGraph();

			std::vector<NodeID> nodeids;
			for(int j=0;j<superNodesI[i].size();j++){
				NodeID nID = superN->superNode->addNode(leaves[superNodesI[i][j]]);
				nodeids.push_back(nID);
			}
			this->addGraphEdgesWRTDistances( *(superN->superNode), nodeids); // add edges

			chk += superNodesI[i].size();
			superN->segmentID = i;
			g.addNode(superN);
		}
	}
	//assert(chk == totSize);

	// ======  add super edges ============
	// get the mapping from seg # to node ids
	// map seg# to super node ID, for adding super edges
	std::vector<NodeID> seg2nodeID_exsc;
	std::vector<bool> nodeExist_exsc;
	seg2nodeID_exsc.resize(this->segment2NodeID.size());
	nodeExist_exsc.resize(this->segment2NodeID.size());

	for(int i=0;i<nodeExist_exsc.size();i++) nodeExist_exsc[i]=false;

	_Graph::vertex_iterator vertexIt, vertexEnd;
	boost::tie(vertexIt, vertexEnd) = boost::vertices(g.g);
	for (; vertexIt != vertexEnd; ++vertexIt){
		NodeID nID = *vertexIt; // dereference vertexIt, get the ID
		int segNum = g.g[nID]->segmentID;
		if(segNum>=0){
			nodeExist_exsc[segNum] = true;
			seg2nodeID_exsc[segNum] = nID;
		}
	}

	// add super edges 
	vGraph*& connectivity = this->SegConnectivity;
	connectivity->clearFlag();
	_Graph::vertex_iterator vStart, vTail;
	boost::tie(vStart, vTail) = boost::vertices(g.g); // any start ID is fine
	NodeID startID = *vStart;

	//BFS visiting
	std::deque<NodeID> q;
	connectivity->g[startID]->visited = true;
	q.push_back(startID);
	_Graph::adjacency_iterator  vIt, vEnd;
	while(!q.empty()){
		NodeID n = q.back();
		q.pop_back();
		// add neighbors
		// direct neighbors
		boost::tie(vIt, vEnd) = boost::adjacent_vertices( n, connectivity->g);
		for(;vIt!=vEnd;vIt++){
			NodeID n2 = *vIt;
			// make sure the edge is only added once between two nodes (no paralell edges)
			if( connectivity->g[n2]->visited  ){}
			else{
				connectivity->g[n2]->visited = true;
				q.push_back(n2);

				int segNum1 = connectivity->g[n]->segmentID;
				int segNum2 = connectivity->g[n2]->segmentID;

				if(nodeExist_exsc[segNum1] && nodeExist_exsc[segNum2]){
					NodeID u=seg2nodeID_exsc[segNum1];
					NodeID v=seg2nodeID_exsc[segNum2];

					vEdge* e=new vEdge();
					e->from = g.g[u];
					e->to = g.g[v];
					g.addEdge(u,v,e);
				}
			}
		}
	}
	connectivity->clearFlag();
	return;
}


