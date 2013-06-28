#pragma once

#include <ctime>
#include <iomanip>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <NuiApi.h>
#include <JunhaLibrary.h>

#include "Player.h"
#include "Frame.h"

using namespace std;
using namespace cv;
using namespace JunhaLibrary;

class Grabber : public Resolution
{
public:
	Grabber(void);
	~Grabber(void);

private:
	static const DWORD m_nuiInitFlags;
	static const NUI_TRANSFORM_SMOOTH_PARAMETERS defaultParams;

	Frame frame;

	bool m_isNearMode;

	INuiSensor* m_pNuiSensor;
	HANDLE m_hDepthStreamHandle;
	HANDLE m_hColorStreamHandle;
    HANDLE m_hNextDepthFrameEvent;
	HANDLE m_hNextColorFrameEvent;
	HANDLE m_hNextSkeletonFrameEvent;

private:
	HRESULT CreateFirstConnected();
	Vec3f ConvertSkeletonToPoint(
		Vector4 position,
		INuiCoordinateMapper* pMapping,
		NUI_IMAGE_RESOLUTION depthResolution,
		NUI_IMAGE_RESOLUTION colorResolution) const;
	

public:
	void Initialize(void);
	bool IsValid(void) const;

	HRESULT Update(DWORD waitMillis, const bool smoothing_mode);
	HRESULT UpdateColorFrame(DWORD waitMillis);
	HRESULT UpdateStorage(DWORD waitMillis, const bool smoothing_mode);
	
	Frame GetFrame(void) const {return frame;}

	bool Save(const string& path) {return frame.Save(path);}
	//HRESULT UpdateDepthFrame(DWORD waitMillis);
	//HRESULT UpdateAllInOnce(DWORD waitMillis);
	//HRESULT UpdatePointFrame(DWORD waitMillis);
	//HRESULT UpdateSkeletonFrame(DWORD waitMillis);

	void ToggleNearMode();
	void SetNearMode(const bool mode);
	void ElevationAngleDecrease();
	void ElevationAngleIncrease();
};
