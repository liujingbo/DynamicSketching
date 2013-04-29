#include "StrokeHandler.h"
#include <fstream>
#include <iostream>
#include <queue>
#define FADE_PT_NUM 10
#define MAX_MASK_VALUE 100

CStrokeHandler::CStrokeHandler(void)
{
    m_strokeType = 1;
    InitializeData();
}
CStrokeHandler::~CStrokeHandler(void)
{
}

void CStrokeHandler::InitializeData()
{
    m_allStrokes.clear();
    m_allStrokePixels.clear();
	this->m_allPointProperties.clear();
    m_strokeWidth = 3;
    m_animationInterval = 10;
    m_imgScaleRatio = 1.0 / 3.0;
    //m_bgImgName = "..\\data\\whitepaper.png";
    //m_strokeImgName = "..\\data\\stroketexture.jpg";
    m_pencilTextureName = "..\\data\\penciltexture_2.png";
    //m_pencilTextureName = "..\\data\\charcoal.png";
    m_pencilTextureName2 = "..\\data\\stroketexture.jpg";
    cv::Mat tempPencilTexture = cv::imread(m_pencilTextureName);
    cv::resize(tempPencilTexture, m_cvImgPencilTexture, cv::Size(), 0.2, 0.2);
    tempPencilTexture = cv::imread(m_pencilTextureName2);
    cv::resize(tempPencilTexture, m_cvImgPencilTexture2, cv::Size(), 1 / m_imgScaleRatio, 1 / m_imgScaleRatio);
}

