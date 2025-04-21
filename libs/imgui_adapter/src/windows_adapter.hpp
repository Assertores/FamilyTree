#pragma once

#include "imgui_adapter/imgui_adapter.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// clang-format off
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>
// clang-format on

class WindowsAdapter : public ImGuiAdapter {
public:
	void Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) override;
	bool Update() override;
	void Show() override;
	void Shutdown() override;

private:
	struct WglWindowData {
		HDC hDC;
	};

	bool myIsValid = false;
	HGLRC myHrc{};
	WglWindowData myMainWindow{};
	HWND myHwnd{};
	ImVec4 myClearColor;
	WNDCLASSEXW myWc{};

	bool PrivCreateDeviceWGL(HWND aHwnd, WglWindowData* aData);
	static void PrivCleanupDeviceWGL(HWND aHwnd, WglWindowData* aData);
};