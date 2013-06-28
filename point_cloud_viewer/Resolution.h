#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/flann/flann.hpp>
#include <Windows.h>
#include <NuiApi.h>
#include <JunhaLibrary.h>

using namespace std;
using namespace cv;
using namespace cvflann;
using namespace JunhaLibrary;


class Resolution
{
public:
	Resolution() {}
	~Resolution() {}
	
	static const float FX;
	static const float FY;
	static const float CX;
	static const float CY;

protected:
	virtual void Initialize() {}
	virtual bool IsValid() const {return false;}

	static const NUI_IMAGE_RESOLUTION m_colorResolution;
	static const NUI_IMAGE_RESOLUTION m_depthResolution;
	static const NUI_IMAGE_RESOLUTION m_pointResolution;
	static const NUI_IMAGE_RESOLUTION m_indexResolution;

	static const int depthWidth = 640;
	static const int depthHeight = 480;
	static const int depthCount = depthWidth * depthHeight;
	static const int colorWidth = 1280;
	static const int colorHeight = 960;
	static const int colorCount = colorWidth * colorHeight;

	static void InitColorMatrix(Mat& colorMatrix);
	static void InitDepthMatrix(Mat& depthMatrix);
	static void InitPointMatrix(Mat& pointMatrix);
	static void InitIndexMatrix(Mat& indexMatrix);

	static bool SaveMatrix(FileStorage& fs, const string tag, const Mat& data);
	static bool LoadMatrix(FileStorage& fs, const string tag, Mat& data);
};