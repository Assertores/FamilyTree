// Unit tests of the Builder: reading a Data Folder into a Context through Questions.

#include "builder_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>
#include <family_tree/api.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace test;

size_t
PersonCount(const ContextGuard& aContext) {
	size_t count = 0;
	if (FT_GetPersonCount(aContext.Get(), &count, nullptr) != FT_OK) {
		return static_cast<size_t>(-1);
	}
	return count;
}

bool
HasPerson(const ContextGuard& aContext, FT_PersonId aId) {
	const FT_Person* person = nullptr;
	return FT_GetPerson(aContext.Get(), aId, &person, nullptr) == FT_OK && person != nullptr;
}

DataFolder
ThreePersons() {
	DataFolder folder;
	folder.AddPerson("1_Anna", PersonFile(1))
		.AddPerson("2_Bernd", PersonFile(2))
		.AddPerson("3_Clara", PersonFile(3))
		.SetRelations({});
	return folder;
}

// docs/requirements/req_0001.md
bool
ReadsThePersonFileOfEveryPersonFolder() {
	auto folder = ThreePersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	CHECK(PersonCount(context), 3U);
	CHECK(HasPerson(context, 1), true);
	CHECK(HasPerson(context, 2), true);
	CHECK(HasPerson(context, 3), true);
	return true;
}

// docs/requirements/req_0001.md
bool
SkipsAndReportsAPersonFileWithAnUnsupportedFormatVersion() {
	auto folder = ThreePersons();
	folder.AddPerson(
		"3_Clara",
		std::string(R"({"Version": ")") + theOldPersonFileVersion + R"(", "Id": 3})");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	CHECK(HasPerson(context, 3), false);
	CHECK(trace.HasWarning("3_Clara"), true);
	return true;
}

// docs/requirements/req_0001.md
bool
OnlyReadsTheFileNamedDataJson() {
	auto folder = ThreePersons();
	folder.AddFile("1_Anna/data_extra.json", PersonFile(4));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK, FT_OK_WITH_WARNINGS);
	CHECK(HasPerson(context, 4), false);
	return true;
}

// docs/requirements/req_0002.md, docs/requirements/req_0013.md
bool
SkipsAPersonFileWithoutVersion() {
	auto folder = ThreePersons();
	folder.AddPerson("3_Clara", "{\"Id\": 3}");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(HasPerson(context, 3), false);
	CHECK(PersonCount(context), 2U);
	return true;
}

// docs/requirements/req_0002.md, docs/requirements/req_0013.md
bool
SkipsAPersonFileWithoutId() {
	auto folder = ThreePersons();
	folder.AddPerson("3_Clara", std::string(R"({"Version": ")") + thePersonFileVersion + "\"}");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	CHECK(trace.HasWarning("3_Clara"), true);
	return true;
}

// docs/requirements/req_0002.md, docs/requirements/req_0013.md
bool
SkipsAPersonFileWhoseIdIsNoInteger() {
	auto folder = ThreePersons();
	folder.AddPerson(
		"3_Clara",
		std::string(R"({"Version": ")") + thePersonFileVersion + R"(", "Id": "three"})");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	return true;
}

// docs/requirements/req_0006.md
bool
ReadsARelationsFileWithASupportedFormatVersion() {
	auto folder = ThreePersons();
	folder.SetRelations({"1,2,StrictlyLower,Kind,,"});
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	CHECK(relations[0].second, 2);
	return true;
}

// docs/requirements/req_0006.md
bool
IgnoresTheSecondLineOfTheRelationsFile() {
	auto folder = ThreePersons();
	folder.AddFile(
		"relations.csv",
		std::string("Version,") + theRelationsFileVersion
			+ "\n1,2,StrictlyLower,,,\n2,3,StrictlyLower,,,\n");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 0U);
	CHECK(FT_GetRelations(context.Get(), 3, &relations, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	return true;
}

// docs/requirements/req_0006.md
bool
SkipsAndReportsARelationsFileWithAnUnsupportedFormatVersion() {
	auto folder = ThreePersons();
	folder.AddFile(
		"relations.csv",
		"Version,3B5589D2-D9AF-40A8-BC40-574DAB6FFC57\nheader\n1,2,StrictlyLower,,,\n");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 3U);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 0U);
	CHECK(trace.HasWarning("relations.csv"), true);
	return true;
}

