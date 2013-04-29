#include "strokeinputwidget.h"
#include <QtGui>
StrokeInputWidget::StrokeInputWidget(QWidget *parent)
    : QWidget(parent)
{
    m_strokeHandlerp = NULL;
    m_canvasZoomRatio = 1;
    setFixedSize(1000, 800);
    //setBackgroundRole(QPalette::Dark);
    //setBackgroundColor(QColor(220, 220, 220));
}

StrokeInputWidget::~StrokeInputWidget()
{

}
void StrokeInputWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this); 
    painter.setPen(QPen(Qt::black));
    painter.save();
    painter.drawRect(1, 1, 998, 798);
    painter.restore();
    DrawAllStrokes(painter); 
    DrawThisStroke(painter); 
}
void StrokeInputWidget::DrawAllStrokes(QPainter& painter)
{
    if (m_strokeHandlerp->m_allStrokes.size() == 0)
    {
        return;
    }
    QBrush thisBrush(Qt::red);
    QPen thisPen(thisBrush, m_strokeHandlerp->m_strokeWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(thisPen);
    for (unsigned int i = 0; i < m_strokeHandlerp->m_allStrokes.size(); ++i)
    {
        std::vector<cv::Point>& thisStroke = (m_strokeHandlerp->m_allStrokes[i]);
        if (thisStroke.size() == 0)
        {
            continue;
        }
        painter.save();
        painter.drawPoint(thisStroke[0].x, thisStroke[0].y);
        painter.restore();
        for (unsigned int j = 1; j < thisStroke.size(); ++j)
        {
            painter.save();
            painter.drawLine(thisStroke[j - 1].x, thisStroke[j - 1].y, thisStroke[j].x, thisStroke[j].y);
            painter.restore();
        }
    }
}
void StrokeInputWidget::DrawThisStroke(QPainter& painter)
{
    if (m_strokePtsTemp.size() == 0)
    {
        return;
    }
    QBrush thisBrush(Qt::blue);
    QPen thisPen(thisBrush, m_strokeHandlerp->m_strokeWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(thisPen);
    painter.save();
    painter.drawPoint(m_strokePtsTemp[0].x, m_strokePtsTemp[0].y);
    painter.restore();
    for (int i = 1; i < m_strokePtsTemp.size(); i++)
    {
        painter.save();
        painter.drawLine(m_strokePtsTemp[i - 1].x, m_strokePtsTemp[i - 1].y, m_strokePtsTemp[i].x, m_strokePtsTemp[i].y);
        painter.restore();
    }
}
bool StrokeInputWidget::event(QEvent* e)
{
    static cv::Point prevPt = cv::Point(-1, -1);
    static cv::Point thisPt = cv::Point(-1, -1);
    const QMouseEvent* const me = static_cast<const QMouseEvent*>(e);
    if (m_strokeHandlerp == NULL)
    {
        return QWidget::event(e);
    }
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
        {
            if (me->button() == Qt::LeftButton)
            {
                m_strokePtsTemp.clear();

                prevPt.x = me->pos().x();
                prevPt.y = me->pos().y();

                m_strokePtsTemp.push_back(prevPt);                
                repaint();
            }
        }
        break;
    case QEvent::MouseButtonRelease:
        {
            if (me->button() == Qt::LeftButton)
            {

                thisPt.x = me->pos().x();
                thisPt.y = me->pos().y();
                prevPt = cv::Point(-1, -1);
                m_strokeHandlerp->AddStroke(m_strokePtsTemp);
                m_strokePtsTemp.clear();
                repaint();
            }
        }
        break;
    case QEvent::MouseMove:
        {
            if (me->buttons() & Qt::LeftButton)
            {
                //std::cout << "left button move: ";
                //std::cout << me->pos().x() << ", " << me->pos().y() << std::endl;
                thisPt.x = me->pos().x();
                thisPt.y = me->pos().y();
                m_strokePtsTemp.push_back(thisPt);
                if (prevPt.x < 0)
                {
                    prevPt = thisPt;
                }
                prevPt = thisPt;
                repaint();
            }
        }
        break;
    }
    return QWidget::event(e);
}
void StrokeInputWidget::ZoomInCanvas()
{
    if (m_canvasZoomRatio >= 5.0)
    {
        return;
    }
    m_canvasZoomRatio *= 1.25;
    repaint();
}
void StrokeInputWidget::ZoomOutCanvas()
{
    if (m_canvasZoomRatio <= 0.1)
    {
        return;
    }
    m_canvasZoomRatio *= 0.8;
    repaint();
}
void StrokeInputWidget::ZoomOriginalCanvas()
{
    m_canvasZoomRatio = 1.0;
    repaint();
}
void StrokeInputWidget::SetData(CStrokeHandler* strokeHandler)
{
    m_strokeHandlerp = strokeHandler;
}