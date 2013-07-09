#include "Transformation.h"

void Transformation::Initialize(void)
{
	m_transformation.clear();
}

bool Transformation::IsValid(void) const
{
	if(m_transformation.size() != N_PART) return false;
	else return true;
}

bool Transformation::ComputeTransformations(
	const Skeleton& refSkeleton,
	const Skeleton& newSkeleton)
{
	// initialization
	Initialize();
	if(refSkeleton.IsValid() == false ||
		newSkeleton.IsValid() == false) false;

	// compute transformations
	for(int ii = 0; ii < N_PART; ii++)
	{
		const Part& newPart = newSkeleton.GetPart(ii);
		const Part& refPart = refSkeleton.GetPart(ii);
		m_transformation.push_back(newPart.GetTransform(refPart));
	}
	return true;
}


Mat Transformation::TransformSkeletonFrame(
	const Mat& skeletonFrame,
	const Mat& labelFrame)
{
	// initialization
	Mat transformedSkeletonFrame;
	if(IsValid() == false || skeletonFrame.empty() || labelFrame.empty())
		return transformedSkeletonFrame;
	transformedSkeletonFrame = InitMatrix(CV_32FC3);

	// transform pixel by pixel
	for(int rr = 0; rr < DEPTH_HEIGHT; rr++)
	{
		for(int cc = 0; cc < DEPTH_WIDTH; cc++)
		{
			// get point
			Vec3f point = skeletonFrame.at<Vec3f>(rr, cc);
			if(point == Vec3f(0, 0, 0)) continue;

			// get label
			const int label = labelFrame.at<int>(rr, cc);
			if(label < 0 || label >= N_PART) continue;

			// get transformation from valid label
			const Mat& transformation = m_transformation[label];
			if(transformation.empty()) continue;

			// transform point
			Vec3f newPoint = StorageHandler::Multiply_Vec3f_from_Mat4x4_by_Vec3f(transformation, point);
			transformedSkeletonFrame.at<Vec3f>(rr, cc) = newPoint;
		}
	}

	return transformedSkeletonFrame;
}