// Unit tests of the Person File reader and writer of the Data Entry Tool. Saved files are read
// back through the Library, which is what the Person Files are written for.

#include "person_file_document_tests.hpp"

#include <check.hpp>
#include <family_tree/api.h>
#include <person_file_document.hpp>
#include <support/test_support.hpp>

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

namespace {

using Strings = std::vector<std::string>;

// A Data Folder in the temporary folder of the operating system, removed again afterwards.
class DiskFolder {
public:
	DiskFolder()
		: myPath(std::filesystem::temp_directory_path() / "family_tree_data_entry_test") {
		std::filesystem::remove_all(myPath);
		std::filesystem::create_directories(myPath);
		Write("relations.csv", test::RelationsFile({}));
	}
	DiskFolder(const DiskFolder&) = delete;
	DiskFolder& operator=(const DiskFolder&) = delete;
	DiskFolder(DiskFolder&&) = delete;
	DiskFolder& operator=(DiskFolder&&) = delete;
	~DiskFolder() { std::filesystem::remove_all(myPath); }

	std::filesystem::path operator/(const std::string& aRelativePath) const {
		return myPath / std::filesystem::u8path(aRelativePath);
	}

	void Write(const std::string& aRelativePath, const std::string& aContent) const {
		const auto path = *this / aRelativePath;
		std::filesystem::create_directories(path.parent_path());
		std::ofstream(path, std::ios::binary) << aContent;
	}

	[[nodiscard]] std::string Read(const std::string& aRelativePath) const {
		const std::ifstream file(*this / aRelativePath, std::ios::binary);
		std::stringstream content;
		content << file.rdbuf();
		return content.str();
	}

	[[nodiscard]] std::string Path() const { return myPath.u8string(); }

private:
	std::filesystem::path myPath;
};

PersonFileDetails
Anna() {
	PersonFileDetails details;
	details.id = 19;
	details.firstNames = Strings{"Anna", "Maria"};
	details.titleOfNobility = "";
	details.lastNames = Strings{"Huber"};
	details.dateOfBirth = "um 1850";
	details.placeOfBirth = "Würzburg";
	details.professions = Strings{};
	details.placesOfResidence = std::vector<ResidenceEntry>{{"Würzburg", "1850", "1870"}};
	details.remarkLines = Strings{"Er wurde nur „der Bäcker“ genannt.", "", "Danach zog sie fort."};
	return details;
}

// docs/requirements/req_0032.md
bool
SavesTheCurrentFormatVersion() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	const auto text = disk.Read("19_Anna/data.json");
	CHECK(text.find("\"Version\"") != std::string::npos, true);
	CHECK(text.find(PersonFileDocument::CurrentFormatVersion()) != std::string::npos, true);
	CHECK(
		std::string(PersonFileDocument::CurrentFormatVersion()),
		std::string(test::thePersonFileVersion));
	return true;
}

// docs/requirements/req_0032.md, docs/features/ftr_0007.md#recording-a-person
bool
TheLibraryReadsEverythingThatWasEntered() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	test::ContextGuard context;
	test::TraceBuffer trace;
	CHECK(FT_Create(disk.Path().c_str(), &context, &trace), FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 19, &person, nullptr), FT_OK);
	CHECK(test::Texts(person->firstNames), Strings({"Anna", "Maria"}));
	CHECK(test::Texts(person->lastNames), Strings({"Huber"}));
	CHECK(test::Text(person->dateOfBirth), "um 1850");
	CHECK(test::Text(person->placeOfBirth), "Würzburg");
	CHECK(person->placesOfResidence.count, 1U);
	CHECK(test::Text(person->placesOfResidence.items[0].endDate), "1870");
	return true;
}

// docs/requirements/req_0003.md, docs/features/ftr_0007.md#remarks-with-several-lines
bool
WritesRemarksAsOneStringPerLine() {
	CHECK(PersonFileDocument::SplitIntoLines("erste\n\ndritte"), Strings({"erste", "", "dritte"}));
	CHECK(PersonFileDocument::JoinLines({"erste", "", "dritte"}), std::string("erste\n\ndritte"));
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	test::ContextGuard context;
	CHECK(FT_Create(disk.Path().c_str(), &context, nullptr), FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 19, &person, nullptr), FT_OK);
	CHECK(test::Texts(person->remarks), *Anna().remarkLines);
	return true;
}

// docs/requirements/req_0052.md, docs/requirements/req_0003.md, docs/requirements/req_0015.md
bool
LeavesOutWhatWasNotEntered() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	const auto text = disk.Read("19_Anna/data.json");
	CHECK(text.find("\"Title\""), std::string::npos);
	CHECK(text.find("\"DateOfDeath\""), std::string::npos);
	return true;
}

// docs/requirements/req_0052.md, docs/requirements/req_0050.md
bool
WritesADetailKnownNotToExist() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	const auto text = disk.Read("19_Anna/data.json");
	CHECK(text.find("\"TitleOfNobility\"") != std::string::npos, true);
	CHECK(text.find("\"Profession\"") != std::string::npos, true);
	test::ContextGuard context;
	CHECK(FT_Create(disk.Path().c_str(), &context, nullptr), FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 19, &person, nullptr), FT_OK);
	bool missing = true;
	CHECK(FT_IsDefaultString(context.Get(), person->titleOfNobility, &missing, nullptr), FT_OK);
	CHECK(missing, false);
	CHECK(person->professions.count, 0U);
	missing = true;
	CHECK(
		FT_IsDefaultString(context.Get(), person->professions.items[0], &missing, nullptr),
		FT_OK);
	CHECK(missing, false);
	missing = false;
	CHECK(FT_IsDefaultString(context.Get(), person->title, &missing, nullptr), FT_OK);
	CHECK(missing, true);
	return true;
}

