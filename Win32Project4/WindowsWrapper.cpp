#include "stdafx.h"
#include "WindowsWrapper.h"
#include <CommCtrl.h>
#include <math.h>
#include <windowsx.h>


WindowsWrapper::WindowsWrapper(int width, int height, TCHAR *windowName, TCHAR *className)
{
	quitRequested_ = false;
	lastKeyHit_ = 0;
	lastVKeyHit_ = 0;
	
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// register a windows class
	WNDCLASSEX wcex;
	if (!GetClassInfoEx(hInstance, className, &wcex))
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(WindowsWrapper*);
		wcex.hInstance = hInstance;
		wcex.hIcon = 0; //LoadIcon(hInstance, MAKEINTRESOURCE(IDC_TRACKINGAPI));
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = className;
		wcex.hIconSm = 0; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
		ATOM wClass = RegisterClassEx(&wcex);
	}
	

	// create a window of that type
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	RECT wr = { 0, 0, width, height };
	AdjustWindowRect(&wr, dwStyle, FALSE);    // adjust the size
	hwnd = CreateWindowEx(0, className, windowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	// store a pointer to this on the GDI window
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	mousePressed_ = false;
}

WindowsWrapper::~WindowsWrapper()
{
	DestroyWindow(hwnd);
}


LRESULT CALLBACK WindowsWrapper::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowsWrapper *wrapper = (WindowsWrapper*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (wrapper) // we'll get messages during window construction before the windowlongptr is set
		return wrapper->LocalWndProc(hWnd, message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT WindowsWrapper::LocalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	mousePressed_ = false;
	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_CLOSE:
		quitRequested_ = true;
		return 0; 
	case WM_CHAR:
		lastKeyHit_ = (int)wParam;
		return 0;
	case WM_KEYDOWN:
		lastVKeyHit_ = (int)wParam;
		return 0;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool WindowsWrapper::QuitRequested() const
{
	return quitRequested_;
}

void WindowsWrapper::Tick()
{
	MSG msg;

	while (PeekMessage(&msg, hwnd, 0, 0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int WindowsWrapper::GetLastKeyHit() const
{
	return lastKeyHit_;
}

int WindowsWrapper::GetLastVKeyHit() const
{
	return lastVKeyHit_;
}

bool WindowsWrapper::GetLastMousePress(float &x, float &y)
{
	if (mousePressed_)
	{
		RECT rec;
		GetClientRect(hwnd,&rec);
		x = (float)mouseX_ / (float)(rec.right - rec.left);
		y = (float)mouseY_ / (float)(rec.bottom - rec.top); 
		return true;
	}
	return false; 
}