// docs/requirements/req_0006.md
bool
SkipsARelationsFileWithoutVersionLine() {
	auto folder = ThreePersons();
	folder.AddFile(
		"relations.csv",
		"personA,personB,type,relationship,start,end\n1,2,StrictlyLower,,,\n");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 0U);
	return true;
}

// docs/requirements/req_0013.md
bool
SkipsAMalformedPersonFileAndReadsTheOthers() {
	auto folder = ThreePersons();
	folder.AddPerson(
		"3_Clara",
		std::string(R"({"Version": ")") + thePersonFileVersion + R"(", "Id": 3)");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	CHECK(trace.HasWarning("3_Clara"), true);
	return true;
}

// docs/requirements/req_0013.md
bool
ReportsAFolderWithoutPersonFile() {
	auto folder = ThreePersons();
	folder.AddFile("Bilder/a.png", "");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 3U);
	CHECK(trace.HasWarning("Bilder"), true);
	return true;
}

// docs/requirements/req_0013.md
bool
SkipsAnEmptyPersonFile() {
	auto folder = ThreePersons();
	folder.AddPerson("3_Clara", "");
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	return true;
}

// docs/requirements/req_0013.md, docs/requirements/req_0007.md
bool
SkipsARelationRowWithTooFewColumns() {
	auto folder = ThreePersons();
	folder.SetRelations({"1,2", "2,3,StrictlyLower,,,"});
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 0U);
	CHECK(FT_GetRelations(context.Get(), 3, &relations, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	CHECK(trace.HasWarning("relations.csv"), true);
	return true;
}

// docs/requirements/req_0051.md, docs/requirements/req_0010.md
bool
SkipsOnlyAValueThatIsNotUtf8() {
	auto folder = ThreePersons();
	folder.AddPerson(
		"3_Clara",
		PersonFile(3, "\"PlaceOfBirth\": \"W\xFCrzburg\", \"PlaceOfDeath\": \"Bamberg\""));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(trace.HasWarning("3_Clara"), true);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 3, &person, nullptr), FT_OK);
	bool missing = false;
	CHECK(FT_IsDefaultString(context.Get(), person->placeOfBirth, &missing, nullptr), FT_OK);
	CHECK(missing, true);
	CHECK(Text(person->placeOfDeath), "Bamberg");
	return true;
}

// docs/requirements/req_0051.md
bool
SkipsOnlyARelationRowThatIsNotUtf8() {
	auto folder = ThreePersons();
	folder.SetRelations({"1,2,StrictlyLower,M\xFCtter,,", "2,3,StrictlyLower,,,"});
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(trace.HasWarning("relations.csv"), true);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 0U);
	CHECK(FT_GetRelations(context.Get(), 3, &relations, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	return true;
}

// docs/requirements/req_0049.md
bool
ADataFolderWithoutRelationsFileHasNoRelations() {
	DataFolder folder;
	folder.AddPerson("1_Anna", PersonFile(1)).AddPerson("2_Bernd", PersonFile(2));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	CHECK(trace.HasWarning("relations.csv"), true);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 0U);
	return true;
}

// docs/requirements/req_0053.md
bool
KeepsThePersonFileOfTheFirstPersonFolderOfADuplicatePersonId() {
	DataFolder folder;
	folder.AddPerson("1_Anna", PersonFile(1, R"("FirstNames": ["Anna"])"))
		.AddPerson("2_Bernd", PersonFile(1, R"("FirstNames": ["Bernd"])"))
		.SetRelations({})
		.ListFoldersInReverse();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 1U);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 1, &person, nullptr), FT_OK);
	CHECK(Texts(person->firstNames), std::vector<std::string>{"Anna"});
	CHECK(trace.HasWarning("2_Bernd"), true);
	return true;
}

// docs/requirements/req_0053.md
bool
SkipsAndReportsEveryFurtherPersonFileWithTheSamePersonId() {
	DataFolder folder;
	folder.AddPerson("1_Anna", PersonFile(1, R"("FirstNames": ["Anna"])"))
		.AddPerson("2_Bernd", PersonFile(1, R"("FirstNames": ["Bernd"])"))
		.AddPerson("3_Clara", PersonFile(1, R"("FirstNames": ["Clara"])"))
		.AddPerson("4_Dora", PersonFile(4))
		.SetRelations({});
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	CHECK(HasPerson(context, 4), true);
	CHECK(trace.HasWarning("2_Bernd"), true);
	CHECK(trace.HasWarning("3_Clara"), true);
	return true;
}

