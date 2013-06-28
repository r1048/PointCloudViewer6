#include "Storage.h"

const string Storage::TAG_COLOR = "COLOR";
const string Storage::TAG_DEPTH = "DEPTH";
const string Storage::TAG_POINT = "POINT";
const string Storage::TAG_SKELETON = "SKELETON";
const string Storage::TAG_COORDINATE = "COORDINATE";

void Storage::Initialize(void)
{
	m_colorFrame = Mat();
	m_depthFrame = Mat();
	m_pointFrame = Mat();
	m_skeletonFrame = Mat();
}

bool Storage::IsValid(void) const
{
	return !m_colorFrame.empty() && !m_depthFrame.empty() &&
		!m_pointFrame.empty() && !m_skeletonFrame.empty();
}

void Storage::UpdateColor(const unsigned char*& colorSrc)
{
	InitColorMatrix(m_colorFrame);

	int index = 0;
	for(int rr = 0; rr < colorHeight; rr++)
	{
		for(int cc = 0; cc < colorWidth; cc++)
		{
			Vec4b value;
			for(int ii = 0; ii < 4; ii++, index++)
				value[ii] = colorSrc[index];	
			m_colorFrame.at<Vec3b>(rr, cc) = Vec3b(value[0], value[1], value[2]);
		}
	}

	//resize(m_colorFrame, m_colorFrame, Size(depthWidth, depthHeight), 0.0, 0.0, INTER_NEAREST);
}

void Storage::UpdateColor(const Mat& colorSrc, const Mat& indexFrame, const int index)
{
	if(index <= 0 || index > NUI_SKELETON_COUNT) return ;

	Mat resizedColorSrc;
	m_colorFrame = Mat::zeros(depthHeight, depthWidth, CV_8UC3);
	resize(colorSrc, resizedColorSrc, Size(depthWidth, depthHeight));
	if(!indexFrame.empty()) resizedColorSrc.copyTo(m_colorFrame, indexFrame == index);
	else resizedColorSrc.copyTo(m_colorFrame);
}

void Storage::UpdateDepth(const NUI_DEPTH_IMAGE_PIXEL*& depthSrc, Mat& indexFrame)
{
	InitDepthMatrix(m_depthFrame);
	InitIndexMatrix(indexFrame);

	int index = 0;
	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++, index++)
		{
			const NUI_DEPTH_IMAGE_PIXEL depthPixel = depthSrc[index];
			m_depthFrame.at<unsigned short>(rr, cc) = depthPixel.depth;
			indexFrame.at<int>(rr, cc) = static_cast<int>(depthPixel.playerIndex);
		}
	}
}

void Storage::UpdateDepth(const Mat& depthSrc, const Mat& indexFrame, const int index)
{
	if(index <= 0 || index > NUI_SKELETON_COUNT) return ;
	if(depthSrc.size() != indexFrame.size()) return ;
	if(depthSrc.type() != CV_16UC1) return ;
	if(indexFrame.type() != CV_32SC1) return ;

	InitDepthMatrix(m_depthFrame);
	depthSrc.copyTo(m_depthFrame, indexFrame == index);
}

