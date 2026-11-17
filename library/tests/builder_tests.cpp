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

// docs/decisions/adr_0020.md
bool
AParsingQuestionNamesTheFileItIsAbout() {
	auto folder = ThreePersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	std::string lastRead;
	bool parsedPersonFile = false;
	bool parsedRelations = false;
	for (const auto& asked : folder.Asked()) {
		if (asked.type == FT_QUESTION_READ_FILE) {
			lastRead = asked.path;
		}
		const bool sameAsRead = !asked.path.empty() && asked.path == lastRead;
		parsedPersonFile |= asked.type == FT_QUESTION_PARSE_PERSON_FILE && sameAsRead
							&& asked.path.find("1_Anna/data.json") != std::string::npos;
		parsedRelations |= asked.type == FT_QUESTION_PARSE_RELATIONS_FILE && sameAsRead
						   && asked.path.find("relations.csv") != std::string::npos;
	}
	CHECK(parsedPersonFile, true);
	CHECK(parsedRelations, true);
	return true;
}

// docs/requirements/req_0037.md, docs/decisions/adr_0005.md
bool
UsesThePersonAnApplicationParsedItself() {
	const char* firstNames[] = {"Selbst"};
	FT_PersonRecord record{};
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

// docs/requirements/req_0050.md, docs/decisions/adr_0020.md
bool
AnAnswerTellsEmptyDetailsFromMissingOnes() {
	const char* noProfessions[] = {nullptr};
	FT_PersonRecord record{};
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

// Walks through the Questions of a Data Folder with the Person Folder "7_Selbst" and no Relations
// File, one answer at a time, so that a test can answer the Question to parse the Person File
// itself.
class StepByStep {
public:
	StepByStep() = default;
	StepByStep(const StepByStep&) = delete;
	StepByStep& operator=(const StepByStep&) = delete;
	StepByStep(StepByStep&&) = delete;
	StepByStep& operator=(StepByStep&&) = delete;
	~StepByStep() {
		if (myBuilder != nullptr && myQuestion.type != FT_QUESTION_NONE) {
			(void)FT_CancelCreate(myBuilder, nullptr);
		}
	}

	// Answers up to the Question to parse the Person File.
	FT_Result ToParsing(const std::string& aPersonFile) {
		myPersonFile = aPersonFile;
		FT_Result result = FT_StartCreate(theRoot, &myBuilder, &myQuestion, &myTrace);
		const char* folders[] = {"7_Selbst"};
		FT_Answer list{};
		list.folderNames = folders;
		list.folderCount = 1;
		if (result == FT_OK) {
			result = Continue(list);
		}
		FT_Answer read{};
		read.found = true;
		read.content = myPersonFile.c_str();
		if (result == FT_OK) {
			result = Continue(read);
		}
		return result;
	}

	// Answers up to the Question to parse the Relations File, leaving the Person File to the
	// Library.
	FT_Result ToParsingRelations(
		const std::string& aPersonFile,
		const std::string& aRelationsFile) {
		myRelationsFile = aRelationsFile;
		FT_Result result = ToParsing(aPersonFile);
		FT_Answer byDefault{};
		byDefault.useDefault = true;
		if (result == FT_OK) {
			result = Continue(byDefault);
		}
		FT_Answer read{};
		read.found = true;
		read.content = myRelationsFile.c_str();
		if (result == FT_OK) {
			result = Continue(read);
		}
		return result;
	}

	FT_Result Continue(const FT_Answer& aAnswer) {
		return FT_Continue(myBuilder, &aAnswer, &myQuestion, &myContext, &myTrace);
	}

	// Answers that there is no Relations File, which ends the creation with a warning.
	FT_Result Finish() {
		const FT_Answer missing{};
		return Continue(missing);
	}

	[[nodiscard]] FT_Question& Question() { return myQuestion; }
	[[nodiscard]] FT_Context* Context() const { return myContext.Get(); }
	[[nodiscard]] const TraceBuffer& Trace() const { return myTrace; }

private:
	std::string myPersonFile;
	std::string myRelationsFile;
	TraceBuffer myTrace;
	FT_Builder* myBuilder = nullptr;
	FT_Question myQuestion{};
	ContextGuard myContext;
};

// docs/decisions/adr_0020.md
bool
KeepsTheTextsAnApplicationParsedInPlace() {
	StepByStep steps;
	CHECK(steps.ToParsing(PersonFile(7)), FT_OK);
	CHECK(steps.Question().type, FT_QUESTION_PARSE_PERSON_FILE);
	// The Application decodes a first name into the text of the Question.
	char* text = steps.Question().text;
	const std::string_view name{"Selbst"};
	CHECK(std::char_traits<char>::length(text) > name.size(), true);
	name.copy(text, name.size());
	text[name.size()] = '\0';
	const char* firstNames[] = {text};
	FT_PersonRecord record{};
	record.person.id = 7;
	record.person.firstNames.items = firstNames;
	record.person.firstNames.count = 1;
	FT_Answer answer{};
	answer.person = &record;
	CHECK(steps.Continue(answer), FT_OK);
	CHECK(steps.Finish(), FT_OK_WITH_WARNINGS);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(steps.Context(), 7, &person, nullptr), FT_OK);
	CHECK(person->firstNames.items[0] == text, true);
	CHECK(Texts(person->firstNames), std::vector<std::string>{"Selbst"});
	return true;
}

// docs/decisions/adr_0020.md
bool
ARepeatedAnswerFindsTheTextUnchanged() {
	StepByStep steps;
	const std::string relations = RelationsFile({"7,7,Unrestricted,selbst"});
	CHECK(steps.ToParsingRelations(PersonFile(7), relations), FT_OK);
	CHECK(steps.Question().type, FT_QUESTION_PARSE_RELATIONS_FILE);
	// The Application changes the text, and then gives an answer that does not fit.
	steps.Question().text[0] = 'X';
	FT_RelationList withoutItems{};
	withoutItems.count = 1;
	FT_Answer broken{};
	broken.relations = &withoutItems;
	CHECK(steps.Continue(broken), FT_ERROR_UNEXPECTED_ANSWER);
	CHECK(std::string(steps.Question().text), relations);
	FT_Answer byDefault{};
	byDefault.useDefault = true;
	CHECK(steps.Continue(byDefault), FT_OK);
	const FT_Relation* found = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(steps.Context(), 7, &found, &count, nullptr), FT_OK);
	CHECK(count, 1U);
	CHECK(Text(found[0].relationship), "selbst");
	return true;
}

