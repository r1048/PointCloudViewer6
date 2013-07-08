#include "Resolution.h"

const NUI_IMAGE_RESOLUTION Resolution::m_colorResolution = NUI_IMAGE_RESOLUTION_1280x960;
const NUI_IMAGE_RESOLUTION Resolution::m_depthResolution = NUI_IMAGE_RESOLUTION_640x480;
const NUI_IMAGE_RESOLUTION Resolution::m_indexResolution = NUI_IMAGE_RESOLUTION_640x480;
const NUI_IMAGE_RESOLUTION Resolution::m_pointResolution = NUI_IMAGE_RESOLUTION_640x480;

const Size Resolution::depthSize = Size(depthWidth, depthHeight);
const Size Resolution::colorSize = Size(colorWidth, colorHeight);

const float Resolution::FX = 5.9421434211923247e+02f;
const float Resolution::FY = 5.9104053696870778e+02f;
const float Resolution::CX = 3.3930780975300314e+02f;
const float Resolution::CY = 2.4273913761751615e+02f;

void Resolution::InitColorMatrix(Mat& colorMatrix)
{
	if(colorMatrix.empty()) colorMatrix = Mat::zeros(colorHeight, colorWidth, CV_8UC3);
	else memset(colorMatrix.ptr<unsigned char>(0), 0, sizeof(unsigned char) * colorCount * 3);
}

void Resolution::InitDepthMatrix(Mat& depthMatrix)
{
	if(depthMatrix.empty()) depthMatrix = Mat::zeros(depthHeight, depthWidth, CV_16UC1);
	else memset(depthMatrix.ptr<unsigned short>(0), 0, sizeof(unsigned short) * depthCount);
}

void Resolution::InitPointMatrix(Mat& pointMatrix)
{
	if(pointMatrix.empty()) pointMatrix = Mat::zeros(depthHeight, depthWidth, CV_32FC3);
	else memset(pointMatrix.ptr<float>(0), 0, sizeof(float) * depthCount * 3);
}

void Resolution::InitIndexMatrix(Mat& indexMatrix)
{
	if(indexMatrix.empty()) indexMatrix = Mat::zeros(depthHeight, depthWidth, CV_32SC1);
	else memset(indexMatrix.ptr<int>(0), 0, sizeof(int) * depthCount);
}


