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
	m_valid = false;
	m_index = -1;
	m_length = 0.0f;
	m_startJoint = Vec3f(0, 0, 0);
	m_endJoint = Vec3f(0, 0, 0);
	m_rotation = Mat::eye(3, 3, CV_32FC1);
	m_transformation = Mat();
}

// initialize part with joint and matrix information
void Part::Initialize(
	const NUI_SKELETON_DATA& skeletonData,
	const vector<Vec3f>& skeletonJoints,
	const vector<Mat>& absoluteMatrices,
	const int partIndex)
{
	// initialize with default values
	Initialize();

	// set joints and valid flag for rest of procedures
	if(partIndex < 0 || partIndex > 18) return ;
	m_index = partIndex;
	const int endIndex = partIndex + 1;
	const int startIndex = parent_indices[endIndex];
	m_startJoint = skeletonJoints[startIndex];
	m_endJoint = skeletonJoints[endIndex];
	const Vec3f delta = m_endJoint - m_startJoint;
	m_length = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
	m_valid = skeletonData.eSkeletonPositionTrackingState[startIndex] == NUI_SKELETON_POSITION_TRACKED &&
		skeletonData.eSkeletonPositionTrackingState[endIndex] == NUI_SKELETON_POSITION_TRACKED;

	// compute rotation if valid
	if(!m_valid) return;
	m_rotation = absoluteMatrices[endIndex];
}