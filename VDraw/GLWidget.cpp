
#include <QtGui/QMouseEvent>
#include "GLWidget.h"

#define useSC 1

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent) {
	setMouseTracking(true);


	this->winWidth = this->width();
	this->winHeight = this->height();
	initializeMembers();

	// construct the Drawers we'll use in this demo
	// push into Mesh drawers when load the trimesh file
	b = new BaseDrawer();
	b1 = new EdgeContourDrawer(vec(0,1,0),1.0);
	b2 = new SuggestiveContourDrawer(vec(1,0,0), 1.0, true, 1);
	b3 = new PatchDrawer();
	b4 = new PrimitiveDrawer();
	//b3= new FaceContourDrawer(vec(0,0,1),2);
}
GLWidget::~GLWidget(){
	if(this->b){
		delete this->b;
	}
	if(this->b1){
		delete this->b1;
	}
	if(this->b2){
		delete this->b2;
	}
	if(this->b3){
		delete this->b3;
	}
	if(this->b4){
		delete this->b4;
	}
	// if(hierarchy !=NULL)delete this->hierarchy;

}


void GLWidget::initializeGL() {
	/*glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glEnable(GL_POLYGON_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 0);*/

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	//glutCreateWindow("Comp541 Mesh Viewer");

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glPolygonOffset(1.0, 1.0);

	// depth buffer
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);

	// light
	GLfloat light0Position[] = { 0, 1, 0, 1.0 }; 
	glLightfv (GL_LIGHT0, GL_POSITION, light0Position);
	glEnable(GL_LIGHT0);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	printf ("Checking for Vertex Buffer Object support ...");
	if (GLEW_ARB_vertex_buffer_object)
	{printf(" OK\n");}
	else{
		printf("No VBO support. This application requires at least OpenGL 1.4 with ARB extensions.\n");
		exit(3);
	}

	// move b drawers to constructor


	// loading mesh info
	/*
	this->LoadTriMeshFile("..\\exosk\\models\\horse.off");
	// exoskeleton
	if(this->LoadExoAtlasFile("..\\exosk\\patches\\horse.atlas")){
	b3->SetVertexColored(this->SetPatchNumber());
	}*/

	//this->LoadTriMeshFile("..\\testdata\\models\\cactus.obj");// cactus  gbones column cow.ply cylinder1p2 cylinder1.ply

	//this->LoadTriMeshFile("..\\princeton\\models\\390.off");

	//this->LoadTriMeshFile("..\\princeton\\models\\horse.ply");
	
	//this->LoadTriMeshFile("..\\princeton\\models\\1.off");
	this->LoadTriMeshFile("..\\testdata\\examples\\named\\mech_cyl.off"); 
	this->LoadAvailableColors();
	//b3->SetVertexColored(this->LoadPrincetonSegmentation("..\\princeton\\patches\\396\\396_1.seg"));

	//bool tmpb=this->LoadPrincetonSegmentation("..\\princeton\\patches\\1\\1_12.seg",0);
	bool tmpb=this->LoadPrincetonSegmentation("..\\testdata\\examples\\named\\mech_cyl.seg",0);

	b3->SetVertexColored(tmpb);
	b4->SetVertexColored(tmpb);

	this->models[0]->initPatches();

	//LoadFitting("..\\testdata\\fits\\1.fit",0);
	//LoadFitting("..\\testdata\\examples\\199_2.fit",0);
	LoadFitting("..\\testdata\\examples\\named\\mech_cyl.fit",0);
}

void GLWidget::initializeMembers(){
	//zNear = 1.0; zFar = 100.0;
	//g_fov = 45.0;
	//sdepth = 10;
	displayMode = FLATSHADED;

	sphi = 90.0; stheta = 45.0;// sdepth = 10;	// for simple trackball
	xpan = 0.0; ypan = 0.0;				// for simple trackball
	leftDown=middleDown= middleUp= shiftDown=false;	

	//sc 
	// toggle for diffuse lighting
	diffuse = true;
	buttonstate = 0;
	//sc end

	// 
	this->isRenderConvexHull2D = false;

	this->numOfDrawnLines =-1;

	// interactive fitting
	interacting_segment =0;

	// graph
	this->graph = NULL;// create when loading segmentation
}
void GLWidget::resizeGL(int w, int h) {
	winWidth = w;
	winHeight = h;
	//winAspect = (double)w/(double)h;
	glViewport(0, 0, w, h);
	//glutPostRedisplay();
	this->repaint();
}

