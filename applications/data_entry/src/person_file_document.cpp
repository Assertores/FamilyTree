// Stubs until the work package of the Data Entry Tool implements the reading and writing with
// QJsonDocument. Only the comparison and the plain accessors are complete.

#include "person_file_document.hpp"

#include <tuple>

struct PersonFileDocument::Content {
	PersonFileDetails details;
};

bool
operator==(const ResidenceEntry& aLeft, const ResidenceEntry& aRight) {
	return std::tie(aLeft.name, aLeft.startDate, aLeft.endDate)
		   == std::tie(aRight.name, aRight.startDate, aRight.endDate);
}

bool
operator==(const PersonFileDetails& aLeft, const PersonFileDetails& aRight) {
	const auto fields = [](const PersonFileDetails& aDetails) {
		return std::tie(
			aDetails.id,
			aDetails.title,
			aDetails.firstNames,
			aDetails.titleOfNobility,
			aDetails.lastNames,
			aDetails.gender,
			aDetails.dateOfBirth,
			aDetails.placeOfBirth,
			aDetails.dateOfDeath,
			aDetails.placeOfDeath,
			aDetails.professions,
			aDetails.placesOfResidence,
			aDetails.recording,
			aDetails.pictures,
			aDetails.remarkLines);
	};
	return fields(aLeft) == fields(aRight);
}

const char*
PersonFileDocument::CurrentFormatVersion() {
	// docs/decisions/adr_0016.md
	return "1F10B769-08DE-442F-85BA-737B7F2B4803";
}

std::vector<std::string>
PersonFileDocument::SplitIntoLines(const std::string& /*aText*/) {
	return {};
}

// NOLINTNEXTLINE(modernize-use-string-view): a stub, until the lines are joined
std::string
PersonFileDocument::JoinLines(const std::vector<std::string>& /*aLines*/) {
	return {};
}

PersonFileDocument::PersonFileDocument()
	: myContent(std::make_shared<Content>()) {}

std::optional<PersonFileDocument>
PersonFileDocument::Open(const std::filesystem::path& /*aPath*/) {
	return std::nullopt;
}

const PersonFileDetails&
PersonFileDocument::Details() const {
	return myContent->details;
}

void
PersonFileDocument::SetDetails(const PersonFileDetails& aDetails) {
	myContent->details = aDetails;
}

bool
PersonFileDocument::Save(const std::filesystem::path& /*aPath*/) const {
	return false;
}
