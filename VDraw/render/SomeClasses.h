#pragma once
#include <opencv2/core/core.hpp>
namespace SC
{

class CStrokePoint 
{
public:
    CStrokePoint()
    {
        pos.x = pos.y = 0;
        color[0] = color[1] = color[2] = 0;
    }
    CStrokePoint(int x, int y, cv::Vec3b& c)
    {
        pos.x = x;
        pos.y = y;
        color = c;
    }
public:
    cv::Point pos;
    cv::Vec3b color;
};









}
