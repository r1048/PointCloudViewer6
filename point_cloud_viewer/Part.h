#pragma once

#include "Common.h"
#include "StorageHandler.h"

using namespace std;
using namespace cv;

class Part
{
public:
	Part(void);
	~Part(void);

	// initialize part with joint and matrix information
	void Initialize(void);

	void Initialize(
		const vector<int>& trackingStates,
		const vector<Vec3f>& skeletonJoints,
		const vector<Mat>& absoluteMatrices,
		const int partIndex);

	void Initialize(
		const vector<int>& partStates,
		const vector<float>& skeletonJoints,
		const vector<Mat>& absoluteMatrices,
		const int partIndex);

	// variables
	int m_trackingState;
	int m_partIndex;
	float m_length;
	Vec3f m_startJoint;
	Vec3f m_endJoint;
	Mat m_rotation;

	// flags
	bool IsValid(void) const {return m_trackingState == NUI_SKELETON_POSITION_TRACKED;}

	// getters
	int GetTrackingState(void) const {return m_trackingState;}
	int GetPartIndex(void) const {return m_partIndex;}
	const Vec3f& GetStartJoint(void) const {return m_startJoint;}
	const Vec3f& GetEndJoint(void) const {return m_endJoint;}
	const Mat& GetRotation(void) const {return m_rotation;}
	Mat GetTransform(const Part& ref) const;


protected:

	// parent-child information
	static const int parent_indices[20];
};