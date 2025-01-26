#include "setup_view.hpp"

#include "search_view.hpp"

#include <context_adapter.hpp>
#include <imgui.h>

// clang-format off
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
// clang-format on

namespace ui {
namespace {
std::string
GetFolder() {
	BROWSEINFOW setting{};
	setting.ulFlags = BIF_RETURNONLYFSDIRS;
	auto* result = SHBrowseForFolderW(&setting);
	if (result == nullptr) {
		return {};
	}
	// std::array<wchar_t, MAX_PATH> szDirectory{};
	std::wstring szDirectory{};
	szDirectory.resize(MAX_PATH);
	SHGetPathFromIDListW(result, szDirectory.data());

	int nameLengthAsUtf8Char = WideCharToMultiByte(
		CP_UTF8,
		0,
		szDirectory.c_str(),
		static_cast<int>(szDirectory.size()),
		nullptr,
		0,
		nullptr,
		nullptr);
	if (nameLengthAsUtf8Char == 0) {
		return {};
	}
	std::string convertedName{};
	convertedName.resize(nameLengthAsUtf8Char);
	WideCharToMultiByte(
		CP_UTF8,
		0,
		szDirectory.c_str(),
		static_cast<int>(szDirectory.size()),
		convertedName.data(),
		nameLengthAsUtf8Char,
		nullptr,
		nullptr);

	return convertedName;
}
} // namespace

std::shared_ptr<View>
SetupView::Print(WindowFactory aWindowFactory) {
	if (ImGui::Button("Load Data")) {
		auto folder = GetFolder();
		if (!folder.empty()) {
			return std::make_shared<SearchView>(ContextAdapter::Create(folder.c_str()));
		}
	}
	return nullptr;
}
} // namespace ui
