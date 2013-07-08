#pragma once

#include "Common.h"
#include "Mapper.h"

using namespace std;
using namespace cv;

class Storage
{
public:
	Storage() {Initialize();}
	~Storage() {}

protected:
	Mat m_colorFrame;
	Mat m_depthFrame;
	Mat m_skeletonFrame;
	Mat m_coordinateFrame;

public:
	const Mat& GetColor(void) const {return m_colorFrame;}
	const Mat& GetDepth(void) const {return m_depthFrame;}
	const Mat& GetSkeleton(void) const {return m_skeletonFrame;}
	const Mat& GetCoordinate(void) const {return m_coordinateFrame;}

	void Initialize();
	bool IsValid() const;
	
	bool Save(const string& colorPath, const string& storagePath) const;
	bool Load(const string& colorPath, const string& storagePath);
	bool SaveColor(const string& colorPath) const;
	bool LoadColor(const string& colorPath);
	bool SaveStorage(FileStorage& fs) const;
	bool LoadStorage(FileStorage& fs);

	void Update(const Storage& storage, const Mat& mask);

	void UpdateColor(const unsigned char*& colorSrc);
	void UpdateColor(const Mat& colorSrc, const Mat& indexFrame = Mat(), const int index = 0);
	void UpdateDepth(const NUI_DEPTH_IMAGE_PIXEL*& depthSrc, Mat& indexFrame);
	void UpdateDepth(const Mat& depthSrc, const Mat& indexFrame, const int index);
	void UpdateDepth(const Mat& depthSrc);
	void UpdateSkeleton(const Mat& transformed);
	void UpdateSkeletonAndCoordinate(const Mapper& mapper);

private:
	bool SaveStorage(const string& dataPath) const;
	bool LoadStorage(const string& storagePath);

	static const string TAG_COLOR;
	static const string TAG_DEPTH;
	static const string TAG_SKELETON;
	static const string TAG_COORDINATE;
};