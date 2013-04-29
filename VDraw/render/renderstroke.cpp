#include "renderstroke.h"
#include <iostream>
#define MAX_ANIMATION_INTERVAL 200
#define MIN_ANIMATION_INTERVAL 1

RenderStroke::RenderStroke(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    TransferData();
    SetAnimationSpeedSliderPos(m_strokeHandler.m_animationInterval);
    ui.actionType_1->setChecked(true);
    ui.actionType_2->setChecked(false);
    ui.scrollArea_strokeInput->setBackgroundRole(QPalette::Light);
}

RenderStroke::~RenderStroke()
{

}
void RenderStroke::LoadStroke(char* fileName){
	m_strokeHandler.LoadStroke(fileName);
	ui.strokeDisplay->repaint();
	ui.strokeInput->repaint();
}

void RenderStroke::OpenStroke()
{
    std::cout << "OpenStroke" << std::endl;
    QString path = "..\\testdata\\strokes";

    QString thisFileName = QFileDialog::getOpenFileName(this, tr("Open Stroke"), path);
    if (!thisFileName.isEmpty())
    {
        std::string name = thisFileName.toStdString();
        m_strokeHandler.LoadStroke(&name[0]);
        ui.strokeDisplay->repaint();
        ui.strokeInput->repaint();
    }
}
void RenderStroke::OpenPencilTexture()
{
    std::cout << "OpenPencilTexture" << std::endl;
    QString path = "..\\data\\";
    QString thisFileName = QFileDialog::getOpenFileName(this, tr("Open Pencil Texture"), path);
    if (!thisFileName.isEmpty())
    {
        std::string name = thisFileName.toStdString();
        m_strokeHandler.LoadPencilTexture(&name[0]);
        ui.strokeDisplay->repaint();
        ui.strokeInput->repaint();
    }
}
void RenderStroke::StrokeAnimation()
{
    ui.strokeDisplay->StrokeAnimationBegin();
}
void RenderStroke::ClearStrokes()
{
    m_strokeHandler.ClearStrokes();
    ui.strokeDisplay->repaint();
    ui.strokeInput->repaint();
}
void RenderStroke::ChangeAnimationSpeed(int pos)
{
    int p1, p2;
    p1 = ui.horizontalSlider_animationSpeed->maximum();
    p2 = ui.horizontalSlider_animationSpeed->minimum();
    double tempCoef;
    tempCoef = (pos - p2) / (double (p1 - p2));
    m_strokeHandler.m_animationInterval = MIN_ANIMATION_INTERVAL + tempCoef * (MAX_ANIMATION_INTERVAL - MIN_ANIMATION_INTERVAL);
    std::cout << m_strokeHandler.m_animationInterval << std::endl;
}
void RenderStroke::SetAnimationSpeedSliderPos(int animationInterval)
{
    double tempCoef;
    tempCoef = (animationInterval - MIN_ANIMATION_INTERVAL) / (double (MAX_ANIMATION_INTERVAL - MIN_ANIMATION_INTERVAL));
    int p1, p2;
    p1 = ui.horizontalSlider_animationSpeed->maximum();
    p2 = ui.horizontalSlider_animationSpeed->minimum();
    int tempPos = p2 + (p1 - p2) * tempCoef;
    ui.horizontalSlider_animationSpeed->setValue(tempPos);
    std::cout << m_strokeHandler.m_animationInterval << std::endl;
}
void RenderStroke::ChangeStrokeType1()
{
    m_strokeHandler.m_strokeType = 1;
    ui.actionType_1->setChecked(true);
    ui.actionType_2->setChecked(false);
}
void RenderStroke::ChangeStrokeType2()
{
    m_strokeHandler.m_strokeType = 2;
    ui.actionType_2->setChecked(true);
    ui.actionType_1->setChecked(false);
}
void RenderStroke::ZoomIn()
{
    ui.strokeDisplay->ZoomInCanvas();
    ui.strokeInput->ZoomInCanvas();
}
void RenderStroke::ZoomOut()
{
    ui.strokeDisplay->ZoomOutCanvas();
    ui.strokeInput->ZoomOutCanvas();
}
void RenderStroke::ZoomOriginal()
{
    ui.strokeDisplay->ZoomOriginalCanvas();
    ui.strokeInput->ZoomOriginalCanvas();
}
void RenderStroke::TransferData()
{
    ui.strokeDisplay->SetData(&m_strokeHandler);
    ui.strokeInput->SetData(&m_strokeHandler);
}

