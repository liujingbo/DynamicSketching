#pragma once
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SomeClasses.h"
#include <string>
class CStrokeHandler
{
public:
    CStrokeHandler(void);
    ~CStrokeHandler(void);
public:
    std::vector<std::vector<cv::Point>> m_allStrokes;
    std::vector<std::vector<std::vector<SC::CStrokePoint>>> m_allStrokePixels;
	std::vector<std::vector<std::vector<float>>> m_allPointProperties;
    int m_strokeWidth;
    int m_animationInterval;
    double m_imgScaleRatio;
    cv::Mat m_cvImgPencilTexture;
    cv::Mat m_cvImgPencilTexture2;
    //std::string m_bgImgName;
    //std::string m_strokeImgName;
    std::string m_pencilTextureName;
    std::string m_pencilTextureName2;
    int m_strokeType;
public:
    void LoadStroke(char* fileName);
    void SaveStroke(char* fileName);
    void LoadPencilTexture(char* fileName);
    void PrepareAllStrokePixels();
private:
    void InitializeData();
	void PrepareThisStrokePixels(const std::vector<cv::Point>& thisStroke, std::vector<std::vector<SC::CStrokePoint>>& thisStrokePixels, const std::vector<std::vector<float>> thisStrokeProperty);
    void PrepareThisStrokePixels2(const std::vector<cv::Point>& thisStroke, std::vector<std::vector<SC::CStrokePoint>>& thisStrokePixels, const std::vector<std::vector<float>> thisStrokeProperty);
    void GetFadeLine(cv::Mat& thisMask, std::vector<cv::Point>& pts);

public:
    void AddStroke(std::vector<cv::Point>& stroke);
    void ClearStrokes();
};

