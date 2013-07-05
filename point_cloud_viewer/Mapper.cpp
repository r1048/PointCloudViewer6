#include "Mapper.h"

const string Mapper::TAG_MAPPER_COUNT = "MAPPER_COUNT";
const string Mapper::TAG_MAPPER_DATA = "MAPPER_DATA";

Mapper::Mapper(void)
{
	Initialize();
}

Mapper::Mapper(const Mapper& mapper)
{
	count = 0;
	data = NULL;
	pMapper = NULL;

	if(mapper.count == 0) return ;
	if(mapper.data == NULL || mapper.pMapper == NULL) return ;

	this->count = mapper.count;
	this->data = new BYTE[count];
	memcpy_s(data, sizeof(BYTE) * count, mapper.data, sizeof(BYTE) * count);
	NuiCreateCoordinateMapperFromParameters(count, (void*)data, &pMapper);
}

Mapper::~Mapper(void)
{
	if(data != NULL)
		delete [] data;
	if(pMapper != NULL)
		pMapper->Release();
}

void Mapper::Initialize()
{
	count = 0;
	data = NULL;
	pMapper = NULL;
}

bool Mapper::Initialize(INuiSensor*& pNuiSensor)
{
	if(pNuiSensor == NULL) return false;

	HRESULT hr = pNuiSensor->NuiGetCoordinateMapper(&pMapper);
	if(FAILED(hr)) return false;

	pMapper->GetColorToDepthRelationalParameters(&count, (void**)&data);
	return true;
}

void Mapper::operator=(const Mapper& mapper)
{
	if(data != NULL) delete [] data;
	if(pMapper != NULL) pMapper->Release();
	if(count != 0) count = 0;

	if(mapper.count == 0) return ;
	if(mapper.data == NULL || mapper.pMapper == NULL) return ;

	this->count = mapper.count;
	this->data = new BYTE[count];
	memcpy_s(data, sizeof(BYTE) * count, mapper.data, sizeof(BYTE) * count);
	NuiCreateCoordinateMapperFromParameters(count, (void*)data, &pMapper);
}

bool Mapper::Save(FileStorage& fs) const
{
	if(!fs.isOpened()) return false;
	try
	{
		fs << TAG_MAPPER_COUNT << static_cast<int>(count);
		fs << TAG_MAPPER_DATA << "[:";
		for(int ii = 0; ii < count; ii++)
		{
			fs << static_cast<unsigned char>(data[ii]);
		}
		fs << "]";
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Mapper::Load(FileStorage& fs)
{
	if(data != NULL) delete [] data;
	if(pMapper != NULL) pMapper->Release();
	Initialize();

	if(!fs.isOpened()) return false;
	try
	{
		int _count;
		fs[TAG_MAPPER_COUNT] >> _count;
		count = static_cast<ULONG>(_count);

		vector<unsigned char> _data;
		fs[TAG_MAPPER_DATA] >> _data;
		if(_count != _data.size()) throw exception("invalid data size");
		data = new BYTE[_count];
		for(int ii = 0; ii < _data.size(); ii++)
			data[ii] = static_cast<BYTE>(_data[ii]);

		NuiCreateCoordinateMapperFromParameters(count, (void*)data, &pMapper);
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;	
}

Vec3f Mapper::transformSkeletonPointTo3DPoint(const Vec3f& skeletonPoint) const
{
	Vec3f transformedPoint(0, 0, 0);

	if(skeletonPoint == Vec3f(0, 0, 0)) return transformedPoint;
	Vector4 skeletonPoint4;
	skeletonPoint4.x = skeletonPoint[0];
	skeletonPoint4.y = skeletonPoint[1];
	skeletonPoint4.z = skeletonPoint[2];
	NUI_COLOR_IMAGE_POINT colorPoint;
	NUI_DEPTH_IMAGE_POINT depthPoint;
	pMapper->MapSkeletonPointToColorPoint(
		&skeletonPoint4,
		NUI_IMAGE_TYPE_COLOR,
		m_depthResolution,
		&colorPoint);
	pMapper->MapSkeletonPointToDepthPoint(
		&skeletonPoint4,
		m_depthResolution,
		&depthPoint);
	if(depthPoint.depth == 0) return transformedPoint;

	transformedPoint[0] = colorPoint.x;
	transformedPoint[1] = colorPoint.y;
	transformedPoint[2] = static_cast<float>(depthPoint.depth);
	transformedPoint[0] = -(transformedPoint[0] - CX) * transformedPoint[2] / FX;
	transformedPoint[1] = -(transformedPoint[1] - CY) * transformedPoint[2] / FY;

	return transformedPoint;
}

Mat Mapper::transformSkeletonToPoint(const Mat& skeletonFrame) const
{
	Mat pointFrame;
	if(skeletonFrame.empty()) return pointFrame;
	if(!IsValid()) return pointFrame;
	InitPointMatrix(pointFrame);

	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++)
		{
			const Vec3f& skeletonPoint = skeletonFrame.at<Vec3f>(rr, cc);
			const Vec3f transformedPoint = transformSkeletonPointTo3DPoint(skeletonPoint);
			pointFrame.at<Vec3f>(rr, cc) = transformedPoint;
		}
	}

	return pointFrame;
}

vector<Vec3f> Mapper::transformSkeletonJointToPointJoint(const vector<Vec3f>& skeletonJointList) const
{
	vector<Vec3f> pointJointList;
	if(!IsValid()) return pointJointList;

	for(int ii = 0; ii < skeletonJointList.size(); ii++)
	{
		const Vec3f& skeletonJoint = skeletonJointList[ii];
		const Vec3f pointJoint = transformSkeletonPointTo3DPoint(skeletonJoint);
		pointJointList.push_back(pointJoint);
	}

	return pointJointList;
}

Part Mapper::transformSkeletonPartToPointPart(const Part& part) const
{
	Part transformedPart;
	if(!IsValid()) return transformedPart;

	transformedPart.m_trackingState = part.m_trackingState;
	transformedPart.m_partIndex = part.m_partIndex;
	transformedPart.m_rotation = part.m_rotation;

	const Vec3f& startJoint = part.GetStartJoint();
	const Vec3f& endJoint = part.GetEndJoint();
	const Vec3f newStartJoint = transformSkeletonPointTo3DPoint(startJoint);
	const Vec3f newEndJoint = transformSkeletonPointTo3DPoint(endJoint);
	const Vec3f delta = newEndJoint - newStartJoint;
	const float length = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);

	transformedPart.m_length = length;
	transformedPart.m_startJoint = newStartJoint;
	transformedPart.m_endJoint = newEndJoint;

	return transformedPart;
}