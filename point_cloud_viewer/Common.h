#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/flann/flann.hpp>
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
}

using namespace Parameter;