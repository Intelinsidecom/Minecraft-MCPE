#include "AppPlatform_win32.h"
#include "util/Mth.h"

int AppPlatform_win32::getScreenWidth()  { return 854; }
int AppPlatform_win32::getScreenHeight() { return 480; }

float AppPlatform_win32::getPixelsPerMillimeter() {
	// assuming 24" @ 1920x1200
	const int w = 1920;
	const int h = 1200;
	const float pixels = Mth::sqrt(w*w + h*h);
	const float mm	   = 24 * 25.4f;
	return pixels / mm;
}

bool AppPlatform_win32::supportsTouchscreen()  { return true; }
bool AppPlatform_win32::hasBuyButtonWhenInvalidLicense() { return true; }

void AppPlatform_win32::grabMouse() {
	if (!_hwnd || _grabbed) return;
	_grabbed = true;
	_ignoreNextMove = true;
	ShowCursor(FALSE);
	RECT rect;
	GetClientRect(_hwnd, &rect);
	int centerX = (rect.right - rect.left) / 2;
	int centerY = (rect.bottom - rect.top) / 2;
	POINT pt = { centerX, centerY };
	ClientToScreen(_hwnd, &pt);
	SetCursorPos(pt.x, pt.y);

	GetClientRect(_hwnd, &rect);
	ClientToScreen(_hwnd, (LPPOINT)&rect.left);
	ClientToScreen(_hwnd, (LPPOINT)&rect.right);
	ClipCursor(&rect);
}

void AppPlatform_win32::releaseMouse() {
	if (!_grabbed) return;
	_grabbed = false;
	ShowCursor(TRUE);
	ClipCursor(NULL);
}
