/*
	This file was modified from the Microsoft Kinect 1.8 SDK Developer Toolkit
	SkeletonBasics - C++ program.
*/

#include "stdafx.h"
//#include <strsafe.h>
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
void KinectController::Run()
{
	// Initialize the keyPressed boolean for holding down key

	int forward = 0x27;
	INPUT ip;
	int size = 20;
	bool keyPressedArray[20];
	for (int i = 0; i < size; i++) {
		keyPressedArray[i] = false;
	};
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

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
        Update(forward, ip, keyPressedArray);
    }
}

/// <summary>
/// Main processing function
/// </summary>
void KinectController::Update(int &forward, INPUT &ip, bool *keyPressedArray)
{
	//std::cout << "UPDATE\n";
    if (NULL == m_pNuiSensor)
    {
        return;
    }

    // Wait for 0ms, just quickly test if it is time to process a skeleton
    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0) )
    {
        ProcessSkeleton(forward, ip, keyPressedArray);
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
void KinectController::ProcessSkeleton(int &forward, INPUT &ip, bool *keyPressedArray)
{
	NUI_SKELETON_FRAME skeletonFrame = { 0 };

	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if (FAILED(hr))
	{
		return;
	}

	// smooth out the skeleton data
	m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

	for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;

		if (NUI_SKELETON_TRACKED == trackingState)
		{

			for (int i = 0; i < 20; i++) {
				keyPressedArray[i] = false;
			}
			// We're tracking the skeleton

			/*std::cout << "Spine value: ";
			std::cout << skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].y << "\n";
			std::cout << "Left Foot Value: ";
			std::cout << skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_LEFT].y << "\n";
			std::cout << "Right Foot Value: ";
			std::cout << skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].y << "\n";*/

			// keyPressedArray[0]
			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x -
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].x > .5) {
				std::cout << "RIGHT ARM EXTENDED\n";
				ip.ki.wVk = 0x27; // virtual-key code for right arrow
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[0] = true;
				forward = 0x27;
			}

			// keyPressedArray[2]
			else if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x -
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].x < -.5) {
				std::cout << "LEFT ARM EXTENDED\n";
				ip.ki.wVk = 0x25; // virtual-key code for left arrow
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[2] = true;
				forward = 0x25;
			}

			// SHORYUKEN
			if ((skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y -
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].y > .65)) {
				std::cout << "SHORYUKEN\n";
				ip.ki.wVk = 0x27; // virtual-key code for forward
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(50);
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				ip.ki.wVk = 0x28; // virtual-key code for down
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(50);
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				ip.ki.wVk = 0x27; // virtual-key code for forward
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(50);
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				ip.ki.wVk = 0x31; // virtual-key code for 1
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(100);
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
			}

			// keyPressedArray[1]
			else if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y - 
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].y > .65) {
				std::cout << "RIGHT ARM UP\n";
				ip.ki.wVk = 0x26; // virtual-key code for up arrow
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[1] = true;
			}

			// keyPressedArray[4]
			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_LEFT].z - 
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.7) {
				std::cout << "LEFT FOOT FORWARD\n";
				ip.ki.wVk = 0x57; // virtual-key code for w
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[4] = true;
				Sleep(300);
			}

			// keyPressedArray[5]
			if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].z - 
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.7) {
				std::cout << "RIGHT FOOT FORWARD\n";
				ip.ki.wVk = 0x51; // virtual-key code for q
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[5] = true;
				Sleep(700);
			}

			if ((skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z -
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.5) &&
				(skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z -
					skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.5) &&
					(skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].x -
						skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].x < .1) &&
						(skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].y -
							skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].y < .1)) {
				std::cout << "HADOUKEN\n";
				ip.ki.wVk = 0x28; // virtual-key code for down
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(50);
				ip.ki.wVk = 0x27; // virtual-key code for forward
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(50);
				ip.ki.wVk = 0x28; // virtual-key code for down
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				ip.ki.wVk = 0x31; // virtual-key code for 1
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(100);
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
				ip.ki.wVk = 0x27; // virtual-key code for down
				ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
				SendInput(1, &ip, sizeof(INPUT));
			}

			// keyPressedArray[3]
			else if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT].z -
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.5) {
				std::cout << "RIGHT ARM FORWARD\n";
				ip.ki.wVk = 0x31; // virtual-key code for 1
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[3] = true;
			}

			// keyPressedArray[6]
			else if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT].z - 
				skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].z < -.5) {
				std::cout << "LEFT ARM FORWARD\n";
				ip.ki.wVk = 0x32; // virtual-key code for 2
				ip.ki.dwFlags = 0; // 0 for key press
				SendInput(1, &ip, sizeof(INPUT));
				Sleep(30);
				keyPressedArray[6] = true;
			}

			//// Crouch - keyPressedArray[7]
			//if (skeletonFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SPINE].y < -.6) {
			//	std::cout << "CROUCHING\n";
			//	ip.ki.wVk = 0x28; // virtual-key code for down arrow
			//	ip.ki.dwFlags = 0; // 0 for key press
			//	SendInput(1, &ip, sizeof(INPUT));
			//	Sleep(30);
			//	keyPressedArray[7] = true;
			//}

			for (int i = 0; i < 20; i++) {
				if (keyPressedArray[i] == false) {
					ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
					SendInput(1, &ip, sizeof(INPUT));
				}
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