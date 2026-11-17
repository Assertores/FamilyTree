/// @file
/// @brief Public API of the Family Tree Library.
///
/// The Library reads a Data Folder, the Person Files and the Relations File of a family, into an
/// immutable Context, and answers questions about it: the Details of a Person, their Relations,
/// Parents, Children, Siblings and Partners, their Generation, a search, and the Family Tree
/// around a Person.
///
/// Rules that hold for every function:
/// - Every function returns an @ref FT_Result, and returns its values through output parameters.
///   Output parameters are only written when the function returns @ref FT_OK or
///   @ref FT_OK_WITH_WARNINGS.
/// - Every function takes an optional @ref FT_Trace as its last parameter. NULL means no Trace.
/// - Results are either views or caller buffers. A view is a pointer into the Context, valid until
///   @ref FT_Free. A caller buffer is provided by the Application; its minimum size is known before
///   the call, e.g. from @ref FT_GetPersonCount, so no function has to be called twice.
/// - All text is UTF-8 and zero terminated, and text inside the Context is never NULL. A missing
///   Detail points to the Default String of the Context, an empty text that
///   @ref FT_IsDefaultString recognizes by its address. A Detail that is empty in the Person File
///   is present, and points to another empty text. It is therefore always safe to write:
///   @code{.c}
///   printf("%s %s, born %s", person->firstNames.items[0], person->lastNames.items[0],
///          person->dateOfBirth);
///   @endcode
/// - The Library allocates memory only while it creates a Context, and frees all of it in
///   @ref FT_Free. The API is not thread safe for one Context, but separate Contexts are
///   independent of each other.
///
/// See docs/decisions/adr_0020.md and docs/decisions/adr_0016.md.
#ifndef FT_API_H
#define FT_API_H

/// @cond DO_NOT_DOCUMENT
#ifndef FT_API
#if defined(_WIN32) && defined(FT_BUILD_SHARED)
#define FT_API __declspec(dllexport)
#elif defined(_WIN32) && defined(FT_USE_SHARED)
#define FT_API __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define FT_API __attribute__((visibility("default")))
#else
#define FT_API
#endif
#endif
/// @endcond

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/// @defgroup Results Results and Trace
/// @brief How every function reports its success and what it did.
/// @{

/// @brief The result of every function of the Library.
///
/// @ref FT_OK and @ref FT_OK_WITH_WARNINGS mean success; every other value is an error, and the
/// output parameters were not written.
typedef enum {
	FT_OK = 0,			 ///< Done.
	FT_OK_WITH_WARNINGS, ///< Done, but the Library worked around problems in the data; the Trace
						 ///< holds them as warnings, e.g. a skipped Person File.
	FT_ERROR_INVALID_ARGUMENT,	///< A null pointer or a value out of range.
	FT_ERROR_UNKNOWN_PERSON,	///< The Person Id does not exist in the Context.
	FT_ERROR_UNKNOWN_FAMILY,	///< The Family index does not exist in the Context.
	FT_ERROR_BUFFER_TOO_SMALL,	///< A caller buffer is smaller than the function requires;
								///< nothing was written.
	FT_ERROR_OUT_OF_MEMORY,		///< Memory ran out; only possible while creating a Context.
	FT_ERROR_UNEXPECTED_ANSWER, ///< The answer does not fit the Question it answers.
	FT_ERROR_PLATFORM,		///< The Default Platform failed, e.g. the Data Folder does not exist.
	FT_ERROR_NOT_SUPPORTED, ///< There is no Default Platform for this operating system.
	FT_ERROR_CONFLICTING_GENERATIONS, ///< The Relations contradict each other, e.g. a Person is
									  ///< their own ancestor; no Context was created.
} FT_Result;

