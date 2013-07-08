#pragma once

#include "Common.h"
#include "StorageHandler.h"
#include "Storage.h"
#include "Player.h"
#include "Mapper.h"

class Frame
{
public:
	Frame() {Initialize();}
	~Frame() {}

	Mapper m_mapper;
	Mat m_indexFrame;
	vector<Player> m_players;
	Storage m_storage;
	string m_timestamp;

	const vector<Player>& GetPlayers(void) const {return m_players;}
	const Storage& GetStorage(void) const {return m_storage;}

	void Initialize(void);
	bool IsValid(void) const;

	bool Load(const string& path, const string& timestamp, const vector<string>& filelist);
	bool Save(const string& path);

	bool UpdateMapper(INuiSensor*& pNuiSensor);
	void UpdateColor(const NUI_LOCKED_RECT& lockedRect);
	void UpdateDepth(const NUI_LOCKED_RECT& lockedRect);
	void UpdateSkeletonAndCoordinate(const bool isSmoothing);
	
	void UpdatePlayers(const NUI_SKELETON_FRAME& skeletonFrame);
	void SegmentPlayers();
	void NormalPlayers();
	void GraphCutPlayers();
	void TransformPlayers(const Frame& refFrame);

public:
	static const string SUFFIX_COLOR;
	static const string SUFFIX_STORAGE;
	static const string SUFFIX_PLAYER;

protected:
	static const string TAG_INDEX_FRAME;

	void Smoothing();
};