// deliver GL information to the outside
GLfloat* GLWidget::depthBuffer(){
	GLfloat* depth_data = new GLfloat[this->width()*this->height()];
	glReadPixels (0, 0, this->width(), this->height(), GL_DEPTH_COMPONENT, GL_FLOAT, depth_data);
	return depth_data;
}

// Signal a redraw
void GLWidget::need_redraw()
{
	this->updateGL();//glutPostRedisplay();
	emit glRepaint();
}


void GLWidget::paintGL() {

	// call sc rendering 
	redraw();

	//glutSwapBuffers();
}

void GLWidget::mousePressEvent(QMouseEvent * me) {
	int button;
	switch(me->button()){
	case Qt::LeftButton://0x00000001
		button = GLUT_LEFT_BUTTON; //GLUT_LEFT_BUTTON = 0
		break;
	case Qt::RightButton:	//0x00000002
		button = GLUT_RIGHT_BUTTON; //GLUT_RIGHT_BUTTON = 2
		break;
	case Qt::MidButton:	//0x00000004
		button = GLUT_MIDDLE_BUTTON; //GLUT_MIDDLE_BUTTON = 1
		break;
	}

	this->mousebuttonfunc(button,GLUT_DOWN,me->x(),me->y(), me->modifiers());
	leftDown = (me->buttons() == Qt::LeftButton) ;

	this->setFocus();

}
void GLWidget::mouseMoveEvent(QMouseEvent * me) {

	if(leftDown )
		this->mousemotionfunc(me->x(),me->y());

}
void GLWidget::mouseReleaseEvent ( QMouseEvent * me ) {

	int button;
	switch(me->button()){
	case Qt::LeftButton://0x00000001
		button = GLUT_LEFT_BUTTON; //GLUT_LEFT_BUTTON = 0
		break;
	case Qt::RightButton:	//0x00000002
		button = GLUT_RIGHT_BUTTON; //GLUT_RIGHT_BUTTON = 2
		break;
	case Qt::MidButton:	//0x00000004
		button = GLUT_MIDDLE_BUTTON; //GLUT_MIDDLE_BUTTON = 1
		break;
	}

	this->mousebuttonfunc(button,GLUT_UP ,me->x(),me->y(),me->modifiers());
}

