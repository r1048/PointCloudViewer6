#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/flann/flann.hpp>
#include <Windows.h>
#include <NuiApi.h>
#include <JunhaLibrary.h>

#include "Common.h"
#include "StorageHandler.h"
#include "Storage.h"
#include "Skeleton.h"
#include "Segmentation.h"
#include "NormalVector.h"
#include "GraphCut\GraphCutter.h"
#include "Transformation.h"
#include "Mapper.h"

using namespace std;
using namespace cv;
using namespace cvflann;
using namespace JunhaLibrary;

class Player
{
public:
	Player() {Initialize();}
	~Player() {}

protected:
	int m_index;
	Storage m_storage;
	Skeleton m_skeleton;
	Mat m_labelMatrix;
	Mat m_normalMatrix;
	//Segmentation m_segment;
	
public:
	void Initialize();
	bool Initialize(
		const Storage& storage,
		const Mat& indexFrame,
		const int index,
		const NUI_SKELETON_DATA& skeletonData);
	
	// functions for status retrieval
	bool IsValid(void) const;
	bool IsNormalComputed(void) const;
	bool IsLabeled(void) const;

	// functions for post-processing
	bool Label(const int method = 0);
	bool Transform(const Player& refPlayer, Mapper& mapper);

	int GetIndex(void) const {return m_index;}
	const Storage& GetStorage(void) const {return m_storage;}
	Skeleton GetSkeleton(void) const {return m_skeleton;}
	const Mat& GetLabel(void) const {return m_labelMatrix;}
	const Mat& GetNormal(void) const {return m_normalMatrix;}

	void UpdateColorFrame(const Mat& colorFrame);
	void UpdatePointFrame(const Mat& pointFrame);

	bool Load(const string& path);
	bool Save(const string& path) const;

protected:
	static const string TAG_PLAYER_INDEX;
	static const string TAG_LABEL_MATRIX;
	static const string TAG_NORMAL_MATRIX;

	bool Segment(void);
	bool Normal(void);
	bool GraphCut(void);
};