/// @brief A text buffer the Library writes its reports into.
///
/// Set @a buffer and @a capacity, and everything else to zero, then pass a pointer to it to any
/// number of calls. Every call appends one Subtrace, a first line with its outcome, the function
/// and a short description, followed by its reports, indented by one tab per depth:
/// @code
/// [WARN]	FT_Create	/home/vater/Familie
/// 	[WARN]	ReadPersonFile	1_Philippa_Perry/data.json
/// 		WARNING	unknown key "death"
/// @endcode
/// The outcome is `[ OK ]`, `[WARN]` or `[FAIL]`, the worst of everything inside the Subtrace.
/// Fields are separated by tabs. A Subtrace that succeeded keeps only its first line and a summary
/// of its steps; one with warnings also keeps its warnings, and one that failed keeps every line.
/// When the buffer is full, the oldest lines are dropped. See docs/decisions/adr_0018.md.
typedef struct {
	char* buffer;		 ///< Zero terminated text, one Subtrace or report per line.
	size_t capacity;	 ///< Size of @a buffer in bytes; 0 discards everything.
	size_t used;		 ///< Bytes written, without the terminating zero.
	size_t droppedLines; ///< Lines that were dropped because the buffer was full, the oldest first.
	size_t depth;		 ///< Internal: the number of open Subtraces.
} FT_Trace;

/// @}

/// @defgroup Data Data
/// @brief The Context and what it returns.
/// @{

/// @brief Everything the Library read from a Data Folder, and everything derived from it.
///
/// Opaque, and immutable once created. Create it with @ref FT_Create or @ref FT_StartCreate, and
/// free it with @ref FT_Free.
typedef struct FT_Context FT_Context;

/// @brief A Context being created through Questions; opaque. See @ref FT_StartCreate.
typedef struct FT_Builder FT_Builder;

/// @brief Identifies a Person: the "Id" of their Person File.
///
/// The Library never assigns ids of its own; Person Ids need not be consecutive. An Application
/// gets them from @ref FT_Search, e.g. with an empty prototype and a minimum of 0 to get everyone.
typedef long long FT_PersonId;

/// @brief Identifies a Family within one Context, from 0 to @ref FT_GetFamilyCount minus 1.
///
/// Families have no id in the data, so an index is only meaningful within the Context it came
/// from.
typedef size_t FT_FamilyIndex;

/// @brief A Detail with several texts, e.g. the first names.
typedef struct {
	/// The texts. Never NULL: if @a count is 0, @a items[0] still exists. It is the Default String
	/// if the Detail is missing in the Person File, and another empty text if the list is empty
	/// there, i.e. known not to exist (docs/decisions/adr_0020.md).
	const char* const* items;
	size_t count; ///< The number of texts.
} FT_TextList;

/// @brief A Place of Residence of a Person, with the dates the Person lived there.
typedef struct {
	const char* name;	   ///< The name of the place, e.g. "Würzburg".
	const char* startDate; ///< When the Person moved there, as written in the Person File.
	const char* endDate;   ///< When the Person moved away, as written in the Person File.
} FT_Residence;

/// @brief The Places of Residence of a Person.
typedef struct {
	/// The Places of Residence. Never NULL: if @a count is 0, @a items[0] is a placeholder whose
	/// texts are all the Default String if the Detail is missing, or all another empty text if the
	/// list is empty in the Person File.
	const FT_Residence* items;
	size_t count; ///< The number of Places of Residence.
} FT_ResidenceList;

/// @brief The Details of a Person.
///
/// Every text is returned exactly as written in the Person File, e.g. a date like "um 1850". Use
/// @ref FT_IsDefaultString to find out whether a Detail is missing.
typedef struct {
	FT_PersonId id;						///< The Person Id.
	const char* title;					///< e.g. "Dr."
	FT_TextList firstNames;				///< All first names, e.g. "Anna" and "Maria".
	const char* titleOfNobility;		///< e.g. "von"
	FT_TextList lastNames;				///< All last names, e.g. a birth name and a married name.
	const char* gender;					///< As written in the Person File.
	const char* dateOfBirth;			///< As written in the Person File.
	const char* placeOfBirth;			///< As written in the Person File.
	const char* dateOfDeath;			///< As written in the Person File.
	const char* placeOfDeath;			///< As written in the Person File.
	FT_TextList professions;			///< All professions.
	FT_ResidenceList placesOfResidence; ///< All Places of Residence.
	FT_TextList remarks;				///< Free text, one item per line.
} FT_Person;