/**
* Handle keyboard events to toggle some functionalities in the drawers, for demonstration purposes in this sample
*/
void GLWidget::keyPressEvent(QKeyEvent* ke) {

	switch(ke->key()) {
	case Qt::Key_Escape:
		close();
		break;
	case Qt::Key_A: // toggle basedrawer
		b->toggleVisibility();
		printf ("Toggled Base Drawer Visiblity to %i \n", b->isVisible());
		break;
	case Qt::Key_Z: // toggle contourdrawer
		b1->toggleVisibility();
		printf ("Toggled Contour Drawer Visiblity to %i \n", b1->isVisible());
		break;
	case Qt::Key_E: // toggle suggestive contour drawer
		b2->toggleVisibility();
		printf ("Toggled Suggestive Contour Drawer Visibility to %i \n", b2->isVisible());
		break;
	case Qt::Key_F: // toggle suggestive contour fading
		b2->toggleFading();
		printf ("Toggled Suggestive Contour fading to %i \n", b2->isFaded());
		break;
	case Qt::Key_G: // toggle colored lines
		b2->setLineColor(vec(1.0,0.0,0.0));
		printf ("Suggestive Contour Lines in false color \n");
		break;
	case Qt::Key_H: // toggle colored lines
		b2->setLineColor(vec(0.0,0.0,0.0));
		printf ("Suggestive Contour Lines in black color \n");
		break;
	case Qt::Key_D: // toggle diffuse lighting
		diffuse = !diffuse;
		printf ("Toggled diffuse lighting to %i \n", diffuse);
		break;
	case Qt::Key_W: // dump image to file
		dump_image();
		break;
	case Qt::Key_C: // dump image to file
		//dump_image();
		b3->toggleCylinder();
		break;
	case Qt::Key_Left:
		b3->PrevStep();b4->PrevStep();
		break;
	case Qt::Key_Right:
		b3->NextStep();b4->NextStep();
		break;
	case Qt::Key_Up:
		this->numOfDrawnLines -= 1;
		//interacting_segment -= 1;
		//interacting_segment %= this->models[0]->fitting_cylinders.size();
		break;
	case Qt::Key_Down:
		this->numOfDrawnLines += 1;
		//interacting_segment += 1;
		//interacting_segment %= this->models[0]->fitting_cylinders.size();
		break;
	case Qt::Key_I:
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis[0] -=0.01;
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis.Normalize();
		break;
	case Qt::Key_K:
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis[0] +=0.01;
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis.Normalize();
		break;
	case Qt::Key_J:
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis[1] -=0.01;
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis.Normalize();
		break;
	case Qt::Key_L:
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis[1] +=0.01;
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis.Normalize();
		break;
	case Qt::Key_U:
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis[2] -=0.01;
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis.Normalize();
		break;
	case Qt::Key_O:
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis[2] +=0.01;
		this->models[0]->fitting_cylinders[interacting_segment]->cylinder_axis.Normalize();
		break;
	case Qt::Key_P:
		for(int i=0; i<this->models[0]->fitting_cylinders.size();i++){
			this->models[0]->fitting_cylinders[i]->OutputCylinders();
		}
		break;
	case Qt::Key_0:
	case Qt::Key_1:
	case Qt::Key_2:
	case Qt::Key_3:
	case Qt::Key_4:
	case Qt::Key_5:
	case Qt::Key_6:
	case Qt::Key_7:
	case Qt::Key_8:
	case Qt::Key_9:
		if(ke->key() >=48 && ke->key()-48<models[0]->fitting_cylinders.size()){
			interacting_segment = ke->key()-48;
		}
		break;
	default:
		ke->ignore();
		return;
		break;
	}

	need_redraw();
}

/**
* Handle mouse motions
* (from TriMesh2 library)
*/
void GLWidget::mousemotionfunc(int x, int y){
	static const Mouse::button map[] = {
		Mouse::NONE, Mouse::ROTATE, Mouse::MOVEXY, Mouse::MOVEZ,
		Mouse::MOVEZ, Mouse::MOVEXY, Mouse::MOVEXY, Mouse::MOVEXY,
	};

	// find out what exactly happened
	Mouse::button b = Mouse::NONE;
	if (buttonstate & (1 << 3))
		b = Mouse::WHEELUP;
	else if (buttonstate & (1 << 4))
		b = Mouse::WHEELDOWN;
	else if (buttonstate & (1 << 30))
		b = Mouse::LIGHT;
	else // hmm, it was something else
		b = map[buttonstate & 7];

	// pass mouse movement to camera
	camera.mouse(x, y, b,global_transf * global_bsph.center, global_bsph.r,global_transf);

	// if we identified something as mouse movement, force redisplay
	if (b != Mouse::NONE)
		need_redraw();
}

/**
* Handle mouse button clicks
* (from TriMesh2 library)
*/
void GLWidget::mousebuttonfunc(int button, int state, int x, int y, Qt::KeyboardModifiers mod){
	static timestamp last_click_time;
	static unsigned last_click_buttonstate = 0;
	if (mod == Qt::ControlModifier) //glutGetModifiers() & GLUT_ACTIVE_CTRL
		buttonstate |= (1 << 30);
	else
		buttonstate &= ~(1 << 30);

	if (state == GLUT_DOWN) {
		buttonstate |= (1 << button);
		last_click_time = now();
		last_click_buttonstate = buttonstate;
	}
	else {
		buttonstate &= ~(1 << button);
	}
	mousemotionfunc(x, y);
}


/**
* A simple GLUT-based OpenGL viewer.
* (Based on the TriMesh2 library code where indicated)
* author: Jeroen Baert
*/


