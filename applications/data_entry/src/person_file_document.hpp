#pragma once

// The Person File reader and writer of the Data Entry Tool (docs/architecture.md). The
// implementation uses QJsonDocument, but the interface uses standard types only, so that it can
// be tested without the form.

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

using Text = std::optional<std::string>;
using TextList = std::optional<std::vector<std::string>>;

// The Details as the form shows them (docs/requirements/req_0052.md): std::nullopt is a Detail
// that was not entered, and is left out of the Person File; an empty text or list is a Detail
// known not to exist, and is written as such.
struct PersonFileDetails {
	std::optional<long long> id;
	Text title;
	TextList firstNames;
	Text titleOfNobility;
	TextList lastNames;
	Text gender;
	Text dateOfBirth;
	Text placeOfBirth;
	Text dateOfDeath;
	Text placeOfDeath;
	TextList professions;
	std::optional<std::vector<ResidenceEntry>> placesOfResidence;
	TextList remarkLines;
	// References to the Media; empty means none.
	std::string recording;
	std::vector<std::string> pictures;
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
