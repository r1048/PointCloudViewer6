#include "Transformation.h"

void Transformation::Initialize(void)
{
	m_transformation.clear();
}

bool Transformation::IsValid(void) const
{
	if(m_transformation.size() != NUI_SKELETON_POSITION_COUNT - 1) return false;
	else return true;
}

void Transformation::ComputeTransformation(
	const Skeleton& refSkeleton,
	const Skeleton& newSkeleton)
{
	// compute transformations
	m_transformation.clear();
	const int nBone = NUI_SKELETON_POSITION_COUNT - 1;

	// generate trans list
	const vector<Vec3f>& refJoints = refSkeleton.GetSkeletonJoints();
	const vector<Vec3f>& newJoints = newSkeleton.GetSkeletonJoints();

	vector<Vec3f> refTransList;
	vector<Vec3f> newTransList;
	for(int ii = 0; ii < 20; ii++)
	{
		const int parentIndex = Skeleton::GetParentIndex(ii);
		const Vec3f& currRefJoint = refJoints[ii];
		const Vec3f& parentRefJoint = refJoints[parentIndex];
		const Vec3f& currNewJoint = newJoints[ii];
		const Vec3f& parentNewJoint = newJoints[parentIndex];
		const Vec3f refTrans = currRefJoint - parentRefJoint;
		const Vec3f newTrans = currNewJoint - parentNewJoint;
		refTransList.push_back(refTrans);
		newTransList.push_back(newTrans);
	}

	// generate abs rot list
	const vector<Mat>& refRotations = refSkeleton.m_absoluteMatrices;
	const vector<Mat>& newRotations = newSkeleton.m_absoluteMatrices;

	// generate len list
	vector<float> refLengthList;
	vector<float> newLengthList;
	for(int ii = 0; ii < 20; ii++)
	{
		float refValue = 0.0f;
		float newValue = 0.0f;
		for(int jj = 0; jj < 3; jj++)
		{
			refValue += refTransList[ii][jj] * refTransList[ii][jj];
			newValue += newTransList[ii][jj] * newTransList[ii][jj];
		}
		refLengthList.push_back(sqrtf(refValue));
		newLengthList.push_back(sqrtf(newValue));
	}

	// generate trans list
	for(int ii = 0; ii < 20; ii++)
	{
		if(ii == 0) continue;
		const float ratio = refLengthList[ii] / newLengthList[ii];
		const Mat rotation = ratio * refRotations[ii].inv() * newRotations[ii];
		const int parentIndex = Skeleton::GetParentIndex(ii);
		Mat transformation = Mat::zeros(4, 4, CV_32FC1);
		for(int rr = 0; rr < 3; rr++)
			for(int cc = 0; cc < 3; cc++)
				transformation.at<float>(rr, cc) = rotation.at<float>(rr, cc);
		const Vec3f translation = refJoints[parentIndex] - ApplyTransform3(rotation, newJoints[parentIndex]);
		for(int rr = 0; rr < 3; rr++)
			transformation.at<float>(rr, 3) = translation[rr];
		transformation.at<float>(3, 3) = 1.0f;
		m_transformation.push_back(transformation);
	}
}


Mat Transformation::TransformSkeletonFrame(
	const Mat& skeletonFrame,
	const Mat& labelFrame,
	const NUI_SKELETON_POSITION_TRACKING_STATE* stateList)
{
	Mat transformedSkeleton;
	if(!IsValid()) return transformedSkeleton;
	InitPointMatrix(transformedSkeleton);

	vector<bool> validSegment;
	for(int ii = 0; ii < 19; ii++)
	{
		const int currIndex = ii + 1;
		const int parentIndex = Skeleton::GetParentIndex(currIndex);
		if(stateList[currIndex] == NUI_SKELETON_POSITION_TRACKED
			&& stateList[parentIndex] == NUI_SKELETON_POSITION_TRACKED)
			validSegment.push_back(true);
		else validSegment.push_back(false);
	}

	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++)
		{
			Vec3f point = skeletonFrame.at<Vec3f>(rr, cc);
			if(point == Vec3f(0, 0, 0)) continue;
			const int label = labelFrame.at<int>(rr, cc);
			if(label < 0 || label >= NUI_SKELETON_POSITION_COUNT - 1) continue;
			//if(stateList[label+1] != NUI_SKELETON_POSITION_TRACKED) continue;
			if(!validSegment[label]) continue;
			
			const Mat& transformation = m_transformation[label];
			Vec3f newPoint = ApplyTransform4(transformation, point);
			transformedSkeleton.at<Vec3f>(rr, cc) = newPoint;
		}
	}

	return transformedSkeleton;
}

void Transformation::MultiplyMatrix(const Mat& transformation, Vec3f& vector)
{
	if(vector == Vec3f(0, 0, 0)) return ;
	
	Mat vector4 = Mat::zeros(4, 1, CV_32FC1);
	vector4.at<float>(0, 0) = vector[0];
	vector4.at<float>(1, 0) = vector[1];
	vector4.at<float>(2, 0) = vector[2];
	vector4.at<float>(3, 0) = 1.0f;

	vector4 = transformation * vector4;
	vector[0] = vector4.at<float>(0, 0);
	vector[1] = vector4.at<float>(1, 0);
	vector[2] = vector4.at<float>(2, 0);
}

Vec3f Transformation::ApplyTransform3(const Mat& transform3x3, const Vec3f& vector)
{
	if(vector == Vec3f(0, 0, 0)) return vector;
	Vec3f newVector(0, 0, 0);
	Mat mVector = Mat::zeros(3, 1, CV_32FC1);
	for(int ii = 0; ii < 3; ii++)
		mVector.at<float>(ii, 0) = vector[ii];
	mVector = transform3x3 * mVector;
	for(int ii = 0; ii < 3; ii++)
		newVector[ii] = mVector.at<float>(ii, 0);
	return newVector;
}

Vec3f Transformation::ApplyTransform4(const Mat& transform4x4, const Vec3f& vector)
{
	if(vector == Vec3f(0, 0, 0)) return vector;
	Vec3f newVector(0, 0, 0);
	Mat mVector = Mat::zeros(4, 1, CV_32FC1);
	for(int ii = 0; ii < 3; ii++)
		mVector.at<float>(ii, 0) = vector[ii];
	mVector.at<float>(3, 0) = 1.0f;
	mVector = transform4x4 * mVector;
	for(int ii = 0; ii < 3; ii++)
		newVector[ii] = mVector.at<float>(ii, 0);
	return newVector;
}

