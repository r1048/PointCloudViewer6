#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/flann/flann.hpp>
#include <Windows.h>
#include <NuiApi.h>
#include <JunhaLibrary.h>

#include "Resolution.h"
#include "Skeleton.h"

using namespace std;
using namespace cv;
using namespace cvflann;
using namespace JunhaLibrary;

class Segmentation : public Resolution
{
public:
	Segmentation() {Initialize();}
	~Segmentation() {}

protected:
	static const int N_PART = Skeleton::N_PART;

public:
	static void ComputeLabel(
		const vector<Vec3f>& skeletonPoints,
		const Mat& pointMatrix,
		Mat& labelMatrix);

	static void ComputeLabelAndNorm(
		const vector<Vec3f>& skeletonPoints,
		const Mat& pointMatrix,
		const Mat& normalMatrix,
		Mat& labelMatrix,
		Mat& normSimilarity);

	bool IsValid() const;

	void Initialize();

	// functions used for compute label
	static void ComputeDistanceAndNorm(
		const vector<Vec3f>& skeletonPoints,
		const vector<Vec3f>& pointList,
		const vector<Vec3f>& normList,
		Mat& dists,
		Mat& normSimilarity);

protected:
	static float ComputeLambda(
		const Vec3f& p1,
		const Vec3f& p2,
		const Vec3f& pq);

	static void ComputeDistanceAndNorm(
		const Vec3f& p1,
		const Vec3f& p2,
		const Vec3f& pq,
		const Vec3f& norm,
		float& distance,
		float& normSimilarity);

public:
	static float ComputeNorm(const Vec3f& vector);
	static Vec3f OuterProduct(const Vec3f& v1, const Vec3f& v2);
	static float InnerProduct(const Vec3f& v1, const Vec3f& v2);
	static void Normalize(Vec3f& vector);
};