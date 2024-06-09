#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string_view>

namespace intern {
template <class Lhs, class Rhs>
bool
Equal(const Lhs& aLhs, const Rhs& aRhs) {
	return aLhs == aRhs;
}

template <class Lhs = char*, class Rhs = char*>
bool
Equal(const char* aLhs, const char* aRhs) {
	return std::string_view(aLhs) == aRhs;
}

template <class T, class... E>
bool
Check(
	std::string_view aFile,
	size_t aLine,
	std::string_view aVariableName,
	const T& aVariable,
	E... aExpectations) {
	bool result = (Equal(aVariable, aExpectations) || ...);
	if (!result) {
		std::cout << aFile << " (" << aLine << ")\n" << aVariableName << " expected to be: ";
		((std::cout << aExpectations << ", "), ...);
		std::cout << "but actualy: " << aVariable << '\n';
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
	bool result = !(Equal(aVariable, aExpectations) || ...);
	if (!result) {
		std::cout << aFile << " (" << aLine << ")\n" << aVariableName << " expected to not be: ";
		((std::cout << aExpectations << ", "), ...);
		std::cout << "but actualy: " << aVariable << '\n';
	}
	return result;
}
} // namespace intern

#define CHECK(aValue, ...) \
	if (!intern::Check(__FILE__, __LINE__, #aValue, aValue, __VA_ARGS__)) return false
#define CHECK_EXCLUDE(aValue, ...) \
	if (!intern::CheckExclude(__FILE__, __LINE__, #aValue, aValue, __VA_ARGS__)) return false
