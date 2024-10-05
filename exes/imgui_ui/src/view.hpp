#pragma once

#include <family_tree/api.h>

#include <array>
#include <string>
#include <vector>

static constexpr auto theInputfieldSize = 256;

class View {
public:
	void Print();
	void Shutdown();

private:
	Context* myContext = nullptr;
	std::vector<Person> mySearchResults;
	std::array<char, theInputfieldSize> myTitleFilter{};
	std::vector<std::array<char, theInputfieldSize>> myFirstNameFilters;
	std::array<char, theInputfieldSize> myTitleOfNobilityFilter{};
	std::vector<std::array<char, theInputfieldSize>> myLastNameFilters;

	static std::string PrivGetFolder();
	void PrivShowFilters();
	void PrivDoSearch();
	void PrivShowPerson(const Person& aPerson);
};