void Storage::UpdatePoint(INuiCoordinateMapper*& pMapping)
{
	if(m_depthFrame.empty()) return ;
	if(pMapping == NULL) return ;

	NUI_DEPTH_IMAGE_PIXEL* depthPixels = new NUI_DEPTH_IMAGE_PIXEL[depthCount];
	NUI_COLOR_IMAGE_POINT* colorPoints = new NUI_COLOR_IMAGE_POINT[depthCount];
	int index = 0;
	for(int rr = 0; rr < depthHeight; rr++) {
		for(int cc = 0; cc < depthWidth; cc++, index++) {
			depthPixels[index].depth = m_depthFrame.at<unsigned short>(rr, cc);
			depthPixels[index].playerIndex = 0;
		}
	}
	
	pMapping->MapDepthFrameToColorFrame(
		m_depthResolution,
		depthCount,
		depthPixels,
		NUI_IMAGE_TYPE_COLOR,
		m_depthResolution,
		depthCount,
		colorPoints);

	InitPointMatrix(m_pointFrame);
	index = 0;
	for(int rr = 0; rr < depthHeight; rr++) {
		for(int cc = 0; cc < depthWidth; cc++, index++) {
			unsigned short depth = m_depthFrame.at<unsigned short>(rr, cc);
			if(depth == 0) continue;
			Vec3f point(colorPoints[index].x, colorPoints[index].y, depth);
			point[0] = -(point[0] - CX) * point[2] / FX;
			point[1] = -(point[1] - CY) * point[2] / FY;
			m_pointFrame.at<Vec3f>(rr, cc) = point;
		}
	}

	delete [] depthPixels;
	delete [] colorPoints;

	UpdateCoordinate();
}

void Storage::UpdateSkeleton(INuiCoordinateMapper*& pMapping)
{
	if(m_depthFrame.empty()) return ;
	if(pMapping == NULL) return ;

	NUI_DEPTH_IMAGE_PIXEL* depthPixels = new NUI_DEPTH_IMAGE_PIXEL[depthCount];
	Vector4* skeletonPoints = new Vector4[depthCount];
	int index = 0;
	for(int rr = 0; rr < depthHeight; rr++) {
		for(int cc = 0; cc < depthWidth; cc++, index++) {
			depthPixels[index].depth = m_depthFrame.at<unsigned short>(rr, cc);
			depthPixels[index].playerIndex = 0;
		}
	}

	pMapping->MapDepthFrameToSkeletonFrame(
		m_depthResolution,
		depthCount,
		depthPixels,
		depthCount,
		skeletonPoints);

	InitPointMatrix(m_skeletonFrame);
	index = 0;
	for(int rr = 0; rr < depthHeight; rr++) {
		for(int cc = 0; cc < depthWidth; cc++, index++) {
			unsigned short depth = m_depthFrame.at<unsigned short>(rr, cc);
			if(depth == 0) continue;
			Vector4 point = skeletonPoints[index];
			m_skeletonFrame.at<Vec3f>(rr, cc) = Vec3f(point.x, point.y, point.z);
		}
	}

	delete [] depthPixels;
	delete [] skeletonPoints;
}

void Storage::Smoothing(INuiCoordinateMapper*& pMapping)
{
	if(!IsValid()) return;

	//const int fSize = 9;
	//const float fSigData = 100.0f;
	//const float fSigSpace = 25.0f;
	const int fSize = 5;
	const float fSigData = 40.0f;
	const float fSigSpace = 10.0f;
	
	// compute filtered depth
	Mat fDepthFrame = Mat::zeros(depthHeight, depthWidth, CV_32FC1);
	Mat zeroFrame = m_depthFrame == 0;
	m_depthFrame.convertTo(m_depthFrame, CV_32FC1);
	bilateralFilter(m_depthFrame, fDepthFrame, fSize, fSigData, fSigSpace);

	// update depth
	fDepthFrame.setTo(Scalar(0), zeroFrame);
	fDepthFrame.convertTo(m_depthFrame, CV_16UC1);

	// update skeleton
	UpdatePoint(pMapping);
	UpdateSkeleton(pMapping);
}

void Storage::UpdatePointFrame(const Mat& pointFrame)
{
	if(!pointFrame.empty() && pointFrame.rows == depthHeight &&
		pointFrame.cols == depthWidth && pointFrame.type() == CV_32FC3)
	{
		this->m_pointFrame = pointFrame;
		UpdateCoordinate();
	}
}

void Storage::UpdateTransformed(const Mat& transformed)
{
	if(!transformed.empty() && transformed.rows == depthHeight &&
		transformed.cols == depthWidth && transformed.type() == CV_32FC3)
	{
		this->m_pointFrame = transformed;
	}
}

