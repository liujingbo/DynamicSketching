#include "vdraw.h"

#include <QFileDialog>
#include <qstring.h>

#pragma warning(disable:4480)

VDraw::VDraw(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	this->iniWindow();
	this->renderMode=SIMPLE;
	this->filterMode=NO_FILTER;
	//this->renderMode=DEPTH;
	//this->filterMode=HOUGH;
	
	this->image_process = new imageProc();
	IplImage* ipl_image = cvLoadImage("203-QT2486-Scenic-Drive-Lakeside.jpg");

	setQimageWithIpl(ipl_image);

	ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));
	this->image_process->setOrignalImage(ipl_image);
	//this->image_process->testBlack();
	// ************ edit the iplimage in image_process ************


	// QImage ( const uchar * data, int width, int height, int bytesPerLine, Format format )
	// cv_image.ptr()
	// the above is uchar* type

	// signals/slots mechanism in action
	//connect( ui.actionLoad_mesh, SIGNAL( clicked() ), this, SLOT( LoadMesh() ) ); 
	connect(ui.gl_widget, SIGNAL(glRepaint()), this, SLOT( catchGlRepaint() ));
	connect(ui.gl_widget2, SIGNAL(glRepaint()), this, SLOT( catchGlRepaint() ));

	// opengl tabs config
	// tab 1
	ui.gl_widget->b->setUnvisible();
	
	// ui.gl_widget->b1->setUncompute();// 
	// ui.gl_widget->b1->setUnvisible();
	 ui.gl_widget->b2->setUnvisible();
	//ui.gl_widget->b3->setUnvisible();
	ui.gl_widget->b3->SetColoredPatchesDisplay(true);
	ui.gl_widget->b3->SetPrimitivesDisplay(false);
	ui.gl_widget->b4->SetPrimitivesDisplay(false);
	ui.gl_widget->isRenderConvexHull2D = false;
	
	
	// tab 2 -- render convex hull
	//ui.gl_widget2->isRenderConvexHull2D=true;
	//ui.gl_widget2->b->setUnvisible();
	ui.gl_widget2->b1->setUnvisible();
	ui.gl_widget2->b2->setUnvisible();
	//ui.gl_widget2->b3->SetColoredPatchesDisplay(false);
	//ui.gl_widget2->b3->SetPrimitivesDisplay(true);
	ui.gl_widget2->b3->setUnvisible();

	// ================== more ui ==============
	// ================== stella
	this->ui.dockWidget->hide();
	this->renderCanvas.hide();
	

}

VDraw::~VDraw()
{
	if(image_process) delete image_process;
}

void VDraw::loadTriMesh(){
	ui.output_list->addItem("loading trimesh...");

	QString path = "..\\testdata\\examples\\named";

	QString filename = QFileDialog::getOpenFileName( 
        this, 
        tr("Open TriMesh"), 
        path, 
        tr("off files (*.off)") );
	if( !filename.isNull() )
    {
      qDebug( filename.toAscii() );
    }
	else{return;}
	std::string str = std::string(filename.toAscii().data());
	const char * stuff = str.c_str();
	
	// reset GLwidget
	ui.gl_widget->primitives.clear();
	ui.gl_widget->princetonsegmentation.clear();

	// reset drawers
	for(int i=0;i < ui.gl_widget->models[0]->drawers_.size();i++){
		ui.gl_widget->models[0]->drawers_[i]->init();
	}
	// reset canvas
	this->image_process->clearData();
	ui.gl_widget->LoadTriMeshFile(stuff);
	ui.output_list->addItem("open file: "+filename);

	// ============  load segmentation and fitting BtW  =================
	// get the model name
	std::string modelName = str.substr(0,str.size()-4);
	std::string segName = modelName + ".seg";
	std::string fitName = modelName + ".fit";
	std::string relaName = modelName + ".rela";

	// load segmentation
	const char * stuff2 = segName.c_str();
	bool tmpb = ui.gl_widget->LoadPrincetonSegmentation(stuff2);
	this->image_process->SegConnectivity = ui.gl_widget->graph; // TODO: pointer handlling, handle reload, handle delete
	this->image_process->segment2NodeID = ui.gl_widget->segment2NodeID;
	ui.output_list->addItem("open segmentation: "+QString(stuff2));

	// load fitting
	ui.gl_widget->LoadFitting(fitName.c_str(),0);

	// load segment relations
	this->image_process->loadSegmentRelations(relaName.c_str());

	// TODO: update the stella
	//setStella();

	// repaint
	this->ui.gl_widget->repaint();
	catchGlRepaint();
	
	return;
}