/// @brief How a Relation restricts the Generations of its two Persons.
typedef enum {
	FT_RELATION_UNRESTRICTED = 0, ///< No restriction, e.g. a marriage.
	FT_RELATION_STRICTLY_LOWER,	  ///< The second Person is a Child of the first.
	FT_RELATION_STRICTLY_HIGHER,  ///< The second Person is a Parent of the first.
} FT_RelationType;

/// @brief A row of the Relations File: a Relation between two Persons.
typedef struct {
	FT_PersonId first;		  ///< The first Person.
	FT_PersonId second;		  ///< The second Person.
	FT_RelationType type;	  ///< How the Relation restricts their Generations.
	const char* relationship; ///< What the first Person is to the second, e.g. "Stiefvater".
	const char* startDate;	  ///< When the Relation began, as written in the Relations File.
	const char* endDate;	  ///< When the Relation ended, as written in the Relations File.
} FT_Relation;

/// @brief A Family: Parents together with all the Children they have in common.
typedef struct {
	const FT_PersonId* parents;	 ///< The Parents.
	size_t parentCount;			 ///< The number of Parents.
	const FT_PersonId* children; ///< The Children.
	size_t childCount;			 ///< The number of Children.
} FT_Family;

/// @}

/// @defgroup Creation Creation
/// @brief Turning a Data Folder into a Context.
///
/// @ref FT_Create reads the Data Folder in one call with the Default Platform. An Application
/// that reads the data in its own way, e.g. from an archive, uses @ref FT_StartCreate and
/// @ref FT_Continue instead: the Library asks Questions, e.g. for the content of a file, and the
/// Application answers each one itself, or leaves it to the Default Platform. The Library never
/// calls back into the Application (docs/decisions/adr_0005.md).
/// @{

/// @brief What the Library asks for while it creates a Context.
typedef enum {
	FT_QUESTION_NONE = 0,	  ///< No Question: the creation is over, with a Context or an error.
	FT_QUESTION_LIST_FOLDERS, ///< The names of the subfolders of @ref FT_Question::path.
	FT_QUESTION_FILE_EXISTS,  ///< Whether the file @ref FT_Question::path exists.
	FT_QUESTION_READ_FILE,	  ///< The content of the file @ref FT_Question::path.
	FT_QUESTION_PARSE_PERSON_FILE,	  ///< The parsed content of the Person File in
									  ///< @ref FT_Question::text.
	FT_QUESTION_PARSE_RELATIONS_FILE, ///< The parsed content of the Relations File in
									  ///< @ref FT_Question::text.
} FT_QuestionType;

/// @brief A Question of the Library, see @ref FT_QuestionType.
typedef struct {
	FT_QuestionType type; ///< What is asked.
	/// The folder or file asked about. The Data Folder joined with the relative parts by `/`,
	/// which Windows accepts as well. A parsing Question has the path of the file it read before,
	/// e.g. to name the file in messages (docs/decisions/adr_0020.md).
	const char* path;
	/// The text to parse, for PARSE_PERSON_FILE and PARSE_RELATIONS_FILE: a copy of the file that
	/// belongs to the Library, valid until the Question is answered. The Application may change it
	/// while it parses, e.g. decode its strings in place; strings of the answer that start and end
	/// within it are kept without a copy. If the answer fails, the Library restores the text, so
	/// that the Question can be answered again (docs/decisions/adr_0020.md).
	char* text;
} FT_Question;

/// @brief The parsed content of a Person File, as an answer to FT_QUESTION_PARSE_PERSON_FILE.
///
/// A NULL text, and a list whose items are NULL, mean missing; the Library replaces them with the
/// Default String. An empty text, and a list with items set and a count of 0, mean present but
/// empty. Its Format Version is checked by whoever parses the file, before the rest is read
/// (docs/decisions/adr_0020.md).
typedef struct {
	FT_Person person;			 ///< The Person Id and the Details.
	const char* recording;		 ///< The Recording, relative to the Person Folder.
	const char* const* pictures; ///< The Pictures, relative to the Person Folder.
	size_t pictureCount;		 ///< The number of Pictures.
} FT_PersonRecord;

