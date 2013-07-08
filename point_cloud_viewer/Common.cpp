#include "Common.h"

Mat Parameter::InitMatrix(const int type)
{
	return Mat::zeros(DEPTH_HEIGHT, DEPTH_WIDTH, type);
}

Mat Parameter::InitColorMatrix(void)
{
	return Mat::zeros(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC3);
}