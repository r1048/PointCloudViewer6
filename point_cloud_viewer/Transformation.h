#pragma once

#include <math.h>
#include <iostream>
#include <fstream>

#include <opencv2\opencv.hpp>	// OpenCV header file
#include <Windows.h>			// Kinect SDK header files
#include <NuiApi.h>				// Kinect SDK header files
#include <gl\freeglut.h>		// OpenGL header files
#include <opencv2\ml\ml.hpp>	// Use KNN

#include "Resolution.h"
#include "Skeleton.h"

using namespace cv;
using namespace std;

class Transformation : public Resolution
{
public:
	Transformation() {Initialize();}
	Transformation(const Skeleton& refSkeleton,
		const Skeleton& newSkeleton)
		{ComputeTransformations(refSkeleton, newSkeleton);}
	~Transformation() {}

	void Initialize(void);
	bool IsValid(void) const;

	vector<Mat> m_transformation;
	bool ComputeTransformations(
		const Skeleton& refSkeleton,
		const Skeleton& newSkeleton);

	Mat TransformSkeletonFrame(
		const Mat& skeletonFrame,
		const Mat& labelFrame);

	const vector<Mat>& GetTransformations(void) const {return m_transformation;}
};