/// @brief The parsed content of a Relations File, as an answer to
/// FT_QUESTION_PARSE_RELATIONS_FILE. A NULL text of a Relation means missing.
typedef struct {
	const FT_Relation* items; ///< The Relations.
	size_t count;			  ///< The number of Relations.
} FT_RelationList;

/// @brief The answer to a Question. Only the fields of the Question asked are read.
///
/// The Library copies what it needs from the answer before @ref FT_Continue returns, so the
/// answer and everything it points to may be freed afterwards. The only exception are strings of a
/// parsing answer that lie within @ref FT_Question::text, which the Library owns.
typedef struct {
	bool useDefault; ///< Leave this Question to the Default Platform; every other field is ignored.
	const char* const* folderNames; ///< LIST_FOLDERS: the names of the subfolders.
	size_t folderCount;				///< LIST_FOLDERS: the number of subfolders.
	bool exists;					///< FILE_EXISTS: whether the file exists.
	bool found;						///< READ_FILE: whether the file could be read.
	const char* content;			///< READ_FILE: the content, zero terminated.
	/// PARSE_PERSON_FILE: the parsed Person File, or NULL if it could not be read, e.g. as its
	/// Format Version is not supported; the Library then skips the file with a warning.
	const FT_PersonRecord* person;
	/// PARSE_RELATIONS_FILE: the parsed Relations File, or NULL if it could not be read, like
	/// @a person.
	const FT_RelationList* relations;
} FT_Answer;

/// @brief Creates a Context from a Data Folder in one call, leaving every Question to the
/// Default Platform.
/// @param aDataFolder The path of the Data Folder, UTF-8.
/// @param aOutContext Receives the Context; free it with @ref FT_Free.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_OK_WITH_WARNINGS if the Library skipped data it could not read,
/// e.g. a malformed Person File or a missing Relations File. @ref FT_ERROR_PLATFORM if the Data
/// Folder cannot be read, @ref FT_ERROR_NOT_SUPPORTED on an operating system without a Default
/// Platform, @ref FT_ERROR_CONFLICTING_GENERATIONS if the Relations contradict each other.
FT_API FT_Result FT_Create(const char* aDataFolder, FT_Context** aOutContext, FT_Trace* aOutTrace);

/// @brief Starts creating a Context through Questions.
///
/// Never finishes the creation itself: it always returns the first Question, the folders of the
/// Data Folder, so that every creation can be cancelled with @ref FT_CancelCreate.
/// @param aDataFolder The path of the Data Folder, as the Application understands it.
/// @param aOutBuilder Receives the Builder, which @ref FT_Continue and @ref FT_CancelCreate take.
/// @param aOutQuestion Receives the first Question.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_ERROR_INVALID_ARGUMENT for a NULL Data Folder.
FT_API FT_Result FT_StartCreate(
	const char* aDataFolder,
	FT_Builder** aOutBuilder,
	FT_Question* aOutQuestion,
	FT_Trace* aOutTrace);

/// @brief Answers the last Question, and returns the next one.
///
/// When the next Question is @ref FT_QUESTION_NONE, the creation is over: the Context is in
/// @a aOutContext, and the Builder has been freed. On an error, the Builder is left as it was
/// before the call, and the same Question is still open: answer it again, e.g. after fixing a file,
/// or stop with @ref FT_CancelCreate.
/// @param aBuilder The Builder from @ref FT_StartCreate.
/// @param aAnswer The answer to the last Question; copied before the function returns.
/// @param aOutQuestion Receives the next Question.
/// @param aOutContext Receives the Context when the creation is over; free it with @ref FT_Free.
/// @param aOutTrace Optional Trace, or NULL.
/// @return The call that ends the creation returns @ref FT_OK_WITH_WARNINGS if a warning was
/// reported at any time during the creation, not only during that call. Errors:
/// @ref FT_ERROR_INVALID_ARGUMENT for a NULL Builder or answer, @ref FT_ERROR_UNEXPECTED_ANSWER,
/// @ref FT_ERROR_OUT_OF_MEMORY, @ref FT_ERROR_CONFLICTING_GENERATIONS.
FT_API FT_Result FT_Continue(
	FT_Builder* aBuilder,
	const FT_Answer* aAnswer,
	FT_Question* aOutQuestion,
	FT_Context** aOutContext,
	FT_Trace* aOutTrace);