void VDraw::loadSegmentation(){

	ui.output_list->addItem("loading segmentation...");
	QString path = "..\\testdata\\examples\\named";
	QString filename = QFileDialog::getOpenFileName( 
        this, 
        tr("Open Segmentation"), 
        path, 
        tr("All files (*.seg)") );
	if( !filename.isNull() )
    {
      qDebug( filename.toAscii() );
	}else{
		return;
	}
	std::string str = std::string(filename.toAscii().data());
	const char * stuff = str.c_str();

	// reset canvas
	this->image_process->clearData();

	bool tmpb = ui.gl_widget->LoadPrincetonSegmentation(stuff);

	this->image_process->SegConnectivity = ui.gl_widget->graph; // TODO: pointer handlling, handle reload, handle delete
	this->image_process->segment2NodeID = ui.gl_widget->segment2NodeID;

	ui.output_list->addItem("open segmentation: "+filename);

	// TODO: update the stella
	//setStella();

	// repaint
	this->ui.gl_widget->repaint();
	catchGlRepaint();
	return;
}

void VDraw::loadExoAtlas(){
	ui.output_list->addItem("loading atlas...");
	QString filename = QFileDialog::getOpenFileName( 
        this, 
        tr("Open Atlas"), 
        QDir::currentPath(), 
        tr("Atlas files (*.atlas);;All files (*.*)") );
	if( !filename.isNull() )
    {
      qDebug( filename.toAscii() );
    }
	std::string str = std::string(filename.toAscii().data());
	const char * stuff = str.c_str();
	ui.gl_widget->LoadExoAtlasFile(stuff);
	
	ui.output_list->addItem("open file: "+filename);
	catchGlRepaint();
	return;

}

void VDraw::renderDepthBuffer(){
	this->renderMode=DEPTH;
	int w = (this->ui.gl_widget->width());
	int h=  this->ui.gl_widget->height();
	GLfloat* depth_data = this->ui.gl_widget->depthBuffer();
	IplImage* ipl_image =cvCreateImage( cvSize( w , h),IPL_DEPTH_8U,3);
	for(int i=0; i<w;i++){
		for(int j=0; j<h; j++){
			cvSet2D(ipl_image,h-1-j, i, cvScalarAll( depth_data[i+j*w]*255.0));
		}
	}
	
	// iplimage to qimage conversion
	setQimageWithIpl(ipl_image); // only reset when ipl/Qt image address changes

	ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));
	this->image_process->setOrignalImage(ipl_image);// only reset when ipl image address changes

	return;
}
void VDraw::renderTextured(){
	this->renderCanvas.LoadStroke("..\\testdata\\strokes\\1_1.strokes");
	this->renderCanvas.show();
	return;
}

void VDraw::setQimageWithIpl(const IplImage *ipl_image){
	// iplimage to qimage conversion
	const unsigned char * data = (unsigned char *)(ipl_image->imageData);
	int width 			= ipl_image->width;
	int height			= ipl_image->height;
	int bytesPerLine 	= ipl_image->widthStep;
	q_image =QImage( data, width, height, bytesPerLine, QImage::Format_RGB888 );
}

/*
	call after the rendering image is changed
	q_image's data share memory with image_proc's ipl_image
*/
void VDraw::updateRendering(){
	ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));
	return;
}

