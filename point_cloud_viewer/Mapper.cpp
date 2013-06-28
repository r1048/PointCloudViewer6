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

Mat Mapper::transformSkeletonToPoint(const Mat& skeletonFrame)
{
	Mat pointFrame;
	if(skeletonFrame.empty()) return pointFrame;
	if(!IsValid()) return pointFrame;
	InitPointMatrix(pointFrame);

	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++)
		{
			const Vec3f& skeleton = skeletonFrame.at<Vec3f>(rr, cc);
			if(skeleton == Vec3f(0, 0, 0)) continue;

			Vector4 skeletonPoint;
			skeletonPoint.x = skeleton[0];
			skeletonPoint.y = skeleton[1];
			skeletonPoint.z = skeleton[2];
			NUI_COLOR_IMAGE_POINT colorPoint;
			NUI_DEPTH_IMAGE_POINT depthPoint;
			pMapper->MapSkeletonPointToColorPoint(
				&skeletonPoint,
				NUI_IMAGE_TYPE_COLOR,
				m_depthResolution,
				&colorPoint);
			pMapper->MapSkeletonPointToDepthPoint(
				&skeletonPoint,
				m_depthResolution,
				&depthPoint);
			if(depthPoint.depth == 0) continue;

			Vec3f point;
			point[0] = colorPoint.x;
			point[1] = colorPoint.y;
			point[2] = static_cast<float>(depthPoint.depth);
			point[0] = -(point[0] - CX) * point[2] / FX;
			point[1] = -(point[1] - CY) * point[2] / FY;
			pointFrame.at<Vec3f>(rr, cc) = point;
		}
	}

	return pointFrame;
}