/// @brief Stops a creation that is not over, also after an error, and frees the Builder.
/// @param aBuilder The Builder from @ref FT_StartCreate.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_ERROR_INVALID_ARGUMENT for a NULL Builder.
FT_API FT_Result FT_CancelCreate(FT_Builder* aBuilder, FT_Trace* aOutTrace);

/// @brief Frees the Context and everything it ever returned.
///
/// Every view taken from the Context becomes invalid. Other Contexts are not affected.
/// @param aContext The Context to free.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_ERROR_INVALID_ARGUMENT for a NULL Context.
FT_API FT_Result FT_Free(FT_Context* aContext, FT_Trace* aOutTrace);

/// @}

/// @defgroup Bounds Upper bounds
/// @brief The sizes every caller buffer can be allocated with once.
/// @{

/// @brief The number of Persons in the Context; the size of the Person buffers.
/// @param aContext The Context.
/// @param aOutCount Receives the number of Persons.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_ERROR_INVALID_ARGUMENT.
FT_API FT_Result
FT_GetPersonCount(const FT_Context* aContext, size_t* aOutCount, FT_Trace* aOutTrace);

/// @brief The number of Families in the Context; the size of the Family buffers.
/// @param aContext The Context.
/// @param aOutCount Receives the number of Families.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_ERROR_INVALID_ARGUMENT.
FT_API FT_Result
FT_GetFamilyCount(const FT_Context* aContext, size_t* aOutCount, FT_Trace* aOutTrace);

/// @}

/// @defgroup Views Views
/// @brief Pointers into the Context, valid until @ref FT_Free.
///
/// Every function returns @ref FT_ERROR_INVALID_ARGUMENT for a NULL pointer, and
/// @ref FT_ERROR_UNKNOWN_PERSON or @ref FT_ERROR_UNKNOWN_FAMILY for an id that does not exist.
/// An empty result is a count of 0, not an error.
/// @{

/// @brief The Details of a Person.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutPerson Receives a view of the Person.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetPerson(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_Person** aOutPerson,
	FT_Trace* aOutTrace);

/// @brief All Relations that contain the Person, as the first or as the second Person.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutRelations Receives a view of the Relations.
/// @param aOutCount Receives the number of Relations.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetRelations(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_Relation** aOutRelations,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Parents of a Person.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutParents Receives a view of the Person Ids of the Parents.
/// @param aOutCount Receives the number of Parents.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetParents(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutParents,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Children of a Person.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutChildren Receives a view of the Person Ids of the Children.
/// @param aOutCount Receives the number of Children.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetChildren(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutChildren,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Siblings of a Person: everyone with at least one Parent in common, without the
/// Person themselves.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutSiblings Receives a view of the Person Ids of the Siblings.
/// @param aOutCount Receives the number of Siblings.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetSiblings(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutSiblings,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Partners of a Person: everyone with at least one Child in common. An Unrestricted
/// Relation alone, e.g. a marriage without Children, does not make two Persons Partners.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutPartners Receives a view of the Person Ids of the Partners.
/// @param aOutCount Receives the number of Partners.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetPartners(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutPartners,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Families a Person is a member of: the one they are a Child in, and every one they
/// are a Parent in.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutFamilies Receives a view of the Family indices; read each with @ref FT_GetFamily.
/// @param aOutCount Receives the number of Families.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetFamiliesOf(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_FamilyIndex** aOutFamilies,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The members of a Family.
/// @param aContext The Context.
/// @param aFamily The Family index.
/// @param aOutFamily Receives a view of the Family.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetFamily(
	const FT_Context* aContext,
	FT_FamilyIndex aFamily,
	const FT_Family** aOutFamily,
	FT_Trace* aOutTrace);