void VDraw::iniWindow(){
}

QImage * VDraw::IplImageToQImage(const IplImage *img)   
{   
    QImage *image;   
    //cvCvtColor(img,img,CV_BGR2RGB);   
    uchar *imgData=(uchar *)img->imageData;   
    image=new QImage(imgData,img->width,img->height,QImage::Format_RGB888);
    return image;   
} 



/*************************************************************************
				slots - handle UI events
**************************************************************************/
void VDraw::catchGlRepaint(){
	if(this->renderMode==DEPTH){
		int w = (this->ui.gl_widget->width());
		int h=  this->ui.gl_widget->height();
		GLfloat* depth_data = this->ui.gl_widget->depthBuffer();
		IplImage* ipl_image = this->image_process->display_image;
		for(int i=0; i<w;i++){
			for(int j=0; j<h; j++){
				cvSet2D(ipl_image,h-1-j, i, cvScalarAll(depth_data[i+j*w]*255.0));
			}
		}
		//setQimageWithIpl(ipl_image);
		//this->image_process->setOrignalImage(ipl_image);
	}
	else{
		int w = (this->ui.gl_widget->width());
		int h=  this->ui.gl_widget->height();
		IplImage* ipl_image = this->image_process->display_image;

		// image size needs to change -> re-create the image
		if(ipl_image->width != w || ipl_image->height != h){
			ipl_image =cvCreateImage( cvSize( w , h),IPL_DEPTH_8U,3);
			// iplimage to qimage conversion
			setQimageWithIpl(ipl_image); // only reset when ipl/Qt image address changes
			ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));
			this->image_process->setOrignalImage(ipl_image);// only reset when ipl image address changes
		}
		for(int i=0; i<w;i++){
			for(int j=0; j<h; j++){
				cvSet2D(ipl_image,h-1-j, i, cvScalarAll(255.0));
			}
		}
	}
	
	// update 2D domain curves
	switch(this->filterMode){
	case NO_FILTER:break;
	case CANNY:{
		this->image_process->canny();
		this->ui.output_list->addItem("Canny filter applied.");
		break;
			   }
	case HOUGH:{
		int num_of_lines= (this->image_process->houghTransform());
		this->ui.output_list->addItem("Hough filter applied, got "+ (QString().setNum(num_of_lines))+" lines.");
		break;
			   }
	default: break;
	}

	// update 3D domain curves
	// edge contour is updated when call corresponding drawer, and stored in model, can be retrived through GLWidget
	
	int ind =0; //model indexx
	Model*& model = ui.gl_widget->models[ind];
	GLWidget* glwidget = this->ui.gl_widget;

	vec lighting= vec(0,1,0);
	// which tab is being shown
	int tabInd = ui.left_panel_tab->currentIndex();

	vec camera_pos = inv(glwidget->global_transf) * point(0,0,0);

	/********************************************************************
					pose compute
	********************************************************************/
	// conditions
	// 1. show pose line
	// 2. segmentation ready
	if(this->image_process->isPoseLinesVisible && this->ui.gl_widget->graph!=NULL && tabInd ==0){
		// set up transformations
		this->image_process->modelview = this->ui.gl_widget->getModelViewMatrix(ind);
		this->image_process->projection = this->ui.gl_widget->getProjectionMatrix(ind);
		this->image_process->viewport = this->ui.gl_widget->getViewPort(ind);

		// get pose lines, from model class
		this->image_process->addPoseLine(&(this->ui.gl_widget->models[ind]->poselines));

		this->image_process->SegConnectivity = ui.gl_widget->graph; // TODO: pointer handlling, handle reload, handle delete
		this->image_process->segment2NodeID = ui.gl_widget->segment2NodeID;
	}


	/********************************************************************
						edge contour compute
	********************************************************************/
	
	if(this->image_process->isEdgeContourVisible ){
		std::vector<std::vector<int>> edgeContourChains;
		if(tabInd ==0  && this->ui.gl_widget && ui.gl_widget->models.size()>0 && this->ui.gl_widget->b1->isCompute() && ui.gl_widget->models[ind]->edgeVertices.size()>0){
			this->image_process->modelview = this->ui.gl_widget->getModelViewMatrix(ind);
			this->image_process->projection = this->ui.gl_widget->getProjectionMatrix(ind);
			this->image_process->viewport = this->ui.gl_widget->getViewPort(ind);
			
			edgeContourChains = this->ui.gl_widget->models[ind]->buildEdgeChains();

			int cnt_numofcurves = edgeContourChains.size();
			std::vector<std::vector<int>>* mergedChains = model->tryMergeChains(&edgeContourChains);
			
			while(mergedChains->size() < cnt_numofcurves){
				cnt_numofcurves = mergedChains->size();
				mergedChains = model->tryMergeChains(mergedChains);
			}

			this->image_process->addEdgeContourChains(mergedChains, 
				&(model->mesh_->vertices),  lighting);
		}
		this->image_process->splitEdgeContourChainsRecursively();
		this->image_process->buildSimEdgeGraph(this->image_process->graphs[2], &(model->pointcolors_));
	}



	/********************************************************************
					get convex hull from GLwidget : may move to model
	********************************************************************/
	if(this->image_process->isConvexHullVisible && tabInd ==0){
		// update convex hull
		model->update_convex_hull_2d(this->image_process->h, false);
		//this->image_process->addConvexHullLines(&(this->ui.gl_widget->models[ind]->convex_hulls_2d));

		this->image_process->modelview = this->ui.gl_widget->getModelViewMatrix(ind);
		this->image_process->projection = this->ui.gl_widget->getProjectionMatrix(ind);
		this->image_process->viewport = this->ui.gl_widget->getViewPort(ind);

		this->image_process->addConvexHullLinesAndBuildSimHullGraph(
			&(this->ui.gl_widget->models[ind]->convex_hulls_2d), this->image_process->graphs[1], model->branches, false);
	}

	/********************************************************************
					primitives compute: update the transformations
	********************************************************************/
	if(this->image_process->isPrimVisible && this->ui.gl_widget->graph!=NULL && glwidget->primitives.size()>0 && tabInd ==0){
		// set up transformations
		this->image_process->modelview = this->ui.gl_widget->getModelViewMatrix(ind);
		this->image_process->projection = this->ui.gl_widget->getProjectionMatrix(ind);
		this->image_process->viewport = this->ui.gl_widget->getViewPort(ind);

		// get pose lines, from GL class
		this->image_process->addPrimitiveLinesAndBuildPrimsGraph( &( glwidget->primitives), this->image_process->graphs[3], lighting);
		
	}

	/********************************************************************
					sc compute
	********************************************************************/
	// mesh surface based
	if( this->image_process->isSCVisible && this->ui.gl_widget->b2->isCompute() && tabInd ==0 ){
		// set up transformations
		this->image_process->modelview = this->ui.gl_widget->getModelViewMatrix(ind);
		this->image_process->projection = this->ui.gl_widget->getProjectionMatrix(ind);
		this->image_process->viewport = this->ui.gl_widget->getViewPort(ind);

		std::vector<std::vector<int>> scFaceChains =
			this->ui.gl_widget->models[ind]->tryMergeFaces(& this->ui.gl_widget->models[ind]->buildFaceChains(&(this->ui.gl_widget->models[ind]->scFaces)));
		
		std::vector<std::vector<int>> scLines = this->ui.gl_widget->models[ind]->faceChain2VertexChain(&scFaceChains);
		
		int w = (this->ui.gl_widget->width());
		int h=  this->ui.gl_widget->height();
		GLfloat* depth_data = this->ui.gl_widget->depthBuffer();
		std::vector<std::vector<int>> extendedSC;
		std::vector<std::vector<int>> edges;

		
		if(this->ui.gl_widget->models[ind]->edgeVertices.size()>0 ) { // vertex based, merge with edges
			// prepare edge contour
			// model->edgeVertices = model->filterUnderDepthVertices( &model->edgeVertices,depth_data,w,h);
			// TODO: use contour edges for the construction, instead of contour vertices 
			edges = this->ui.gl_widget->models[ind]->buildEdgeChains2();

			// strict loop
			int cnt_numofcurves = edges.size();
			std::vector<std::vector<int>>* mergedChains = model->tryMergeChains_strict(&edges);
			while(mergedChains->size() < cnt_numofcurves){
				cnt_numofcurves = mergedChains->size();
				mergedChains = model->tryMergeChains_strict(mergedChains);
			}

			// relaxed loop
			cnt_numofcurves = mergedChains->size();
			mergedChains = model->tryMergeChains(mergedChains);
			while(mergedChains->size() < cnt_numofcurves){
				cnt_numofcurves = mergedChains->size();
				mergedChains = model->tryMergeChains(mergedChains);
			}

			// merge sc and edge
			//this->ui.gl_widget->models[ind]->tryMergeEdgeAndSC(&(edges),sc_raw, &extendedSC);
			extendedSC = model->buildEXSC(&scLines, &edges);
			this->image_process->addSCLines(&extendedSC,&edges, &( model->mesh_->vertices), 
				lighting,camera_pos, &(model->mesh_->normals), &(model->mesh_->curv1), &(model->mesh_->curv2));
			this->image_process->splitEXSCChainsRecursively();
			this->image_process->buildEXSCGraph(this->image_process->graphs[4], &(model->pointcolors_));
			this->ui.output_list->addItem("sc len="+QString::number(extendedSC.size()));
		}
		else{ // face based
			this->image_process->addSCFaces(&scFaceChains, &(this->ui.gl_widget->models[ind]->mesh_->faces),&(this->ui.gl_widget->models[ind]->mesh_->vertices));
		}
	}

	// interpolated vertex based
	if(false && this->image_process->isSCVisible && this->ui.gl_widget->b2->isCompute() && tabInd ==0  ){
		// set up transformations
		this->image_process->modelview = this->ui.gl_widget->getModelViewMatrix(ind);
		this->image_process->projection = this->ui.gl_widget->getProjectionMatrix(ind);
		this->image_process->viewport = this->ui.gl_widget->getViewPort(ind);
		std::vector<std::vector<vec>>* sc_raw = 
			this->ui.gl_widget->models[ind]->filtlerLongSegments(&(this->ui.gl_widget->models[ind]->suggestiveContour));
		int w = (this->ui.gl_widget->width());
		int h=  this->ui.gl_widget->height();
		GLfloat* depth_data = this->ui.gl_widget->depthBuffer();
		/*std::vector<std::vector<vec>> sc_filtered;
		this->ui.gl_widget->models[ind]->filterUnderDepthSegments(sc_raw,depth_data,w,h, &sc_filtered);
		this->ui.gl_widget->models[ind]->tryMergeChains(&sc_filtered);
		this->image_process->addSCLine(&sc_filtered);*/
		this->ui.gl_widget->models[ind]->tryMergeChains(sc_raw);
		//this->image_process->addSCLine(sc_raw);
		
		/********************************************
				TO MERGE CONTOUR & SC
		********************************************/
		std::vector<std::vector<vec>> extendedSC;
		std::vector<std::vector<int>> edges;
		if(this->ui.gl_widget->models[ind]->edgeVertices.size()>0 ) {
			// prepare edge contour
			
			model->edgeVertices = model->filterUnderDepthVertices( &model->edgeVertices,depth_data,w,h);
			edges = this->ui.gl_widget->models[ind]->buildEdgeChains();
			this->ui.gl_widget->models[ind]->tryMergeChains(&edges);
			this->ui.gl_widget->models[ind]->tryMergeEdgeAndSC(&(edges),sc_raw, &extendedSC);
		}
		this->image_process->addSCLine(&extendedSC);
		this->ui.output_list->addItem("sc len="+QString::number(extendedSC.size()));

		/*if(tabInd ==0  && this->ui.gl_widget && ui.gl_widget->models.size()>0 && this->ui.gl_widget->b1->isCompute() && ui.gl_widget->models[ind]->edgeVertices.size()>0){
			
			this->image_process->addEdgeContourChains(&edges, &(this->ui.gl_widget->models[ind]->mesh_->vertices));
		}*/
	
	}
	/********************************************************************
					draw curves
	********************************************************************/
	this->image_process->draw(); 

	/********************************************************************
					set QImage
	********************************************************************/
	ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));


}
void VDraw::canvasRepaint(){
	if(this->renderMode==DEPTH){
		int w = (this->ui.gl_widget->width());
		int h=  this->ui.gl_widget->height();
		GLfloat* depth_data = this->ui.gl_widget->depthBuffer();
		IplImage* ipl_image = this->image_process->display_image;
		for(int i=0; i<w;i++){
			for(int j=0; j<h; j++){
				cvSet2D(ipl_image,h-1-j, i, cvScalarAll(depth_data[i+j*w]*255.0));
			}
		}
		//setQimageWithIpl(ipl_image);
		//this->image_process->setOrignalImage(ipl_image);
	}
	else{
		int w = (this->ui.gl_widget->width());
		int h=  this->ui.gl_widget->height();
		IplImage* ipl_image = this->image_process->display_image;

		// image size needs to change -> re-create the image
		if(ipl_image->width != w || ipl_image->height != h){
			ipl_image =cvCreateImage( cvSize( w , h),IPL_DEPTH_8U,3);
			// iplimage to qimage conversion
			setQimageWithIpl(ipl_image); // only reset when ipl/Qt image address changes
			ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));
			this->image_process->setOrignalImage(ipl_image);// only reset when ipl image address changes
		}
		for(int i=0; i<w;i++){
			for(int j=0; j<h; j++){
				cvSet2D(ipl_image,h-1-j, i, cvScalarAll(255.0));
			}
		}
	}
	
	// update 2D domain curves
	switch(this->filterMode){
	case NO_FILTER:break;
	case CANNY:{
		this->image_process->canny();
		this->ui.output_list->addItem("Canny filter applied.");
		break;
			   }
	case HOUGH:{
		int num_of_lines= (this->image_process->houghTransform());
		this->ui.output_list->addItem("Hough filter applied, got "+ (QString().setNum(num_of_lines))+" lines.");
		break;
			   }
	default: break;
	}

	this->image_process->draw(); 

	ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));

	return;
}
void VDraw::filterCanny(){
	if(ui.actionCanny->isChecked()){
		//this->image_process->canny();
		//this->image_process->houghTransform();
		this->filterMode=CANNY;
	}
	else{
		this->filterMode=NO_FILTER;
		// TODO: need to uncheck all other filters
	}
	catchGlRepaint();
	// ui.render_label->setPixmap(QPixmap::fromImage(q_image.rgbSwapped(),0));
}
void VDraw::filterHough(){
	if(ui.actionHough_Transform->isChecked()){
		this->filterMode = HOUGH;
	}
	else{
		this->filterMode=NO_FILTER;
		// TODO: need to uncheck all other filters
	}
	catchGlRepaint();

}


