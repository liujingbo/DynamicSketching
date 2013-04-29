#ifndef STROKEDISPLAYWIDGET_H
#define STROKEDISPLAYWIDGET_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
#include "StrokeHandler.h"

class StrokeDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    StrokeDisplayWidget(QWidget *parent);
    ~StrokeDisplayWidget();

protected:
    void paintEvent(QPaintEvent* e);
    bool event(QEvent* e);

private:
    void InitializeData();
    void PrepareCrtStrokeSegment();
private slots:
    void StrokeAnimationUpdate();
public:
    void SetData(CStrokeHandler* strokeHandler);
    void StrokeAnimationBegin();
    void ZoomInCanvas();
    void ZoomOutCanvas();
    void ZoomOriginalCanvas();
private:
    //QImage m_qImgBG;
    QImage m_qImgBGforAnimation;
    cv::Mat m_cvImgBGforAnimation;
    CStrokeHandler* m_strokeHandlerp;
    QTimer* m_timer;
    int m_crtStrokeIdx;
    int m_crtSegIdx;
    bool m_isAnimation;
    QString m_bgImgName;
    //QString m_strokeImgName;
    double m_canvasZoomRatio;
};

#endif // STROKEDISPLAYWIDGET_H