// docs/requirements/req_0053.md
bool
ComparesPersonFolderNamesByTheirBytes() {
	// In byte order, upper case letters come before lower case ones, and both before "Ä".
	DataFolder folder;
	folder.AddPerson("anna", PersonFile(1, R"("FirstNames": ["anna"])"))
		.AddPerson("Bernd", PersonFile(1, R"("FirstNames": ["Bernd"])"))
		.AddPerson("Änne", PersonFile(1, R"("FirstNames": ["Änne"])"))
		.SetRelations({})
		.ListFoldersInReverse();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 1, &person, nullptr), FT_OK);
	CHECK(Texts(person->firstNames), std::vector<std::string>{"Bernd"});
	return true;
}

// The UTF-8 byte order mark, and the start of a file in UTF-16 with its byte order mark. A text in
// UTF-16 holds zero bytes, so an answer to READ_FILE ends after its first character.
constexpr std::string_view theUtf8ByteOrderMark{"\xEF\xBB\xBF"};
constexpr std::string_view theUtf16Start{"\xFF\xFE{\0", 4};

// docs/requirements/req_0054.md
bool
ReadsAPersonFileThatStartsWithAByteOrderMark() {
	auto folder = ThreePersons();
	folder.AddPerson("3_Clara", std::string(theUtf8ByteOrderMark) + PersonFile(3));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	CHECK(PersonCount(context), 3U);
	CHECK(HasPerson(context, 3), true);
	return true;
}

// docs/requirements/req_0054.md
bool
ReadsARelationsFileThatStartsWithAByteOrderMark() {
	auto folder = ThreePersons();
	folder.AddFile(
		"relations.csv",
		std::string(theUtf8ByteOrderMark) + RelationsFile({"1,2,StrictlyLower"}));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	return true;
}

// docs/requirements/req_0010.md, docs/requirements/req_0013.md
bool
SkipsAPersonFileInUtf16() {
	auto folder = ThreePersons();
	folder.AddPerson("3_Clara", std::string(theUtf16Start));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 2U);
	CHECK(trace.HasWarning("3_Clara"), true);
	return true;
}

// docs/requirements/req_0010.md, docs/requirements/req_0006.md
bool
SkipsARelationsFileInUtf16() {
	auto folder = ThreePersons();
	folder.AddFile("relations.csv", std::string("\xFF\xFEV\0", 4));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(PersonCount(context), 3U);
	CHECK(trace.HasWarning("relations.csv"), true);
	return true;
}

// docs/requirements/req_0037.md
bool
ReadsTheDataFolderOnlyThroughQuestions() {
	auto folder = ThreePersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	bool listedRoot = false;
	bool readRelations = false;
	bool readPersonFile = false;
	for (const auto& asked : folder.Asked()) {
		listedRoot |= asked.type == FT_QUESTION_LIST_FOLDERS && asked.path == theRoot;
		readRelations |= asked.type == FT_QUESTION_READ_FILE
						 && asked.path.find("relations.csv") != std::string::npos;
		readPersonFile |=
			asked.type == FT_QUESTION_READ_FILE && asked.path.find("1_Anna") != std::string::npos;
	}
	CHECK(listedRoot, true);
	CHECK(readRelations, true);
	CHECK(readPersonFile, true);
	return true;
}

// docs/requirements/req_0037.md, docs/decisions/adr_0005.md
bool
UsesThePersonAnApplicationParsedItself() {
	const char* firstNames[] = {"Selbst"};
	FT_PersonRecord record{};
	record.formatVersion = thePersonFileVersion;
	record.person.id = 7;
	record.person.firstNames.items = firstNames;
	record.person.firstNames.count = 1;
	DataFolder folder;
	folder.AddPerson("7_Selbst", "this text is never parsed by the Library").SetRelations({});
	folder.AnswerPersonParsing(record);
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 7, &person, nullptr), FT_OK);
	CHECK(Texts(person->firstNames), std::vector<std::string>{"Selbst"});
	bool missing = false;
	CHECK(FT_IsDefaultString(context.Get(), person->title, &missing, nullptr), FT_OK);
	CHECK(missing, true);
	return true;
}

