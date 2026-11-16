// Unit tests of the Parsers: the content of Person Files and Relations Files, read through the
// Builder with the Library's own parsers.

#include "parsers_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>
#include <family_tree/api.h>

#include <cstddef>
#include <string>
#include <vector>

namespace {

using namespace test;

using Strings = std::vector<std::string>;

// Creates a Context with one Person File, and returns the Person.
struct OnePerson {
	DataFolder folder;
	ContextGuard context;
	TraceBuffer trace;
	FT_Result result = FT_ERROR_INVALID_ARGUMENT;
	const FT_Person* person = nullptr;

	explicit OnePerson(const std::string& aFurtherKeys)
		: folder(DataFolder().AddPerson("1_Anna", PersonFile(1, aFurtherKeys)).SetRelations({}))
		, result(Create(folder, context, trace)) {
		if (context) {
			(void)FT_GetPerson(context.Get(), 1, &person, nullptr);
		}
	}

	bool IsMissing(const char* aText) const {
		bool missing = false;
		return FT_IsDefaultString(context.Get(), aText, &missing, nullptr) == FT_OK && missing;
	}
};

// Creates a Context with Persons 1 to 3 and the given rows of the Relations File, and returns
// the Relations of Person 1.
struct RelationsOfOne {
	DataFolder folder;
	ContextGuard context;
	TraceBuffer trace;
	FT_Result result = FT_ERROR_INVALID_ARGUMENT;
	const FT_Relation* relations = nullptr;
	size_t count = 0;

	explicit RelationsOfOne(const Strings& aRows)
		: folder(
			  DataFolder()
				  .AddPerson("1", PersonFile(1))
				  .AddPerson("2", PersonFile(2))
				  .AddPerson("3", PersonFile(3))
				  .SetRelations(aRows))
		, result(Create(folder, context, trace)) {
		if (context) {
			(void)FT_GetRelations(context.Get(), 1, &relations, &count, nullptr);
		}
	}
};

// docs/requirements/req_0003.md, docs/requirements/req_0014.md
bool
ReadsEveryKeyOfAPersonFile() {
	const OnePerson one(R"("Title": "Prof. Dr.",
	"FirstNames": ["Philippa", "Rosa"],
	"TitleOfNobility": "von und zu",
	"LastNames": ["Perry", "Figueroa"],
	"Gender": "weiblich",
	"DateOfBirth": "04.05.1896",
	"PlaceOfBirth": "Würzburg",
	"DateOfDeath": "11.11.1961",
	"PlaceOfDeath": "Bamberg",
	"Profession": ["Bäckerin", "Lehrerin"],
	"PlaceOfResidence": [{"Name": "Würzburg", "StartDate": "1896", "EndDate": "1920"}],
	"Remarks": ["erste Zeile", "zweite Zeile"])");
	CHECK(one.result, FT_OK);
	CHECK(one.person != nullptr, true);
	const auto& person = *one.person;
	CHECK(person.id, 1);
	CHECK(Text(person.title), "Prof. Dr.");
	CHECK(Texts(person.firstNames), Strings({"Philippa", "Rosa"}));
	CHECK(Text(person.titleOfNobility), "von und zu");
	CHECK(Texts(person.lastNames), Strings({"Perry", "Figueroa"}));
	CHECK(Text(person.gender), "weiblich");
	CHECK(Text(person.dateOfBirth), "04.05.1896");
	CHECK(Text(person.placeOfBirth), "Würzburg");
	CHECK(Text(person.dateOfDeath), "11.11.1961");
	CHECK(Text(person.placeOfDeath), "Bamberg");
	CHECK(Texts(person.professions), Strings({"Bäckerin", "Lehrerin"}));
	CHECK(person.placesOfResidence.count, 1U);
	CHECK(Text(person.placesOfResidence.items[0].name), "Würzburg");
	CHECK(Text(person.placesOfResidence.items[0].startDate), "1896");
	CHECK(Text(person.placesOfResidence.items[0].endDate), "1920");
	CHECK(Texts(person.remarks), Strings({"erste Zeile", "zweite Zeile"}));
	return true;
}

// docs/requirements/req_0003.md
bool
ReadsSeveralPlacesOfResidenceInTheirOrder() {
	const OnePerson one(R"("PlaceOfResidence": [
		{"Name": "Würzburg", "StartDate": "1896"},
		{"Name": "Bamberg"}])");
	CHECK(one.result, FT_OK);
	CHECK(one.person->placesOfResidence.count, 2U);
	CHECK(Text(one.person->placesOfResidence.items[0].name), "Würzburg");
	CHECK(Text(one.person->placesOfResidence.items[1].name), "Bamberg");
	CHECK(one.IsMissing(one.person->placesOfResidence.items[0].endDate), true);
	CHECK(one.IsMissing(one.person->placesOfResidence.items[1].startDate), true);
	return true;
}

