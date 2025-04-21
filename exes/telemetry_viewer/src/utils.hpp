#pragma once

#include <filesystem>
#include <vector>

namespace ui {
std::filesystem::path GetFolder();
std::vector<std::filesystem::path> GetDirs(std::filesystem::path aPath);
} // namespace ui