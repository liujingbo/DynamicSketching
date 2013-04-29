#ifndef VDRAW_H
#define VDRAW_H

#include <QtGui/QMainWindow>
#include <QScrollArea>
#include <QThread>

#include "ui_vdraw.h"
#include "GLWidget.h"
#include "imageProc.h"
#include "render\LinkLibs.h"
#include "render\renderstroke.h"

//**************************************************************************

/*
programming guideline:
1. all UI elements in vdraw
2. all OGL in GLWidget
3. all image processing in openCV

*/

//**************************************************************************

class VDraw : public QMainWindow
{
	Q_OBJECT

public:
	VDraw(QWidget *parent = 0, Qt::WFlags flags = 0);
	~VDraw();
	void updateRendering();
private:
	void iniWindow();
	//void vpCvImageToQImage(const IplImage* frame, QImage &qimage) ;
	void setQimageWithIpl(const IplImage *ipl_image);
	QImage * IplImageToQImage(const IplImage *img);
	
	// animation and timer
	QTimer animateOrderTimer;
	
	// render strokes
	RenderStroke renderCanvas;

public slots:
	void loadTriMesh();
	void loadSegmentation();
	void loadExoAtlas();
	void catchGlRepaint();
	void canvasRepaint(); // for efficiency, when change does not involve GL

	// render
	void renderDepthBuffer();
	void renderTextured();

	// filter
	void filterCanny();
	void filterHough();

	// commands
	void orderCurves();
	void stratAnimateOrder();

	// testing parameters
	void paraNameChanged(QString str);
	void paraValChanged(int val);
	void strokeNumChanged(int);
	void showSimLine(bool);
	void showPoseLine(bool);
	void showSCLines(bool);
	void showConvexHull2D(bool);
	void showPrimLines(bool);
	void setStella();

private slots:
	void showNextStroke();
private:
	Ui::VDrawClass ui;
	imageProc* image_process;

	QImage q_image;

	enum RENDERMODE{SIMPLE, DEPTH};
	enum FILTERMODE{NO_FILTER, CANNY, HOUGH};
	RENDERMODE renderMode;
	FILTERMODE filterMode;


	// structure backbones
	//QScrollArea* left_panel_scrollArea;
	//QHBoxLayout * layout; 


	//GLWidget* gl_widget;
};

#endif // VDRAW_H
