#include <QtGui>
#include "strokedisplaywidget.h"
#include <iostream>
#include <queue>

#define MAX_MASK_VALUE 100
#define FADE_PT_NUM 2
StrokeDisplayWidget::StrokeDisplayWidget(QWidget *parent)
    : QWidget(parent)
{
    m_strokeHandlerp = NULL;
    m_timer = NULL;
    m_isAnimation =false;
    m_canvasZoomRatio = 1;
    InitializeData();
    setFixedSize(1000, 800);
}
StrokeDisplayWidget::~StrokeDisplayWidget()
{
    if (m_timer)
    {
        delete m_timer;
    }
}
void StrokeDisplayWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    //std::cout << "paintEvent" << std::endl;
    double scaleRatia = m_strokeHandlerp->m_imgScaleRatio * m_canvasZoomRatio;
    QRect targetRect = QRect(0, 0, m_qImgBGforAnimation.width() * scaleRatia, m_qImgBGforAnimation.height() * scaleRatia);
    QRect sourceRect = QRect(0, 0, m_qImgBGforAnimation.width(), m_qImgBGforAnimation.height());
    painter.drawImage(targetRect, m_qImgBGforAnimation, sourceRect);
}
bool StrokeDisplayWidget::event(QEvent* e)
{
    return QWidget::event(e);
}
void StrokeDisplayWidget::InitializeData()
{
    //m_qImgBG.load("whitepaper.jpg");
    m_bgImgName = "..\\data\\whitepaper.png";
    //m_strokeImgName = "..\\data\\stroketexture.jpg";
    //m_qImgBG.load(m_bgImgName);
    m_qImgBGforAnimation.load(m_bgImgName);
    m_cvImgBGforAnimation = cv::Mat(m_qImgBGforAnimation.size().height(), m_qImgBGforAnimation.size().width(), CV_8UC4, m_qImgBGforAnimation.bits());
    srand(0);
}
void StrokeDisplayWidget::StrokeAnimationBegin()
{
    if (m_strokeHandlerp == NULL || m_strokeHandlerp->m_allStrokes.size() == 0)
    {
        std::cout << "No stroke data" << std::endl;
        return;
    } 
    std::cout << "PrepareAllStrokePixels" << std::endl;
    m_strokeHandlerp->PrepareAllStrokePixels();
    std::cout << "Animation Start" << std::endl;
    if (m_timer)
    {
        delete m_timer;
    }
    m_timer = new QTimer(this);
    m_timer->setInterval(m_strokeHandlerp->m_animationInterval);
    m_crtStrokeIdx = 0;
    m_crtSegIdx = 0;
    m_qImgBGforAnimation.load(m_bgImgName);
    m_cvImgBGforAnimation = cv::Mat(m_qImgBGforAnimation.size().height(), m_qImgBGforAnimation.size().width(), CV_8UC4, m_qImgBGforAnimation.bits());
    connect(m_timer, SIGNAL(timeout()), this, SLOT(StrokeAnimationUpdate()));
    m_timer->start();
    repaint();
}
void StrokeDisplayWidget::StrokeAnimationUpdate()
{

    //std::cout << "StrokeAnimationUpdate" << std::endl;
    //std::cout << m_strokeHandlerp->m_allStrokePixels.size() << " ";
    //std::cout << m_strokeHandlerp->m_allStrokePixels[m_crtStrokeIdx].size() << std::endl;
    //std::cout << m_crtStrokeIdx << " " << m_crtSegIdx << std::endl;
    if (m_crtSegIdx >= m_strokeHandlerp->m_allStrokePixels[m_crtStrokeIdx].size())
    {
        m_crtSegIdx = 0;
        ++m_crtStrokeIdx;
    }
    if (m_crtStrokeIdx >= m_strokeHandlerp->m_allStrokePixels.size())
    {
        m_timer->stop();
        m_isAnimation =false;
    }
    if (m_crtStrokeIdx < m_strokeHandlerp->m_allStrokePixels.size())
    {
        PrepareCrtStrokeSegment();
        repaint();
    }
    m_crtSegIdx++;
    m_timer->setInterval(m_strokeHandlerp->m_animationInterval);
}
void StrokeDisplayWidget::PrepareCrtStrokeSegment()
{
    if (m_crtStrokeIdx >= m_strokeHandlerp->m_allStrokes.size())
        return;
    if (m_crtSegIdx >= m_strokeHandlerp->m_allStrokePixels[m_crtStrokeIdx].size())
        return;
    //std::cout << "PrepareCrtStrokeSegment" << std::endl;
    std::vector<SC::CStrokePoint>& thisSegment = m_strokeHandlerp->m_allStrokePixels[m_crtStrokeIdx][m_crtSegIdx];
    //cv::Vec3b whiteColor(255, 255, 255);
    for (unsigned int i = 0; i < thisSegment.size(); i++)
    {
        cv::Vec4b& thisPixel = m_cvImgBGforAnimation.at<cv::Vec4b>(thisSegment[i].pos);
        int thisR, thisG, thisB;
        thisR = (255 - thisPixel[0]) * 1 + (255 - thisSegment[i].color[0]) * 0.8;
        thisG = (255 - thisPixel[1]) * 1 + (255 - thisSegment[i].color[1]) * 0.8;
        thisB = (255 - thisPixel[2]) * 1 + (255 - thisSegment[i].color[2]) * 0.8;
        if (thisR > 255) thisR = 255;
        if (thisG > 255) thisG = 255;
        if (thisB > 255) thisB = 255;
        thisR = 255 - thisR;
        thisG = 255 - thisG;
        thisB = 255 - thisB;

        thisPixel[0] = thisR;
        thisPixel[1] = thisG;
        thisPixel[2] = thisB;
    }
}
void StrokeDisplayWidget::ZoomInCanvas()
{
    if (m_canvasZoomRatio >= 5.0)
    {
        return;
    }
    m_canvasZoomRatio *= 1.25;
    repaint();
}
void StrokeDisplayWidget::ZoomOutCanvas()
{
    if (m_canvasZoomRatio <= 0.1)
    {
        return;
    }
    m_canvasZoomRatio *= 0.8;
    repaint();
}
void StrokeDisplayWidget::ZoomOriginalCanvas()
{
    m_canvasZoomRatio = 1.0;
    repaint();
}
void StrokeDisplayWidget::SetData(CStrokeHandler* strokeHandler)
{
    m_strokeHandlerp = strokeHandler;
}
