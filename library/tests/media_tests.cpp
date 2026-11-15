// Unit tests of the Media paths, and the arguments of the Media defaults. Whether a default
// actually opens a program of the operating system cannot be checked by a unit test.

#include "media_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>
#include <family_tree/api.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace {

using namespace test;

bool
EndsWith(const std::string& aText, const std::string& aEnd) {
	std::string text = aText;
	std::replace(text.begin(), text.end(), '\\', '/');
	return text.size() >= aEnd.size()
		   && text.compare(text.size() - aEnd.size(), aEnd.size(), aEnd) == 0;
}

struct Media {
	DataFolder folder;
	ContextGuard context;
	TraceBuffer trace;
	FT_Result result = FT_ERROR_INVALID_ARGUMENT;

	explicit Media(const std::string& aFurtherKeys, const std::vector<std::string>& aMediaFiles)
		: folder(PrivFolder(aFurtherKeys, aMediaFiles))
		, result(Create(folder, context, trace)) {}

	[[nodiscard]] std::vector<std::string> Pictures() const {
		const char* const* paths = nullptr;
		size_t count = 0;
		if (FT_GetPicturePaths(context.Get(), 1, &paths, &count, nullptr) != FT_OK) {
			return {"<error>"};
		}
		return {paths, paths + count};
	}

	[[nodiscard]] std::string Recording() const {
		const char* path = "<untouched>";
		if (FT_GetRecordingPath(context.Get(), 1, &path, nullptr) != FT_OK) {
			return "<error>";
		}
		return Text(path);
	}

private:
	static DataFolder PrivFolder(
		const std::string& aFurtherKeys,
		const std::vector<std::string>& aMediaFiles) {
		DataFolder folder;
		folder.AddPerson("1_Anna", PersonFile(1, aFurtherKeys)).SetRelations({});
		for (const auto& file : aMediaFiles) {
			folder.AddFile("1_Anna/" + file, "not really an image or a recording");
		}
		return folder;
	}
};

// docs/requirements/req_0023.md, docs/requirements/req_0011.md
bool
PicturePathsAreResolvedInThePersonFolder() {
	const Media media(
		R"("Images": ["Trohnsaal.png", "Hochzeit.jpg"])",
		{"Trohnsaal.png", "Hochzeit.jpg"});
	CHECK(media.result, FT_OK);
	const auto pictures = media.Pictures();
	CHECK(pictures.size(), 2U);
	CHECK(EndsWith(pictures[0], "family/1_Anna/Trohnsaal.png"), true);
	CHECK(EndsWith(pictures[1], "family/1_Anna/Hochzeit.jpg"), true);
	return true;
}

// docs/requirements/req_0023.md, docs/requirements/req_0025.md
bool
AMissingPictureFileIsLeftOutAndReported() {
	const Media media(R"("Images": ["Trohnsaal.png", "verloren.png"])", {"Trohnsaal.png"});
	CHECK(media.result, FT_OK_WITH_WARNINGS);
	const auto pictures = media.Pictures();
	CHECK(pictures.size(), 1U);
	CHECK(EndsWith(pictures[0], "Trohnsaal.png"), true);
	CHECK(media.trace.HasWarning("verloren.png"), true);
	return true;
}

// docs/requirements/req_0025.md
bool
APersonWithoutPicturesHasNoPaths() {
	const Media media("", {});
	CHECK(media.result, FT_OK);
	CHECK(media.Pictures().size(), 0U);
	return true;
}

// docs/requirements/req_0024.md, docs/requirements/req_0011.md
bool
TheRecordingPathIsResolvedInThePersonFolder() {
	const Media media(R"("Audio": "Sunday_Plans.mp3")", {"Sunday_Plans.mp3"});
	CHECK(media.result, FT_OK);
	CHECK(EndsWith(media.Recording(), "family/1_Anna/Sunday_Plans.mp3"), true);
	return true;
}

// docs/requirements/req_0024.md, docs/requirements/req_0025.md
bool
APersonWithoutRecordingHasNoPath() {
	const Media media("", {});
	CHECK(media.result, FT_OK);
	CHECK(media.Recording(), "<null>");
	return true;
}

// docs/requirements/req_0024.md, docs/requirements/req_0025.md
bool
AMissingRecordingFileHasNoPathAndIsReported() {
	const Media media(R"("Audio": "verloren.mp3")", {});
	CHECK(media.result, FT_OK_WITH_WARNINGS);
	CHECK(media.Recording(), "<null>");
	CHECK(media.trace.HasWarning("verloren.mp3"), true);
	return true;
}

// docs/requirements/req_0023.md, docs/requirements/req_0024.md
bool
TheDefaultsNeedAPath() {
	CHECK(FT_DefaultShowPicture(nullptr, nullptr), FT_ERROR_INVALID_ARGUMENT);
	CHECK(FT_DefaultPlayRecording(nullptr, nullptr), FT_ERROR_INVALID_ARGUMENT);
	return true;
}

// docs/decisions/adr_0006.md
bool
TheConvenienceFunctionsNeedAKnownPerson() {
	const Media media("", {});
	CHECK(FT_ShowPicturesOfPerson(media.context.Get(), 2, nullptr), FT_ERROR_UNKNOWN_PERSON);
	CHECK(FT_PlayRecordingOfPerson(media.context.Get(), 2, nullptr), FT_ERROR_UNKNOWN_PERSON);
	return true;
}

// docs/requirements/req_0025.md, docs/decisions/adr_0006.md
bool
PlayingThePersonWithoutRecordingDoesNothing() {
	const Media media("", {});
	CHECK(FT_PlayRecordingOfPerson(media.context.Get(), 1, nullptr), FT_OK);
	CHECK(FT_ShowPicturesOfPerson(media.context.Get(), 1, nullptr), FT_OK);
	return true;
}

} // namespace

bool
MediaSuite() {
	auto result = SUITE("Media");
	result &= RUN(PicturePathsAreResolvedInThePersonFolder);
	result &= RUN(AMissingPictureFileIsLeftOutAndReported);
	result &= RUN(APersonWithoutPicturesHasNoPaths);
	result &= RUN(TheRecordingPathIsResolvedInThePersonFolder);
	result &= RUN(APersonWithoutRecordingHasNoPath);
	result &= RUN(AMissingRecordingFileHasNoPathAndIsReported);
	result &= RUN(TheDefaultsNeedAPath);
	result &= RUN(TheConvenienceFunctionsNeedAKnownPerson);
	result &= RUN(PlayingThePersonWithoutRecordingDoesNothing);
	return result;
}