void Storage::UpdateCoordinate()
{
	if(m_pointFrame.empty()) return;

	m_coordinateFrame = Mat::zeros(depthHeight, depthWidth, CV_32FC2);
	for(int rr = 0; rr < depthHeight; rr++)
	{
		for(int cc = 0; cc < depthWidth; cc++)
		{
			const Vec3f& point3 = m_pointFrame.at<Vec3f>(rr, cc);
			if(point3 == Vec3f(0, 0, 0)) continue;
			else if(point3[2] < 1e-10) continue;
			else {
				Vec2f point2;
				point2[0] = point3[0] * -Resolution::FX / point3[2] + Resolution::CX;
				point2[1] = point3[1] * -Resolution::FY / point3[2] + Resolution::CY;
				m_coordinateFrame.at<Vec2f>(rr, cc) = point2;
			}
		}
	}
}




bool Storage::Save(const string& colorPath, const string& dataPath) const
{
	// Save color and data
	bool isColorSaved = SaveColor(colorPath);
	bool isStorageSaved = SaveStorage(dataPath);
	
	// Print the status
	if(isColorSaved) cout << "succeed to save color image: " << colorPath << endl;
	else cerr << "failed to save color image: " << colorPath << endl;
	if(isStorageSaved) cout << "succeed to save storage: " << dataPath << endl;
	else cerr << "failed to save storage: " << dataPath << endl;
	return isColorSaved && isStorageSaved;
}

// Save color matrix with png format
bool Storage::SaveColor(const string& colorPath) const
{
	return imwrite(colorPath, GetColor());
}

// Save data matrices with yml format
bool Storage::SaveStorage(const string& storagePath) const
{
	FileStorage fs;
	bool isStorageOpened = fs.open(storagePath, FileStorage::WRITE);
	bool isStorageSaved = SaveStorage(fs);
	if(isStorageOpened) fs.release();
	return isStorageOpened && isStorageSaved;
}

bool Storage::SaveStorage(FileStorage& fs) const
{
	if(!fs.isOpened()) return false;
	try
	{
		fs << TAG_COLOR << GetColor();
		fs << TAG_DEPTH << GetDepth();
		fs << TAG_POINT << GetPoint();
		fs << TAG_SKELETON << GetSkeleton();
		fs << TAG_COORDINATE << GetCoordinate();
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Storage::Load(const string& colorPath, const string& storagePath)
{
	// Load color and data
	bool isColorLoaded = LoadColor(colorPath);
	bool isStorageLoaded  = LoadStorage(storagePath);
	
	// Print the status
	if(isColorLoaded) cout << "succeed to load color image: " << colorPath << endl;
	else cerr << "failed to load color image: " << colorPath << endl;
	if(isStorageLoaded) cout << "succeed to load storage: " << storagePath << endl;
	else cerr << "failed to load storage: " << storagePath << endl;
	return isColorLoaded && isStorageLoaded;
}

bool Storage::LoadColor(const string& colorPath)
{
	// Load a color image
	m_colorFrame = imread(colorPath);
	return !m_colorFrame.empty();
}

bool Storage::LoadStorage(FileStorage& fs)
{
	if(!fs.isOpened()) return false;
	try
	{
		fs[TAG_COLOR] >> m_colorFrame;
		fs[TAG_DEPTH] >> m_depthFrame;
		fs[TAG_POINT] >> m_pointFrame;
		fs[TAG_SKELETON] >> m_skeletonFrame;
		fs[TAG_COORDINATE] >> m_coordinateFrame;
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Storage::LoadStorage(const string& storagePath)
{
	// Load a storage
	FileStorage fs;
	bool isStorageOpened = fs.open(storagePath, FileStorage::READ);
	bool isStorageLoaded = LoadStorage(fs);
	if(isStorageOpened) fs.release();
	return isStorageOpened && isStorageLoaded;
}