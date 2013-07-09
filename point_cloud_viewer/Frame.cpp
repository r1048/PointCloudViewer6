#include "Frame.h"

const string Frame::SUFFIX_COLOR = "-color.png";
const string Frame::SUFFIX_STORAGE = "-storage.yml";
const string Frame::SUFFIX_PLAYER = "-player";

const string Frame::TAG_INDEX_FRAME = "INDEX_FRAME";

bool Frame::Load(const string& path, const string& timestamp, const vector<string>& filelist)
{
	// get timestamp and generate default path
	this->m_timestamp = timestamp;
	const string strPath = Directorize(path).append(m_timestamp);

	// Load color and storage
	bool isLoaded = true;
	const string strColorPath = strPath + SUFFIX_COLOR;
	const string strStoragePath = strPath + SUFFIX_STORAGE;
	if(isLoaded) isLoaded &= m_storage.LoadColor(strColorPath);
	if(isLoaded) {
		FileStorage fs;
		isLoaded &= fs.open(strStoragePath, FileStorage::READ);
		isLoaded &= m_storage.LoadStorage(fs);
		isLoaded &= m_mapper.Load(fs);
		isLoaded &= StorageHandler::Load(fs, m_indexFrame, TAG_INDEX_FRAME);
		if(fs.isOpened()) fs.release();
	}

	// Load players
	m_players.clear();
	for(int ii = 0; ii < filelist.size(); ii++)
	{
		const string filename = filelist[ii];
		size_t found = filename.find(m_timestamp);
		if(found != string::npos)
		{
			const string strPlayerPath = filename;
			cout << strPlayerPath << endl;
			Player player;
			bool isLoaded = player.Load(strPlayerPath);
			if(isLoaded) {
				player.UpdateColorFrame(m_storage.GetColor());
				m_players.push_back(player);
				cout << "player was loaded!" << endl;
			}
		}
	}
	return true;
}
	
bool Frame::Save(const string& path)
{
	// get timestamp and generate default path
	this->m_timestamp = GetTimeStamp();
	const string strPath = Directorize(path).append(m_timestamp);

	// Save color and storage
	bool isSaved = true;
	const string strColorPath = strPath + SUFFIX_COLOR;
	const string strStoragePath = strPath + SUFFIX_STORAGE;
	if(isSaved) isSaved &= m_storage.SaveColor(strColorPath);
	if(isSaved) {
		FileStorage fs;
		isSaved &= fs.open(strStoragePath, FileStorage::WRITE);
		isSaved &= m_storage.SaveStorage(fs);
		isSaved &= m_mapper.Save(fs);
		isSaved &= StorageHandler::Save(fs, m_indexFrame, TAG_INDEX_FRAME);
		if(fs.isOpened()) fs.release();
	}

	// Save players
	char buff[30];
	vector<string> strPlayersPath;
	for(int ii = 0; ii < m_players.size(); ii++)	
	{
		_itoa_s(ii, buff, 10);
		strPlayersPath.push_back(strPath + SUFFIX_PLAYER + buff + ".yml");
		m_players[ii].Save(strPlayersPath[ii]);
	}
	return true;
}


void Frame::Initialize(void)
{
	m_players.clear();
	m_storage = Storage();
	m_mapper = Mapper();
	m_timestamp = "";
}

bool Frame::IsValid(void) const
{
	return m_storage.IsValid();
}

void Frame::UpdateColor(const NUI_LOCKED_RECT& lockedRect)
{
	const unsigned char* colorSrc = reinterpret_cast<const unsigned char*>(lockedRect.pBits);
	m_storage.UpdateColor(colorSrc);
}

void Frame::UpdateDepth(const NUI_LOCKED_RECT& lockedRect)
{
	const NUI_DEPTH_IMAGE_PIXEL* depthSrc = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL*>(lockedRect.pBits);
	m_storage.UpdateDepth(depthSrc, m_indexFrame);
}

bool Frame::UpdateMapper(INuiSensor*& pNuiSensor)
{
	return m_mapper.Initialize(pNuiSensor);
}

void Frame::UpdateSkeletonAndCoordinate(const bool isSmoothing)
{
	if(m_mapper.IsValid())
	{
		if(isSmoothing) Smoothing();
		m_storage.UpdateSkeletonAndCoordinate(m_mapper);
	}
}

void Frame::Smoothing()
{
	if(!IsValid()) return;

	const int fSize = 5;
	const float fSigData = 40.0f;
	const float fSigSpace = 10.0f;
	
	// compute filtered depth
	Mat uDepthFrame = m_storage.GetDepth();
	Mat fDepthFrame = InitMatrix(CV_32FC1);
	Mat zeroFrame = uDepthFrame == 0;
	uDepthFrame.convertTo(uDepthFrame, CV_32FC1);
	bilateralFilter(uDepthFrame, fDepthFrame, fSize, fSigData, fSigSpace);

	// update depth
	fDepthFrame.setTo(Scalar(0), zeroFrame);
	fDepthFrame.convertTo(uDepthFrame, CV_16UC1);

	// update filtered depth
	m_storage.UpdateDepth(uDepthFrame);
}

void Frame::UpdatePlayers(const NUI_SKELETON_FRAME& skeletonFrame)
{
	m_players.clear();
	for(int ii = 0; ii < NUI_SKELETON_COUNT; ii++)
	{
		const int index = ii + 1;
		Player player;
		bool initialized = player.Initialize(
			m_storage,
			m_indexFrame,
			index,
			skeletonFrame.SkeletonData[ii]);
		if(initialized) m_players.push_back(player);
	}
}

bool Frame::LabelPlayers(const int method)
{
	bool isLabeled = true;
	for(int ii = 0; ii < m_players.size(); ii++)
	{
		Player& player = m_players[ii];
		isLabeled &= player.Label(method);
	}
	return isLabeled;
}

void Frame::TransformPlayers(const Frame& refFrame)
{
	const vector<Player>& refPlayers = refFrame.GetPlayers();
	if(refPlayers.empty()) return;

	for(int ii = 0; ii < refPlayers.size(); ii++)
	{
		const Player& refPlayer = refPlayers[ii];
		const int refIndex = refPlayer.GetIndex();
		for(int jj = 0; jj < m_players.size(); jj++)
		{
			Player& newPlayer = m_players[jj];
			const int newIndex = newPlayer.GetIndex();
			if(refIndex == newIndex)
				newPlayer.Transform(refPlayer, m_mapper);
		}
	}
}