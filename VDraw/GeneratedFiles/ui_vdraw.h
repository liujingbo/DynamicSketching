/********************************************************************************
** Form generated from reading UI file 'vdraw.ui'
**
** Created: Fri Apr 5 04:15:07 2013
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VDRAW_H
#define UI_VDRAW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include "glwidget.h"
#include "stellawidget.h"

QT_BEGIN_NAMESPACE

class Ui_VDrawClass
{
public:
    QAction *actionLoad_mesh;
    QAction *actionDepth_Buffer;
    QAction *actionCanny;
    QAction *actionHough_Transform;
    QAction *actionContour;
    QAction *actionSuggestive_Contour;
    QAction *actionDiffuse;
    QAction *actionSegmentation;
    QAction *actionPrimitive_fitting;
    QAction *actionLoad_segmentation;
    QAction *actionStella_Prepare;
    QAction *stellaStartItem;
    QAction *actionStart;
    QAction *actionTextured_Rendering;
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QWidget *backgroud_widget;
    QGridLayout *gridLayout;
    QSplitter *splitter;
    QWidget *left_panel_widget;
    QGridLayout *gridLayout_3;
    QTabWidget *left_panel_tab;
    QWidget *tab_gl;
    QGridLayout *gridLayout_4;
    QScrollArea *scrollArea;
    GLWidget *gl_widget;
    QWidget *tab_gl2;
    QScrollArea *scrollArea_2;
    GLWidget *gl_widget2;
    QWidget *right_panel_widget;
    QGridLayout *gridLayout_5;
    QLabel *render_label;
    QGroupBox *testing_zone;
    QListWidget *output_list;
    QComboBox *testParaName;
    QSlider *testParaValue;
    QGroupBox *checking_zone;
    QCheckBox *checkBox;
    QCheckBox *checkPrims;
    QCheckBox *checkBox_3;
    QCheckBox *checkPose;
    QCheckBox *checkSimLines;
    QPushButton *pushButton;
    QSlider *horizontalSlider;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuRender;
    QMenu *menuFilter;
    QMenu *menuGL;
    QMenu *menuEffects;
    QMenu *menuStella;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout_6;
    StellaWidget *stellaWidget;

    void setupUi(QMainWindow *VDrawClass)
    {
        if (VDrawClass->objectName().isEmpty())
            VDrawClass->setObjectName(QString::fromUtf8("VDrawClass"));
        VDrawClass->resize(1101, 700);
        VDrawClass->setMinimumSize(QSize(1100, 700));
        VDrawClass->setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::ForceTabbedDocks);
        actionLoad_mesh = new QAction(VDrawClass);
        actionLoad_mesh->setObjectName(QString::fromUtf8("actionLoad_mesh"));
        actionDepth_Buffer = new QAction(VDrawClass);
        actionDepth_Buffer->setObjectName(QString::fromUtf8("actionDepth_Buffer"));
        actionDepth_Buffer->setCheckable(false);
        actionCanny = new QAction(VDrawClass);
        actionCanny->setObjectName(QString::fromUtf8("actionCanny"));
        actionCanny->setCheckable(true);
        actionHough_Transform = new QAction(VDrawClass);
        actionHough_Transform->setObjectName(QString::fromUtf8("actionHough_Transform"));
        actionHough_Transform->setCheckable(true);
        actionContour = new QAction(VDrawClass);
        actionContour->setObjectName(QString::fromUtf8("actionContour"));
        actionSuggestive_Contour = new QAction(VDrawClass);
        actionSuggestive_Contour->setObjectName(QString::fromUtf8("actionSuggestive_Contour"));
        actionDiffuse = new QAction(VDrawClass);
        actionDiffuse->setObjectName(QString::fromUtf8("actionDiffuse"));
        actionSegmentation = new QAction(VDrawClass);
        actionSegmentation->setObjectName(QString::fromUtf8("actionSegmentation"));
        actionPrimitive_fitting = new QAction(VDrawClass);
        actionPrimitive_fitting->setObjectName(QString::fromUtf8("actionPrimitive_fitting"));
        actionLoad_segmentation = new QAction(VDrawClass);
        actionLoad_segmentation->setObjectName(QString::fromUtf8("actionLoad_segmentation"));
        actionStella_Prepare = new QAction(VDrawClass);
        actionStella_Prepare->setObjectName(QString::fromUtf8("actionStella_Prepare"));
        stellaStartItem = new QAction(VDrawClass);
        stellaStartItem->setObjectName(QString::fromUtf8("stellaStartItem"));
        actionStart = new QAction(VDrawClass);
        actionStart->setObjectName(QString::fromUtf8("actionStart"));
        actionTextured_Rendering = new QAction(VDrawClass);
        actionTextured_Rendering->setObjectName(QString::fromUtf8("actionTextured_Rendering"));
        centralWidget = new QWidget(VDrawClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        backgroud_widget = new QWidget(centralWidget);
        backgroud_widget->setObjectName(QString::fromUtf8("backgroud_widget"));
        backgroud_widget->setMinimumSize(QSize(1000, 500));
        backgroud_widget->setLayoutDirection(Qt::LeftToRight);
        gridLayout = new QGridLayout(backgroud_widget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        splitter = new QSplitter(backgroud_widget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setFrameShape(QFrame::Panel);
        splitter->setOrientation(Qt::Horizontal);
        splitter->setOpaqueResize(false);
        left_panel_widget = new QWidget(splitter);
        left_panel_widget->setObjectName(QString::fromUtf8("left_panel_widget"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(left_panel_widget->sizePolicy().hasHeightForWidth());
        left_panel_widget->setSizePolicy(sizePolicy1);
        left_panel_widget->setMinimumSize(QSize(500, 0));
        gridLayout_3 = new QGridLayout(left_panel_widget);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        left_panel_tab = new QTabWidget(left_panel_widget);
        left_panel_tab->setObjectName(QString::fromUtf8("left_panel_tab"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(50);
        sizePolicy2.setVerticalStretch(50);
        sizePolicy2.setHeightForWidth(left_panel_tab->sizePolicy().hasHeightForWidth());
        left_panel_tab->setSizePolicy(sizePolicy2);
        left_panel_tab->setMinimumSize(QSize(200, 0));
        tab_gl = new QWidget();
        tab_gl->setObjectName(QString::fromUtf8("tab_gl"));
        gridLayout_4 = new QGridLayout(tab_gl);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        scrollArea = new QScrollArea(tab_gl);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        sizePolicy.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
        scrollArea->setSizePolicy(sizePolicy);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        gl_widget = new GLWidget();
        gl_widget->setObjectName(QString::fromUtf8("gl_widget"));
        gl_widget->setGeometry(QRect(0, 0, 693, 500));
        gl_widget->setMinimumSize(QSize(500, 500));
        scrollArea->setWidget(gl_widget);

        gridLayout_4->addWidget(scrollArea, 0, 0, 1, 1);

        left_panel_tab->addTab(tab_gl, QString());
        tab_gl2 = new QWidget();
        tab_gl2->setObjectName(QString::fromUtf8("tab_gl2"));
        scrollArea_2 = new QScrollArea(tab_gl2);
        scrollArea_2->setObjectName(QString::fromUtf8("scrollArea_2"));
        scrollArea_2->setGeometry(QRect(0, 0, 500, 500));
        sizePolicy.setHeightForWidth(scrollArea_2->sizePolicy().hasHeightForWidth());
        scrollArea_2->setSizePolicy(sizePolicy);
        scrollArea_2->setMinimumSize(QSize(500, 500));
        scrollArea_2->setWidgetResizable(true);
        gl_widget2 = new GLWidget();
        gl_widget2->setObjectName(QString::fromUtf8("gl_widget2"));
        gl_widget2->setGeometry(QRect(0, 0, 498, 498));
        scrollArea_2->setWidget(gl_widget2);
        left_panel_tab->addTab(tab_gl2, QString());

        gridLayout_3->addWidget(left_panel_tab, 0, 0, 1, 1);

        splitter->addWidget(left_panel_widget);
        right_panel_widget = new QWidget(splitter);
        right_panel_widget->setObjectName(QString::fromUtf8("right_panel_widget"));
        sizePolicy.setHeightForWidth(right_panel_widget->sizePolicy().hasHeightForWidth());
        right_panel_widget->setSizePolicy(sizePolicy);
        gridLayout_5 = new QGridLayout(right_panel_widget);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        render_label = new QLabel(right_panel_widget);
        render_label->setObjectName(QString::fromUtf8("render_label"));
        sizePolicy2.setHeightForWidth(render_label->sizePolicy().hasHeightForWidth());
        render_label->setSizePolicy(sizePolicy2);
        render_label->setMaximumSize(QSize(500, 500));

        gridLayout_5->addWidget(render_label, 0, 0, 1, 1);

        splitter->addWidget(right_panel_widget);

        gridLayout->addWidget(splitter, 0, 0, 1, 1);


        gridLayout_2->addWidget(backgroud_widget, 1, 0, 1, 1);

        testing_zone = new QGroupBox(centralWidget);
        testing_zone->setObjectName(QString::fromUtf8("testing_zone"));
        output_list = new QListWidget(testing_zone);
        output_list->setObjectName(QString::fromUtf8("output_list"));
        output_list->setGeometry(QRect(20, 20, 371, 71));
        sizePolicy1.setHeightForWidth(output_list->sizePolicy().hasHeightForWidth());
        output_list->setSizePolicy(sizePolicy1);
        testParaName = new QComboBox(testing_zone);
        testParaName->setObjectName(QString::fromUtf8("testParaName"));
        testParaName->setGeometry(QRect(412, 22, 104, 20));
        testParaValue = new QSlider(testing_zone);
        testParaValue->setObjectName(QString::fromUtf8("testParaValue"));
        testParaValue->setGeometry(QRect(522, 22, 211, 19));
        testParaValue->setOrientation(Qt::Horizontal);
        checking_zone = new QGroupBox(testing_zone);
        checking_zone->setObjectName(QString::fromUtf8("checking_zone"));
        checking_zone->setGeometry(QRect(410, 40, 371, 51));
        checkBox = new QCheckBox(checking_zone);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(280, 20, 51, 17));
        checkBox->setChecked(false);
        checkPrims = new QCheckBox(checking_zone);
        checkPrims->setObjectName(QString::fromUtf8("checkPrims"));
        checkPrims->setGeometry(QRect(140, 20, 51, 17));
        checkBox_3 = new QCheckBox(checking_zone);
        checkBox_3->setObjectName(QString::fromUtf8("checkBox_3"));
        checkBox_3->setGeometry(QRect(60, 20, 81, 17));
        checkPose = new QCheckBox(checking_zone);
        checkPose->setObjectName(QString::fromUtf8("checkPose"));
        checkPose->setGeometry(QRect(11, 21, 46, 17));
        checkPose->setChecked(true);
        checkSimLines = new QCheckBox(checking_zone);
        checkSimLines->setObjectName(QString::fromUtf8("checkSimLines"));
        checkSimLines->setGeometry(QRect(200, 20, 63, 17));
        pushButton = new QPushButton(testing_zone);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(840, 30, 101, 21));
        horizontalSlider = new QSlider(testing_zone);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(820, 60, 160, 19));
        horizontalSlider->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(testing_zone, 2, 0, 1, 1);

        VDrawClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(VDrawClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1101, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuRender = new QMenu(menuBar);
        menuRender->setObjectName(QString::fromUtf8("menuRender"));
        menuFilter = new QMenu(menuBar);
        menuFilter->setObjectName(QString::fromUtf8("menuFilter"));
        menuGL = new QMenu(menuBar);
        menuGL->setObjectName(QString::fromUtf8("menuGL"));
        menuEffects = new QMenu(menuGL);
        menuEffects->setObjectName(QString::fromUtf8("menuEffects"));
        menuStella = new QMenu(menuBar);
        menuStella->setObjectName(QString::fromUtf8("menuStella"));
        VDrawClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(VDrawClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        VDrawClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(VDrawClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        VDrawClass->setStatusBar(statusBar);
        dockWidget = new QDockWidget(VDrawClass);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        dockWidget->setMinimumSize(QSize(550, 550));
        dockWidget->setMaximumSize(QSize(900, 700));
        dockWidget->setFloating(true);
        dockWidget->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        gridLayout_6 = new QGridLayout(dockWidgetContents);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        stellaWidget = new StellaWidget(dockWidgetContents);
        stellaWidget->setObjectName(QString::fromUtf8("stellaWidget"));

        gridLayout_6->addWidget(stellaWidget, 0, 0, 1, 1);

        dockWidget->setWidget(dockWidgetContents);
        VDrawClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuGL->menuAction());
        menuBar->addAction(menuRender->menuAction());
        menuBar->addAction(menuFilter->menuAction());
        menuBar->addAction(menuStella->menuAction());
        menuFile->addAction(actionLoad_mesh);
        menuFile->addAction(actionLoad_segmentation);
        menuRender->addAction(actionDepth_Buffer);
        menuRender->addAction(actionStart);
        menuRender->addAction(actionTextured_Rendering);
        menuFilter->addAction(actionCanny);
        menuFilter->addAction(actionHough_Transform);
        menuGL->addAction(menuEffects->menuAction());
        menuEffects->addAction(actionContour);
        menuEffects->addAction(actionSuggestive_Contour);
        menuEffects->addAction(actionDiffuse);
        menuEffects->addAction(actionSegmentation);
        menuEffects->addAction(actionPrimitive_fitting);
        menuStella->addAction(actionStella_Prepare);
        menuStella->addAction(stellaStartItem);

        retranslateUi(VDrawClass);
        QObject::connect(actionLoad_mesh, SIGNAL(activated()), VDrawClass, SLOT(loadTriMesh()));
        QObject::connect(actionDepth_Buffer, SIGNAL(activated()), VDrawClass, SLOT(renderDepthBuffer()));
        QObject::connect(actionCanny, SIGNAL(activated()), VDrawClass, SLOT(filterCanny()));
        QObject::connect(actionHough_Transform, SIGNAL(activated()), VDrawClass, SLOT(filterHough()));
        QObject::connect(testParaName, SIGNAL(activated(QString)), VDrawClass, SLOT(paraNameChanged(QString)));
        QObject::connect(testParaValue, SIGNAL(actionTriggered(int)), VDrawClass, SLOT(paraValChanged(int)));
        QObject::connect(checkSimLines, SIGNAL(toggled(bool)), VDrawClass, SLOT(showSimLine(bool)));
        QObject::connect(checkPose, SIGNAL(toggled(bool)), VDrawClass, SLOT(showPoseLine(bool)));
        QObject::connect(actionLoad_segmentation, SIGNAL(activated()), VDrawClass, SLOT(loadSegmentation()));
        QObject::connect(checkBox, SIGNAL(toggled(bool)), VDrawClass, SLOT(showSCLines(bool)));
        QObject::connect(checkBox_3, SIGNAL(toggled(bool)), VDrawClass, SLOT(showConvexHull2D(bool)));
        QObject::connect(pushButton, SIGNAL(clicked()), VDrawClass, SLOT(orderCurves()));
        QObject::connect(horizontalSlider, SIGNAL(valueChanged(int)), VDrawClass, SLOT(strokeNumChanged(int)));
        QObject::connect(checkPrims, SIGNAL(toggled(bool)), VDrawClass, SLOT(showPrimLines(bool)));
        QObject::connect(stellaStartItem, SIGNAL(triggered()), VDrawClass, SLOT(setStella()));
        QObject::connect(actionStart, SIGNAL(triggered()), VDrawClass, SLOT(stratAnimateOrder()));
        QObject::connect(actionTextured_Rendering, SIGNAL(activated()), VDrawClass, SLOT(renderTextured()));

        left_panel_tab->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(VDrawClass);
    } // setupUi

    void retranslateUi(QMainWindow *VDrawClass)
    {
        VDrawClass->setWindowTitle(QApplication::translate("VDrawClass", "VDraw", 0, QApplication::UnicodeUTF8));
        actionLoad_mesh->setText(QApplication::translate("VDrawClass", "Load mesh", 0, QApplication::UnicodeUTF8));
        actionDepth_Buffer->setText(QApplication::translate("VDrawClass", "Depth Buffer", 0, QApplication::UnicodeUTF8));
        actionCanny->setText(QApplication::translate("VDrawClass", "Canny", 0, QApplication::UnicodeUTF8));
        actionHough_Transform->setText(QApplication::translate("VDrawClass", "Hough Transform", 0, QApplication::UnicodeUTF8));
        actionContour->setText(QApplication::translate("VDrawClass", "Contour", 0, QApplication::UnicodeUTF8));
        actionSuggestive_Contour->setText(QApplication::translate("VDrawClass", "Suggestive Contour", 0, QApplication::UnicodeUTF8));
        actionDiffuse->setText(QApplication::translate("VDrawClass", "Diffuse", 0, QApplication::UnicodeUTF8));
        actionSegmentation->setText(QApplication::translate("VDrawClass", "Segmentation", 0, QApplication::UnicodeUTF8));
        actionPrimitive_fitting->setText(QApplication::translate("VDrawClass", "Primitive fitting", 0, QApplication::UnicodeUTF8));
        actionLoad_segmentation->setText(QApplication::translate("VDrawClass", "Load segmentation", 0, QApplication::UnicodeUTF8));
        actionStella_Prepare->setText(QApplication::translate("VDrawClass", "Stella Prepare", 0, QApplication::UnicodeUTF8));
        stellaStartItem->setText(QApplication::translate("VDrawClass", "Stella Start", 0, QApplication::UnicodeUTF8));
        actionStart->setText(QApplication::translate("VDrawClass", "Start", 0, QApplication::UnicodeUTF8));
        actionTextured_Rendering->setText(QApplication::translate("VDrawClass", "Textured Rendering", 0, QApplication::UnicodeUTF8));
        left_panel_tab->setTabText(left_panel_tab->indexOf(tab_gl), QApplication::translate("VDrawClass", "Tab 1", 0, QApplication::UnicodeUTF8));
        left_panel_tab->setTabText(left_panel_tab->indexOf(tab_gl2), QApplication::translate("VDrawClass", "Tab 2", 0, QApplication::UnicodeUTF8));
        render_label->setText(QApplication::translate("VDrawClass", "TextLabel", 0, QApplication::UnicodeUTF8));
        testing_zone->setTitle(QApplication::translate("VDrawClass", "vZone", 0, QApplication::UnicodeUTF8));
        testParaName->clear();
        testParaName->insertItems(0, QStringList()
         << QApplication::translate("VDrawClass", "None", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("VDrawClass", "connect_dist", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("VDrawClass", "connect_angle", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("VDrawClass", "Splitting Chains", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("VDrawClass", "Show Curvature", 0, QApplication::UnicodeUTF8)
        );
        checking_zone->setTitle(QApplication::translate("VDrawClass", "Checking Zone", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("VDrawClass", "S.C.", 0, QApplication::UnicodeUTF8));
        checkPrims->setText(QApplication::translate("VDrawClass", "Prims", 0, QApplication::UnicodeUTF8));
        checkBox_3->setText(QApplication::translate("VDrawClass", "First Sketch", 0, QApplication::UnicodeUTF8));
        checkPose->setText(QApplication::translate("VDrawClass", "Pose", 0, QApplication::UnicodeUTF8));
        checkSimLines->setText(QApplication::translate("VDrawClass", "SimLines", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("VDrawClass", "Order", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("VDrawClass", "File", 0, QApplication::UnicodeUTF8));
        menuRender->setTitle(QApplication::translate("VDrawClass", "Render", 0, QApplication::UnicodeUTF8));
        menuFilter->setTitle(QApplication::translate("VDrawClass", "Filter", 0, QApplication::UnicodeUTF8));
        menuGL->setTitle(QApplication::translate("VDrawClass", "GL", 0, QApplication::UnicodeUTF8));
        menuEffects->setTitle(QApplication::translate("VDrawClass", "effects", 0, QApplication::UnicodeUTF8));
        menuStella->setTitle(QApplication::translate("VDrawClass", "Stella", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VDrawClass: public Ui_VDrawClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VDRAW_H