/// @brief The paths of the Pictures of a Person whose files exist.
///
/// The paths are the Person Folder joined with the names in the Person File; Pictures whose files
/// were missing during the creation were reported as warnings and are left out.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutPaths Receives a view of the paths.
/// @param aOutCount Receives the number of paths; 0 if the Person has no Pictures.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetPicturePaths(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const char* const** aOutPaths,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The path of the Recording of a Person, if its file exists.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutPath Receives a view of the path, or NULL if the Person has no Recording, or its
/// file was missing during the creation.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetRecordingPath(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const char** aOutPath,
	FT_Trace* aOutTrace);

/// @}

/// @defgroup Values Values
/// @{

/// @brief Whether a text is the Default String the Context gives every missing Detail.
///
/// It compares addresses, so it recognizes the pointers the Context returned, but not copies of
/// them. An empty text that was present in the Person File is not the Default String.
/// @param aContext The Context the text came from.
/// @param aText The text to check.
/// @param aOutMissing Receives true if @a aText is the Default String.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or @ref FT_ERROR_INVALID_ARGUMENT.
FT_API FT_Result FT_IsDefaultString(
	const FT_Context* aContext,
	const char* aText,
	bool* aOutMissing,
	FT_Trace* aOutTrace);

/// @brief The Generation of a Person relative to a reference Person.
///
/// Generations count from 0 at the top of a Family Graph downwards, towards the Children; the
/// result is the Generation of @a aPerson minus that of @a aReference. A Child is therefore 1
/// relative to its Parent, and a grandparent -2 relative to their grandchild.
/// @param aContext The Context.
/// @param aPerson The Person Id of the Person.
/// @param aReference The Person Id of the reference Person.
/// @param aOutGeneration Receives the relative Generation.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetRelativeGeneration(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	FT_PersonId aReference,
	int* aOutGeneration,
	FT_Trace* aOutTrace);

/// @}

/// @defgroup Buffers Buffer queries
/// @brief Functions that fill caller buffers in a single call.
///
/// Every buffer must be at least as large as the upper bound the function names. A smaller one
/// fails with @ref FT_ERROR_BUFFER_TOO_SMALL, and nothing is written.
/// @{

/// @brief Searches for the Persons whose Details match a prototype.
///
/// A given Detail is a text of the prototype that is neither NULL nor "". Every value of a list,
/// and every name or date of a Place of Residence, counts as one given Detail. It matches if its
/// text is contained in the Person's Detail, or in any one of its values; a Detail missing in the
/// Person File never matches. The id of the prototype is ignored. An empty prototype with a
/// minimum of 0 returns every Person.
/// @param aContext The Context.
/// @param aPrototype The Details to search for.
/// @param aMinimumMatches How many given Details must match at least.
/// @param aOutFound Caller buffer for the Person Ids found.
/// @param aCapacity The size of @a aOutFound; at least @ref FT_GetPersonCount.
/// @param aOutCount Receives the number of Persons found.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, @ref FT_ERROR_INVALID_ARGUMENT or @ref FT_ERROR_BUFFER_TOO_SMALL.
FT_API FT_Result FT_Search(
	const FT_Context* aContext,
	const FT_Person* aPrototype,
	size_t aMinimumMatches,
	FT_PersonId* aOutFound,
	size_t aCapacity,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Persons that are Parents of all of the given Persons.
/// @param aContext The Context.
/// @param aPersons The Person Ids; an empty set returns no Persons.
/// @param aPersonCount The number of Person Ids in @a aPersons.
/// @param aOutParents Caller buffer for the Person Ids of the common Parents.
/// @param aCapacity The size of @a aOutParents; at least @ref FT_GetPersonCount.
/// @param aOutCount Receives the number of common Parents.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetCommonParents(
	const FT_Context* aContext,
	const FT_PersonId* aPersons,
	size_t aPersonCount,
	FT_PersonId* aOutParents,
	size_t aCapacity,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief The Persons that are Children of all of the given Persons.
/// @param aContext The Context.
/// @param aPersons The Person Ids; an empty set returns no Persons.
/// @param aPersonCount The number of Person Ids in @a aPersons.
/// @param aOutChildren Caller buffer for the Person Ids of the common Children.
/// @param aCapacity The size of @a aOutChildren; at least @ref FT_GetPersonCount.
/// @param aOutCount Receives the number of common Children.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetCommonChildren(
	const FT_Context* aContext,
	const FT_PersonId* aPersons,
	size_t aPersonCount,
	FT_PersonId* aOutChildren,
	size_t aCapacity,
	size_t* aOutCount,
	FT_Trace* aOutTrace);

