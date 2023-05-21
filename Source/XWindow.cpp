#pragma once
#include "XWindow.h"
#include <sstream>

/*
* Window
*/

XWindow::XWindow(int Width, int Height, const LPCWSTR Name)
{
	// throw XWndExcept(ERROR_ARENA_TRASHED);
	// throw std::runtime_error("Error!");
	// throw 666;
	// calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = Width + wr.left;
	wr.top = 100;
	wr.bottom = Height + wr.top;
	
	if (FAILED(AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE)))
	{
		throw XWindowLastExcept();
	}
	// create window and get hwnd
	// hWnd = CreateWindowEx(0, XWindowClass::GetName(), Name, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, XWindowClass::GetInstance(), this);
	hWnd = CreateWindowEx(0, L"XWindowClass::GetName()", Name, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, XWindowClass::GetInstance(), this);
	if (hWnd == nullptr)
	{
		throw XWindowLastExcept();
	}
	// show window
	ShowWindow(hWnd, SW_SHOW);
};

XWindow::~XWindow() 
{
	DestroyWindow(hWnd);
}

LRESULT CALLBACK XWindow::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept 
{
	if (msg == WM_NCCREATE)
	{
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		XWindow* const pWnd = static_cast<XWindow*>(pCreate->lpCreateParams);
		// set WINAPI managed user data to store ptr to window class
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&XWindow::HandleMsgThunk));
		// forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	// if we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);

}

LRESULT XWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept 
{
	switch (msg)
	{
		case WM_CLOSE: 
		{
			PostQuitMessage(666);
			return 0;
		}
		default: 
		{
			break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK XWindow::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept 
{
	XWindow* const pWnd = reinterpret_cast<XWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

/*
* WindowClass
*/

XWindow::XWindowClass XWindow::XWindowClass::SingletonWindowClass;

XWindow::XWindowClass::XWindowClass() noexcept : Instance(GetModuleHandle(nullptr)) 
{
	WNDCLASSEX WindowClass = { 0 };
	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_OWNDC;
	WindowClass.lpfnWndProc = HandleMsgSetup;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetInstance();
	WindowClass.hIcon = nullptr;
	WindowClass.hCursor = nullptr;
	WindowClass.hbrBackground = nullptr;
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = GetName();
	WindowClass.hIconSm = nullptr;
	RegisterClassEx(&WindowClass);
}

XWindow::XWindowClass::~XWindowClass() noexcept 
{
	UnregisterClass(XWindowClass::GetName(), XWindowClass::GetInstance());
}

const LPCWSTR XWindow::XWindowClass::GetName() noexcept 
{
	return ClassName;
}

HINSTANCE XWindow::XWindowClass::GetInstance() noexcept 
{
	return SingletonWindowClass.Instance;
}

/*
* XException
*/

XWindow::XWindowException::XWindowException(int Line, const char* File, HRESULT hr): XExceptionBase(Line, File), hr(hr)
{

}

const char* XWindow::XWindowException::what() const noexcept
{
	std::ostringstream outstringstream;
	outstringstream << GetType() << std::endl << "[Error Code]" << GetErrorCode() << std::endl << "[Description]" << GetErrorString() << std::endl << GetOriginString();
	WhatBuffer = outstringstream.str();
	return WhatBuffer.c_str();
}

const char* XWindow::XWindowException::GetType() const noexcept
{
	return "XWindow XException";
}

std::string XWindow::XWindowException::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuff = nullptr;
	DWORD nMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&pMsgBuff), 0, nullptr);
	if (nMsgLen == 0)
	{
		return "Undefined Error Code";
	}
	std::string ErrorString = pMsgBuff;
	LocalFree(pMsgBuff);
	return ErrorString;
}

HRESULT XWindow::XWindowException::GetErrorCode() const noexcept
{
	return hr;
}

std::string XWindow::XWindowException::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr);
}

std::wstring ToWide(const std::string& narrow)
{
	std::wstring wide;
	wide.resize(narrow.size() + 1);
	size_t actual;
	mbstowcs_s(&actual, wide.data(), wide.size(), narrow.c_str(), _TRUNCATE);
	if (actual > 0)
	{
		wide.resize(actual - 1);
		return wide;
	}
	return {};
}

std::string ToNarrow(const std::wstring& wide)
{
	std::string narrow;
	narrow.resize(wide.size() * 2);
	size_t actual;
	wcstombs_s(&actual, narrow.data(), narrow.size(), wide.c_str(), _TRUNCATE);
	narrow.resize(actual - 1);
	return narrow;
}
