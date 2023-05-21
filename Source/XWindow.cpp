#include "XWindow.h"

/*
* WindowClass
*/

XWindow::XWindowClass XWindow::XWindowClass::SingletonWindowClass;

XWindow::XWindowClass::XWindowClass() noexcept : Instance(GetModuleHandle(nullptr)) {
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

XWindow::XWindowClass::~XWindowClass() noexcept {
	UnregisterClass(XWindowClass::GetName(), XWindowClass::GetInstance());
}

const LPCWSTR XWindow::XWindowClass::GetName() noexcept {
	return ClassName;
}

HINSTANCE XWindow::XWindowClass::GetInstance() noexcept {
	return SingletonWindowClass.Instance;
}

/*
* Window
*/

XWindow::XWindow(int Width, int Height, const LPCWSTR Name) noexcept {
	// calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = Width + wr.left;
	wr.top = 100;
	wr.bottom = Height + wr.top;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
	// create window and get hwnd
	hWnd = CreateWindowEx(0, XWindowClass::GetName(), Name, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, XWindowClass::GetInstance(), this);
	// show window
	ShowWindow(hWnd, SW_SHOW);
};

XWindow::~XWindow() {
	DestroyWindow(hWnd);
}

LRESULT CALLBACK XWindow::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
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

LRESULT XWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg)
	{
		case WM_CLOSE: {
			PostQuitMessage(666);
			return 0;
		}
		default: {
			break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK XWindow::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	XWindow* const pWnd = reinterpret_cast<XWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}


