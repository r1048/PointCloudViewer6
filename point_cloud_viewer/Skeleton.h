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
	// valid flag: if player was tracked not inferred
	bool m_valid;
	// user index: starts from 1 not 0
	int m_index;
	// joint and part list IN SKELETON SPACE
	vector<Vec3f> m_joints;
	vector<Part> m_parts;

	void Initialize(void);
	void Initialize(
		const int index,
		const NUI_SKELETON_DATA& skeletonData);
	bool IsValid(void) const;

	// Getters
	const Part& GetPart(int index) const;
	const Vec3f& GetJoint(int index) const;
	const vector<Vec3f>& GetJointList(void) const;
	const int GetIndex() const {return m_index;}

	// IO Functions
	bool Save(FileStorage& fs) const;
	bool Load(FileStorage& fs);

protected:
	static const int parent_indices[20];

	static const string TAG_PARTS;

	void SetJoints(const NUI_SKELETON_DATA& data);
	void SetParts(const NUI_SKELETON_DATA& data);

	Mat ConvertMatrix4ToMat3x3(const Matrix4& rotation);
	Vec3f ConvertSkeletonToPoint(const Vector4& point4, INuiCoordinateMapper*& pMapper);
};