/**
* Clears the OpenGL Draw and Depth buffer, resets all relevant OpenGL states
*/
void GLWidget::cls(){
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);
	glClearColor(1, 1, 1, 0);;
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
* Update the global bounding sphere
* (from TriMesh2 library)
*/
void GLWidget::update_boundingsphere(){
	// largest box possible
	point boxmin(1e38, 1e38, 1e38);
	point boxmax(-1e38, -1e38, -1e38);
	// find outer coords
	for (unsigned int i = 0; i < models.size(); i++){
		point c = transformations[i] * models[i]->mesh_->bsphere.center;
		float r = models[i]->mesh_->bsphere.r;
		for (int j = 0; j < 3; j++) {
			boxmin[j] = min(boxmin[j], c[j]-r);
			boxmax[j] = max(boxmax[j], c[j]+r);
		}
	}
	point &gc = global_bsph.center;
	float &gr = global_bsph.r;
	gc = 0.5f * (boxmin + boxmax);
	gr = 0.0f;
	// find largest possible radius for sphere
	for (unsigned int i = 0; i < models.size(); i++) {
		point c = transformations[i] * models[i]->mesh_->bsphere.center;
		float r = models[i]->mesh_->bsphere.r;
		gr = max(gr, dist(c, gc) + r);
	}
}

/**
* Reset the current view: undo all camera transformations.
*/
void GLWidget::resetview()
{
	// kill the cam
	camera.stopspin();
	// undo all model transformations
	for (unsigned int i = 0; i < models.size(); i++){
		transformations[i] = xform();
	}
	// recompute bounding sphere
	update_boundingsphere();
	// put ourselves in middle
	global_transf = xform::trans(0, 0, -5.0f * global_bsph.r)*xform::trans(-global_bsph.center);
}

/**
* Setup the OpenGL lighting
*/
void GLWidget::setup_lighting(){
	if(!diffuse){
		Color c(1.0f);
		glColor3fv(c);
		glDisable(GL_LIGHTING);
	}
	else{
		GLfloat light0_diffuse[] = { 0.85, 0.85, 0.85, 0.85 };
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_NORMALIZE);
	}
}