// docs/requirements/req_0050.md, docs/decisions/adr_0017.md
bool
AnAnswerTellsEmptyDetailsFromMissingOnes() {
	const char* noProfessions[] = {nullptr};
	FT_PersonRecord record{};
	record.formatVersion = thePersonFileVersion;
	record.person.id = 7;
	record.person.titleOfNobility = "";
	record.person.professions.items = noProfessions;
	record.person.professions.count = 0;
	DataFolder folder;
	folder.AddPerson("7_Selbst", "this text is never parsed by the Library").SetRelations({});
	folder.AnswerPersonParsing(record);
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(context.Get(), 7, &person, nullptr), FT_OK);
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
	missing = false;
	CHECK(FT_IsDefaultString(context.Get(), person->lastNames.items[0], &missing, nullptr), FT_OK);
	CHECK(missing, true);
	return true;
}

// docs/decisions/adr_0005.md
bool
CancellingACreationReleasesTheBuilder() {
	FT_Builder* builder = nullptr;
	FT_Question question{};
	CHECK(FT_StartCreate(theRoot, &builder, &question, nullptr), FT_OK);
	CHECK(question.type == FT_QUESTION_NONE, false);
	CHECK(FT_CancelCreate(builder, nullptr), FT_OK);
	return true;
}

// docs/decisions/adr_0017.md
bool
ContinueWithoutAnswerIsAnInvalidArgument() {
	FT_Builder* builder = nullptr;
	FT_Question question{};
	CHECK(FT_StartCreate(theRoot, &builder, &question, nullptr), FT_OK);
	FT_Context* context = nullptr;
	CHECK(FT_Continue(builder, nullptr, &question, &context, nullptr), FT_ERROR_INVALID_ARGUMENT);
	CHECK(FT_CancelCreate(builder, nullptr), FT_OK);
	return true;
}

// docs/decisions/adr_0017.md
bool
StartCreateWithoutDataFolderIsAnInvalidArgument() {
	FT_Builder* builder = nullptr;
	FT_Question question{};
	CHECK(FT_StartCreate(nullptr, &builder, &question, nullptr), FT_ERROR_INVALID_ARGUMENT);
	return true;
}

} // namespace

bool
BuilderSuite() {
	auto result = SUITE("Builder");
	result &= RUN(ReadsThePersonFileOfEveryPersonFolder);
	result &= RUN(SkipsAndReportsAPersonFileWithAnUnsupportedFormatVersion);
	result &= RUN(OnlyReadsTheFileNamedDataJson);
	result &= RUN(SkipsAPersonFileWithoutVersion);
	result &= RUN(SkipsAPersonFileWithoutId);
	result &= RUN(SkipsAPersonFileWhoseIdIsNoInteger);
	result &= RUN(ReadsARelationsFileWithASupportedFormatVersion);
	result &= RUN(IgnoresTheSecondLineOfTheRelationsFile);
	result &= RUN(SkipsAndReportsARelationsFileWithAnUnsupportedFormatVersion);
	result &= RUN(SkipsARelationsFileWithoutVersionLine);
	result &= RUN(SkipsAMalformedPersonFileAndReadsTheOthers);
	result &= RUN(ReportsAFolderWithoutPersonFile);
	result &= RUN(SkipsAnEmptyPersonFile);
	result &= RUN(SkipsARelationRowWithTooFewColumns);
	result &= RUN(SkipsOnlyAValueThatIsNotUtf8);
	result &= RUN(SkipsOnlyARelationRowThatIsNotUtf8);
	result &= RUN(ADataFolderWithoutRelationsFileHasNoRelations);
	result &= RUN(KeepsThePersonFileOfTheFirstPersonFolderOfADuplicatePersonId);
	result &= RUN(SkipsAndReportsEveryFurtherPersonFileWithTheSamePersonId);
	result &= RUN(ComparesPersonFolderNamesByTheirBytes);
	result &= RUN(ReadsAPersonFileThatStartsWithAByteOrderMark);
	result &= RUN(ReadsARelationsFileThatStartsWithAByteOrderMark);
	result &= RUN(SkipsAPersonFileInUtf16);
	result &= RUN(SkipsARelationsFileInUtf16);
	result &= RUN(ReadsTheDataFolderOnlyThroughQuestions);
	result &= RUN(UsesThePersonAnApplicationParsedItself);
	result &= RUN(AnAnswerTellsEmptyDetailsFromMissingOnes);
	result &= RUN(CancellingACreationReleasesTheBuilder);
	result &= RUN(ContinueWithoutAnswerIsAnInvalidArgument);
	result &= RUN(StartCreateWithoutDataFolderIsAnInvalidArgument);
	return result;
}
