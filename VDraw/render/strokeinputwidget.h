#ifndef STROKEINPUTWIDGET_H
#define STROKEINPUTWIDGET_H

#include <QWidget>
#include "StrokeHandler.h"
class StrokeInputWidget : public QWidget
{
    Q_OBJECT

public:
    StrokeInputWidget(QWidget *parent);
    ~StrokeInputWidget();
protected:
    void paintEvent(QPaintEvent* e);
    bool event(QEvent* e);
private:
    void DrawAllStrokes(QPainter& painter);
    void DrawThisStroke(QPainter& painter);
public:
    void SetData(CStrokeHandler* strokeHandler);
    void ZoomInCanvas();
    void ZoomOutCanvas();
    void ZoomOriginalCanvas();
private:
    CStrokeHandler* m_strokeHandlerp;
    std::vector<cv::Point> m_strokePtsTemp;
    double m_canvasZoomRatio;
};

#endif // STROKEINPUTWIDGET_H
