#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/flann/flann.hpp>
#include <Windows.h>
#include <NuiApi.h>
#include <JunhaLibrary.h>

#include "Resolution.h"
#include "StorageHandler.h"

using namespace std;
using namespace cv;

//class Part : public Resolution
//{
//public:
//	Part(const Skeleton& skeleton,
//		const int partIndex,
//		const Mat& pointMatrix,
//		const Mat& labelMatrix);
//	~Part() {}
//
//	bool m_valid;
//	Vec3f m_startJoint;
//	Vec3f m_endJoint;
//	Mat m_rotation;
//	Mat m_skeletonPoints;
//	Mat m_transformed;
//};

class Skeleton : public Resolution
{
public:
	Skeleton(void) {Initialize();}
	~Skeleton(void) {}

	vector<Vec3f> m_skeletonJoints;
	vector<Vec3f> m_pointJoints;
	vector<Mat> m_absoluteMatrices;
	vector<Mat> m_hierarchicalMatrices;
	NUI_SKELETON_DATA m_skeletonData;

	static const int N_JOINT = 20;
	static const int N_PART = 19;

	void Initialize(void);
	void Initialize(
		const int index,
		const NUI_SKELETON_DATA& skeletonData,
		INuiCoordinateMapper*& pMapper);
	bool IsValid(void) const;

	const vector<Vec3f>& GetSkeletonJoints() const {return m_skeletonJoints;}
	const vector<Vec3f>& GetPointJoints() const {return m_pointJoints;}
	const vector<Mat>& GetAbsoluteMatrices() const {return m_absoluteMatrices;}
	const vector<Mat>& GetHierarchicalMatrices() const {return m_hierarchicalMatrices;}
	static int GetParentIndex(const int& index);

	bool Save(FileStorage& fs) const;
	bool Load(FileStorage& fs);

protected:
	static const int parent_indices[20];

	static const string TAG_SKELETON_JOINTS;
	static const string TAG_POINT_JOINTS;
	static const string TAG_ABSOLUTE_MATRICES;
	static const string TAG_HIERARCHICAL_MATRICES;
	static const string TAG_JOINT_INDICES;
	static const string TAG_SKELETON_DATA;

	static const string TAG_POINT_MATRIX;
	static const string TAG_USER_INDEX;
	static const string TAG_TRACKING_ID;
	static const string TAG_QUALITY_FLAGS;
	static const string TAG_POSITION;
	static const string TAG_STATE;
	static const string TAG_SKELETON_POSITION;
	static const string TAG_SKELETON_STATE;
	static const string TAG_SKELETON_POINTS;

	void SetFromData(INuiCoordinateMapper*& pMapper);
	Mat ConvertMatrix4ToMat3x3(const Matrix4& rotation);
	Vec3f ConvertSkeletonToPoint(const Vector4& point4, INuiCoordinateMapper*& pMapper);

	bool SaveSkeletonJoints(FileStorage& fs) const;
	bool SavePointJoints(FileStorage& fs) const;
	bool SaveAbsoluteMatrices(FileStorage& fs) const;
	bool SaveSkeletonData(FileStorage& fs) const;

	bool LoadSkeletonJoints(FileStorage& fs);
	bool LoadPointJoints(FileStorage& fs);
	bool LoadAbsoluteMatrices(FileStorage& fs);
	bool LoadSkeletonData(FileStorage& fs);
};