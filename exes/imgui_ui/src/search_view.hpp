#pragma once

#include <context_adapter.hpp>

#include <array>
#include <string>
#include <vector>

namespace ui {

static constexpr auto theInputfieldSize = 256;

class SearchView {
public:
	void Print();

private:
	std::shared_ptr<ContextAdapter> myContext;
	std::vector<Person> mySearchResults;
	std::array<char, theInputfieldSize> myTitleFilter{};
	std::vector<std::array<char, theInputfieldSize>> myFirstNameFilters;
	std::array<char, theInputfieldSize> myTitleOfNobilityFilter{};
	std::vector<std::array<char, theInputfieldSize>> myLastNameFilters;

	static std::string PrivGetFolder();
	void PrivShowFilters();
	void PrivDoSearch();
	void PrivShowPerson(const Person& aPerson);

	void PrivShowName(const Person& aPerson);
	void PrivShowDates(const Person& aPerson);
	void PrivShowProfessions(const Person& aPerson);
	void PrivShowResidence(const Person& aPerson);
	void PrivShowRemarks(const Person& aPerson);
};
} // namespace ui
