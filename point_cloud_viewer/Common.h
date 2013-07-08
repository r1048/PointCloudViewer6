#pragma once

#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <NuiApi.h>
#include <JunhaLibrary.h>

using namespace std;
using namespace cv;
using namespace JunhaLibrary;

namespace Parameter
{
	const int N_JOINT = 20;
	const int N_PART = 19;

	const int DEPTH_WIDTH = 640;
	const int DEPTH_HEIGHT = 480;
	const int DEPTH_COUNT = DEPTH_WIDTH * DEPTH_HEIGHT;
	const Size DEPTH_SIZE = Size(DEPTH_WIDTH, DEPTH_HEIGHT);
	const int COLOR_WIDTH = 1280;
	const int COLOR_HEIGHT = 960;

	const float FOCAL_LENGTH_X = 5.9421434211923247e+02f;
	const float FOCAL_LENGTH_Y = 5.9104053696870778e+02f;
	const float CENTER_X = 3.3930780975300314e+02f;
	const float CENTER_Y = 2.4273913761751615e+02f;

	const NUI_IMAGE_RESOLUTION COLOR_RESOLUTION = NUI_IMAGE_RESOLUTION_1280x960;
	const NUI_IMAGE_RESOLUTION DEPTH_RESOLUTION = NUI_IMAGE_RESOLUTION_640x480;
	const NUI_IMAGE_RESOLUTION INDEX_RESOLUTION = NUI_IMAGE_RESOLUTION_640x480;
	const NUI_IMAGE_RESOLUTION POINT_RESOLUTION = NUI_IMAGE_RESOLUTION_640x480;

	Mat InitMatrix(const int type);
	Mat InitColorMatrix(void);
}

using namespace Parameter;