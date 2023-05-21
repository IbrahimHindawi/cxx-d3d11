#pragma once
#include "common.h"

/* 
* Window class handles window messages, creation & destruction of hWnd
*/
class XWindow {
private:
	/* 
	* singleton design pattern manages registration and cleanup of Window class WNDCLASSEX 
	*/
	class XWindowClass {
	public:
		static const LPCWSTR GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		static XWindowClass SingletonWindowClass;
		XWindowClass() noexcept;
		~XWindowClass() noexcept;
		XWindowClass(const XWindowClass& InputWindow) = delete;
		static constexpr const LPCWSTR ClassName = TEXT("cxxhw3d");
		HINSTANCE Instance;

	};
private:
	HWND hWnd = 0;
	int Width = 0;
	int Height = 0;
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	XWindow(int Width, int Height, LPCWSTR Name) noexcept;
	~XWindow();
	XWindow(const XWindow&) = delete;
	XWindow& operator=(const XWindow&) = delete;
};