// docs/decisions/adr_0020.md
bool
AnApplicationCanSkipAFileItCannotParse() {
	StepByStep steps;
	CHECK(steps.ToParsing(PersonFile(7)), FT_OK);
	const FT_Answer withoutRecord{};
	CHECK(steps.Continue(withoutRecord), FT_OK_WITH_WARNINGS);
	CHECK(steps.Finish(), FT_OK_WITH_WARNINGS);
	size_t count = 0;
	CHECK(FT_GetPersonCount(steps.Context(), &count, nullptr), FT_OK);
	CHECK(count, 0U);
	CHECK(steps.Trace().HasWarning("7_Selbst"), true);
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

// docs/decisions/adr_0020.md
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

// docs/decisions/adr_0020.md
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
	result &= RUN(AParsingQuestionNamesTheFileItIsAbout);
	result &= RUN(UsesThePersonAnApplicationParsedItself);
	result &= RUN(AnAnswerTellsEmptyDetailsFromMissingOnes);
	result &= RUN(KeepsTheTextsAnApplicationParsedInPlace);
	result &= RUN(ARepeatedAnswerFindsTheTextUnchanged);
	result &= RUN(AnApplicationCanSkipAFileItCannotParse);
	result &= RUN(CancellingACreationReleasesTheBuilder);
	result &= RUN(ContinueWithoutAnswerIsAnInvalidArgument);
	result &= RUN(StartCreateWithoutDataFolderIsAnInvalidArgument);
	return result;
}
