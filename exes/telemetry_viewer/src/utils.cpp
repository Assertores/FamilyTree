#include "utils.hpp"

#include <iostream>

// clang-format off
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
// clang-format on

namespace ui {
std::filesystem::path
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

	// NOLINTNEXTLINE(readability-redundant-string-cstr) to remove additional 0 bytes at the end.
	return convertedName.c_str();
}

std::vector<std::filesystem::path>
GetDirs(std::filesystem::path aPath) {
	std::vector<std::filesystem::path> result;
	for (const auto& dir : std::filesystem::directory_iterator{aPath}) {
		if (!dir.is_directory()) {
			continue;
		}
		result.emplace_back(dir.path());
	}
	return result;
}
} // namespace ui