/// @brief A Person of a Family Tree.
typedef struct {
	FT_PersonId person;		///< The Person Id.
	int relativeGeneration; ///< The Generation relative to the Person the Family Tree is built
							///< around, as in @ref FT_GetRelativeGeneration.
} FT_TreePerson;

/// @brief The Family Tree around a Person: the Person, and all Families within a Distance, with
/// all their members.
///
/// The Distance counts steps from Family to Family through a Person who is a member of both; the
/// Families the Person is a member of have the Distance 0. The Person is always part of the
/// result, also if they are a member of no Family. See docs/decisions/adr_0009.md.
/// @param aContext The Context.
/// @param aPerson The Person Id of the Person the Family Tree is built around.
/// @param aDistance The largest Distance of the Families included.
/// @param aOutPersons Caller buffer for the Persons, each with their relative Generation.
/// @param aPersonCapacity The size of @a aOutPersons; at least @ref FT_GetPersonCount.
/// @param aOutPersonCount Receives the number of Persons.
/// @param aOutFamilies Caller buffer for the Family indices; read each with @ref FT_GetFamily.
/// @param aFamilyCapacity The size of @a aOutFamilies; at least @ref FT_GetFamilyCount.
/// @param aOutFamilyCount Receives the number of Families.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, or an error.
FT_API FT_Result FT_GetFamilyTree(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	size_t aDistance,
	FT_TreePerson* aOutPersons,
	size_t aPersonCapacity,
	size_t* aOutPersonCount,
	FT_FamilyIndex* aOutFamilies,
	size_t aFamilyCapacity,
	size_t* aOutFamilyCount,
	FT_Trace* aOutTrace);

/// @}

/// @defgroup Media Media
/// @brief Showing Pictures and playing Recordings with the programs of the operating system.
///
/// The Library itself only provides the paths, see @ref FT_GetPicturePaths and
/// @ref FT_GetRecordingPath. These functions are a default for Applications that do not show the
/// Media themselves, on Windows and Linux (docs/decisions/adr_0006.md).
/// @{

/// @brief Opens a Picture with the program the operating system uses for it.
/// @param aPath The path of the Picture.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, @ref FT_ERROR_INVALID_ARGUMENT for a NULL path, @ref FT_ERROR_PLATFORM if
/// the program could not be started, or @ref FT_ERROR_NOT_SUPPORTED on another operating system.
FT_API FT_Result FT_DefaultShowPicture(const char* aPath, FT_Trace* aOutTrace);

/// @brief Plays a Recording with the program the operating system uses for it.
/// @param aPath The path of the Recording.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, @ref FT_ERROR_INVALID_ARGUMENT for a NULL path, @ref FT_ERROR_PLATFORM if
/// the program could not be started, or @ref FT_ERROR_NOT_SUPPORTED on another operating system.
FT_API FT_Result FT_DefaultPlayRecording(const char* aPath, FT_Trace* aOutTrace);

/// @brief Opens every Picture of a Person with @ref FT_DefaultShowPicture.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, also if the Person has no Pictures; otherwise an error.
FT_API FT_Result
FT_ShowPicturesOfPerson(const FT_Context* aContext, FT_PersonId aPerson, FT_Trace* aOutTrace);

/// @brief Plays the Recording of a Person with @ref FT_DefaultPlayRecording.
/// @param aContext The Context.
/// @param aPerson The Person Id.
/// @param aOutTrace Optional Trace, or NULL.
/// @return @ref FT_OK, also if the Person has no Recording; otherwise an error.
FT_API FT_Result
FT_PlayRecordingOfPerson(const FT_Context* aContext, FT_PersonId aPerson, FT_Trace* aOutTrace);

/// @}

#ifdef __cplusplus
}
#endif

#endif // FT_API_H
