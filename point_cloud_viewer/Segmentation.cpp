#include "Segmentation.h"

void Segmentation::Initialize(void)
{
}

bool Segmentation::IsValid() const
{
	return true;
}

float Segmentation::ComputeNorm(const Vec3f& vector)
{
	return sqrtf(InnerProduct(vector, vector));
}

Vec3f Segmentation::OuterProduct(const Vec3f& v1, const Vec3f& v2)
{
	Vec3f v3;
	v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
	v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
	v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
	return v3;
}

float Segmentation::InnerProduct(const Vec3f& v1, const Vec3f& v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void Segmentation::Normalize(Vec3f& vector)
{
	const float norm = ComputeNorm(vector);
	const float epsilon = 1e-5f;
	if(norm < epsilon) return ;
	else vector /= norm;
}


float Segmentation::ComputeLambda(
	const Vec3f& p1,
	const Vec3f& p2,
	const Vec3f& pq)
{
	const Vec3f deltaLine = p1 - p2;
	const Vec3f deltaQuery = pq - p2;
	const float lengthSquare = InnerProduct(deltaLine, deltaLine);
	if(lengthSquare < 1e-20F) return FLT_MAX;
	const float innerQuery = InnerProduct(deltaLine, deltaQuery);
	return innerQuery / lengthSquare;
}

void Segmentation::ComputeDistanceAndNorm(
	const Vec3f& p1,
	const Vec3f& p2,
	const Vec3f& pq,
	const Vec3f& norm,
	float& distance,
	float& normSimilarity)
{
	const bool normMode = norm != Vec3f(0, 0, 0);
	distance = FLT_MAX;
	normSimilarity = 0.0f;

	const float lambda = ComputeLambda(p1, p2, pq);
	bool inside = (lambda >= 0.0f && lambda <= 1.0f) ? true : false;
	
	const Vec3f projectedPoint = lambda * p1 + (1.0f - lambda) * p2;
	Vec3f interPoint;
	if(inside == true) interPoint = lambda * p1 + (1.0f - lambda) * p2;
	else if(lambda < 0.0f) interPoint = p1;
	else interPoint = p2;

	if(inside == true) distance = ComputeNorm(pq - projectedPoint);
	else 
	{
		const float dist1 = ComputeNorm(pq - p1);
		const float dist2 = ComputeNorm(pq - p2);
		distance = dist1 < dist2 ? dist1 : dist2;
	}

	if(normMode)
	{
		Vec3f normQuery = pq - projectedPoint;
		Normalize(normQuery);
		normSimilarity = InnerProduct(normQuery, norm);
	}
}

void Segmentation::ComputeDistanceAndNorm(
	const Skeleton& skeleton,
	const vector<Vec3f>& pointList,
	const vector<Vec3f>& normList,
	Mat& dists,
	Mat& normSimilarity)
{
	// assertion
	if(skeleton.IsValid() == false) return;
	if(pointList.size() == 0) return;
	const int count = pointList.size();
	const bool normMode = pointList.size() == normList.size();

	// compute dists and normSimilarity
	dists = Mat::zeros(count, N_PART, CV_32FC1);
	if(normMode) normSimilarity = Mat::zeros(count, N_PART, CV_32FC1);
	for(int ii = 0; ii < count; ii++)
	{
		const Vec3f& pointQuery = pointList[ii];
		Vec3f normQuery(0, 0, 0);
		if(normMode) normQuery = normList[ii];
		for(int jj = 0; jj < N_PART; jj++)
		{
			const Part& part = skeleton.GetPart(jj);
			const Vec3f& startJoint = part.GetStartJoint();
			const Vec3f& endJoint = part.GetEndJoint();

			float dist, normSim;
			ComputeDistanceAndNorm(
				startJoint,
				endJoint,
				pointQuery,
				normQuery,
				dist,
				normSim);
			dists.at<float>(ii, jj) = dist;
			if(normMode) normSimilarity.at<float>(ii, jj) = normSim;
		}
	}
}

void Segmentation::ComputeLabel(
	const Skeleton& skeleton,
	const Mat& pointMatrix,
	Mat& labelMatrix)
{
	ComputeLabelAndNorm(
		skeleton,
		pointMatrix,
		Mat(),
		labelMatrix,
		Mat());
}

void Segmentation::ComputeLabelAndNorm(
	const Skeleton& skeleton,
	const Mat& pointMatrix,
	const Mat& normalMatrix,
	Mat& labelMatrix,
	Mat& normSimilarity)
{
	// assertion
	labelMatrix = Mat();
	if(pointMatrix.rows != depthHeight || pointMatrix.cols != depthWidth) return ;
	if(pointMatrix.empty()) return ;
	const bool normMode = !normalMatrix.empty();

	// count valid points
	int count = 0;
	vector<Vec3f> pointList;
	vector<Vec3f> normList;
	vector< pair<int, int> > indexList;
	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++)
		{
			const Vec3f& point = pointMatrix.at<Vec3f>(rr, cc);
			if(point == Vec3f(0.0f, 0.0f, 0.0f)) continue;
			else
			{
				pointList.push_back(point);
				if(normMode) normList.push_back(normalMatrix.at<Vec3f>(rr, cc));
				indexList.push_back(pair<int, int>(rr, cc));
			}
		}
	}

	// compute normSim and distances
	Mat dists;
	Segmentation::ComputeDistanceAndNorm(
		skeleton,
		pointList,
		normList,
		dists,
		normSimilarity);

	// set label wrt distance and norm
	vector<int> labelList;
	const float maxRatio = 0.4f;
	for(int ii = 0; ii < count; ii++)
	{
		int index1 = -1;
		int index2 = -1;
		float minDist1 = FLT_MAX;
		float minDist2 = FLT_MAX;
		for(int jj = 0; jj < N_PART; jj++)
		{
			const float dist = dists.at<float>(ii, jj);
			if(dist < minDist1)
			{
				minDist2 = minDist1;
				minDist1 = dist;
				index2 = index1;
				index1 = jj;
			}
		}

		if(!normMode) labelList.push_back(index1);
		else
		{
			const float ratio = minDist1 / minDist2;
			if(ratio < maxRatio) labelList.push_back(index1);
			else
			{
				if(index1 < 0 || index2 < 0) cout << "invalid index" << endl;
				const float normSim1 = normSimilarity.at<float>(ii, index1);
				const float normSim2 = normSimilarity.at<float>(ii, index2);
				const float ratio1 = minDist1 / normSim1;
				const float ratio2 = minDist2 / normSim2;
				labelList.push_back(ratio1 < ratio2 ? index1 : index2);
			}
		}
	}

	// generate label matrix from lists
	labelMatrix = Mat::zeros(depthHeight, depthWidth, CV_32SC1);
	for(int ii = 0; ii < count; ii++)
		labelMatrix.at<int>(indexList[ii].first, indexList[ii].second) = labelList[ii];
}