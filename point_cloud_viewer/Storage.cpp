#include "Storage.h"

const string Storage::TAG_COLOR = "COLOR";
const string Storage::TAG_DEPTH = "DEPTH";
const string Storage::TAG_SKELETON = "SKELETON";
const string Storage::TAG_COORDINATE = "COORDINATE";

void Storage::Initialize(void)
{
	m_colorFrame = Mat();
	m_depthFrame = Mat();
	m_skeletonFrame = Mat();
}

bool Storage::IsValid(void) const
{
	return !m_colorFrame.empty() &&
		!m_depthFrame.empty() &&
		!m_skeletonFrame.empty();
}

void Storage::Update(const Storage& storage, const Mat& mask)
{
	if(!storage.IsValid() || mask.empty()) return ;

	//storage.GetColor().copyTo(m_colorFrame, mask);
	storage.GetColor().copyTo(m_colorFrame, mask);
	storage.GetDepth().copyTo(m_depthFrame, mask);
	storage.GetSkeleton().copyTo(m_skeletonFrame, mask);
	storage.GetCoordinate().copyTo(m_coordinateFrame, mask);
}

void Storage::UpdateColor(const unsigned char*& colorSrc)
{
	m_colorFrame = InitMatrix(CV_8UC3);

	Mat color = InitColorMatrix();
	int index = 0;
	for(int rr = 0; rr < COLOR_HEIGHT; rr++)
	{
		for(int cc = 0; cc < COLOR_WIDTH; cc++)
		{
			Vec4b value;
			for(int ii = 0; ii < 4; ii++, index++) value[ii] = colorSrc[index];	
			color.at<Vec3b>(rr, cc) = Vec3b(value[0], value[1], value[2]);
		}
	}

	resize(color, m_colorFrame, DEPTH_SIZE, 0.0, 0.0, INTER_NEAREST);
}

void Storage::UpdateColor(const Mat& colorSrc, const Mat& indexFrame, const int index)
{
	if(index <= 0 || index > NUI_SKELETON_COUNT) return ;
	if(!indexFrame.empty()) colorSrc.copyTo(m_colorFrame, indexFrame == index);
}

void Storage::UpdateDepth(const NUI_DEPTH_IMAGE_PIXEL*& depthSrc, Mat& indexFrame)
{
	m_depthFrame = InitMatrix(CV_16UC1);
	indexFrame = InitMatrix(CV_32SC1);

	int index = 0;
	for(int rr = 0; rr < DEPTH_HEIGHT; rr++)
	{
		for(int cc = 0; cc < DEPTH_WIDTH; cc++, index++)
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

	m_depthFrame = InitMatrix(CV_16UC1);
	depthSrc.copyTo(m_depthFrame, indexFrame == index);
}

void Storage::UpdateDepth(const Mat& depthSrc)
{
	if(depthSrc.rows != DEPTH_HEIGHT || depthSrc.cols != DEPTH_WIDTH ||
		depthSrc.type() != CV_16UC1) return;

	m_depthFrame = depthSrc;
}

void Storage::UpdateSkeletonAndCoordinate(const Mapper& mapper)
{
	if(m_depthFrame.empty() || !mapper.IsValid()) return ;
	mapper.TransformDepthToSkeletonAndCoordinate(
		m_depthFrame,
		m_skeletonFrame,
		m_coordinateFrame);
}

bool Storage::UpdateSkeleton(const Mat& transformed)
{
	if(transformed.empty()) return false;
	m_skeletonFrame = transformed;
	return true;
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