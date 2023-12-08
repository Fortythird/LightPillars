#pragma once

#include "export.h"
#include "DisplayWin32.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT umessage, WPARAM wparam, LPARAM lparam) 
{
	KeyboardInputEventArgs keyboardInfo;
	RawMouseEventArgs mouseInfo;

	switch (umessage)
	{
	case WM_NCCREATE:
		SetLastError(0);
		SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)(((CREATESTRUCT*)lparam)->lpCreateParams));
		
		if (GetLastError() != 0) {
			std::cout << "SetWindowLongPtr error: " << GetLastError() << std::endl;
		}

		return true;
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		((InputDevice*)GetWindowLongPtr(hWnd, DWLP_USER))->OnChangeScreenSize(LOWORD(lparam), HIWORD(lparam));
		return 0;
	case WM_KEYDOWN:
		if (static_cast<unsigned int>(wparam) == (unsigned int)Keys::Escape)
		{
			std::cout << "Exiting the game..." << std::endl;
			PostQuitMessage(0);
		}
		else
		{
			keyboardInfo.Message = umessage;
			keyboardInfo.VKey = static_cast<unsigned int>(wparam);
			keyboardInfo.MakeCode = (HIWORD(lparam) & 0xFF);
			keyboardInfo.Flags = ((HIWORD(lparam) & 0xFF00) >> 8);
			((InputDevice*)GetWindowLongPtr(hWnd, DWLP_USER))->OnKeyDown(keyboardInfo);
		}
		
		return 0;
	case WM_KEYUP:
		keyboardInfo.Message = umessage;
		keyboardInfo.VKey = static_cast<unsigned int>(wparam);
		keyboardInfo.MakeCode = (HIWORD(lparam) & 0xFF);
		keyboardInfo.Flags = ((HIWORD(lparam) & 0xFF00) >> 8);
		((InputDevice*)GetWindowLongPtr(hWnd, DWLP_USER))->OnKeyDown(keyboardInfo);
		return 0;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		mouseInfo.X = LOWORD(lparam);
		mouseInfo.Y = HIWORD(lparam);
		mouseInfo.WheelDelta = 0;
		mouseInfo.ButtonFlags = LOWORD(wparam);
		((InputDevice*)GetWindowLongPtr(hWnd, DWLP_USER))->OnMouseMove(mouseInfo);
		return 0;
	case WM_MOUSEWHEEL:
		mouseInfo.X = LOWORD(lparam);
		mouseInfo.Y = HIWORD(lparam);
		mouseInfo.WheelDelta = (int)HIWORD(wparam);
		mouseInfo.ButtonFlags = LOWORD(wparam);
		((InputDevice*)GetWindowLongPtr(hWnd, DWLP_USER))->OnMouseMove(mouseInfo);
		return 0;
	default:
	{
		return DefWindowProc(hWnd, umessage, wparam, lparam);
	}
 	}
}

void DisplayWin32::CreateDisplay(InputDevice* inputDevice) 
{
	applicationName = L"CGLabFramework";
	screenWidth = 800;
	screenHeight = 800;

	hInstance = GetModuleHandle(nullptr);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 24;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class
	RegisterClassEx(&wc);

	dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

	posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	RECT windowRect = { 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		applicationName,
		applicationName,
		dwStyle,
		posX,
		posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		inputDevice
	);

	ShowClientWindow(inputDevice);
};

void DisplayWin32::ShowClientWindow(InputDevice* inputDevice) {
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ShowCursor(true);

}

void DisplayWin32::OnChangeScreenSize(const ScreenSize& arguments) {
	screenWidth = arguments.Width;
	screenHeight = arguments.Height;
}
