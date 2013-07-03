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
		const vector<bool>& trackingStates,
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

	// flags
	bool IsValid(void) const {return m_valid;}

	// getters
	int GetIndex(void) const {return m_index;}
	const Vec3f& GetStartJoint(void) const {return m_startJoint;}
	const Vec3f& GetEndJoint(void) const {return m_endJoint;}
	const Mat& GetRotation(void) const {return m_rotation;}
	Mat GetTransform(const Part& ref) const;

	// IO
	bool Save(FileStorage& fs) const;
	bool Load(FileStorage& fs);

	// overloadings
	bool operator==(const Part& ref);

protected:

	// parent-child information
	static const int parent_indices[20];

	// tags
	static const string TAG_VALID;
	static const string TAG_LENGTH;
	static const string TAG_INDEX;
	static const string TAG_STARTJOINT;
	static const string TAG_ENDJOINT;
	static const string TAG_ROTATION;
};