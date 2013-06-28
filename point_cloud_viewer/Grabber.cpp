#include "Grabber.h"

using namespace std;
using namespace cv;
using namespace JunhaLibrary;


const DWORD Grabber::m_nuiInitFlags =
	NUI_INITIALIZE_FLAG_USES_COLOR |
	NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
	NUI_INITIALIZE_FLAG_USES_SKELETON;

const NUI_TRANSFORM_SMOOTH_PARAMETERS Grabber::defaultParams =
	{0.5f, 0.5f, 0.5f, 0.05f, 0.04f};

Grabber::Grabber(void)
{
	Initialize();

	HRESULT hr = this->CreateFirstConnected();
	if(FAILED(hr))
	{
		cerr << "failed to initialize device" << endl;
		if(m_pNuiSensor != NULL)
			m_pNuiSensor->Release();
		m_pNuiSensor = NULL;
	}
	else
	{
		// turn off auto exposure and white balance
		INuiColorCameraSettings* settings;
		m_pNuiSensor->NuiGetColorCameraSettings(&settings);
		settings->SetAutoExposure(FALSE);
		settings->SetAutoWhiteBalance(FALSE);
	}
}

Grabber::~Grabber(void)
{
	if (m_pNuiSensor) 
	{
		m_pNuiSensor->NuiShutdown();
		m_pNuiSensor = NULL;
	}
	
	if (m_hNextColorFrameEvent && (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextColorFrameEvent);
		m_hNextColorFrameEvent = NULL;
	}

    if (m_hNextDepthFrameEvent && (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextDepthFrameEvent);
		m_hNextDepthFrameEvent = NULL;
	}
	
	if (m_hNextSkeletonFrameEvent && (m_hNextSkeletonFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonFrameEvent);
		m_hNextSkeletonFrameEvent = NULL;
	}
}

void Grabber::Initialize(void)
{
	frame = Frame();

	m_isNearMode = false;

	m_pNuiSensor = NULL;
	m_hDepthStreamHandle = INVALID_HANDLE_VALUE;
	m_hColorStreamHandle = INVALID_HANDLE_VALUE;
	m_hNextColorFrameEvent = INVALID_HANDLE_VALUE;
	m_hNextDepthFrameEvent = INVALID_HANDLE_VALUE;
	m_hNextSkeletonFrameEvent = INVALID_HANDLE_VALUE;
}

bool Grabber::IsValid(void) const
{
	return frame.IsValid();
}

HRESULT Grabber::CreateFirstConnected()
{
    INuiSensor * pNuiSensor;
    HRESULT hr;

    int iSensorCount = 0;
    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr)) return hr;

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr)) continue;

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_pNuiSensor = pNuiSensor;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }


    if (NULL != m_pNuiSensor)
    {
        // Initialize the Kinect and specify that we'll be using depth
		hr = m_pNuiSensor->NuiInitialize(m_nuiInitFlags);
		if(FAILED(hr)) return hr;

		m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hNextSkeletonFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		
		// Open image stream
		hr = m_pNuiSensor->NuiImageStreamOpen(
            NUI_IMAGE_TYPE_COLOR,
			m_colorResolution,
            0,
            2,
            m_hNextColorFrameEvent,
            &m_hColorStreamHandle);
		if(FAILED(hr)) return hr;

		hr = m_pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
			m_depthResolution,
            0,
            2,
            m_hNextDepthFrameEvent,
            &m_hDepthStreamHandle);
		if(FAILED(hr)) return hr;
		
		hr = m_pNuiSensor->NuiSkeletonTrackingEnable(
			m_hNextSkeletonFrameEvent,
			0);
		if(FAILED(hr)) return hr;
    }

    return hr;
}

HRESULT Grabber::Update(DWORD waitMillis, const bool smoothing_mode)
{
	HRESULT hr;
	hr = UpdateColorFrame(waitMillis);
	if(FAILED(hr)) return hr;

	hr = UpdateStorage(waitMillis, smoothing_mode);
	if(FAILED(hr)) return hr;

	return S_OK;
}

