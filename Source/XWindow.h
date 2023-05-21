#pragma once
#include "common.h"
#include "XExceptionBase.h"

#define XWindowExcept(hr) XWindow::XWindowException(__LINE__, __FILE__, hr)
#define XWindowLastExcept() XWindow::XWindowException(__LINE__, __FILE__, GetLastError())

/* 
* XWindow: Class handles window messages, creation & destruction of hWnd.
*/
class XWindow 
{
private:
	HWND hWnd = 0;
	int Width = 0;
	int Height = 0;
	/*
	 * These functions cannot handle exceptions because they are C based. 
	 * You'd have to create an exception marshaling system.
	 */
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	XWindow(int Width, int Height, LPCWSTR Name);
	~XWindow();
	XWindow(const XWindow&) = delete;
	XWindow& operator=(const XWindow&) = delete;
private:
	/* 
	* XWindowClass: Singleton design pattern that manages registration and cleanup of WNDCLASSEX.
	* NOTE: you could create the singleton through a function and try catch errors because currently
	* this class gets constructed outside the try catch block in main upon execution startup.
	*/
	class XWindowClass 
	{
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
	/* 
	* XException: Class designed to handle errors via std::exceptions.
	*/
	class XWindowException : public XExceptionBase 
	{
	private:
		HRESULT hr;
	public:
		XWindowException(int Line, const char* File, HRESULT hr);
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept override;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	};
};
