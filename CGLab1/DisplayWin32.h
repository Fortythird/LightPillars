#pragma once

#include "export.h"
#include "InputDevice.h"

class DisplayWin32
{
private:
	LPCWSTR applicationName;
	HINSTANCE hInstance;
	HWND hWnd;
	WNDCLASSEX wc;

	int screenWidth;
	int screenHeight;
	int posX;
	int posY;

	LONG dwStyle;

public:
	int getScreenHeight() {
		return screenHeight;
	}

	int getScreenWidth() {
		return screenWidth;
	}

	long getDisplayStyle() {
		return dwStyle;
	}

	HWND getHWND() {
		return hWnd;
	}

	void CreateDisplay(InputDevice* inputDevice);
	void ShowClientWindow(InputDevice* inputDevice);
	void OnChangeScreenSize(const ScreenSize& arguments);
};