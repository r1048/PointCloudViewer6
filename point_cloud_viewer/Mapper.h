#pragma once
#include <Windows.h>
#include <NuiApi.h>

#include "Resolution.h"
#include "Part.h"

using namespace std;

class Mapper : public Resolution
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

	Mat transformSkeletonToPoint(const Mat& skeletonFrame) const;
	vector<Vec3f> transformSkeletonJointToPointJoint(const vector<Vec3f>& skeletonJointList) const;
	Part transformSkeletonPartToPointPart(const Part& part) const;

protected:
	static const string TAG_MAPPER_COUNT;
	static const string TAG_MAPPER_DATA;

	Vec3f transformSkeletonPointTo3DPoint(const Vec3f& skeletonPoint) const;
};