#include "NormalVector.h"


NormalVector::NormalVector(void)
{
	Initialize();
}


NormalVector::~NormalVector(void)
{
}

void NormalVector::Initialize(void)
{
}

bool NormalVector::IsValid(void) const
{
	return true;
}

Mat NormalVector::ComputeNormalVector(const Mat& pointMatrix)
{
	Mat normalMatrix;
	if(pointMatrix.empty()) return normalMatrix;

	vector<Vec3f> pointList;
	vector<int> labelList;
	Mat queryPoint;
	Mat queryResponse;
	Mat labelMatrix = Mat::ones(DEPTH_HEIGHT, DEPTH_WIDTH, CV_32SC1) * -1;

	// generate label
	int label = 0;
	for(int rr = 0; rr < DEPTH_HEIGHT; rr++)
	{
		for(int cc = 0; cc < DEPTH_WIDTH; cc++)
		{
			Vec3f vec = pointMatrix.at<Vec3f>(rr, cc);
			if(vec == Vec3f(0, 0, 0)) continue;

			pointList.push_back(vec);
			labelList.push_back(label);
			labelMatrix.at<int>(rr, cc) = label;
			label++;
		}
	}

	if(pointList.size() != labelList.size() ||
		pointList.size() != label)
		return normalMatrix;
	const int nLabel = label;

	// generate query data
	queryPoint = Mat::zeros(nLabel, 1, CV_32FC3);
	for(int ii = 0; ii < nLabel; ii++)
		queryPoint.at<Vec3f>(ii, 0) = pointList[ii];
	queryPoint = queryPoint.reshape(1);
	queryResponse = Mat::zeros(nLabel, 1, CV_32SC1);
	for(int ii = 0; ii < nLabel; ii++)
		queryResponse.at<int>(ii, 0) = labelList[ii];

	// train a kNN
	KNearest knn;
	knn.train(queryPoint, queryResponse);

	// nearest neighbor list
	const int nn = 12;
	Mat indices, dists;
	knn.find_nearest(queryPoint, nn + 1, Mat(), indices, dists);

	// estimate normal vector using nearest neighbors
	queryPoint = queryPoint.reshape(3);
	Mat normalVector = Mat::zeros(nLabel, 1, CV_32FC3);
	for(int ii = 0; ii < nLabel; ii++)
	{
		const Vec3f& defaultPoint = queryPoint.at<Vec3f>(ii, 0);
		Mat neighborMatrix = Mat::zeros(3, nn, CV_32FC1);
		for(int jj = 1; jj < nn + 1; jj++)
		{
			const int label = static_cast<int>(indices.at<float>(ii, jj));
			const Vec3f& point = queryPoint.at<Vec3f>(label, 0) - defaultPoint;
			for(int kk = 0; kk < 3; kk++)
				neighborMatrix.at<float>(kk, jj - 1) = point[kk];
		}

		SVD svd;
		svd(neighborMatrix);
		const Mat normal = svd.u.col(2);
		Vec3f normalVec3(normal.at<float>(0), normal.at<float>(1), normal.at<float>(2));
		Segmentation::Normalize(normalVec3);
		if(normalVec3[2] > 0) normalVec3 = -normalVec3;
		normalVector.at<Vec3f>(ii, 0) = normalVec3;
	}

	normalMatrix = Mat::zeros(DEPTH_HEIGHT, DEPTH_WIDTH, CV_32FC3);
	for(int rr = 0; rr < DEPTH_HEIGHT; rr++)
	{
		for(int cc = 0; cc < DEPTH_WIDTH; cc++)
		{
			const int label = labelMatrix.at<int>(rr, cc);
			if(label >= 0)
				normalMatrix.at<Vec3f>(rr, cc) = normalVector.at<Vec3f>(label, 0);
		}
	}

	return normalMatrix;
}