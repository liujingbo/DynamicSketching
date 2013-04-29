#ifndef TESTWIND_H
#define TESTWIND_H


#include <QtGui/QMainWindow>
#include <QtGui/QtGui>

class testWind : public QMainWindow
{
    Q_OBJECT

public:
    testWind(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~testWind();
};

#endif