// computation commands
void VDraw::orderCurves(){
	int max_curve = this->image_process->order2DStrokes();

	this->ui.horizontalSlider->setMaximum(max_curve);
	this->ui.horizontalSlider->setMinimum(0);
	this->ui.horizontalSlider->setValue(max_curve);

	
	return;
}
void VDraw::stratAnimateOrder(){
	this->ui.horizontalSlider->setValue( ui.horizontalSlider->minimum());
	animateOrderTimer.setInterval(1000/20);
	connect(&animateOrderTimer, SIGNAL(timeout()), this, SLOT(showNextStroke()));
	animateOrderTimer.start();
}
void VDraw::showNextStroke(){
	int cur_val=this->ui.horizontalSlider->value();
	cur_val ++;
	if(cur_val<= this->ui.horizontalSlider->maximum()){
		this->ui.horizontalSlider->setValue( cur_val);
		this->strokeNumChanged(cur_val);
	}
	else{
		animateOrderTimer.stop();
	}
}

void VDraw::strokeNumChanged(int val){
	this->image_process->numOfStrokesChanged(val);
	this->canvasRepaint();
	return;
}


// testing parameters
void  VDraw::paraNameChanged(QString str){
	double para_val=0;
	double max, min;
	if(str == "Splitting Chains"){
		double reval = this->image_process->paraNameChanged_split(max, min);
		para_val = (reval -min)/(max-min)*100;
	}
	else if(str == "Show Curvature"){
	}
	else if(str == "connect_dist"){
		double reval = this->ui.gl_widget->models[0]->paraNameChanged_coonectDist(max, min);
		para_val = (reval -min)/(max-min)*100;
	}
	else if(str == "connect_angle"){
		double reval = this->ui.gl_widget->models[0]->paraNameChanged_coonectAng(max, min);
		para_val = (reval -min)/(max-min)*100;
	}
	this->ui.testParaValue->setValue(para_val);
	this->ui.testParaValue->repaint();
	return;
}
void VDraw::paraValChanged(int val){
	val = this->ui.testParaValue->value();
	QString str = this->ui.testParaName->currentText();
	if(str == "Splitting Chains"){
		this->image_process->paraValChanged_split(val);
	}
	else if(str == "Show Curvature"){
	}
	else if(str == "connect_dist"){
		this->ui.gl_widget->models[0]->paraValChanged_coonectDist(val);
	}
	else if(str == "connect_angle"){
		this->ui.gl_widget->models[0]->paraValChanged_coonectAng(val);
	}
	
	catchGlRepaint();
	return;
}
void VDraw::showSimLine(bool chk){
	if(chk)
		this->image_process->setEdgeContourVisible();
	else
		this->image_process->setEdgeContourUnvisible();
	
	catchGlRepaint();
	return;
}
void VDraw::showPoseLine(bool chk){
	if(chk)
		this->image_process->setPoseLineVisible();
	else
		this->image_process->setPoseLineUnisible();
	
	catchGlRepaint();
	return;
}
void VDraw::showSCLines(bool chk){
	if(chk)
		this->image_process->setSCVisible();
	else
		this->image_process->setSCUnvisible();
	
	catchGlRepaint();
	return;
}
void VDraw::showConvexHull2D(bool chk){
	if(chk)
		this->image_process->setConvexHull2dVisible();
	else
		this->image_process->setConvexHull2dUnvisible();
	
	catchGlRepaint();
	return;
}
void VDraw::showPrimLines(bool chk){

	if(chk)
		this->image_process->setPrimLinesVisible();
	else
		this->image_process->setPrimLinesUnisible();
	
	catchGlRepaint();
	return;

}

void VDraw::setStella(){
	showPoseLine(true);
	showPrimLines(true);
	showConvexHull2D(true);
	showSCLines(true);
	showSimLine(true);
	QSize sz = ui.render_label->size();
	vector<vector<vector<NodeID>>> order;
	this->image_process->order2DStrokes();
	order.push_back(image_process->poseGraphOrder);
	order.push_back(image_process->simHullGraphOrder);
	order.push_back(image_process->SimEdgeGraphOrder);
	order.push_back(image_process->PrimsGraphOrder);
	order.push_back(image_process->EXSCGraphOrder);

	this->ui.dockWidget->show();
	ui.stellaWidget->setScale(sz.width(), sz.height());
	ui.stellaWidget->setGraphs(image_process->graphs, 5);
	ui.stellaWidget->setOrder(order);
	ui.stellaWidget->initialize();
	ui.stellaWidget->setImgProcess(image_process);
	ui.stellaWidget->repaint();
}