/*
	This file was modified from the Microsoft Kinect 1.8 SDK Developer Toolkit
	SkeletonBasics - C++ program.
*/

#pragma once

#include "NuiApi.h"
#define WINVER 0x0500

class KinectController
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    KinectController();

    /// <summary>
    /// Destructor
    /// </summary>
    ~KinectController();

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    void Run();

private:
    // Current Kinect
    INuiSensor* m_pNuiSensor;
    
    HANDLE m_pSkeletonStreamHandle;
    HANDLE m_hNextSkeletonEvent;

	/// <summary>
	/// Main processing function
	/// </summary>
	void Update();

    /// <summary>
    /// Create the first connected Kinect found 
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT CreateFirstConnected();

    /// <summary>
    /// Handle new skeleton data
    /// </summary>
    void ProcessSkeleton();

};
