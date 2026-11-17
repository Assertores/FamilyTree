#pragma once

// Helpers shared by the unit tests of the Library and the Data Entry Tool; test code only, see
// docs/conventions.md.

#include <family_tree/api.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace test {

// The Format Versions the Library supports (docs/decisions/adr_0016.md).
inline constexpr const char* thePersonFileVersion = "1F10B769-08DE-442F-85BA-737B7F2B4803";
inline constexpr const char* theRelationsFileVersion = "227E82BA-6603-426C-9C95-5BC86A10A642";
// The Format Version of the Person Files written before "Remarks" became a list of lines; no
// longer supported.
inline constexpr const char* theOldPersonFileVersion = "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57";

inline constexpr const char* theRoot = "family";

// A Person File with the supported Format Version, the given Person Id, and the given further
// keys, e.g. R"("FirstNames": ["Anna"])".
inline std::string
PersonFile(FT_PersonId aId, const std::string& aFurtherKeys = "") {
	std::string result = "{\n\t\"Version\": \"" + std::string(thePersonFileVersion) + "\",\n";
	result += "\t\"Id\": " + std::to_string(aId);
	if (!aFurtherKeys.empty()) {
		result += ",\n\t" + aFurtherKeys;
	}
	return result + "\n}\n";
}

// A Relations File with the supported Format Version, the header line, and the given rows.
inline std::string
RelationsFile(const std::vector<std::string>& aRows) {
	std::string result = "Version," + std::string(theRelationsFileVersion) + "\n";
	result += "personA,personB,type,relationship,start,end\n";
	for (const auto& row : aRows) {
		result += row + "\n";
	}
	return result;
}

// A buffer for the Trace, and helpers to read it.
class TraceBuffer {
public:
	explicit TraceBuffer(size_t aCapacity = 1U << 20U)
		: myBuffer(aCapacity, '\0') {
		myTrace.buffer = myBuffer.data();
		myTrace.capacity = myBuffer.size();
	}
	TraceBuffer(const TraceBuffer&) = delete;
	TraceBuffer& operator=(const TraceBuffer&) = delete;
	TraceBuffer(TraceBuffer&&) = delete;
	TraceBuffer& operator=(TraceBuffer&&) = delete;
	~TraceBuffer() = default;

	FT_Trace* operator&() { return &myTrace; } // NOLINT(google-runtime-operator)
	[[nodiscard]] const FT_Trace& Get() const { return myTrace; }

	[[nodiscard]] std::string Text() const { return {myTrace.buffer, myTrace.used}; }

	[[nodiscard]] std::vector<std::string> Lines() const {
		std::vector<std::string> lines;
		std::istringstream stream(Text());
		for (std::string line; std::getline(stream, line);) {
			lines.push_back(line);
		}
		return lines;
	}

	// Whether a line holds the given level (STEP, WARNING or ERROR) and contains the given text.
	[[nodiscard]] bool Has(const std::string& aLevel, const std::string& aText) const {
		const auto lines = Lines();
		return std::any_of(lines.begin(), lines.end(), [&](const std::string& aLine) {
			const auto start = aLine.find_first_not_of('\t');
			return start != std::string::npos
				   && aLine.compare(start, aLevel.size() + 1, aLevel + "\t") == 0
				   && aLine.find(aText) != std::string::npos;
		});
	}

	[[nodiscard]] bool HasWarning(const std::string& aText = "") const {
		return Has("WARNING", aText);
	}
	[[nodiscard]] bool HasError(const std::string& aText = "") const { return Has("ERROR", aText); }

private:
	std::vector<char> myBuffer;
	FT_Trace myTrace{};
};

// Frees a Context when it goes out of scope.
class ContextGuard {
public:
	ContextGuard() = default;
	ContextGuard(const ContextGuard&) = delete;
	ContextGuard& operator=(const ContextGuard&) = delete;
	ContextGuard(ContextGuard&&) = delete;
	ContextGuard& operator=(ContextGuard&&) = delete;
	~ContextGuard() {
		if (myContext != nullptr) {
			(void)FT_Free(myContext, nullptr);
		}
	}

	FT_Context** operator&() { return &myContext; } // NOLINT(google-runtime-operator)
	[[nodiscard]] FT_Context* Get() const { return myContext; }
	explicit operator bool() const { return myContext != nullptr; }

private:
	FT_Context* myContext = nullptr;
};

// One Question the Library asked, as recorded by DataFolder.
struct AskedQuestion {
	FT_QuestionType type = FT_QUESTION_NONE;
	std::string path;
};

// A Data Folder that only exists in memory. It answers the Questions of the Library from its
// files, and leaves the parsing Questions to the Library, unless a test answers them itself.
// Moving it may throw with the standard library of MSVC, whose std::map allocates when it is
// moved; a test that runs out of memory there may end the run.
// NOLINTNEXTLINE(bugprone-exception-escape)
class DataFolder {
public:
	// Adds a file, given by its path relative to the Data Folder, e.g. "1_Anna/data.json".
	DataFolder& AddFile(const std::string& aRelativePath, const std::string& aContent) {
		myFiles[PrivNormalize(std::string(theRoot) + "/" + aRelativePath)] = aContent;
		return *this;
	}