// docs/requirements/req_0052.md, docs/requirements/req_0033.md
bool
ShowsAnEmptyDetailOfAReopenedFileAsKnownNotToExist() {
	const DiskFolder disk;
	disk.Write(
		"1_Anna/data.json",
		test::PersonFile(1, R"("TitleOfNobility": "", "Profession": [])"));
	auto document = PersonFileDocument::Open(disk / "1_Anna/data.json");
	CHECK(document.has_value(), true);
	CHECK(document->Details().titleOfNobility, Text(""));
	CHECK(document->Details().professions, TextList(Strings{}));
	CHECK(document->Details().title.has_value(), false);
	return true;
}

// docs/requirements/req_0033.md
bool
ReopeningAndSavingKeepsEveryDetail() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	auto reopened = PersonFileDocument::Open(disk / "19_Anna/data.json");
	CHECK(reopened.has_value(), true);
	CHECK(reopened->Details() == Anna(), true);
	CHECK(reopened->Save(disk / "copy/data.json"), true);
	auto again = PersonFileDocument::Open(disk / "copy/data.json");
	CHECK(again.has_value(), true);
	CHECK(again->Details() == Anna(), true);
	return true;
}

// docs/requirements/req_0033.md, docs/features/ftr_0008.md#keeping-unknown-keys
bool
ReopeningAndSavingKeepsUnknownKeys() {
	const DiskFolder disk;
	disk.Write(
		"1_Anna/data.json",
		test::PersonFile(1, R"("death": "11.11.2021", "FirstNames": ["Anna"])"));
	auto document = PersonFileDocument::Open(disk / "1_Anna/data.json");
	CHECK(document.has_value(), true);
	CHECK(document->Save(disk / "1_Anna/data.json"), true);
	const auto text = disk.Read("1_Anna/data.json");
	CHECK(text.find("\"death\"") != std::string::npos, true);
	CHECK(text.find("\"11.11.2021\"") != std::string::npos, true);
	return true;
}

// docs/requirements/req_0033.md, docs/features/ftr_0008.md#adding-a-date-of-death
bool
AddingADetailKeepsTheOthers() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "19_Anna/data.json"), true);
	auto reopened = PersonFileDocument::Open(disk / "19_Anna/data.json");
	CHECK(reopened.has_value(), true);
	auto details = reopened->Details();
	details.dateOfDeath = "1921";
	reopened->SetDetails(details);
	CHECK(reopened->Save(disk / "19_Anna/data.json"), true);
	auto changed = PersonFileDocument::Open(disk / "19_Anna/data.json");
	CHECK(changed.has_value(), true);
	auto expected = Anna();
	expected.dateOfDeath = "1921";
	CHECK(changed->Details() == expected, true);
	return true;
}

// docs/requirements/req_0033.md,
// docs/features/ftr_0008.md#a-person-file-of-an-unsupported-format-version
bool
RefusesAPersonFileOfAnotherFormatVersion() {
	const DiskFolder disk;
	disk.Write(
		"1_Anna/data.json",
		std::string(R"({"Version": ")") + test::theOldPersonFileVersion + R"(", "Id": 1})");
	CHECK(PersonFileDocument::Open(disk / "1_Anna/data.json").has_value(), false);
	return true;
}

// docs/requirements/req_0033.md
bool
RefusesAMalformedPersonFile() {
	const DiskFolder disk;
	disk.Write("1_Anna/data.json", "{\"Version\": ");
	CHECK(PersonFileDocument::Open(disk / "1_Anna/data.json").has_value(), false);
	CHECK(PersonFileDocument::Open(disk / "missing/data.json").has_value(), false);
	return true;
}

// docs/requirements/req_0034.md
bool
SavesWhereTheUserChose() {
	const DiskFolder disk;
	PersonFileDocument document;
	document.SetDetails(Anna());
	CHECK(document.Save(disk / "anywhere/deeper/data.json"), true);
	CHECK(std::filesystem::exists(disk / "anywhere/deeper/data.json"), true);
	return true;
}

} // namespace

bool
PersonFileDocumentSuite() {
	auto result = SUITE("PersonFileDocument");
	result &= RUN(SavesTheCurrentFormatVersion);
	result &= RUN(TheLibraryReadsEverythingThatWasEntered);
	result &= RUN(WritesRemarksAsOneStringPerLine);
	result &= RUN(LeavesOutWhatWasNotEntered);
	result &= RUN(WritesADetailKnownNotToExist);
	result &= RUN(ShowsAnEmptyDetailOfAReopenedFileAsKnownNotToExist);
	result &= RUN(ReopeningAndSavingKeepsEveryDetail);
	result &= RUN(ReopeningAndSavingKeepsUnknownKeys);
	result &= RUN(AddingADetailKeepsTheOthers);
	result &= RUN(RefusesAPersonFileOfAnotherFormatVersion);
	result &= RUN(RefusesAMalformedPersonFile);
	result &= RUN(SavesWhereTheUserChose);
	return result;
}
