/********************************************************************************
** Form generated from reading UI file 'renderstroke.ui'
**
** Created: Fri Apr 5 00:38:34 2013
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RENDERSTROKE_H
#define UI_RENDERSTROKE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include "strokeinputwidget.h"
#include "strokedisplaywidget.h"

QT_BEGIN_NAMESPACE

class Ui_RenderStrokeClass
{
public:
    QAction *actionOpen_Stroke;
    QAction *actionStroke_Animation;
    QAction *actionClear_Strokes;
    QAction *actionType_1;
    QAction *actionType_2;
    QAction *actionZoom_In;
    QAction *actionZoom_Out;
    QAction *actionZoom_Original;
    QAction *actionOpen_Pencil_Texture;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QScrollArea *scrollArea_strokeInput;
    QWidget *scrollAreaWidgetContents_2;
    QGridLayout *gridLayout_3;
    StrokeInputWidget *strokeInput;
    QScrollArea *scrollArea_strokeDisplay;
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout_2;
    StrokeDisplayWidget *strokeDisplay;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuStroke_Type;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget_controlPanel;
    QWidget *dockWidgetContents;
    QGroupBox *groupBox;
    QSlider *horizontalSlider_animationSpeed;
    QLabel *label;

    void setupUi(QMainWindow *RenderStrokeClass)
    {
        if (RenderStrokeClass->objectName().isEmpty())
            RenderStrokeClass->setObjectName(QString::fromUtf8("RenderStrokeClass"));
        RenderStrokeClass->resize(1038, 492);
        actionOpen_Stroke = new QAction(RenderStrokeClass);
        actionOpen_Stroke->setObjectName(QString::fromUtf8("actionOpen_Stroke"));
        actionStroke_Animation = new QAction(RenderStrokeClass);
        actionStroke_Animation->setObjectName(QString::fromUtf8("actionStroke_Animation"));
        actionClear_Strokes = new QAction(RenderStrokeClass);
        actionClear_Strokes->setObjectName(QString::fromUtf8("actionClear_Strokes"));
        actionType_1 = new QAction(RenderStrokeClass);
        actionType_1->setObjectName(QString::fromUtf8("actionType_1"));
        actionType_1->setCheckable(true);
        actionType_2 = new QAction(RenderStrokeClass);
        actionType_2->setObjectName(QString::fromUtf8("actionType_2"));
        actionType_2->setCheckable(true);
        actionZoom_In = new QAction(RenderStrokeClass);
        actionZoom_In->setObjectName(QString::fromUtf8("actionZoom_In"));
        actionZoom_Out = new QAction(RenderStrokeClass);
        actionZoom_Out->setObjectName(QString::fromUtf8("actionZoom_Out"));
        actionZoom_Original = new QAction(RenderStrokeClass);
        actionZoom_Original->setObjectName(QString::fromUtf8("actionZoom_Original"));
        actionOpen_Pencil_Texture = new QAction(RenderStrokeClass);
        actionOpen_Pencil_Texture->setObjectName(QString::fromUtf8("actionOpen_Pencil_Texture"));
        centralWidget = new QWidget(RenderStrokeClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        scrollArea_strokeInput = new QScrollArea(centralWidget);
        scrollArea_strokeInput->setObjectName(QString::fromUtf8("scrollArea_strokeInput"));
        scrollArea_strokeInput->setWidgetResizable(true);
        scrollAreaWidgetContents_2 = new QWidget();
        scrollAreaWidgetContents_2->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_2"));
        scrollAreaWidgetContents_2->setGeometry(QRect(0, 0, 413, 408));
        gridLayout_3 = new QGridLayout(scrollAreaWidgetContents_2);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        strokeInput = new StrokeInputWidget(scrollAreaWidgetContents_2);
        strokeInput->setObjectName(QString::fromUtf8("strokeInput"));
        strokeInput->setAutoFillBackground(false);

        gridLayout_3->addWidget(strokeInput, 1, 0, 1, 1);

        scrollArea_strokeInput->setWidget(scrollAreaWidgetContents_2);

        gridLayout->addWidget(scrollArea_strokeInput, 0, 0, 1, 1);

        scrollArea_strokeDisplay = new QScrollArea(centralWidget);
        scrollArea_strokeDisplay->setObjectName(QString::fromUtf8("scrollArea_strokeDisplay"));
        scrollArea_strokeDisplay->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 413, 408));
        gridLayout_2 = new QGridLayout(scrollAreaWidgetContents);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        strokeDisplay = new StrokeDisplayWidget(scrollAreaWidgetContents);
        strokeDisplay->setObjectName(QString::fromUtf8("strokeDisplay"));

        gridLayout_2->addWidget(strokeDisplay, 1, 0, 1, 1);

        scrollArea_strokeDisplay->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea_strokeDisplay, 0, 1, 1, 1);

        RenderStrokeClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(RenderStrokeClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1038, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuStroke_Type = new QMenu(menuBar);
        menuStroke_Type->setObjectName(QString::fromUtf8("menuStroke_Type"));
        RenderStrokeClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(RenderStrokeClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        RenderStrokeClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(RenderStrokeClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        RenderStrokeClass->setStatusBar(statusBar);
        dockWidget_controlPanel = new QDockWidget(RenderStrokeClass);
        dockWidget_controlPanel->setObjectName(QString::fromUtf8("dockWidget_controlPanel"));
        dockWidget_controlPanel->setMinimumSize(QSize(180, 38));
        dockWidget_controlPanel->setMaximumSize(QSize(180, 524287));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        groupBox = new QGroupBox(dockWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(0, 20, 171, 101));
        horizontalSlider_animationSpeed = new QSlider(groupBox);
        horizontalSlider_animationSpeed->setObjectName(QString::fromUtf8("horizontalSlider_animationSpeed"));
        horizontalSlider_animationSpeed->setGeometry(QRect(50, 20, 111, 19));
        horizontalSlider_animationSpeed->setOrientation(Qt::Horizontal);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(12, 20, 30, 13));
        dockWidget_controlPanel->setWidget(dockWidgetContents);
        RenderStrokeClass->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget_controlPanel);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuStroke_Type->menuAction());
        menuFile->addAction(actionOpen_Stroke);
        menuFile->addAction(actionOpen_Pencil_Texture);
        menuEdit->addAction(actionStroke_Animation);
        menuEdit->addAction(actionClear_Strokes);
        menuEdit->addAction(actionZoom_In);
        menuEdit->addAction(actionZoom_Out);
        menuEdit->addAction(actionZoom_Original);
        menuStroke_Type->addAction(actionType_1);
        menuStroke_Type->addAction(actionType_2);
        mainToolBar->addAction(actionStroke_Animation);
        mainToolBar->addAction(actionClear_Strokes);
        mainToolBar->addAction(actionZoom_In);
        mainToolBar->addAction(actionZoom_Out);
        mainToolBar->addAction(actionZoom_Original);

        retranslateUi(RenderStrokeClass);
        QObject::connect(actionOpen_Stroke, SIGNAL(triggered()), RenderStrokeClass, SLOT(OpenStroke()));
        QObject::connect(actionStroke_Animation, SIGNAL(triggered()), RenderStrokeClass, SLOT(StrokeAnimation()));
        QObject::connect(actionClear_Strokes, SIGNAL(triggered()), RenderStrokeClass, SLOT(ClearStrokes()));
        QObject::connect(horizontalSlider_animationSpeed, SIGNAL(valueChanged(int)), RenderStrokeClass, SLOT(ChangeAnimationSpeed(int)));
        QObject::connect(actionType_1, SIGNAL(triggered()), RenderStrokeClass, SLOT(ChangeStrokeType1()));
        QObject::connect(actionType_2, SIGNAL(triggered()), RenderStrokeClass, SLOT(ChangeStrokeType2()));
        QObject::connect(actionZoom_In, SIGNAL(triggered()), RenderStrokeClass, SLOT(ZoomIn()));
        QObject::connect(actionZoom_Out, SIGNAL(triggered()), RenderStrokeClass, SLOT(ZoomOut()));
        QObject::connect(actionZoom_Original, SIGNAL(triggered()), RenderStrokeClass, SLOT(ZoomOriginal()));
        QObject::connect(actionOpen_Pencil_Texture, SIGNAL(triggered()), RenderStrokeClass, SLOT(OpenPencilTexture()));

        QMetaObject::connectSlotsByName(RenderStrokeClass);
    } // setupUi

    void retranslateUi(QMainWindow *RenderStrokeClass)
    {
        RenderStrokeClass->setWindowTitle(QApplication::translate("RenderStrokeClass", "RenderStroke", 0, QApplication::UnicodeUTF8));
        actionOpen_Stroke->setText(QApplication::translate("RenderStrokeClass", "Open Stroke", 0, QApplication::UnicodeUTF8));
        actionStroke_Animation->setText(QApplication::translate("RenderStrokeClass", "Stroke Animation", 0, QApplication::UnicodeUTF8));
        actionClear_Strokes->setText(QApplication::translate("RenderStrokeClass", "Clear Strokes", 0, QApplication::UnicodeUTF8));
        actionType_1->setText(QApplication::translate("RenderStrokeClass", "Type 1", 0, QApplication::UnicodeUTF8));
        actionType_2->setText(QApplication::translate("RenderStrokeClass", "Type 2", 0, QApplication::UnicodeUTF8));
        actionZoom_In->setText(QApplication::translate("RenderStrokeClass", "Zoom In", 0, QApplication::UnicodeUTF8));
        actionZoom_Out->setText(QApplication::translate("RenderStrokeClass", "Zoom Out", 0, QApplication::UnicodeUTF8));
        actionZoom_Original->setText(QApplication::translate("RenderStrokeClass", "Zoom Original", 0, QApplication::UnicodeUTF8));
        actionOpen_Pencil_Texture->setText(QApplication::translate("RenderStrokeClass", "Open Pencil Texture", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("RenderStrokeClass", "File", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("RenderStrokeClass", "Edit", 0, QApplication::UnicodeUTF8));
        menuStroke_Type->setTitle(QApplication::translate("RenderStrokeClass", "Stroke Type", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("RenderStrokeClass", "Animation", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("RenderStrokeClass", "Speed", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class RenderStrokeClass: public Ui_RenderStrokeClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENDERSTROKE_H
