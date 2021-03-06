/* -LICENSE-START-
 ** Copyright (c) 2016 Blackmagic Design
 **
 ** Permission is hereby granted, free of charge, to any person or organization
 ** obtaining a copy of the software and accompanying documentation covered by
 ** this license (the "Software") to use, reproduce, display, distribute,
 ** execute, and transmit the Software, and to prepare derivative works of the
 ** Software, and to permit third-parties to whom the Software is furnished to
 ** do so, all subject to the following:
 **
 ** The copyright notices in the Software and this entire statement, including
 ** the above license grant, this restriction and the following disclaimer,
 ** must be included in all copies of the Software, in whole or in part, and
 ** all derivative works of the Software, unless such copies or derivative
 ** works are solely in the form of machine-executable object code generated by
 ** a source language processor.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 ** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 ** FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 ** SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 ** FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 ** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 ** DEALINGS IN THE SOFTWARE.
 ** -LICENSE-END-
 */

#include "BMDSwitcherAPI.h"
#include <iostream>

static const uint8_t kCameraAddress = 1;	// Camera Number 1

int main(int argc, const char* argv[])
{
	HRESULT result;
	
	// Create discovery instance
	IBMDSwitcherDiscovery* discovery = CreateBMDSwitcherDiscoveryInstance();
	if (! discovery)
	{
		std::cerr << "Failed to create IBMDSwitcherDiscovery!" << std::endl;
		return 1;
	}
	
	// Use discovery instance to connect to switcher
	std::cout << "Connecting to switcher..." << std::endl;
	IBMDSwitcher* switcher;
	BMDSwitcherConnectToFailure connectToFailReason;
	result = discovery->ConnectTo(CFSTR("192.168.10.240"), &switcher, &connectToFailReason);
	discovery->Release();
	if (result != S_OK)
	{
		std::cerr << "Failed to connect to switcher!" << std::endl;
		return 1;
	}
	
	// Get camera control interface from switcher object
	IBMDSwitcherCameraControl* cameraControl;
	result = switcher->QueryInterface(IID_IBMDSwitcherCameraControl, (void**)&cameraControl);
	if (result != S_OK)
	{
		std::cerr << "Failed to obtain IBMDSwitcherCameraControl!" << std::endl;
		switcher->Release();
		return 1;
	}
	
	// Set focus to nearest
	std::cout << "Setting focus to nearest..." << std::endl;
	double nearestFocusValue = 0.0;
	result = cameraControl->SetFloats	(
											kCameraAddress,		// Camera number
											0,					// Lens
											0,					// Focus
											1, 					// Array size
											&nearestFocusValue	// Single element array with the desired value
										);
	if (result != S_OK)
	{
		std::cerr << "Failed to send set focus command!" << std::endl;
		cameraControl->Release();
		switcher->Release();
		return 1;
	}
	sleep(4);

	// Set focus to furthest
	std::cout << "Setting focus to furthest..." << std::endl;
	double furthestFocusValue = 1.0;
	result = cameraControl->SetFloats	(
											kCameraAddress,		// Camera number
											0,					// Lens
											0,					// Focus
											1, 					// Array size
											&furthestFocusValue	// Single element array with the desired value
										);
	if (result != S_OK)
	{
		std::cerr << "Failed to send set focus command!" << std::endl;
		cameraControl->Release();
		switcher->Release();
		return 1;
	}
	sleep(4);
	
	// Send auto focus command to camera
	std::cout << "Performing auto focus..." << std::endl;
	result = cameraControl->SetFlags	(
											kCameraAddress,		// Camera number
											0,					// Lens
										 	1,					// Instantaneous autofocus
										 	0,					// This command has no additional parameter
										 	NULL
										);
	if (result != S_OK)
	{
		std::cerr << "Failed to send auto focus command!" << std::endl;
		cameraControl->Release();
		switcher->Release();
		return 1;
	}
	sleep(4);

	// Keep adjusting focus until user break
	std::cout << "Continuously adjusting focus, press Ctrl+C to quit..." << std::endl;
	double midFocusValue = 0.5;
	result = cameraControl->SetFloats	(
											kCameraAddress,		// Camera number
											0,					// Lens
											0,					// Focus
											1, 					// Array size
											&midFocusValue		// Single element array with the desired value
										);
	int direction = 1;
	double accumulatedFocusOffset = 0;
	while (direction != 0)
	{
		double focusOffset = 0.05 * direction;
		
		result = cameraControl->OffsetFloats	(
													kCameraAddress,	// Camera number
													0,				// Lens
													0,				// Focus
													1, 				// Array size
													&focusOffset	// Single element array with the desired offset
												);
		if (result != S_OK)
		{
			std::cerr << "Failed to send offset focus command!" << std::endl;
			cameraControl->Release();
			switcher->Release();
			return 1;
		}
		
		accumulatedFocusOffset += focusOffset;
		if (fabs(accumulatedFocusOffset) > 0.45)
			direction *= -1;

		usleep(500000);
	}
	
	cameraControl->Release();
	switcher->Release();
	
    return 0;
}
