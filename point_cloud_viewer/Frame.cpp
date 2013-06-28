#include "Frame.h"

const string Frame::SUFFIX_COLOR = "-color.png";
const string Frame::SUFFIX_STORAGE = "-storage.yml";
const string Frame::SUFFIX_PLAYER = "-player";

const string Frame::TAG_INDEX_FRAME = "INEX_FRAME";

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
		isLoaded &= LoadMatrix(fs, TAG_INDEX_FRAME, m_indexFrame);
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
		isSaved &= SaveMatrix(fs, TAG_INDEX_FRAME, m_indexFrame);
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

void Frame::UpdatePoint()
{
	if(m_mapper.IsValid())
		m_storage.UpdatePoint(m_mapper.GetMapper());
}

void Frame::UpdateSkeleton()
{
	if(m_mapper.IsValid())
		m_storage.UpdateSkeleton(m_mapper.GetMapper());
}

void Frame::Smoothing()
{
	m_storage.Smoothing(m_mapper.GetMapper());
	for(int ii = 0; ii < m_players.size(); ii++)
		m_players[ii].Smoothing(m_mapper.GetMapper());
}

void Frame::UpdatePlayers(const NUI_SKELETON_FRAME& skeletonFrame)
{
	m_players.clear();
	for(int ii = 0; ii < NUI_SKELETON_COUNT; ii++)
	{
		const int index = ii + 1;
		Player player;
		bool isInitialized = player.Initialize(
			m_storage,
			m_indexFrame,
			index,
			skeletonFrame.SkeletonData[ii],
			m_mapper.GetMapper());
		if(isInitialized)
			m_players.push_back(player);
	}
}

void Frame::SegmentPlayers()
{
	for(int ii = 0; ii < m_players.size(); ii++)
	{
		Player& player = m_players[ii];
		player.Segment();
	}
}

void Frame::NormalPlayers()
{
	for(int ii = 0; ii < m_players.size(); ii++)
	{
		Player& player = m_players[ii];
		player.Normal();
	}
}

void Frame::GraphCutPlayers()
{
	for(int ii = 0; ii < m_players.size(); ii++)
	{
		Player& player = m_players[ii];
		player.GraphCut();
	}
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