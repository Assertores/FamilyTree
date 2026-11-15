#pragma once

#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>
#include <variant>
#include <vector>

#if _WIN32
#include <csignal>
#include <stdexcept>
#include <string>

namespace intern {
void
SignalHandler(int aSignal) {
	throw std::runtime_error("Signal: " + std::to_string(aSignal));
}
} // namespace intern
#else
#endif

#ifndef NDEBUG
#if _WIN32
#include <crtdbg.h>
#define LEAK_CHECK _CrtDumpMemoryLeaks() != 0
#else
#include <sanitizer/lsan_interface.h>
#define LEAK_CHECK __lsan_do_recoverable_leak_check() != 0
#endif
#else
#define LEAK_CHECK false
#endif

namespace intern {
template <class Lhs, class Rhs>
bool
Equal(const Lhs& aLhs, const Rhs& aRhs) {
	return aLhs == aRhs;
}

template <class Lhs = char*, class Rhs = char*>
bool
Equal(const char* aLhs, const char* aRhs) {
	if (aLhs == aRhs) {
		return true;
	}
	if (aLhs == nullptr) {
		return false;
	}
	return std::string_view(aLhs) == aRhs;
}

template <class... T, class Lhs = std::variant<T...>, class Rhs>
bool
Equal(const std::variant<T...>& aLhs, const Rhs& aRhs) {
	if (!std::holds_alternative<Rhs>(aLhs)) {
		return false;
	}
	return Equal(std::get<Rhs>(aLhs), aRhs);
}

template <class T, class Lhs = std::optional<T>, class Rhs>
bool
Equal(const std::optional<T>& aLhs, const Rhs& aRhs) {
	if (!aLhs) {
		return false;
	}
	if (std::is_same_v<Rhs, std::nullopt_t>) {
		return false;
	}
	return Equal(aLhs.value(), aRhs);
}

template <class T>
inline std::string
Print(const T& aValue) {
	std::stringstream stream;
	stream << aValue;
	return stream.str();
}

template <typename T>
inline std::string
Print(const std::vector<T>& aVector) {
	std::stringstream stream;
	for (const T& element : aVector) {
		stream << Print(element) << ' ';
	}
	return stream.str();
}

template <typename... T>
inline std::string
Print(const std::variant<T...>& aVariant) {
	return std::visit([&](const auto& aValue) { return Print(aValue); }, aVariant);
}

template <class T = std::nullopt_t>
inline std::string
Print(const std::nullopt_t& /*unused*/) {
	return "NullOpt";
}

template <typename T>
inline std::string
Print(const std::optional<T>& aOptional) {
	return aOptional ? Print(aOptional.value()) : Print(std::nullopt);
}

template <class T, class... E>
bool
Check(
	std::string_view aFile,
	size_t aLine,
	std::string_view aVariableName,
	const T& aVariable,
	E... aExpectations) {
	const bool result = (Equal(aVariable, aExpectations) || ...);
	if (!result) {
		std::cout << aFile << " (" << aLine << ")\n" << aVariableName << " expected to be: ";
		((std::cout << Print(aExpectations) << ", "), ...);
		std::cout << "but actually: " << Print(aVariable) << '\n';
	}
	return result;
}

template <class T, class... E>
bool
CheckExclude(
	std::string_view aFile,
	size_t aLine,
	std::string_view aVariableName,
	const T& aVariable,
	E... aExpectations) {
	const bool result = !(Equal(aVariable, aExpectations) || ...);
	if (!result) {
		std::cout << aFile << " (" << aLine << ")\n" << aVariableName << " expected to not be: ";
		((std::cout << Print(aExpectations) << ", "), ...);
		std::cout << "but actually: " << Print(aVariable) << '\n';
	}
	return result;
}

// The part of a test name that selects the tests to run, from the first command-line argument.
// Empty runs every test.
inline std::string_view&
Filter() {
	static std::string_view filter;
	return filter;
}

// How many tests were selected by the filter and ran.
inline size_t&
RunCount() {
	static size_t count = 0;
	return count;
}

// A filter that selected no test is a failure, so that a mistyped name does not look like success.
inline bool
NothingSelected() {
	return !Filter().empty() && RunCount() == 0;
}

inline bool
Run(const char* aTestName, bool (*aTest)()) {
	if (std::string_view(aTestName).find(Filter()) == std::string_view::npos) {
		return true;
	}
	++RunCount();
	std::cout << "[TEST   ] ===== " << aTestName << '\n';
	auto result = false;
	try {
		result = aTest();
	} catch (const std::exception& e) {
		std::cout << "caught exception: " << e.what() << '\n';
	} catch (...) {
		std::cout << "caught exception.\n";
	}
	std::cout << "[" << (result ? "SUCCESS" : " FAILED") << "]\n";
	return result;
}

class ResultTracker {
public:
	explicit ResultTracker(bool aIsTestSuite)
		: myIsTestSuite(aIsTestSuite) {}
	ResultTracker(const ResultTracker&) = delete;
	ResultTracker& operator=(const ResultTracker&) = delete;

	ResultTracker(ResultTracker&&) = delete;
	ResultTracker& operator=(ResultTracker&&) = delete;

	~ResultTracker() {
		if (!myIsTestSuite && LEAK_CHECK) {
			std::cout << "\n!!!!! >> A memory leak was detected << !!!!!\n";
		}
		if (!myIsTestSuite && NothingSelected()) {
			std::cout << "\nNo test name contains \"" << Filter() << "\"\n";
		}

		std::cout << (myIsTestSuite ? "[ SUITE ] " : "\n")
				  << (PrivSucceeded() ? "All Succeeded" : "Failure detected") << '\n';
		if (!myIsTestSuite) {
			std::cout << "===== DONE =====\n";
		}
	}

	ResultTracker& operator&=(bool aNewValue) {
		myResult &= aNewValue;
		return *this;
	}

	// NOLINTNEXTLINE
	operator int() const { return PrivSucceeded() ? 0 : 1; }
	// NOLINTNEXTLINE
	operator bool() const { return PrivSucceeded(); }

private:
	[[nodiscard]] bool PrivSucceeded() const {
		return myResult && (myIsTestSuite || !NothingSelected());
	}

	bool myResult = true;
	bool myIsTestSuite = false;
};

// The first command-line argument, if any, selects the tests whose name contains it.
inline ResultTracker
Setup(int aArgumentCount, const char* const* aArguments) {
#if _WIN32
	(void)signal(SIGSEGV, SignalHandler);
#endif
	if (aArgumentCount > 1) {
		Filter() = aArguments[1];
	}
	return ResultTracker(false);
}

inline ResultTracker
SetupSuite(std::string_view aName) {
	std::cout << "\n[ SUITE ] " << aName << '\n';
	return ResultTracker(true);
}
} // namespace intern

#define CHECK(aValue, ...) \
	if (!intern::Check(__FILE__, __LINE__, #aValue, aValue, __VA_ARGS__)) return false
#define CHECK_EXCLUDE(aValue, ...) \
	if (!intern::CheckExclude(__FILE__, __LINE__, #aValue, aValue, __VA_ARGS__)) return false
#define RUN(aTest) intern::Run(#aTest, &(aTest))

#define SETUP(aArgumentCount, aArguments) intern::Setup(aArgumentCount, aArguments)
#define SUITE(aName) intern::SetupSuite(aName)