	DataFolder& AddPerson(const std::string& aFolder, const std::string& aPersonFile) {
		return AddFile(aFolder + "/data.json", aPersonFile);
	}

	DataFolder& SetRelations(const std::vector<std::string>& aRows) {
		return AddFile("relations.csv", RelationsFile(aRows));
	}

	// The answer given to every Question to parse a Person File, instead of the Library's parser.
	// Lists folders in reverse byte order, as an operating system may list them in any order.
	DataFolder& ListFoldersInReverse() {
		myReverse = true;
		return *this;
	}

	DataFolder& AnswerPersonParsing(const FT_PersonRecord& aRecord) {
		myPersonRecord = aRecord;
		return *this;
	}

	// Runs the Questions until the Context is ready, or creation failed. Returns the result of the
	// last call, which covers the whole creation, and cancels the creation after an error, which
	// leaves the Builder to the Application (docs/decisions/adr_0016.md).
	FT_Result Create(FT_Context** aOutContext, FT_Trace* aOutTrace) {
		myAsked.clear();
		FT_Builder* builder = nullptr;
		FT_Question question{};
		FT_Result result = FT_StartCreate(theRoot, &builder, &question, aOutTrace);
		while (IsSuccess(result) && question.type != FT_QUESTION_NONE) {
			myAsked.push_back({question.type, question.path != nullptr ? question.path : ""});
			FT_Answer answer{};
			std::vector<std::string> names;
			std::vector<const char*> namePointers;
			std::string content;
			switch (question.type) {
			case FT_QUESTION_LIST_FOLDERS:
				names = PrivSubfoldersOf(question.path);
				for (const auto& name : names) {
					namePointers.push_back(name.c_str());
				}
				answer.folderNames = namePointers.data();
				answer.folderCount = namePointers.size();
				break;
			case FT_QUESTION_FILE_EXISTS:
				answer.exists = myFiles.count(PrivNormalize(question.path)) > 0;
				break;
			case FT_QUESTION_READ_FILE: {
				const auto file = myFiles.find(PrivNormalize(question.path));
				answer.found = file != myFiles.end();
				if (answer.found) {
					content = file->second;
					answer.content = content.c_str();
				}
				break;
			}
			case FT_QUESTION_PARSE_PERSON_FILE:
				if (myPersonRecord) {
					answer.person = &*myPersonRecord;
				} else {
					answer.useDefault = true;
				}
				break;
			case FT_QUESTION_PARSE_RELATIONS_FILE:
			case FT_QUESTION_NONE:
				answer.useDefault = true;
				break;
			}
			result = FT_Continue(builder, &answer, &question, aOutContext, aOutTrace);
		}
		if (!IsSuccess(result) && builder != nullptr) {
			(void)FT_CancelCreate(builder, aOutTrace);
		}
		return result;
	}

	[[nodiscard]] const std::vector<AskedQuestion>& Asked() const { return myAsked; }

	[[nodiscard]] static bool IsSuccess(FT_Result aResult) {
		return aResult == FT_OK || aResult == FT_OK_WITH_WARNINGS;
	}

private:
	static std::string PrivNormalize(std::string aPath) {
		std::replace(aPath.begin(), aPath.end(), '\\', '/');
		while (!aPath.empty() && aPath.back() == '/') {
			aPath.pop_back();
		}
		return aPath;
	}

	std::vector<std::string> PrivSubfoldersOf(const char* aPath) const {
		const std::string prefix = PrivNormalize(aPath) + "/";
		std::set<std::string> names;
		for (const auto& [path, content] : myFiles) {
			if (path.compare(0, prefix.size(), prefix) != 0) {
				continue;
			}
			const auto rest = path.substr(prefix.size());
			const auto slash = rest.find('/');
			if (slash != std::string::npos) {
				names.insert(rest.substr(0, slash));
			}
		}
		if (myReverse) {
			return {names.rbegin(), names.rend()};
		}
		return {names.begin(), names.end()};
	}

	std::map<std::string, std::string> myFiles;
	std::optional<FT_PersonRecord> myPersonRecord;
	std::vector<AskedQuestion> myAsked;
	bool myReverse = false;
};

// Creates a Context from a DataFolder; for tests that only care about the Context.
inline FT_Result
Create(DataFolder& aFolder, ContextGuard& aOutContext, TraceBuffer& aOutTrace) {
	return aFolder.Create(&aOutContext, &aOutTrace);
}

inline std::string
Text(const char* aText) {
	return aText != nullptr ? std::string(aText) : std::string("<null>");
}

inline std::vector<std::string>
Texts(const FT_TextList& aList) {
	std::vector<std::string> result;
	result.reserve(aList.count);
	for (size_t i = 0; i < aList.count; ++i) {
		result.emplace_back(aList.items[i]);
	}
	return result;
}

// The Person Ids in a view or buffer, sorted, for comparison.
inline std::vector<FT_PersonId>
Sorted(const FT_PersonId* aIds, size_t aCount) {
	std::vector<FT_PersonId> result(aIds, aIds + aCount);
	std::sort(result.begin(), result.end());
	return result;
}

inline std::string
Join(const std::vector<FT_PersonId>& aIds) {
	std::string result;
	for (const auto personId : aIds) {
		result += (result.empty() ? "" : ",") + std::to_string(personId);
	}
	return result;
}

} // namespace test