void CStrokeHandler::LoadStroke(char* fileName)
{
    InitializeData();
    std::ifstream ifs(fileName);
    cv::Point thisPt;
	std::vector<float> pt_property;
    while (!ifs.eof())
    {
        char thisLetter;
        ifs >> thisLetter;
        switch (thisLetter)
        {
        case '#':
			char thisLine[50];
			ifs.getline(thisLine, 50);
            m_allStrokes.push_back(std::vector<cv::Point>());
			m_allPointProperties.push_back(std::vector<std::vector<float>>());
            break;
        case 'p':
			float intensity,thickness,velocity;
            ifs >> thisPt.x;
			ifs >> thisPt.y;
			ifs >> intensity;
			ifs >>thickness;
			ifs >>velocity;
            m_allStrokes.back().push_back(thisPt);
			pt_property.clear();
			pt_property.push_back(intensity);
			pt_property.push_back(thickness);
			pt_property.push_back(velocity);
			m_allPointProperties.back().push_back(pt_property);
            break;
		default:
			assert(0);// false format
        }
    }
	assert(m_allStrokes.size() == m_allPointProperties.size());
    ifs.clear();
    ifs.close();
}
void CStrokeHandler::SaveStroke(char* fileName)
{
    std::fstream fout(fileName, std::ios::out);
    //fout << m_allStrokes.size() << std::endl;
    for (unsigned int i = 0; i <  m_allStrokes.size(); ++i)
    {
        fout << 's' << std::endl;
        for (unsigned int j = 0; j < m_allStrokes[i].size(); ++j)
        {
            fout << 'p' << '\t' << m_allStrokes[i][j].x << '\t' << m_allStrokes[i][j].y << std::endl;
        }
    }
    fout.flush();
    fout.close();
    fout.clear();
}
void CStrokeHandler::LoadPencilTexture(char* fileName)
{
    m_pencilTextureName = fileName;
    cv::Mat tempPencilTexture = cv::imread(m_pencilTextureName);
    cv::resize(tempPencilTexture, m_cvImgPencilTexture, cv::Size(), 0.2, 0.2);
}
void CStrokeHandler::AddStroke(std::vector<cv::Point>& stroke)
{
    m_allStrokes.push_back(stroke);
}
void CStrokeHandler::ClearStrokes()
{
    m_allStrokes.clear();
}
void CStrokeHandler::PrepareThisStrokePixels(const std::vector<cv::Point>& thisStroke, std::vector<std::vector<SC::CStrokePoint>>& thisStrokePixels, const std::vector<std::vector<float>> thisStrokeProperty)
{
    cv::Point maxPt, minPt;
    double scaleRatio = m_imgScaleRatio;
    maxPt.x = minPt.x = thisStroke[0].x / scaleRatio;
    maxPt.y = minPt.y = thisStroke[0].y / scaleRatio;
    std::vector<cv::Point> thisStrokeScaled;

    for (unsigned int i = 0; i < thisStroke.size(); ++i)
    {
        thisStrokeScaled.push_back(cv::Point(thisStroke[i].x / scaleRatio, thisStroke[i].y / scaleRatio));
        if (maxPt.x < thisStrokeScaled[i].x)
            maxPt.x = thisStrokeScaled[i].x;
        if (maxPt.y < thisStrokeScaled[i].y)
            maxPt.y = thisStrokeScaled[i].y;
        if (minPt.x > thisStrokeScaled[i].x)
            minPt.x = thisStrokeScaled[i].x;
        if (minPt.y > thisStrokeScaled[i].y)
            minPt.y = thisStrokeScaled[i].y;
    }
    maxPt += cv::Point(m_strokeWidth * 3, m_strokeWidth * 3);
    minPt -= cv::Point(m_strokeWidth * 3, m_strokeWidth * 3);
    if (minPt.x < 0)
        minPt.x = 0;
    if (minPt.y < 0)
        minPt.y = 0;
    cv::Size thisStrokeSize;
    thisStrokeSize.width = maxPt.x - minPt.x;
    thisStrokeSize.height = maxPt.y - minPt.y;
    //int startPos = rand() % m_cvImgPencilTexture.size().width;
    // need modify here
    int strokeLength = 0;
    for (unsigned int i = 1; i < thisStrokeScaled.size(); ++i)
    {
        int thisL;
        cv::Point thisVec(thisStrokeScaled[i].x - thisStrokeScaled[i - 1].x, thisStrokeScaled[i].y - thisStrokeScaled[i - 1].y);
        thisL = sqrt((double)thisVec.dot(thisVec));
        strokeLength += thisL;
    }
    //cv::Mat thisStrokeTexture = cv::Mat(m_cvImgPencilTexture, cv::Rect(0, 0,  strokeLength + 30, m_cvImgPencilTexture.size().height)).clone();
    // if thisL > m_cvImgPencilTexture.size().width, need modify texture
    std::vector<cv::Point> srcPts;
    std::vector<cv::Point> dstPts;
    cv::Point thisSrcPt, thisDstPt;
    thisSrcPt.y = m_cvImgPencilTexture.size().height / 2 + 1;
    thisSrcPt.x = 0;
    srcPts.push_back(thisSrcPt);
    dstPts.push_back(thisStrokeScaled[0] - minPt);
    for (unsigned int i = 1; i < thisStrokeScaled.size(); ++i)
    {
        float thisL;
        cv::Point thisVec(thisStrokeScaled[i].x - thisStrokeScaled[ i - 1].x, thisStrokeScaled[i].y - thisStrokeScaled[i - 1].y);
		thisL = cv::norm(thisVec);
		thisSrcPt.x += thisL/strokeLength*(m_cvImgPencilTexture.size().width);
        srcPts.push_back(thisSrcPt);
        dstPts.push_back(thisStrokeScaled[i] - minPt);
    }
    cv::Mat thisSegmentMask, prevSegmentMask, thisSegment;
    int thisStrokeWidth = m_strokeWidth / scaleRatio;
    
    bool hasFadePt = true;
    if (dstPts.size() < FADE_PT_NUM * 2)
    {
        hasFadePt = false;
    }
    int numNoFadePt;
    if (hasFadePt)
    {
        numNoFadePt = dstPts.size() - FADE_PT_NUM;
    }
    else
    {
        numNoFadePt = dstPts.size();
    }
    cv::Mat thisMask = cv::Mat::zeros(thisStrokeSize, CV_8U);
    for (unsigned int i = 1; i < numNoFadePt; ++i)
    {
        //cv::line(segmentIdxs, dstPts[i - 1], dstPts[i], i, thisStrokeWidth + 2);
        cv::line(thisMask, dstPts[i - 1], dstPts[i], MAX_MASK_VALUE, thisStrokeWidth);
    }
    if (hasFadePt)
    {
        std::vector<cv::Point> fadePts;
        for (unsigned int i = numNoFadePt - 1; i < dstPts.size(); ++i)
        {
            fadePts.push_back(dstPts[i]);
        }
        GetFadeLine(thisMask, fadePts);
    }
    cv::GaussianBlur(thisMask, thisMask, cv::Size(3, 3), 0.7);
    prevSegmentMask = cv::Mat::zeros(thisStrokeSize, CV_32S);
    std::vector<SC::CStrokePoint> thisSegPixels;

    for (unsigned int i = 1; i < dstPts.size(); ++i)
    {
        thisSegPixels.clear();
        thisSegment = cv::Mat::zeros(thisStrokeSize, CV_32S);
        thisSegmentMask = cv::Mat::zeros(thisStrokeSize, CV_32S);
        cv::line(thisSegment, dstPts[i - 1], dstPts[i], i, thisStrokeWidth);
        cv::Vec2d thisVec;
        thisVec[0] = dstPts[i].x - dstPts[i - 1].x;
        thisVec[1] = dstPts[i].y - dstPts[i - 1].y;
        //double thisSegL = cv::norm(thisVec);
        thisVec = cv::normalize(thisVec);

		// fading factor, stroke intensity
		float intensity_start = 1;
		float intensity_end = 1;
		if(thisStrokeProperty.size()>=i+1){
			intensity_start = thisStrokeProperty[i-1][0];
			intensity_end = thisStrokeProperty[i][0];
		}

        for (unsigned int y = 0; y < thisSegment.rows; ++y)
        {
            for (unsigned int x = 0; x < thisSegment.cols; ++x)
            {
                if (thisSegment.at<int>(y, x) > 0 && prevSegmentMask.at<int>(y, x) == 0)
                {
                    cv::Vec2d thisVec2;
                    thisVec2[0] = (int) x - dstPts[i].x;
                    thisVec2[1] = (int) y - dstPts[i].y;
                    if (thisVec.dot(thisVec2) > 0)
                    {
                        continue;
                    }
                    cv::Point thisPosInTexture;
                    thisPosInTexture.x = srcPts[i].x + thisVec.dot(thisVec2)/strokeLength*(m_cvImgPencilTexture.size().width);
                    cv::Vec3d v1, v2, v3;
                    v1[0] = thisVec[0];
                    v1[1] = thisVec[1];
                    v1[2] = 0;
                    v2[0] = thisVec2[0];
                    v2[1] = thisVec2[1];
                    v2[2] = 0;
                    v3 = v1.cross(v2);

					thisPosInTexture.y = srcPts[i].y + v3[2]/(m_strokeWidth*2)*(m_cvImgPencilTexture.size().height);
                    thisPosInTexture.x = thisPosInTexture.x % m_cvImgPencilTexture.cols;
                    if (thisPosInTexture.x >= 0 && thisPosInTexture.x < m_cvImgPencilTexture.cols
                        && thisPosInTexture.y >= 0 && thisPosInTexture.y < m_cvImgPencilTexture.rows)
                    {
                        thisSegmentMask.at<int>(y, x) = 1;
                        cv::Vec3b thisPixel = m_cvImgPencilTexture.at<cv::Vec3b>(thisPosInTexture);
                        int thisR, thisG, thisB;
                        thisR = (255 - thisPixel[0])*intensity_start * (double) thisMask.at<uchar>(y, x) / ((double)MAX_MASK_VALUE);
                        thisG = (255 - thisPixel[1])*intensity_start * (double) thisMask.at<uchar>(y, x) / ((double)MAX_MASK_VALUE);
                        thisB = (255 - thisPixel[2])*intensity_start * (double) thisMask.at<uchar>(y, x) / ((double)MAX_MASK_VALUE);
                        if (thisR > 255) thisR = 255;
                        if (thisG > 255) thisG = 255;
                        if (thisB > 255) thisB = 255;
                        thisR = 255 - thisR;
                        thisG = 255 - thisG;
                        thisB = 255 - thisB;
                        thisPixel[0] = thisR;
                        thisPixel[1] = thisG;
                        thisPixel[2] = thisB;
                        thisSegPixels.push_back(SC::CStrokePoint(x + minPt.x, y + minPt.y, thisPixel));
                    }
                }
            }
        }
        prevSegmentMask = thisSegmentMask.clone();
        thisStrokePixels.push_back(thisSegPixels);
    }
}
void CStrokeHandler::PrepareThisStrokePixels2(const std::vector<cv::Point>& thisStroke, std::vector<std::vector<SC::CStrokePoint>>& thisStrokePixels, const std::vector<std::vector<float>> thisStrokeProperty)
{
    cv::Point maxPt, minPt;
    double scaleRatio = m_imgScaleRatio;
    maxPt.x = minPt.x = thisStroke[0].x / scaleRatio;
    maxPt.y = minPt.y = thisStroke[0].y / scaleRatio;
    std::vector<cv::Point> thisStrokeScaled;

    for (unsigned int i = 0; i < thisStroke.size(); ++i)
    {
        thisStrokeScaled.push_back(cv::Point(thisStroke[i].x / scaleRatio, thisStroke[i].y / scaleRatio));
        if (maxPt.x < thisStrokeScaled[i].x)
            maxPt.x = thisStrokeScaled[i].x;
        if (maxPt.y < thisStrokeScaled[i].y)
            maxPt.y = thisStrokeScaled[i].y;
        if (minPt.x > thisStrokeScaled[i].x)
            minPt.x = thisStrokeScaled[i].x;
        if (minPt.y > thisStrokeScaled[i].y)
            minPt.y = thisStrokeScaled[i].y;
    }
    maxPt += cv::Point(m_strokeWidth * 3, m_strokeWidth * 3);
    minPt -= cv::Point(m_strokeWidth * 3, m_strokeWidth * 3);
    if (minPt.x < 0)
        minPt.x = 0;
    if (minPt.y < 0)
        minPt.y = 0;
    cv::Size thisStrokeSize;
    thisStrokeSize.width = maxPt.x - minPt.x;
    thisStrokeSize.height = maxPt.y - minPt.y;
    cv::Point startPt;
    startPt.x = rand() % (m_cvImgPencilTexture2.cols - thisStrokeSize.width);
    startPt.y = rand() % (m_cvImgPencilTexture2.rows - thisStrokeSize.height);
    cv::Mat imgThisSTROI = cv::Mat(m_cvImgPencilTexture2, cv::Rect(startPt.x, startPt.y, thisStrokeSize.width, thisStrokeSize.height));
    
    std::vector<cv::Point> dstPts;
    for (unsigned int i = 0; i < thisStrokeScaled.size(); ++i)
    {
        dstPts.push_back(thisStrokeScaled[i] - minPt);
    }
    int thisStrokeWidth = m_strokeWidth / scaleRatio;
    //cv::Mat imgthisStroke = cv::Mat::zeros(thisStrokeSize, CV_8UC3);
    bool hasFadePt = true;
    if (dstPts.size() < FADE_PT_NUM * 2)
    {
        hasFadePt = false;
    }
    int numNoFadePt;
    if (hasFadePt)
    {
        numNoFadePt = dstPts.size() - FADE_PT_NUM;
    }
    else
    {
        numNoFadePt = dstPts.size();
    }
    cv::Mat thisMask = cv::Mat::zeros(thisStrokeSize, CV_8U);
    for (unsigned int i = 1; i < numNoFadePt; ++i)
    {
        //cv::line(segmentIdxs, dstPts[i - 1], dstPts[i], i, thisStrokeWidth + 2);
        cv::line(thisMask, dstPts[i - 1], dstPts[i], MAX_MASK_VALUE, thisStrokeWidth);
    }
    if (hasFadePt)
    {
        std::vector<cv::Point> fadePts;
        for (unsigned int i = numNoFadePt - 1; i < dstPts.size(); ++i)
        {
            fadePts.push_back(dstPts[i]);
        }
        GetFadeLine(thisMask, fadePts);
    }
    cv::GaussianBlur(thisMask, thisMask, cv::Size(3, 3), 0.7);
    thisStrokePixels.clear();
    //thisStrokePixels.resize(dstPts.size() - 1, std::vector<SC::CStrokePoint>());
    cv::Mat thisSegmentMask, prevSegmentMask, thisSegment;
    std::vector<SC::CStrokePoint> thisSegPixels;
    prevSegmentMask = cv::Mat::zeros(thisStrokeSize, CV_32S);
    for (unsigned int i = 1; i < dstPts.size(); ++i)
    {
        thisSegPixels.clear();
        thisSegment = cv::Mat::zeros(thisStrokeSize, CV_32S);
        thisSegmentMask = cv::Mat::zeros(thisStrokeSize, CV_32S);
        cv::line(thisSegment, dstPts[i - 1], dstPts[i], i, thisStrokeWidth + 2);
        for (unsigned int y = 0; y < thisSegment.rows; ++y)
        {
            for (unsigned int x = 0; x < thisSegment.cols; ++x)
            {
                if (thisSegment.at<int>(y, x) > 0 && prevSegmentMask.at<int>(y, x) == 0)
                { 
                    thisSegmentMask.at<int>(y, x) = 1;
                    cv::Vec3b thisPixel = imgThisSTROI.at<cv::Vec3b>(y, x);
                    int thisR, thisG, thisB;
                    thisR = (255 - thisPixel[0]) * (double) thisMask.at<uchar>(y, x) / ((double)MAX_MASK_VALUE);
                    thisG = (255 - thisPixel[1]) * (double) thisMask.at<uchar>(y, x) / ((double)MAX_MASK_VALUE);
                    thisB = (255 - thisPixel[2]) * (double) thisMask.at<uchar>(y, x) / ((double)MAX_MASK_VALUE);
                    if (thisR > 255) thisR = 255;
                    if (thisG > 255) thisG = 255;
                    if (thisB > 255) thisB = 255;
                    thisR = 255 - thisR;
                    thisG = 255 - thisG;
                    thisB = 255 - thisB;
                    thisPixel[0] = thisR;
                    thisPixel[1] = thisG;
                    thisPixel[2] = thisB;
                    thisSegPixels.push_back(SC::CStrokePoint(x + minPt.x, y + minPt.y, thisPixel));
                }
            }
        }
        prevSegmentMask = thisSegmentMask.clone();
        thisStrokePixels.push_back(thisSegPixels);
    }
    /*cv::namedWindow("thisMask");
    cv::imshow("thisMask", thisMask);*/
}
void CStrokeHandler::GetFadeLine(cv::Mat& thisMask, std::vector<cv::Point>& pts)
{
    cv::Size imgSize = thisMask.size();
    cv::Mat tempMask = cv::Mat::zeros(thisMask.size(), CV_8U);
    int thisStrokeWidth = m_strokeWidth / m_imgScaleRatio; 
    for (unsigned int i = 1; i < pts.size(); i++)
    {
        cv::line(tempMask, pts[i - 1], pts[i], 1, thisStrokeWidth);
    }
    cv::Mat fadeCoefMat = cv::Mat::zeros(thisMask.size(), CV_32S);
    std::queue<cv::Point>* thisQueue = new std::queue<cv::Point>();
    std::queue<cv::Point>* nextQueue = new std::queue<cv::Point>();
    cv::Point pt1 = pts[0];
    thisQueue->push(pt1);
    int crtStep = 1;
    if (pt1.x < 0 || pt1.x >= imgSize.width || pt1.y < 0 || pt1.y >= imgSize.height)
    {
        return;
    }
    while (true)
    {
        //std::cout << "while" << std::endl; 
        while (!thisQueue->empty())
        {
            //std::cout << crtStep << " ";
            cv::Point thisPt = thisQueue->front();
            thisQueue->pop();
            fadeCoefMat.at<int>(thisPt) = crtStep;
            cv::Point topPt = cv::Point(thisPt.x, thisPt.y - 1);
            cv::Point bottomPt = cv::Point(thisPt.x, thisPt.y + 1);
            cv::Point leftPt = cv::Point(thisPt.x - 1, thisPt.y);
            cv::Point rightPt = cv::Point(thisPt.x + 1, thisPt.y);
            cv::Point topLeftPt = cv::Point(thisPt.x - 1, thisPt.y - 1);
            cv::Point topRightPt = cv::Point(thisPt.x + 1, thisPt.y - 1);
            cv::Point bottomLeftPt = cv::Point(thisPt.x - 1, thisPt.y + 1);
            cv::Point bottomRightPt = cv::Point(thisPt.x + 1, thisPt.y + 1);

            if (topPt.y >=0 && fadeCoefMat.at<int>(topPt) == 0 
                && tempMask.at<uchar>(topPt) == 1)
            {
                nextQueue->push(topPt);
                fadeCoefMat.at<int>(topPt) = -1;
            }
            if (bottomPt.y < imgSize.height && fadeCoefMat.at<int>(bottomPt) == 0 
                && tempMask.at<uchar>(bottomPt) == 1)
            {
                nextQueue->push(bottomPt);
                fadeCoefMat.at<int>(bottomPt) = -1;
            }
            if (leftPt.x >=0 && fadeCoefMat.at<int>(leftPt) == 0 
                && tempMask.at<uchar>(leftPt) == 1)
            {
                nextQueue->push(leftPt);
                fadeCoefMat.at<int>(leftPt) = -1;
            }
            if (rightPt.x < imgSize.width && fadeCoefMat.at<int>(rightPt) == 0 
                && tempMask.at<uchar>(rightPt) == 1)
            {
                nextQueue->push(rightPt);
                fadeCoefMat.at<int>(rightPt) = -1;
            }
            if (topLeftPt.y >=0 && topLeftPt.x >=0 && fadeCoefMat.at<int>(topLeftPt) == 0 
                && tempMask.at<uchar>(topLeftPt) == 1)
            {
                nextQueue->push(topLeftPt);
                fadeCoefMat.at<int>(topLeftPt) = -1;
            }
            if (topRightPt.y >=0 && topRightPt.x < imgSize.width && fadeCoefMat.at<int>(topRightPt) == 0 
                && tempMask.at<uchar>(topRightPt) == 1)
            {
                nextQueue->push(topRightPt);
                fadeCoefMat.at<int>(topRightPt) = -1;
            }
            if (bottomLeftPt.y < imgSize.height && bottomLeftPt.x >=0 && fadeCoefMat.at<int>(bottomLeftPt) == 0 
                && tempMask.at<uchar>(bottomLeftPt) == 1)
            {
                nextQueue->push(bottomLeftPt);
                fadeCoefMat.at<int>(bottomLeftPt) = -1;
            }
            if (bottomRightPt.y < imgSize.height && bottomRightPt.x < imgSize.width && fadeCoefMat.at<int>(bottomRightPt) == 0 
                && tempMask.at<uchar>(bottomRightPt) == 1)
            {
                nextQueue->push(bottomRightPt);
                fadeCoefMat.at<int>(bottomRightPt) = -1;
            }
        }
        std::queue<cv::Point>* tempQueue = nextQueue;
        nextQueue = thisQueue;
        thisQueue = tempQueue;
        crtStep++;
        if (thisQueue->empty())
        {
            break;
        }
    }
    //std::cout << std::endl;

    for (int i = 0; i < imgSize.height; ++i)
    {
        for (int j = 0; j < imgSize.width; ++j)
        {
            if (tempMask.at<uchar>(i, j) == 1)
            {
                int thisStep = fadeCoefMat.at<int>(i, j);
                //std::cout << thisStep << " ";
                double thisRatio = (crtStep - thisStep) / (double)(crtStep - 10);
                if (thisRatio > 1)
                {
                    thisRatio = 1;
                }
                if (thisMask.at<uchar>(i, j) == 0)
                {
                    thisMask.at<uchar>(i, j) = MAX_MASK_VALUE * thisRatio;
                    //std::cout << thisRatio << " ";
                    //std::cout << (int)thisMask.at<uchar>(i, j) << std::endl;
                }
            }
        }
    }

    delete nextQueue;
    delete thisQueue;
}
void CStrokeHandler::PrepareAllStrokePixels()
{
    if (m_allStrokes.size() == 0)
        return;
    m_allStrokePixels.clear();
    m_allStrokePixels.resize(m_allStrokes.size(), std::vector<std::vector<SC::CStrokePoint>>());
    for (unsigned int i = 0; i < m_allStrokes.size(); ++i)
    {
        if (m_allStrokes[i].size() == 0)
        {
            continue;
        }
        switch (m_strokeType)
        {
			
        case 1:
            PrepareThisStrokePixels(m_allStrokes[i], m_allStrokePixels[i],m_allPointProperties[i]);
            break;
        case 2:
            PrepareThisStrokePixels2(m_allStrokes[i], m_allStrokePixels[i],m_allPointProperties[i]);
            break;

        }
    }
}
