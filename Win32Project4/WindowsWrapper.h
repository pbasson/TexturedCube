#pragma once

// some general numerical typedefs

typedef unsigned char u8; // unsigned char

// A wrapper to a MS window

class WindowsWrapper
{
public:
	WindowsWrapper(int width, int height, TCHAR *windowName, TCHAR *className);
	~WindowsWrapper();
	void Tick();
	bool QuitRequested() const;
	int GetLastKeyHit() const;
	int GetLastVKeyHit() const;
	bool GetLastMousePress(float &x, float &y);
	LRESULT LocalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND GetHwnd() { return hwnd; }

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); // there can only be one of these - we encode the hwnd on the window

private:
	WindowsWrapper();

	HWND hwnd;
	bool quitRequested_;
	int lastKeyHit_;
	int lastVKeyHit_;
	bool mousePressed_;
	int mouseX_, mouseY_;
};
