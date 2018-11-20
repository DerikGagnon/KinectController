/*
	This file was modified from the Microsoft Kinect 1.8 SDK Developer Toolkit
	SkeletonBasics - C++ program.
*/

#include "stdafx.h"
#include <strsafe.h>
#include "KinectControls.h"
#include <iostream>


/// <summary>
/// Constructor
/// </summary>
KinectController::KinectController() :
    m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
    m_pSkeletonStreamHandle(INVALID_HANDLE_VALUE),
    m_pNuiSensor(NULL)
{
}

/// <summary>
/// Destructor
/// </summary>
KinectController::~KinectController()
{
	std::cout << "DESTRUCTOR\n";
    if (m_pNuiSensor)
    {
        m_pNuiSensor->NuiShutdown();
    }

    if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
    {
        CloseHandle(m_hNextSkeletonEvent);
    }

    SafeRelease(m_pNuiSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
void KinectController::Run()
{
	std::cout << "RUN\nPolling For Motions...\n";
	CreateFirstConnected();

    const int eventCount = 1;
    HANDLE hEvents[eventCount];

    // Main message loop
    while (true)
    {
        hEvents[0] = m_hNextSkeletonEvent;

        // Check to see if we have either a message (by passing in QS_ALLEVENTS)
        // Or a Kinect event (hEvents)
        // Update() will check for Kinect events individually, in case more than one are signalled
        MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);

        // Explicitly check the Kinect frame event since MsgWaitForMultipleObjects
        // can return for other reasons even though it is signaled.
        Update();

    }
}

/// <summary>
/// Main processing function
/// </summary>
void KinectController::Update()
{
	//std::cout << "UPDATE\n";
    if (NULL == m_pNuiSensor)
    {
        return;
    }

    // Wait for 0ms, just quickly test if it is time to process a skeleton
    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0) )
    {
        ProcessSkeleton();
    }
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT KinectController::CreateFirstConnected()
{
    INuiSensor * pNuiSensor;

    int iSensorCount = 0;
    HRESULT hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

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
        // Initialize the Kinect and specify that we'll be using skeleton and depth
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_DEPTH); 
        if (SUCCEEDED(hr))
        {
            // Create an event that will be signaled when skeleton data is available
            m_hNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

            // Open a skeleton stream to receive skeleton data
            hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0); 
        }
    }

    if (NULL == m_pNuiSensor || FAILED(hr))
    {
        //SetStatusMessage(L"No ready Kinect found!");
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Handle new skeleton data
/// </summary>
void KinectController::ProcessSkeleton()
{
    NUI_SKELETON_FRAME skeletonFrame = {0};

    HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
    if ( FAILED(hr) )
    {
        return;
    }

    // smooth out the skeleton data
    m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

    for (int i = 0 ; i < NUI_SKELETON_COUNT; ++i)
    {
        NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;

        if (NUI_SKELETON_TRACKED == trackingState)
        {
            // We're tracking the skeleton

			/*std::cout << "Spine value: ";
			std::cout << skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z << "\n";
			std::cout << "Right Foot Value: ";
			std::cout << skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].z << "\n";*/

			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x - skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].x > .5) {
				std::cout << "RIGHT ARM EXTENDED\n";
				INPUT ip;
				ip.type = INPUT_KEYBOARD;
				ip.ki.wScan = 0;
				ip.ki.time = 0;
				ip.ki.dwExtraInfo = 0;
				ip.ki.wVk = 0x27; // virtual-key code for right arrow
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				// Release the "right arrow" key
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
			}

			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x - skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].x < -.5) {
				std::cout << "LEFT ARM EXTENDED\n";
				INPUT ip;
				ip.type = INPUT_KEYBOARD;
				ip.ki.wScan = 0;
				ip.ki.time = 0;
				ip.ki.dwExtraInfo = 0;
				ip.ki.wVk = 0x25; // virtual-key code for left arrow
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				// Release the "left arrow" key
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
			}

			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z - skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.5) {
				std::cout << "RIGHT ARM FORWARD\n";
				INPUT ip;
				ip.type = INPUT_KEYBOARD;
				ip.ki.wScan = 0;
				ip.ki.time = 0;
				ip.ki.dwExtraInfo = 0;
				ip.ki.wVk = 0x31; // virtual-key code for 1
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				// Release the "1" key
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
			}

			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z - skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.5) {
				std::cout << "LEFT ARM FORWARD\n";
				INPUT ip;
				ip.type = INPUT_KEYBOARD;
				ip.ki.wScan = 0;
				ip.ki.time = 0;
				ip.ki.dwExtraInfo = 0;
				ip.ki.wVk = 0x32; // virtual-key code for 2
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				// Release the "2" key
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
			}

			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_LEFT].z - skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.7) {
				std::cout << "LEFT FOOT FORWARD\n";
				INPUT ip;
				ip.type = INPUT_KEYBOARD;
				ip.ki.wScan = 0;
				ip.ki.time = 0;
				ip.ki.dwExtraInfo = 0;
				ip.ki.wVk = 0x57; // virtual-key code for w
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				// Release the "W" key
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(300);
			}

			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].z - skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.7) {
				std::cout << "RIGHT FOOT FORWARD\n";
				INPUT ip;
				ip.type = INPUT_KEYBOARD;
				ip.ki.wScan = 0;
				ip.ki.time = 0;
				ip.ki.dwExtraInfo = 0;
				ip.ki.wVk = 0x51; // virtual-key code for q
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				// Release the "Q" key
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(700);
			}
        }
    }
}

int main() {

	//create control instance
	KinectController application;

	application.Run();

	system("pause");
	return 0;
}