#ifndef _GLWIDGET_H
#define _GLWIDGET_H

// sc
#include <GL/glew.h> // OpenGL Extension Wrangler functions
#include <GL/gl.h> // OpenGL functions itself
#include <GL/glut.h> // FreeGlut window management
#include "XForm.h" // xform to keep track of transformations (part of TriMesh2)
#include "GLCamera.h" // OpenGL camera (part of Trimesh2)
#include <string>
#include <sstream>

#include "sc_cpu/Model.h"
#include "sc_cpu/BaseDrawer.h"
#include "sc_cpu/EdgeContourDrawer.h"
#include "sc_cpu/FaceContourDrawer.h"
#include "sc_cpu/SuggestiveContourDrawer.h"
#include "PrimitiveDrawer.h"
//#include "sc_cpu/FPSCounter.h"

using namespace std; 
using std::string;
//end sc

#include <QtOpenGL/QGLWidget>

#include "vector3.h"
//#include "matrix.h"
// #include "mesh.h"
// #include "OpenGLProjector.h"
#include "Vec.h"

// segmentation exoskeleton 
#include "atlasIO.h"
#include <vector>
#include "PatchDrawer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// hierarchy graph
// #include "../ordering/HierarchyGraph.h"
#include "ordering/vGraph.h"
// primitives
#include "fitting/Primitives.h"
#include <string>

class GLWidget : public QGLWidget {

	// default 800*600 -> set minimum size for GL widget in scroll area
	// may need to set minimum size again everytime resize GLWidget

    Q_OBJECT // must include this if you use Qt signals/slots

public:
	// *********************************************
	//           display 
	// **********************************************
	// set bounding box
	// --Enumeration
	enum EnumDisplayMode { WIREFRAME, HIDDENLINE, FLATSHADED, SMOOTHSHADED };
	int displayMode;	// current display mode
	
	// --for simple trackball
	int lastX, lastY;				// last mouse motion position
	bool leftDown, middleDown, middleUp, shiftDown;		// mouse down and shift down flags
	double sphi, stheta;	// for simple trackball
	double xpan, ypan;				// for simple trackball
	int winWidth, winHeight;

	// mesh related
	// Mesh mesh; // not used in sc, sc uses model
	AtlasIO atlas; // one type of segmentation
	bool atlasLoaded;
	vector<int> princetonsegmentation;
	vector<float> availablecolors;

	// interactive fitting
	int interacting_segment;

	// *********************************************
	//           Hierarchy Graph
	// **********************************************
	// HierarchyGraph* hierarchy;
	vGraph* graph;
	std::vector<NodeID> segment2NodeID;
	std::vector<Prism> primitives;

	// *********************************************
	//           3D curves
	// **********************************************
	// store and deliver curves outside
	// bridge between curve manager and drawers
	// if edge contour drawer is enabled, then this is ready

	// deliver transformation matrix
	double* getModelViewMatrix(int ind){ if(this->models.size()<=0) return NULL; else return this->models[ind]->modelview; }
	double* getProjectionMatrix(int ind){if(this->models.size()<=0) return NULL; else return this->models[ind]->projection;}
	int* getViewPort(int ind){if(this->models.size()<=0) return NULL; else return this->models[ind]->viewport;}
	vector<vector<vec2>> * getConvexHull(int ind){if(this->models.size()<=0) return NULL; else return &(this->models[ind]->convex_hulls_2d);}

	// *********************************************
	//           suggestive contour  
	// **********************************************
	
	// Global variables (if this Viewer were a class, this would be its attributes)
	vector<Model*> models; // the model list
	vector<xform> transformations; // model transformations
	TriMesh::BSphere global_bsph; // global boundingbox
	xform global_transf; // global transformations
	GLCamera camera; // global camera

	// our fps counter
	//FPSCounter* fps;

	// The drawers we'll use in this demo
	BaseDrawer* b;
	EdgeContourDrawer* b1;
	SuggestiveContourDrawer* b2;
	PatchDrawer* b3;
	PrimitiveDrawer* b4;

	// toggle for diffuse lighting
	bool diffuse;
	// toggle for convex hull
	bool isRenderConvexHull2D;

	// convex hull demos
	int numOfDrawnLines;
	
	unsigned buttonstate;

	void cls();
	void update_boundingsphere();
	void resetview();
	void setup_lighting();
	void redraw();
	void dump_image();
	void mousemotionfunc(int x, int y);
	void mousebuttonfunc(int button, int state, int x, int y, Qt::KeyboardModifiers mod);
	void keyboardfunc(unsigned char key, int x, int y);
	void idle();
	
	void usage(const char *myname);


public:
    GLWidget(QWidget *parent = NULL);
	~GLWidget();
	bool LoadObjFile(const char *filename);
	bool LoadTriMeshFile(const char* str);
	bool LoadExoAtlasFile(const char* str);
	bool LoadPrincetonSegmentation(const char* str, int modelnumber =0);
	bool LoadFitting(const char* str, int modelnumber =0);
	bool LoadAvailableColors();
	bool SetPatchNumber(int modelnumber =0);

	GLfloat* depthBuffer();
	void need_redraw();

	public slots:
signals:
	void glRepaint();


protected:
    void initializeGL();
	void initializeMembers();
    void resizeGL(int w, int h);
    void paintGL();
    
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent ( QMouseEvent * event ) ;
    void keyPressEvent(QKeyEvent *event);

	void InitGeometry();
	

	void DrawWireframe();
	void DrawHiddenLine();
	void DrawFlatShaded();
	void DrawSmoothShaded();
};

#endif  /* _GLWIDGET_H */