// docs/requirements/req_0003.md
bool
ReadsRemarksAsOneLinePerString() {
	const OnePerson one(
		R"("Remarks": ["Er wurde nur „der Bäcker“ genannt.", "", "Danach zog er fort."])");
	CHECK(one.result, FT_OK);
	CHECK(
		Texts(one.person->remarks),
		Strings({"Er wurde nur „der Bäcker“ genannt.", "", "Danach zog er fort."}));
	return true;
}

// docs/requirements/req_0003.md
bool
SkipsAndReportsAKeyOfTheWrongType() {
	const OnePerson one(R"("FirstNames": "Anna", "LastNames": ["Huber"])");
	CHECK(one.result, FT_OK_WITH_WARNINGS);
	CHECK(one.person != nullptr, true);
	CHECK(one.person->firstNames.count, 0U);
	CHECK(Texts(one.person->lastNames), Strings({"Huber"}));
	CHECK(one.trace.HasWarning("FirstNames"), true);
	return true;
}

// docs/requirements/req_0003.md
bool
SkipsATextKeyWithAListValue() {
	const OnePerson one(R"("Gender": ["weiblich"], "PlaceOfBirth": "Würzburg")");
	CHECK(one.result, FT_OK_WITH_WARNINGS);
	CHECK(one.IsMissing(one.person->gender), true);
	CHECK(Text(one.person->placeOfBirth), "Würzburg");
	return true;
}

// docs/requirements/req_0005.md
bool
IgnoresAndReportsAnUnknownKeyAndReadsTheRest() {
	const OnePerson one(R"("death": "11.11.2021", "PlaceOfDeath": "PlaceB")");
	CHECK(one.result, FT_OK_WITH_WARNINGS);
	CHECK(one.person != nullptr, true);
	CHECK(one.IsMissing(one.person->dateOfDeath), true);
	CHECK(Text(one.person->placeOfDeath), "PlaceB");
	CHECK(one.trace.HasWarning("death"), true);
	return true;
}

// docs/requirements/req_0005.md
bool
ReportsEveryUnknownKey() {
	const OnePerson one(R"("death": "1", "birth": "2")");
	CHECK(one.trace.HasWarning("death"), true);
	CHECK(one.trace.HasWarning("birth"), true);
	return true;
}

// docs/requirements/req_0007.md
bool
ReadsARowWithOnlyTheMandatoryColumns() {
	const RelationsOfOne one({"1,2,StrictlyLower"});
	CHECK(one.result, FT_OK);
	CHECK(one.count, 1U);
	CHECK(one.relations[0].first, 1);
	CHECK(one.relations[0].second, 2);
	CHECK(one.relations[0].type, FT_RELATION_STRICTLY_LOWER);
	bool missing = false;
	CHECK(
		FT_IsDefaultString(one.context.Get(), one.relations[0].relationship, &missing, nullptr),
		FT_OK);
	CHECK(missing, true);
	return true;
}

// docs/requirements/req_0007.md, docs/requirements/req_0009.md
bool
ReadsTheOptionalColumnsAsWritten() {
	const RelationsOfOne one({"1,2,Unrestricted,verheiratet,um 1920,1945"});
	CHECK(one.result, FT_OK);
	CHECK(one.count, 1U);
	CHECK(Text(one.relations[0].relationship), "verheiratet");
	CHECK(Text(one.relations[0].startDate), "um 1920");
	CHECK(Text(one.relations[0].endDate), "1945");
	return true;
}

// docs/requirements/req_0007.md
bool
ReadsAQuotedFieldWithAComma() {
	const RelationsOfOne one({R"(1,2,Unrestricted,"Stiefvater, später Pate",,)"});
	CHECK(one.result, FT_OK);
	CHECK(one.count, 1U);
	CHECK(Text(one.relations[0].relationship), "Stiefvater, später Pate");
	return true;
}