/**
* Reposition the camera and draw every model in the scene.
*/
void GLWidget::redraw(){
	// setup camera and push global transformations
	camera.setupGL(global_transf * global_bsph.center, global_bsph.r);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixd(global_transf);
	cls();

	// enable depth checking and backface culling
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// compute new camera position
	vec camera_pos = inv(global_transf) * point(0,0,0);

	// setup lighting
	setup_lighting();


	// draw every model
	for (unsigned int i = 0; i < models.size(); i++){
		double modelview[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		for(int j=0;j<16;j++){
			this->models[i]->modelview[j] = modelview[j];
		}

		double projection[16];
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		for(int j=0;j<16;j++){
			this->models[i]->projection[j] = projection[j];
		}

		int viewport[4] ;
		glGetIntegerv(GL_VIEWPORT, viewport); // x,y,w,h
		for(int j=0; j<4; j++){
			this->models[i]->viewport[j] = viewport[j];
		}

		glMatrixMode(GL_MODELVIEW);

		// push model-specific transformations
		glPushMatrix();
		glMultMatrixd(transformations[i]);
		// tell model to execute its drawer stack
		models[i]->draw(camera_pos);
		
		// face based sc test
		// ==============================================================
		glBegin(GL_TRIANGLES);
		glColor3f(0.1, 0.2, 0.3);
		const vector<TriMesh::Face>& fList   = this->models[i]->mesh_->faces;
		const vector<point>&	vList   = this->models[i]->mesh_->vertices;
		const vector<int>& scFaces = this->models[i]->scFaces;
		
		for(int j=0;j<scFaces.size();j++){
			TriMesh::Face f=fList[scFaces[j]];
			vec v0 = vList[f.v[0]];
			vec v1 = vList[f.v[1]];
			vec v2 = vList[f.v[2]];
		
			glVertex3d(v0[0],v0[1],v0[2]);
			glVertex3d(v1[0],v1[1],v1[2]);
			glVertex3d(v2[0],v2[1],v2[2]);
		}
		glEnd();
		// ==============================================================
		// draw primitives test
		if(false){
			for(int i=0;i<this->primitives.size();i++){
				Prism& p = this->primitives[i];
				for(int j=0;j<4;j++){
					glBegin(GL_LINES);
					int pi = 0+j;
					glVertex3d( p.points[pi][0],p.points[pi][1],p.points[pi][2]);
					pi= (1+j)%4;
					glVertex3d( p.points[pi][0],p.points[pi][1],p.points[pi][2]);
					glEnd();

					glBegin(GL_LINES);
					pi = 0+j +4;
					glVertex3d( p.points[pi][0],p.points[pi][1],p.points[pi][2]);
					pi= (1+j)%4 +4;
					glVertex3d( p.points[pi][0],p.points[pi][1],p.points[pi][2]);
					glEnd();

					glBegin(GL_LINES);
					pi = 0+j;
					glVertex3d( p.points[pi][0],p.points[pi][1],p.points[pi][2]);
					pi= 0 +j + 4;
					glVertex3d( p.points[pi][0],p.points[pi][1],p.points[pi][2]);
					glEnd();
				}
			}
		}
		// ==============================================================

		// pop again
		glPopMatrix();

		
	}
	// pop global transformations
	glPopMatrix();
	//glutSwapBuffers();
	// update FPS counter
	//fps->updateCounter();
	//std::stringstream out;
	//out << "Object Space Contours Demo | FPS: " << fps->FPS;
	//string s = out.str();
	//glutSetWindowTitle(s.c_str());


	//// render convex hull
	//if(this->isRenderConvexHull2D){
	//	glMatrixMode(GL_PROJECTION);
	//	glPushMatrix();
	//	glLoadIdentity();
	//	gluOrtho2D(0,this->width(),0,this->height());
	//	glMatrixMode(GL_MODELVIEW);
	//	glPushMatrix();
	//	glLoadIdentity();
	//	glDisable(GL_CULL_FACE);
	//	glDisable(GL_DEPTH_TEST);
	//	glDisable(GL_LIGHTING);
	//	glDepthMask(GL_FALSE);
	//	glColor3f(0,0,0);
	//	vector<vector<Point>>& hulls = this->models[0]->convex_hulls_2d;

	//	int accumulateLines=0;
	//	int totLines =0;

	//	for(int i=0;i<hulls.size();i++){
	//		totLines += hulls[i].size();
	//	}

	//	this->numOfDrawnLines = this->numOfDrawnLines % totLines;

	//	// ordered segment ~~ higher level ordering
	//	//vector<int> order= this->models[0]->hierarchy->order;

	//	for(int i=0;i<hulls.size();i++){
	//		int displayPatchNum = i;//order[i];
	//		glBegin(GL_LINE_STRIP);
	//		for(int j=0;j<hulls[displayPatchNum].size() && j<this->numOfDrawnLines - accumulateLines;j++){
	//			glVertex2i(hulls[displayPatchNum][j].x, hulls[displayPatchNum][j].y);
	//		}
	//		glEnd();
	//		if(this->numOfDrawnLines - accumulateLines >= hulls[displayPatchNum].size()){
	//			accumulateLines += hulls[displayPatchNum].size();
	//		}else{
	//			break;
	//		}
	//	}

	//	glEnable(GL_CULL_FACE);
	//	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_LIGHTING);
	//	glMatrixMode(GL_MODELVIEW);
	//	glPopMatrix();
	//	glMatrixMode(GL_PROJECTION);
	//	glPopMatrix();
	//}
}

/**
* Save the current image to a PPM file.
* (from TriMesh2 library)
*/
void GLWidget::dump_image()
{
	// Find first non-used filename
	const char filenamepattern[] = "img%d.ppm";
	int imgnum = 0;
	FILE *f;
	while (1) {
		char filename[1024];
		sprintf(filename, filenamepattern, imgnum++);
		f = fopen(filename, "rb");
		if (!f) {
			f = fopen(filename, "wb");
			printf("\n\nSaving image %s... ", filename);
			fflush(stdout);
			break;
		}
		fclose(f);
	}

	// Read pixels
	GLint V[4];
	glGetIntegerv(GL_VIEWPORT, V);
	GLint width = V[2], height = V[3];
	char *buf = new char[width*height*3];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(V[0], V[1], width, height, GL_RGB, GL_UNSIGNED_BYTE, buf);

	// Flip top-to-bottom
	for (int i = 0; i < height/2; i++) {
		char *row1 = buf + 3 * width * i;
		char *row2 = buf + 3 * width * (height - 1 - i);
		for (int j = 0; j < 3 * width; j++)
			swap(row1[j], row2[j]);
	}

	// Write out file
	fprintf(f, "P6\n#\n%d %d\n255\n", width, height);
	fwrite(buf, width*height*3, 1, f);
	fclose(f);
	delete [] buf;

	printf("Done.\n\n");
}


/**
* GLUT idle callback
* (from TriMesh2 library)
*/
void GLWidget::idle(){
	xform tmp_xf = global_transf;
	if (camera.autospin(tmp_xf)) // if the camera is still spinning
		glutPostRedisplay();
	else
		usleep(10000); // do nothing
	global_transf = tmp_xf;

}
void GLWidget::usage(const char *myname)
{   
	fprintf(stderr, "Usage: %s infile...\n", myname);
	exit(1);
}


bool GLWidget::LoadTriMeshFile(const char* str){
	//const char *filename = "..\\bin\\tablecloth.obj";
	const char *name = str;
	// creat model
	Model* m = new Model(name);
	// add drawers to model
	m->pushDrawer(b);
	m->pushDrawer(b1);
	m->pushDrawer(b2);
	m->pushDrawer(b3);
	m->pushDrawer(b4);

	// need to reset every thing here
	if( this->models.size()>0){
		Model* oldm = this->models[0];
		delete oldm;
		models.clear();

		this->graph = NULL;// already deleted by the model
	}
	models.push_back(m);
	// push back blank tranformation matrix
	transformations.push_back(xform());

	// initialize FPS counter
	//fps = new FPSCounter();

	resetview();
	this->repaint();
	return true;
}

bool GLWidget::LoadExoAtlasFile(const char* str){

	if (str==NULL || strlen(str)==0) return false;
	ifstream ifs(str);
	if (ifs.fail()) return false;
	ifs>>atlas;
	this->atlasLoaded=true;
	return true;
}

bool GLWidget::LoadAvailableColors(){
	const char str[] = "..\\princeton\\available_colors.txt";
	if (str==NULL || strlen(str)==0) return false;
	ifstream ifs(str);
	if (ifs.fail()) return false;
	int numofcolors;
	ifs>>numofcolors;

	this->availablecolors.clear();
	this->availablecolors.resize(numofcolors*3);

	for(int i=0; i< numofcolors*3; i++){
		float c;
		ifs>>c;
		availablecolors[i]=c;
	}

	return true;
}


/*
	set value to:
	facecolors_
	pointcolors_
	colorlist_
	segmentationSharedVertices

	create new graph
*/
bool GLWidget::LoadPrincetonSegmentation(const char* str, int modelnumber ){
	if (str==NULL || strlen(str)==0) return false;
	ifstream ifs(str);
	if (ifs.fail()) return false;

	const vector<TriMesh::Face>& fList = this->models[modelnumber]->mesh_->faces;
	const vector<point>& vList = this->models[modelnumber]->mesh_->vertices;
	vector<int>& facecolors_=  this->models[modelnumber]->facecolors_; // for patch color
	vector<int>& pointcolors_ = this->models[modelnumber]->pointcolors_;
	vector<float>& colorlist_ = this->models[modelnumber]->colorlist_;
	vector<vector<vector<int>>>& hEdge = this->models[modelnumber]->segmentationSharedVertices; // for hierachy graph

	int facelength = fList.size();

	// reset
	facecolors_.clear();
	pointcolors_.clear();
	colorlist_.clear();
	hEdge.clear();
	segment2NodeID.clear();
	if(this->graph != NULL){ delete this->graph;graph=NULL; }
	graph = new vGraph();

	facecolors_.resize(facelength);
	pointcolors_.resize(vList.size());

	// hierarchy edge init int[][]
	int segmentlength = 50;
	hEdge.resize(segmentlength); //50: must larger then the number of segments, int[a][b][]
	for(int i=0;i<segmentlength;i++){
		hEdge[i].resize(segmentlength);
	}

	for(int i=0; i<pointcolors_.size();i++){pointcolors_[i]=-1;}// color index # [0, ...] represents segment number, but length may not be the same here, later resize them to be the same
	int countmax=0;
	for(int i=0; i< facelength; i++){
		int tmppatchnumber;
		ifs>>tmppatchnumber;
		facecolors_[i] = tmppatchnumber;
		if(tmppatchnumber > countmax)countmax=tmppatchnumber;

		TriMesh::Face f = fList[i];

		// assign point color index, and find shared vertices
		for(int j=0;j<3;j++){
			if(pointcolors_[f.v[j]] != -1 && pointcolors_[f.v[j]] != tmppatchnumber){ // more then one patch number is assigned to one vertex -> the vertex is shared between segments

				int ind1 = max(pointcolors_[f.v[j]],tmppatchnumber);
				int ind2 = min(pointcolors_[f.v[j]],tmppatchnumber);
				hEdge[ind2][ind1].push_back(f.v[j]); // only store at [i][j] where i<j
				// hEdge[tmppatchnumber][pointcolors_[f.v[j]]].push_back(f.v[j]);
			}
			pointcolors_[f.v[j]]=(tmppatchnumber);
		}
	}
	// get the number of segments
	segmentlength = countmax+1;

	//initialize colors
	colorlist_.resize(segmentlength*3);
	int numofavailablecolors = this->availablecolors.size();
	assert(numofavailablecolors>0);
	for(int i=0; i<(countmax+1)*3; i++ ){
		colorlist_[i] = this->availablecolors[i%numofavailablecolors];
	}

	//***************************** build the graph*********************
	// add node
	for(int i=0;i<segmentlength;i++){
		vNode* segNode = new vNode(i);
		segNode->setVertices(&vList);//(this->models[0]->mesh_->vertices));
		NodeID nID = this->graph->addNode(segNode);
		this->segment2NodeID.push_back(nID);
	}
	for(int i=0;i<pointcolors_.size();i++){
		this->graph->getNode(segment2NodeID[pointcolors_[i]])->push_vertex_index(i);
	}
	for(int i=0;i<segmentlength;i++){
		this->graph->getNode(segment2NodeID[i])->computeCenter();
	}
	// add edges
	for(int i=0;i<segmentlength;i++){
		for(int j=i+1;j<segmentlength;j++){ // symmetric, and make sure j>i
			if(hEdge[i][j].size() > 10 ){
				vEdge* e = new vEdge();
				e->copy_vertices_index_array(hEdge[i][j]);// shared vertices
				e->setVertices(&vList);
				e->computeCenter();
				this->graph->addEdge( this->segment2NodeID[i],this->segment2NodeID[j],e);
			}
		}
	}

	// share it to model - or should say the graph belongs to the model
	this->models[modelnumber]->setGraph(this->graph, this->segment2NodeID);
	//******************************************************************

	//// resize hierarchy edge to save space & set edge costs
	//vector<vector<int>> Edges;
	//hEdge.resize(segmentlength);
	//int maxCost=-1;
	//for(int i=0;i<segmentlength;i++){
	//	hEdge[i].resize(segmentlength);
	//	for(int j=i;j<segmentlength;j++){ // symmetric, and make sure j>i
	//		if(hEdge[i][j].size() > 0 && i!=j){
	//			vector<int> e;
	//			e.resize(3);
	//			e[0]=i;
	//			e[1]=j;
	//			e[2] = hEdge[i][j].size();
	//			Edges.push_back(e);
	//			if(e[2]>maxCost) maxCost=e[2];
	//		}
	//	}
	//}
	//maxCost += 100;
	//// convert from max spanning -> MST
	//for(int i=0;i<Edges.size();i++){
	//	Edges[i][2] = maxCost-Edges[i][2];
	//}

	//// add symmetry info
	//const int symCost=15;
	//vector<int> legs[3];
	//for(int i=0;i<3; i++){
	//	legs[i].resize(3);
	//	legs[i][0]=(i%4)+2;
	//	legs[i][1]=(i%4)+3;
	//	legs[i][2]=symCost;
	//	Edges.push_back(legs[i]);
	//}
	//vector<int> leg4;
	//leg4.resize(3);
	//leg4[0]=2;leg4[1]=5;leg4[2]=symCost;
	//Edges.push_back(leg4);

	//// covered priorities
	//const int coveredBonus=5;
	//for(int i=0;i<Edges.size();i++){
	//	if(Edges[i][0] == 2 || Edges[i][1] == 2 || Edges[i][0] == 5 || Edges[i][1] == 5){
	//		Edges[i][2] -= coveredBonus;
	//	}
	//}

	//// overlap relation priority
	//const int overlapBonus=5;
	//for(int i=0;i<Edges.size();i++){
	//	if(Edges[i][0]==4 && Edges[i][1]==5) Edges[i][2] -= overlapBonus;
	//	if(Edges[i][0]==2 && Edges[i][1]==3) Edges[i][2] -= overlapBonus;
	//}

	//// compute heirarchy 
	//hierarchy= new HierarchyGraph(segmentlength, Edges.size(),Edges);
	//this->models[modelnumber]->setHierarchyGraph(hierarchy);


	// other operations
	b3->SetVertexColored(true);
	b4->SetVertexColored(true);
	models[0]->initPatches();
	this->models[0]->buildPoseLines();

	return true;
}

// load Pengfei's segmentation results
bool GLWidget::LoadFitting(const char* str, int modelnumber){
	if (str==NULL || strlen(str)==0) return false;
	fstream ifs(str, ios::in);
	if (ifs.fail()) return false;

	//if(this->models[modelnumber]->)
	int a;
	ifs>>a;

	primitives.clear();
	while(ifs.good()){
		string type;
		ifs >> type;
		if(type == "prism"){
			Prism prism;

			double planes[6][4];
			vec points[8]; 
	
			// add planes
			for(int i=0;i<6;i++){
				for(int j=0;j<4;j++){
					ifs>>planes[i][j];
				}
			}

			// add points
			for(int i=0;i<8;i++){
				double tmp_p[3]={0};
				ifs >> tmp_p[0] >> tmp_p[1] >> tmp_p[2];
				points[i] = vec(tmp_p[0] , tmp_p[1], tmp_p[2]);
			}

			prism.addPlanes(planes);
			prism.addPoints(points);

			primitives.push_back(prism);
		}
	}
	return true;
}


/**
* Set the color of each face from the atlas result
*
* @param modelnumber : the model number (from the model list)
*/

bool GLWidget::SetPatchNumber(int modelnumber){

	if(!this->atlasLoaded) return false;

	const std::vector<int>& facelabels = this->atlas.Labels;
	const std::vector<float>&   colors = this->atlas.Colors;

	const vector<TriMesh::Face>& fList = this->models[modelnumber]->mesh_->faces;
	vector<int>& facecolors_=  this->models[modelnumber]->facecolors_; // for patch color
	vector<int>& pointcolors_ = this->models[modelnumber]->pointcolors_;
	vector<float>& colorlist_ = this->models[modelnumber]->colorlist_;

	facecolors_.clear();
	pointcolors_.clear();
	colorlist_.clear();

	int facelength= facelabels.size();
	facecolors_.resize(facelength);

	int pointlength = this->models[modelnumber]->mesh_->vertices.size();
	pointcolors_.resize(pointlength);

	int colorlength_ = colors.size();
	colorlist_.resize(colorlength_);
	for(int i=0; i< colorlength_; i++) {colorlist_[i] =colors[i]; }

	for(int i=0; i< facelength; i++){
		//facecolors_[i*3]=   colors[(facelabels[i])*3];
		//facecolors_[i*3+1]=   colors[(facelabels[i])*3+1];
		//facecolors_[i*3+2]=   colors[(facelabels[i])*3+2];

		facecolors_[i]=   facelabels[i];

		TriMesh::Face f = fList[i];

		pointcolors_[f.v[0]] = (facelabels[i]);
		pointcolors_[f.v[1]] = (facelabels[i]);
		pointcolors_[f.v[2]] = (facelabels[i]);
	}

	return true;

}