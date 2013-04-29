#ifndef RENDERSTROKE_H
#define RENDERSTROKE_H


#include <QtGui/QMainWindow>
#include <QtGui/QtGui>
#include "ui_renderstroke.h"
#include "StrokeHandler.h"
class RenderStroke : public QMainWindow
{
    Q_OBJECT

public:
    RenderStroke(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~RenderStroke();

public:
	void LoadStroke(char* fileName);

private slots:
    void OpenStroke();
    void OpenPencilTexture();
    void StrokeAnimation();
    void ClearStrokes();
    void ChangeAnimationSpeed(int pos);
    void ChangeStrokeType1();
    void ChangeStrokeType2();
    void ZoomIn();
    void ZoomOut();
    void ZoomOriginal();
private:
    Ui::RenderStrokeClass ui;

private:
    CStrokeHandler m_strokeHandler;
private:
    void TransferData();
    void SetAnimationSpeedSliderPos(int animationInterval);
};

#endif // RENDERSTROKE_H
