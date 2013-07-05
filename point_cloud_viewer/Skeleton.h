#pragma once

#include "Common.h"
#include "Resolution.h"
#include "Part.h"

class Skeleton : public Resolution
{
public:
	Skeleton(void) {Initialize();}
	~Skeleton(void) {}

	// member variables
	int m_trackingState; // tracking state: 2 if tracked
	vector<Vec3f> m_jointList;	// joint position, N_JOINT, in skeleton space
	vector<Part> m_partList;	// part list, N_PART, in skeleton space

	void Initialize(void);
	void Initialize(
		const int index,
		const NUI_SKELETON_DATA& skeletonData);
	bool IsValid(void) const;

	// Getters
	const int GetTrackingState(void) const {return m_trackingState;}
	const Part& GetPart(int index) const;
	const Vec3f& GetJoint(int index) const;
	const vector<Vec3f>& GetJointList(void) const;

	// IO Functions
	bool Save(FileStorage& fs) const;
	bool Load(FileStorage& fs);

protected:
	static const int parent_indices[20];

	static const string TAG_SKELETON_STATE;
	static const string TAG_SKELETON_JOINTS;
	static const string TAG_PART_STATES;
	static const string TAG_PART_ROTATIONS;

	void SetJoints(const NUI_SKELETON_DATA& data);
	void SetParts(const NUI_SKELETON_DATA& data);

	Mat ConvertMatrix4ToMat3x3(const Matrix4& rotation);
	Vec3f ConvertSkeletonToPoint(const Vector4& point4, INuiCoordinateMapper*& pMapper);
};