#include <Windows.h>
#include <Ole2.h>

#include <SDL_opengl.h>
#include <SDL.h>

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

#define width 640
#define height 480

// OpenGl Variables
GLuint textureId;;
GLubyte data[width * height * 4];

// Kinect Variables
HANDLE rgbStream;
INuiSensor* sensor;

bool initKinect() {
	// Get a working kinect sensor
	int numSensors;
	if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1)
		return false;
	if (NuiCreateSensorByIndex(0, &sensor) < 0)
		return false;

	// Init the sensor itself
	sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_COLOR);
	sensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,          // Depth camera or rgb camera?
		NUI_IMAGE_RESOLUTION_640x480,  // Image resolution
		0,                             // Image stream flags, ie near mode
		2,                             // Number of frames to buffer
		NULL,                          // Event Handle
		&rgbStream);                   // Our handle that we made
	return sensor;
}

void getKinectData(GLubyte* dest) {

	// Fetch and Lock the frame so it doesn't get corrupted while we are reading it
	NUI_IMAGE_FRAME imageFrame;
	NUI_LOCKED_RECT LockedRect;

	// Second parameter of GetNextFrame is amount of time to wait before getting next frame
	// Increase this if it is returning negative numbers
	if (sensor->NuiImageStreamGetNextFrame(rgbStream, 0, &imageFrame) < 0)
		return;    // Not valid data
	INuiFrameTexture* texture = imageFrame.pFrameTexture;
	texture->LockRect(0, &LockedRect, NULL, 0);

	// Copy the data from the frame to our own memory locations
	// Pitch is how many bytes are in each row of the frame
	if (LockedRect.Pitch != 0)  // Make sure the frame is not empty
	{
		const BYTE* curr = (const BYTE*)LockedRect.pBits;
		const BYTE* dataEnd = curr + (width*height) * 4;

		while (curr < dataEnd) {
			*dest++ = *curr++;
		}

		// Release the frame so the kinect can read another
		texture->UnlockRect(0);
		sensor->NuiImageStreamReleaseFrame(rgbStream, &imageFrame);
	}
}

