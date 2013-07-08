#pragma once
#include <Windows.h>
#include <NuiApi.h>

#include "Common.h"
#include "Part.h"

using namespace std;

class Mapper
{
public:
	Mapper(void);
	Mapper(const Mapper& mapper);
	~Mapper(void);

protected:
	ULONG count;
	BYTE* data;
	INuiCoordinateMapper* pMapper;

public:
	void Initialize(void);
	bool Initialize(INuiSensor*& pNuiSensor);
	bool IsValid(void) const {return data != NULL && pMapper != NULL;}
	INuiCoordinateMapper*& GetMapper(void) {return pMapper;}
	void operator=(const Mapper& mapper);

	bool Save(FileStorage& fs) const;
	bool Load(FileStorage& fs);

	// transform skeleton and coordinate from depth frame
	void TransformDepthToSkeletonAndCoordinate(
		const Mat& depthFrame, Mat& skeletonFrame, Mat& coordinateFrame) const;

	vector<Vec3f> transformSkeletonJointToPointJoint(const vector<Vec3f>& skeletonJointList) const;
	Part transformSkeletonPartToPointPart(const Part& part) const;

protected:
	static const string TAG_MAPPER_COUNT;
	static const string TAG_MAPPER_DATA;

	Vec2f transformSkeletonPointTo2DColor(const Vec3f& skeletonPoint) const;
	Vec3f transformSkeletonPointTo3DPoint(const Vec3f& skeletonPoint) const;
};

	//Mat transformDepthToSkeleton(const Mat& depthFrame) const;
	//Mat transformDepthToPoint(const Mat& depthFrame) const;
	//Mat transformSkeletonToCoordinate(const Mat& skeletonFrame) const;
	//Mat transformSkeletonToPoint(const Mat& skeletonFrame) const;