// docs/requirements/req_0007.md
bool
ReadsADoubledQuotationMarkInAQuotedField() {
	const RelationsOfOne one({R"(1,2,Unrestricted,"genannt ""Onkel""",,)"});
	CHECK(one.result, FT_OK);
	CHECK(one.count, 1U);
	CHECK(Text(one.relations[0].relationship), "genannt \"Onkel\"");
	return true;
}

// docs/requirements/req_0008.md
bool
ReadsEveryRelationType() {
	const RelationsOfOne one({"1,2,Unrestricted", "1,3,StrictlyHigher", "2,3,StrictlyLower"});
	CHECK(one.result, FT_OK);
	CHECK(one.count, 2U);
	for (size_t i = 0; i < one.count; ++i) {
		const auto& relation = one.relations[i];
		if (relation.second == 2) {
			CHECK(relation.type, FT_RELATION_UNRESTRICTED);
		} else {
			CHECK(relation.type, FT_RELATION_STRICTLY_HIGHER);
		}
	}
	return true;
}

// docs/requirements/req_0008.md
bool
ReadsAnUnknownRelationTypeAsUnrestrictedWithAWarning() {
	const RelationsOfOne one({"1,2,Child"});
	CHECK(one.result, FT_OK_WITH_WARNINGS);
	CHECK(one.count, 1U);
	CHECK(one.relations[0].type, FT_RELATION_UNRESTRICTED);
	CHECK(one.trace.HasWarning("Child"), true);
	return true;
}

// docs/requirements/req_0008.md
bool
ReadsAnEmptyRelationTypeAsUnrestrictedWithAWarning() {
	const RelationsOfOne one({"1,2,,Kind,,"});
	CHECK(one.result, FT_OK_WITH_WARNINGS);
	CHECK(one.count, 1U);
	CHECK(one.relations[0].type, FT_RELATION_UNRESTRICTED);
	CHECK(one.trace.HasWarning(), true);
	return true;
}

// docs/requirements/req_0009.md
bool
KeepsDatesAndPlacesExactlyAsWritten() {
	const OnePerson one(R"("DateOfBirth": "um 1850", "PlaceOfBirth": "  bei Würzburg (?) ")");
	CHECK(one.result, FT_OK);
	CHECK(Text(one.person->dateOfBirth), "um 1850");
	CHECK(Text(one.person->placeOfBirth), "  bei Würzburg (?) ");
	return true;
}

// docs/requirements/req_0009.md
bool
DecodesTheEscapeSequencesOfJson() {
	const OnePerson one(
		R"("Remarks": ["genannt \"der Bäcker\"", "C:\\Bilder", "Tab\there", "\u00e4\u00f6\u00fc\u00df"])");
	CHECK(one.result, FT_OK);
	CHECK(
		Texts(one.person->remarks),
		Strings({"genannt \"der Bäcker\"", "C:\\Bilder", "Tab\there", "äöüß"}));
	return true;
}

// docs/requirements/req_0009.md, docs/requirements/req_0010.md
bool
DecodesASurrogatePairIntoUtf8() {
	const OnePerson one(R"("Remarks": ["\ud83c\udf82"])");
	CHECK(one.result, FT_OK);
	CHECK(Texts(one.person->remarks), Strings({"\xF0\x9F\x8E\x82"}));
	return true;
}

// docs/requirements/req_0054.md, docs/requirements/req_0009.md
bool
KeepsAByteOrderMarkInsideAText() {
	const OnePerson one("\"PlaceOfBirth\": \"\xEF\xBB\xBFW\xC3\xBCrzburg\"");
	CHECK(one.result, FT_OK);
	CHECK(Text(one.person->placeOfBirth), "\xEF\xBB\xBFW\xC3\xBCrzburg");
	return true;
}

// docs/requirements/req_0010.md
bool
ReturnsUtf8TextUnchanged() {
	const OnePerson one(R"("PlaceOfBirth": "Łódź", "LastNames": ["Müller-Šimić"])");
	CHECK(one.result, FT_OK);
	CHECK(Text(one.person->placeOfBirth), "Łódź");
	CHECK(Texts(one.person->lastNames), Strings({"Müller-Šimić"}));
	return true;
}

} // namespace

bool
ParsersSuite() {
	auto result = SUITE("Parsers");
	result &= RUN(ReadsEveryKeyOfAPersonFile);
	result &= RUN(ReadsSeveralPlacesOfResidenceInTheirOrder);
	result &= RUN(ReadsRemarksAsOneLinePerString);
	result &= RUN(SkipsAndReportsAKeyOfTheWrongType);
	result &= RUN(SkipsATextKeyWithAListValue);
	result &= RUN(IgnoresAndReportsAnUnknownKeyAndReadsTheRest);
	result &= RUN(ReportsEveryUnknownKey);
	result &= RUN(ReadsARowWithOnlyTheMandatoryColumns);
	result &= RUN(ReadsTheOptionalColumnsAsWritten);
	result &= RUN(ReadsAQuotedFieldWithAComma);
	result &= RUN(ReadsADoubledQuotationMarkInAQuotedField);
	result &= RUN(ReadsEveryRelationType);
	result &= RUN(ReadsAnUnknownRelationTypeAsUnrestrictedWithAWarning);
	result &= RUN(ReadsAnEmptyRelationTypeAsUnrestrictedWithAWarning);
	result &= RUN(KeepsDatesAndPlacesExactlyAsWritten);
	result &= RUN(DecodesTheEscapeSequencesOfJson);
	result &= RUN(DecodesASurrogatePairIntoUtf8);
	result &= RUN(KeepsAByteOrderMarkInsideAText);
	result &= RUN(ReturnsUtf8TextUnchanged);
	return result;
}
