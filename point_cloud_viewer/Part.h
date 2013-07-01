#pragma once

#include "Resolution.h"

using namespace std;
using namespace cv;

class Part : public Resolution
{
public:
	Part(void);
	~Part(void);

	// initialize part with joint and matrix information
	void Initialize(void);

	void Initialize(
		const NUI_SKELETON_DATA& skeletonData,
		const vector<Vec3f>& skeletonJoints,
		const vector<Mat>& absoluteMatrices,
		const int partIndex);

	// variables
	bool m_valid;
	float m_length;
	int m_index;
	Vec3f m_startJoint;
	Vec3f m_endJoint;
	Mat m_rotation;
	Mat m_transformation;

	// flags
	bool IsValid(void) const {return m_valid;}
	bool IsTransformed(void) const {return !m_transformation.empty();}

	// getters
	int GetIndex(void) const {return m_index;}

	// parent-child information
	static const int parent_indices[20];
};