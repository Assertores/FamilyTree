#pragma once

// The Person File reader and writer of the Data Entry Tool (docs/architecture.md). Declarations
// only, invented while writing the tests of step 5; see docs/workspace/interface_draft.md. The
// implementation uses QJsonDocument, but the interface uses standard types only, so that it can
// be tested without Qt.

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct ResidenceEntry {
	std::string name;
	std::string startDate;
	std::string endDate;
};

// The Details as the form shows them. An empty text or an empty list is not written into the
// Person File, so the Detail is missing there.
struct PersonFileDetails {
	std::optional<long long> id;
	std::string title;
	std::vector<std::string> firstNames;
	std::string titleOfNobility;
	std::vector<std::string> lastNames;
	std::string gender;
	std::string dateOfBirth;
	std::string placeOfBirth;
	std::string dateOfDeath;
	std::string placeOfDeath;
	std::vector<std::string> professions;
	std::vector<ResidenceEntry> placesOfResidence;
	std::string recording;
	std::vector<std::string> pictures;
	std::vector<std::string> remarkLines;
};

bool operator==(const ResidenceEntry& aLeft, const ResidenceEntry& aRight);
bool operator==(const PersonFileDetails& aLeft, const PersonFileDetails& aRight);

class PersonFileDocument {
public:
	// The Format Version the Data Entry Tool writes, and the only one it opens.
	static const char* CurrentFormatVersion();

	// Splits the text of a multi line field into lines, and joins them again.
	static std::vector<std::string> SplitIntoLines(const std::string& aText);
	static std::string JoinLines(const std::vector<std::string>& aLines);

	// An empty document, for a new Person.
	PersonFileDocument();

	// Opens a Person File. Fails if it is malformed, or has another Format Version.
	static std::optional<PersonFileDocument> Open(const std::filesystem::path& aPath);

	[[nodiscard]] const PersonFileDetails& Details() const;
	void SetDetails(const PersonFileDetails& aDetails);

	// Writes the Person File in the current Format Version, with every key of the opened file
	// the Data Entry Tool does not know. Creates missing folders. Returns false if writing failed.
	[[nodiscard]] bool Save(const std::filesystem::path& aPath) const;

private:
	struct Content;
	std::shared_ptr<Content> myContent;
};