HRESULT Grabber::UpdateColorFrame(DWORD waitMillis)
{
	if(!m_pNuiSensor)	return E_NUI_DEVICE_NOT_READY;

	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(
		m_hColorStreamHandle,
		waitMillis,
		&imageFrame);
	if (FAILED(hr)) return hr;

	// Lock frame texture to allow for copy
    INuiFrameTexture* pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT lockedRect;
    pTexture->LockRect(0, &lockedRect, NULL, 0);

    // Check if image is valid
    if (lockedRect.Pitch != 0)
    {
		frame.UpdateColor(lockedRect);
    }

    // Unlock texture
    pTexture->UnlockRect(0);

    // Release image stream frame
    hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_hColorStreamHandle, &imageFrame);

	return hr;
}

HRESULT Grabber::UpdateStorage(DWORD waitMillis, const bool smoothing_mode)
{
	// Fail if Kinect is not initialized or stream is not enabled
    if (!m_pNuiSensor)		return E_NUI_DEVICE_NOT_READY;

	// Get next image stream frame
    NUI_IMAGE_FRAME imageFrame;
    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(
        m_hDepthStreamHandle,
        waitMillis,
        &imageFrame);
    if (FAILED(hr)) return hr;

    // Get the depth image pixel texture
	INuiFrameTexture* pTexture;
	BOOL nearMode;
    hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
        m_hDepthStreamHandle,
		&imageFrame,
		&nearMode,
		&pTexture);
    if (FAILED(hr)) {
		m_pNuiSensor->NuiImageStreamReleaseFrame(m_hDepthStreamHandle, &imageFrame);
		return hr;
	}

	// Get next skeleton frame
	NUI_SKELETON_FRAME skeletonFrame;
	hr = m_pNuiSensor->NuiSkeletonGetNextFrame(waitMillis, &skeletonFrame);
	if (FAILED(hr)) return hr;
	
	// Smooth skeletons
	hr = m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, &defaultParams);
	

	// Lock frame texture to allow for copy
    NUI_LOCKED_RECT lockedRect;
    pTexture->LockRect(0, &lockedRect, NULL, 0);

    // Check if image is valid
    if (lockedRect.Pitch != 0)
    {
		frame.UpdateMapper(m_pNuiSensor);
		frame.UpdateDepth(lockedRect);
		frame.UpdatePoint();
		frame.UpdateSkeleton();

		if(smoothing_mode)
			frame.Smoothing();

		frame.UpdatePlayers(skeletonFrame);
    }

    // Unlock texture
    pTexture->UnlockRect(0);

    // Release image stream frame
    hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_hDepthStreamHandle, &imageFrame);

    return hr;
}


void Grabber::ToggleNearMode()
{
	m_isNearMode = !m_isNearMode;
	HRESULT hr;
	if(this->m_pNuiSensor != NULL) {
		hr = this->m_pNuiSensor->NuiImageStreamSetImageFrameFlags(
			m_hDepthStreamHandle,
			m_isNearMode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0);
		if(FAILED(hr)) cout << "failed to toggle near mode" << endl;
	}
}

void Grabber::SetNearMode(const bool mode)
{
	if(this->m_isNearMode == mode) return ;
	else this->ToggleNearMode();
}

void Grabber::ElevationAngleDecrease()
{
	LONG angle;
	::NuiCameraElevationGetAngle(&angle);
	angle -= (LONG)1.0;
	if(NUI_CAMERA_ELEVATION_MINIMUM > angle)
		angle = NUI_CAMERA_ELEVATION_MINIMUM;
	::NuiCameraElevationSetAngle(angle);
}

void Grabber::ElevationAngleIncrease()
{
	LONG angle;
	::NuiCameraElevationGetAngle(&angle);
	angle += (LONG)1.0;
	if(NUI_CAMERA_ELEVATION_MAXIMUM < angle)
		angle = NUI_CAMERA_ELEVATION_MAXIMUM;
	::NuiCameraElevationSetAngle(angle);
}
