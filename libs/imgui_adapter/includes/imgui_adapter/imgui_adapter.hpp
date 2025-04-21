#pragma once

#include <imgui.h>

#include <memory>
#include <string>

class ImGuiAdapter {
public:
	static std::unique_ptr<ImGuiAdapter> CreateWindows();

	virtual ~ImGuiAdapter() = default;

	virtual void Init(int aWidth, int aHight, std::string aName, ImVec4 aClearColor) = 0;
	virtual bool Update() = 0;
	virtual void Show() = 0;
	virtual void Shutdown() = 0;
};