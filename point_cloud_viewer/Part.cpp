#include "Part.h"

const int Part::parent_indices[20] = 
	{0, 0, 1, 2, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 14, 0, 16, 17, 18};


Part::Part(void)
{
	Initialize();
}

Part::~Part(void)
{
}

void Part::Initialize(void)
{
	m_trackingState = 0;
	m_partIndex = -1;
	m_length = 0.0f;
	m_startJoint = Vec3f(0, 0, 0);
	m_endJoint = Vec3f(0, 0, 0);
	m_rotation = Mat::eye(3, 3, CV_32FC1);
}

// initialize part with joint and matrix information
void Part::Initialize(
	const vector<int>& trackingStates,
	const vector<Vec3f>& skeletonJoints,
	const vector<Mat>& absoluteMatrices,
	const int partIndex)
{
	// initialize with default values
	Initialize();

	// assertion
	if(trackingStates.size() != N_JOINT ||
		skeletonJoints.size() != N_JOINT ||
		absoluteMatrices.size() != N_JOINT ||
		partIndex < 0 || partIndex >= N_PART)
		return ;

	// set joints and valid flag for rest of procedures
	const int endIndex = partIndex + 1;
	const int startIndex = parent_indices[endIndex];
	m_trackingState = std::min<int>(trackingStates[startIndex], trackingStates[endIndex]);
	m_partIndex = partIndex;
	m_startJoint = skeletonJoints[startIndex];
	m_endJoint = skeletonJoints[endIndex];
	const Vec3f delta = m_endJoint - m_startJoint;
	m_length = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);

	// compute rotation if valid
	if(IsValid() == true)
		m_rotation = absoluteMatrices[endIndex];
}

void Part::Initialize(
	const vector<int>& partStates,
	const vector<float>& skeletonJoints,
	const vector<Mat>& absoluteMatrices,
	const int partIndex)
{
	// initialize with default values
	Initialize();

	// assertion
	if(partStates.size() != N_PART ||
		skeletonJoints.size() != 3 * N_JOINT ||
		absoluteMatrices.size() != N_PART ||
		partIndex < 0 || partIndex >= N_PART)
		return ;

	// set joints and valid flag for rest of procedures
	m_trackingState = partStates[partIndex];
	m_partIndex = partIndex;
	const int endIndex = partIndex + 1;
	const int startIndex = parent_indices[endIndex];
	for(int ii = 0; ii < 3; ii++)
	{
		m_startJoint[ii] = skeletonJoints[3 * startIndex + ii];
		m_endJoint[ii] = skeletonJoints[3 * endIndex + ii];
	}
	const Vec3f delta = m_endJoint - m_startJoint;
	m_length = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);

	// compute rotation if valid
	if(IsValid() == true)
		m_rotation = absoluteMatrices[partIndex];
}

// Compute a 4-by-4 transformation matrix given reference part
Mat Part::GetTransform(const Part& ref) const
{
	// handle exceptions: invalid case, equal case
	if(this->IsValid() == false || ref.IsValid() == false) return Mat();
	if(this == &ref) return Mat::eye(4, 4, CV_32FC1);

	// declare transformation variables
	Mat transformation1 = Mat::eye(4, 4, CV_32FC1);
	Mat transformation2 = Mat::eye(4, 4, CV_32FC1);

	// prepare R, t for transformation1 from current part [R -Rt; 0 0 0 1];
	Mat translation1 = -StorageHandler::Multiply_Mat_from_Mat3x3_by_Vec3f(m_rotation, m_startJoint);
	Mat rotation1 = m_rotation;

	// prepare R, t for transformation2 from reference part [inv(R) t; 0 0 0 1];
	Mat translation2 = Mat::zeros(3, 1, CV_32FC1);
	for(int rr = 0; rr < 3; rr++)
		translation2.at<float>(rr, 0) = ref.GetStartJoint()[rr];
	Mat rotation2 = ref.GetRotation().inv();
	
	// set transformations
	for(int rr = 0; rr < 3; rr++)
	{
		for(int cc = 0; cc < 3; cc++) {
			transformation1.at<float>(rr, cc) = rotation1.at<float>(rr, cc);
			transformation2.at<float>(rr, cc) = rotation2.at<float>(rr, cc);
		}
		transformation1.at<float>(rr, 3) = translation1.at<float>(rr, 0);
		transformation2.at<float>(rr, 3) = translation2.at<float>(rr, 0);
	}

	// return transformation
	return transformation2 * transformation1;
}