#include "vdraw.h"
#include "render\renderstroke.h"
#include <QtGui/QApplication>
#include "testwind.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	VDraw w;
	w.resize(800,600);
	//RenderStroke w;
	w.show();
	return a.exec();
}
