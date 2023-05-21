#include <codecvt>
#include <string.h>
#include "common.h"
#include "XWindow.h"
#include "XExceptionBase.h"
#include <stringapiset.h>
/*
LPCWSTR Char2WChar(const char* InputString)
{
	int ConvertResult = MultiByteToWideChar(CP_UTF8, 0, InputString, (int)strlen(InputString), NULL, 0);
}
*/
#if 0
const LPCWSTR pClassName = TEXT("cxxhw3d");
WCHAR buffer[2] = { '\0', '\0' };

/*
* NOTE: wParam & lParam will mutate based on message.
*/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message)
	{
		case WM_CLOSE: {
			PostQuitMessage(666);
			break;
		}
		case WM_KEYDOWN: {
			if (wParam == 'F') {
				SetWindowTextW(hWnd, TEXT("FFFFFFFFFFFFFFFF"));
			}
			if (wParam == VK_ESCAPE) {
				PostQuitMessage(666);
			}
			break;
		}
		case WM_KEYUP: {
			if (wParam == 'F') {
				SetWindowTextW(hWnd, TEXT("0000000000000000"));
			}
			break;
		}
		 /* this has shift sensitivity */
		case WM_CHAR: {
			buffer[0] = wParam;
			SetWindowTextW(hWnd, buffer);
			break;
		}
		case WM_LBUTTONDOWN: {
			POINTS point = MAKEPOINTS(lParam);
			break;
		}
		default: {
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif

int CALLBACK WinMain ( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd ) {
#if 0
	WNDCLASSEX WindowClass = { 0 };
	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_OWNDC;
	WindowClass.lpfnWndProc = WindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = hInstance;
	WindowClass.hIcon = nullptr;
	WindowClass.hCursor = nullptr;
	WindowClass.hbrBackground = nullptr;
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = pClassName;
	WindowClass.hIconSm = nullptr;

	RegisterClassEx(&WindowClass);

	HWND hWnd = CreateWindowEx(0, pClassName, pClassName, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, 200, 200, 640, 480, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hWnd, SW_SHOW);
#endif

	try
	{
		XWindow Window = XWindow(640, 480, TEXT("cxxhwd3d"));

		MSG message;
		BOOL gResult;
		while ((gResult = GetMessage(&message, nullptr, 0, 0)) > 0) 
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (gResult == -1) 
		{
			return -1;
		}
		else 
		{
			return (int)message.wParam;
		}
	}
	catch (const XExceptionBase& e)
	{
		::std::wstring WideWhat;
		if (e.what() != nullptr)
		{
			int ConvertResult = MultiByteToWideChar(CP_UTF8, 0, e.what(), (int)strlen(e.what()), NULL, 0);
			if (ConvertResult <= 0)
			{
				WideWhat = L"Exception occurred: Failure to convert its message text using MultiByteToWideChar: convertResult=";
			}
			else
			{
				WideWhat.resize(ConvertResult + 10);
				ConvertResult = MultiByteToWideChar(CP_UTF8, 0, e.what(), (int)strlen(e.what()), &WideWhat[0], (int)WideWhat.size());
				if (ConvertResult <= 0)
				{
					WideWhat = L"Failed to convert Message!";
				}
				else
				{
					WideWhat.insert(0, L"Exception Occurred");
				}
			}
		}
		else
		{
			WideWhat = L"Exception occurred: Unknown.";
		}
		MessageBox(nullptr, WideWhat.c_str(), TEXT("XException"), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		::std::wstring WideWhat;
		if (e.what() != nullptr)
		{
			int ConvertResult = MultiByteToWideChar(CP_UTF8, 0, e.what(), (int)strlen(e.what()), NULL, 0);
			if (ConvertResult <= 0)
			{
				WideWhat = L"Exception occurred: Failure to convert its message text using MultiByteToWideChar: convertResult=";
				// WideWhat += ConvertResult.ToString()->Data();
				// WideWhat += L"  GetLastError()=";
				// WideWhat += GetLastError().ToString()->Data();
			}
			else
			{
				WideWhat.resize(ConvertResult + 10);
				ConvertResult = MultiByteToWideChar(CP_UTF8, 0, e.what(), (int)strlen(e.what()), &WideWhat[0], (int)WideWhat.size());
				if (ConvertResult <= 0)
				{
					WideWhat = L"Failed to convert Message!";
				}
				else
				{
					WideWhat.insert(0, L"Exception Occurred");
				}
			}
		}
		else
		{
			WideWhat = L"Exception occurred: Unknown.";
		}
		MessageBox(nullptr, WideWhat.c_str(), TEXT("Standard Exception"), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(nullptr, TEXT("No details available"), TEXT("Unknown Exception"), MB_OK | MB_ICONEXCLAMATION );
	}
	return 0;
}