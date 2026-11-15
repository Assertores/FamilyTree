// Unit tests of the Default Platform: reading a real Data Folder from the disk.

#include "default_platform_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>
#include <family_tree/api.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <system_error>

namespace {

using namespace test;

// A Data Folder in the temporary folder of the operating system, removed again afterwards.
class DiskFolder {
public:
	DiskFolder()
		: myPath(std::filesystem::temp_directory_path() / "family_tree_default_platform_test") {
		std::filesystem::remove_all(myPath);
		std::filesystem::create_directories(myPath);
	}
	DiskFolder(const DiskFolder&) = delete;
	DiskFolder& operator=(const DiskFolder&) = delete;
	DiskFolder(DiskFolder&&) = delete;
	DiskFolder& operator=(DiskFolder&&) = delete;
	~DiskFolder() {
		// A folder that cannot be removed is left behind; the next run removes it.
		std::error_code ignored;
		std::filesystem::remove_all(myPath, ignored);
	}

	void Write(const std::string& aRelativePath, const std::string& aContent) const {
		const auto path = myPath / std::filesystem::u8path(aRelativePath);
		std::filesystem::create_directories(path.parent_path());
		std::ofstream(path, std::ios::binary) << aContent;
	}

	[[nodiscard]] std::string Path() const { return myPath.u8string(); }

private:
	std::filesystem::path myPath;
};

// docs/requirements/req_0037.md, docs/requirements/req_0001.md, docs/requirements/req_0006.md
bool
ReadsADataFolderFromTheDisk() {
	const DiskFolder disk;
	disk.Write("1_Anna/data.json", PersonFile(1, R"("FirstNames": ["Anna"])"));
	disk.Write("2_Bernd/data.json", PersonFile(2));
	disk.Write("relations.csv", RelationsFile({"1,2,StrictlyLower"}));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(FT_Create(disk.Path().c_str(), &context, &trace), FT_OK);
	size_t count = 0;
	CHECK(FT_GetPersonCount(context.Get(), &count, nullptr), FT_OK);
	CHECK(count, 2U);
	const FT_PersonId* children = nullptr;
	CHECK(FT_GetChildren(context.Get(), 1, &children, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	CHECK(children[0], 2);
	return true;
}

// docs/requirements/req_0037.md, docs/requirements/req_0010.md
bool
ReadsFolderNamesWithUmlauts() {
	const DiskFolder disk;
	disk.Write("1_Jürgen_Weiß/data.json", PersonFile(1));
	disk.Write("relations.csv", RelationsFile({}));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(FT_Create(disk.Path().c_str(), &context, &trace), FT_OK);
	size_t count = 0;
	CHECK(FT_GetPersonCount(context.Get(), &count, nullptr), FT_OK);
	CHECK(count, 1U);
	return true;
}

// docs/requirements/req_0025.md, docs/requirements/req_0037.md
bool
ChecksWhetherMediaFilesExistOnTheDisk() {
	const DiskFolder disk;
	disk.Write("1_Anna/data.json", PersonFile(1, R"("Images": ["da.png", "fehlt.png"])"));
	disk.Write("1_Anna/da.png", "");
	disk.Write("relations.csv", RelationsFile({}));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(FT_Create(disk.Path().c_str(), &context, &trace), FT_OK_WITH_WARNINGS);
	const char* const* paths = nullptr;
	size_t count = 0;
	CHECK(FT_GetPicturePaths(context.Get(), 1, &paths, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	CHECK(std::filesystem::exists(std::filesystem::u8path(paths[0])), true);
	return true;
}

// docs/requirements/req_0037.md, docs/decisions/adr_0017.md
bool
AMissingDataFolderIsAPlatformError() {
	ContextGuard context;
	TraceBuffer trace;
	const auto missing =
		(std::filesystem::temp_directory_path() / "family_tree_does_not_exist").u8string();
	CHECK(FT_Create(missing.c_str(), &context, &trace), FT_ERROR_PLATFORM);
	CHECK(static_cast<bool>(context), false);
	return true;
}

// docs/requirements/req_0037.md
bool
LeavingEveryQuestionToTheDefaultIsTheSameAsCreate() {
	const DiskFolder disk;
	disk.Write("1_Anna/data.json", PersonFile(1));
	disk.Write("relations.csv", RelationsFile({}));
	FT_Builder* builder = nullptr;
	FT_Question question{};
	FT_Context* context = nullptr;
	FT_Result result = FT_StartCreate(disk.Path().c_str(), &builder, &question, nullptr);
	for (size_t step = 0; step < 100 && result == FT_OK && question.type != FT_QUESTION_NONE;
		 ++step) {
		FT_Answer answer{};
		answer.useDefault = true;
		result = FT_Continue(builder, &answer, &question, &context, nullptr);
	}
	CHECK(result, FT_OK);
	CHECK(context != nullptr, true);
	size_t count = 0;
	CHECK(FT_GetPersonCount(context, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	CHECK(FT_Free(context, nullptr), FT_OK);
	return true;
}

} // namespace

bool
DefaultPlatformSuite() {
	auto result = SUITE("DefaultPlatform");
	result &= RUN(ReadsADataFolderFromTheDisk);
	result &= RUN(ReadsFolderNamesWithUmlauts);
	result &= RUN(ChecksWhetherMediaFilesExistOnTheDisk);
	result &= RUN(AMissingDataFolderIsAPlatformError);
	result &= RUN(LeavingEveryQuestionToTheDefaultIsTheSameAsCreate);
	return result;
}
